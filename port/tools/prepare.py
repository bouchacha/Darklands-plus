#!/usr/bin/env python3
"""prepare.py - make the game module (runtime/split.h) from the player's own DARKLAND.EXE.

  prepare.py --game DARKLANDS_FOLDER --out MODULE [--tcc TCC] [--work DIR] [--id]

Runs on the player's machine, once (the program starts it when the module is missing or was
made by another translator): the analysis of DARKLAND.EXE (tools/analyze.py), its translation
to C (port/recomp), and TinyCC, which compiles that C into darklands_game.dll /
libdarklands_game.so. Nothing of the game is in a Darklands+ download; all of it comes from
here. Progress lines for the program's window start with "@@ <percent> ".

--id prints the translator id (the hash of port/tools/translator_files.txt's files) and exits.
"""
import sys, os, subprocess, shutil, tempfile, hashlib, argparse, time

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, '..', '..'))      # the repository, or darklands_plus/src
GOG_48307_MD5 = '0d51cc9458a6724e1bd34a9fe0d81e7f'
# the game's drivers, translated too (gen.sh): GOG ships the FM drivers as .DC/.DB
DRIVERS = [('mgraphic', 'MGRAPHIC.EXE'), ('misc', 'MISC.EXE'), ('none_c', 'NSOUND.DLC'),
           ('roland_c', 'RSOUND.DLC'), ('roland_b', 'RSOUND.DLB'), ('opl3_c', 'PSOUND.DC'),
           ('opl3_b', 'PSOUND.DB'), ('adlib_c', 'ASOUND.DC'), ('adlib_b', 'ASOUND.DB'),
           ('spk_c', 'ISOUND.DLC'), ('spk_b', 'ISOUND.DLB')]


def translator_id():
    h = hashlib.sha1()
    for line in open(os.path.join(HERE, 'translator_files.txt')):
        p = line.split('#')[0].strip()
        if p:
            h.update(open(os.path.join(ROOT, p), 'rb').read())
    return h.hexdigest()[:12]


def progress(pct, msg):
    print('@@ %d %s' % (pct, msg), flush=True)


def find_file(folder, name):
    """name in folder, whatever its case (the game's files are upper case on GOG)"""
    p = os.path.join(folder, name)
    if os.path.exists(p):
        return p
    for f in os.listdir(folder):
        if f.lower() == name.lower():
            return os.path.join(folder, f)
    return None


def run(cmd, what, pct0, pct1, expect_s):
    """a step as a child process; the bar moves on with the time it usually takes"""
    t0 = time.time()
    flags = getattr(subprocess, 'CREATE_NO_WINDOW', 0)     # Windows: no console window for the steps
    p = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, creationflags=flags)
    import threading
    out = []
    th = threading.Thread(target=lambda: out.extend(p.stdout.read().decode('latin1', 'replace').splitlines()))
    th.start()
    last = -1
    while p.poll() is None:
        frac = min(0.95, (time.time() - t0) / expect_s)
        pct = int(pct0 + (pct1 - pct0) * frac)
        if pct != last:
            progress(pct, what)
            last = pct
        time.sleep(0.25)
    th.join()
    for l in out[-30:]:
        print('   ', l, flush=True)
    if p.returncode != 0:
        raise SystemExit('%s failed (exit %d)' % (what, p.returncode))
    progress(pct1, what)


def find_tcc(given):
    if given:
        return given
    for c in (os.path.join(ROOT, '..', 'tcc', 'tcc.exe'), os.path.join(ROOT, '..', 'tcc', 'tcc')):
        if os.path.exists(c):
            return os.path.abspath(c)
    t = shutil.which('tcc')
    if t:
        return t
    raise SystemExit('TinyCC (tcc) not found')


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--game')
    ap.add_argument('--out')
    ap.add_argument('--tcc')
    ap.add_argument('--work')
    ap.add_argument('--id', action='store_true')
    a = ap.parse_args()
    tid = translator_id()
    if a.id:
        print(tid)
        return
    if not a.game or not a.out:
        ap.error('--game and --out are needed')
    exe = find_file(a.game, 'DARKLAND.EXE')
    if not exe:
        raise SystemExit('DARKLAND.EXE is not in %s' % a.game)
    md5 = hashlib.md5(open(exe, 'rb').read()).hexdigest()
    if md5 != GOG_48307_MD5:
        print('note: DARKLAND.EXE is not the GOG version 483.07 (md5 %s); trying anyway' % md5, flush=True)
    tcc = find_tcc(a.tcc)
    work = a.work or tempfile.mkdtemp(prefix='dlprep')
    py = [sys.executable, '-I']
    progress(1, 'Reading DARKLAND.EXE')
    an = os.path.join(work, 'analysis')
    run(py + [os.path.join(ROOT, 'tools', 'analyze.py'), exe, an], 'Reading DARKLAND.EXE', 1, 50, 50)
    gen = os.path.join(work, 'gen')
    os.makedirs(gen, exist_ok=True)
    run(py + [os.path.join(ROOT, 'port', 'recomp', 'recomp16.py'), exe, os.path.join(an, 'analysis.pickle'), gen],
        'Translating the game', 50, 90, 45)
    drv = ['%s=%s' % (k, find_file(a.game, f)) for k, f in DRIVERS if find_file(a.game, f)]
    run(py + [os.path.join(ROOT, 'port', 'recomp', 'recomp_drv.py'), gen] + drv, 'Translating the drivers', 90, 93, 5)
    rt = os.path.join(ROOT, 'port', 'runtime')
    srcs = sorted(os.path.join(gen, f) for f in os.listdir(gen) if f.endswith('.c'))
    srcs += [os.path.join(rt, f) for f in ('cpu.c', 'overrides.c', 'gensplit.c')]
    tmp = a.out + '.new'
    rsp = os.path.join(work, 'module.rsp')
    with open(rsp, 'w') as f:      # one argument per line (paths may hold spaces)
        for x in ['-shared', '-w', '-DDL_GENSPLIT', '-DGEN_TRANSLATOR="%s"' % tid, '-I' + rt, '-I' + gen, '-o', tmp] + srcs:
            f.write('"%s"\n' % x.replace('\\', '/').replace('"', '\\"'))
    run([tcc, '@' + rsp], 'Compiling', 93, 99, 15)
    if os.path.exists(a.out):
        os.remove(a.out)
    os.replace(tmp, a.out)
    if not a.work:
        shutil.rmtree(work, ignore_errors=True)
    progress(100, 'Done')


if __name__ == '__main__':
    main()
