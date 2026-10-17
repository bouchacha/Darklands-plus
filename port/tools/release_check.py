#!/usr/bin/env python3
"""release_check.py - a scripted session through the parts of Darklands+ that a release must not
break, in a window (native screens are drawn only there), on your own game files.

  release_check.py --game /path/to/DARKLAND [--exe build/port/darklands] [--out DIR]
  (an ASAN build, make ASAN=1, also reports memory errors: --exe build/port_asan/darklands)

It starts the game, Quickstart, reads the opening cards, walks to the merchants with Go to
(through the native cards), sells and buys on the native shop screen, opens a member's native
sheet from it and back, leaves, fights a test battle and shares out its loot, and opens the alchemy screen. Each step is OK or FAILED by the
log; screenshots go to --out. Needs Xvfb (port/tools/nctl.py --x11). The game folder is
copied, not changed.
"""
import sys, os, subprocess, argparse, re, time, shutil

HERE = os.path.dirname(os.path.abspath(__file__))
TOP = os.path.abspath(os.path.join(HERE, '..', '..'))
NCTL = [sys.executable, '-I', os.path.join(HERE, 'nctl.py')]
NAME = 'relcheck'
BAD = re.compile(r'ERROR: AddressSanitizer|runtime error:|FATAL|unmatched return|CRASH|hook: .*(hooked twice|too many)|nscreen: no game function')

# test hits on every enemy, 30 rounds (an unlucky battle, a slow build)
HITS = '; '.join('; '.join('ucmd uhit:1,%d,40,0' % c for c in range(5, 13)) + '; wait 1' for _ in range(30))

