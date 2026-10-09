# multi.py file func_suffix candfile.py : candfile defines SUBS = list of (OLD, [cands]); full product searched
import sys, os, io, contextlib, itertools
sys.path.insert(0, '/home/claude/darklands-re/tools')
import fmatch, capstone
cap = []
def hook(orig, code, base, **kw):
    cap.append((orig, code, base))
fmatch.show_diff = hook
md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_16)
def calls(b, base):
    return ' '.join(i.op_str.split(',')[-1].strip()[-4:] for i in md.disasm(b, base) if i.mnemonic in ('lcall', 'call'))
f, fs, cf = sys.argv[1], sys.argv[2], sys.argv[3]
ns = {}
exec(open(cf).read(), ns)
SUBS = ns['SUBS']
src = open(f).read()
for o, _ in SUBS: assert o in src, o
best = None; shown = False
try:
    for i, combo in enumerate(itertools.product(*[c for _, c in SUBS])):
        s = src
        for (o, _), c in zip(SUBS, combo): s = s.replace(o, c)
        open(f, 'w').write(s)
        cap.clear()
        with contextlib.redirect_stdout(io.StringIO()):
            r = fmatch.check(f, True, only=fs.split(','))
        if r['error']:
            print(i, 'ERROR', r['error'][:300].replace('\n', ' ')); continue
        fns = [x for x in r['functions'] if any(x['name'].endswith(q) for q in fs.split(','))]
        st = ' '.join('M' if fn['match'] else 'd%x' % fn['first_diff'] for fn in fns)
        if cap and not shown:
            print('orig calls:', ' / '.join(calls(o, b) for o, c, b in cap)); shown = True
        print(i, st, r['file_match'] and 'FILE' or '', '|', ' / '.join(calls(c, b) for o, c, b in cap), '|', ' || '.join(c.replace('\n', ' ')[:60] for c in combo), flush=True)
        if all(fn['match'] for fn in fns):
            best = s; break
finally:
    open(f, 'w').write(best if best is not None else src)
print('applied' if best is not None else 'none matched')
