"""ptry.py FILE FUNC VARIANTFILE [JOBS]: like try.py but parallel, one tmp file per job."""
import sys, re, subprocess, os
from concurrent.futures import ThreadPoolExecutor
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from getf import getf
src, func, vf = sys.argv[1], sys.argv[2], sys.argv[3]
jobs = int(sys.argv[4]) if len(sys.argv) > 4 else 2
s = open(src).read(); old = getf(src, func)
variants = open(vf).read().split('\n====\n')
d = os.path.dirname(os.path.abspath(__file__))
def run(k):
    out = os.path.join(d, 'pt%d_%s' % (k % jobs, os.path.basename(src)))
    return k, out
import threading
locks = [threading.Lock() for _ in range(jobs)]
def work(k):
    j = k % jobs
    with locks[j]:
        out = os.path.join(d, os.environ.get('PT','pt') + '%d_%s' % (j, os.path.basename(src)))
        open(out, 'w').write(s.replace(old, variants[k].strip('\n')))
        r = subprocess.run(['python3', '-I', 'tools/fmatch.py', out], capture_output=True, text=True, cwd='/home/claude/darklands-re')
    line = [l for l in r.stdout.splitlines() if func in l]
    tot = r.stdout.splitlines()[0] if r.stdout else r.stderr[-300:]
    return '%d %s %s' % (k, tot.split(':')[-1].strip(), line[0].strip() if line else 'MATCH')
with ThreadPoolExecutor(jobs) as ex:
    for res in ex.map(work, range(len(variants))):
        print(res, flush=True)
