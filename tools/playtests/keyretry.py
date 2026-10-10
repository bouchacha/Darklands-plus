#!/usr/bin/env python3
"""keyretry.py: press a key in ONE lockstep session until the screen changes (xdotool key events
reach the game only about half the time). Uses tools/playtests/lockstep.py sessions ($LOCKSTEP_DIR).
  LOCKSTEP_DIR=... keyretry.py NAME KEY SHOT [--tries N] [--wait S] [--park X,Y]
Takes a reference shot, then 'key KEY; wait S; shot SHOT' until SHOT differs from the reference in
more than --min (60) pixels (a menu highlight moving is ~100-400). Prints NAME KEY: tries (or FAILED). --park first moves the game pointer
(keys are ignored while it is over a menu option; the party sidebar, e.g. 30,185, is safe).
Reliability: high for keys that change the screen (F10, alt+s, alt+p, alt+q, Escape on a menu).
Careful with toggles (F10 opens AND closes the menu): one changed screen = one key that got through.
"""
import sys, os, argparse
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import lockstep
from PIL import Image, ImageChops

def changed(a, b, mn):
    d = ImageChops.difference(Image.open(a).convert('RGB'), Image.open(b).convert('RGB')).convert('L')
    n = sum(d.histogram()[1:])
    return n > mn

if __name__ == '__main__':
    ap = argparse.ArgumentParser(); ap.add_argument('name'); ap.add_argument('key'); ap.add_argument('shot')
    ap.add_argument('--tries', type=int, default=6); ap.add_argument('--wait', type=float, default=3)
    ap.add_argument('--park', default=None)
    ap.add_argument('--min', type=int, default=60, help='changed pixels that count as a change')
    a = ap.parse_args()
    pre = ('at %s %s; wait 1; ' % tuple(a.park.split(','))) if a.park else ''
    lockstep.do(a.name, pre + 'shot _ref')
    out = os.path.join(lockstep.D, a.name)
    for i in range(a.tries):
        lockstep.do(a.name, 'key %s; wait %s; shot %s' % (a.key, a.wait, a.shot))
        if changed(os.path.join(out, '_ref.png'), os.path.join(out, a.shot + '.png'), a.min):
            print('%s %s: %d' % (a.name, a.key, i + 1)); break
    else:
        print('%s %s: FAILED' % (a.name, a.key))
