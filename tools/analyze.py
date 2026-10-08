#!/usr/bin/env python3
"""
Whole-program static analysis of DARKLAND.EXE (RTLink/Plus overlaid, MSC 6 large model).

Produces:
  build/analysis/db.json        images, segments, functions, call graph, jump tables, vectors
  build/analysis/lst/<img>.lst  annotated disassembly listing per image

Address model
  Every image (root, overlay sections, data section) lives at its *link-time* segment.
  A location is (image, linear) with linear = seg*16 + off in link-time space. Overlay
  sections alias each other in linear space, so the image id is always carried along.
  Images: 'root' (MZ load module), 'sNNN' (RTLink section NNN). s136 is the preloaded
  data section (far data segments + DGROUP).

Cross-image references
  - direct far call/jump: resolved within the referencing image's visible set
    (itself, its overlay ancestors, root, s136)
  - RTLink vectors: 10-byte stubs in the manager segment (root 09c0:17e3..354d):
        e8 <rel16>      call vector_handler (09c0:052b)
        ea <off> <seg>  jmp far target   (relocated)
        <u16>           section index of the target
    A far call to a vector is recorded as a call to (section, target).
"""
import sys, os, json, struct, collections, re
import capstone
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from rtlink import RTLinkExe

MGR_SEG = 0x9c0
VEC_LO, VEC_HI = 0x17e3, 0x354d          # vector table in manager segment (offsets)
DATA_SECTION = 136

md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_16)
md.detail = False

COND_JUMPS = {0x70 + i for i in range(16)} | {0xe0, 0xe1, 0xe2, 0xe3}


class Image:
    def __init__(self, name, base_seg, mem, reloc_sites, parent=None, section=None):
        self.name = name
        self.base_seg = base_seg
        self.mem = mem
        self.lo = base_seg * 16
        self.hi = base_seg * 16 + len(mem)
        self.reloc_sites = set(reloc_sites)      # linear addresses of relocated words
        self.parent = parent                     # parent image name (overlay tree) or None
        self.section = section
        self.segs = set([base_seg])              # segment bases inside this image
        self.is_data = False

    def contains(self, lin):
        return self.lo <= lin < self.hi

    def byte(self, lin):
        return self.mem[lin - self.lo]

    def word(self, lin):
        return struct.unpack_from('<H', self.mem, lin - self.lo)[0]

    def bytes_at(self, lin, n):
        return bytes(self.mem[lin - self.lo: lin - self.lo + n])

    def seg_of(self, lin, segs_sorted):
        # largest segment base <= lin (segs_sorted ascending list of bases*16)
        import bisect
        i = bisect.bisect_right(segs_sorted, lin) - 1
        return segs_sorted[i] // 16 if i >= 0 else self.base_seg


class Program:
    def __init__(self, exe_path):
        self.exe = exe = RTLinkExe(exe_path)
        self.images = {}
        root = Image('root', 0, exe.root_memory(), exe.reloc_sites())
        self.images['root'] = root
        for s in exe.sections:
            name = 's%03d' % s.index
            parent = 'root' if s.parent == 0xffff else 's%03d' % s.parent
            img = Image(name, s.load_seg, exe.section_memory(s.index), exe.reloc_sites(s.index),
                        parent=parent, section=s.index)
            if s.index == DATA_SECTION:
                img.is_data = True
                img.parent = 'root'
            self.images[name] = img
        self.vectors = {}       # vector offset -> (target image name, linear)
        self.dgroup = None

    # ------------------------------------------------------------------ visibility
    def visible(self, name):
        out = []
        n = name
        while n is not None:
            out.append(n)
            n = self.images[n].parent
        if 's%03d' % DATA_SECTION not in out:
            out.append('s%03d' % DATA_SECTION)
        return out

    def resolve(self, from_img, lin):
        for n in self.visible(from_img):
            if self.images[n].contains(lin):
                return n
        return None

    # ------------------------------------------------------------------ vectors
    def load_vectors(self):
        root = self.images['root']
        o = VEC_LO
        while o < VEC_HI:
            lin = MGR_SEG * 16 + o
            b = root.bytes_at(lin, 10)
            assert b[0] == 0xe8 and b[3] == 0xea, hex(o)
            toff, tseg, sect = struct.unpack_from('<HHH', b, 4)
            tname = 'root' if sect == 0xffff else 's%03d' % sect
            tlin = tseg * 16 + toff
            assert self.images[tname].contains(tlin), (hex(o), tname, hex(tseg), hex(toff))
            self.vectors[o] = (tname, tlin, tseg)
            o += 10

    # ------------------------------------------------------------------ segments
    def collect_segments(self):
        """Segment bases = relocated segment values, resolved in the referencing image's context."""
        for name, img in self.images.items():
            for site in img.reloc_sites:
                v = img.word(site)
                t = self.resolve(name, v * 16)
                if t is None and v * 16 == self.images[name].hi:
                    continue
                if t is not None:
                    self.images[t].segs.add(v)
        for o, (tname, tlin, tseg) in self.vectors.items():
            self.images[tname].segs.add(tseg)
        # relocation entries are (offset, frame-of-containing-segment): direct evidence of frames
        self.reloc_frames = collections.defaultdict(list)     # img -> [(lin, frame)]
        for off, seg in self.exe.mz_relocs:
            self.images['root'].segs.add(seg)
            self.reloc_frames['root'].append((seg * 16 + off, seg))
        for s in self.exe.sections:
            n = 's%03d' % s.index
            for off, seg in s.relocs:
                self.images[n].segs.add(seg)
                self.reloc_frames[n].append((seg * 16 + off, seg))
        for img in self.images.values():
            img.segs_sorted = sorted(s * 16 for s in img.segs)


