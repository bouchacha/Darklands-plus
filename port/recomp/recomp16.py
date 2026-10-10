#!/usr/bin/env python3
"""
recomp16.py - static recompiler: 16-bit x86 real-mode code of DARKLAND.EXE -> C.

  python3 -I port/recomp/recomp16.py <DARKLAND.EXE> <analysis.pickle> <out_dir>

Every function found by tools/analyze.py becomes one C function that operates on the machine
state in port/runtime/cpu.h. Control flow inside a function becomes labels and gotos; calls
become C calls (the return address is still pushed on the emulated stack, so stack layouts,
BP frames and code that inspects the stack behave exactly as in the original). Indirect calls
and jumps go through the runtime dispatcher. Calls through RTLink overlay vectors load the
target overlay section (port/runtime/ovl.c) before calling the target directly.

Output: one C file per image (root, s000 .. s135), funcs.h with prototypes, and dispatch.c
with the address -> function table.
"""
import sys, os, struct, pickle, collections, re
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
from rtlink import RTLinkExe
import capstone
from capstone import x86 as X

MD = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_16)
MD.detail = True

REG16 = {X.X86_REG_AX: 'AX', X.X86_REG_BX: 'BX', X.X86_REG_CX: 'CX', X.X86_REG_DX: 'DX',
         X.X86_REG_SI: 'SI', X.X86_REG_DI: 'DI', X.X86_REG_BP: 'BP', X.X86_REG_SP: 'SP',
         X.X86_REG_CS: 'CS', X.X86_REG_DS: 'DS', X.X86_REG_ES: 'ES', X.X86_REG_SS: 'SS'}
REG8 = {X.X86_REG_AL: 'AL', X.X86_REG_AH: 'AH', X.X86_REG_BL: 'BL', X.X86_REG_BH: 'BH',
        X.X86_REG_CL: 'CL', X.X86_REG_CH: 'CH', X.X86_REG_DL: 'DL', X.X86_REG_DH: 'DH'}
SEGREG = {X.X86_REG_CS: 'CS', X.X86_REG_DS: 'DS', X.X86_REG_ES: 'ES', X.X86_REG_SS: 'SS'}
JCC = {'je': 'C_E', 'jne': 'C_NE', 'jb': 'C_B', 'jae': 'C_AE', 'jbe': 'C_BE', 'ja': 'C_A',
       'jl': 'C_L', 'jge': 'C_GE', 'jle': 'C_LE', 'jg': 'C_G', 'js': 'C_S', 'jns': 'C_NS',
       'jo': 'C_O', 'jno': 'C_NO'}


# functions replaced by hand-written C in port/runtime/overrides.c
OVERRIDES = set()
_ov = os.path.join(HERE, 'overrides.txt')
if os.path.exists(_ov):
    OVERRIDES = {l.split('#')[0].strip() for l in open(_ov)} - {''}


# functions always run by the interpreter (coroutines switching stacks, ...)
INTERP_ALWAYS = set()
_ia = os.path.join(HERE, 'interp_functions.txt')
if os.path.exists(_ia):
    INTERP_ALWAYS = {l.split('#')[0].strip() for l in open(_ia)} - {''}

# callees that consume their caller's frame (pop the return address and leave through it):
# a call to one of these ends the calling function
NORETURN = {'F_root_04c4a'}


class Unsupported(Exception):
    pass


