#!/usr/bin/env python3
"""gotosweep.py - Go to (G) to every place of a town and on from there, in a running game: the walk
must arrive or stop for a reason the player is told (shut at night, not in this town), never fail
for want of a way ("not found in this city", "cannot find the way", "the way is blocked").

  gotosweep.py --game DIR [--save DKSAVExx.SAV] [--exe EXE] [--seed 7]

Loads the save (by default a Quickstart in Olmuetz) and quick-saves, then for each place the town has
(Go to's list): the quick save loaded again, Go to the place (a fixed random seed: no street
encounters on the way), and whether it got there.
"""
import argparse, os, re, subprocess, sys, time
HERE = os.path.dirname(os.path.abspath(__file__))
NCTL = [sys.executable, '-I', os.path.join(HERE, 'nctl.py')]
NAME = 'gotosweep'
FAIL = re.compile(r'Go to (.+) failed \((not found in this city|cannot find the way|the way is blocked|too many steps|something happened on the way|leave this place first)')

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--game', required=True); ap.add_argument('--exe'); ap.add_argument('--save'); ap.add_argument('--seed', type=int, default=7)
    a = ap.parse_args()
    subprocess.run(NCTL + ['stop', NAME], capture_output=True)
    env = dict(os.environ, NCTL_SCREEN='1600x900')
    subprocess.run(NCTL + ['start', NAME, '--game', a.game, '--x11'] + (['--exe', a.exe] if a.exe else []), env=env, capture_output=True, check=True)
    log = os.path.join(os.environ.get('NCTL_DIR', '/tmp/nctl'), NAME, 'out.txt')
    start = 'QUICKSTART; ' + ('ucmd uload:%s; wait 10' % a.save if a.save else 'key Return; wait 12; key Return; wait 3; key Return; wait 3; key Return; wait 3; key Return; wait 3')
    subprocess.run(NCTL + ['do', NAME, start + '; ucmd uquicksave:; wait 3; ucmd ugotoplaces:; wait 1'], capture_output=True, timeout=600)
    places = [int(x) for x in re.findall(r'gotoplaces:([ \d]*)', open(log, errors='replace').read())[-1].split()]
    fails = 0
    for d in places:
        n0 = len(open(log, errors='replace').read())
        # (each walk from the same moment: the quick save reloaded, so time and the night watch do not pile up)
        subprocess.run(NCTL + ['do', NAME, 'ucmd uquickload:; wait 6; ucmd urand:%d; ucmd ugoto:%d; wait 12; key Escape; wait 2; key Escape; wait 2' % (a.seed, d)], capture_output=True, timeout=300)
        text = open(log, errors='replace').read()[n0:]
        m = FAIL.search(text)
        went = re.search(r'Going to: ([^.]+)\.', text)
        print('%2d %-20s %s' % (d, went.group(1) if went else '-', 'FAILED: ' + m.group(2) if m else 'OK'), flush=True)
        fails += bool(m)
    subprocess.run(NCTL + ['stop', NAME], capture_output=True)
    print('%d failed' % fails)
    sys.exit(1 if fails else 0)

if __name__ == '__main__':
    main()
