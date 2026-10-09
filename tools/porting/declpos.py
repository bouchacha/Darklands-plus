"""declpos.py FILE FUNC DECLLINE : move DECLLINE to each possible position among top-level lines before first function body; report diff counts"""
import sys, subprocess
f, func, decl = sys.argv[1], sys.argv[2], sys.argv[3]
lines = open(f).read().split('\n')
idx = lines.index(decl)
rest = lines[:idx] + lines[idx + 1:]
# positions: after header (line 3) up to first line that starts a function body '{'
end = next(i for i, l in enumerate(rest) if l == '{') - 1
step = int(sys.argv[4]) if len(sys.argv) > 4 else 1
for pos in range(4, end, step):
    q = rest[:pos] + [decl] + rest[pos:]
    t = f.replace('.c', '_dp.c')
    open(t, 'w').write('\n'.join(q))
    out = subprocess.run(['python3', '-I', 'build/agent/r3w2/score.py', t, func], capture_output=True, text=True).stdout
    print(pos, int(out.split()[1]), rest[pos][:50] if pos < len(rest) else '', flush=True)
