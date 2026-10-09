"""dsearch.py FILE FUNC N DECL1 [DECL2 ...]: random positions of the given decl lines among top-level lines; stop at func match. N = tries"""
import sys, os, io, contextlib, random
sys.path.insert(0, '/home/claude/darklands-re/tools')
os.chdir('/home/claude/darklands-re')
import fmatch, capstone
cap = []
fmatch.show_diff = lambda orig, code, base, **kw: cap.append((orig, code, base))
md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_16)
def calls(b, base):
    return ' '.join(i.op_str.split(',')[-1].strip()[-4:] for i in md.disasm(b, base) if i.mnemonic in ('lcall', 'call'))
f, fn, N = sys.argv[1], sys.argv[2], int(sys.argv[3])
decls = sys.argv[4:]
lines = open(f).read().split('\n')
rest = [l for l in lines if l not in decls]
end = next(i for i, l in enumerate(rest) if l == '{') - 1
out = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'ds_' + os.path.basename(f))
random.seed(int(os.environ.get('SEED', '1')))
for t in range(N):
    pos = sorted(((random.randrange(5, end) if os.environ.get('SCAN') is None else t + 5, d) for d in decls), key=lambda x: -x[0])
    q = list(rest)
    for p, d in pos: q.insert(p, d)
    open(out, 'w').write('\n'.join(q))
    cap.clear()
    with contextlib.redirect_stdout(io.StringIO()):
        r = fmatch.check(out, True, only=fn.split(','))
    fns = [x for x in r['functions'] if any(x['name'].endswith(q) for q in fn.split(','))]
    ok = fns and all(x['match'] for x in fns)
    print(t, [p for p, d in pos], 'MATCH' if ok else ' / '.join(calls(c, b) for o, c, b in cap), r['file_match'] and 'FILE' or '', flush=True)
    if ok:
        print('written', out); break
