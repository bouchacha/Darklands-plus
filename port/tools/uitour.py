#!/usr/bin/env python3
"""uitour.py - every tooltip beside what it describes (never over it nor over the pointer), and no
two of Darklands+'s panels over each other.

  uitour.py --game DIR [--exe EXE] [--screen 1600x900] [--step 64]

Starts the game (nctl --x11), Quickstarts, and on each screen perfcheck.py visits (a card, a shop,
the character sheet's pages, alchemy, the world map, the F12 window, a battle) rests the pointer on
a grid of points over the whole window, the tooltips shown at once (Tooltip delay: at once). The game
checks every frame where each tooltip went (ui/tipplace.cpp ui_tip_audit) and logs "tooltip over the
pointer" or "tooltip over what it describes", and whether two panels lie over each other
(ui/panelcheck.cpp: "panels overlap"), and whether the game's hand shows over a window of Darklands+
(two pointers); any such line is a FAIL, printed with the screen.
"""
import argparse, os, re, subprocess, sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from perfcheck import START, SCREENS   # (the same screens, reached the same way)

NCTL = [sys.executable, '-I', os.path.join(HERE, 'nctl.py')]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--game', required=True)
    ap.add_argument('--exe')
    ap.add_argument('--screen', default='1600x900')
    ap.add_argument('--step', type=int, default=64, help='the grid, in window pixels')
    a = ap.parse_args()
    name = 'uitour'
    W, H = (int(v) for v in a.screen.split('x'))
    subprocess.run(NCTL + ['stop', name], capture_output=True)
    env = dict(os.environ, NCTL_SCREEN=a.screen, DL_TIP_CHECK='1')
    cmd = NCTL + ['start', name, '--game', a.game, '--x11'] + (['--exe', a.exe] if a.exe else [])
    subprocess.run(cmd, env=env, capture_output=True, timeout=200)
    log = '/tmp/nctl/%s/out.txt' % name
    subprocess.run(NCTL + ['do', name, START + '; ucmd uoption:display.tooltip_delay=0; wait 1'], capture_output=True, timeout=300)
    fails = 0
    for title, steps in SCREENS:
        subprocess.run(NCTL + ['do', name, steps], capture_output=True, timeout=300)
        n0 = len(open(log, errors='replace').read())
        pts = ['ucmd uhover:%d,%d; wait 0.12' % (x, y) for y in range(a.step // 2, H, a.step) for x in range(a.step // 2, W, a.step)]
        for i in range(0, len(pts), 200):
            subprocess.run(NCTL + ['do', name, '; '.join(pts[i:i + 200])], capture_output=True, timeout=600)
        bad = [l for l in open(log, errors='replace').read()[n0:].splitlines() if l.startswith(('tooltip over', 'panels overlap', "the game's hand over"))]
        fails += len(bad)
        print('%-18s %s' % (title, 'OK' if not bad else 'FAIL'))
        for l in bad: print('    ' + l)
    subprocess.run(NCTL + ['stop', name], capture_output=True)
    print('every tooltip beside its thing, no panels over each other' if not fails else '%d misplaced' % fails)
    sys.exit(1 if fails else 0)


if __name__ == '__main__':
    main()
