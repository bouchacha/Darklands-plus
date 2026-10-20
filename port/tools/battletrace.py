#!/usr/bin/env python3
"""battletrace.py - stress tests of battle movement (pathfinding in a melee crowd), on your own game files.

  battletrace.py run TRACE --game DIR [--exe EXE] [--save DKSAVExx.SAV] BATTLE [CMD ...]
  battletrace.py show TRACE [-v]

run: starts the game in an nctl --x11 session (port/tools/nctl.py; the game folder is copied, not
changed), Quickstart or a saved game, and fights a test battle (ucmd ufight: BATTLE is
battlefield,seed,type,quality,number[,type,quality,number x2], e.g. 30,5,10,2,8 for eight pirates
in a street; 46,7,42,2,8 wolves in a clearing; 48,9,7,2,8 bandits all round) with DL_BATTLE_TRACE
on (port/mods/pathfix.c), pressing Space when the battle stops for orders, until it ends (or 150 s).
CMDs are test commands sent first (upath:0,0 = the original pathfinding); @CLOCK:CMD sends one when
the battle's clock passes CLOCK (@600:uwalk:0,680,320 walks member 0 there; @200:!wshot x takes a
screenshot).

show: what the trace says. Stalls: a fighter that wants to walk keeps its spot for more than
twice its usual step interval and 4 ticks (who blocked it, walking or standing); planning: each
route's cost (P lines) and the original planner's (O); what each side did (walking, fighting,
standing without orders, the first blows); and the wall-clock time between combat ticks (K lines:
ticks over 4 ms, with the time in ai_costs and the original's route trackers).
"""
import sys, os, subprocess, time, argparse, collections, statistics

HERE = os.path.dirname(os.path.abspath(__file__))
NCTL = [sys.executable, '-I', os.path.join(HERE, 'nctl.py')]
NAME = 'btrace'


def run(a):
    out = '/tmp/nctl/%s/out.txt' % NAME
    def do(s): subprocess.run(NCTL + ['do', NAME, s], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, timeout=900)
    def log():
        try: return open(out, 'rb').read().replace(b'\0', b'').decode('latin1')
        except OSError: return ''
    def mode():
        n = len(log()); do('ucmd umode:; wait 1')
        for l in log()[n:].splitlines()[::-1]:
            if l.startswith('mode '): return int(l.split()[1])
        return -1
    def lastclk():
        try:
            with open(a.trace, 'rb') as f:
                f.seek(0, 2); n = f.tell(); f.seek(max(0, n - 400))
                ls = [l for l in f.read().decode('latin1').splitlines() if l.startswith('T ')]
                return int(ls[-1].split()[1]) if ls else -1
        except OSError: return -1
    extra = [c for c in a.cmd if not c.startswith('@')]
    timed = sorted((int(c[1:].split(':', 1)[0]), c.split(':', 1)[1]) for c in a.cmd if c.startswith('@'))
    if os.path.exists(a.trace): os.remove(a.trace)
    env = dict(os.environ, NCTL_SCREEN=os.environ.get('NCTL_SCREEN', '1600x900'), DL_BATTLE_TRACE=os.path.abspath(a.trace))
    start = NCTL + ['start', NAME, '--x11', '--game', a.game] + (['--exe', a.exe] if a.exe else [])
    subprocess.run(start, env=env, stdout=subprocess.DEVNULL, timeout=180)
    time.sleep(3)
    cmds = ['QUICKSTART', 'click', 'wait 3']
    if a.save: cmds += ['ucmd uload:%s' % a.save, 'wait 8']
    else: cmds += ['key Return; wait 10'] + ['key Return; wait 3'] * 5        # (the opening cards)
    cmds += ['ucmd ' + c for c in extra] + ['ucmd ufight:%s' % a.battle, 'ucmd ugoto:24', 'wait 12']
    do('; '.join(cmds))
    t0 = time.time(); seen = False; prev = None
    while time.time() - t0 < 150:
        m = mode()
        if m == 4:
            seen = True
            c = lastclk()
            while timed and c >= timed[0][0]:
                e = timed.pop(0)[1]; do(e[1:] if e.startswith('!') else 'ucmd ' + e)
            if c >= 0 and c == prev: do('key space')          # (a battle stops for orders when foes come into view)
            prev = c
        elif seen: break
        time.sleep(3)
    do('wshot end')
    print('battle %s after %.0f s; trace %s' % ('over' if seen and mode() != 4 else 'still on' if seen else 'not started', time.time() - t0, a.trace))
    subprocess.run(NCTL + ['stop', NAME], stdout=subprocess.DEVNULL)


