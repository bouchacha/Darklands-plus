"""declsearch.py FILE FUNCS(comma) DECL1 [DECL2 ...] : coordinate descent over the positions of declaration lines
(among top-level lines before the first function body). Minimises the sum of score.py over FUNCS. Writes FILE on success
(score 0) or prints best; FILE is only rewritten when the total score improves."""
import sys, subprocess
f, funcs, decls = sys.argv[1], sys.argv[2].split(','), sys.argv[3:]
lines = open(f).read().split('\n')
tmp = f.replace('.c', '_ds.c')

def score(L):
    open(tmp, 'w').write('\n'.join(L))
    tot = 0
    for fn in funcs:
        out = subprocess.run(['python3', '-I', 'build/agent/r3w2/score.py', tmp, fn], capture_output=True, text=True).stdout
        tot += int(out.split()[1]) if out.startswith('SCORE') else 999
    return tot

cur = list(lines)
best = score(cur)
print('start', best, flush=True)
improved = True
while improved and best > 0:
    improved = False
    for d in decls:
        rest = [l for l in cur if l != d]
        end = next(i for i, l in enumerate(rest) if l == '{') - 1
        for pos in range(4, end):
            q = rest[:pos] + [d] + rest[pos:]
            sc = score(q)
            if sc < best:
                best, cur, improved = sc, q, True
                print(d, pos, best, flush=True)
                if best == 0:
                    break
        if best == 0:
            break
if best < score(lines):
    open(f, 'w').write('\n'.join(cur))
print('final', best)