class Analyzer:
    def __init__(self, prog):
        self.p = prog
        self.insns = {}          # (img, lin) -> (size, mnemonic, op_str, bytes)
        self.func_entries = {}   # (img, lin) -> set of reasons
        self.calls = collections.defaultdict(set)     # (img,lin of insn) -> target (img, lin)
        self.call_edges = set()  # (caller_func, callee_func) filled later
        self.jump_tables = {}    # (img, table_lin) -> dict
        self.data_bytes = collections.defaultdict(set)  # img -> set of linear addresses known as data in code
        self.unresolved = []
        self.noreturn = set()
        self.block_ends = set()
        self.far_xrefs = collections.defaultdict(set)  # target (img,lin) -> set of (img, insn lin)
        self.near_xrefs = collections.defaultdict(set)
        self.jump_targets = set()
        self.problems = []
        self.entry_frame = {}
        self.frame_conflicts = set()

    def add_entry(self, img, lin, why, frame=None):
        self.func_entries.setdefault((img, lin), set()).add(why)
        if frame is not None:
            old = self.entry_frame.get((img, lin))
            if old is not None and old != frame:
                self.frame_conflicts.add((img, lin, old, frame))
            else:
                self.entry_frame[(img, lin)] = frame

    # ------------------------------------------------------------------ decode one
    def frame_for(self, img, lin):
        """Frame (CS) for an address with no explicit frame: segment range lookup."""
        I = self.p.images[img]
        rng = getattr(I, 'seg_ranges', None)
        if rng:
            import bisect
            i = bisect.bisect_right(rng[0], lin) - 1
            if i >= 0:
                return rng[1][i]
        return I.seg_of(lin, I.segs_sorted)

    def decode(self, img, lin, frame):
        key = (img, lin)
        if key in self.insns:
            return self.insns[key]
        I = self.p.images[img]
        if not I.contains(lin):
            return None
        off = lin - frame * 16
        if not (0 <= off <= 0xffff):
            return None
        code = I.bytes_at(lin, 16)
        try:
            ins = next(md.disasm(code, off, 1))
        except StopIteration:
            return None
        rec = (ins.size, ins.mnemonic, ins.op_str, bytes(ins.bytes), frame, off)
        self.insns[key] = rec
        return rec

    # ------------------------------------------------------------------ recursive descent
    def explore(self, img, entry, frame=None):
        if frame is None:
            frame = self.entry_frame.get((img, entry)) or self.frame_for(img, entry)
        work = [(entry, frame)]
        I = self.p.images[img]
        while work:
            lin, frame = work.pop()
            while True:
                if (img, lin) in self.insns:
                    if self.insns[(img, lin)][4] != frame:
                        self.frame_conflicts.add((img, lin, self.insns[(img, lin)][4], frame))
                    break
                if not I.contains(lin) or lin in self.data_bytes[img]:
                    self.problems.append(('flow_into_data_or_outside', img, lin))
                    break
                rec = self.decode(img, lin, frame)
                if rec is None:
                    self.problems.append(('bad_decode', img, lin))
                    break
                size, mn, ops, raw, seg, off = rec
                # overlap check: instruction bytes must not overlap other insns' starts
                nxt = lin + size
                op = raw[0]
                # skip prefixes for opcode classification
                k = 0
                while raw[k] in (0x26, 0x2e, 0x36, 0x3e, 0xf2, 0xf3, 0xf0) and k < len(raw) - 1:
                    k += 1
                op = raw[k]
                if op == 0xe8:                                   # call near
                    rel = struct.unpack_from('<h', raw, k + 1)[0]
                    toff = (off + size + rel) & 0xffff
                    tlin = seg * 16 + toff
                    self.near_xrefs[(img, tlin)].add((img, lin))
                    self.calls[(img, lin)].add((img, tlin))
                    self.add_entry(img, tlin, 'near_call', seg)
                    work.append((tlin, seg))
                    if (img, tlin) in self.noreturn:
                        break
                    lin = nxt
                    continue
                if op == 0x9a:                                   # call far
                    toff, tseg = struct.unpack_from('<HH', raw, k + 1)
                    tgt = self.far_target(img, lin, tseg, toff)
                    if tgt:
                        self.calls[(img, lin)].add(tgt[:2])
                        self.far_xrefs[tgt[:2]].add((img, lin))
                        self.add_entry(tgt[0], tgt[1], 'far_call', tgt[2])
                        if tgt[:2] in self.noreturn:
                            break
                    lin = nxt
                    continue
                if op == 0xea:                                   # jmp far
                    toff, tseg = struct.unpack_from('<HH', raw, k + 1)
                    tgt = self.far_target(img, lin, tseg, toff)
                    if tgt:
                        self.calls[(img, lin)].add(tgt[:2])
                        self.far_xrefs[tgt[:2]].add((img, lin))
                        self.add_entry(tgt[0], tgt[1], 'far_jmp', tgt[2])
                    break
                if op in (0xe9, 0xeb):                           # jmp near/short
                    rel = struct.unpack_from('<h' if op == 0xe9 else '<b', raw, k + 1)[0]
                    toff = (off + size + rel) & 0xffff
                    tlin = seg * 16 + toff
                    self.jump_targets.add((img, tlin))
                    work.append((tlin, seg))
                    break
                if op in COND_JUMPS or (op == 0x0f and 0x80 <= raw[k + 1] <= 0x8f):
                    if op == 0x0f:
                        rel = struct.unpack_from('<h', raw, k + 2)[0]
                    else:
                        rel = struct.unpack_from('<b', raw, k + 1)[0]
                    toff = (off + size + rel) & 0xffff
                    tlin = seg * 16 + toff
                    self.jump_targets.add((img, tlin))
                    work.append((tlin, seg))
                    lin = nxt
                    continue
                if op in (0xc3, 0xc2, 0xcb, 0xca, 0xcf):         # returns
                    break
                if op == 0xff:
                    modrm = raw[k + 1]
                    reg = (modrm >> 3) & 7
                    if reg in (4, 5):                            # jmp indirect near/far
                        if reg == 4:
                            self.jump_table(img, lin, rec, work)
                        break
                    lin = nxt
                    continue
                if op == 0xf4:                                   # hlt
                    break
                if mn in ('(bad)',):
                    self.problems.append(('bad_insn', img, lin))
                    break
                lin = nxt

    def far_target(self, img, at, tseg, toff):
        """Resolve a far call/jmp operand (link-time values) to (image, linear)."""
        lin = tseg * 16 + toff
        if tseg == MGR_SEG and toff in self.p.vectors:
            tname, tlin, vseg = self.p.vectors[toff]
            return (tname, tlin, vseg)
        t = self.p.resolve(img, lin)
        if t is None:
            self.unresolved.append((img, at, tseg, toff))
            return None
        if self.p.images[t].is_data:
            self.problems.append(('far_call_into_data', img, at, tseg, toff))
            return None
        return (t, lin, tseg)

    # ------------------------------------------------------------------ switch tables
    def jump_table(self, img, lin, rec, work):
        size, mn, ops, raw, seg, off = rec
        m = re.match(r'word ptr cs:\[(bx|si|di)( \+ (0x[0-9a-f]+|\d+))?\]$', ops)
        if not m:
            self.problems.append(('indirect_jmp', img, lin, mn + ' ' + ops))
            return
        base = int(m.group(3), 0) if m.group(3) else 0
        reg = m.group(1)
        # look back up to 8 instructions for "cmp <reg|ax>, N" bound
        n_entries = None
        back = self.prev_insns(img, lin, 10)
        scaled = False          # index doubled (shl x,1 / add x,x) between the bound check and the jump
        for b in back:
            bmn, bops = b[1], b[2]
            if (bmn == 'shl' and bops.endswith(', 1')) or (bmn == 'add' and re.match(r'(\w+), \1$', bops)):
                scaled = True
            mm = re.match(r'(\w+), (0x[0-9a-f]+|\d+)$', bops) if bmn == 'cmp' else None
            if mm:
                bound = int(mm.group(2), 0)
                n_entries = bound + 1 if scaled else bound // 2 + 1
                break
        I = self.p.images[img]
        tlin = seg * 16 + base
        entries = []
        limit = n_entries if n_entries else 256
        for i in range(limit):
            el = tlin + 2 * i
            if not I.contains(el + 1):
                break
            if n_entries is None and i > 0 and ((img, el) in self.insns or (img, el) in self.func_entries):
                break
            v = I.word(el)
            target = seg * 16 + v
            if not I.contains(target):
                break
            if n_entries is None and (v < 0 or abs(target - lin) > 0x4000):
                break
            entries.append(target)
        for i in range(len(entries) * 2):
            self.data_bytes[img].add(tlin + i)
        self.jump_tables[(img, tlin)] = {'at': lin, 'seg': seg, 'base': base, 'count': len(entries),
                                         'bounded': n_entries is not None,
                                         'targets': entries}
        for t in entries:
            self.jump_targets.add((img, t))
            work.append((t, seg))

    def prev_insns(self, img, lin, n):
        """Linear predecessors by scanning back for decoded insns that end at lin."""
        out = []
        cur = lin
        for _ in range(n):
            found = None
            for sz in range(1, 7):
                r = self.insns.get((img, cur - sz))
                if r and r[0] == sz:
                    found = (cur - sz, r)
                    break
            if not found:
                break
            cur = found[0]
            out.append(found[1])
        return out

    # ------------------------------------------------------------------ driver
    def run(self):
        p = self.p
        root = p.images['root']
        # entry point
        mz = p.exe.mz
        self.add_entry('root', mz['cs'] * 16 + mz['ip'], 'mz_entry', mz['cs'])
        # vector targets
        for o, (tname, tlin, tseg) in p.vectors.items():
            self.add_entry(tname, tlin, 'vector', tseg)
        # far code pointers stored in data / code (relocated seg word preceded by offset)
        self.scan_far_pointers()
        changed = True
        rounds = 0
        while changed:
            rounds += 1
            before = len(self.insns)
            for key in list(self.func_entries):
                if key not in self.insns:
                    self.explore(*key)
            self.prologue_scan()
            self.scan_far_imm_pairs()
            for key in list(self.func_entries):
                if key not in self.insns:
                    self.explore(*key)
            if not (len(self.insns) != before):
                self.gap_scan()
                for key in list(self.func_entries):
                    if key not in self.insns:
                        self.explore(*key)
            changed = len(self.insns) != before
            if rounds > 10:
                break
        self.find_noreturn()
        return self

    def scan_far_imm_pairs(self):
        """Far code pointers built in code: 'mov word ptr [m], off' + 'mov word ptr [m+2], SEG'."""
        p = self.p
        by_img = collections.defaultdict(list)
        for (img, lin), r in self.insns.items():
            if r[3][:1] == b'\xc7':
                by_img[img].append(lin)
        for img, lins in by_img.items():
            I = p.images[img]
            lins.sort()
            prev = None
            for lin in lins:
                r = self.insns[(img, lin)]
                raw = r[3]
                if (lin + r[0] - 2) in I.reloc_sites and prev is not None:
                    pr = self.insns[(img, prev)]
                    praw = pr[3]
                    if prev + pr[0] == lin and len(praw) == len(raw) and praw[1] == raw[1] and len(raw) >= 5:
                        # same addressing mode; displacement must differ by 2
                        dl = len(raw) - 3
                        if dl >= 1:
                            d0 = int.from_bytes(praw[2:2 + dl - 1 + 1 - 1 + 1][:dl - 0], 'little', signed=True) if False else None
                        disp_a = int.from_bytes(praw[2:len(praw) - 2], 'little', signed=True) if len(praw) > 4 else 0
                        disp_b = int.from_bytes(raw[2:len(raw) - 2], 'little', signed=True) if len(raw) > 4 else 0
                        if disp_b - disp_a == 2:
                            off = int.from_bytes(praw[-2:], 'little')
                            seg = int.from_bytes(raw[-2:], 'little')
                            if seg == MGR_SEG:
                                if off in p.vectors:
                                    tname, tlin, vseg = p.vectors[off]
                                    self.add_entry(tname, tlin, 'far_ptr_code_vector', vseg)
                                prev = lin
                                continue
                            t = p.resolve(img, seg * 16 + off)
                            if t and not p.images[t].is_data:
                                self.add_entry(t, seg * 16 + off, 'far_ptr_code', seg)
                prev = lin

    def gap_scan(self):
        """Undiscovered code between functions (dead code, or reached only through tables we
        cannot see): a gap that decodes cleanly into a function ending in ret/retf, with every
        relocation inside it at an operand position, becomes a function."""
        p = self.p
        for name, I in p.images.items():
            if I.is_data or name == 'root':      # root holds RTLink tables between its code
                continue
            st, fr = I.seg_ranges if hasattr(I, 'seg_ranges') else ([], [])
            cov = self.covered(name)
            for k in range(len(self.data_bytes[name])):
                break
            lin = I.lo
            hi = I.hi
            jt = self.data_bytes[name]
            while lin < hi:
                if lin in cov or lin in jt:
                    lin += 1
                    continue
                g0 = lin
                while lin < hi and lin not in cov and lin not in jt:
                    lin += 1
                g1 = lin
                q = g0
                # skip alignment padding
                while q < g1 and I.byte(q) in (0x90, 0x00) and (q - g0) < 2:
                    q += 1
                if g1 - q < 3:
                    continue
                self.try_gap_function(name, q, g1)

    def try_gap_function(self, name, start, end):
        I = self.p.images[name]
        # typical MSC function starts only
        if I.byte(start) not in (0x55, 0xc8, 0x56, 0x57, 0x6a, 0x68, 0x1e, 0xa1, 0xa0, 0xc7, 0xc6, 0x8b,
                                 0x2b, 0x33, 0x83, 0x80, 0x3d, 0x3c, 0x8e, 0x9a, 0xb8, 0xb9, 0xba, 0xbb,
                                 0xbe, 0xbf, 0xff, 0x26, 0xcb):
            return
        frame = self.frame_for(name, start)
        cur = start
        ok = False
        n = 0
        while cur < end and n < 400:
            code = I.bytes_at(cur, 16)
            try:
                ins = next(md.disasm(code, cur - frame * 16, 1))
            except StopIteration:
                return
            if ins.mnemonic in ('(bad)',) or cur + ins.size > end:
                return
            raw = bytes(ins.bytes)
            for rs in range(cur, cur + ins.size):
                if rs in I.reloc_sites:
                    if not ((raw[0] in (0x9a, 0xea) and rs == cur + 3) or (0xb8 <= raw[0] <= 0xbf and rs == cur + 1)
                            or (raw[0] in (0xc7, 0x68) and rs == cur + ins.size - 2)):
                        return
            if (cur - 1) in I.reloc_sites:
                return
            if ins.mnemonic in ('hlt', 'int3', 'in', 'out', 'les', 'lds') and n == 0:
                return
            n += 1
            cur += ins.size
            if raw[0] in (0xc3, 0xcb, 0xc2, 0xca, 0xcf) or raw[0] in (0xe9, 0xeb, 0xea):
                ok = True
                break
        if ok and n >= 2:
            self.add_entry(name, start, 'gap_scan', frame)

    def scan_far_pointers(self):
        p = self.p
        for name, img in p.images.items():
            for site in sorted(img.reloc_sites):
                if site - 2 < img.lo:
                    continue
                # skip relocs that are operands of far call/jmp instructions or mov seg immediates
                seg = img.word(site)
                off = img.word(site - 2)
                if site - 3 >= img.lo and img.byte(site - 3) in (0x9a, 0xea):
                    continue
                lin = seg * 16 + off
                if seg == MGR_SEG and off in p.vectors:
                    tname, tlin, vseg = p.vectors[off]
                    self.add_entry(tname, tlin, 'far_ptr_vector', vseg)
                    continue
                t = p.resolve(name, lin)
                if t and not p.images[t].is_data and img.is_data:
                    # only trust pointers stored in data that hit a plausible prologue
                    b = p.images[t].bytes_at(lin, 3)
                    if b[:3] == b'\x55\x8b\xec' or b[:1] in (b'\x55', b'\x1e', b'\x56', b'\x57', b'\xb8', b'\xc8'):
                        self.add_entry(t, lin, 'far_ptr_data', seg)

    def covered(self, img):
        cov = set()
        for (i, lin), r in self.insns.items():
            if i == img:
                cov.update(range(lin, lin + r[0]))
        return cov

    def prologue_scan(self):
        """Find functions only reachable through pointers: 'push bp; mov bp,sp' or 'enter' in gaps."""
        for name, img in self.p.images.items():
            if img.is_data:
                continue
            cov = self.covered(name)
            mem = img.mem
            n = len(mem)
            i = 0
            pat1 = b'\x55\x8b\xec'
            start = 0
            while True:
                j = mem.find(pat1, start)
                if j < 0:
                    break
                lin = img.lo + j
                start = j + 1
                if lin in cov or lin in self.data_bytes[name]:
                    continue
                # preceding byte should be a function end / padding (ret, retf, nop, 0, jmp)
                prev = mem[j - 1] if j else 0xcb
                if prev in (0xc3, 0xcb, 0x90, 0x00, 0xcf) or (j >= 3 and mem[j - 3] in (0xca, 0xc2)) \
                        or (j >= 2 and mem[j - 2] == 0xeb) or (j >= 3 and mem[j - 3] == 0xe9):
                    self.add_entry(name, lin, 'prologue_scan')
            # 'enter N,0' prologue
            start = 0
            while True:
                j = mem.find(b'\xc8', start)
                if j < 0:
                    break
                start = j + 1
                lin = img.lo + j
                if lin in cov or j + 4 > n or mem[j + 3] != 0:
                    continue
                prev = mem[j - 1] if j else 0xcb
                if prev in (0xc3, 0xcb, 0x90) or (j >= 3 and mem[j - 3] in (0xca, 0xc2)):
                    self.add_entry(name, lin, 'prologue_scan_enter')

    def find_noreturn(self):
        pass


