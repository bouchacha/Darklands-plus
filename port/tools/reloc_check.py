#!/usr/bin/env python3
"""reloc_check.py - every segment the DOS loader would have fixed up in DARKLAND.EXE's code, and whether the
translation (port/recomp/recomp16.py) adds the load segment there too.

  reloc_check.py DARKLAND.EXE build/analysis/analysis.pickle

A relocation is a word of the program that holds a link-time segment; DOS adds the program's load segment
to it when loading. The translated code must do the same, or a comparison or a stored address is off by the
load segment: the dwarves' iron door in the mines stayed shut after its riddle for that (cmp word [es:bx+218e],
seg vec_1829 compared without it; fixed in recomp16.py's imm_val, 1.3.1). The translator adds it for the
immediate of mov, push and the arithmetic and logic instructions, and for the segment of a far call or jump;
this lists any relocation found elsewhere in an instruction (a new kind of instruction carrying one, or data
the analysis took for code). Not one of the translator's files: changing it makes no player's game module
be made again. Prints the instructions and exits 1 when there is any.
"""
import sys, os
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', 'recomp'))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
import recomp16                                     # noqa: E402

# relocations known to be outside code that runs: root 06b1f follows the C runtime's call through [DS:7722]
# on the way out of the program (root 06a1:010b, which does not come back); its bytes are data holding a segment
KNOWN = {('root', 0x06b1f)}
IMM = ('mov', 'push', 'cmp', 'add', 'sub', 'adc', 'sbb', 'and', 'or', 'xor', 'test')   # the immediate: its last two bytes
FAR = ('lcall', 'ljmp')                                                                   # the segment: the last two bytes


def main():
    if len(sys.argv) != 3:
        sys.exit(__doc__)
    g = recomp16.Gen(sys.argv[1], sys.argv[2])
    counts, odd = {}, []
    for (img, lin), r in sorted(g.insns.items()):
        size, mn, ops = r[0], r[1], r[2]
        rel = g.has_reloc(img, lin, size)
        if not rel:
            continue
        counts[mn] = counts.get(mn, 0) + 1
        ins = next(recomp16.MD.disasm(bytes(r[3]), r[5]))
        in_last2 = set(rel) <= {lin + size - 2, lin + size - 1}
        imm16 = bool(ins.operands) and ins.operands[-1].type == recomp16.X.X86_OP_IMM and ins.operands[-1].size == 2
        ok = in_last2 and (mn in FAR or (mn in IMM and imm16))
        if not ok and (img, lin) not in KNOWN:
            odd.append('%s %05x %s %s (relocation at %s)' % (img, lin, mn, ops, ', '.join('+%d' % (p - lin) for p in rel)))
    print('instructions with a relocation:', ', '.join('%s %d' % kv for kv in sorted(counts.items())))
    for o in odd:
        print('  not translated with the load segment:', o)
    print('all of them translated with the load segment' if not odd else '%d not' % len(odd))
    return 1 if odd else 0


if __name__ == '__main__':
    sys.exit(main())
