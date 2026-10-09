#!/usr/bin/env python3
"""
Emit reassemblable MASM/JWasm source for every segment region of DARKLAND.EXE.

One .asm file per region (= one original segment / module contribution, identified by its
link-time frame). Every byte of every image is emitted, so the result can be rebuilt into a
byte-identical executable by tools/omflink.py.

  - decoded instructions become MASM instructions; branch/call targets are labels
  - every relocated word becomes a symbolic SEG reference (so the linker regenerates the
    relocation tables instead of copying them)
  - far calls/jumps become 'call far ptr <label>' / 'jmp far ptr <label>'
  - switch jump tables become 'dw <label>' lists
  - instructions whose MASM encoding differs from the original (found by the verify loop in
    tools/build.py) are emitted as 'db' bytes with the decoded instruction in a comment
  - DGROUP / data offsets stay numeric (layout-preserving baseline)

Label naming (globally unique, derivable from any reference):
  sub_<img>_<frame>_<off>   function entry
  loc_<img>_<frame>_<off>   any other referenced location
  FRM_<img>_<frame>         frame anchor (offset 0 of the frame) used for SEG references
"""
import sys, os, re, json, struct, pickle, bisect, collections
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from rtlink import RTLinkExe

COND = {0x70 + i for i in range(16)} | {0xe0, 0xe1, 0xe2, 0xe3}
PREFIXES = (0x26, 0x2e, 0x36, 0x3e, 0xf2, 0xf3, 0xf0)
STRING_OPS = {'movsb', 'movsw', 'stosb', 'stosw', 'lodsb', 'lodsw', 'scasb', 'scasw',
              'cmpsb', 'cmpsw', 'insb', 'insw', 'outsb', 'outsw'}
MNEMONIC_FIX = {'cdq': 'cwd', 'cwde': 'cbw', 'xlatb': 'xlat', 'int3': 'int 3',
                'pushal': 'pusha', 'popal': 'popa', 'pushfd': 'pushf', 'popfd': 'popf',
                'iretd': 'iret', 'lcall': 'call', 'ljmp': 'jmp'}


def hexm(v):
    s = '%Xh' % v
    return ('0' + s) if s[0] in 'ABCDEF' else s


def masm_operands(ops):
    """Convert capstone operand text to MASM syntax (numbers, direct memory, sizes)."""
    ops = re.sub(r'0x([0-9a-f]+)', lambda m: hexm(int(m.group(1), 16)), ops)
    # 'ptr [..]' without size (lds/les/far indirect) -> dword ptr
    ops = re.sub(r'(^|, )ptr ', r'\1dword ptr ', ops)
    # direct memory operand without segment: [1234h] -> ds:[1234h]
    ops = re.sub(r'(?<![:\w])\[([0-9A-F]+h|\d+)\]', r'ds:[\1]', ops)
    ops = ops.replace(' + ', '+').replace(' - ', '-')
    return ops


def load_names(path=None):
    """symbols/names.txt: '<generated label> <name> [; comment]' lines."""
    path = path or os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), 'symbols', 'names.txt')
    names = {}
    if os.path.exists(path):
        for line in open(path):
            line = line.split(';')[0].strip()
            if not line or line.startswith('#'):
                continue
            a, b = line.split()[:2]
            names[a] = b
    if len(set(names.values())) != len(names):
        raise SystemExit('duplicate names in symbols/names.txt')
    return names


