#!/usr/bin/env python3
"""Dump full disassemblies of a draft's compiled functions next to the originals (relocated as in
tools/fmatch.py), for reading a non-matching draft against its assembly.
  cdis.py FILE.c OUTDIR [--func SUBSTR]
writes OUTDIR/<func>.orig.txt and OUTDIR/<func>.c.txt (address-free 'mnemonic operands' lines are
easy to diff -y)."""
import sys, os
sys.path.insert(0, '/home/claude/darklands-re/tools')
import fmatch
out = sys.argv[2]; os.makedirs(out, exist_ok=True)
only = sys.argv[sys.argv.index('--func') + 1] if '--func' in sys.argv else None
cur = {}
def dump(orig, code, base, **kw):
    import capstone
    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_16)
    n = cur['n']
    for tag, b in (('orig', orig), ('c', code)):
        with open(os.path.join(out, '%s.%s.txt' % (n, tag)), 'w') as f:
            for x in md.disasm(b, base):
                f.write('%04x  %s %s\n' % (x.address, x.mnemonic, x.op_str))
fmatch.show_diff = dump
_print = print
import builtins
def pr(*a, **k):
    s = ' '.join(str(x) for x in a)
    if s.startswith('  sub_'): cur['n'] = s.split()[0]
    _print(*a, **k)
builtins.print = pr
fmatch.check(sys.argv[1], True, only=[only] if only else None)