def show(a):
    T = collections.defaultdict(list); P = []; O = []; K = []; C = []; at = collections.defaultdict(dict)
    for l in open(a.trace):
        p = l.split()
        if not p: continue
        if p[0] == 'T' and len(p) >= 13:     # clock i x y action moving target cbt_action anim tx ty us [blocker replan pathstep]
            r = tuple(map(int, p[1:])); T[r[1]].append(r); at[r[0]][r[1]] = r
        elif p[0] == 'P' and len(p) == 13: P.append(tuple(map(int, p[1:])))   # clock i x y tx ty target stuck steps expanded tests us
        elif p[0] == 'O' and len(p) == 5: O.append(tuple(map(int, p[1:])))    # clock i result us
        elif p[0] == 'K' and len(p) == 7: K.append(tuple(map(int, p[1:])))    # clock us costs_us costs_n trackers_us trackers_n
        elif p[0] == 'C' and len(p) == 7: C.append(tuple(map(int, p[1:])))    # clock i target steps expanded us (ai_costs)
    if not T: print('no battle in the trace'); return
    clks = sorted(at)
    print('%d ticks' % (clks[-1] - clks[0]))
    tot = collections.Counter(); worst = []; blockers = collections.Counter()
    for i in sorted(T):
        rows = T[i]; steps = []; last = None
        for r in rows:
            if last is None or (r[2], r[3]) != last: steps.append((r[0], (r[2], r[3]))); last = (r[2], r[3])
        iv = [b[0] - a_[0] for a_, b in zip(steps, steps[1:]) if b[0] - a_[0] < 40]
        med = statistics.median(iv) if iv else 8
        lim = 2 * med + 4
        start = None; pos = None; nst = 0; stall = 0; longest = 0; blk = 0
        for r in rows:
            want = r[5] == 1 or (r[4] & 1)
            if want and (r[2], r[3]) == pos and start is not None:
                if len(r) > 12 and r[12]: blk |= r[12]
                continue
            if start is not None:
                d = r[0] - start
                if d > lim:
                    nst += 1; stall += d - med; longest = max(longest, d); worst.append((d, i, start, pos, blk))
                    for k in range(13):
                        if blk >> k & 1:
                            b = at.get(start + 1, {}).get(k)
                            blockers['%s by %s (%s)' % ('party' if i < 5 else 'enemy', 'party' if k < 5 else 'enemy', 'walking' if b and b[5] else 'standing')] += 1
                    if not blk: blockers['nobody'] += 1
            start = r[0] if want else None; pos = (r[2], r[3]); blk = r[12] if len(r) > 12 and want else 0
        if a.v: print('%2d: %4d steps (every %.0f ticks), %2d stalls, %4d ticks stalled, longest %3d' % (i, len(steps), med, nst, stall, longest))
        tot['steps'] += len(steps); tot['nst'] += nst; tot['stall'] += stall; tot['longest'] = max(tot['longest'], longest)
    print('steps %d; stalls %d, %d ticks in all, longest %d' % (tot['steps'], tot['nst'], tot['stall'], tot['longest']))
    print('stalls by blocker:', dict(blockers))
    if a.v: print('worst stalls (ticks, who, clock, where, blockers):', sorted(worst, reverse=True)[:6])
    if P:
        us = sorted(p[11] for p in P)
        print('plans %d (%d around everyone), no way %d: median %.1f ms, 95%% %.1f ms, max %.1f ms, all %.0f ms'
              % (len(P), sum(1 for p in P if p[7]), sum(1 for p in P if p[8] < 0), us[len(us) // 2] / 1e3, us[int(len(us) * .95)] / 1e3, us[-1] / 1e3, sum(us) / 1e3))
    if O:
        uo = sorted(o[3] for o in O)
        print("the original planner %d times (%d found): median %.1f ms, max %.1f ms" % (len(O), sum(1 for o in O if o[2] == 1), uo[len(uo) // 2] / 1e3, uo[-1] / 1e3))
    if C:
        uc = sorted(c[5] for c in C)
        print("the AI's walking costs %d (%d with a way): median %.1f ms, max %.1f ms" % (len(C), sum(1 for c in C if c[3] >= 0), uc[len(uc) // 2] / 1e3, uc[-1] / 1e3))
    per = collections.Counter()
    for p in P: per[p[0]] += p[11]
    for o in O: per[o[0]] += o[3]
    if per: c, u = per.most_common(1)[0]; print('most planning in one tick: %.1f ms (clock %d)' % (u / 1e3, c))
    acts = collections.Counter(); first = {}; idle = collections.Counter(); walkt = collections.Counter()
    for i in sorted(T):
        rows = T[i]
        for x, y in zip(rows, rows[1:]):
            dt = y[0] - x[0]
            if dt <= 0 or dt > 5: continue
            if x[7] == 1: acts[i] += dt; first.setdefault(i, x[0])
            if x[4] == 0 and x[5] == 0: idle[i] += dt
            if x[5] == 1 or (x[4] & 1): walkt[i] += dt
    for side, ks in (('party', [i for i in T if i < 5]), ('enemies', [i for i in T if i >= 5])):
        print('%s: first blows at %s; walking %d ticks, fighting %d, standing without orders %d'
              % (side, sorted(first.get(i, -1) for i in ks), sum(walkt[i] for i in ks), sum(acts[i] for i in ks), sum(idle[i] for i in ks)))
    if K:
        print('slow ticks (over 4 ms): %d, the slowest %.0f ms; in ai_costs %.0f ms in all (max %.0f), in the original trackers %.0f ms (max %.0f)'
              % (len(K), max(k[1] for k in K) / 1e3, sum(k[2] for k in K) / 1e3, max(k[2] for k in K) / 1e3, sum(k[4] for k in K) / 1e3, max(k[4] for k in K) / 1e3))


ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
sub = ap.add_subparsers(dest='what', required=True)
r = sub.add_parser('run'); r.add_argument('trace'); r.add_argument('--game', required=True); r.add_argument('--exe'); r.add_argument('--save')
r.add_argument('battle'); r.add_argument('cmd', nargs='*')
s = sub.add_parser('show'); s.add_argument('trace'); s.add_argument('-v', action='store_true')
a = ap.parse_args()
run(a) if a.what == 'run' else show(a)
