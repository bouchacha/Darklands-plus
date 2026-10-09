"""trys.py FILE FUNC OLD NEW1 [NEW2 ...]: like try.py but scores with score.py (edit distance); NEW may use \\n"""
import sys, subprocess
f, func, old = sys.argv[1], sys.argv[2], sys.argv[3].replace('\\n', '\n')
base = open(f).read()
assert old in base, 'old not found'
for new in sys.argv[4:]:
    new = new.replace('\\n', '\n')
    t = f.replace('.c', '_try.c')
    open(t, 'w').write(base.replace(old, new))
    out = subprocess.run(['python3', '-I', 'build/agent/r3w2/score.py', t, func], capture_output=True, text=True).stdout
    print('%5s  %s' % (out.split()[1] if out.startswith('SCORE') else 'ERR', new.replace('\n', ' | ')[:160]), flush=True)
