#!/usr/bin/env python3
"""Find pointer-vs-array confusions in drafts: a DGROUP address the ORIGINAL code only reads as a
variable ([X]) but the compiled C uses as an immediate address (push X / mov r, X) - i.e. the C
declares a pointer variable as an array (e.g. 'extern struct loot D_c291[]' for 'struct loot *D_c291')
- and the reverse. Only non-matching functions are compared (per function, relocated like fmatch).
  ptrcheck.py FILE.c [...]      (default: every stamped src/wip draft)"""
import sys, os, re, io, glob, contextlib
sys.path.insert(0, '/home/claude/darklands-re/tools')
import fmatch, capstone
REPO = '/home/claude/darklands-re'
md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_16)
MEM = re.compile(r'(?<![:\w])\[(0x[0-9a-f]{3,4})\]')
IMM = re.compile(r'(?:^|, |push )(0x[0-9a-f]{3,4})$')
res = []
cur = {}
def scan(b, base):
    mem, imm = set(), set()
    for x in md.disasm(b, base):
        s = '%s %s' % (x.mnemonic, x.op_str)
        if x.mnemonic in ('call', 'lcall', 'jmp', 'ljmp') or x.mnemonic.startswith('j') or x.mnemonic == 'loop':
            continue
        for m in MEM.finditer(s): mem.add(int(m.group(1), 16))
        m = IMM.search(x.op_str if x.mnemonic != 'push' else 'push ' + x.op_str)
        if m and not x.op_str.startswith('['): imm.add(int(m.group(1), 16))
    return mem, imm
def grab(orig, code, base, **kw):
    om, oi = scan(orig, base); cm, ci = scan(code, base)
    a = sorted(x for x in (ci - oi) & om if x >= 0x100)
    b = sorted(x for x in (oi - ci) & cm if x >= 0x100)
    if a or b: res.append((cur.get('n', '?'), a, b))
fmatch.show_diff = grab
import builtins
_p = builtins.print
def pr(*a, **k):
    s = ' '.join(str(x) for x in a)
    if s.startswith('  sub_'): cur['n'] = s.split()[0]
builtins.print = pr
def check(path):
    res.clear()
    fmatch.check(path, True)
    return list(res)
if __name__ == '__main__':
    files = sys.argv[1:] or sorted(glob.glob(REPO + '/src/wip/s*/*.c'))
    for f in files:
        if '/* orig:' not in open(f, encoding='latin-1').read(3000): continue
        try: r = check(f)
        except BaseException as e: r = [('error', repr(e)[:80], [])]
        for n, a, b in r:
            _p('%s %s: C takes the address of %s (orig reads the variable); C reads %s (orig uses the address)' % (
                os.path.relpath(f, REPO), n, ' '.join('%04x' % x for x in a) or '-', ' '.join('%04x' % x for x in b) or '-'))
