#!/usr/bin/env python3
"""perfcheck.py - how long the interface takes to draw on each screen (run after every new feature).

  perfcheck.py --game DIR [--exe EXE] [--screen 2560x1440] [--out DIR]

Starts the game (nctl --x11, DL_PRESENT_TIME on), Quickstarts and visits the screens the interface
draws (a card, a shop, the character sheet's pages, the alchemy screen, the world map, a battle, the
F12 window), waits on each, and prints per screen the average and worst milliseconds a frame took to
prepare (the game's picture), build the interface (Darklands+'s own drawing code, the part that
grows with features; the drawing of it, done by the graphics card at home, is left out since the
container draws in software) and present. "build" is the number to watch: a budget of 3 ms average, 8 ms worst, at 2560x1440. Over budget: FAIL.
"""
import argparse, os, re, subprocess, sys, time

HERE = os.path.dirname(os.path.abspath(__file__))
NCTL = [sys.executable, '-I', os.path.join(HERE, 'nctl.py')]
UI_AVG, UI_WORST, VTX = 3.0, 8.0, 60000
START = 'QUICKSTART; key Return; wait 12; key Return; wait 3; key Return; wait 3; key Return; wait 3; key Return; wait 3; key Return; wait 3'
SCREENS = [
    ('a card', 'wait 4'),
    ('a shop', 'ucmd urand:7; ucmd ugoto:0; wait 18; at 110 63; wait 4'),   # (the merchants: every town has them)
    ('sheet: items', 'at 20 70; click; wait 3; at 250 80; wait 4'),
    ('sheet: skills', 'at 254 15; click; wait 4'),
    ('sheet: saints', 'at 280 15; click; wait 4'),
    ('sheet: formulae', 'at 305 15; click; wait 4; key Escape; wait 2; ucmd unspush:5; wait 4'),
    ('alchemy', 'ucmd ualchemy:2,8; at 30 190; wait 3; click; wait 3; click; wait 3; click; wait 3; click; wait 4'),
    ('world map', 'ucmd unspush:5; wait 3; ucmd umap:1; wait 2; ucmd ufind:Lubeck; wait 5; ucmd umap:1; wait 2'),
    ('F12 window', 'key F12; wait 4; key F12; wait 2'),
    ('a battle', 'ucmd ufight:48,9,7,2,8; ucmd ugoto:24; wait 14; ucmd uunpause:; wait 6'),
]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--game', required=True)
    ap.add_argument('--exe')
    ap.add_argument('--screen', default='2560x1440')
    a = ap.parse_args()
    name = 'perf'
    subprocess.run(NCTL + ['stop', name], capture_output=True)
    env = dict(os.environ, DL_PRESENT_TIME='10', NCTL_SCREEN=a.screen)
    cmd = NCTL + ['start', name, '--game', a.game, '--x11'] + (['--exe', a.exe] if a.exe else [])
    subprocess.run(cmd, env=env, capture_output=True, timeout=200)
    log = '/tmp/nctl/%s/out.txt' % name
    subprocess.run(NCTL + ['do', name, START], capture_output=True, timeout=300)
    fails = 0
    print('%-18s %8s %8s %9s %8s' % ('screen', 'build', 'b worst', 'vertices', 'frame'))
    for title, steps in SCREENS:
        n0 = len(open(log, errors='replace').read().splitlines())
        subprocess.run(NCTL + ['do', name, steps], capture_output=True, timeout=300)
        lines = open(log, errors='replace').read().splitlines()[n0:]
        st = [l for l in lines if l.startswith('present:')][-3:]     # (the last few hundred frames: the screen settled)
        if not st:
            print('%-18s (no frames)' % title); continue
        ui = [float(re.search(r'build ([\d.]+)', l).group(1)) for l in st]
        uiws = [float(re.search(r'build worst ([\d.]+)', l).group(1)) for l in st]
        avg = [float(re.search(r'present: ([\d.]+)', l).group(1)) for l in st]
        worst = [float(re.search(r'([\d.]+) ms worst', l).group(1)) for l in st]
        uiw = max(uiws)
        vtx = max(int(re.search(r'max (\d+)\)', l).group(1)) for l in st)
        bad = sum(ui) / len(ui) > UI_AVG or uiw > UI_WORST
        fails += bad
        bad = bad or vtx > VTX
        fails += bad and not (sum(ui) / len(ui) > UI_AVG or uiw > UI_WORST)
        print('%-18s %8.2f %8.2f %9d %8.2f %s' % (title, sum(ui) / len(ui), uiw, vtx, sum(avg) / len(avg), 'FAIL' if bad else ''))
    subprocess.run(NCTL + ['stop', name], capture_output=True)
    print('%s (ui budget: %.0f ms average, %.0f ms over 100 frames)' % ('all within budget' if not fails else '%d over budget' % fails, UI_AVG, UI_WORST))
    sys.exit(1 if fails else 0)


if __name__ == '__main__':
    main()