class Gen:
    def __init__(self, exe_path, pickle_path):
        self.exe = RTLinkExe(exe_path)
        d = pickle.load(open(pickle_path, 'rb'))
        self.insns = d['insns']
        self.entries = d['entries']
        self.calls = d['calls']
        self.jt = d['jump_tables']
        self.owner = d['owner']
        self.vectors = d['vectors']          # offset in 09c0 -> (image, lin, seg)
        self.relocs = {'root': set(self.exe.reloc_sites())}
        self.section_of = {'root': -1}
        self.base = {'root': 0}
        for s in self.exe.sections:
            n = 's%03d' % s.index
            self.relocs[n] = set(self.exe.reloc_sites(s.index))
            self.section_of[n] = s.index
            self.base[n] = s.load_seg
        self.mem = {'root': self.exe.root_memory()}
        for s in self.exe.sections:
            self.mem['s%03d' % s.index] = self.exe.section_memory(s.index)
        self.add_extra_entries(os.path.join(HERE, 'extra_entries.txt'))
        # functions
        self.funcs = collections.defaultdict(list)       # (img, entry) -> [lin]
        for (img, lin), ent in self.owner.items():
            self.funcs[(img, ent)].append(lin)
        for k in self.funcs:
            self.funcs[k].sort()
        self.entryset = set(self.funcs)
        self.jt_at = {}
        for (img, tl), t in self.jt.items():
            self.jt_at[(img, t['at'])] = (tl, t)
        self.vec_base = 0x9c00
        self.problems = collections.Counter()
        # jumps into the middle of another function (shared tails in hand-written code):
        # such addresses become extra entry points of their owner function
        self.extra = collections.defaultdict(set)
        for (img, lin), r in self.insns.items():
            mn = r[1]
            if not (mn in JCC or mn in ('jmp', 'loop', 'jcxz')):
                continue
            ins = next(MD.disasm(bytes(r[3]), r[5]))
            if not ins.operands or ins.operands[0].type != X.X86_OP_IMM:
                continue
            tlin = r[4] * 16 + (ins.operands[0].imm & 0xffff)
            ow = self.owner.get((img, tlin))
            if ow is not None and ow != self.owner.get((img, lin)) and tlin != ow:
                self.extra[(img, ow)].add(tlin)
        for (i_, lin_), r_ in self.insns.items():
            if r_[1] in ('ret', 'retf', 'iret', 'jmp', 'ljmp', 'hlt'):
                continue
            t_ = lin_ + r_[0]
            ow_ = self.owner.get((i_, t_))
            if ow_ is not None and ow_ != self.owner.get((i_, lin_)) and t_ != ow_:
                self.extra[(i_, ow_)].add(t_)
        self.find_smc()
        self.extra_name = {}
        for (img, ow), ts in self.extra.items():
            for t in ts:
                self.extra_name[(img, t)] = (ow, self.fname(img, t))

    def add_extra_entries(self, path):
        """Code reached only through data (far pointers patched at run time, ...): entries
        listed in extra_entries.txt as 'img seg:off' are decoded here by recursive descent."""
        if not os.path.exists(path):
            return
        todo = []
        for line in open(path):
            line = line.split('#')[0].split()
            if len(line) == 2:
                seg, off = (int(x, 16) for x in line[1].split(':'))
                todo.append((line[0], seg, off))
        while todo:
            img, frame, off = todo.pop()
            ent = frame * 16 + off
            if (img, ent) in self.owner:
                continue
            mem, base = self.mem[img], self.base[img] * 16
            work = [off]
            while work:
                o = work.pop()
                while True:
                    lin = frame * 16 + o
                    if (img, lin) in self.owner:
                        break
                    try:
                        ins = next(MD.disasm(bytes(mem[lin - base:lin - base + 16]), o))
                    except StopIteration:
                        break
                    self.insns[(img, lin)] = (ins.size, ins.mnemonic, ins.op_str, bytes(ins.bytes), frame, o)
                    self.owner[(img, lin)] = ent
                    m, ops = ins.mnemonic, ins.operands
                    if m in JCC or m in ('loop', 'jcxz'):
                        work.append(ops[0].imm & 0xffff)
                    elif m == 'jmp':
                        if ops[0].type == X.X86_OP_IMM:
                            work.append(ops[0].imm & 0xffff)
                        break
                    elif m == 'call' and ops[0].type == X.X86_OP_IMM:
                        t = ops[0].imm & 0xffff
                        self.calls[(img, lin)] = [(img, frame * 16 + t)]
                        todo.append((img, frame, t))
                    elif m in ('ret', 'retf', 'iret', 'ljmp', 'hlt'):
                        break
                    o = (o + ins.size) & 0xffff

    def find_smc(self):
        """Self-modifying code: instructions whose bytes are patched at run time by writes
        through cs:[absolute]. Their non-branch ones are executed by the interpreter."""
        cover = {}
        for (img, lin), r in self.insns.items():
            for k in range(r[0]):
                cover[(img, lin + k)] = lin
        wr = {'mov', 'add', 'sub', 'and', 'or', 'xor', 'inc', 'dec', 'adc', 'sbb', 'not', 'neg', 'xchg', 'pop'}
        self.smc = set()
        for (img, lin), r in self.insns.items():
            if r[1] not in wr:
                continue
            ins = next(MD.disasm(bytes(r[3]), r[5]))
            if not ins.operands:
                continue
            o = ins.operands[0]
            if o.type != X.X86_OP_MEM or o.mem.segment != X.X86_REG_CS or o.mem.base or o.mem.index:
                continue
            t = r[4] * 16 + (o.mem.disp & 0xffff)
            c = cover.get((img, t))
            if c is None:
                continue
            tm = self.insns[(img, c)][1]
            if tm in JCC or tm in ('jmp', 'ljmp', 'call', 'lcall', 'ret', 'retf', 'loop', 'jcxz'):
                continue        # patched transfers: far ones are read from memory at run time
            self.smc.add((img, c))
            print('  self-modifying: %s %05x patches %05x (%s %s)' % (img, lin, c, tm, self.insns[(img, c)][2]))

    @staticmethod
    def fname(img, lin):
        return 'F_%s_%05x' % (img, lin)

    # ------------------------------------------------------------------ operands
    def has_reloc(self, img, lin, size):
        rs = self.relocs[img]
        return [p for p in range(lin, lin + size) if p in rs]

    def ea(self, op, ins):
        m = op.mem
        parts = []
        for r in (m.base, m.index):
            if r:
                parts.append(REG16[r])
        if m.disp:
            parts.append(str(m.disp & 0xffff))
        off = '(u16)(%s)' % ('+'.join(parts) if parts else '0')
        if m.segment:
            if m.segment not in SEGREG:
                raise Unsupported('segment register')
            seg = SEGREG[m.segment]
            if seg == 'CS':
                seg = self.cur_csv
        elif m.base in (X.X86_REG_BP,):
            seg = 'SS'
        else:
            seg = 'DS'
        return seg, off

    def rd(self, op, ins, size=None):
        size = size or op.size
        if op.type == X.X86_OP_REG:
            return REG16.get(op.reg) or REG8[op.reg]
        if op.type == X.X86_OP_IMM:
            return str(op.imm & (0xff if size == 1 else 0xffff))
        seg, off = self.ea(op, ins)
        return ('rb(%s, %s)' if size == 1 else 'rw(%s, %s)') % (seg, off)

    def wr(self, op, ins, val, size=None):
        size = size or op.size
        if op.type == X.X86_OP_REG:
            r = REG16.get(op.reg) or REG8[op.reg]
            return '%s = (%s)(%s);' % (r, 'u8' if size == 1 else 'u16', val)
        seg, off = self.ea(op, ins)
        return ('wb(%s, %s, (u8)(%s));' if size == 1 else 'ww(%s, %s, (u16)(%s));') % (seg, off, val)

    # ------------------------------------------------------------------ one function
    def gen_function(self, img, ent):
        if self.fname(img, ent) in OVERRIDES:
            return '/* %s: implemented in port/runtime/overrides.c */\n' % self.fname(img, ent)
        if self.fname(img, ent) in INTERP_ALWAYS:
            fr = self.insns[(img, ent)][4]
            txt = 'void %s(void) { interp_run((u16)(LOADSEG + 0x%x), 0x%04x); }  /* interp_functions.txt */\n' % (
                self.fname(img, ent), fr, ent - fr * 16)
            for t in sorted(self.extra.get((img, ent), ())):
                fr2 = self.insns[(img, t)][4]
                txt += 'void %s(void) { interp_run((u16)(LOADSEG + 0x%x), 0x%04x); }\n' % (self.fname(img, t), fr2, t - fr2 * 16)
            return txt
        lins = self.funcs[(img, ent)]
        own = set(lins)
        out = []
        cs = None
        for lin in lins:
            r = self.insns[(img, lin)]
            size, mn, ops, raw, frame, off = r
            if cs is None:
                cs = frame
            try:
                code = self.gen_insn(img, ent, own, lin, r)
            except (Unsupported, KeyError, ValueError) as e:
                self.problems[str(e).split(':')[0]] += 1
                code = 'fatal("unsupported %s at %s:%05x");' % (str(e).replace('"', "'"), img, lin)
            out.append('L_%05x: %s' % (lin, code))
            if mn not in ('ret', 'retf', 'iret', 'jmp', 'ljmp', 'hlt') and (lin + size) not in own \
                    and not code.endswith('(); return;') and not code.endswith('return; }'):
                # falls through into code owned by another function
                out.append('/* fallthrough */ %s' % self.target_code(img, ent, own, lin + size, lin, call=True))
        body = '\n'.join('    ' + l for l in out)
        ex = sorted(self.extra.get((img, ent), ()))
        tag = '    FENTER(0x%x); const u16 my_sp = FRAME_SP();\n' % (((self.section_of[img] + 2) << 20) | ent)
        sect = self.section_of[img]
        def chk(t):
            fr = self.insns[(img, t)][4]
            return 'FCHECK(0x%x, 0x%x, %d);' % (t, fr, sect)
        if lins[0] != ent and not ex:
            body = '    goto L_%05x;\n' % ent + body     # the entry is not the lowest address
        body = tag + body
        if not ex:
            return 'void %s(void)\n{\n    %s\n%s\n}\n' % (self.fname(img, ent), chk(ent), body)
        # several entry points: one body with a dispatch on the entry, thin wrappers
        sw = ' '.join('case %d: goto L_%05x;' % (k + 1, t) for k, t in enumerate(ex))
        txt = 'static void B_%s_%05x(int e_)\n{\n%s    switch (e_) { %s default: goto L_%05x; }\n%s\n}\n' % (
            img, ent, tag, sw, ent, body[len(tag):])
        txt += 'void %s(void) { %s B_%s_%05x(0); }\n' % (self.fname(img, ent), chk(ent), img, ent)
        for k, t in enumerate(ex):
            txt += 'void %s(void) { %s B_%s_%05x(%d); }\n' % (self.fname(img, t), chk(t), img, ent, k + 1)
        return txt

    def target_code(self, img, ent, own, tlin, cur_lin, call=False):
        """goto inside the function, or a tail call into another function."""
        if tlin in own and not call:
            return ('POLL(); ' if tlin <= cur_lin else '') + 'goto L_%05x;' % tlin
        if (img, tlin) in self.entryset:
            return 'TAIL_SET(); %s(); return;' % self.fname(img, tlin)
        if (img, tlin) in self.extra_name:
            return 'TAIL_SET(); %s(); return;' % self.extra_name[(img, tlin)][1]
        self.problems['jump to unknown code'] += 1
        return 'fatal("jump to unknown %s:%05x");' % (img, tlin)

    def call_target(self, img, lin, tlin_hint=None):
        """(target image, target lin) of a direct call at lin."""
        t = self.calls.get((img, lin))
        if t:
            return t[0]
        return None

    def gen_insn(self, img, ent, own, lin, r):
        size, mn, ops_s, raw, frame, off = r
        ins = next(MD.disasm(bytes(raw), off))
        ops = ins.operands
        nxt = (off + size) & 0xffff
        csv = '(u16)(LOADSEG+0x%04x)' % frame
        self.cur_csv = csv
        if (img, lin) in getattr(self, 'smc', ()):
            return 'interp_step(%s, 0x%04x);' % (csv, off)
        relocs = self.has_reloc(img, lin, size)
        prefix = ''
        m = mn
        if m.startswith('rep'):
            pass
        # --------------------------------------------------- data movement
        if m == 'mov':
            d, s = ops
            if s.type == X.X86_OP_IMM and relocs:
                val = '(u16)(LOADSEG + %d)' % (s.imm & 0xffff)
            elif s.type == X.X86_OP_REG and s.reg == X.X86_REG_CS:
                val = csv
            else:
                val = self.rd(s, ins, d.size)
            return self.wr(d, ins, val)
        if m == 'push':
            s = ops[0]
            if s.type == X.X86_OP_IMM:
                v = s.imm & 0xffff
                return 'push16(%s);' % ('(u16)(LOADSEG + %d)' % v if relocs else str(v))
            if s.type == X.X86_OP_REG and s.reg == X.X86_REG_CS:
                return 'push16(%s);' % csv
            if s.type == X.X86_OP_REG and s.reg == X.X86_REG_SP:
                return '{ u16 t = SP; push16(t); }'
            return 'push16(%s);' % self.rd(s, ins, 2)
        if m == 'pop':
            d = ops[0]
            if d.type == X.X86_OP_MEM:
                return '{ u16 t = pop16(); %s }' % self.wr(d, ins, 't', 2)
            return self.wr(d, ins, 'pop16()', 2)
        if m == 'xchg':
            a, b = ops
            w = a.size
            ty = 'u8' if w == 1 else 'u16'
            return '{ %s t = %s; %s %s }' % (ty, self.rd(a, ins), self.wr(a, ins, self.rd(b, ins)), self.wr(b, ins, 't'))
        if m == 'lea':
            d, s = ops
            seg, o = self.ea(s, ins)
            return self.wr(d, ins, o)
        if m in ('les', 'lds'):
            d, s = ops
            seg, o = self.ea(s, ins)
            sr = 'ES' if m == 'les' else 'DS'
            return '{ u16 o_ = %s; u16 v_ = rw(%s, o_); u16 s_ = rw(%s, (u16)(o_ + 2)); %s %s = s_; }' % (
                o, seg, seg, self.wr(d, ins, 'v_'), sr)
        if m == 'cwde':
            return 'AX = (u16)(s16)(s8)AL;'
        if m == 'cdq':
            return 'DX = (AX & 0x8000) ? 0xffff : 0;'
        if m == 'xlatb':
            seg = 'DS'
            if ins.prefix[1] or raw[0] in (0x26, 0x2e, 0x36, 0x3e):
                seg = {0x26: 'ES', 0x2e: 'CS', 0x36: 'SS', 0x3e: 'DS'}[raw[0]]
            if seg == 'CS':
                seg = csv
            return 'AL = rb(%s, (u16)(BX + AL));' % seg
        if m == 'lahf':
            return 'AH = (u8)get_flags();'
        if m == 'sahf':
            return 'set_flags((u16)((get_flags() & 0xff00) | AH));'
        if m == 'pushf':
            return 'push16(get_flags());'
        if m == 'popf':
            return 'set_flags(pop16());'
        if m in ('cli', 'sti'):
            return 'R.iflag = %d;' % (m == 'sti')
        if m == 'cld':
            return 'R.df = 0;'
        if m == 'std':
            return 'R.df = 1;'
        if m == 'clc':
            return 'set_flags((u16)(get_flags() & ~1));'
        if m == 'stc':
            return 'set_flags((u16)(get_flags() | 1));'
        if m == 'cmc':
            return 'set_flags((u16)(get_flags() ^ 1));'
        if m in ('nop', 'int3'):
            return ';'
        # --------------------------------------------------- arithmetic
        if m in ('add', 'sub', 'adc', 'sbb', 'cmp'):
            d, s = ops
            w = 8 * d.size
            sv = self.rd(s, ins, d.size)
            if s.type == X.X86_OP_IMM:
                sv = str(s.imm & (0xff if w == 8 else 0xffff))
            fn = {'add': 'f_add', 'sub': 'f_sub', 'adc': 'f_adc', 'sbb': 'f_sbb', 'cmp': 'f_sub'}[m]
            e = '%s(%s, %s, %d)' % (fn, self.rd(d, ins), sv, w)
            if m == 'cmp':
                return '%s;' % e
            return self.wr(d, ins, e)
        if m in ('and', 'or', 'xor', 'test'):
            d, s = ops
            w = 8 * d.size
            sv = self.rd(s, ins, d.size)
            if s.type == X.X86_OP_IMM:
                sv = str(s.imm & (0xff if w == 8 else 0xffff))
            c = {'and': '&', 'or': '|', 'xor': '^', 'test': '&'}[m]
            e = 'f_logic(%s %s %s, %d)' % (self.rd(d, ins), c, sv, w)
            if m == 'test':
                return '%s;' % e
            return self.wr(d, ins, e)
        if m in ('inc', 'dec', 'neg'):
            d = ops[0]
            w = 8 * d.size
            return self.wr(d, ins, 'f_%s(%s, %d)' % (m, self.rd(d, ins), w))
        if m == 'not':
            d = ops[0]
            return self.wr(d, ins, '~%s' % self.rd(d, ins))
        if m in ('shl', 'shr', 'sar', 'rol', 'ror', 'rcl', 'rcr'):
            d = ops[0]
            w = 8 * d.size
            if len(ops) == 1:
                n = '1'
            else:
                c = ops[1]
                n = 'CL' if c.type == X.X86_OP_REG else str(c.imm & 0xff)
            return self.wr(d, ins, 'f_%s(%s, (%s) & 31, %d)' % (m, self.rd(d, ins), n, w))
        if m in ('mul', 'imul', 'div', 'idiv') and len(ops) == 1:
            s = ops[0]
            w = 8 * s.size
            cast = {('mul', 8): 'u8', ('mul', 16): 'u16', ('imul', 8): 's8', ('imul', 16): 's16',
                    ('div', 8): 'u8', ('div', 16): 'u16', ('idiv', 8): 's8', ('idiv', 16): 's16'}[(m, w)]
            return 'f_%s%d((%s)%s);' % (m, w, cast, self.rd(s, ins))
        if m == 'imul':
            if len(ops) == 3:
                d, s, i = ops
                return self.wr(d, ins, 'f_imul3((s16)%s, (s16)%d)' % (self.rd(s, ins), i.imm & 0xffff if i.imm >= 0 else i.imm))
            d, s = ops
            return self.wr(d, ins, 'f_imul3((s16)%s, (s16)%s)' % (self.rd(d, ins), self.rd(s, ins)))
        if m == 'daa':
            return 'f_daa();'
        if m == 'das':
            return 'f_das();'
        # --------------------------------------------------- string ops
        sm = re.match(r'^(rep |repe |repne )?(movs|stos|lods|scas|cmps)([bw])$', m)
        if sm:
            rep = (sm.group(1) or '').strip()
            op = sm.group(2)
            w = 1 if sm.group(3) == 'b' else 2
            src = 'DS'
            for b in raw:
                if b in (0x26, 0x2e, 0x36, 0x3e):
                    src = {0x26: 'ES', 0x2e: 'CS', 0x36: 'SS', 0x3e: 'DS'}[b]
            if src == 'CS':
                src = csv
            return 'str_%s%d(%d, %s);' % (op, w * 8, {'': 0, 'rep': 1, 'repe': 1, 'repne': 2}[rep], src)
        # --------------------------------------------------- stack frames
        if m == 'enter':
            n, lvl = ops[0].imm, ops[1].imm
            if lvl:
                raise Unsupported('enter level')
            return 'push16(BP); BP = SP; SP = (u16)(SP - %d);' % n
        if m == 'leave':
            return 'SP = BP; BP = pop16();'
        # --------------------------------------------------- control flow
        if m in JCC:
            t = ins.operands[0].imm & 0xffff
            tlin = frame * 16 + t
            return 'if (%s) { %s }' % (JCC[m], self.target_code(img, ent, own, tlin, lin))
        if m == 'loop':
            t = ins.operands[0].imm & 0xffff
            return 'if (--CX) { %s }' % self.target_code(img, ent, own, frame * 16 + t, lin)
        if m == 'jcxz':
            t = ins.operands[0].imm & 0xffff
            return 'if (!CX) { %s }' % self.target_code(img, ent, own, frame * 16 + t, lin)
        if m == 'jmp':
            o = ops[0]
            if o.type == X.X86_OP_IMM:
                return self.target_code(img, ent, own, frame * 16 + (o.imm & 0xffff), lin)
            # jump table?
            jt = self.jt_at.get((img, lin))
            if jt:
                tl, t = jt
                idx_reg = {X.X86_REG_BX: 'BX', X.X86_REG_SI: 'SI', X.X86_REG_DI: 'DI'}.get(o.mem.base or o.mem.index) \
                    if o.type == X.X86_OP_MEM else None
                if idx_reg:
                    cases = []
                    tbl = t['seg'] * 16 + t['base']
                    disp = o.mem.disp & 0xffff
                    for k, tg in enumerate(t['targets']):
                        # entry k sits at table + 2k; the register holds disp-relative byte offset
                        regval = (t['base'] + 2 * k - disp) & 0xffff
                        cases.append('case %d: %s' % (regval, self.target_code(img, ent, own, tg, lin)))
                    return 'switch (%s) { %s default: { u16 t_ = %s; TAIL_SET(); dispatch_jmp_near(%s, t_); return; } }' % (
                        idx_reg, ' '.join(cases), self.rd(o, ins, 2), csv)
            self.problems['indirect jmp'] += 1
            return '{ u16 t_ = %s; TAIL_SET(); dispatch_jmp_near(%s, t_); return; }' % (self.rd(o, ins, 2), csv)
        if m == 'ljmp':
            o = ops[0]
            if o.type == X.X86_OP_IMM:
                # 'ljmp seg:off' - capstone gives two imm operands
                pass
            if len(ops) == 2:
                tseg, toff = ops[0].imm & 0xffff, ops[1].imm & 0xffff
                return self.far_transfer(img, lin, size, tseg, toff, nxt, csv, jump=True)
            seg, o2 = self.ea(o, ins)
            return '{ u16 o_ = %s; u16 t_ = rw(%s, o_); u16 s_ = rw(%s, (u16)(o_ + 2)); TAIL_SET(); dispatch_jmp_far(s_, t_); return; }' % (
                o2, seg, seg)
        if m == 'call':
            o = ops[0]
            if o.type == X.X86_OP_IMM:
                tlin = frame * 16 + (o.imm & 0xffff)
                if (img, tlin) in self.entryset:
                    if self.fname(img, tlin) in NORETURN:
                        return 'push16(0x%04x); %s(); return;' % (nxt, self.fname(img, tlin))
                    return 'push16(0x%04x); %s(); UNW_CHECK();' % (nxt, self.fname(img, tlin))
                self.problems['near call to non-entry'] += 1
                return 'push16(0x%04x); fatal("near call to non-entry %s:%05x");' % (nxt, img, tlin)
            return '{ u16 t_ = %s; push16(0x%04x); dispatch_near(%s, t_); } UNW_CHECK();' % (self.rd(o, ins, 2), nxt, csv)
        if m == 'lcall':
            if len(ops) == 2:
                tseg, toff = ops[0].imm & 0xffff, ops[1].imm & 0xffff
                return self.far_transfer(img, lin, size, tseg, toff, nxt, csv, jump=False)
            o = ops[0]
            seg, o2 = self.ea(o, ins)
            return '{ u16 o_ = %s; u16 t_ = rw(%s, o_); u16 s_ = rw(%s, (u16)(o_ + 2)); push16(%s); push16(0x%04x); dispatch_far(s_, t_); } UNW_CHECK();' % (
                o2, seg, seg, csv, nxt)
        if m == 'ret':
            n = ops[0].imm if ops else 0
            return 'RET_NEAR(%d, %s);' % (n, csv)
        if m == 'retf':
            n = ops[0].imm if ops else 0
            return 'RET_FAR(%d);' % n
        if m == 'iret':
            return 'IRET();'
        if m == 'int':
            return 'hle_int(0x%02x);' % (ops[0].imm & 0xff)
        if m == 'in':
            d, s = ops
            p = 'DX' if s.type == X.X86_OP_REG else str(s.imm & 0xff)
            if d.size != 1:
                return '{ u16 p_ = %s; AL = hle_in8(p_); AH = hle_in8((u16)(p_ + 1)); }' % p
            return 'AL = hle_in8(%s);' % p
        if m == 'out':
            d, s = ops
            p = 'DX' if d.type == X.X86_OP_REG else str(d.imm & 0xff)
            if s.size != 1:
                return '{ u16 p_ = %s; hle_out8(p_, AL); hle_out8((u16)(p_ + 1), AH); }' % p
            return 'hle_out8(%s, AL);' % p
        raise Unsupported('mnemonic %s' % m)

    def far_transfer(self, img, lin, size, tseg, toff, nxt, csv, jump):
        """direct far call/jmp. The segment word is relocated (link-time frame)."""
        push = 'TAIL_SET(); ' if jump else 'push16(%s); push16(0x%04x); ' % (csv, nxt)
        tail = ' return;' if jump else ''
        # RTLink vector stub in root 09c0
        if tseg == 0x9c0 and toff in self.vectors:
            timg, tlin, vseg = self.vectors[toff]
            sect = self.section_of[timg]
            back = ''
            if not jump and self.section_of[img] >= 0:
                back = ' ovl_enter(%d);' % self.section_of[img]
            if (timg, tlin) in self.entryset:
                return '%sovl_enter(%d); %s();%s%s' % (push, sect, self.fname(timg, tlin), back, tail or ' UNW_CHECK();')
            self.problems['vector to non-entry'] += 1
            return 'fatal("vector to non-entry %s:%05x");' % (timg, tlin)
        t = self.calls.get((img, lin))
        cand = [x for x in (t or []) if x in self.entryset]
        if not self.has_reloc(img, lin + size - 2, 2):
            cand = []           # unrelocated far target: patched at run time (driver slots)
        if cand:
            timg, tlin = cand[0]
            return '%s%s();%s' % (push, self.fname(timg, tlin), tail or ' UNW_CHECK();')
        # not resolved statically (e.g. driver slots patched at run time): read the operand
        # from memory at run time and dispatch
        self.problems['dynamic far ' + ('jmp' if jump else 'call')] += 1
        loc = 'LIN(LOADSEG + 0x%04x, 0x%04x)' % (self.insns[(img, lin)][4], (self.insns[(img, lin)][5] + 1) & 0xffff)
        fn = 'dispatch_jmp_far' if jump else 'dispatch_far'
        return '%s{ u32 l_ = %s; %s((u16)(M[l_ + 2] | (M[l_ + 3] << 8)), (u16)(M[l_] | (M[l_ + 1] << 8))); }%s' % (
            push, loc, fn, tail or ' UNW_CHECK();')

    # ------------------------------------------------------------------ files
    def run(self, out_dir):
        os.makedirs(out_dir, exist_ok=True)
        by_img = collections.defaultdict(list)
        for (img, ent) in self.funcs:
            by_img[img].append(ent)
        protos = []
        table = []
        for img in sorted(by_img):
            ents = sorted(by_img[img])
            body = ['/* generated by port/recomp/recomp16.py - do not edit */', '#include "cpu.h"',
                    '#include "funcs.h"', '#include "strops.h"', '']
            for ent in ents:
                body.append(self.gen_function(img, ent))
                protos.append('void %s(void);' % self.fname(img, ent))
                table.append((ent, self.section_of[img], self.fname(img, ent)))
                for t in sorted(self.extra.get((img, ent), ())):
                    protos.append('void %s(void);' % self.fname(img, t))
                    table.append((t, self.section_of[img], self.fname(img, t)))
            # split big images to keep compile units reasonable
            open(os.path.join(out_dir, 'g_%s.c' % img), 'w').write('\n'.join(body))
        open(os.path.join(out_dir, 'funcs.h'), 'w').write(
            '/* generated */\n#ifndef FUNCS_H\n#define FUNCS_H\n' + '\n'.join(protos) + '\n#endif\n')
        table.sort()
        rows = ',\n'.join('    {0x%05x, %d, %s}' % t for t in table)
        ia = sorted(e for (e, sec, fn) in table if fn in INTERP_ALWAYS)
        open(os.path.join(out_dir, 'dispatch_table.c'), 'w').write(
            '/* generated */\n#include "cpu.h"\n#include "funcs.h"\n#include "dispatch.h"\n'
            'const FuncEntry FUNC_TABLE[] = {\n%s\n};\nconst int FUNC_COUNT = %d;\n' % (rows, len(table)) +
            'const unsigned INTERP_ALWAYS_TABLE[] = {%s};\nconst int INTERP_ALWAYS_COUNT = %d;\n' % (
                ', '.join('0x%x' % x for x in ia) or '0', len(ia)))
        # vectors: 09c0 offset -> (section, function)
        vrows = []
        for o, (timg, tlin, vseg) in sorted(self.vectors.items()):
            if (timg, tlin) in self.entryset:
                vrows.append('    {0x%04x, %d, %s}' % (o, self.section_of[timg], self.fname(timg, tlin)))
        open(os.path.join(out_dir, 'vector_table.c'), 'w').write(
            '/* generated */\n#include "cpu.h"\n#include "funcs.h"\n#include "dispatch.h"\n'
            'const VecEntry VEC_TABLE[] = {\n%s\n};\nconst int VEC_COUNT = %d;\n' % (',\n'.join(vrows), len(vrows)))
        # return sites: addresses right after a call in recompiled code (a jump there is a
        # return through a popped return address, see dispatch.c)
        rs = set()
        for (img, lin), r in self.insns.items():
            if r[1] in ('call', 'lcall') and self.owner.get((img, lin)) is not None:
                rs.add((lin + r[0], self.section_of[img]))
        rrows = ',\n'.join('    {0x%05x, %d}' % t for t in sorted(rs))
        open(os.path.join(out_dir, 'retsite_table.c'), 'w').write(
            '/* generated */\n#include "dispatch.h"\nconst RetSite RET_TABLE[] = {\n%s\n};\nconst int RET_COUNT = %d;\n' % (rrows, len(rs)))
        print('functions:', len(table), 'images:', len(by_img))
        for k, v in self.problems.most_common():
            print('  %6d %s' % (v, k))


if __name__ == '__main__':
    g = Gen(sys.argv[1], sys.argv[2])
    g.run(sys.argv[3])