# (name, steps, patterns that must appear in the log after the steps)
STEPS = [
    ('start and Quickstart', 'QUICKSTART; at 160 62; click; wait 12; key Return; wait 3; key Return; wait 3; key Return; wait 3; key Return; wait 3; key Return; wait 3; wshot 1_card',
     [r'cardview: card \$', r'^card \$\S+ n \d+ menu 1'], 'TITLE'),   # (a first click in a new window may be lost)
    ('Go to the merchants (native cards)', 'ucmd urand:7; ucmd ugoto:0; wait 18; wshot 2_shop', [r'cardview: chose option', r'shop: open']),   # (a fixed seed: no street encounter on the way)
    ('sell (native shop)', 'ucmd unspush:2,0,1; wait 2; wshot 3_sold', [r'shop: (Sold|Not sold)']),
    ('buy (native shop)', 'ucmd unspush:1,0,1; wait 2', [r'shop: (Bought|Not bought)']),
    ('a member\'s sheet from the shop', 'at 20 70; click; wait 4; wshot 4_sheet; key Escape; wait 3', [r'sheet: open', r'sheet: closed']),
    ('leave the shop', 'ucmd ufight:99,5; ucmd unspush:5; wait 4', [r'shop: closed']),
    ('a battle (test battle at the next card)', 'wait 3; key Return; wait 3; key Return; wait 3; key Return; wait 3; key Return; wait 10; ucmd umode:; wait 1; wshot 6_battle; ucmd uunpause:; wait 10; ucmd umode:; wait 1', [r'^mode 4$']),
    ('a sheet in the battle (F2)', 'key F2; wait 5; wshot 6b_sheet; key Escape; wait 4; ucmd umode:; wait 1', [r'sheet: open', r'sheet: closed', r'^mode 4$']),
    ('win it (test hits) and the loot (native)', HITS + '; ucmd uunpause:; wait 25; wshot 7_loot', [r'loot screen: open'],
     '; '.join('ucmd uhit:1,%d,40,0' % c for c in range(5, 13)) + '; ucmd uunpause:; wait 2'),   # (a foe that came late: hit it too; the hits land as the battle runs)
    ('share out and leave the loot', 'ucmd unspush:5; wait 2; ucmd unspush:6; wait 4', [r'loot screen: closed']),
    ('the alchemy screen (native)', 'ucmd ualchemy:0,8; at 30 190; wait 3; click; wait 3; click; wait 3; click; wait 3; click; wait 3', [r'alchemy: open']),
    ('leave the alchemy screen', 'wshot 8_alchemy; ucmd unspush:1,0; wait 1; ucmd unspush:5; wait 3', [r'alchemy: closed']),
    ('the store room (native)', 'ucmd ucache:; wait 4; wshot 9_store', [r'store room: open']),
    ('leave the store room', 'ucmd unspush:5; wait 3', [r'store room: closed']),
    ('calling on a saint (native)', 'ucmd usaint:0,0; wait 4; wshot 10_pray', [r'prayer: open']),
    ('not praying', 'ucmd unspush:4; wait 3', [r'prayer: not prayed']),
    ('save (Alt+S, native page)', 'ucmd down:Left Alt; wait 0.3; key s; wait 0.3; ucmd up:Left Alt; wait 3; wshot 11_save; key Return; wait 4', [r'saves: the Save page', r'saves: the page closed \(chosen\)']),
    ('the load list (Alt+L), cancelled', 'ucmd down:Left Alt; wait 0.3; key l; wait 0.3; ucmd up:Left Alt; wait 3; wshot 12_load; key Escape; wait 3', [r'saves: the Load page', r'saves: the page closed \(cancelled\)']),
    ('the menus (right click, Game)', 'at 200 100; rclick; wait 1; at 70 5; click; wait 1; wshot 13_menu; key Escape; wait 1', [r'.']),
    ('party info (Party menu, native)', 'ucmd umenu:3,0; wait 3; wshot 14_info; key Escape; wait 2', [r'party info: open', r'party info: closed']),
    ('marching order (Party menu, native)', 'ucmd umenu:3,1; wait 3; wshot 15_march; ucmd unspush:2,0; wait 1; ucmd unspush:3; wait 2', [r'marching order: open', r'marching order: closed \(set\)']),
]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--exe', default=os.path.join(TOP, 'build', 'port', 'darklands'))
    ap.add_argument('--game', required=True)
    ap.add_argument('--out', default=os.path.join(TOP, 'build', 'release_check'))
    ap.add_argument('--patience', type=float, default=60, help='seconds to wait for what a step should log (an ASAN build is slow)')
    a = ap.parse_args()
    os.makedirs(a.out, exist_ok=True)
    subprocess.run(NCTL + ['stop', NAME], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    env = dict(os.environ, DL_CARDVIEW_LOG='1', DL_LOOT_LOG='1')
    subprocess.run(NCTL + ['start', NAME, '--x11', '--exe', a.exe, '--game', a.game], stdout=subprocess.DEVNULL, check=True, env=env)
    log = os.path.join(os.environ.get('NCTL_DIR', '/tmp/nctl'), NAME, 'out.txt')
    t0 = time.time()                                # (a first start makes the game module first: minutes)
    while time.time() - t0 < 900:
        if os.path.exists(log) and 'driver mgraphic' in open(log, encoding='latin1', errors='replace').read():
            break
        time.sleep(2)
    time.sleep(3)
    failed = 0
    seen = 0
    try:
        for step in STEPS:
            name, steps, want = step[:3]
            probe = step[3] if len(step) > 3 else None     # sent again while waiting (a slow build)
            r = subprocess.run(NCTL + ['do', NAME, steps], capture_output=True, text=True, timeout=900)
            deadline = time.time() + a.patience          # (slow builds: wait for what is expected)
            while True:
                time.sleep(0.5)
                text = open(log, encoding='latin1', errors='replace').read().replace('\r', '')
                lines = text[seen:].splitlines()
                miss = [w for w in want if not any(re.search(w, l) for l in lines)]
                if not miss or time.time() > deadline:
                    break
                if probe == 'TITLE':                     # still at the title (the click was lost): click Quickstart again
                    p = 'key Return; wait 3; ucmd ucard:' if 'cardview: card' in text else 'at 160 62; click; wait 8; ucmd ucard:'
                    subprocess.run(NCTL + ['do', NAME, p], capture_output=True, text=True, timeout=120)
                elif probe:
                    subprocess.run(NCTL + ['do', NAME, probe], capture_output=True, text=True, timeout=120)
            seen = len(text)
            bad = [l for l in lines if BAD.search(l)]
            ended = 'SESSION ENDED' in r.stdout
            ok = not miss and not bad and not ended
            failed += not ok
            print('%-40s %s' % (name, 'OK' if ok else 'FAILED'))
            for w in miss: print('    missing in the log: %s' % w)
            for l in bad[:5]: print('    ' + l)
            if ended: print('    the session ended')
            for line in r.stdout.splitlines():
                if line.endswith('.png') and os.path.exists(line):
                    shutil.copy(line, a.out)
            if ended:
                break
    finally:
        subprocess.run(NCTL + ['stop', NAME], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    shutil.copy(log, os.path.join(a.out, 'log.txt'))
    print('%s; screenshots and the log in %s' % ('all passed' if not failed else '%d FAILED' % failed, a.out))
    return 1 if failed else 0


if __name__ == '__main__':
    sys.exit(main())