def compute_seg_ranges(an):
    """Exact segment start per frame: lowest address decoded with that frame (code), else the
    first byte after the previous segment's code that spills into the frame's paragraph."""
    p = an.p
    lo = collections.defaultdict(lambda: None)
    hi_by_frame = collections.defaultdict(int)
    for (img, lin), r in an.insns.items():
        k = (img, r[4])
        if lo[k] is None or lin < lo[k]:
            lo[k] = lin
        hi_by_frame[k] = max(hi_by_frame[k], lin + r[0])
    for (img, tl), jt in an.jump_tables.items():
        k = (img, jt['seg'])
        hi_by_frame[k] = max(hi_by_frame[k], tl + 2 * jt['count'])
    for name, lst in p.reloc_frames.items():
        for lin, f in lst:
            k = (name, f)
            if lo[k] is None or lin < lo[k]:
                lo[k] = lin
            hi_by_frame[k] = max(hi_by_frame[k], lin + 2)
    report = []
    for name, I in p.images.items():
        frames = sorted(I.segs)
        starts = []
        prev_hi = I.lo
        for f in frames:
            st = lo[(name, f)]
            if st is None or st >= f * 16 + 16:
                # nothing decoded in the frame's first paragraph: segment starts right after the
                # previous segment's last known byte, or at the paragraph if that is earlier
                st = max(f * 16, prev_hi) if prev_hi < f * 16 + 16 else f * 16
            if not (f * 16 <= st < f * 16 + 16):
                report.append((name, hex(f), hex(st)))
            starts.append(st)
            prev_hi = max(prev_hi, hi_by_frame[(name, f)])
        I.seg_ranges = (starts, frames)
    return report


