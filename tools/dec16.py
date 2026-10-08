#!/usr/bin/env python3
"""
dec16 - decompiler for DARKLAND.EXE's Microsoft C 6.00 (/AM /G2 /Gs /Ox) code.

Turns the original machine code of one source file (build/analysis/srcfiles.json) into C
meant to recompile to the same bytes. It is not a general decompiler: it knows the code
generator's idioms (inline strcpy/memset, far-data CONST slots, struct returns in DX:AX,
jump tables, compare chains, ...) and emits plain C with gotos where structure is unclear.
Whether the output matches is decided by compiling it (tools/fmatch.py), never assumed.

  python3 -I tools/dec16.py s070 19af [-o out.c]       decompile the source file at s070 19af:0000
"""
import sys, os, re, json, pickle, struct, collections, bisect
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from rtlink import RTLinkExe
import capstone
from capstone import x86 as X

REPO = os.path.dirname(HERE)
EXE = os.environ.get('DARKLAND_EXE', '/home/claude/dl_raw/extracted/DARKLAND/DARKLAND.EXE')
DG = 0x321a


class DecompileError(Exception):
    pass


# =============================================================================== program context
class Prog:
    def __init__(self, exe_path=EXE):
        self.exe = RTLinkExe(exe_path)
        an = os.path.join(REPO, 'build', 'analysis')
        self.syms = json.load(open(os.path.join(REPO, 'build', 'symbols.json')))
        self.addr2name = {}
        for n, (img, lin, fr) in self.syms.items():
            if n.startswith('FRM_'):
                continue
            k = (img, lin)
            # prefer generated sub_ names for code so C names stay stable
            if k not in self.addr2name or n.startswith('sub_') or n.startswith('vec_'):
                self.addr2name[k] = n
        self.files = json.load(open(os.path.join(an, 'srcfiles.json')))
        self.funcs = json.load(open(os.path.join(an, 'functions.json')))
        d = pickle.load(open(os.path.join(an, 'analysis.pickle'), 'rb'))
        self.jump_tables = {}
        for (img, tlin), jt in d['jump_tables'].items():
            self.jump_tables[(img, jt['at'])] = dict(jt, table=tlin)
        self.vectors = {}
        for off, (timg, tlin, tfr) in d['vectors'].items():
            self.vectors['vec_%04x' % off] = (timg, tlin, tfr)
        self.vec_by_addr = {(0x9c00 + off): 'vec_%04x' % off for off in d['vectors']}
        self._mems = {}
        self._relocs = {}
        s = self.exe.sections[136]
        self.dmem = self.exe.section_memory(136)
        self.dlo = s.load_seg * 16
        self.dgroup_relocs = set(self.exe.reloc_sites(136))
        self.data_segs = set()     # far data frames (segments inside s136)
        for img, lin, fr in self.syms.values():
            if img == 's136':
                self.data_segs.add(fr)
        self._argc = None
        self._uses_result = None
        self._dref = None

    def mem(self, img):
        if img not in self._mems:
            if img == 'root':
                self._mems[img] = (self.exe.root_memory(), 0)
            else:
                i = int(img[1:])
                self._mems[img] = (self.exe.section_memory(i), self.exe.sections[i].load_seg * 16)
        return self._mems[img]

    def relocs(self, img):
        if img not in self._relocs:
            self._relocs[img] = set(self.exe.reloc_sites(None if img == 'root' else int(img[1:])))
        return self._relocs[img]

    def dword(self, off):
        p = DG * 16 + off - self.dlo
        return struct.unpack_from('<H', self.dmem, p)[0]

    def dbytes(self, off, n):
        p = DG * 16 + off - self.dlo
        return bytes(self.dmem[p:p + n])

    def cstring(self, off, maxlen=400):
        p = DG * 16 + off - self.dlo
        if p < 0 or p >= len(self.dmem):
            return None
        e = self.dmem.find(b'\0', p, p + maxlen)
        if e < 0:
            return None
        return bytes(self.dmem[p:e])

    def is_dgroup_seg_slot(self, off):
        return DG * 16 + off in self.dgroup_relocs

    def file_at(self, img, frame):
        for f in self.files:
            if f['image'] == img and int(f['frame'], 16) == frame:
                return f
        return None

    def file_functions(self, f):
        """[(name, linear start, linear end)] of a source file."""
        img = f['image']
        starts = sorted(self.syms[n][1] if n in self.syms else
                        int(n.split('_')[2], 16) * 16 + int(n.split('_')[3], 16) for n in f['functions'])
        allst = sorted(int(g['start'], 16) for g in self.files if g['image'] == img)
        k = allst.index(int(f['start'], 16))
        if k + 1 < len(allst):
            end = allst[k + 1]
        else:
            mem, lo = self.mem(img)
            end = lo + len(mem)
            while end > lo and mem[end - 1 - lo] == 0:
                end -= 1
        out = []
        for i, s in enumerate(starts):
            out.append((self.addr2name.get((img, s), 'sub_%s_%s_%04x' % (img, f['frame'], s - int(f['frame'], 16) * 16)),
                        s, starts[i + 1] if i + 1 < len(starts) else end))
        return out

    # --- whole-program facts: argument counts and result use per callee
    def _scan_calls(self):
        cache = os.path.join(REPO, 'build', 'analysis', 'dec16_scan.pickle')
        if os.path.exists(cache) and os.path.getmtime(cache) > os.path.getmtime(os.path.join(REPO, 'build', 'symbols.json')):
            self._argc, self._uses_result, self._dref, self._memdisp = pickle.load(open(cache, 'rb'))
            return
        self._scan_calls_raw()
        pickle.dump((self._argc, self._uses_result, self._dref, self._memdisp), open(cache, 'wb'))

    def _scan_calls_raw(self):
        md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_16)
        argc = collections.defaultdict(collections.Counter)
        uses = collections.defaultdict(collections.Counter)
        dref = collections.defaultdict(set)
        memdisp = set()
        for f in self.files:
            img = f['image']
            mem, lo = self.mem(img)
            fr = int(f['frame'], 16)
            for name, s, e in self.file_functions(f):
                ins = list(md.disasm(bytes(mem[s - lo:e - lo]), s))
                for k, i in enumerate(ins):
                    if i.mnemonic == 'lcall' or (i.mnemonic == 'call' and k and ins[k - 1].mnemonic == 'push'
                                                  and ins[k - 1].op_str == 'cs'):
                        tgt = self.call_target(img, fr, i)
                        if tgt is None:
                            continue
                        n = 0
                        j = k + 1
                        if j < len(ins) and ins[j].mnemonic == 'add' and re.match(r'sp, (0x[0-9a-f]+|\d+)$', ins[j].op_str):
                            n = int(ins[j].op_str[4:], 0) // 2
                            j += 1
                        elif j < len(ins) and ins[j].mnemonic == 'pop' and ins[j].op_str in ('bx', 'cx'):
                            n = 1
                            j += 1
                        elif j < len(ins) and ins[j].mnemonic == 'mov' and ins[j].op_str == 'sp, bp':
                            n = None
                            j += 1
                        if n is not None:
                            argc[tgt][n] += 1
                        # result used?
                        u = 0
                        for i2 in ins[j:j + 3]:
                            ops = i2.op_str
                            if re.search(r'\b(ax|al)\b', ops):
                                u = 1 if not (i2.mnemonic in ('mov', 'xor', 'sub') and re.match(r'(ax|al)\b', ops)
                                              and i2.mnemonic != 'mov' or ops.startswith('ax, ') and i2.mnemonic == 'mov'
                                              and not ops.endswith('ax')) else 0
                                break
                            if i2.mnemonic in ('lcall', 'call', 'retf', 'ret') or i2.mnemonic.startswith('j'):
                                if i2.mnemonic in ('retf', 'ret'):
                                    u = 2       # tail: may be returned
                                break
                        uses[tgt][u] += 1
                    for m in re.finditer(r'0x([0-9a-f]{3,4})\b', i.op_str):
                        dref[int(m.group(1), 16)].add((img, fr))
                    for m in re.finditer(r'(?<!es:)\[(?:b[xp] [+-] |[sd]i [+-] )?0x([0-9a-f]{3,4})\]', i.op_str):
                        memdisp.add(int(m.group(1), 16))
                    if k and i.mnemonic == 'push' and ins[k - 1].mnemonic == 'push' and ins[k - 1].op_str in ('ds', 'ss') \
                            and re.match(r'0x[0-9a-f]+$', i.op_str):
                        memdisp.add(int(i.op_str, 16))
                    if i.mnemonic == 'mov' and re.match(r'(si|di), 0x[0-9a-f]+$', i.op_str):
                        memdisp.add(int(i.op_str[4:], 16))
                    if i.mnemonic == 'mov' and re.match(r'ax, 0x[0-9a-f]+$', i.op_str) and k + 1 < len(ins) and \
                            ins[k + 1].op_str == 'di, ax':
                        memdisp.add(int(i.op_str[4:], 16))
        self._argc = {k: dict(v) for k, v in argc.items()}
        self._uses_result = {k: dict(v) for k, v in uses.items()}
        self._dref = dict(dref)
        self._memdisp = memdisp

    def is_data_addr(self, v):
        if self._dref is None:
            self._scan_calls()
        if v in self._memdisp:
            return True
        st = self.cstring(v)
        return st is not None and len(st) >= 2 and all(0x20 <= c < 0x7f or c in (10, 13) for c in st)

    def argc(self, name):
        if self._argc is None:
            self._scan_calls()
        c = self._argc.get(name)
        return max(c.items(), key=lambda kv: kv[1])[0] if c else None

    def argc_variants(self, name):
        if self._argc is None:
            self._scan_calls()
        return set(self._argc.get(name, {}))

    def result_used(self, name):
        if self._uses_result is None:
            self._scan_calls()
        c = self._uses_result.get(name)
        return bool(c and c.get(1))

    def data_ref_files(self, off):
        if self._dref is None:
            self._scan_calls()
        return self._dref.get(off, set())

    def call_target(self, img, frame, i):
        """Name of the target of a call instruction (capstone insn)."""
        if i.mnemonic == 'lcall':
            b = i.bytes
            if len(b) != 5 or b[0] != 0x9a:
                return None
            off, seg = struct.unpack_from('<HH', b, 1)
            lin = seg * 16 + off
            if seg == 0x9c0 and lin in self.vec_by_addr:
                return self.vec_by_addr[lin]
            for im in ('root', img):
                n = self.addr2name.get((im, lin))
                if n:
                    return n
            # target in a parent overlay
            for n2, (im2, l2, f2) in self.syms.items():
                pass
            return self.name_at_any(lin, seg)
        if i.mnemonic == 'call' and i.bytes[0] == 0xe8:
            lin = (i.address + 3 + struct.unpack_from('<h', i.bytes, 1)[0])
            return self.addr2name.get((img, lin), 'sub_%s_%04x_%04x' % (img, frame, lin - frame * 16))
        return None

    def name_at_any(self, lin, seg):
        if not hasattr(self, '_by_lin'):
            self._by_lin = collections.defaultdict(list)
            for n, (im, l, f) in self.syms.items():
                if not n.startswith('FRM_'):
                    self._by_lin[l].append((im, f, n))
        c = [x for x in self._by_lin.get(lin, []) if x[1] == seg]
        if c:
            c.sort(key=lambda x: (not x[2].startswith('sub_'), x[2]))
            return c[0][2]
        return None


# =============================================================================== expressions
class E:
    """Expression node. kind + args; size in bytes (1, 2, 4) where meaningful."""
    __slots__ = ('k', 'a', 'size', 'uns', 'sid')
    _n = 0

    def __init__(self, k, *a, size=2, uns=False):
        self.k, self.a, self.size, self.uns = k, a, size, uns
        E._n += 1
        self.sid = E._n

    def __repr__(self):
        return '%s%r' % (self.k, self.a)

    def key(self):
        return (self.k, self.size) + tuple(x.key() if isinstance(x, E) else
                                           (tuple(y.key() if isinstance(y, E) else y for y in x) if isinstance(x, list) else x)
                                           for x in self.a)

    def eq(self, o):
        return isinstance(o, E) and self.key() == o.key()

    def has_call(self):
        if self.k in ('call', 'icall'):
            return True
        for x in self.a:
            if isinstance(x, E) and x.has_call():
                return True
            if isinstance(x, list) and any(isinstance(y, E) and y.has_call() for y in x):
                return True
        return False


def C(v, size=2):
    return E('const', v & (0xff if size == 1 else 0xffff if size == 2 else 0xffffffff), size=size)


def sconst(e):
    """Signed value of a 16-bit constant node."""
    v = e.a[0]
    if e.size == 1:
        return v - 0x100 if v >= 0x80 else v
    return v - 0x10000 if v >= 0x8000 else v


def is_const(e, v=None):
    return isinstance(e, E) and e.k == 'const' and (v is None or e.a[0] == (v & (0xff if e.size == 1 else 0xffff)))


# =============================================================================== lifter
JCC = {'je': '==', 'jne': '!=', 'jl': '<', 'jle': '<=', 'jg': '>', 'jge': '>=',
       'jb': '<u', 'jbe': '<=u', 'ja': '>u', 'jae': '>=u', 'js': '<0', 'jns': '>=0'}
NEG = {'==': '!=', '!=': '==', '<': '>=', '>=': '<', '>': '<=', '<=': '>',
       '<u': '>=u', '>=u': '<u', '>u': '<=u', '<=u': '>u'}

R16 = ('ax', 'bx', 'cx', 'dx', 'si', 'di', 'bp', 'sp')
R8 = {'al': ('ax', 0), 'ah': ('ax', 1), 'bl': ('bx', 0), 'bh': ('bx', 1), 'cl': ('cx', 0), 'ch': ('cx', 1),
      'dl': ('dx', 0), 'dh': ('dx', 1)}
SREG = ('es', 'ds', 'ss', 'cs')


class Block:
    def __init__(self, start):
        self.start = start
        self.insns = []
        self.succ = []          # list of linear addresses
        self.term = None        # ('jmp', t) ('jcc', mnem, t, fall) ('ret',) ('table', [targets]) ('fall', t)
        self.preds = []


class Func:
    def __init__(self, prog, img, frame, name, start, end, file_funcs):
        self.prog, self.img, self.frame, self.name, self.start, self.end = prog, img, frame, name, start, end
        self.file_funcs = file_funcs
        self.mem, self.lo = prog.mem(img)
        self.relocs = prog.relocs(img)
        self.warn = []

    # ---------------------------------------------------------------- decode + CFG
    def decode(self):
        md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_16)
        md.detail = True
        self.md = md
        insns = {}
        work = [self.start]
        targets = {self.start}
        self.tables = {}
        while work:
            a = work.pop()
            while self.start <= a < self.end and a not in insns:
                b = bytes(self.mem[a - self.lo: a - self.lo + 16])
                try:
                    i = next(md.disasm(b, a))
                except StopIteration:
                    raise DecompileError('undecodable at %x' % a)
                insns[a] = i
                mn = i.mnemonic
                nxt = a + i.size
                if mn == 'jmp':
                    op = i.operands[0]
                    if op.type == X.X86_OP_IMM:
                        work.append(op.imm)
                        targets.add(op.imm)
                    else:
                        jt = self.prog.jump_tables.get((self.img, a))
                        if jt is None:
                            raise DecompileError('indirect jmp without table at %x' % a)
                        self.tables[a] = jt
                        for t in jt['targets']:
                            work.append(t)
                            targets.add(t)
                    break
                if mn.startswith('j') or mn in ('loop', 'jcxz'):
                    work.append(i.operands[0].imm)
                    targets.add(i.operands[0].imm)
                    targets.add(nxt)
                    a = nxt
                    continue
                if mn in ('retf', 'ret', 'iret'):
                    break
                a = nxt
        self.insns = insns
        addrs = sorted(insns)
        blocks = {}
        cur = None
        for a in addrs:
            i = insns[a]
            if cur is None or a in targets or (cur.insns and cur.insns[-1].address + cur.insns[-1].size != a):
                if cur is not None and cur.term is None:
                    cur.term = ('fall', a)
                cur = Block(a)
                blocks[a] = cur
            cur.insns.append(i)
            mn = i.mnemonic
            if mn == 'jmp':
                if a in self.tables:
                    cur.term = ('table', list(self.tables[a]['targets']))
                else:
                    cur.term = ('jmp', i.operands[0].imm)
                cur = None
            elif mn.startswith('j'):
                cur.term = ('jcc', mn, i.operands[0].imm, a + i.size)
                cur = None
            elif mn in ('retf', 'ret'):
                cur.term = ('ret',)
                cur = None
        if cur is not None and cur.term is None:
            cur.term = ('fall', cur.insns[-1].address + cur.insns[-1].size)
        for b in blocks.values():
            t = b.term
            if t[0] in ('jmp', 'fall'):
                b.succ = [t[1]]
            elif t[0] == 'jcc':
                b.succ = [t[2], t[3]]
            elif t[0] == 'table':
                b.succ = list(dict.fromkeys(t[1]))
            for s in b.succ:
                if s in blocks:
                    blocks[s].preds.append(b.start)
        self.blocks = blocks
        self.order = sorted(blocks)

    # ---------------------------------------------------------------- frame
    def analyse_frame(self):
        first = self.blocks[self.start].insns
        k = 0
        self.frame_size = 0
        self.has_bp = False
        self.saves = []
        if first and first[0].mnemonic == 'enter':
            self.frame_size = first[0].operands[0].imm
            self.has_bp = True
            k = 1
        elif len(first) > 1 and first[0].mnemonic == 'push' and first[0].op_str == 'bp' and \
                first[1].mnemonic == 'mov' and first[1].op_str == 'bp, sp':
            self.has_bp = True
            k = 2
            if len(first) > k and first[k].mnemonic == 'sub' and first[k].op_str.startswith('sp, '):
                self.frame_size = first[k].operands[1].imm
                k += 1
        while k < len(first) and first[k].mnemonic == 'push' and first[k].op_str in ('si', 'di'):
            self.saves.append(first[k].op_str)
            k += 1
        self.prologue_len = k
        # epilogue instructions: pop si/di, leave / mov sp,bp / pop bp, retf
        self.epi = set()
        for b in self.blocks.values():
            if b.term[0] != 'ret':
                continue
            ins = b.insns
            j = len(ins) - 1
            self.epi.add(ins[j].address)
            j -= 1
            if j >= 0 and ins[j].mnemonic == 'leave':
                self.epi.add(ins[j].address)
                j -= 1
            elif j >= 0 and ins[j].mnemonic == 'pop' and ins[j].op_str == 'bp':
                self.epi.add(ins[j].address)
                j -= 1
                if j >= 0 and ins[j].mnemonic == 'mov' and ins[j].op_str == 'sp, bp':
                    self.epi.add(ins[j].address)
                    j -= 1
            while j >= 0 and ins[j].mnemonic == 'pop' and ins[j].op_str in ('si', 'di'):
                self.epi.add(ins[j].address)
                j -= 1


