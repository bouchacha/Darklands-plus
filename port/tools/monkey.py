#!/usr/bin/env python3
"""monkey.py NAME MINUTES --game DIR [--seed S] [--x11] [--exe EXE] [--load SAVEDIR:FILE]: soak test. Plays at random in an nctl session of the
native build: picks card options, travels on the map, lets battles run, leaves screens. Logs every
step's mode/card to NAME.log; stops and reports when the process dies or nothing changes for long."""
import sys, os, time, random, re, argparse, subprocess
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import nctl

ap = argparse.ArgumentParser(); ap.add_argument('name'); ap.add_argument('minutes', type=float)
ap.add_argument('--seed', type=int, default=None); ap.add_argument('--x11', action='store_true')
ap.add_argument('--game', default=os.environ.get('DARKLANDS_DIR'), help='the Darklands folder (copied, not changed; default $DARKLANDS_DIR)')
ap.add_argument('--exe', default=os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'build', 'port', 'darklands'))
ap.add_argument('--post', default='', help='script to run after the load')
ap.add_argument('--load', default='', help='SAVEDIR:FILE - copy SAVEDIR into the game and load FILE after the start')
a = ap.parse_args()
rnd = random.Random(a.seed)
name = a.name
d = nctl.sdir(name)
log = open(os.path.join(os.path.dirname(d), name + '.log'), 'w')
def L(*x):
    print(time.strftime('%H:%M:%S'), *x, file=log, flush=True)

nctl.start(name, a.exe, a.game, x11=a.x11)
out = os.path.join(d, 'out.txt')
if a.load:
    import shutil
    sd, sf = a.load.rsplit(':', 1)
    os.makedirs(os.path.join(d, 'game', 'SAVES'), exist_ok=True)
    for f in os.listdir(sd): shutil.copy(os.path.join(sd, f), os.path.join(d, 'game', 'SAVES', f))
    nctl.do(name, 'QUICKSTART; click; wait 3; ucmd uload:%s; wait 8; click; wait 3' % sf)
    if a.post: nctl.do(name, a.post)
else:
    nctl.do(name, 'QUICKSTART; click; wait 3; click; wait 3; click; wait 3; click; wait 3; click; wait 4')

def ask(cmds, wait=0.4):
    n0 = os.path.getsize(out)
    nctl.do(name, '; '.join('ucmd ' + c for c in cmds) + '; wait %g' % wait)
    return open(out, encoding='latin1', errors='replace', newline='').read()[n0:].replace('\r', '')

def alive():
    try:
        st = open('/proc/' + open(os.path.join(d, 'pid')).read().strip() + '/status').read()
        return 'zombie' not in st
    except Exception: return False

