#!/usr/bin/env python3
"""lockstep_sync.py: wrapper around lockstep.py (same commands) that
 - picks X displays from $DBASE (default 150) upward, so parallel `start`s (and other agents' runs, which
   use 77+) never race for the same display (two DOSBoxes on one display = keys/screens mixed up);
 - launches DOSBox on a 3-second clock boundary, so two sessions started together share the RNG seed
   (plain lockstep.py starts drift by the game-dir copy time and land in different cities ~1 in 3).
Usage: DBASE=150 python3 -I lockstep_sync.py start O orig & DBASE=170 python3 -I lockstep_sync.py start X <exe> & wait
       python3 -I lockstep_sync.py do "script" O X ; python3 -I lockstep_sync.py stop O X
Reliability: high (cities matched in every synced start, 2026-10-09).
"""
import sys, os
sys.path.insert(0, '/home/claude/darklands-re/tools/playtests'); sys.path.insert(0, '/home/claude/darklands-re/tools')
import play
def fd():
    for n in range(int(os.environ.get("DBASE","150")), 199):
        if not os.path.exists('/tmp/.X%d-lock' % n) and not os.path.exists('/tmp/.X11-unix/X%d' % n):
            return ':%d' % n
play._free_display = fd
import subprocess, time
_P = subprocess.Popen
def P(args, *a, **k):
    if args and args[0] == 'dosbox':
        t = time.time(); time.sleep(3 - (t % 3) + 0.1)   # start on a 3 s boundary so both sessions share the clock second
    return _P(args, *a, **k)
subprocess.Popen = P
sys.argv = ['lockstep.py'] + sys.argv[1:]
import runpy
runpy.run_path(os.path.join(os.path.dirname(os.path.abspath(__file__)), 'lockstep.py'), run_name='__main__')