# =============================================================================== per-file decompiler
class FileDec:
    """Decompiles all functions of one original source file; collects declarations."""

    def __init__(self, prog, img, frame):
        self.prog, self.img, self.frame = prog, img, frame
        self.f = prog.file_at(img, frame)
        if self.f is None:
            raise DecompileError('no source file at %s %04x' % (img, frame))
        self.funcs = prog.file_functions(self.f)
        self.local_names = {n for n, s, e in self.funcs}
        self.globals = {}       # addr -> {'sizes': Counter, 'uns': Counter, 'array': elem size or None, 'addr': bool}
        self.fars = {}          # CONST slot -> {'seg': s, 'fields': {disp: size}}
        self.farabs = {}        # (seg, off) -> True   (address taken)
        self.callees = {}       # name -> {'ret': 0/2/4, 'far': True}
        self.strings = {}       # dgroup offset -> bytes
        self.arrays = {}        # base -> loop-filled array info
        self.structs = {}

    def is_literal(self, off):
        s = self.prog.cstring(off)
        if s is None or len(s) > 300:
            return None
        if any(c < 0x20 and c not in (9, 10, 13) or c > 0x7e for c in s):
            if sum(c >= 0x80 for c in s) > 2 or any(c < 0x20 and c not in (9, 10, 13) for c in s):
                return None
        # referenced from other source files -> a shared global, not this file's literal
        refs = self.prog.data_ref_files(off)
        if refs and refs != {(self.img, self.frame)}:
            return None
        return s

    def note_global(self, addr, size, uns=None, array=None, addr_taken=False):
        g = self.globals.setdefault(addr, {'sizes': collections.Counter(), 'uns': collections.Counter(),
                                           'array': None, 'addr': False, 'elem': None})
        g['sizes'][size] += 1
        if uns is not None:
            g['uns'][uns] += 1
        if array:
            g['array'] = array
        if addr_taken:
            g['addr'] = True
        return g


class Lifter:
    """Symbolic execution of one function's blocks into statements."""

    def __init__(self, fd, fn):
        self.fd, self.fn = fd, fn
        self.prog = fd.prog
        self.calln = 0
        self.loopvars = []
        self.struct_locals = set()
        self.shared_cleanup = set()
        self.locals = {}        # bp offset -> {'size': n, 'array': bool, 'init0': n}
        self.regvars = set()
        self.ret_size = None    # 0 void, 2 int, 4 struct/long
        self.ret_struct = None  # bp offset of returned struct

    # --- operand evaluation
    def mem_expr(self, st, op, size):
        m = op.mem
        seg = self.fn.md.reg_name(m.segment) if m.segment else None
        base = self.fn.md.reg_name(m.base) if m.base else None
        idx = self.fn.md.reg_name(m.index) if m.index else None
        disp = m.disp & 0xffff
        if base == 'bp' and idx is None and seg in (None, 'ss'):
            off = m.disp
            self.locals.setdefault(off, {'size': 0, 'array': False})
            self.locals[off]['size'] = max(self.locals[off]['size'], size)
            return E('local', off, size=size)
        if seg == 'cs':
            raise DecompileError('cs: data access')
        if seg == 'es':
            esv = st['es']
            if base is None and idx is None:
                if esv is not None and esv.k == 'farslot':
                    slot = esv.a[0]
                    fr = self.fd.fars.setdefault(slot, {'seg': esv.a[1], 'fields': {}})
                    fr['fields'][disp] = max(fr['fields'].get(disp, 0), size)
                    return E('far', slot, esv.a[1], disp, size=size)
                raise DecompileError('es:[disp] with unknown es %r' % (esv,))
            # es:[bx+disp] through a far pointer
            ptr = st[base] if base else None
            if idx:
                raise DecompileError('es: with index')
            if esv is not None and esv.k == 'hi' and ptr is not None and ptr.k == 'lo' and ptr.a[0].eq(esv.a[0]):
                return E('fderef', ptr.a[0], disp, size=size)
            if esv is not None and esv.k == 'farslot' and ptr is not None:
                slot = esv.a[0]
                fr = self.fd.fars.setdefault(slot, {'seg': esv.a[1], 'fields': {}})
                return E('farindex', slot, esv.a[1], ptr, disp, size=size)
            raise DecompileError('es:[reg] unknown es=%r ptr=%r' % (esv, ptr))
        # DS-relative
        if base is None and idx is None:
            self.fd.note_global(disp, size)
            return E('g', disp, size=size)
        if idx is not None:
            raise DecompileError('base+index addressing')
        b = st[base]
        if b is None:
            raise DecompileError('memory via unknown %s' % base)
        return self.indexed(b, disp, size)

    def indexed(self, b, disp, size):
        """ds:[reg+disp] where reg holds expression b."""
        # scaled index: b = (i << k) or i * n
        scale, i = 1, b
        if b.k == 'bin' and b.a[0] == '<<' and is_const(b.a[2]):
            scale, i = 1 << b.a[2].a[0], b.a[1]
        elif b.k == 'bin' and b.a[0] == '*' and is_const(b.a[2]):
            scale, i = b.a[2].a[0], b.a[1]
        elif b.k == 'bin' and b.a[0] == '+' and b.a[1].eq(b.a[2]):
            scale, i = 2, b.a[1]
        if b.k == 'addr':       # pointer to a known object + disp
            raise DecompileError('deref of address')
        if scale == 1 and size > 1 and b.k in ('local', 'reg', 'g') and disp == 0:
            # plain near pointer variable
            return E('nderef', b, 0, size=size)
        if disp == 0 and scale == 1:
            return E('nderef', b, 0, size=size)
        base = disp
        if scale not in (1, size) and scale != 0:
            # array of structs: element size = scale
            g = self.fd.note_global(base, size, array=scale)
            return E('gidx', base, i, scale, size=size)
        self.fd.note_global(base, size, array=scale)
        return E('gidx', base, i, scale, size=size)

    def val(self, st, op, size=None):
        if op.type == X.X86_OP_REG:
            r = self.fn.md.reg_name(op.reg)
            return self.getreg(st, r)
        if op.type == X.X86_OP_IMM:
            return E('const', op.imm & (0xffff if op.size != 1 else 0xff), size=op.size or 2)
        if op.type == X.X86_OP_MEM:
            return self.mem_expr(st, op, size or op.size)
        raise DecompileError('operand type')

    def getreg(self, st, r):
        if r in R8:
            full, hi = R8[r]
            v = st[full]
            if v is not None and v.k == 'phi':
                raise PhiRead(r)
            if v is None:
                raise DecompileError('read of undefined %s' % r)
            if v.k == 'bytes':
                x = v.a[hi]
                if x is None:
                    raise DecompileError('read of undefined %s' % r)
                return x
            if hi:
                if v.k == 'zext':
                    return C(0, 1)
                return E('hi8', v, size=1)
            if v.k in ('zext', 'sext'):
                return v.a[0]
            if v.k == 'const':
                return C(v.a[0] & 0xff, 1)
            return E('cast', 'char', v, size=1)
        if r in SREG:
            if st[r] is not None and st[r].k == 'phi':
                raise PhiRead(r)
            return st[r] if st[r] is not None else E('seg', r)
        v = st[r]
        if v is not None and v.k == 'phi':
            raise PhiRead(r)
        if v is None or (r in ('si', 'di') and not st.get(r + '_temp')):
            if r in ('si', 'di'):
                self.regvars.add(r)
                return E('reg', r)
            raise DecompileError('read of undefined %s' % r)
        if v.k == 'bytes':
            lo, hi = v.a
            if hi is not None and is_const(hi, 0) and lo is not None:
                return E('zext', lo)
            if lo is not None and hi is not None and lo.k == 'const' and hi.k == 'const':
                return C(lo.a[0] | (hi.a[0] << 8))
            raise DecompileError('16-bit read of partial register %s' % r)
        return v

    def setreg(self, st, r, v):
        if r in R8:
            full, hi = R8[r]
            cur = st[full]
            if cur is not None and cur.k == 'bytes':
                lo_, hi_ = cur.a
            elif cur is not None:
                lo_ = self.getreg(st, full[0] + 'l') if True else None
                hi_ = E('hi8', cur, size=1) if cur.k != 'const' else C(cur.a[0] >> 8, 1)
            else:
                lo_, hi_ = None, None
            if hi:
                hi_ = v
            else:
                lo_ = v
            st[full] = E('bytes', lo_, hi_)
            return
        st[r] = v

    # --- statements
    def emit(self, st, s):
        st['stmts'].append(s)

    def store(self, st, dst_op, v, size):
        if dst_op.type == X.X86_OP_REG:
            r = self.fn.md.reg_name(dst_op.reg)
            if r in ('si', 'di'):
                self.regvars.add(r)
                self.emit(st, ('assign', E('reg', r), v))
                st[r] = None      # value now lives in the variable
                return
            self.setreg(st, r, v)
            return
        lhs = self.mem_expr(st, dst_op, size)
        self.emit(st, ('assign', lhs, v))
        # registers holding the stored call result now refer to the variable
        for r in R16:
            if st.get(r) is v and v.has_call():
                st[r] = lhs
        if v.k == 'bytes':
            pass

    def flush_regs_reading(self, st, lhs):
        """After a store to lhs, registers whose value reads lhs are stale."""
        k = lhs.key()

        def reads(e):
            if not isinstance(e, E):
                return False
            if e.key() == k:
                return True
            return any(reads(x) for x in e.a if isinstance(x, E)) or \
                any(isinstance(x, list) and any(reads(y) for y in x) for x in e.a)
        for r in R16:
            v = st.get(r)
            if v is not None and reads(v) and not v.eq(lhs):
                st[r] = E('stale', r)

    def cleanup_skipped(self, i_next):
        """The call is followed (directly or after stores / a jump) by the function epilogue."""
        fn = self.fn
        a = i_next.address if i_next is not None else None
        steps = 0
        while a is not None and steps < 12:
            steps += 1
            i = fn.insns.get(a)
            if i is None:
                return False
            if a in fn.epi or i.mnemonic in ('leave', 'retf'):
                return True
            if i.mnemonic == 'jmp' and i.operands[0].type == X.X86_OP_IMM:
                a = i.operands[0].imm
                continue
            if i.mnemonic in ('lcall', 'call', 'push', 'pop') or i.mnemonic.startswith('j'):
                return False
            if i.mnemonic == 'add' and i.op_str.startswith('sp'):
                return False
            a = i.address + i.size
        return False

    # --- call
    def do_call(self, st, name, i_next):
        n = None
        if i_next is not None and i_next.mnemonic == 'add' and re.match(r'sp, (0x[0-9a-f]+|\d+)$', i_next.op_str):
            n = int(i_next.op_str[4:], 0) // 2
        if n is None and i_next is not None and i_next.mnemonic == 'mov' and i_next.op_str == 'sp, bp':
            n = len(st['stack'])
        if n is None and i_next is not None and i_next.mnemonic == 'jmp' and i_next.operands[0].type == X.X86_OP_IMM:
            # cleanup shared by several call sites (cross-jumping): jmp to 'add sp, N'
            t = self.fn.insns.get(i_next.operands[0].imm)
            if t is not None and t.mnemonic == 'add' and re.match(r'sp, (0x[0-9a-f]+|\d+)$', t.op_str):
                n = int(t.op_str[4:], 0) // 2
                self.shared_cleanup.add(t.address)
        if n is None and self.cleanup_skipped(i_next):
            # no 'add sp' because the function returns right away: every pending push is an argument
            n = len(st['stack'])
        if n is None:
            n = self.prog.argc(name) or 0
            variants = self.prog.argc_variants(name)
            if i_next is not None and not (i_next.mnemonic == 'add' and i_next.op_str.startswith('sp, ')):
                n = 0 if 0 in variants or not variants else n
        if n > len(st['stack']):
            raise DecompileError('call %s needs %d args, stack has %d' % (name, n, len(st['stack'])))
        words = [self.addr_const(st['stack'].pop()) for _ in range(n)]
        args = group_args(words)
        self.calln += 1
        e = E('call', name, args, self.calln)
        self.fd.callees.setdefault(name, {'ret': 0, 'nargs': n})
        return e


def _addr_const(self, w):
    """A pushed constant that is the address of DGROUP data -> address expression."""
    if w.k != 'const' or w.size != 2:
        return w
    v = w.a[0]
    if v < 0x1000 or v >= 0xff00 or v & (v - 1) == 0:
        return w
    if not self.prog.is_data_addr(v):
        return w
    s = self.fd.is_literal(v)
    if s is not None:
        return E('str', v, s, 'guess')
    self.fd.note_global(v, 1, addr_taken=True)
    return E('addr', E('g', v, size=1), 'guess')


Lifter.addr_const = _addr_const


def group_args(words):
    """Combine consecutive pushed words (first argument first) into C arguments."""
    out = []
    k = 0
    while k < len(words):
        w = words[k]
        nx = words[k + 1] if k + 1 < len(words) else None
        if nx is not None:
            # offset then segment
            if nx.k == 'seg' and nx.a[0] in ('ds', 'ss'):
                if w.k == 'str' and len(w.a) > 2:
                    w = E('str', w.a[0], w.a[1])        # a far pointer to DGROUP: certainly data
                out.append(E('fptr_near', w, nx.a[0], size=4))
                k += 2
                continue
            if nx.k == 'segconst':
                out.append(E('fptr_const', nx.a[0], w, size=4))
                k += 2
                continue
            if nx.k == 'hi32' and w.k in ('call', 'icall') and nx.a[0] is w:
                out.append(w)
                k += 2
                continue
            if nx.k == 'cwd' and nx.a[0].eq(w):
                out.append(E('cast', 'long', w, size=4))
                k += 2
                continue
            if w.k == 'lo' and nx.k == 'hi' and w.a[0].eq(nx.a[0]):
                out.append(w.a[0])
                k += 2
                continue
            if w.k == 'g' and nx.k == 'g' and nx.a[0] == w.a[0] + 2 and w.size == 2 and nx.size == 2:
                out.append(E('g', w.a[0], size=4))
                k += 2
                continue
            if w.k == 'far' and nx.k == 'far' and nx.a[0] == w.a[0] and nx.a[2] == w.a[2] + 2:
                out.append(E('far', w.a[0], w.a[1], w.a[2], size=4))
                k += 2
                continue
            if w.k == 'local' and nx.k == 'local' and nx.a[0] == w.a[0] + 2:
                out.append(E('local', w.a[0], size=4))
                k += 2
                continue
            if w.k == 'fderef' and nx.k == 'fderef' and w.a[0].eq(nx.a[0]) and nx.a[1] == w.a[1] + 2:
                out.append(E('fderef', w.a[0], w.a[1], size=4))
                k += 2
                continue
        out.append(w)
        k += 1
    return out


# =============================================================================== block lifting
class PhiRead(DecompileError):
    pass


def new_state():
    st = {r: None for r in R16}
    st.update({'es': None, 'ds': E('seg', 'ds'), 'ss': E('seg', 'ss'), 'stack': [], 'flags': None})
    return st


def copy_state(st):
    n = dict(st)
    n['stack'] = list(st['stack'])
    return n


def itext(i):
    return '%s %s' % (i.mnemonic, i.op_str) if i.op_str else i.mnemonic


RE_STRCPY = re.compile(
    r'^(?P<setup>(?:(?:mov|lea) (?:di|si), [^;]+;){0,2})push (?P<seg>ds|ss);pop es;mov cx, 0xffff;xor ax, ax;'
    r'repne scasb [^;]*;not cx;sub di, cx;xchg si, di;shr cx, 1;rep movsw [^;]*;adc cx, cx;rep movsb [^;]*;')
RE_LITCPY = re.compile(
    r'^(?P<setup>(?:mov ax, [^;]+;mov di, ax;|(?:mov|lea) di, [^;]+;)(?:mov|lea) si, [^;]+;)push (?P<seg>ds|ss);pop es;'
    r'(?P<body>(?:mov cx, 0x[0-9a-f]+;rep movsw [^;]*;|movsw [^;]*;)*(?:movsb [^;]*;)?)')
