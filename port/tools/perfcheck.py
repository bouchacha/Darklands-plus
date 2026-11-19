#!/usr/bin/env python3
"""perfcheck.py - how long the interface takes to draw on each screen (run after every new feature).

  perfcheck.py --game DIR [--exe EXE] [--screen 2560x1440] [--out DIR]

Starts the game (nctl --x11, DL_PRESENT_TIME on), Quickstarts and visits the screens the interface
draws (a card, a shop, the character sheet's pages, the alchemy screen, the world map, a battle, the
F12 window), waits on each, and prints per screen the average and worst milliseconds a frame took to
prepare (the game's picture), build the interface (Darklands+'s own drawing code, the part that
grows with features; the drawing of it, done by the graphics card at home, is left out since the
container draws in software) and present. "build" is the number to watch: a budget of 3 ms average, 8 ms worst, at 2560x1440. Over budget: FAIL.
A worst over 8 ms in only one of the last three windows is a one-time cost and is shown as "once",
not failed (the Saints page: 8 to 11 ms in one frame after it opens, then at most 2.3 ms over 30
seconds). More vertices than VTX is shown as "watch", not failed: a graphics card draws 60,000 with ease;
it says where the count grows (2026-11-19: alchemy and the world map, about 61,000 each; on the
alchemy screen the 36 item icons are 12,000 of them and the crisp text another 12,000).
"""
import argparse, os, re, subprocess, sys, time

HERE = os.path.dirname(os.path.abspath(__file__))
NCTL = [sys.executable, '-I', os.path.join(HERE, 'nctl.py')]
UI_AVG, UI_WORST, VTX = 3.0, 8.0, 60000
START = 'QUICKSTART; key Return; wait 12; key Return; wait 3; key Return; wait 3; key Return; wait 3; key Return; wait 3; key Return; wait 3'
SCREENS = [   # (each ends with its screen up, measured then; the next one closes it)
    ('a card', 'wait 4'),
    ('a shop', 'ucmd urand:7; ucmd ugoto:0; wait 18; at 110 63; wait 4'),   # (the merchants: every town has them)
    ('sheet: items', 'at 20 70; click; wait 3; at 250 80; wait 4'),
    ('sheet: skills', 'at 254 15; click; wait 4'),
    ('sheet: saints', 'at 280 15; click; wait 4'),
    ('sheet: formulae', 'at 305 15; click; wait 4'),
    ('alchemy', 'key Escape; wait 2; ucmd unspush:5; wait 4; ucmd ualchemy:2,8; at 30 190; wait 3; click; wait 3; click; wait 3; click; wait 3; click; wait 4'),
    ('world map', 'ucmd unspush:5; wait 3; ucmd umap:1; wait 2; ucmd ufind:Lubeck; wait 5'),
    ('F12 window', 'ucmd umap:1; wait 2; key F12; wait 4'),
    ('a battle', 'key F12; wait 2; ucmd ufight:48,9,7,2,8; ucmd ugoto:24; wait 14; ucmd uunpause:; wait 6'),
    ('loot screen', 'ucmd ukill:5; ucmd ukill:6; ucmd ukill:7; ucmd ukill:8; ucmd ukill:9; ucmd ukill:10; ucmd ukill:11; ucmd ukill:12; wait 18'),   # (the battle's eight foes felled)
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
    log = os.path.join(os.environ.get('NCTL_DIR', '/tmp/nctl'), name, 'out.txt')   # (nctl.py's own folder)
    subprocess.run(NCTL + ['do', name, START], capture_output=True, timeout=300)
    fails = 0
    print('%-18s %8s %8s %9s %8s' % ('screen', 'build', 'b worst', 'vertices', 'frame'))
    for title, steps in SCREENS:
        n0 = len(open(log, errors='replace').read().splitlines())
        subprocess.run(NCTL + ['do', name, steps + '; wait 3'], capture_output=True, timeout=300)   # (3 s more: a screen's first frames settle)
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
        over = sum(1 for w in uiws if w > UI_WORST)
        bad = sum(ui) / len(ui) > UI_AVG or over >= 2      # (a worst seen in one window only: a one-time cost)
        fails += bad
        note = 'FAIL' if bad else ' '.join(x for x in ('once' if over == 1 else '', 'watch' if vtx > VTX else '') if x)
        print('%-18s %8.2f %8.2f %9d %8.2f %s' % (title, sum(ui) / len(ui), uiw, vtx, sum(avg) / len(avg), note))
    subprocess.run(NCTL + ['stop', name], capture_output=True)
    print('%s (ui budget: %.0f ms average, %.0f ms worst in two of the last three windows; "watch": over %d vertices)' % ('all within budget' if not fails else '%d over budget' % fails, UI_AVG, UI_WORST, VTX))
    sys.exit(1 if fails else 0)


if __name__ == '__main__':
    main()
