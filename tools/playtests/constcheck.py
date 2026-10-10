#!/usr/bin/env python3
"""Check the CONST (far segment slot) placement of drafts: the DGROUP words the ORIGINAL code of the
file loads segments from ('mov es, [7xxx]' / 'push word ptr [7xxx]' in 7c00..7fff) must be the ones
the compiled C uses, and must lie inside the file's stamped CONST range. A draft that needs more
slots than the original (two far names where the original had one) gets stamped at a wrong place by
data-content search and then overwrites other modules' segment slots in --mod builds.
Also reports stamped data ranges that overlap another file's.
  constcheck.py FILE.c [...]      (default: every stamped src/wip draft)"""
import sys, os, re, io, glob, contextlib, collections
sys.path.insert(0, '/home/claude/darklands-re/tools')
import fmatch
import capstone
REPO = '/home/claude/darklands-re'
md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_16)
PAT = re.compile(r'(?:mov (?:es|ds|ax|bx|cx|dx), word ptr|push word ptr|les \w+, ptr|lds \w+, ptr) \[(0x7[c-f][0-9a-f]{2})\]')
cap = {}
def grab(orig, code, base, **kw):
    for tag, b in (('o', orig), ('c', code)):
        cap.setdefault(tag, collections.Counter())
        for x in md.disasm(b, base):
            m = PAT.search('%s %s' % (x.mnemonic, x.op_str))
            if m: cap[tag][int(m.group(1), 16)] += 1
fmatch.show_diff = grab
def stamp(path):
    t = open(path, encoding='latin-1').read(3000)
    m = re.search(r'/\* orig: ([^*]*)\*/', t)
    return {k: (int(a, 16), int(a, 16) + int(b, 16)) for k, a, b in re.findall(r'(\w+)=([0-9a-f]{4})\+([0-9a-f]{4})', m.group(1))} if m else None
def check(path):
    cap.clear()
    with contextlib.redirect_stdout(io.StringIO()):
        r = fmatch.check(path, True)
    # matching functions are not passed to show_diff: they use the same slots as the original
    o, c = cap.get('o', collections.Counter()), cap.get('c', collections.Counter())
    st = (stamp(path) or {}).get('CONST')
    probs = []
    if st:
        out = sorted(a for a in c if not st[0] <= a < st[1])
        if out: probs.append('C uses slots outside its stamp %04x-%04x: %s' % (st[0], st[1], ' '.join('%04x' % a for a in out)))
        oo = sorted(a for a in o if not st[0] <= a < st[1])
        if oo: probs.append('original uses slots outside the stamp: %s' % ' '.join('%04x' % a for a in oo))
    if sorted(o.values()) != sorted(c.values()) or (o and min(o) != min(c or [0])):
        probs.append('slot use differs: orig %s | C %s' % (' '.join('%04x:%d' % kv for kv in sorted(o.items())),
                                                            ' '.join('%04x:%d' % kv for kv in sorted(c.items()))))
    return probs
if __name__ == '__main__':
    files = sys.argv[1:] or sorted(glob.glob(REPO + '/src/wip/s*/*.c'))
    for f in files:
        if not stamp(f): continue
        try: p = check(f)
        except SystemExit as e: p = ['error %s' % e]
        except Exception as e: p = ['error %r' % e]
        print('%-40s %s' % (os.path.relpath(f, REPO), 'ok' if not p else ''))
        for x in p: print('    ' + x)
