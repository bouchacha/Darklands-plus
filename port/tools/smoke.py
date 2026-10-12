#!/usr/bin/env python3
"""smoke.py - quick checks of a Darklands+ build against your own game files, before a release.

  smoke.py --game /path/to/DARKLAND [--exe build/port/darklands] [--wine]

Runs a few short headless sessions (port/tools/nplay.py) and reports each as OK or FAILED:
the start and Quickstart, a battle, a quick save and load.
A session that stops with an error, or a program crash, fails it. The game folder is copied;
your own saves are not touched.
"""
import sys, os, subprocess, argparse, tempfile, re

HERE = os.path.dirname(os.path.abspath(__file__))
TOP = os.path.abspath(os.path.join(HERE, '..', '..'))
START = 'QUICKSTART; click; wait 3; click; wait 3'
CHECKS = [
    ('start', START + '; ucmd umode:; wait 1', [r'^mode 1$']),
    ('battle', START + '; ucmd ufight:99,5; click; wait 2; click; wait 2; click; wait 4; ucmd umode:; key space; wait 20; ucmd umode:; wait 1',
     [r'^mode 4$']),
    ('save and load', START + '; ucmd uquicksave:; wait 4; ucmd uquickload:; wait 10; ucmd umode:; wait 1',
     [r'Game saved', r'Loaded']),
]
BAD = re.compile(r'FATAL|unmatched return|CRASH|program exit: int 21h 4Ch code [1-9]')


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--exe', default=os.path.join(TOP, 'build', 'port', 'darklands'))
    ap.add_argument('--game', required=True, help='your Darklands folder (it is copied, not changed)')
    ap.add_argument('--wine', action='store_true')
    a = ap.parse_args()
    failed = 0
    for name, script, want in CHECKS:
        out = tempfile.mkdtemp(prefix='smoke')
        cmd = [sys.executable, '-I', os.path.join(HERE, 'nplay.py'), '--exe', a.exe, '--game', a.game, out, script]
        if a.wine:
            cmd.insert(3, '--wine')
        subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, timeout=600)
        err = open(os.path.join(out, 'stderr.txt'), encoding='latin1', errors='replace').read() if os.path.exists(os.path.join(out, 'stderr.txt')) else ''
        lines = err.replace('\r', '').splitlines()
        missing = [w for w in want if not any(re.search(w, l) for l in lines)]
        bad = [l for l in lines if BAD.search(l)]
        ok = not missing and not bad
        failed += not ok
        print('%-14s %s%s' % (name, 'OK' if ok else 'FAILED', '' if ok else '  (%s)' % '; '.join(bad[:2] + ['missing: ' + m for m in missing])))
        print('               log: %s' % os.path.join(out, 'stderr.txt'))
    print('all OK' if not failed else '%d FAILED' % failed)
    sys.exit(1 if failed else 0)


if __name__ == '__main__':
    main()