def build_functions(an):
    """Partition decoded instructions into functions (contiguous ownership from entries)."""
    p = an.p
    funcs = {}
    by_img = collections.defaultdict(list)
    for (img, lin) in an.func_entries:
        if (img, lin) in an.insns:
            by_img[img].append(lin)
    insn_by_img = collections.defaultdict(list)
    for (img, lin), r in an.insns.items():
        insn_by_img[img].append(lin)
    owner = {}
    for img, entries in by_img.items():
        entries_set = set(entries)
        for e in entries:
            # flood fill without crossing other entries
            stack = [e]
            seen = set()
            while stack:
                lin = stack.pop()
                while lin not in seen and (img, lin) in an.insns:
                    if lin != e and lin in entries_set:
                        break
                    seen.add(lin)
                    if (img, lin) not in owner:
                        owner[(img, lin)] = e
                    r = an.insns[(img, lin)]
                    size, mn, ops, raw = r[0], r[1], r[2], r[3]
                    k = 0
                    while raw[k] in (0x26, 0x2e, 0x36, 0x3e, 0xf2, 0xf3, 0xf0) and k < len(raw) - 1:
                        k += 1
                    op = raw[k]
                    seg, off = r[4], r[5]
                    if op in (0xe9, 0xeb):
                        rel = struct.unpack_from('<h' if op == 0xe9 else '<b', raw, k + 1)[0]
                        stack.append(seg * 16 + ((off + size + rel) & 0xffff))
                        break
                    if op in COND_JUMPS or (op == 0x0f and 0x80 <= raw[k + 1] <= 0x8f):
                        rel = struct.unpack_from('<h', raw, k + 2)[0] if op == 0x0f else struct.unpack_from('<b', raw, k + 1)[0]
                        stack.append(seg * 16 + ((off + size + rel) & 0xffff))
                    if op in (0xc3, 0xc2, 0xcb, 0xca, 0xcf, 0xea, 0xf4):
                        break
                    if op == 0xff and ((raw[k + 1] >> 3) & 7) in (4, 5):
                        jt = [v for kk, v in an.jump_tables.items() if v['at'] == lin and kk[0] == img]
                        for t in jt:
                            stack.extend(t['targets'])
                        break
                    if (op == 0xe8 or op == 0x9a) and (img, lin) in an.calls:
                        if any(t in an.noreturn for t in an.calls[(img, lin)]):
                            break
                    lin = lin + size
    return owner