class Emitter:
    def __init__(self, exe_path, pickle_path, force_db=(), holes=()):
        # holes: (image, lin_start, lin_end, {lin: public name}) ranges supplied by compiled C
        self.holes = list(holes)
        self.overrides = {}
        for img, a, b, names in self.holes:
            for lin, nm in names.items():
                self.overrides[(img, lin)] = nm
        self.exe = RTLinkExe(exe_path)
        d = pickle.load(open(pickle_path, 'rb'))
        self.insns = d['insns']
        self.entries = d['entries']
        self.calls = d['calls']
        self.jump_tables = d['jump_tables']
        self.seg_ranges = d['seg_ranges']
        self.vectors = d['vectors']
        self.force_db = set(tuple(x) for x in force_db)
        self.images = {}
        e = self.exe
        self.images['root'] = dict(base=0, mem=e.root_memory(), relocs=set(e.reloc_sites()),
                                   parent=None)
        for s in e.sections:
            n = 's%03d' % s.index
            self.images[n] = dict(base=s.load_seg, mem=e.section_memory(s.index),
                                  relocs=set(e.reloc_sites(s.index)),
                                  parent=None if s.parent == 0xffff else 's%03d' % s.parent)
        self.data_img = 's136'
        self.dgroup_init_end = len(self.exe.sections[136].image)
        # jump table bytes per image
        self.jt_at = {}
        for (img, tl), jt in self.jump_tables.items():
            self.jt_at[(img, tl)] = jt
        # labels needed: (img, lin)
        self.emitted = []
        self.far_ptr_sym = 0
        sp = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), 'symbols', 'split.txt')
        self.split = set()
        if os.path.exists(sp):
            for line in open(sp):
                line = line.split(';')[0].strip()
                if line and not line.startswith('#'):
                    self.split.add(line)
        self.names = load_names()
        self.labels = set()
        self.ext_refs = collections.defaultdict(set)   # region -> set of (kind, name)
        # vector stubs in the RTLink manager segment: label each stub, and resolve its far jmp
        self.vec_names = {}          # root lin -> 'vec_xxxx'
        self.vec_jmp = {}            # root lin of the 'jmp far' -> (target image, target lin)
        for o, (tname, tlin, tseg) in self.vectors.items():
            self.vec_names[0x9c00 + o] = 'vec_%04x' % o
            self.vec_jmp[0x9c00 + o + 3] = (tname, tlin, tseg)
        self.collect_labels()

    # ------------------------------------------------------------------ helpers
    def region_of(self, img, lin):
        st, fr = self.seg_ranges[img]
        i = bisect.bisect_right(st, lin) - 1
        if i < 0:
            i = 0
        return fr[i], st[i], (st[i + 1] if i + 1 < len(st) else None)

    def region_name(self, img, frame):
        return '%s_%04x' % (img, frame)

    def label_name(self, img, lin):
        if (img, lin) in self.overrides:
            return self.overrides[(img, lin)]
        f, _, _ = self.region_of(img, lin)
        kind = 'sub' if (img, lin) in self.entries else 'loc'
        gen = '%s_%s_%04x_%04x' % (kind, img, f, lin - f * 16)
        return self.names.get(gen, gen)

    def frame_symbol(self, img, frame):
        return 'FRM_%s_%04x' % (img, frame)

    def visible(self, img):
        out = [img]
        n = self.images[img]['parent']
        while n:
            out.append(n)
            n = self.images[n]['parent']
        for extra in ('root', self.data_img):
            if extra not in out:
                out.append(extra)
        return out

    def resolve_frame(self, img, frame_value):
        """Which image does a relocated segment value refer to, seen from img?"""
        lin = frame_value * 16
        for n in self.visible(img):
            I = self.images[n]
            lo = I['base'] * 16
            hi = lo + len(I['mem'])
            if lo <= lin < hi or (lin == lo):
                st, fr = self.seg_ranges[n]
                if frame_value in fr:
                    return n
        return None

    def collect_labels(self):
        for (img, lin), tg in self.calls.items():
            for t in tg:
                self.labels.add(t)
        for (img, lin) in self.entries:
            if (img, lin) in self.insns:
                self.labels.add((img, lin))
        for (img, tl), jt in self.jump_tables.items():
            self.labels.add((img, tl))
            self.labels.add((img, jt['at']))
            for t in jt['targets']:
                self.labels.add((img, t))
        for (img, lin), r in self.insns.items():
            raw = r[3]
            k = 0
            while raw[k] in PREFIXES and k < len(raw) - 1:
                k += 1
            op = raw[k]
            tgt = self.branch_target(img, lin, r)
            if tgt is not None:
                self.labels.add((img, tgt))
        for o, (tname, tlin, tseg) in self.vectors.items():
            self.labels.add((tname, tlin))
        self.find_far_offsets()

    def find_far_offsets(self):
        """Offset halves of far code pointers built in code. MSC writes a far function pointer as
        two instructions, one with a relocated segment and one with a plain 16-bit offset:
            mov word ptr [m], off   /  mov word ptr [m+2], SEG x       (store)
            push SEG x              /  push off                        (argument)
            mov ax, off             /  mov dx, SEG x                   (return value)
        The offset is emitted as 'OFFSET label' so it follows the function if code moves
        (tools/build.py --mod). Only offsets that land on a function entry of that frame count."""
        self.far_off = {}
        by_img = collections.defaultdict(list)
        for (img, lin) in self.insns:
            by_img[img].append(lin)
        for img, lins in by_img.items():
            I = self.images[img]
            if img == self.data_img:
                continue
            lins.sort()
            for k, lin in enumerate(lins):
                size, mn, ops, raw, frame, off = self.insns[(img, lin)]
                if not raw or raw[0] in PREFIXES:
                    continue
                op = raw[0]
                cand = None
                if op == 0xc7 and size >= 5 and (lin + size - 2) in I['relocs'] and k > 0:
                    pl = lins[k - 1]
                    pr = self.insns[(img, pl)]
                    praw = pr[3]
                    if pl + pr[0] == lin and praw[:2] == raw[:2] and len(praw) == len(raw) and \
                            not any(q in I['relocs'] for q in range(pl, lin)):
                        da = int.from_bytes(praw[2:len(praw) - 2], 'little', signed=True) if len(praw) > 4 else 0
                        db = int.from_bytes(raw[2:len(raw) - 2], 'little', signed=True) if len(raw) > 4 else 0
                        if db - da == 2:
                            cand = (pl, len(praw) - 2)
                elif op == 0x68 and size == 3 and (lin + 1) in I['relocs'] and k + 1 < len(lins):
                    nl = lins[k + 1]
                    nr = self.insns[(img, nl)]
                    if lin + size == nl and nr[3][:1] == b'\x68' and nr[0] == 3 and (nl + 1) not in I['relocs']:
                        cand = (nl, 1)
                elif op == 0xba and size == 3 and (lin + 1) in I['relocs']:
                    for j in (k - 1, k + 1):
                        if 0 <= j < len(lins):
                            ol = lins[j]
                            orr = self.insns[(img, ol)]
                            if orr[3][:1] == b'\xb8' and orr[0] == 3 and (ol + 1) not in I['relocs'] and \
                                    (ol + orr[0] == lin or lin + size == ol):
                                cand = (ol, 1)
                if cand is None:
                    continue
                v = struct.unpack_from('<H', raw, size - 2 if op == 0xc7 else 1)[0]
                timg = self.resolve_frame(img, v)
                if timg is None or timg == self.data_img:
                    continue
                ol, at = cand
                o = struct.unpack_from('<H', self.insns[(img, ol)][3], at)[0]
                tlin = v * 16 + o
                if (timg, tlin) not in self.entries or (timg, tlin) not in self.insns:
                    continue
                f2, _, _ = self.region_of(timg, tlin)
                if f2 != v:
                    continue
                self.far_off[(img, ol)] = (timg, tlin)

    def branch_target(self, img, lin, r):
        size, mn, ops, raw, frame, off = r
        k = 0
        while raw[k] in PREFIXES and k < len(raw) - 1:
            k += 1
        op = raw[k]
        if op in (0xe8, 0xe9):
            rel = struct.unpack_from('<h', raw, k + 1)[0]
        elif op == 0xeb or op in COND:
            rel = struct.unpack_from('<b', raw, k + 1)[0]
        elif op == 0x0f and len(raw) > k + 3 and 0x80 <= raw[k + 1] <= 0x8f:
            rel = struct.unpack_from('<h', raw, k + 2)[0]
        else:
            return None
        return frame * 16 + ((off + size + rel) & 0xffff)

    # ------------------------------------------------------------------ instruction text
    def insn_text(self, img, lin, r, region, ext):
        """Return MASM text for the instruction or None if it must be emitted as bytes."""
        n0 = len(ext)
        self._sym_hint = False
        t = self._insn_text(img, lin, r, region, ext)
        self._last_symbolic = bool(t) and (len(ext) != n0 or self._sym_hint or
                                           bool(re.search(r'\b(sub|loc|vec|FRM)_', t)) or ' ptr _' in t or 'SEG ' in t)
        return t

    def _insn_text(self, img, lin, r, region, ext):
        size, mn, ops, raw, frame, off = r
        I = self.images[img]
        if (img, lin) in self.force_db:
            return None
        reloc_in = [p for p in range(lin, lin + size) if p in I['relocs'] or (p - 1) in I['relocs']]
        k = 0
        while raw[k] in PREFIXES and k < len(raw) - 1:
            k += 1
        op = raw[k]
        rf, rs, re_ = region
        # ---- far call / jmp with relocated segment
        if op in (0x9a, 0xea) and k == 0 and size == 5:
            if (lin + 3) not in I['relocs']:
                return None
            toff, tseg = struct.unpack_from('<HH', raw, 1)
            tgt = None
            if img == 'root' and lin in self.vec_jmp:
                timg, tlin, vseg = self.vec_jmp[lin]
                f2, _, _ = self.region_of(timg, tlin)
                if f2 != tseg or tlin != tseg * 16 + toff:
                    return None
                name = self.label_name(timg, tlin)
                ext.add(('FAR', name))
                self._sym_hint = True
                return 'jmp far ptr %s' % name
            if tseg == 0x9c0 and toff in self.vectors and img != 'root':
                # call through RTLink vector: target label is the vector stub in root
                name = 'vec_%04x' % toff
                ext.add(('FAR', name))
            else:
                timg = self.resolve_frame(img, tseg)
                if timg is None:
                    return None
                tlin = tseg * 16 + toff
                f2, _, _ = self.region_of(timg, tlin)
                if f2 != tseg:
                    return None
                if timg == 'root' and tseg == 0x9c0 and toff in self.vectors:
                    name = 'vec_%04x' % toff
                else:
                    name = self.label_name(timg, tlin)
                    self.labels.add((timg, tlin))
                if (timg, f2) != (img, rf) or name.startswith('vec_'):
                    ext.add(('FAR', name))
            self._sym_hint = True
            return '%s far ptr %s' % ('call' if op == 0x9a else 'jmp', name)
        if reloc_in:
            # mov r16, SEG x
            if 0xb8 <= op <= 0xbf and k == 0 and size == 3 and (lin + 1) in I['relocs']:
                v = struct.unpack_from('<H', raw, 1)[0]
                fs = self.seg_symbol(img, v, ext)
                if fs is None:
                    return None
                reg = ['ax', 'cx', 'dx', 'bx', 'sp', 'bp', 'si', 'di'][op - 0xb8]
                return 'mov %s, SEG %s' % (reg, fs)
            if op == 0x68 and k == 0 and size == 3 and (lin + 1) in I['relocs']:
                v = struct.unpack_from('<H', raw, 1)[0]
                fs = self.seg_symbol(img, v, ext)
                return None if fs is None else 'push SEG %s' % fs
            if op == 0xc7 and (lin + size - 2) in I['relocs'] and len(reloc_in) <= 2:
                v = struct.unpack_from('<H', raw, size - 2)[0]
                fs = self.seg_symbol(img, v, ext)
                if fs is None:
                    return None
                m = re.match(r'(.*), (0x[0-9a-f]+|\d+)$', ops)
                if not m:
                    return None
                return 'mov %s, SEG %s' % (masm_operands(m.group(1)), fs)
            return None
        # ---- offset half of a far code pointer (see find_far_offsets)
        if (img, lin) in self.far_off and k == 0:
            timg, tlin = self.far_off[(img, lin)]
            name = self.label_name(timg, tlin)
            if not name.startswith('vec_'):
                f2, _, _ = self.region_of(timg, tlin)
                if (timg, f2) != (img, rf):
                    ext.add(('FAR', name))
                self._sym_hint = True
                if op == 0x68:
                    return 'push OFFSET %s' % name
                if op == 0xb8:
                    return 'mov ax, OFFSET %s' % name
                m = re.match(r'(.*), (0x[0-9a-f]+|\d+)$', ops)
                if op == 0xc7 and m:
                    return 'mov %s, OFFSET %s' % (masm_operands(m.group(1)), name)
        # ---- branches
        tgt = self.branch_target(img, lin, r)
        if tgt is not None:
            same = rs <= tgt and (re_ is None or tgt < re_)
            name = self.label_name(img, tgt)
            self._sym_hint = True
            if op == 0xe8 and k == 0:
                if not same:
                    ext.add(('NEAR', name))
                return 'call near ptr %s' % name
            if op == 0xe9 and k == 0:
                if not same:
                    ext.add(('NEAR', name))
                return 'jmp near ptr %s' % name
            if not same or k != 0:
                return None
            if op == 0xeb:
                return 'jmp short %s' % name
            if op == 0x0f:
                return None
            return '%s %s' % (mn, name)
        # ---- switch jump through table
        if op == 0xff and ((raw[k + 1] >> 3) & 7) == 4:
            jts = [t for t in self.jump_tables.values() if t['at'] == lin]
            mm = re.match(r'word ptr cs:\[(bx|si|di) \+ (0x[0-9a-f]+)\]$', ops)
            if jts and mm and jts[0]['seg'] == rf:
                tl = rf * 16 + int(mm.group(2), 16)
                if rs <= tl and (re_ is None or tl < re_):
                    self._sym_hint = True
                    return 'jmp word ptr cs:%s[%s]' % (self.label_name(img, tl), mm.group(1))
        # ---- generic
        m = MNEMONIC_FIX.get(mn, mn)
        if mn in ('lcall', 'ljmp'):
            if ':' in ops and '[' not in ops:
                return None
            o = masm_operands(ops)
            if not o.startswith('dword ptr'):
                o = 'dword ptr ' + o
            return '%s %s' % (m, o)
        parts = mn.split()
        base = parts[-1]
        if base in STRING_OPS:
            # plain string op unless an explicit segment override on the source
            if re.search(r'\b(cs|ss|es|ds):\[si\]', ops) and not ops.count('es:[si]'):
                gen = base[:-1]
                return ' '.join(parts[:-1] + [gen, masm_operands(ops)])
            if k != 0 and raw[0] in (0x26, 0x2e, 0x36, 0x3e):
                return None
            return mn
        if mn == 'xlatb':
            if k:
                seg = {0x26: 'es', 0x2e: 'cs', 0x36: 'ss', 0x3e: 'ds'}.get(raw[0])
                return 'xlat byte ptr %s:[bx]' % seg if seg else None
            return 'xlat'
        o = masm_operands(ops)
        if mn in ('rol', 'ror', 'rcl', 'rcr', 'shl', 'sal', 'shr', 'sar') and ',' not in o and op in (0xd0, 0xd1):
            o += ', 1'
        return (m + ' ' + o).strip()

    def string_note(self, r):
        """Comment text for 'push imm16' / 'mov reg, imm16' whose value starts a DGROUP C string."""
        raw = r[3]
        if not ((raw[:1] == b'\x68' and len(raw) == 3) or (0xb8 <= raw[0] <= 0xbf and len(raw) == 3)):
            return ''
        v = struct.unpack_from('<H', raw, 1)[0]
        d = self.images[self.data_img]
        lin = 0x321a0 + v - d['base'] * 16
        mem = d['mem']
        if lin <= 0 or lin >= self.dgroup_init_end or mem[lin - 1] != 0:
            return ''
        end = mem.find(b'\0', lin, lin + 120)
        if end < 0 or end - lin < 3:
            return ''
        t = bytes(mem[lin:end])
        if sum(32 <= c < 127 for c in t) < len(t):
            return ''
        return '  "%s"' % t.decode('latin-1').replace('"', "'")

    def far_pointer_label(self, img, seg, off):
        """Label for a far pointer seg:off if it points at a known label whose frame is seg."""
        if seg == 0x9c0 and off in self.vectors:
            return 'vec_%04x' % off
        timg = self.resolve_frame(img, seg)
        if timg is None or timg == self.data_img:
            return None
        tlin = seg * 16 + off
        if (timg, tlin) not in self.labels:
            return None
        f, _, _ = self.region_of(timg, tlin)
        if f != seg:
            return None
        return self.label_name(timg, tlin)

    def is_local(self, img, name, frame, start, end):
        return name in self._local_names

    def seg_symbol(self, img, v, ext):
        timg = self.resolve_frame(img, v)
        if timg is None:
            name = 'FRM_abs_%04x' % v
        else:
            name = self.frame_symbol(timg, v)
        ext.add(('BYTE', name))
        return name

    # ------------------------------------------------------------------ region emission
    def emit_region(self, img, frame, start, end, rname=None):
        I = self.images[img]
        mem = I['mem']
        base = I['base'] * 16
        rname = rname or self.region_name(img, frame)
        ext = set()
        body = []
        pub = set()
        p = start
        hi = end
        # label positions in this region
        lab = sorted(l for (i, l) in self.labels if i == img and start <= l < hi)
        labset = set(lab)
        anylab = set(labset)
        if img == 'root':
            anylab |= {l for l in self.vec_names if start <= l < hi}
        self._local_names = {self.label_name(img, l) for l in labset}
        if img == 'root':
            self._local_names |= {self.vec_names[l] for l in self.vec_names if start <= l < hi}
        jt_bytes = {}
        for (i, tl), jt in self.jump_tables.items():
            if i == img and start <= tl < hi:
                for kk in range(jt['count']):
                    jt_bytes[tl + 2 * kk] = (jt, kk)
        pending = []        # data bytes buffer

        def flush():
            if not pending:
                return
            # pending: list of (lin, byte)
            line = []
            for lin, b in pending:
                line.append(hexm(b))
                if len(line) == 16:
                    body.append('\tdb ' + ', '.join(line))
                    line = []
            if line:
                body.append('\tdb ' + ', '.join(line))
            pending.clear()

        def put_label(lin):
            if lin in labset:
                nm = self.label_name(img, lin)
                body.append('%s:' % nm)
                pub.add(nm)
            if img == 'root' and lin in self.vec_names:
                body.append('%s:' % self.vec_names[lin])
                pub.add(self.vec_names[lin])

        while p < hi:
            if p in anylab:
                flush()
                put_label(p)
            r = self.insns.get((img, p))
            if r is not None and p + r[0] <= hi and not any(q in anylab for q in range(p + 1, p + r[0])) \
                    and not any(q in jt_bytes for q in range(p, p + r[0])):
                txt = self.insn_text(img, p, r, (frame, start, end), ext)
                if txt is not None:
                    flush()
                    note = self.string_note(r)
                    body.append('\t%-40s; @%05x %s%s' % (txt, p, r[3].hex(), note))
                    self.emitted.append((img, p, txt, r[3], self._last_symbolic))
                    p += r[0]
                    continue
                # emit as bytes, with relocs as SEG words
                flush()
                body.append('\t; @%05x %-14s %s %s' % (p, r[3].hex(), r[1], r[2]))
                q = p
                end_q = p + r[0]
                while q < end_q:
                    if q in I['relocs']:
                        flush()
                        v = struct.unpack_from('<H', mem, q - base)[0]
                        fs = self.seg_symbol(img, v, ext)
                        body.append('\tdw SEG %s\t; @%05x reloc' % (fs, q))
                        q += 2
                        end_q = max(end_q, q)
                    else:
                        pending.append((q, mem[q - base]))
                        q += 1
                flush()
                p = q
                continue
            if p in jt_bytes and p + 1 < hi and (p + 1) not in anylab:
                jt, kk = jt_bytes[p]
                flush()
                v = struct.unpack_from('<H', mem, p - base)[0]
                tgt = jt['seg'] * 16 + v
                if jt['seg'] == frame and start <= tgt < hi:
                    body.append('\tdw %s\t; @%05x case %d' % (self.label_name(img, tgt), p, kk))
                else:
                    body.append('\tdw %s\t; @%05x case %d (numeric)' % (hexm(v), p, kk))
                p += 2
                continue
            if img == 'root' and (p - 1) in self.vec_jmp and (p + 2) in I['relocs'] and p + 4 <= hi:
                # RTLink vector stub 'call handler / jmp far target / dw section': the jump target
                # is a far pointer into an overlay; keep it symbolic so it follows the code
                timg, tlin, tseg = self.vec_jmp[p - 1]
                off, v = struct.unpack_from('<HH', mem, p - base)
                f2, _, _ = self.region_of(timg, tlin)
                if f2 == v and tlin == v * 16 + off:
                    nm = self.label_name(timg, tlin)
                    flush()
                    ext.add(('FAR', nm))
                    body.append('\tdd %s\t; @%05x vector target' % (nm, p))
                    self.far_ptr_sym += 1
                    p += 4
                    continue
            if (p + 2) in I['relocs'] and p not in I['relocs'] and (p + 1) not in I['relocs'] \
                    and p + 4 <= hi and not any(q in anylab for q in (p + 1, p + 2, p + 3)) \
                    and (img, p) not in self.insns:
                # far pointer stored as data: offset word + relocated segment word
                off, v = struct.unpack_from('<HH', mem, p - base)
                nm = self.far_pointer_label(img, v, off)
                if nm:
                    flush()
                    if not self.is_local(img, nm, frame, start, end):
                        ext.add(('FAR', nm))
                    body.append('\tdd %s\t; @%05x far ptr' % (nm, p))
                    self.far_ptr_sym += 1
                    p += 4
                    continue
            if p in I['relocs']:
                flush()
                v = struct.unpack_from('<H', mem, p - base)[0]
                fs = self.seg_symbol(img, v, ext)
                # a far pointer whose offset word precedes: keep offset numeric
                body.append('\tdw SEG %s\t; @%05x reloc' % (fs, p))
                p += 2
                continue
            pending.append((p, mem[p - base]))
            p += 1
            if len(pending) >= 16:
                flush()
        flush()
        return {'img': img, 'frame': frame, 'start': start, 'end': end, 'rname': rname,
                'ext': ext, 'pub': pub, 'body': body}

    def render(self, R, exported):
        img, frame, start, end, rname = R['img'], R['frame'], R['start'], R['end'], R['rname']
        seg_name = 'C_%s' % rname
        out = []
        out.append('; %s  frame %04x  linear %05x-%05x  (%d bytes)' % (rname, frame, start, end, end - start))
        out.append('; generated by tools/emit_asm.py - regenerate rather than hand-edit until ported')
        out.append('.286')
        out.append('OPTION NOLJMP')
        fsym = self.frame_symbol(img, frame)
        owns_frame = rname == self.region_name(img, frame)   # split-off parts reference the anchor
        defined = R['pub'] | ({fsym} if owns_frame else set())
        for kind, nm in sorted(R['ext']):
            if nm in defined:
                continue
            out.append('EXTERN %s:%s' % (nm, kind))
        out.append('%s SEGMENT BYTE PUBLIC \'CODE\'' % seg_name)
        out.append('ASSUME CS:%s, DS:NOTHING, ES:NOTHING, SS:NOTHING' % seg_name)
        if owns_frame:
            out.append('PUBLIC %s' % fsym)
        for nm in sorted(R['pub']):
            if nm in exported or nm.startswith('sub_') or nm.startswith('vec_') or not nm.startswith('loc_'):
                out.append('PUBLIC %s' % nm)
        if owns_frame:
            out.append('%s LABEL BYTE' % fsym)
        if start - frame * 16:
            out.append('ORG %s' % hexm(start - frame * 16))
        out.extend(R['body'])
        out.append('%s ENDS' % seg_name)
        out.append('END')
        return '\n'.join(out) + '\n'

    def regions(self):
        for img, I in self.images.items():
            st, fr = self.seg_ranges[img]
            lo = I['base'] * 16
            hi = lo + len(I['mem'])
            if hi == lo:
                continue
            # trim BSS of the data section: only file-backed bytes are emitted
            if img == 's136':
                hi = lo + len(self.exe.sections[136].image)
            for i, (s, f) in enumerate(zip(st, fr)):
                e = st[i + 1] if i + 1 < len(st) else hi
                s2 = max(s, lo)
                e = min(e, hi)
                if i == 0 and s2 > lo:
                    s2 = lo
                if e <= s2:
                    continue
                # functions listed in symbols/split.txt become modules of their own (the unit
                # that a compiled C replacement takes over); the rest of the region is split around them
                cuts = []
                ents = sorted(l for (i, l) in self.entries if i == img and s2 <= l < e and (i, l) in self.insns)
                for k, l in enumerate(ents):
                    if self.label_name(img, l) in self.split:
                        fe = ents[k + 1] if k + 1 < len(ents) else e
                        cuts.append((l, fe, self.label_name(img, l)))
                for himg, ha, hb, _ in self.holes:
                    if himg == img and s2 <= ha < e:
                        if hb > e:
                            raise SystemExit('C replacement %s:%05x-%05x crosses a segment boundary' % (img, ha, hb))
                        cuts = [c for c in cuts if not (ha <= c[0] < hb)]
                        cuts.append((ha, hb, None))
                cuts.sort()
                pos = s2
                base_name = self.region_name(img, f)
                for fs, fe, nm in cuts:
                    if fs > pos:
                        yield img, f, pos, fs, base_name if pos == s2 else '%s_%05x' % (base_name, pos)
                    if nm is not None:
                        yield img, f, fs, fe, '%s__%s' % (base_name, nm.lstrip('_'))
                    pos = fe
                if pos < e:
                    yield img, f, pos, e, base_name if pos == s2 else '%s_%05x' % (base_name, pos)

    def emit_all(self, out_dir, only=None):
        files = []
        layout = {'regions': []}
        regs = []
        for img, f, st, e, rn in self.regions():
            regs.append(self.emit_region(img, f, st, e, rn))
        exported = set()
        for R in regs:
            exported |= {nm for kind, nm in R['ext']}
        for R in regs:
            img, f = R['img'], R['frame']
            d = os.path.join(out_dir, img)
            os.makedirs(d, exist_ok=True)
            fn = os.path.join(d, R['rname'] + '.asm')
            txt = self.render(R, exported)
            old = open(fn).read() if os.path.exists(fn) else None
            if old != txt:
                with open(fn, 'w') as fh:
                    fh.write(txt)
            files.append(fn)
            layout['regions'].append({'image': img, 'frame': f, 'start': R['start'], 'end': R['end'],
                                      'file': os.path.relpath(fn, out_dir),
                                      'segment': 'C_' + R['rname']})
        return files, layout


def main():
    exe, pk, out_dir = sys.argv[1:4]
    force = []
    if len(sys.argv) > 4 and os.path.exists(sys.argv[4]):
        force = json.load(open(sys.argv[4]))
    em = Emitter(exe, pk, force)
    files, layout = em.emit_all(out_dir)
    with open(os.path.join(out_dir, 'regions.json'), 'w') as f:
        json.dump(layout, f, indent=0)
    print('emitted', len(files), 'region files')


if __name__ == '__main__':
    main()