RE_MEMSET = re.compile(
    r'^mov (?P<r>bx|dx), (?P<dst>[^;]+);mov cx, (?P<n>[^;]+);mov di, (?P=r);push ds;pop es;rep stos(?P<w>[bw]) [^;]*;')
RE_MEMSET3 = re.compile(
    r'^mov di, (?P<dst>[^;]+);mov cx, (?P<n>[^;]+);push ds;pop es;rep stos(?P<w>[bw]) [^;]*;')
RE_MEMSET2 = re.compile(r'^mov di, (?P<dst>[^;]+);mov cx, (?P<n>[^;]+);rep stos(?P<w>[bw]) [^;]*;')
RE_ZINIT = re.compile(
    r'^mov byte ptr \[bp - (?P<a>0x[0-9a-f]+)\], 0;sub ax, ax;mov cx, (?P<n>[^;]+);lea di, \[bp - (?P<b>0x[0-9a-f]+)\];'
    r'push ss;pop es;rep stos(?P<w>[bw]) [^;]*;(?P<tail>stosb [^;]*;)?')


def imm_of(txt):
    return int(txt, 0)


class FuncLifter(Lifter):
    def lift(self):
        fn = self.fn
        fn.decode()
        fn.analyse_frame()
        self.instances = []          # dicts: block, stmts, term, state, inline, src (pred instance, edge)
        self.by_block = collections.defaultdict(list)
        self.zinit = {}              # bp offset -> total size of zero-initialised local array
        oi = 0
        while oi < len(fn.order):
            a = fn.order[oi]
            oi += 1
            b = fn.blocks[a]
            if a == fn.start:
                entries = [(None, new_state())]
            else:
                entries = []
                lifted = True
                for p in b.preds:
                    if p not in self.by_block or p >= a and not self.by_block[p]:
                        lifted = False
                        continue
                    for inst in self.by_block[p]:
                        entries.append((inst, self.edge_state(inst, a)))
                if not entries and not b.preds:
                    continue        # unreachable
                if not lifted and len(entries) == 1 and b.start in b.succ:
                    inst = self.try_arrloop(b, entries[0][1])
                    if inst is not None:
                        inst['inline'] = False
                        inst['src'] = None
                        self.add_instance(inst)
                        continue
                if not lifted or not entries:
                    # loop header reached by a back edge: nothing carried except register variables
                    st = new_state()
                    for _, s in entries:
                        if s['stack']:
                            raise DecompileError('stack carried into loop header %x' % a)
                    entries = [(None, st)]
            need_inline = any(s['stack'] for _, s in entries) or (len(entries) == 1 and entries[0][0] is not None and
                                                                   entries[0][0]['inline'])
            if not need_inline and len(entries) > 1:
                merged = self.merge([s for _, s in entries])
                try:
                    inst = self.run_block(b, merged)
                    inst['inline'] = False
                    inst['src'] = None
                    self.add_instance(inst)
                    continue
                except PhiRead:
                    need_inline = True
                    if self.split_after_phi(b, merged, self._cur_k):
                        # the block was split: only the prefix that reads the merged registers is copied
                        pass
            if not need_inline:
                inst = None
                if b.start in b.succ:
                    inst = self.try_arrloop(b, entries[0][1])
                if inst is None:
                    inst = self.run_block(b, copy_state(entries[0][1]))
                inst['inline'] = False
                inst['src'] = None
                self.add_instance(inst)
                continue
            if any(s['stack'] for _, s in entries) and len(entries) > 1:
                self.split_after_drain(b, entries)
            for src, s in entries:
                if src is None:
                    raise DecompileError('inline block without predecessor at %x' % a)
                inst = self.run_block(b, copy_state(s))
                inst['inline'] = True
                inst['src'] = src
                self.add_instance(inst)
        return self

    def split_after_drain(self, b, entries):
        """A block entered with arguments already pushed (cross-jumped call tails): only the part
        up to the call that consumes them is copied per predecessor; the rest is shared."""
        probe = []
        saved = (self.calln, len(self.locals))
        try:
            self.run_block(b, copy_state(entries[0][1]), probe)
        except DecompileError:
            return False
        ins = b.insns
        last = len(ins) - (1 if ins and ins[-1].mnemonic.startswith('j') else 0)
        for k, depth in probe:
            if depth == 0:
                p = k + 1
                if p < len(ins) and ins[p].mnemonic == 'add' and ins[p].op_str.startswith('sp'):
                    p += 1
                if p >= last or ins[p].address in self.fn.epi:
                    return False
                self._split(b, p)
                return True
        return False

    def _split(self, b, p):
        fn = self.fn
        ins = b.insns
        nb = Block(ins[p].address)
        nb.insns = ins[p:]
        nb.term = b.term
        nb.succ = b.succ
        nb.preds = [b.start]
        b.insns = ins[:p]
        b.term = ('fall', nb.start)
        for sx in nb.succ:
            if sx in fn.blocks:
                fn.blocks[sx].preds = [nb.start if x == b.start else x for x in fn.blocks[sx].preds]
        b.succ = [nb.start]
        fn.blocks[nb.start] = nb
        fn.order = sorted(fn.blocks)

    def split_after_phi(self, b, merged, kfail):
        """Split block b after the instructions that consume registers whose value depends on the
        predecessor, so that only that prefix is copied per predecessor."""
        fn = self.fn
        rn = fn.md.reg_name
        phi = {r for r in list(R16) + ['es'] if merged.get(r) is not None and merged[r].k == 'phi'}
        ins = b.insns
        last = len(ins) - (1 if ins and ins[-1].mnemonic.startswith('j') else 0)

        def regs(i):
            rr, ww = i.regs_access()
            f = lambda x: {{'al': 'ax', 'ah': 'ax', 'bl': 'bx', 'bh': 'bx', 'cl': 'cx', 'ch': 'cx', 'dl': 'dx',
                            'dh': 'dx', 'eax': 'ax'}.get(rn(y), rn(y)) for y in x}
            r, w = f(rr), f(ww)
            if i.mnemonic in ('lcall', 'call'):
                w |= {'ax', 'bx', 'cx', 'dx', 'es'}
            return r, w
        # never split inside an inline string idiom
        inside = set()
        for k in range(len(ins)):
            win = ';'.join(itext(i) for i in ins[k:k + 18]) + ';'
            for rx in (RE_STRCPY, RE_LITCPY, RE_MEMSET, RE_MEMSET3, RE_ZINIT):
                mm = rx.match(win)
                if mm:
                    n = mm.group(0).count(';')
                    inside.update(range(k + 1, k + n))
        written = set()
        for p in range(1, last):
            r, w = regs(ins[p - 1])
            written |= w
            if p <= kfail or p in inside:
                continue
            live = phi - written
            ok = True
            wr2 = set()
            for i in ins[p:]:
                r2, w2 = regs(i)
                if (r2 - wr2) & live:
                    ok = False
                    break
                wr2 |= w2
            if not ok:
                continue
            if ins[p].address in fn.epi:
                return False
            # split
            nb = Block(ins[p].address)
            nb.insns = ins[p:]
            nb.term = b.term
            nb.succ = b.succ
            nb.preds = [b.start]
            b.insns = ins[:p]
            b.term = ('fall', nb.start)
            for sx in nb.succ:
                if sx in fn.blocks:
                    fn.blocks[sx].preds = [nb.start if x == b.start else x for x in fn.blocks[sx].preds]
            b.succ = [nb.start]
            fn.blocks[nb.start] = nb
            fn.order = sorted(fn.blocks)
            return True
        return False

    def edge_state(self, inst, target):
        if inst.get('eq_edge') == target:
            return inst['eq_state']
        return inst['state']

    def add_instance(self, inst):
        if len(self.instances) > 3 * len(self.fn.blocks) + 20:
            raise DecompileError('too many inlined block copies')
        inst['id'] = len(self.instances)
        self.instances.append(inst)
        self.by_block[inst['block']].append(inst)

    def merge(self, states):
        st = new_state()
        for r in list(R16) + ['es']:
            vals = [s[r] for s in states]
            if all(v is None for v in vals):
                st[r] = None
            elif all(v is not None and v.eq(vals[0]) for v in vals):
                st[r] = vals[0]
            else:
                st[r] = E('phi', r)
        st['axf'] = all(s.get('axf') for s in states)
        for r in ('si', 'di'):
            st[r + '_temp'] = any(s.get(r + '_temp') for s in states)
        fl = [s.get('flags') for s in states]
        st['flags'] = fl[0] if all(f is not None and f[0] == fl[0][0] and all(
            isinstance(x, E) and isinstance(y, E) and x.eq(y) for x, y in zip(f[1:], fl[0][1:])) for f in fl) else None
        return st

    # ---------------------------------------------------------------- strength-reduced init loops
    def try_arrloop(self, b, st):
        """A single-block loop that fills arrays with constants:
            [mov byte ptr [di-K], v] add bx,S  mov [bx+-d], v ...  [inc di]  cmp bx, END  jb/jl self
        -> for (i = 0; i < N; i++) { A[i] = v; B[i][j] = w; ... }"""
        fn = self.fn
        if b.start not in b.succ or b.term[0] != 'jcc' or b.term[2] != b.start:
            return None
        ins = b.insns
        if len(ins) < 4:
            return None
        bx0 = st.get('bx')
        if bx0 is None or bx0.k != 'const':
            return None
        B0 = bx0.a[0]
        rn = fn.md.reg_name
        off = 0               # bx relative to B0 + S*i at this point of the body
        S = None
        stores = []           # (kind 'bx'/'di', addr at i=0, size, value E)
        uses_di = False
        k = 0
        while k < len(ins) - 2:
            i = ins[k]
            mn = i.mnemonic
            if mn == 'add' and i.op_str.startswith('bx, ') and i.operands[1].type == X.X86_OP_IMM:
                if S is not None:
                    return None
                S = i.operands[1].imm
                off += S
            elif mn == 'inc' and i.op_str == 'di':
                uses_di = True
            elif mn == 'mov' and i.operands[0].type == X.X86_OP_MEM and i.operands[1].type == X.X86_OP_IMM:
                m = i.operands[0].mem
                base = rn(m.base) if m.base else None
                if m.index or m.segment:
                    return None
                size = i.operands[0].size
                imm = i.operands[1].imm & (0xffff if size == 2 else 0xff)
                reloc = (i.address + i.size - 2) in fn.relocs
                v = E('segconst', imm) if reloc else C(imm, size)
                if base == 'bx':
                    stores.append(('bx', (B0 + off + m.disp) & 0xffff, size, v))
                elif base == 'di':
                    stores.append(('di', m.disp & 0xffff, size, v))
                else:
                    return None
            else:
                return None
            k += 1
        cmpi, jcc = ins[-2], ins[-1]
        if cmpi.mnemonic != 'cmp' or not cmpi.op_str.startswith('bx, ') or cmpi.operands[1].type != X.X86_OP_IMM:
            return None
        if S is None or S <= 0:
            return None
        END = cmpi.operands[1].imm & 0xffff
        # compared after the increment: iterations while B0 + S*(i+1) < END
        n = (END - B0) // S
        if (END - B0) % S or n <= 0:
            return None
        if uses_di:
            dv = st.get('di')
        # group bx stores into arrays (row stride S)
        bx_st = sorted([x for x in stores if x[0] == 'bx'], key=lambda x: x[1])
        groups = []
        for x in bx_st:
            if groups and x[1] - groups[-1][0] < S and x[1] >= groups[-1][0]:
                groups[-1][1].append(x)
            else:
                groups.append([x[1], [x]])
        body = []
        for x in stores:
            if x[0] == 'di':
                body.append((x[1], 1, None, 0, x[2], x[3]))
        for base, items in groups:
            for kind, addr, size, v in items:
                body.append((base, S, addr - base, 0, size, v))
        # restore source order of the stores
        order = {id(x): n_ for n_, x in enumerate(stores)}
        out = []
        for kind, addr, size, v in stores:
            if kind == 'di':
                out.append(('di', addr, 1, 0, size, v))
            else:
                g = [gg for gg in groups if any(it[1] == addr for it in gg[1])][0]
                out.append(('bx', g[0], S, addr - g[0], size, v))
        # combine offset/segment word pairs into far pointers
        comb = []
        j = 0
        while j < len(out):
            a = out[j]
            if j + 1 < len(out):
                b2 = out[j + 1]
                if a[0] == b2[0] == 'bx' and a[1] == b2[1] and b2[3] == a[3] + 2 and a[4] == b2[4] == 2 and \
                        a[5].k == 'const' and b2[5].k == 'segconst':
                    comb.append((a[0], a[1], a[2], a[3], 4, E('fptr_const', b2[5].a[0], a[5], size=4)))
                    j += 2
                    continue
            comb.append(a)
            j += 1
        self.loopn = getattr(self, 'loopn', 0) + 1
        ivar = 'v_di' if uses_di else 'k_%d' % self.loopn
        if not uses_di:
            self.loopvars.append(ivar)
        else:
            self.regvars.add('di')
        for kind, base, stride, fo, size, v in comb:
            info = self.fd.arrays.setdefault(base, {'n': 0, 'stride': stride, 'fields': {}, 'elem': None})
            info['n'] = max(info['n'], n)
            info['stride'] = stride
            info['fields'][fo] = (size, v.k)
        st2 = copy_state(st)
        st2['bx'] = None
        st2['di'] = None
        st2['flags'] = None
        return {'block': b.start, 'stmts': [('arrloop', ivar, n, comb, uses_di)], 'term': ('goto', b.term[3]),
                'state': st2}

    # ---------------------------------------------------------------- one block
    def run_block(self, b, st, probe=None):
        fn = self.fn
        st = copy_state(st)
        st['stmts'] = []
        st['ax_pending'] = None
        st['axc'] = None
        st['axc_done'] = []
        ins = b.insns
        k = fn.prologue_len if b.start == fn.start else 0
        term = None
        while k < len(ins):
            i = ins[k]
            self._cur_k = k
            if i.address in fn.epi or (k == len(ins) - 1 and i.mnemonic.startswith('j')):
                k += 1
                continue
            n = self.idiom(ins, k, st)
            if n:
                for j in ins[k:k + n]:
                    self.track_ax(j, st)
                    self.fresh_ax(j, st)
                k += n
                continue
            nxt = ins[k + 1] if k + 1 < len(ins) else None
            pend = st.get('ax_pending')
            self.track_ax(i, st)
            self.fresh_ax(i, st)
            self.step(i, nxt, st)
            if probe is not None:
                probe.append((k, len(st['stack'])))
            if i.mnemonic == 'cwde' and pend is not None:
                st['ax_pending'] = (pend[0], st['ax'])
            elif i.mnemonic == 'mov' and i.operands[0].type == X.X86_OP_REG and \
                    fn.md.reg_name(i.operands[0].reg) in ('ax', 'al') and i.operands[1].type == X.X86_OP_MEM:
                st['ax_pending'] = (len(st['stmts']), st['ax'])
            k += 1
        st['axc'] = None
        self.apply_axc(st)
        t = b.term
        if t[0] == 'ret':
            term = ('ret', self.ret_value(b, st))
        elif t[0] in ('jmp', 'fall'):
            term = ('goto', t[1])
        elif t[0] == 'jcc':
            term = ('if', self.condition(st, t[1]), t[2], t[3])
        elif t[0] == 'table':
            term = ('table', st.get('bx'), t[1])
        res = {'block': b.start, 'stmts': st.pop('stmts'), 'term': term, 'state': st}
        # on the edge where 'x == c' holds, registers holding x hold c (MSC uses that)
        if term and term[0] == 'if' and term[2] != term[3]:
            c = term[1]
            if c.k == 'cmp' and c.a[0] in ('==', '!=') and is_const(c.a[2]):
                x = c.a[1]
                st_eq = copy_state(st)
                hit = False
                for r in ('ax', 'bx', 'cx', 'dx'):
                    v = st_eq.get(r)
                    if v is not None and (v is x or v.eq(x)):
                        st_eq[r] = C(c.a[2].a[0])
                        hit = True
                if hit:
                    res['eq_state'] = st_eq
                    res['eq_edge'] = term[2] if c.a[0] == '==' else term[3]
        return res

    def fresh_ax(self, i, st):
        """st['axf'] / st['dxf']: AX / DX hold a value written and not used since (return candidates)."""
        rn = self.fn.md.reg_name
        mn = i.mnemonic
        if mn in ('lcall', 'call'):
            st['axf'] = st['dxf'] = False
            st['flag_ax'] = False
            return
        if mn.startswith('j'):
            if st.get('flag_ax'):
                st['axf'] = False
            return
        rr, ww = i.regs_access()
        r = {rn(x) for x in rr}
        w = {rn(x) for x in ww}
        sets_flags = mn in ('cmp', 'test', 'add', 'sub', 'and', 'or', 'xor', 'inc', 'dec', 'neg', 'shl',
                            'shr', 'sar', 'sbb', 'adc')
        for reg, key in (({'ax', 'al', 'ah', 'eax'}, 'axf'), ({'dx', 'dl', 'dh', 'edx'}, 'dxf')):
            if w & reg:
                st[key] = not mn.startswith('rep') and mn not in ('xchg', 'mul', 'imul', 'div', 'idiv') or \
                    (key == 'axf' and mn in ('mul', 'imul', 'div', 'idiv'))
            elif r & reg:
                st[key] = False
        if sets_flags:
            st['flag_ax'] = bool((w | r) & {'ax', 'al', 'ah', 'eax'})

    def track_axc(self, i, st):
        """A constant loaded into AX, stored exactly once and then overwritten: MSC only does that
        when the constant was also assigned to something that is overwritten later (a dead store):
        'y = c; x = c; ... y = ...;'."""
        rn = self.fn.md.reg_name
        axc = st.get('axc')
        mn = i.mnemonic
        if mn == 'mov' and i.operands[0].type == X.X86_OP_REG and rn(i.operands[0].reg) == 'ax' and \
                i.operands[1].type == X.X86_OP_IMM:
            self.finish_axc(st)
            st['axc'] = {'v': i.operands[1].imm, 'uses': 0, 'store_pos': None}
            return
        if axc is None:
            return
        if mn in ('lcall', 'call'):
            self.finish_axc(st)
            return
        rr, ww = i.regs_access()
        r = {rn(x) for x in rr}
        w = {rn(x) for x in ww}
        if r & {'ax', 'al', 'ah', 'eax'}:
            if mn == 'mov' and i.operands[0].type == X.X86_OP_MEM and i.op_str.endswith(', ax'):
                axc['uses'] += 1
                axc['store_pos'] = len(st['stmts'])
            else:
                axc['uses'] += 100
        if w & {'ax', 'al', 'ah', 'eax'}:
            self.finish_axc(st)

    def finish_axc(self, st):
        axc = st.get('axc')
        st['axc'] = None
        if axc is None or axc['uses'] != 1 or axc['store_pos'] is None:
            return
        st.setdefault('axc_done', []).append(axc)

    def apply_axc(self, st):
        for axc in sorted(st.pop('axc_done', []), key=lambda a: -a['store_pos']):
            self.apply_one_axc(st, axc)

    def apply_one_axc(self, st, axc):
        stmts = st['stmts']
        p = axc['store_pos']
        if p >= len(stmts) or stmts[p][0] != 'assign':
            return
        mine = stmts[p][1]
        for q in range(p + 1, len(stmts)):
            sq = stmts[q]
            if sq[0] == 'assign' and sq[1].k == 'local' and not sq[1].eq(mine):
                stmts.insert(p, ('assign', sq[1], E('const', axc['v'], size=sq[1].size)))
                return

    def track_ax(self, i, st):
        """A value loaded into AX and overwritten unused is a dead store's load: 'x = v; x = w;'."""
        self.track_axc(i, st)
        pend = st.get('ax_pending')
        if pend is None:
            return
        rn = self.fn.md.reg_name
        mn = i.mnemonic
        if mn in ('lcall', 'call') or (mn in ('xor', 'sub') and i.op_str in ('ax, ax', 'al, al')):
            eff = 'write'
        else:
            rr, ww = i.regs_access()
            r = {rn(x) for x in rr}
            w = {rn(x) for x in ww}
            if r & {'ax', 'al', 'ah', 'eax'}:
                eff = 'read'
            elif w & {'ax', 'al', 'eax'}:
                eff = 'write'
            else:
                eff = None
        if eff == 'read':
            st['ax_pending'] = None
        elif eff == 'write':
            pos, v = pend
            stmts = st['stmts']
            if pos < len(stmts) and stmts[pos][0] == 'assign' and v is not None and not v.has_call():
                stmts.insert(pos, ('assign', stmts[pos][1], v))
            st['ax_pending'] = None

    def ret_value(self, b, st):
        """Value returned by a block ending in the epilogue (None: whatever AX happens to hold)."""
        ax, dx = st['ax'], st['dx']
        if dx is not None and dx.k == 'local' and dx.a[0] < 0 and st.get('dxf') and ax is not None and \
                ax.k not in ('phi', 'stale'):
            return E('pair', dx, ax, size=4)
        if ax is None or ax.k in ('phi', 'stale', 'call', 'icall'):
            return None
        if not st.get('axf'):
            return None
        if dx is not None and st.get('dxf') and dx.k not in ('phi', 'stale'):
            return E('pair', dx, ax, size=4)
        return ax

    # ---------------------------------------------------------------- idioms
    def string_temp(self, i, r):
        """Is this write to si/di the operand setup of a following string idiom?"""
        fn = self.fn
        a = i.address + i.size
        for _ in range(8):
            j = fn.insns.get(a)
            if j is None:
                return False
            if j.mnemonic == 'jmp' and j.operands[0].type == X.X86_OP_IMM:
                a = j.operands[0].imm
                continue
            if j.mnemonic in ('repne scasb', 'rep movsw', 'movsw', 'movsb', 'rep movsb', 'rep stosw', 'rep stosb'):
                return True
            if j.mnemonic in ('push', 'pop', 'xor') or (j.mnemonic in ('mov', 'lea') and
                                                      re.match(r'(si|di|cx|ax), ', j.op_str)):
                if j.mnemonic in ('mov', 'lea') and j.op_str.startswith(r + ','):
                    return False
                a = j.address + j.size
                continue
            return False
        return False

    def idiom(self, ins, k, st):
        if ins[k].mnemonic not in ('mov', 'lea', 'sub', 'push'):
            return 0
        win = ';'.join(itext(i) for i in ins[k:k + 18]) + ';'
        m = RE_STRCPY.match(win)
        if m:
            src = dst = None
            for r in ('si', 'di'):
                if st.get(r + '_temp') and st.get(r) is not None and st[r].k == 'phi':
                    raise PhiRead(r)
            if st.get('di_temp') and st.get('di') is not None:
                src = self.ptr_value(st['di'])
            if st.get('si_temp') and st.get('si') is not None:
                dst = self.ptr_value(st['si'])
            for part in m.group('setup').rstrip(';').split(';'):
                if not part:
                    continue
                mn, rest = part.split(' ', 1)
                r, opnd = rest.split(', ', 1)
                v = self.ptr_operand(st, mn, opnd)
                if r == 'di':
                    src = v
                else:
                    dst = v
            if src is None or dst is None:
                raise DecompileError('strcpy operands unknown')
            for r in ('si', 'di'):
                st[r] = None
                st[r + '_temp'] = False
            self.emit(st, ('expr', E('call', 'strcpy', [dst, src], 0)))
            self.fd.callees['strcpy'] = {'ret': 2, 'intrinsic': True}
            st['ax'] = None
            st['cx'] = None
            st['es'] = E('seg', m.group('seg'))
            return m.group(0).count(';')
        m = RE_ZINIT.match(win)
        if m:
            a = int(m.group('a'), 16)
            n = int(m.group('n'), 0)
            cnt = 1 + (n * 2 if m.group('w') == 'w' else n) + (1 if m.group('tail') else 0)
            if int(m.group('b'), 16) != a - 1:
                return 0
            self.zinit[-a] = cnt
            self.locals.setdefault(-a, {'size': 1, 'array': True})
            self.locals[-a]['array'] = True
            st['ax'] = C(0)
            st['cx'] = None
            st['es'] = E('seg', 'ss')
            return m.group(0).count(';')
        m = RE_LITCPY.match(win)
        if m:
            setup = m.group('setup').rstrip(';').split(';')
            dst = src = None
            ax_dst = False
            for part in setup:
                mn, rest = part.split(' ', 1)
                r, opnd = rest.split(', ', 1)
                if r == 'ax':
                    dst = self.ptr_operand(st, mn, opnd)
                    ax_dst = True
                elif r == 'di':
                    if opnd != 'ax':
                        dst = self.ptr_operand(st, mn, opnd)
                else:
                    src = (mn, opnd)
            body = m.group('body')
            nbytes = 0
            for part in body.rstrip(';').split(';'):
                if not part:
                    continue
                if part.startswith('mov cx, '):
                    rep = int(part[8:], 0)
                elif part.startswith('rep movsw'):
                    nbytes += 2 * rep
                elif part.startswith('movsw'):
                    nbytes += 2
                elif part.startswith('movsb'):
                    nbytes += 1
            if nbytes == 0:
                return 0
            mn, opnd = src
            lit = None
            if mn == 'mov' and re.match(r'^0x[0-9a-f]+$|^\d+$', opnd):
                off = int(opnd, 0)
                s = self.fd.is_literal(off)
                if s is not None and len(s) + 1 == nbytes:
                    lit = E('str', off, s)
            if lit is None:
                # fixed-size block copy: struct assignment
                srcv = self.ptr_operand(st, mn, opnd)
                self.emit(st, ('copy', dst, srcv, nbytes))
            else:
                self.emit(st, ('expr', E('call', 'strcpy', [dst, lit], 0)))
                self.fd.callees['strcpy'] = {'ret': 2, 'intrinsic': True}
            st['ax'] = dst if ax_dst else st['ax']
            st['cx'] = None
            st['es'] = E('seg', m.group('seg'))
            return m.group(0).count(';')
        m = RE_MEMSET.match(win) or RE_MEMSET3.match(win)
        if m:
            used = m.group(0).count(';')
            val = st['ax']
            stmts = []
            dst = self.ptr_operand(st, 'mov', m.group('dst'))
            n = int(m.group('n'), 0) * (2 if m.group('w') == 'w' else 1)
            stmts.append((dst, n))
            while True:
                m2 = RE_MEMSET2.match(';'.join(itext(i) for i in ins[k + used:k + used + 3]) + ';')
                if not m2:
                    break
                d2 = self.ptr_operand(st, 'mov', m2.group('dst'))
                stmts.append((d2, int(m2.group('n'), 0) * (2 if m2.group('w') == 'w' else 1)))
                used += 3
            if val is None or val.k != 'const':
                raise DecompileError('memset value not constant')
            v = sconst(val)
            for d, n in stmts:
                self.emit(st, ('expr', E('call', 'memset', [d, C(v), C(n)], 0)))
            self.fd.callees['memset'] = {'ret': 2, 'intrinsic': True}
            st['cx'] = None
            st['bx'] = None
            st['dx'] = None
            st['es'] = E('seg', 'ds')
            return used
        return 0

    def ptr_value(self, v):
        if v.k == 'const':
            off = v.a[0]
            s = self.fd.is_literal(off)
            if s is not None:
                return E('str', off, s)
            self.fd.note_global(off, 1, addr_taken=True)
            return E('addr', E('g', off, size=1))
        return v

    def ptr_operand(self, st, mn, opnd):
        """Pointer value of a setup operand of a string idiom."""
        if mn == 'lea':
            mm = re.match(r'^(?:\w+ ptr )?\[bp - (0x[0-9a-f]+|\d+)\]$', opnd)
            if mm:
                off = -int(mm.group(1), 0)
                self.locals.setdefault(off, {'size': 1, 'array': True})
                self.locals[off]['array'] = True
                return E('addr', E('local', off, size=1))
            mm = re.match(r'^(?:\w+ ptr )?\[bp \+ (0x[0-9a-f]+|\d+)\]$', opnd)
            if mm:
                return E('addr', E('local', int(mm.group(1), 0), size=1))
            raise DecompileError('lea operand %s' % opnd)
        if re.match(r'^0x[0-9a-f]+$|^\d+$', opnd):
            off = int(opnd, 0)
            s = self.fd.is_literal(off)
            if s is not None:
                return E('str', off, s)
            self.fd.note_global(off, 1, addr_taken=True)
            return E('addr', E('g', off, size=1))
        if opnd in R16:
            v = self.getreg(st, opnd)
            return v
        raise DecompileError('string operand %s' % opnd)

    # ---------------------------------------------------------------- generic instruction semantics
    def step(self, i, nxt, st):
        mn = i.mnemonic
        ops = i.operands
        fn = self.fn
        rn = fn.md.reg_name

        def seg_imm():
            a = i.address + i.size - 2
            return a in fn.relocs

        if mn == 'mov':
            d, s = ops
            size = d.size
            if d.type == X.X86_OP_REG and rn(d.reg) == 'es' and s.type == X.X86_OP_MEM:
                m = s.mem
                if not m.base and not m.index and self.prog.is_dgroup_seg_slot(m.disp & 0xffff):
                    slot = m.disp & 0xffff
                    st['es'] = E('farslot', slot, self.prog.dword(slot))
                    return
                raise DecompileError('mov es from non-slot')
            if d.type == X.X86_OP_REG and rn(d.reg) == 'sp' and rn(s.reg) == 'bp':
                return          # stack cleanup after a call
            if s.type == X.X86_OP_IMM and seg_imm():
                v = E('segconst', s.imm & 0xffff)
            else:
                v = self.val(st, s, size)
            if d.type == X.X86_OP_REG:
                r = rn(d.reg)
                if r in ('si', 'di'):
                    if self.string_temp(i, r):
                        st[r] = v
                        st[r + '_temp'] = True
                        return
                    self.regvars.add(r)
                    self.emit(st, ('assign', E('reg', r), v))
                    st[r] = None
                    return
                self.setreg(st, r, v)
                if r in ('ax', 'al'):
                    st['ax_fresh'] = True
                if r in ('dx', 'dl'):
                    st['dx_fresh'] = True
                return
            lhs = self.mem_expr(st, d, size)
            self.assign(st, lhs, v)
            return
        if mn == 'push':
            o = ops[0]
            if o.type == X.X86_OP_REG and rn(o.reg) in SREG:
                r = rn(o.reg)
                if r == 'cs':
                    st['stack'].append(E('seg', 'cs'))
                    return
                st['stack'].append(st[r] if st[r] is not None and r == 'es' else E('seg', r))
                return
            if o.type == X.X86_OP_IMM:
                if seg_imm():
                    st['stack'].append(E('segconst', o.imm & 0xffff))
                else:
                    st['stack'].append(C(o.imm))
                return
            st['stack'].append(self.val(st, o, 2))
            return
        if mn == 'pop':
            r = rn(ops[0].reg)
            if not st['stack']:
                raise DecompileError('pop of empty stack')
            v = st['stack'].pop()
            if r in SREG:
                st[r] = v
            else:
                self.setreg(st, r, v)
            return
        if mn in ('lcall', 'call'):
            if ops[0].type == X.X86_OP_MEM:
                # indirect far call through a table
                tgt = self.val(st, ops[0], 4)
                n = 0
                if nxt is not None and nxt.mnemonic == 'add' and re.match(r'sp, (0x[0-9a-f]+|\d+)$', nxt.op_str):
                    n = int(nxt.op_str[4:], 0) // 2
                elif nxt is not None and nxt.mnemonic == 'mov' and nxt.op_str == 'sp, bp':
                    n = len(st['stack'])
                words = [st['stack'].pop() for _ in range(n)]
                self.calln += 1
                e = E('icall', tgt, group_args(words), self.calln)
            else:
                if mn == 'call':
                    if not st['stack'] or st['stack'][-1].k != 'seg' or st['stack'][-1].a[0] != 'cs':
                        raise DecompileError('near call without push cs')
                    st['stack'].pop()
                name = self.prog.call_target(fn.img, fn.frame, i)
                if name is None:
                    raise DecompileError('unknown call target at %x' % i.address)
                e = self.do_call(st, name, nxt)
            self.after_call(st, e)
            return
        if mn == 'add' and ops[0].type == X.X86_OP_REG and rn(ops[0].reg) == 'sp':
            return
        if mn in ('add', 'sub', 'and', 'or', 'xor', 'adc', 'sbb'):
            d, s = ops
            size = d.size
            if mn in ('xor', 'sub', 'sbb') and d.type == X.X86_OP_REG and s.type == X.X86_OP_REG and d.reg == s.reg:
                r = rn(d.reg)
                if mn == 'sbb':
                    f = st['flags']
                    if f is None or f[0] not in ('cmp', 'cmc'):
                        raise DecompileError('sbb without compare')
                    c = canon_cond(self.cond_from_flags(st, 'jb'))
                    v = E('cond', c, C(-1), C(0))
                else:
                    v = C(0, size)
                self.setreg(st, r, v) if r not in ('si', 'di') else self.store(st, d, v, size)
                if r in ('ax', 'al'):
                    st['ax_fresh'] = True
                if r in ('dx', 'dl'):
                    st['dx_fresh'] = True
                st['flags'] = ('cmp', v, C(0))
                return
            if mn in ('adc', 'sbb'):
                raise DecompileError('adc/sbb arithmetic')
            a = self.val(st, d, size)
            b = self.val(st, s, size)
            if mn in ('or', 'and') and d.type == X.X86_OP_REG and s.type == X.X86_OP_REG and d.reg == s.reg:
                st['flags'] = ('cmp', a, C(0, size))
                return
            op = {'add': '+', 'sub': '-', 'and': '&', 'or': '|', 'xor': '^'}[mn]
            v = arith(op, a, b, size)
            post = self.write(st, d, v, size)
            st['flags'] = ('cmp', post, C(0, size))
            return
        if mn in ('inc', 'dec', 'neg', 'not'):
            d = ops[0]
            size = d.size
            a = self.val(st, d, size)
            if mn == 'inc':
                v = arith('+', a, C(1, size), size)
            elif mn == 'dec':
                v = arith('-', a, C(1, size), size)
            elif mn == 'neg':
                v = arith('neg', a, None, size)
            else:
                v = arith('~', a, None, size)
            post = self.write(st, d, v, size)
            st['flags'] = ('cmp', post, C(0, size))
            return
        if mn in ('shl', 'sal', 'shr', 'sar'):
            d, s = ops
            size = d.size
            a = self.val(st, d, size)
            if s.type == X.X86_OP_IMM:
                b = C(s.imm)
            else:
                b = self.getreg(st, rn(s.reg))
            op = {'shl': '<<', 'sal': '<<', 'shr': '>>u', 'sar': '>>'}[mn]
            v = arith(op, a, b, size)
            post = self.write(st, d, v, size)
            st['flags'] = ('cmp', post, C(0, size))
            return
        if mn in ('cmp', 'test'):
            a = self.val(st, ops[0], ops[0].size)
            b = self.val(st, ops[1], ops[0].size)
            if mn == 'test':
                if ops[0].type == X.X86_OP_REG and ops[1].type == X.X86_OP_REG and ops[0].reg == ops[1].reg:
                    st['flags'] = ('cmp', a, C(0, ops[0].size))
                else:
                    st['flags'] = ('test', a, b)
            else:
                st['flags'] = ('cmp', a, b)
            return
        if mn == 'cmc':
            f = st['flags']
            if f is None or f[0] != 'cmp':
                raise DecompileError('cmc without cmp')
            st['flags'] = ('cmc',) + f[1:]
            return
        if mn == 'cwde':        # cbw
            al = self.getreg(st, 'al')
            st['ax'] = E('sext', al, size=2)
            st['ax_fresh'] = True
            return
        if mn == 'cdq':         # cwd
            ax = self.getreg(st, 'ax')
            st['dx'] = E('cwd', ax)
            return
        if mn in ('idiv', 'div'):
            o = ops[0]
            dv = self.val(st, o, o.size)
            ax = self.getreg(st, 'ax')
            dx = st['dx']
            if o.size != 2:
                raise DecompileError('8-bit division')
            if mn == 'idiv':
                if dx is None or dx.k != 'cwd' or not dx.a[0].eq(ax):
                    if dx is not None and dx.k == 'pair':
                        a = dx
                    else:
                        raise DecompileError('idiv without cwd')
                st['ax'] = E('bin', '/', ax, dv)
                st['dx'] = E('bin', '%', ax, dv)
            else:
                if dx is None or not is_const(dx, 0):
                    raise DecompileError('div without zeroed dx')
                st['ax'] = E('bin', '/u', ax, dv)
                st['dx'] = E('bin', '%u', ax, dv)
            st['ax_fresh'] = True
            st['dx_fresh'] = True
            return
        if mn in ('imul', 'mul'):
            if len(ops) == 3:
                a = self.val(st, ops[1], 2)
                v = arith('*', a, C(ops[2].imm), 2)
                self.write(st, ops[0], v, 2)
                return
            if len(ops) == 2:
                a = self.val(st, ops[0], 2)
                b = self.val(st, ops[1], 2)
                self.write(st, ops[0], arith('*', a, b, 2), 2)
                return
            o = ops[0]
            if o.size != 2:
                raise DecompileError('8-bit multiply')
            b = self.val(st, o, 2)
            a = self.getreg(st, 'ax')
            st['ax'] = arith('*' if mn == 'imul' else '*u', a, b, 2)
            st['dx'] = E('mulhi', a, b)
            st['ax_fresh'] = True
            return
        if mn == 'lea':
            d, s = ops
            m = s.mem
            base = rn(m.base) if m.base else None
            idx = rn(m.index) if m.index else None
            if base == 'bp' and idx is None:
                off = m.disp
                if off < 0:
                    self.locals.setdefault(off, {'size': 0, 'array': False})
                v = E('addr', E('local', off, size=2))
            elif idx is None and base is not None:
                v = arith('+', self.getreg(st, base), C(m.disp), 2)
            else:
                raise DecompileError('lea form')
            self.write(st, d, v, 2)
            return
        if mn == 'les':
            d, s = ops
            v = self.val(st, s, 4)
            r = rn(d.reg)
            st['es'] = E('hi', v)
            self.setreg(st, r, E('lo', v))
            return
        if mn == 'xchg':
            a, b = rn(ops[0].reg), rn(ops[1].reg)
            va, vb = st[a], st[b]
            st[a], st[b] = vb, va
            if 'ax' in (a, b):
                st['ax_fresh'] = True
            return
        if mn == 'nop':
            return
        raise DecompileError('unhandled instruction %s %s' % (mn, i.op_str))

    def write(self, st, d, v, size):
        """Store v into operand d; returns the expression that now holds the value (for flags)."""
        if d.type == X.X86_OP_REG:
            r = self.fn.md.reg_name(d.reg)
            if r in ('si', 'di'):
                self.regvars.add(r)
                self.emit(st, ('assign', E('reg', r), v))
                st[r] = None
                return E('reg', r)
            self.setreg(st, r, v)
            return v
        lhs = self.mem_expr(st, d, size)
        self.assign(st, lhs, v)
        return lhs

    def assign(self, st, lhs, v):
        # x = x + c  /  x = x - c  -> compound forms are left to the printer
        self.emit(st, ('assign', lhs, v))
        self.flush_regs_reading(st, lhs)
        for r in ('ax', 'bx', 'cx', 'dx'):
            if st.get(r) is v and v.has_call():
                st[r] = lhs

    def after_call(self, st, e):
        st['ax'] = e
        st['dx'] = E('hi32', e)
        st['bx'] = None
        st['cx'] = None
        st['es'] = None
        st['ax_fresh'] = True
        st['dx_fresh'] = False
        st['pending'] = e
        # the call is a statement unless its value gets consumed; record position
        self.emit(st, ('call', e))

    def condition(self, st, jcc):
        return self.cond_from_flags(st, jcc)

    def cond_from_flags(self, st, jcc):
        f = st['flags']
        if f is None:
            raise DecompileError('branch without flags')
        op = JCC[jcc]
        if f[0] == 'cmc':
            if op not in ('<u', '>=u'):
                raise DecompileError('cmc with %s' % jcc)
            op = NEG[op]
            f = ('cmp',) + f[1:]
        if f[0] == 'test':
            if op not in ('==', '!='):
                raise DecompileError('test with %s' % jcc)
            return E('cmp', op, E('bin', '&', f[1], f[2]), C(0))
        a, b = f[1], f[2]
        if op in ('<0', '>=0'):
            return E('cmp', '<' if op == '<0' else '>=', a, C(0))
        # (x - c) == 0  ->  x == c
        if is_const(b, 0) and a.k == 'bin' and a.a[0] in ('+', '-') and is_const(a.a[2]) and op in ('==', '!='):
            c = sconst(a.a[2])
            r = E('cmp', op, a.a[1], C(c if a.a[0] == '-' else -c))
            r.uns = 'arith'         # value tested through dec/sub on a copy: switch code
            return r
        return E('cmp', op, a, b)


