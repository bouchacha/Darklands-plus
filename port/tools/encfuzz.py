#!/usr/bin/env python3
"""encfuzz.py ENCCHECK [SEED] [CASES]: feeds build/enccheck (port/tools/enccheck.c, built with
-fsanitize=address,undefined) mutated copies of the shipped encounter scripts; a batch whose run
fails (a sanitizer report, a crash) is kept in encfuzz-crash/ with its log."""
import random, subprocess, os, sys, glob, tempfile, shutil
HERE = os.path.dirname(os.path.abspath(__file__))
MOD = os.path.join(HERE, '..', 'content', 'New encounters')
seeds = [open(f, 'rb').read() for f in sorted(glob.glob(os.path.join(MOD, 'ENCOUNTERS', '*.TXT')))]
TOK = [b'(', b')', b'{', b'}', b',', b';', b'goto ', b'if ', b'then ', b'else ', b'exit', b'[', b']', b'option ', b'saint', b'potion ',
       b'0x7fffffff', b'-2147483648', b'9223372036854775807', b'/', b'%', b'*', b'-', b'not ', b'and ', b'or ', b'==', b'\t', b'\n', b'  ',
       b'random(', b'clamp(', b'skill(', b'battle Wilds, Wolf, 1, 99', b'text 99', b'goto start', b'[start]', b'x' * 300, b'$', b'#', b'\r',
       b'block 999', b'saints 1,2,3,4,5,6', b'items -5', b'vars ' + b'a,' * 40, b'set a = 1/0', b'who party', b'chance ', b'ok ', b'fail ']
exe = os.path.abspath(sys.argv[1])
rnd = random.Random(int(sys.argv[2]) if len(sys.argv) > 2 else 1)
total = int(sys.argv[3]) if len(sys.argv) > 3 else 2000
def mutate(d):
    d = bytearray(d)
    for _ in range(rnd.randint(1, 8)):
        op = rnd.randint(0, 6); n = len(d)
        if n == 0: d += rnd.choice(TOK); continue
        i = rnd.randrange(n)
        if op == 0: d[i] = rnd.randrange(256)
        elif op == 1: del d[i:i + rnd.randint(1, 40)]
        elif op == 2: d[i:i] = rnd.choice(TOK)
        elif op == 3: j = rnd.randrange(n); d[i:i] = d[j:j + rnd.randint(1, 200)]
        elif op == 4:
            lines = bytes(d).split(b'\n'); k = rnd.randrange(len(lines)); tail = lines[k:]; rnd.shuffle(tail); d = bytearray(b'\n'.join(lines[:k] + tail))
        elif op == 5: d[i:i] = rnd.choice(TOK) * rnd.randint(1, 300)
        else: d[i:i] = rnd.choice(seeds)[: rnd.randint(0, 3000)]
    return bytes(d)
env = dict(os.environ, ENC_MSG=os.path.join(MOD, 'MSGFILES'))
work = tempfile.mkdtemp(prefix='encfuzz')
bad = 0
for b in range(total // 50):
    files = []
    for k in range(50):
        p = os.path.join(work, 'c%d_%d.txt' % (b, k)); open(p, 'wb').write(mutate(rnd.choice(seeds))); files.append(p)
    r = subprocess.run([exe] + files, capture_output=True, timeout=120, env=env)
    if r.returncode != 0:
        bad += 1
        os.makedirs('encfuzz-crash', exist_ok=True)
        open('encfuzz-crash/batch%d.log' % b, 'wb').write(r.stderr[-6000:])
        for f in files: shutil.copy(f, 'encfuzz-crash/')
        print('batch', b, 'failed: see encfuzz-crash/batch%d.log' % b)
    for f in files: os.remove(f)
shutil.rmtree(work, ignore_errors=True)
print('%d cases, %d failed batches' % (total // 50 * 50, bad))
sys.exit(1 if bad else 0)
