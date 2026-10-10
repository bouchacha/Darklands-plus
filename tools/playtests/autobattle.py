#!/usr/bin/env python3
"""autobattle.py: let a tactical battle run unattended in lockstep session(s) ($LOCKSTEP_DIR).
  LOCKSTEP_DIR=... autobattle.py [--max N] [--every S] NAME [NAME2]
Every S seconds (default 12) takes shot NAME/ab_<i>.png; when the screen did not change since the
previous shot (battle paused: start of battle, 'Edged Wpns changed...' messages) it presses space.
Stops a session when the screen is the overland map (battle over and result card clicked away),
when the DOS prompt shows (game quit: prints DOS!), or after N rounds. Result card / loot screens
are left for the tester: the session stops at the first shot that has no sidebar portraits frame
(i.e. a card or the loot screen); look at the last ab_*.png.
Reliability: medium (some messages need more than one space).
"""
import sys, os, threading, argparse
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import lockstep
from PIL import Image, ImageChops

def img(n, s): return Image.open(os.path.join(lockstep.D, n, s + '.png')).convert('RGB')

def same(a, b):
    return sum(ImageChops.difference(a, b).convert('L').histogram()[1:]) < 30

def is_dos(im):   # text mode prompt: almost all black
    return im.resize((32, 20)).convert("L").histogram()[0] > 620

def battle_view(im):  # tactical view: no card frame at x=70..300; the card border is brown/orange
    # cards and the loot screen have the ornamented frame at the right edge (x=316)
    r, g, b = im.getpixel((316, 100))
    return not (r > 120 and g > 60 and b < 90 and r > g)

def run(n, mx, every, res):
    prev = None
    for i in range(mx):
        lockstep.do(n, 'wait %s; shot ab_%d' % (every, i))
        im = img(n, 'ab_%d' % i)
        if is_dos(im): res[n] = 'DOS! at %d' % i; return
        if not battle_view(im): res[n] = 'card/loot at ab_%d' % i; return
        if prev is not None and same(prev, im):
            lockstep.do(n, 'key space')
        prev = im
    res[n] = 'still running'

if __name__ == '__main__':
    ap = argparse.ArgumentParser(); ap.add_argument('--max', type=int, default=40)
    ap.add_argument('--every', default='12'); ap.add_argument('names', nargs='+')
    a = ap.parse_args(); res = {}
    ts = [threading.Thread(target=run, args=(n, a.max, a.every, res)) for n in a.names]
    [t.start() for t in ts]; [t.join() for t in ts]
    for n in a.names: print('%s: %s' % (n, res.get(n)))