def canon_cond(c):
    """(unsigned)x < 1 -> x == 0;  (unsigned)x >= 1 -> x != 0"""
    if c.k == 'cmp' and is_const(c.a[2], 1) and c.a[0] in ('<u', '>=u'):
        return E('cmp', '==' if c.a[0] == '<u' else '!=', c.a[1], C(0))
    return c


def arith(op, a, b, size):
    """Build a simplified arithmetic node."""
    if a is not None and a.k == 'cond' and (b is None or b.k == 'const') and op in ('&', '+', '-', 'neg', '~', '|'):
        c, x, y = a.a
        if x.k == 'const' and y.k == 'const':
            f = {'&': lambda p: p & b.a[0], '+': lambda p: p + b.a[0], '-': lambda p: p - b.a[0],
                 '|': lambda p: p | b.a[0], 'neg': lambda p: -p, '~': lambda p: ~p}[op]
            return E('cond', c, C(f(x.a[0])), C(f(y.a[0])))
    if b is not None and a.k == 'const' and b.k == 'const' and op in ('+', '-', '&', '|', '^', '<<'):
        f = {'+': lambda p, q: p + q, '-': lambda p, q: p - q, '&': lambda p, q: p & q, '|': lambda p, q: p | q,
             '^': lambda p, q: p ^ q, '<<': lambda p, q: p << q}[op]
        return C(f(a.a[0], b.a[0]), size)
    if b is None:
        if op == 'neg':
            return E('un', '-', a, size=size)
        return E('un', '~', a, size=size)
    # signed division by powers of two:  (x - cwd(x)) >> 1   and   ((|x| >> n) ^ s) - s
    if b.k == 'cwd':
        x = b.a[0]
        if op == '-' and a.eq(x):
            return E('xmcwd', x)
        if op == '^' and a.eq(x):
            return E('sgnxor', x)
        if op == '-' and a.k == 'sgnxor' and a.a[0].eq(x):
            return E('absx', x)
        if op == '^' and a.k == 'absshr' and a.a[0].eq(x):
            return E('sgnxor2', x, a.a[1])
        if op == '-' and a.k == 'sgnxor2' and a.a[0].eq(x):
            return E('bin', '/', x, C(1 << a.a[1]), size=size)
        if op == '&' and is_const(a) is False:
            pass
    if op == '>>' and a.k == 'xmcwd' and is_const(b, 1):
        return E('bin', '/', a.a[0], C(2), size=size)
    if op == '>>' and a.k == 'absx' and b.k == 'const':
        return E('absshr', a.a[0], b.a[0])
    # fold (x + c1) + c2
    if op in ('+', '-') and b.k == 'const' and a.k == 'bin' and a.a[0] in ('+', '-') and is_const(a.a[2]):
        c1 = sconst(a.a[2]) * (1 if a.a[0] == '+' else -1)
        c2 = sconst(b) * (1 if op == '+' else -1)
        t = c1 + c2
        if t == 0:
            return a.a[1]
        return E('bin', '+' if t > 0 else '-', a.a[1], C(abs(t)), size=size)
    if op in ('+', '-') and is_const(b, 0):
        return a
    return E('bin', op, a, b, size=size)

