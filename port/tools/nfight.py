#!/usr/bin/env python3
"""nfight.py NAME [--max N]: in an nctl --x11 session on the travel map, travel about until a
battle starts: on each card the option that leads to a fight is clicked (read from the crisp
text runs), other cards are passed with their last option. Stops in battle (no card text and
not the map) and prints 'battle'."""
import sys, os, time, argparse, re, functools
print = functools.partial(print, flush=True)
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import nctl
from PIL import Image
FIGHT = ('attack', 'fight', 'charge', 'press onward', 'draw weapon', 'stoutly refuse', 'ignore his warning', 'into the fray', 'battle')
SIG = [(70, 5, (117, 77, 40)), (100, 5, (125, 125, 125)), (150, 5, (85, 60, 28))]
ap = argparse.ArgumentParser(); ap.add_argument('name'); ap.add_argument('--max', type=int, default=80)
ap.add_argument('--pts', default='180,90,240,130')
ap.add_argument('--travel', default='', help='location kinds to auto-travel to in turn (e.g. 6,3,8)')
a = ap.parse_args(); p = [int(v) for v in a.pts.split(',')]
out = os.path.join(nctl.sdir(a.name), 'out.txt')

def texts():
    n0 = os.path.getsize(out)
    nctl.do(a.name, 'ucmd utext:; wait 0.5')
    s = open(out, encoding='latin1').read()[n0:]
    runs = []
    for l in s.splitlines():
        m = re.match(r'text (-?\d+) (-?\d+) (\d+) (\d+) ([0-9a-f]+) (.*)', l)
        if m: runs.append((int(m.group(1)), int(m.group(2)), int(m.group(3)), int(m.group(4)), m.group(5), m.group(6)))
    return runs

def on_map():
    nctl.do(a.name, 'shot nf')
    im = Image.open(os.path.join(nctl.sdir(a.name), 'nf.png')).convert('RGB')
    return all(im.getpixel((xx, yy)) == c for xx, yy, c in SIG)

last = {}
for i in range(a.max):
    if on_map():
        if a.travel:
            kinds = [int(k) for k in a.travel.split(',')]
            nctl.do(a.name, 'ucmd travelnear:%d; wait 25' % kinds[i % len(kinds)])
        else:
            x, y = (p[0], p[1]) if i % 2 == 0 else (p[2], p[3])
            nctl.do(a.name, 'at %d %d; click; wait 10' % (x, y))
        continue
    r = texts()
    def grey(c):
        v = int(c, 16); r, g, b = v >> 16, v >> 8 & 255, v & 255
        return abs(r - g) < 12 and abs(g - b) < 12 and 0x40 < r < 0xc0
    opts = [t for t in r if t[5].startswith('...') and not grey(t[4])]
    if not r:
        nctl.do(a.name, 'wait 2')
        if not texts() and not on_map():
            print('battle'); break
        continue
    if not opts:
        nctl.do(a.name, 'click; wait 3'); continue
    pick = None
    for keys in (FIGHT, ('open country', 'leave the city', 'leave town'), ('leave', 'go on', 'travel', 'continue', 'depart', 'sneer', 'avoid', 'ignore')):
        for t in opts:
            if any(k in t[5].lower() for k in keys) and t[5] != last.get('opt'): pick = t; break
        if pick: break
    if pick is None: pick = opts[-1] if opts[-1][5] != last.get('opt') else opts[0]
    last['opt'] = pick[5]
    print('card option:', pick[5])
    nctl.do(a.name, 'at %d %d; click; wait 4' % (pick[0] + 10, pick[1] + pick[3] // 2))
else:
    print('nothing')