def main():
    exe_path = sys.argv[1]
    out_dir = sys.argv[2]
    os.makedirs(out_dir, exist_ok=True)
    prog = Program(exe_path)
    prog.load_vectors()
    prog.collect_segments()
    an = Analyzer(prog).run()
    rep = compute_seg_ranges(an)
    print('pass1: insns', len(an.insns), 'frame conflicts', len(an.frame_conflicts), 'odd seg starts', rep[:10])
    an = Analyzer(prog).run()          # pass 2 with exact segment ranges for frameless entries
    rep = compute_seg_ranges(an)
    print('pass2: frame conflicts', len(an.frame_conflicts), sorted(an.frame_conflicts)[:10])
    print('odd seg starts', len(rep), rep[:20])
    owner = build_functions(an)
    print('images', len(prog.images), 'vectors', len(prog.vectors))
    print('insns', len(an.insns), 'entries', len(an.func_entries), 'jump tables', len(an.jump_tables))
    print('unresolved far refs', len(an.unresolved), an.unresolved[:5])
    pc = collections.Counter(x[0] for x in an.problems)
    print('problems', pc)
    import pickle
    with open(os.path.join(out_dir, 'analysis.pickle'), 'wb') as f:
        pickle.dump({'insns': an.insns, 'entries': {k: sorted(v) for k, v in an.func_entries.items()},
                     'calls': {k: sorted(v) for k, v in an.calls.items()},
                     'jump_tables': an.jump_tables, 'problems': an.problems,
                     'owner': owner,
                     'segs': {n: sorted(i.segs) for n, i in prog.images.items()},
                     'seg_ranges': {n: i.seg_ranges for n, i in prog.images.items()},
                     'frame_conflicts': sorted(an.frame_conflicts),
                     'vectors': prog.vectors,
                     'data_bytes': {k: sorted(v) for k, v in an.data_bytes.items()}}, f)


if __name__ == '__main__':
    main()