# =============================================================================== return-value analysis
def last_ax_write(fn, b):
    """Kind of the last write to AX in block b before its epilogue: 'call', 'value', or None
    (None also when AX is read afterwards)."""
    ins = [i for i in b.insns if i.address not in fn.epi]
    for i in reversed(ins):
        rr, ww = i.regs_access()
        names_r = {fn.md.reg_name(r) for r in rr}
        names_w = {fn.md.reg_name(r) for r in ww}
        if names_w & {'ax', 'al', 'ah', 'eax'}:
            if i.mnemonic in ('lcall', 'call'):
                return 'call'
            if i.mnemonic.startswith('rep') or i.mnemonic in ('xchg',):
                return None
            return 'value'
        if names_r & {'ax', 'al', 'ah', 'eax'}:
            return None
        if i.mnemonic.startswith('j'):
            continue
    return 'none'


def function_returns(fn, prog):
    kinds = []
    for b in fn.blocks.values():
        if b.term[0] != 'ret':
            continue
        k = last_ax_write(fn, b)
        if k == 'none':
            for p in b.preds:
                kinds.append(last_ax_write(fn, fn.blocks[p]))
        else:
            kinds.append(k)
    if 'value' in kinds:
        return True
    if prog.result_used(fn.name):
        return True
    return False


# =============================================================================== C emission
CTYPE = {1: 'char', 2: 'int', 4: 'long'}
PREC = {'call': 16, 'idx': 16, 'field': 16, 'un': 14, 'cast': 14, '*': 13, '/': 13, '%': 13, '+': 12, '-': 12,
        '<<': 11, '>>': 11, 'cmp': 10, 'eq': 9, '&': 8, '^': 7, '|': 6, '&&': 5, '||': 4, '?': 3, '=': 2}


class Printer:
    def __init__(self, fd, lifter, ret_kind):
        self.fd, self.L = fd, lifter
        self.ret_kind = ret_kind
        self.consumed = set()

    def hexs(self, v):
        return '0x%x' % v if v >= 10 else str(v)

    def num(self, e, signed=True):
        v = sconst(e) if signed else e.a[0]
        if v < 0:
            return '-' + self.hexs(-v)
        return self.hexs(v)

    # --- names
    def gname(self, off):
        return 'D_%04x' % off

    def lname(self, off):
        L = self.L
        if L.ret_struct is not None and off in (L.ret_struct, L.ret_struct + 2):
            return 'r.' + ('a' if off == L.ret_struct else 'b')
        if off > 0:
            return 'p_%x' % off
        if off in L.struct_locals:
            return 'l_%x.a' % -off
        if off - 2 in L.struct_locals:
            return 'l_%x.b' % -(off - 2)
        return 'l_%x' % -off

    def lvalue_type(self, e):
        if e.k == 'g':
            g = self.fd.globals[e.a[0]]
            return self.fd.gtype(e.a[0])
        if e.k == 'local':
            return CTYPE.get(e.size, 'int')
        return CTYPE.get(e.size, 'int')

    # --- expressions; returns (text, precedence)
    def ex(self, e, want=None):
        t, p = self._ex(e)
        return t, p

    def par(self, e, prec):
        t, p = self._ex(e)
        return '(%s)' % t if p < prec else t

    def _ex(self, e):
        k = e.k
        if k == 'const':
            return self.num(e), 16
        if k == 'g':
            off = e.a[0]
            ao = self.fd.array_of(off)
            if ao is not None:
                return 'D_%04x%s' % ao, 16
            g = self.fd.globals.get(off)
            decl = self.fd.gsize(off)
            nm = self.gname(off)
            if g is not None and self.fd.is_array(off):
                nm = nm + '[0]'
            if e.size < decl:
                return '(%s)%s' % ('char' if e.size == 1 else 'int', nm), 14
            return nm, 16
        if k == 'gidx':
            base, idx, scale, size = e.a[0], e.a[1], e.a[2], e.size
            if base in self.fd.arrays and self.fd.arrays[base]['stride'] != self.fd.arr_elem(base):
                raise DecompileError('1-d access to a 2-d array')
            return '%s[%s]' % (self.gname(base), self.ex(idx)[0]), 16
        if k == 'local':
            return self.lname(e.a[0]), 16
        if k == 'reg':
            return 'v_' + e.a[0], 16
        if k == 'far':
            slot, seg, disp = e.a
            return '%s.f_%04x' % (self.fd.farname(slot), disp), 16
        if k == 'fderef':
            ptr, disp = e.a
            ty = {1: 'char', 2: 'int', 4: 'long'}[e.size]
            if disp:
                return '*(%s far *)(%s + %s)' % (ty, self.par(ptr, 12), self.hexs(disp)), 14
            return '*(%s far *)%s' % (ty, self.par(ptr, 14)), 14
        if k == 'nderef':
            ptr, disp = e.a
            ty = {1: 'char', 2: 'int', 4: 'long'}[e.size]
            return '*(%s *)%s' % (ty, self.par(ptr, 14)), 14
        if k == 'str':
            return cstr(e.a[1]), 16
        if k == 'addr':
            inner = e.a[0]
            if inner.k == 'g':
                if self.fd.is_array(inner.a[0]):
                    return self.gname(inner.a[0]), 16
                return '&' + self.gname(inner.a[0]), 14
            if inner.k == 'local':
                off = inner.a[0]
                if self.L.locals.get(off, {}).get('array'):
                    return self.lname(off), 16
                return '&' + self.lname(off), 14
            return '&' + self.par(inner, 14), 14
        if k == 'fptr_near':
            off, seg = e.a
            t = self.par(off, 14)
            return '(char far *)%s' % t, 14
        if k == 'fptr_const':
            seg, off = e.a
            if isinstance(off, E):
                if off.k != 'const':
                    raise DecompileError('far pointer with constant segment and variable offset')
                off = off.a[0]
            name = self.fd.code_or_data_at(seg, off)
            return name, 16
        if k == 'sext':
            return self._ex(e.a[0])
        if k == 'zext':
            inner = e.a[0]
            t, p = self._ex(inner)
            if self.fd.expr_unsigned(inner):
                return t, p
            return '(unsigned char)%s' % self.par(inner, 14), 14
        if k == 'cast':
            ty, inner = e.a
            return '(%s)%s' % (ty, self.par(inner, 14)), 14
        if k == 'call':
            name, args, cid = e.a
            self.consumed.add(e.sid)
            return '%s(%s)' % (cname(name), ', '.join(self.ex(a)[0] for a in args)), 16
        if k == 'icall':
            tgt, args, cid = e.a
            self.consumed.add(e.sid)
            return '(*%s)(%s)' % (self.ex(tgt)[0], ', '.join(self.ex(a)[0] for a in args)), 16
        if k == 'hi32':
            inner = e.a[0]
            return '(int)(%s >> 16)' % self.par(inner, 11), 14
        if k == 'bin':
            op, a, b = e.a
            if op in ('/u', '%u', '>>u', '*u'):
                o = op[:-1]
                ta = self.par(a, PREC[o] if o != '>>' else 11)
                if not self.fd.expr_unsigned(a):
                    ta = '(unsigned)' + self.par(a, 14)
                return '%s %s %s' % (ta, o, self.par(b, PREC[o] + 1)), PREC[o]
            if op == '+' and b.k == 'const' and sconst(b) < 0:
                return '%s - %s' % (self.par(a, 12), self.hexs(-sconst(b))), 12
            pr = PREC[op]
            return '%s %s %s' % (self.par(a, pr), op, self.par(b, pr + 1)), pr
        if k == 'un':
            op, a = e.a
            return '%s%s' % (op, self.par(a, 14)), 14
        if k == 'cmp':
            op, a, b = e.a
            uns = op.endswith('u')
            o = op.rstrip('u')
            ta = self.par(a, 10)
            tb = self.par(b, 11)
            if uns and not self.fd.expr_unsigned(a) and not self.fd.expr_unsigned(b):
                if b.k == 'const':
                    ta = '(unsigned)' + self.par(a, 14)
                else:
                    ta = '(unsigned)' + self.par(a, 14)
            pr = 9 if o in ('==', '!=') else 10
            return '%s %s %s' % (ta, o, tb), pr
        if k == 'cond':
            c, a, b = e.a
            if c.k == 'cmp' and is_const(c.a[2], 0) and c.a[0] in ('==', '!='):
                if c.a[0] == '==':
                    a, b = b, a
                return '%s ? %s : %s' % (self.par(c.a[1], 4), self.par(a, 4), self.par(b, 3)), 3
            return '%s ? %s : %s' % (self.par(c, 4), self.par(a, 4), self.par(b, 3)), 3
        if k == 'assign':
            lhs, v = e.a
            return '%s = %s' % (self.ex(lhs)[0], self.par(v, 2)), 2
        if k == 'and':
            return '%s && %s' % (self.pcond(e.a[0], 5), self.pcond(e.a[1], 6)), 5
        if k == 'or':
            return '%s || %s' % (self.pcond(e.a[0], 4), self.pcond(e.a[1], 5)), 4
        if k == 'not':
            return '!%s' % self.pcond(e.a[0], 14), 14
        if k == 'lo':
            return self._ex(e.a[0])
        if k == 'pair':
            hi, lo = e.a
            if hi.k == 'hi32' and hi.a[0] is lo:
                return self._ex(lo)
            if hi.k == 'cwd' and hi.a[0].eq(lo):
                return '(long)%s' % self.par(lo, 14), 14
            raise DecompileError('cannot print pair %r' % (e,))
        raise DecompileError('cannot print %s' % k)

    def pcond(self, e, prec):
        t = self.cond(e)
        p = 14 if t.startswith('!') and e.k != 'cmp' else (self._ex(e)[1] if e.k not in ('cmp',) else
                                                            (9 if e.a[0] in ('==', '!=') else 10))
        if e.k == 'cmp' and is_const(e.a[2], 0) and e.a[0] in ('==', '!='):
            p = 14 if e.a[0] == '==' else self._ex(e.a[1])[1]
        return '(%s)' % t if p < prec else t

    def cond(self, e, negate=False):
        if negate:
            if e.k == 'cmp':
                e = E('cmp', NEG[e.a[0]], e.a[1], e.a[2])
            else:
                return '!(%s)' % self.ex(e)[0]
        if e.k == 'cmp' and e.a[0] in ('!=',) and is_const(e.a[2], 0):
            return self.ex(e.a[1])[0]
        if e.k == 'cmp' and e.a[0] in ('==',) and is_const(e.a[2], 0):
            return '!' + self.par(e.a[1], 14)
        return self.ex(e)[0]


