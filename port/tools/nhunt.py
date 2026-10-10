#!/usr/bin/env python3
"""nhunt.py NAME [--max N] [--pts x1,y1,x2,y2]: travel back and forth on the overland map of
an nctl session until something other than the map shows (an encounter card)."""
import sys, os, argparse
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import nctl
from PIL import Image
SIG = [(70, 5, (117, 77, 40)), (100, 5, (125, 125, 125)), (150, 5, (85, 60, 28))]
ap = argparse.ArgumentParser(); ap.add_argument('name'); ap.add_argument('--max', type=int, default=30)
ap.add_argument('--pts', default='180,90,240,130'); ap.add_argument('--wait', type=float, default=12)
a = ap.parse_args(); p = [int(v) for v in a.pts.split(',')]
for i in range(a.max):
    x, y = (p[0], p[1]) if i % 2 == 0 else (p[2], p[3])
    nctl.do(a.name, 'at %d %d; click; wait %g; shot hunt' % (x, y, a.wait))
    im = Image.open(os.path.join(nctl.sdir(a.name), 'hunt.png')).convert('RGB')
    if not all(im.getpixel((xx, yy)) == c for xx, yy, c in SIG):
        print('found after', i + 1); break
else:
    print('nothing')
