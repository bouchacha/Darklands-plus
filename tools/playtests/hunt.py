#!/usr/bin/env python3
"""Travel back and forth on the overland map until an encounter card (or anything that is not the
map) appears, for each lockstep session independently. Uses tools/playtests/lockstep.py sessions.
  LOCKSTEP_DIR=... hunt.py [--max N] [--pts x1,y1,x2,y2] NAME [NAME2]
Stops a session when the top border of the overland map is gone; prints NAME: steps.
Shots: $LOCKSTEP_DIR/NAME/hunt.png (last).
"""
import sys, os, threading, argparse
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import lockstep
from PIL import Image
SIG = [(70, 5, (117, 77, 40)), (100, 5, (125, 125, 125)), (150, 5, (85, 60, 28))]

def on_map(png):
    im = Image.open(png).convert('RGB')
    return all(im.getpixel((x, y)) == c for x, y, c in SIG)

def hunt(name, mx, pts, res):
    for i in range(mx):
        x, y = pts[i % 2]
        lockstep.do(name, 'at %d %d; click; wait 15; shot hunt' % (x, y))
        if not on_map(os.path.join(lockstep.D, name, 'hunt.png')):
            res[name] = i + 1; return
    res[name] = None

if __name__ == '__main__':
    ap = argparse.ArgumentParser(); ap.add_argument('--max', type=int, default=40)
    ap.add_argument('--pts', default='180,90,240,130'); ap.add_argument('names', nargs='+')
    a = ap.parse_args(); p = [int(v) for v in a.pts.split(',')]
    res = {}
    ts = [threading.Thread(target=hunt, args=(n, a.max, [(p[0], p[1]), (p[2], p[3])], res)) for n in a.names]
    [t.start() for t in ts]; [t.join() for t in ts]
    for n in a.names: print('%s: %s' % (n, res.get(n)))