def cstr(b):
    out = []
    for c in b:
        ch = chr(c)
        if ch == '"':
            out.append('\\"')
        elif ch == '\\':
            out.append('\\\\')
        elif ch == '\n':
            out.append('\\n')
        elif ch == '\r':
            out.append('\\r')
        elif ch == '\t':
            out.append('\\t')
        elif 0x20 <= c < 0x7f:
            out.append(ch)
        else:
            out.append('\\x%02x' % c)
    s = ''.join(out)
    # a hex escape followed by a hex digit would swallow it
    s = re.sub(r'(\\x[0-9a-f]{2})(?=[0-9a-fA-F])', r'\1""', s)
    return '"%s"' % s

# =============================================================================== statement post-processing
def combine_far_stores(stmts):
    """offset store + segment store of the same dword -> one far-pointer assignment."""
    out = []
    k = 0
    while k < len(stmts):
        s = stmts[k]
        n = stmts[k + 1] if k + 1 < len(stmts) else None
        if s[0] == 'assign' and n is not None and n[0] == 'assign' and n[2].k == 'segconst':
            a, b = s[1], n[1]
            same = False
            if a.k == 'g' and b.k == 'g' and b.a[0] == a.a[0] + 2:
                lhs = E('g', a.a[0], size=4)
                same = True
            elif a.k == 'local' and b.k == 'local' and b.a[0] == a.a[0] + 2:
                lhs = E('local', a.a[0], size=4)
                same = True
            elif a.k == 'far' and b.k == 'far' and a.a[0] == b.a[0] and b.a[2] == a.a[2] + 2:
                lhs = E('far', a.a[0], a.a[1], a.a[2], size=4)
                same = True
            if same and s[2].k == 'const':
                out.append(('assign', lhs, E('fptr_const', n[2].a[0], s[2], size=4)))
                k += 2
                continue
        out.append(s)
        k += 1
    return out


def walk(e, f):
    if isinstance(e, E):
        f(e)
        for x in e.a:
            if isinstance(x, (E, list)):
                walk(x, f)
    elif isinstance(e, list):
        for x in e:
            walk(x, f)


