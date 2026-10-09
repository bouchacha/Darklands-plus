"""score.py FILE.c _xxxx : print edit distance between original and compiled normalized instruction lists"""
import sys, os
REPO = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..'))
sys.path.insert(0, os.path.join(REPO, 'tools'))
os.chdir(REPO)
import fmatch, io, contextlib

def norm(i):
    s = i.mnemonic + ' ' + i.op_str
    if i.mnemonic.startswith('j') or i.mnemonic == 'loop':
        s = i.mnemonic + ' X'
    return s

def show_diff(orig, code, base, *a, **k):
    import capstone
    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_16)
    A = [norm(x) for x in md.disasm(orig, base)]
    B = [norm(x) for x in md.disasm(code, base)]
    prev = list(range(len(B) + 1))
    for i in range(1, len(A) + 1):
        cur = [i] + [0] * len(B)
        for j in range(1, len(B) + 1):
            cur[j] = min(prev[j] + 1, cur[j - 1] + 1, prev[j - 1] + (A[i - 1] != B[j - 1]))
        prev = cur
    print('SCORE', prev[-1])
    if len(sys.argv) > 3:
        print('\n'.join(B))

fmatch.show_diff = show_diff
buf = io.StringIO()
with contextlib.redirect_stdout(buf):
    fmatch.check(sys.argv[1], True, None, only=[sys.argv[2]])
out = buf.getvalue()
if 'SCORE' in out:
    print(out[out.index('SCORE'):].strip())
elif 'functions match' in out:
    print('SCORE 0')
else:
    print('SCORE 999')