t_end = time.time() + a.minutes * 60
last_sig, same_since, steps = None, time.time(), 0
recent = []
modes = {}
while time.time() < t_end:
    if not alive():
        try:
            v = int(open('/proc/' + open(os.path.join(d, 'pid')).read().strip() + '/stat').read().rsplit(')', 1)[1].split()[-1])
        except Exception: v = -1
        L('PROCESS ENDED, exit status %d signal %d' % ((v >> 8) & 255, v & 127)); print(name, 'ENDED', v); break
    s = ask(['umode:', 'ucard:', 'uparty:'])
    m = re.search(r'^mode (\d+)', s, re.M); mode = int(m.group(1)) if m else -1
    c = re.search(r'^card (\S*) n (-?\d+)(?: menu (\d) maplive (\d))?', s, re.M); card, n = (c.group(1), int(c.group(2))) if c else ('?', 0)
    menu = c and c.group(3) == '1'; maplive = c and c.group(4) == '1'
    p = re.search(r'^party .*', s, re.M); party = p.group(0) if p else ''
    paused = ' paused 1 ' in party
    modes[mode] = modes.get(mode, 0) + 1
    sig = (mode, card, party.split(':', 1)[-1])
    if sig != last_sig: last_sig, same_since = sig, time.time()
    elif time.time() - same_since > 240 and mode != 4:
        L('STUCK for 4 min:', sig); print(name, 'STUCK', sig)
        nctl.do(name, 'wshot stuck_%d' % steps); same_since = time.time()
        # a screen the rules above do not know (the party roster at an inn, say): clicks anywhere, Escape, Return
        nctl.do(name, '; '.join(rnd.choice(['at %d %d; click; wait 1.5' % (rnd.randrange(60, 320), rnd.randrange(10, 200)),
                                            'key Escape; wait 1', 'key Return; wait 1', 'key r; wait 1']) for _ in range(12)))
        continue
    steps += 1
    L('mode', mode, 'card', card, 'n', n, 'menu' if menu else '', 'map' if maplive else '', party[:120])
    if a.x11 and rnd.random() < 0.08:          # the interface: tabs of the F12 menu, the maps, help
        act = rnd.choice(['tabs', 'map', 'level', 'help'])
        L('ui', act)
        if act == 'tabs': nctl.do(name, 'key F12; wait 0.5; ' + '; '.join('utab %d; wait 0.6' % rnd.randrange(14) for _ in range(4)) + '; key F12; wait 0.5')
        elif act == 'map': nctl.do(name, 'ucmd umap:; wait 1.5; ucmd umap:; wait 0.5')
        elif act == 'level': nctl.do(name, 'ucmd ulevelmap:; wait 1.5; ucmd ulevelmap:; wait 0.5')
        elif act == 'help': nctl.do(name, 'ucmd uhelp:; wait 1; ucmd uhelp:; wait 0.5')
        continue
    if mode == 2 and (menu or (c and c.group(3) is None and n > 0)) and not maplive: mode = 1   # a card over the travel map (village, encounter)
    if mode == 1 and not menu and c and c.group(3) is not None: n = 0      # the card's text, not its options: read on
    if mode == 1: recent.append(card); del recent[:-25]
    if mode == 1 and n > 1 and len(recent) == 25 and len(set(recent)) == 1 and rnd.random() < 0.5:   # going round in circles (a mine's card): another way out
        nctl.do(name, 'ucmd ucardkey:%d; wait 2; at %d %d; click; wait 6' % (rnd.randrange(n) + 1, rnd.randrange(80, 300), rnd.randrange(20, 190))); continue
    if mode in (0, 3) and ': 0/0,0/0 0/0,0/0' in party and rnd.random() < 0.7:   # no party: the title menu
        if a.load:                               # (the party fell: the save again, to go on soaking where it matters)
            L('no party: loading', a.load); nctl.do(name, 'key Escape; wait 1; QUICKSTART; click; wait 3; ucmd uload:%s; wait 8; click; wait 3' % a.load.rsplit(':', 1)[1]); continue
        nctl.do(name, 'at 160 62; click; wait 6; click; wait 3; click; wait 3'); continue
    if mode == 1:
        if n > 0 and rnd.random() < 0.85:
            k = rnd.randrange(n) + 1
            r = ask(['ucardkey:%d' % k], 0.2)
            if 'no option' in r: nctl.do(name, 'at 304 12; click; wait 1')
            else: nctl.do(name, 'wait 1.5')
        else:
            nctl.do(name, 'at 304 12; click; wait 1')
    elif mode == 2:
        r = rnd.random()
        if r < 0.5 and not (len(recent) == 25 and len(set(recent)) == 1): nctl.do(name, 'ucmd travelnear:%d; wait 8' % rnd.choice([0, 1, 2, 3, 4, 6, 8, 9]))
        else: nctl.do(name, 'at %d %d; click; wait 6' % (rnd.randrange(80, 300), rnd.randrange(20, 190)))
    elif mode == 4:
        if time.time() - same_since > 120 and rnd.random() < 0.5:     # (a castle or dungeon to explore: walking as well)
            L('battle stuck for 2 min: striking the enemies'); nctl.do(name, '; '.join('ucmd uhit:0,%d,40,0' % t for t in range(5, 25)) + '; wait 4')
            if paused: nctl.do(name, 'key space; wait 4')
        elif time.time() - same_since > 120:
            L('battle stuck for 2 min: walking'); nctl.do(name, ('key space; wait 1; ' if paused else '') + 'at %d %d; click; wait 8' % (rnd.randrange(70, 315), rnd.randrange(15, 195)))
        elif rnd.random() < 0.06: nctl.do(name, 'key e; wait 2; click; wait 1.5')      # Enemy Info
        elif paused: nctl.do(name, 'key space; wait 6')
        elif time.time() - same_since > 20:     # nothing happens (a castle or dungeon to explore): walk somewhere
            nctl.do(name, 'at %d %d; click; wait 6' % (rnd.randrange(70, 315), rnd.randrange(15, 195)))
        else: nctl.do(name, 'wait 6')
    elif mode == 3:
        if rnd.random() < 0.35:                 # a native screen's actions (buy, sell, take, give, ready, brew ...)
            nctl.do(name, 'ucmd unspush:%d,%d,%d,%d; wait 1.5' % (rnd.randrange(1, 9), rnd.randrange(0, 12), rnd.choice([0, 1, 2, 3, 10, 255]), rnd.choice([0, 1, 255])))
        else:
            nctl.do(name, rnd.choice(['key Escape; wait 1.5', 'key L; wait 1.5', 'key Return; wait 1.5', 'key a; wait 1.5', 'key s; wait 1.5', 'key t; wait 1.5', 'key r; wait 1.5',
                                      'at %d %d; click; wait 1.5' % (rnd.randrange(60, 310), rnd.randrange(10, 190))]))
    else:
        nctl.do(name, rnd.choice(['key Return; wait 1.5', 'at 160 100; click; wait 1.5', 'key space; wait 1.5', 'key Escape; wait 1.5', 'at 160 62; click; wait 3']))
L('done', steps, 'steps, modes', modes)
print(name, 'done', steps, 'steps, modes', modes)
tail = open(out, encoding='latin1', errors='replace').read()[-3000:]
L('output tail:\n' + tail)
nctl.stop(name)