# =============================================================================== function emission
class FuncEmitter:
    def __init__(self, fd, fn, L, returns):
        self.fd, self.fn, self.L = fd, fn, L
        self.returns = returns
        self.P = Printer(fd, L, returns)
        self.lines = []
        self.labels = set()

    def inst_for(self, src, target):
        for j in self.L.by_block.get(target, []):
            if j['inline'] and j['src'] is src:
                return j
        return None

    def main_inst(self, target):
        for j in self.L.by_block.get(target, []):
            if not j['inline']:
                return j
        return None

    def collect_labels(self, order):
        """Blocks that need a label: targets of transfers that are not a fall-through."""
        need = set()
        pos = {inst['block']: n for n, inst in enumerate(order)}

        def visit(inst, nxt_block):
            t = inst['term']
            if t is None:
                return
            targets = []
            if t[0] == 'goto':
                targets = [(t[1], True)]
            elif t[0] == 'if':
                targets = [(t[2], False), (t[3], True)]
            elif t[0] == 'table':
                targets = [(x, False) for x in t[2]]
            elif t[0] == 'switch':
                targets = [(x, False) for x in list(t[2].values()) + [t[3]]]
            for tb, is_fall in targets:
                j = self.inst_for(inst, tb)
                if j is not None:
                    visit(j, nxt_block)
                    continue
                if is_fall and tb == nxt_block:
                    continue
                m = self.main_inst(tb)
                if m is not None and m['term'] and m['term'][0] == 'ret' and not m['stmts'] and \
                        (m['term'][1] is None):
                    continue        # becomes 'return;'
                need.add(tb)
        for n, inst in enumerate(order):
            nxt = order[n + 1]['block'] if n + 1 < len(order) else None
            visit(inst, nxt)
        return need

    def stmt_text(self, s, used):
        P = self.P
        if s[0] == 'assign':
            lhs, v = s[1], s[2]
            lt = P.ex(lhs)[0]
            if lhs.k == 'local' and lhs.a[0] in self.L.struct_locals and v.k in ('call', 'icall'):
                return '%s = %s;' % (lt[:-2], P.ex(v)[0])
            if v.k == 'bin' and v.a[0] in ('+', '-', '&', '|', '^', '<<', '>>', '*') and v.a[1].eq(lhs):
                if v.a[0] in ('+', '-') and is_const(v.a[2], 1):
                    return '%s%s;' % (lt, '++' if v.a[0] == '+' else '--')
                return '%s %s= %s;' % (lt, v.a[0], P.ex(v.a[2])[0])
            return '%s = %s;' % (lt, P.ex(v)[0])
        if s[0] == 'expr':
            return P.ex(s[1])[0] + ';'
        if s[0] == 'call':
            if s[1].sid in used:
                return None
            return P.ex(s[1])[0] + ';'
        if s[0] == 'arrloop':
            ivar, n, comb, uses_di = s[1], s[2], s[3], s[4]
            lines = ['for (%s = 0; %s < %d; %s++) {' % (ivar, ivar, n, ivar)]
            for kind, base, stride, fo, size, v in comb:
                nm = 'D_%04x' % base
                info = self.fd.arrays.get(base)
                es = self.fd.arr_elem(base)
                if stride == es or kind == 'di':
                    ref = '%s[%s]' % (nm, ivar)
                else:
                    ref = '%s[%s][%d]' % (nm, ivar, fo // es)
                lines.append('\t%s = %s;' % (ref, P.ex(v)[0]))
            lines.append('}')
            return '\n'.join(lines)
        if s[0] == 'copy':
            dst, src, n = s[1], s[2], s[3]
            self.fd.copy_sizes.add(n)
            return '*(struct blk%d *)%s = *(struct blk%d *)%s;' % (n, P.par(dst, 14), n, P.par(src, 14))
        raise DecompileError('statement %s' % s[0])

    def body(self):
        L = self.L
        order = [inst for inst in L.instances if not inst['inline']]
        order.sort(key=lambda i: i['block'])
        # sids consumed by expressions other than their own placeholder
        used = set()

        def note(e):
            if e.k in ('call', 'icall'):
                used.add(e.sid)
        for inst in L.instances:
            inst['stmts'] = combine_far_stores(inst['stmts'])
            for s in inst['stmts']:
                if s[0] == 'call':
                    for x in s[1].a:
                        walk(x, note)
                else:
                    for x in s[1:]:
                        walk(x, note)
            t = inst['term']
            if t and (t[0] != 'ret' or self.returns):
                for x in t[1:]:
                    walk(x, note)
        self.used = used
        try:
            S = Structurer(self)
            items = S.run()
            out = []
            self.render(items, 1, S, out)
            # drop a trailing 'return;' of a void function
            if out and out[-1] == (1, 'return;'):
                out.pop()
            if out and out[-1][1].endswith(':'):
                out.append((1, ';'))
            return out
        except DecompileError as ex:
            if os.environ.get('DEC16_STRICT'):
                raise
            self.fn.warn.append('structurer: %s' % ex)
        self.labels = self.collect_labels(order)
        out = []
        for n, inst in enumerate(order):
            nxt = order[n + 1]['block'] if n + 1 < len(order) else None
            if inst['block'] in self.labels:
                out.append((0, 'L_%04x:' % (inst['block'] - self.fn.frame * 16)))
            self.emit_inst(inst, nxt, out, 1, last=(n + 1 == len(order)))
        # a label at the very end needs a statement
        if out and out[-1][1].endswith(':'):
            out.append((1, ';'))
        return out

    def label_name(self, S, nid):
        n = S.nodes[nid]
        base = 'L_%04x' % (n.addr - self.fn.frame * 16)
        return base if n.key[1] == 0 else '%s_%d' % (base, n.id)

    def render(self, items, ind, S, out):
        P = self.P
        for it in items:
            k = it[0]
            if k == 'label':
                if it[1] in S.gotos:
                    out.append((0, self.label_name(S, it[1]) + ':'))
            elif k == 'stmt':
                t = self.stmt_text(it[1], self.used)
                if t is not None:
                    for ln in t.split('\n'):
                        out.append((ind + ln.count('\t'), ln.lstrip('\t')))
            elif k == 'ret':
                v = it[1]
                if self.returns:
                    if v is None:
                        out.append((ind, 'return;'))
                    elif self.L.ret_struct is not None and v.k == 'pair':
                        ax_ = v.a[1]
                        if not (ax_.k == 'local' and ax_.a[0] == self.L.ret_struct):
                            out.append((ind, 'r.a = %s;' % P.ex(ax_)[0]))
                        out.append((ind, 'return r;'))
                    else:
                        out.append((ind, 'return %s;' % P.ex(v)[0]))
                else:
                    out.append((ind, 'return;'))
            elif k == 'goto':
                out.append((ind, 'goto %s;' % self.label_name(S, it[1])))
            elif k == 'break':
                out.append((ind, 'break;'))
            elif k == 'continue':
                out.append((ind, 'continue;'))
            elif k == 'if':
                c, then, els = it[1], it[2], it[3]
                out.append((ind, 'if (%s) {' % P.cond(c)))
                self.render(then, ind + 1, S, out)
                if els:
                    out.append((ind, '} else {'))
                    self.render(els, ind + 1, S, out)
                out.append((ind, '}'))
            elif k == 'loop':
                kind, c, body = it[1], it[2], it[3]
                if kind == 'do':
                    out.append((ind, 'do {'))
                    self.render(body, ind + 1, S, out)
                    out.append((ind, '} while (%s);' % P.cond(c)))
                else:
                    out.append((ind, 'for (;;) {'))
                    self.render(body, ind + 1, S, out)
                    out.append((ind, '}'))
            elif k == 'switch':
                subj, arms = it[1], it[2]
                out.append((ind, 'switch (%s) {' % P.ex(subj)[0]))
                for vals, body in arms:
                    for v in sorted(vals, key=lambda x: (x == 'default', x if x != 'default' else 0)):
                        out.append((ind, 'default:' if v == 'default' else 'case %s:' % self.P.hexs(v) if v >= 0
                                    else 'case -%s:' % self.P.hexs(-v)))
                    self.render(body, ind + 1, S, out)
                out.append((ind, '}'))
            else:
                raise DecompileError('ast %s' % k)

    def emit_inst(self, inst, nxt, out, ind, last=False):
        for s in inst['stmts']:
            t = self.stmt_text(s, self.used)
            if t is not None:
                for ln in t.split('\n'):
                    out.append((ind + ln.count('\t'), ln.lstrip('\t')))
        t = inst['term']
        P = self.P
        if t[0] == 'ret':
            v = t[1]
            if self.returns:
                if v is None:
                    out.append((ind, 'return;'))
                elif self.L.ret_struct is not None and v.k == 'pair':
                    ax_ = v.a[1]
                    if not (ax_.k == 'local' and ax_.a[0] == self.L.ret_struct):
                        out.append((ind, 'r.a = %s;' % P.ex(ax_)[0]))
                    out.append((ind, 'return r;'))
                else:
                    out.append((ind, 'return %s;' % P.ex(v)[0]))
            elif not last or ind > 1:
                out.append((ind, 'return;'))
            return
        if t[0] == 'goto':
            self.transfer(inst, t[1], nxt, out, ind, fall=True)
            return
        if t[0] == 'if':
            c, tgt, fall = t[1], t[2], t[3]
            j = self.inst_for(inst, tgt)
            if j is not None:
                out.append((ind, 'if (%s) {' % P.cond(c)))
                self.emit_inst(j, None, out, ind + 1)
                out.append((ind, '}'))
            else:
                out.append((ind, 'if (%s) %s' % (P.cond(c), self.jump_text(tgt))))
            self.transfer(inst, fall, nxt, out, ind, fall=True)
            return
        raise DecompileError('terminator %s' % t[0])

    def jump_text(self, tgt):
        m = self.main_inst(tgt)
        if m is not None and m['term'] and m['term'][0] == 'ret' and not m['stmts'] and m['term'][1] is None:
            return 'return;'
        return 'goto L_%04x;' % (tgt - self.fn.frame * 16)

    def transfer(self, inst, tgt, nxt, out, ind, fall):
        j = self.inst_for(inst, tgt)
        if j is not None:
            self.emit_inst(j, nxt, out, ind)
            return
        if tgt == nxt and ind == 1:
            return
        out.append((ind, self.jump_text(tgt)))



# =============================================================================== structuring
class Node:
    __slots__ = ('id', 'key', 'stmts', 'term', 'preds', 'addr')

    def __init__(self, nid, key, addr, stmts, term):
        self.id, self.key, self.addr, self.stmts, self.term = nid, key, addr, stmts, term
        self.preds = []


def cnot(c):
    if c.k == 'cmp':
        r = E('cmp', NEG[c.a[0]], c.a[1], c.a[2])
        r.uns = c.uns
        return r
    if c.k == 'not':
        return c.a[0]
    if c.k == 'and':
        return E('or', cnot(c.a[0]), cnot(c.a[1]))
    if c.k == 'or':
        return E('and', cnot(c.a[0]), cnot(c.a[1]))
    return E('not', c)


def ceval(e, subj, v):
    """Evaluate expression e with subject := v (16-bit). None if not computable."""
    if e.key() == subj:
        return v & 0xffff
    k = e.k
    if k == 'const':
        return e.a[0]
    if k == 'cast' and e.a[0] in ('char', 'unsigned char'):
        x = ceval(e.a[1], subj, v)
        return None if x is None else x & 0xff
    if k in ('sext',):
        x = ceval(e.a[0], subj, v)
        return None if x is None else ((x & 0xff) ^ 0x80) - 0x80 & 0xffff
    if k == 'zext':
        x = ceval(e.a[0], subj, v)
        return None if x is None else x & 0xff
    if k == 'bin':
        op = e.a[0]
        a = ceval(e.a[1], subj, v)
        b = ceval(e.a[2], subj, v)
        if a is None or b is None:
            return None
        m = 0xff if e.size == 1 else 0xffff
        if op == '+':
            return (a + b) & m
        if op == '-':
            return (a - b) & m
        if op == '&':
            return a & b
        if op == '|':
            return a | b
        if op == '^':
            return a ^ b
        if op == '<<':
            return (a << b) & m
        return None
    return None


def ccmp(op, a, b, size):
    if a is None or b is None:
        return None
    bits = 8 if size == 1 else 16
    m = (1 << bits) - 1
    a &= m
    b &= m
    if not op.endswith('u') and op not in ('==', '!='):
        a = a - (1 << bits) if a >> (bits - 1) else a
        b = b - (1 << bits) if b >> (bits - 1) else b
    o = op.rstrip('u')
    return {'==': a == b, '!=': a != b, '<': a < b, '<=': a <= b, '>': a > b, '>=': a >= b}[o]


def cond_eval(c, subj, v):
    if c.k == 'cmp':
        op, a, b = c.a
        x = ceval(a, subj, v)
        y = ceval(b, subj, v)
        size = 1 if (a.size == 1 or (a.k == 'cast' and a.a[0] in ('char', 'unsigned char'))) else 2
        return ccmp(op, x, y, size)
    return None


def base_subject(e):
    """Strip constant arithmetic / narrowing casts to find the switched value."""
    while True:
        if e.k == 'bin' and e.a[0] in ('+', '-') and is_const(e.a[2]):
            e = e.a[1]
        elif e.k == 'cast' and e.a[0] in ('char', 'unsigned char'):
            e = e.a[1]
        else:
            return e


class Structurer:
    def __init__(self, FE):
        self.FE = FE
        self.L = FE.L
        self.P = FE.P
        self.fn = FE.fn

    # ------------------------------------------------------------ node graph
    def build(self):
        L = self.L
        self.nodes = {}
        main = {}
        for inst in L.instances:
            if not inst['inline']:
                main[inst['block']] = inst
        self.nid_of_main = {}
        for b, inst in main.items():
            n = Node(len(self.nodes), (b, 0, 0), b, list(inst['stmts']), None)
            self.nodes[n.id] = n
            self.nid_of_main[b] = n.id
        self.inst_of = {self.nid_of_main[b]: main[b] for b in main}
        for b, inst in main.items():
            n = self.nodes[self.nid_of_main[b]]
            n.term = self.resolve_term(n, inst)
        for n in list(self.nodes.values()):
            for s in self.succs(n):
                self.nodes[s].preds.append(n.id)
        # unreachable nodes (e.g. jump-table padding) are dropped
        entry = self.nid_of_main[self.fn.start]
        seen = set()
        work = [entry]
        while work:
            x = work.pop()
            if x in seen:
                continue
            seen.add(x)
            work.extend(self.succs(self.nodes[x]))
        for x in list(self.nodes):
            if x not in seen:
                del self.nodes[x]
        for n in self.nodes.values():
            n.preds = [p for p in n.preds if p in self.nodes]
        self.entry = entry

    def inline_of(self, inst, block):
        for j in self.L.by_block.get(block, []):
            if j['inline'] and j['src'] is inst:
                return j
        return None

    def resolve_term(self, n, inst):
        """Terminator of instance inst in node ids; unconditional transfers into inline instances are merged."""
        t = inst['term']
        if t[0] == 'ret':
            return ('ret', t[1])
        if t[0] == 'goto':
            j = self.inline_of(inst, t[1])
            if j is not None:
                n.stmts.extend(j['stmts'])
                return self.resolve_term(n, j)
            return ('goto', self.target(t[1]))
        if t[0] == 'if':
            return ('if', t[1], self.edge(inst, t[2]), self.edge(inst, t[3]))
        if t[0] == 'table':
            return ('table', t[1], [self.edge(inst, x) for x in t[2]])
        raise DecompileError('terminator %s' % t[0])

    def edge(self, inst, block):
        j = self.inline_of(inst, block)
        if j is None:
            return self.target(block)
        n = Node(len(self.nodes) + 1000, (block, 1, len(self.nodes)), block, list(j['stmts']), None)
        self.nodes[n.id] = n
        n.term = self.resolve_term(n, j)
        return n.id

    def target(self, block):
        if block not in self.nid_of_main:
            raise DecompileError('jump to missing block %x' % block)
        return self.nid_of_main[block]

    def succs(self, n):
        t = n.term
        if t[0] == 'goto':
            return [t[1]]
        if t[0] == 'if':
            return [t[2], t[3]]
        if t[0] == 'table':
            return list(dict.fromkeys(t[2]))
        if t[0] == 'switch':
            return list(dict.fromkeys(list(t[2].values()) + [t[3]]))
        return []

    def visible(self, n):
        """Statements that produce C text (consumed call placeholders do not)."""
        return [s for s in n.stmts if not (s[0] == 'call' and s[1].sid in self.FE.used)]

    def relink(self):
        for n in self.nodes.values():
            n.preds = []
        for n in self.nodes.values():
            for s in self.succs(n):
                self.nodes[s].preds.append(n.id)

    # ------------------------------------------------------------ switch detection
    def find_switches(self):
        changed = True
        while changed:
            changed = False
            for n in sorted(self.nodes.values(), key=lambda x: x.key):
                if n.id not in self.nodes or n.term[0] not in ('if', 'table'):
                    continue
                if self.try_switch(n):
                    changed = True
                    self.relink()
                    break

    def try_switch(self, root):
        t = root.term
        if t[0] == 'if':
            c = t[1]
            if c.k != 'cmp' or not is_const(c.a[2]):
                return False
            subj = base_subject(c.a[1])
        else:
            if t[1] is None:
                return False
            subj = base_subject(base_index(t[1]))
        if subj.k == 'const':
            return False
        sk = subj.key()
        tree = {root.id}
        leaves = set()
        uses_arith = False
        work = list(self.succs(root))
        while work:
            x = work.pop()
            n = self.nodes[x]
            ok = n.term[0] in ('if', 'table') and not self.visible(n) and \
                all(p in tree for p in n.preds) and x not in tree
            if ok and n.term[0] == 'if':
                c = n.term[1]
                ok = c.k == 'cmp' and is_const(c.a[2]) and base_subject(c.a[1]).key() == sk
                if ok and (c.a[1].k == 'bin' or c.uns == 'arith'):
                    uses_arith = True
            elif ok:
                ok = n.term[1] is not None and base_subject(base_index(n.term[1])).key() == sk
            if ok:
                tree.add(x)
                work.extend(self.succs(n))
            else:
                leaves.add(x)
        leaves -= tree
        has_table = any(self.nodes[x].term[0] == 'table' for x in tree)
        if t[0] == 'if' and (root.term[1].a[1].k == 'bin' or root.term[1].uns == 'arith'):
            uses_arith = True
        if len(tree) < 2 and not has_table:
            return False
        if not (has_table or uses_arith or len(leaves) >= 3):
            return False
        if not (has_table or uses_arith) and subj.k in ('reg', 'local', 'g', 'gidx', 'far'):
            return False        # compares on a variable: an if-chain
        # evaluate the tree for candidate values
        consts = set()
        for x in tree:
            n = self.nodes[x]
            if n.term[0] == 'if':
                walk(n.term[1], lambda e: consts.add(sconst(e)) if e.k == 'const' else None)
        cand = set(range(-64, 512))
        for c in list(consts):
            for d in range(-40, 41):
                cand.add(c + d)
        result = {}
        stubs = set()
        for v in sorted(cand):
            x = root.id
            steps = 0
            while x in tree and steps < 100:
                steps += 1
                n = self.nodes[x]
                if n.term[0] == 'if':
                    r = cond_eval(n.term[1], sk, v)
                    if r is None:
                        return False
                    x = n.term[2] if r else n.term[3]
                else:
                    idx = ceval(base_index(n.term[1]), sk, v)
                    if idx is None:
                        return False
                    tg = n.term[2]
                    if idx >= len(tg):
                        return False        # bounds were checked before; unreachable
                    x = tg[idx]
            # thread through empty jump stubs (MSC's 'jmp default' trampolines)
            seen_ = set()
            while x not in tree and x not in seen_:
                seen_.add(x)
                n2 = self.nodes[x]
                if n2.term[0] == 'goto' and not self.visible(n2) and all(p in tree for p in n2.preds):
                    stubs.add(x)
                    x = n2.term[1]
                else:
                    break
            result[v] = x
        cnt = collections.Counter(result.values())
        # default = target of values far from every case
        far_v = [v for v in result if all(abs(v - c) > 45 for c in consts)]
        default = collections.Counter(result[v] for v in far_v).most_common(1)[0][0] if far_v else cnt.most_common(1)[0][0]
        cases = {v: x for v, x in result.items() if x != default}
        if not cases:
            return False
        for x in tree:
            if x != root.id:
                del self.nodes[x]
        for x in stubs:
            if x in self.nodes and x not in cases.values() and x != default:
                del self.nodes[x]
        root.term = ('switch', subj, cases, default)
        return True

    # ------------------------------------------------------------ short-circuit conditions
    def merge_conditions(self):
        changed = True
        while changed:
            changed = False
            order = sorted(self.nodes.values(), key=lambda x: x.key)
            pos = {n.id: k for k, n in enumerate(order)}
            for n in order:
                if n.term[0] != 'if':
                    continue
                c1, t1, f1 = n.term[1], n.term[2], n.term[3]
                m = self.nodes.get(f1)
                if m is None or m.term[0] != 'if' or self.visible(m) or m.preds != [n.id]:
                    continue
                if pos.get(f1) != pos[n.id] + 1:
                    continue
                c2, t2, f2 = m.term[1], m.term[2], m.term[3]
                if t2 == t1:
                    n.term = ('if', E('or', c1, c2), t1, f2)
                elif f2 == t1:
                    n.term = ('if', E('and', cnot(c1), c2), t2, f2)
                else:
                    continue
                n.stmts.extend(m.stmts)
                del self.nodes[m.id]
                self.relink()
                changed = True
                break

    # ------------------------------------------------------------ post-dominators
    def ipdom(self):
        nodes = list(self.nodes)
        EXIT = -1
        succ = {x: (self.succs(self.nodes[x]) or [EXIT]) for x in nodes}
        pd = {x: set(nodes) | {EXIT} for x in nodes}
        pd[EXIT] = {EXIT}
        changed = True
        while changed:
            changed = False
            for x in nodes:
                s = set.intersection(*[pd[y] for y in succ[x]]) | {x}
                if s != pd[x]:
                    pd[x] = s
                    changed = True
        ip = {}
        for x in nodes:
            cands = pd[x] - {x}
            # immediate = the candidate post-dominated by all others
            best = None
            for c in cands:
                if all(c == o or o in pd[c] for o in cands):
                    best = c
            ip[x] = best
        return ip

    # ------------------------------------------------------------ main
    def collapse_long_jumps(self):
        """'jcc over a near jmp' (the 286 has only short conditional jumps) -> one inverted branch."""
        order = sorted(self.nodes.values(), key=lambda x: x.key)
        for k, n in enumerate(order[:-2]):
            if n.term[0] != 'if':
                continue
            c, tg, fl = n.term[1], n.term[2], n.term[3]
            m = self.nodes.get(fl)
            if m is None or m.id != order[k + 1].id or m.term[0] != 'goto' or self.visible(m) or m.preds != [n.id]:
                continue
            if tg != order[k + 2].id:
                continue
            n.term = ('if', cnot(c), m.term[1], tg)
            n.stmts.extend(m.stmts)
            del self.nodes[m.id]
            self.relink()
            return self.collapse_long_jumps()

    def run(self):
        self.build()
        self.collapse_long_jumps()
        self.find_switches()
        self.merge_conditions()
        self.order = sorted(self.nodes, key=lambda x: self.nodes[x].key)
        self.pos = {x: k for k, x in enumerate(self.order)}
        self.ip = self.ipdom()
        self.gotos = set()
        self.emitted = set()
        self.emitted_labels_needed = set()
        body = self.region(0, len(self.order), {'follow': None, 'brk': None, 'cont': None})
        return body

    def region(self, lo, hi, ctx):
        out = []
        i = lo
        order, pos, nodes = self.order, self.pos, self.nodes
        while i < hi:
            nid = order[i]
            n = nodes[nid]
            # loop header?
            back = [p for p in n.preds if pos[p] >= i and pos[p] < hi]
            if back and not ctx.get('in_loop_of') == nid:
                j = max(pos[p] for p in back)
                last = nodes[order[j]]
                exit_ = order[j + 1] if j + 1 < len(order) else None
                lctx = {'follow': nid, 'brk': exit_, 'cont': nid, 'in_loop_of': nid}
                saved = last.term
                if saved[0] == 'if' and saved[2] == nid:
                    kind, cond = 'do', saved[1]
                    last.term = ('loopend', saved[3])
                    lctx['cont'] = order[j] if not self.visible(last) and j > i else None
                elif saved[0] == 'if' and saved[3] == nid:
                    kind, cond = 'do', cnot(saved[1])
                    last.term = ('loopend', saved[2])
                    lctx['cont'] = order[j] if not self.visible(last) and j > i else None
                elif saved[0] == 'goto' and saved[1] == nid:
                    kind, cond = 'for', None
                    last.term = ('loopend', None)
                else:
                    raise DecompileError('loop shape')
                if lctx['cont'] is not None and lctx['cont'] != nid:
                    pass
                inner = self.region(i, j + 1, lctx)
                last.term = saved
                out.append(('loop', kind, cond, inner))
                i = j + 1
                continue
            self.emitted.add(nid)
            out.append(('label', nid))
            for s in self.visible(n):
                out.append(('stmt', s))
            t = n.term
            nxt = order[i + 1] if i + 1 < hi else ctx['follow']
            if t[0] == 'ret':
                out.append(('ret', t[1]))
                i += 1
                continue
            if t[0] == 'loopend':
                i += 1
                continue
            if t[0] == 'goto':
                out.extend(self.jump(t[1], nxt, ctx))
                i += 1
                continue
            if t[0] == 'if':
                c, tg, fl = t[1], t[2], t[3]
                k = pos.get(tg)
                if fl == (order[i + 1] if i + 1 < len(order) else None) and k is not None and k > i + 1 and k <= hi and \
                        tg not in (ctx.get('brk'), ctx.get('cont')) and self.enclosed(i + 1, k, nid):
                    lastn = nodes[order[k - 1]]
                    if lastn.term[0] == 'goto' and pos.get(lastn.term[1], -1) > k and \
                            (pos[lastn.term[1]] <= hi or lastn.term[1] == ctx['follow']) and \
                            lastn.term[1] not in (ctx.get('brk'),) and self.enclosed(k, pos[lastn.term[1]], nid):
                        e = lastn.term[1]
                        ectx = dict(ctx, follow=e)
                        then = self.region(i + 1, k, ectx)
                        els = self.region(k, min(pos[e], hi), ectx)
                        out.append(('if', cnot(c), then, els))
                        i = min(pos[e], hi)
                        continue
                    then = self.region(i + 1, k, dict(ctx, follow=tg))
                    out.append(('if', cnot(c), then, None))
                    i = k
                    continue
                # conditional jump that is a break / continue / return / goto
                out.append(('if', c, self.jump(tg, None, ctx), None))
                out.extend(self.jump(fl, nxt, ctx))
                i += 1
                continue
            if t[0] == 'switch':
                subj, cases, default = t[1], t[2], t[3]
                f = self.ip.get(nid)
                end = pos[f] if f is not None and f in pos else hi
                end = min(end, hi)
                targets = set(cases.values()) | {default}
                body_lo = i + 1
                sctx = dict(ctx, follow=f, brk=f)
                # case labels by position
                labels = collections.defaultdict(list)
                for v, x in cases.items():
                    labels[x].append(v)
                if default != f:
                    labels[default].append('default')
                arms = []
                for x in sorted(labels, key=lambda y: pos.get(y, 1 << 30)):
                    if pos.get(x, 1 << 30) < body_lo or pos.get(x, 1 << 30) >= end:
                        raise DecompileError('switch case outside body')
                    arms.append((x, labels[x]))
                items = []
                for k2, (x, vals) in enumerate(arms):
                    a0 = pos[x]
                    a1 = pos[arms[k2 + 1][0]] if k2 + 1 < len(arms) else end
                    items.append((vals, self.region(a0, a1, dict(sctx, follow=arms[k2 + 1][0] if k2 + 1 < len(arms) else f))))
                if body_lo < len(order) and arms and pos[arms[0][0]] != body_lo:
                    raise DecompileError('switch body does not start with a case')
                out.append(('switch', subj, items))
                i = end
                if f is not None and f in pos and pos[f] >= hi and f != ctx['follow']:
                    out.extend(self.jump(f, None, ctx))
                continue
            if t[0] == 'table':
                raise DecompileError('jump table not in a switch')
            raise DecompileError('terminator %s' % t[0])
        return out

    def enclosed(self, lo, hi, head, extra=()):
        """Nodes in [lo, hi) are entered only from inside the range or from head."""
        order, pos = self.order, self.pos
        inside = set(order[lo:hi])
        for x in inside:
            for p in self.nodes[x].preds:
                if p in inside or p == head:
                    continue
                return False
        return True

    def jump(self, tgt, nxt, ctx):
        # thread through empty nodes (shared stack cleanup etc.)
        seen = set()
        while tgt not in seen and tgt != nxt and tgt not in (ctx.get('brk'), ctx.get('cont')):
            seen.add(tgt)
            n = self.nodes[tgt]
            if not self.visible(n) and n.term[0] == 'goto' and n.id not in self.emitted_labels_needed:
                tgt = n.term[1]
            else:
                break
        if tgt == nxt:
            return []
        if tgt == ctx.get('brk'):
            return [('break',)]
        if tgt == ctx.get('cont'):
            return [('continue',)]
        n = self.nodes[tgt]
        if n.term[0] == 'ret' and not self.visible(n) and (n.term[1] is None or not self.FE.returns):
            return [('ret', None)]
        if tgt == ctx.get('follow') and nxt is None:
            return []
        self.gotos.add(tgt)
        return [('goto', tgt)]


def base_index(e):
    """Jump-table index expression: (x << 1) or x + x -> x."""
    if e.k == 'bin' and e.a[0] == '<<' and is_const(e.a[2], 1):
        return e.a[1]
    if e.k == 'bin' and e.a[0] == '+' and e.a[1].eq(e.a[2]):
        return e.a[1]
    return e


# =============================================================================== file assembly
def cname(n):
    return n[1:] if n.startswith('_') and not n.startswith('__') else (n[1:] if n.startswith('__') else n)


INTRINSIC_PROTOS = {
    'strcpy': 'char *strcpy(char *, const char *);',
    'memset': 'void *memset(void *, int, unsigned);',
    'strlen': 'unsigned strlen(const char *);',
}


def _fd_methods():
    def gsize(self, off):
        g = self.globals.get(off)
        if not g:
            return 2
        return max(g['sizes']) if g['sizes'] else 1

    def is_array(self, off):
        g = self.globals.get(off)
        return bool(g and (g['array'] or g['addr']))

    def gtype(self, off):
        g = self.globals.get(off)
        ty = self.gkinds.get(off)
        if ty:
            return ty
        return CTYPE.get(self.gsize(off), 'int')

    def farname(self, slot):
        fr = self.fars[slot]
        segs = sorted(s for s, f in self.fars.items() if f['seg'] == fr['seg'])
        k = segs.index(slot)
        return 'F_%04x_0000' % fr['seg'] + ('' if k == 0 else '_%d' % k)

    def expr_unsigned(self, e):
        if e.k == 'bin' and e.a[0].endswith('u'):
            return True
        if e.k == 'cast' and e.a[0].startswith('unsigned'):
            return True
        return False

    def code_or_data_at(self, seg, off):
        lin = seg * 16 + off
        if seg in self.prog.data_segs and seg != DG:
            nm = 'F_%04x_%04x' % (seg, off)
            self.farabs[(seg, off)] = True
            return nm
        n = self.prog.addr2name.get((self.img, lin)) or self.prog.name_at_any(lin, seg)
        if n is None:
            raise DecompileError('far constant %04x:%04x' % (seg, off))
        self.fnrefs.add(n)
        return cname(n)
    def arr_elem(self, base):
        a = self.arrays[base]
        sizes = {sz for sz, kind in a['fields'].values()}
        return max(sizes) if sizes else 1

    def array_of(self, addr):
        """(base, index expression text parts) if addr lies inside a loop-filled array."""
        for base, a in self.arrays.items():
            ext = a['n'] * a['stride']
            if base <= addr < base + ext:
                es = self.arr_elem(base)
                rel = addr - base
                row, col = divmod(rel, a['stride'])
                if a['stride'] == es:
                    return base, '[%d]' % row
                if col % es:
                    return None
                return base, '[%d][%d]' % (row, col // es)
        return None

    def arr_decl(self, base):
        a = self.arrays[base]
        es = self.arr_elem(base)
        kinds = {kind for sz, kind in a['fields'].values()}
        if es == 4 and 'fptr_const' in kinds:
            ty = 'fnptr'
        else:
            ty = CTYPE.get(es, 'int')
        dims = '[%d]' % a['n'] if a['stride'] == es else '[%d][%d]' % (a['n'], a['stride'] // es)
        if ty == 'fnptr':
            return '%s (far *D_%04x%s)()' % (self.fnptr_ret(base, self._rtypes), base, dims)
        return '%s D_%04x%s' % (ty, base, dims)

    def fnptr_ret(self, base, rtypes):
        if base in self.fnret:
            return self.fnret[base]
        # the functions stored into it decide
        rs = {rtypes.get(n) for n in self.fnstores.get(base, ())}
        if rs == {'struct res'}:
            return 'struct res'
        return 'void'
    for f in (gsize, is_array, gtype, farname, expr_unsigned, code_or_data_at, arr_elem, array_of, arr_decl,
              fnptr_ret):
        setattr(FileDec, f.__name__, f)


_fd_methods()


def find_struct_temps(fd, fn, L):
    """Locals that receive a 4-byte (struct) call result of which only the first word is kept:
    the frame is larger than the locals that are accessed."""
    neg = {o: i for o, i in L.locals.items() if o < 0}
    used = 0
    for o, i in neg.items():
        if L.ret_struct is not None and o in (L.ret_struct, L.ret_struct + 2):
            continue
        used += max(i['size'], 1) if not i.get('array') else 0
    arrays = [o for o, i in neg.items() if i.get('array')]
    if arrays:
        return
    accounted = used + (4 if L.ret_struct is not None else 0)
    slack = fn.frame_size - accounted
    if slack < 2:
        return
    srcs = collections.defaultdict(list)
    for inst in L.instances:
        for st in inst['stmts']:
            if st[0] == 'assign' and st[1].k == 'local' and st[1].a[0] < 0:
                srcs[st[1].a[0]].append(st[2])
    for o in sorted(neg):
        if slack < 2:
            break
        if neg[o]['size'] != 2 or o + 2 in neg or o + 2 >= 0 and o + 2 != 0:
            continue
        if L.ret_struct is not None and o + 2 == L.ret_struct:
            continue
        v = srcs.get(o)
        if v and all(x.k in ('call', 'icall') for x in v):
            L.struct_locals.add(o)
            slack -= 2
            for x in v:
                if x.k == 'icall':
                    t = x.a[0]
                    if t.k in ('gidx', 'g'):
                        fd.fnret[t.a[0]] = 'struct res'
                else:
                    fd.sret_callees.add(x.a[0])


def note_fnstore(fd, base, v):
    seg, off = v.a
    off = off.a[0] if isinstance(off, E) else off
    n = fd.prog.addr2name.get((fd.img, seg * 16 + off)) or fd.prog.name_at_any(seg * 16 + off, seg)
    if n:
        # a store into an element of a table counts for the table (D_ea18 is D_ea14[1])
        fd.fnstores[base].add(n)


def type_evidence(fd, L):
    """Refine global types from how values are used (function pointers, far pointers)."""
    def visit(e):
        if e.k == 'icall':
            t = e.a[0]
            if t.k == 'gidx':
                fd.gkinds[t.a[0]] = 'fnptr'
            elif t.k == 'g':
                fd.gkinds[t.a[0]] = 'fnptr'
        if e.k == 'fderef':
            p = e.a[0]
            if p.k == 'g':
                fd.gkinds[p.a[0]] = 'fptr'
    for inst in L.instances:
        for s in inst['stmts']:
            for x in s[1:]:
                walk(x, visit)
            if s[0] == 'assign' and s[1].k in ('g', 'gidx') and s[2].k == 'fptr_const':
                seg = s[2].a[0]
                if seg not in fd.prog.data_segs:
                    fd.gkinds[s[1].a[0]] = 'fnptr'
                    note_fnstore(fd, s[1].a[0], s[2])
                else:
                    fd.gkinds[s[1].a[0]] = 'fptr'
            if s[0] == 'arrloop':
                for kind, base, stride, fo, size, v in s[3]:
                    if v.k == 'fptr_const':
                        note_fnstore(fd, base, v)
        t = inst['term']
        if t:
            for x in t[1:]:
                walk(x, visit)


def decl_of(ty, name, array=False, ret='void'):
    if ty == 'fnptr':
        return '%s (far *%s%s)()' % (ret, name, '[]' if array else '')
    if ty == 'fptr':
        return 'char far *%s%s' % (name, '[]' if array else '')
    return '%s %s%s' % (ty, name, '[]' if array else '')


def decompile_file(prog, img, frame, only=None):
    fd = FileDec(prog, img, frame)
    fd.fnret = {}
    fd.fnstores = collections.defaultdict(set)
    fd.sret_callees = set()
    fd._rtypes = {}
    fd.gkinds = {}
    fd.fnrefs = set()
    fd.copy_sizes = set()
    funcs = []
    fd.failed = {}
    for name, s, e in fd.funcs:
        fn = Func(prog, img, frame, name, s, e, fd.funcs)
        try:
            L = FuncLifter(fd, fn).lift()
        except DecompileError as ex:
            fd.failed[name] = str(ex)
            funcs.append((name, fn, None, False))
            continue
        returns = function_returns(fn, prog)
        # struct return: every returned value is (dx=[bp+o+2], ax=[bp+o])
        rs = set()
        rets = [inst['term'][1] for inst in L.instances if inst['term'] and inst['term'][0] == 'ret']
        for v in rets:
            if v is not None and v.k == 'pair' and v.a[0].k == 'local' and v.a[0].a[0] < 0:
                rs.add(v.a[0].a[0] - 2)
        if len(rs) == 1 and returns:
            L.ret_struct = rs.pop()
        for inst in L.instances:
            inst['stmts'] = combine_far_stores(inst['stmts'])
        find_struct_temps(fd, fn, L)
        type_evidence(fd, L)
        funcs.append((name, fn, L, returns))
    prune_guessed_literals(fd, [f[2] for f in funcs if f[2] is not None])
    collect_usage(fd, [f[2] for f in funcs if f[2] is not None])
    # emit bodies
    bodies = []
    for name, fn, L, returns in funcs:
        if L is None:
            bodies.append((name, fn, None, False, None))
            continue
        try:
            FE = FuncEmitter(fd, fn, L, returns)
            lines = FE.body()
        except DecompileError as ex:
            fd.failed[name] = str(ex)
            bodies.append((name, fn, None, False, None))
            continue
        bodies.append((name, fn, L, returns, lines))
    return fd, bodies


def prune_guessed_literals(fd, lifters):
    """A pushed constant taken for a string literal must lie in the file's literal pool."""
    sure, guessed = set(), []

    def visit(e):
        if e.k == 'str':
            if len(e.a) > 2:
                guessed.append(e)
            else:
                sure.add(e.a[0])
    for L in lifters:
        for inst in L.instances:
            for st in inst['stmts']:
                for x in st[1:]:
                    walk(x, visit)
            t = inst['term']
            if t:
                for x in t[1:]:
                    walk(x, visit)
    pool = sorted(sure)
    for e in guessed:
        v = e.a[0]
        near = any(abs(v - p) < 0x100 for p in pool) or any(abs(v - g.a[0]) < 0x40 and g is not e for g in guessed)
        if not near:
            e.k, e.a = 'const', (v,)


def collect_usage(fd, lifters):
    """Declared sizes of globals / far fields from the final expression trees."""
    old = fd.globals
    fd.globals = {}
    for slot in fd.fars:
        fd.fars[slot]['fields'] = {}

    def g(off):
        return fd.globals.setdefault(off, {'sizes': collections.Counter(), 'uns': collections.Counter(),
                                           'array': None, 'addr': False, 'elem': None})

    def visit(e, under_addr=False):
        if not isinstance(e, E):
            if isinstance(e, list):
                for x in e:
                    visit(x)
            return
        k = e.k
        if k == 'addr':
            inner = e.a[0]
            if inner.k == 'g':
                g(inner.a[0])['addr'] = True
                return
            visit(inner, True)
            return
        if k == 'g':
            g(e.a[0])['sizes'][e.size] += 1
        elif k == 'gidx':
            x = g(e.a[0])
            x['sizes'][e.size] += 1
            x['array'] = e.a[2]
        elif k == 'far':
            f = fd.fars[e.a[0]]['fields']
            f[e.a[2]] = max(f.get(e.a[2], 0), e.size)
        for x in e.a:
            if isinstance(x, (E, list)):
                visit(x)
    for L in lifters:
        for inst in L.instances:
            for st in inst['stmts']:
                for x in st[1:]:
                    visit(x)
            t = inst['term']
            if t:
                for x in t[1:]:
                    visit(x)
    # drop far fields hidden inside wider ones
    for slot, fr in fd.fars.items():
        fl = sorted(fr['fields'].items())
        keep = {}
        end = -1
        for d, sz in fl:
            if d < end:
                continue
            keep[d] = sz
            end = d + sz
        fr['fields'] = keep


def ret_type(L, returns):
    if not returns:
        return 'void'
    if L.ret_struct is not None:
        return 'struct res'
    for inst in L.instances:
        t = inst['term']
        if t and t[0] == 'ret' and t[1] is not None and t[1].k == 'pair':
            return 'long'
    return 'int'


def render_file(prog, img, frame, fd, bodies, stubs=()):
    if stubs:
        bodies = [(n, fn, None, False, None) if n in stubs else (n, fn, L, r, ln) for n, fn, L, r, ln in bodies]
        for n in stubs:
            fd.failed.setdefault(n, 'does not compile')
    out = []
    first = fd.funcs[0][1]
    out.append('/* place: %s %04x:%04x */' % (img, frame, first - frame * 16))
    out.append('/* flags: /AM /G2 /Gs /Ox /Zp1 */')
    out.append('/* generated by tools/dec16.py from the original code; matches when tools/fmatch.py says so */')
    out.append('')
    out.append('struct res { int a; int b; };')
    for n in sorted(fd.copy_sizes):
        out.append('struct blk%d { char b[%d]; };' % (n, n))
    rtypes = {}
    for name, fn, L, returns, lines in bodies:
        rtypes[name] = ret_type(L, returns) if L is not None else 'void'
    # far data structs
    for slot in sorted(fd.fars):
        fr = fd.fars[slot]
        nm = fd.farname(slot)
        fields = sorted(fr['fields'].items())
        if not fields:
            continue
        parts = []
        pos = 0
        for disp, size in fields:
            if disp < pos:
                raise DecompileError('overlapping far fields in %s' % nm)
            if disp > pos:
                parts.append('char _p%04x[0x%x];' % (pos, disp - pos))
            parts.append('%s f_%04x;' % (CTYPE[size] if size != 4 else 'char far *', disp))
            pos = disp + size
        out.append('struct %s_t { %s };' % (nm, ' '.join(parts)))
        out.append('extern struct %s_t far %s;' % (nm, nm))
    for (seg, off) in sorted(fd.farabs):
        out.append('extern char far F_%04x_%04x[];' % (seg, off))
    # globals
    fd._rtypes = rtypes
    for base in sorted(fd.arrays):
        out.append('extern %s;' % fd.arr_decl(base))
    for off in sorted(fd.globals):
        if fd.array_of(off) is not None or off in fd.arrays:
            continue
        g = fd.globals[off]
        ty = fd.gtype(off)
        out.append('extern %s;' % decl_of(ty, 'D_%04x' % off, fd.is_array(off), fd.fnptr_ret(off, rtypes)))
    # callees
    local = {n for n, s, e in fd.funcs}
    for n in sorted(fd.callees):
        if n in local:
            continue
        if n in INTRINSIC_PROTOS:
            out.append(INTRINSIC_PROTOS[n])
            continue
        out.append('%s far %s();' % ('struct res' if n in fd.sret_callees else 'int', cname(n)))
    for n in sorted(fd.fnrefs - local - set(fd.callees)):
        out.append('int far %s();' % cname(n))
    for name, fn, L, returns, lines in bodies:
        out.append('%s far %s();' % (rtypes[name], name))
    out.append('')
    for name, fn, L, returns, lines in bodies:
        if L is None:
            out.append('/* %s: not decompiled: %s */' % (name, fd.failed.get(name, '').replace('*/', '* /')))
            out.append('void far %s(void) { }' % name)
            out.append('')
            continue
        params = []
        pos = 6
        pl = sorted(o for o in L.locals if o > 0)
        maxp = max([o + max(L.locals[o]['size'], 2) for o in pl] or [6])
        while pos < maxp:
            sz = L.locals.get(pos, {}).get('size', 2) or 2
            params.append('%s p_%x' % ({1: 'char', 2: 'int', 4: 'long'}.get(sz, 'int'), pos))
            pos += 2 if sz <= 2 else sz
        out.append('%s far %s(%s)' % (rtypes[name], name, ', '.join(params) if params else 'void'))
        out.append('{')
        decls = []
        if L.ret_struct is not None:
            decls.append('struct res r;')
        neg = sorted(o for o in L.locals if o < 0)
        for k, o in enumerate(neg):
            if L.ret_struct is not None and o in (L.ret_struct, L.ret_struct + 2):
                continue
            info = L.locals[o]
            if o in L.struct_locals:
                continue
            if info.get('array'):
                above = [x for x in neg if x > o and not (L.ret_struct is not None and x in (L.ret_struct, L.ret_struct + 2))]
                if L.ret_struct is not None and L.ret_struct > o:
                    above.append(L.ret_struct)
                size = L.zinit.get(o) or ((min(above) - o) if above else -o)
                init = ' = {0}' if o in L.zinit else ''
                decls.append('char l_%x[%d]%s;' % (-o, size, init))
            else:
                decls.append('%s l_%x;' % (CTYPE.get(info['size'] or 2, 'int'), -o))
        for o in sorted(L.struct_locals):
            decls.append('struct res l_%x;' % -o)
        for r in sorted(L.regvars):
            decls.append('int v_%s;' % r)
        for v in L.loopvars:
            decls.append('int %s;' % v)
        for d in decls:
            out.append('    ' + d)
        for ind, t in lines:
            out.append('    ' * ind + t if ind else t)
        out.append('}')
        out.append('')
    return '\n'.join(out)


def main():
    import argparse
    ap = argparse.ArgumentParser()
    ap.add_argument('image')
    ap.add_argument('frame')
    ap.add_argument('-o')
    a = ap.parse_args()
    prog = Prog()
    fr = int(a.frame, 16)
    fd, bodies = decompile_file(prog, a.image, fr)
    text = render_file(prog, a.image, fr, fd, bodies)
    if a.o:
        open(a.o, 'w').write(text)
    else:
        print(text)


if __name__ == '__main__':
    main()
