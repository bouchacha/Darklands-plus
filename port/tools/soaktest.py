#!/usr/bin/env python3
"""soaktest.py - a long automated playtest: plays the game for a while as a restless player would, and
reports what went wrong. Where release_check.py walks one fixed path, this one wanders: Go to round a
town's places, journeys to other towns (camps on the way, the new encounters forced now and then, each
card's choices taken at random), battles won, prayers, the alchemy screen, the character sheet, saves
and loads. Each action is followed by a look at the log; anything that must not happen is reported with
the actions before it, the log lines and a screenshot:

  - a "check:" line (mods/checks.c: memory written past a buffer, an encounter at every step, the party
    on water, the date going back, ...), a hook that left the stack wrong, a crash;
  - Go to that fails for want of a way, a Go to list that the cards contradict ("goto:"), a journey that
    stops getting nearer, an encounter script's error, a card text cut short;
  - the game standing still: the same screen, card and place for many actions in a row although
    something was clicked or pressed each time (a stuck screen or loop).

  soaktest.py --game DIR [--exe EXE] [--minutes 60] [--seed 1] [--out DIR] [--encounters MODDIR] [--load SAVE] [--name N]

--game: a game folder (copied, never changed); --encounters: a "New encounters" folder to put in its mods
and switch on (default: the repo's port/content/New encounters); --load: a saved game (a DKSAVE*.SAV, a
player's) to start from instead of Quickstart's party. The report is OUT/report.txt (OUT:
build/soaktest by default), screenshots in OUT, the whole log in OUT/log.txt. Exit status 1 when anything
was found.
"""
import argparse, os, random, re, shutil, subprocess, sys, time

HERE = os.path.dirname(os.path.abspath(__file__))
TOP = os.path.abspath(os.path.join(HERE, '..', '..'))
NCTL = [sys.executable, '-I', os.path.join(HERE, 'nctl.py')]
NAME = 'soak'
BAD = [
    (re.compile(r'^check: '), 'check'),
    (re.compile(r'FATAL|CRASH|unmatched return|interp: opcode|ERROR: AddressSanitizer|runtime error:'), 'crash'),
    (re.compile(r'hook: .*(left the stack wrong|hooked twice|too many)'), 'hook'),
    (re.compile(r'Go to .* failed \((not found in this city|cannot find the way|the way is blocked|too many steps)'), 'goto'),
    (re.compile(r'^goto: '), 'goto list'),
    (re.compile(r'travel: stopped, no nearer'), 'travel'),
    (re.compile(r'encounters: .*an error while running|encounters: .*\.TXT, line \d+'), 'encounter'),
    (re.compile(r'^cards: .* cut at'), 'text'),
    (re.compile(r'nscreen: no game function'), 'screen'),
]
STATE = re.compile(r'state: mode (-?\d+) screen (-?\d+) card (\S*) menu (\d+) options (\S+) place (-?\d+) tile (-?\d+),(-?\d+) day (-?\d+) hour (-?\d+) travel (\d+) goto (\d+) enc (\d+) members (\d+)')
TOWNS = list(range(0, 92))                      # (DARKLAND.LOC: the cities come first, 0x5c of them)
POS = {}                                        # town -> (x, y), from the game folder's DARKLAND.LOC (read in main)


def read_towns(game):
    import struct
    for n in os.listdir(game):
        if n.upper() == 'DARKLAND.LOC':
            d = open(os.path.join(game, n), 'rb').read()
            for i in TOWNS:
                r = d[2 + 58 * i:2 + 58 * (i + 1)]
                if len(r) == 58:
                    POS[i] = struct.unpack('<hh', r[4:8])
WILD_ENCOUNTERS = 90                            # (uenc: the wild ones are listed first in the New encounters' order - any index works)


class Soak:
    def __init__(self, a):
        self.a = a
        self.rng = random.Random(a.seed)
        self.log = os.path.join(os.environ.get('NCTL_DIR', '/tmp/nctl'), NAME, 'out.txt')
        self.seen = 0
        self.actions = []
        self.found = []
        self.shot = 0
        self.same = 0
        self.last_state = None

    def do(self, steps, timeout=600):
        self.actions.append(steps)
        self.actions = self.actions[-25:]
        try:
            return subprocess.run(NCTL + ['do', NAME, steps], capture_output=True, text=True, timeout=timeout).stdout
        except subprocess.TimeoutExpired:
            self.report('stuck', 'an action took longer than %d s: %s' % (timeout, steps), [])
            return ''

    def text(self):
        try:
            return open(self.log, encoding='latin1', errors='replace').read().replace('\r', '')
        except OSError:
            return ''

    def new_lines(self):
        t = self.text()
        lines = t[self.seen:].splitlines()
        self.seen = len(t)
        return lines

    def state(self):
        self.do('ucmd ustate:; wait 1')
        m = None
        for l in reversed(self.text()[-4000:].splitlines()):
            m = STATE.search(l)
            if m:
                break
        if not m:
            return None
        k = ['mode', 'screen', 'card', 'menu', 'options', 'place', 'tx', 'ty', 'day', 'hour', 'travel', 'goto', 'enc', 'members']
        v = list(m.groups())
        opts = [int(x) for x in v[4].split(',')] if v[4] != '-' else []   # (the option numbers that may be chosen now)
        return dict(zip(k, [int(v[0]), int(v[1]), v[2], int(v[3]), opts] + [int(x) for x in v[5:]]))

    def report(self, kind, what, lines):
        self.shot += 1
        name = 'soak%03d' % self.shot
        out = self.do('wshot %s' % name)
        for l in out.splitlines():
            if l.endswith('.png') and os.path.exists(l):
                shutil.copy(l, os.path.join(self.a.out, name + '.png'))
        self.found.append((kind, what, list(self.actions[-12:]), lines[-12:], name))
        print('FOUND %s: %s' % (kind, what), flush=True)

    def look(self):
        lines = self.new_lines()
        for i, l in enumerate(lines):
            for pat, kind in BAD:
                if pat.search(l):
                    if not any(f[1] == l for f in self.found):
                        self.report(kind, l, lines[max(0, i - 20):i + 3])
                    break
        return lines

    # ---- what a restless player does
    def card(self, s):
        if s['menu'] and s['options']:
            self.do('ucmd uchoose:%d; wait 3' % self.rng.choice(s['options']))
        else:
            self.do('key Return; wait 2')

    def town(self, s):
        r = self.rng.random()
        if self.same >= 3:                                  # (a card Go to and journeys cannot leave: the night watch)
            self.card(s)
            return
        if r < 0.45:
            self.do('ucmd ugotoplaces:; wait 1')
            m = re.findall(r'gotoplaces:([ \d]*)', self.text()[-3000:])
            places = [int(x) for x in m[-1].split()] if m else []
            if places:
                d = self.rng.choice(places)
                self.do('ucmd ugoto:%d; wait 12' % d)
                return
        if r < 0.65:                                        # (out of the gate and away, as the world map's "Leave and travel")
            self.do('ucmd uleavetravel:%d; wait 30' % self.near_town(s))
            return
        self.card(s)

    def near_town(self, s):
        """one of the eight towns nearest the party (a journey of days, not of months)"""
        if not POS or s['tx'] < 0:
            return self.rng.choice(TOWNS)
        near = sorted(POS, key=lambda t: (POS[t][0] - s['tx']) ** 2 + (POS[t][1] - s['ty']) ** 2)
        return self.rng.choice(near[1:9] or near)

    def map(self, s):
        r = self.rng.random()
        if r < 0.15:
            self.do('ucmd uenc:%d; wait 1' % self.rng.randrange(WILD_ENCOUNTERS))
        if r < 0.25:
            self.do('key c; wait 8')                       # (a camp: the lodging screen comes up)
            return
        self.do('ucmd utravel:%d; wait 25' % self.near_town(s))

    def screen(self, s):
        # each native screen (mods/nscreen.h NS_*) with its own actions (ucmd unspush: the screen's *_ enums)
        r, k, m = self.rng.random(), self.rng.randrange(12), self.rng.randrange(max(1, s['members']))
        sc = s['screen']
        if sc == 2:                                          # a shop: buy, sell, another member, leave
            self.do(['ucmd unspush:1,%d,1; wait 2' % k, 'ucmd unspush:2,%d,0; wait 2' % k, 'ucmd unspush:3,%d; wait 1' % m,
                     'ucmd unspush:5; wait 4'][0 if r < 0.3 else 1 if r < 0.55 else 2 if r < 0.7 else 3])
        elif sc in (3, 4):                                   # the sheet: ready, put away, give, drink, leader, close
            self.do(['ucmd unspush:2,%d; wait 1' % k, 'ucmd unspush:3,%d; wait 1' % k, 'ucmd unspush:4,%d,%d; wait 1' % (k, m),
                     'ucmd unspush:6,%d; wait 2' % k, 'ucmd unspush:7,%d; wait 1' % m, 'ucmd unspush:1,%d; wait 1' % m,
                     'ucmd unspush:8; wait 3'][min(6, int(r * 8))])
        elif sc == 5:                                        # the loot: take, take all, share, leave
            self.do(['ucmd unspush:1,%d; wait 1' % k, 'ucmd unspush:4; wait 1', 'ucmd unspush:5; wait 2',
                     'ucmd unspush:6; wait 4'][min(3, int(r * 4))])
        elif sc == 6:                                        # alchemy: a formula, batches, mix, leave
            self.do(['ucmd unspush:1,%d; wait 1' % k, 'ucmd unspush:6,%d; wait 1' % (1 + k % 4), 'ucmd unspush:4; wait 6',
                     'ucmd unspush:5; wait 3'][min(3, int(r * 4))])
        elif sc == 8:                                        # the store room: take, put, leave
            self.do(['ucmd unspush:1,%d; wait 1' % k, 'ucmd unspush:2,%d; wait 1' % k, 'ucmd unspush:5; wait 3'][min(2, int(r * 3))])
        elif sc == 9:                                        # a prayer: pray (for the one who prays or a member), or not
            self.do(['ucmd unspush:5,99; wait 1; ucmd unspush:3,-1; wait 8', 'ucmd unspush:5,99; wait 1; ucmd unspush:3,%d; wait 8' % m,
                     'ucmd unspush:4; wait 3'][min(2, int(r * 3))])
        elif sc == 12:
            self.do('ucmd unspush:1; wait 2')
        elif sc == 13:                                       # marching order: move one, done
            self.do('ucmd unspush:%d,%d; wait 1; ucmd unspush:3; wait 2' % (1 + (k & 1), m))
        elif sc == 14:                                       # the inn or camp: a member's activity, a day, leave
            self.do(['ucmd unspush:2,%d,%d; wait 1' % (k % 6, m), 'ucmd unspush:4; wait 6', 'ucmd unspush:6; wait 4'][min(2, int(r * 3))])
        elif sc == 15:
            self.do('click; wait 2')
        elif sc == 18:                                       # a choice of ours (a cathedral's relics ...)
            self.do('ucmd unspush:1,%d; wait 3' % (k % 3) if r < 0.6 else 'ucmd unspush:2; wait 3')
        else:
            self.do('key Escape; wait 2; key Return; wait 2')

    def battle(self, s):
        if self.rng.random() < 0.25:                         # (walking a castle, a mine, a dungeon: somewhere on its level)
            self.do('ucmd ulvwalk:%d,%d; wait 8' % (self.rng.randrange(40), self.rng.randrange(40)))
            return
        self.do('; '.join('ucmd ukill:%d' % c for c in range(5, 13)) + '; ucmd uunpause:; wait 8')
        self.do('ucmd unspush:5; wait 2; ucmd unspush:6; wait 4; key Return; wait 2')

    def extra(self, s):
        r = self.rng.random()
        if r < 0.3:
            self.do('ucmd uquicksave:; wait 4')
        elif r < 0.45:
            self.do('ucmd uquickload:; wait 12')
        elif r < 0.6 and s['mode'] == 1:
            m = self.rng.randrange(max(1, s['members']))
            self.do('ucmd usaint:%d,%d; wait 4; ucmd unspush:5,99; wait 1; ucmd unspush:3,-1; wait 8; key Return; wait 2' % (m, self.rng.randrange(136)))
        elif r < 0.75 and s['mode'] in (1, 2):
            self.do('ucmd ualchemy:0,8; wait 4; ucmd unspush:1,0; wait 1; ucmd unspush:5; wait 3')
        elif r < 0.8:
            self.do('key F2; wait 4')                         # (the sheet: its actions next, screen())
        elif r < 0.85 and s['mode'] in (1, 2):
            self.do('ucmd umenu:3,%d; wait 3' % self.rng.randrange(2))   # (party info, marching order)
        elif r < 0.9 and s['mode'] in (1, 2):                # (the world map: find a town, close)
            self.do('key m; wait 3; ucmd ufind:%s; wait 2; key m; wait 2' % self.rng.choice(['Koln', 'Wien', 'Graz', 'Bremen', 'Prag', 'Ulm']))
        elif r < 0.95:                                       # (F12: a page, back)
            self.do('key F12; wait 2; ucmd utab:%d; wait 2; key F12; wait 2' % self.rng.randrange(9))
        else:
            self.do('ucmd ufight:99,5; wait 2')

    def run(self):
        end = time.time() + self.a.minutes * 60
        n = 0
        while time.time() < end:
            s = self.state()
            self.look()
            if s is None:
                if 'SESSION ENDED' in self.text()[-2000:] or not os.path.exists(os.path.join(os.path.dirname(self.log), 'pid')):
                    self.report('crash', 'the session ended', self.new_lines())
                    return
                self.do('key Escape; wait 2')
                continue
            key = (s['mode'], s['screen'], s['card'], s['menu'], s['place'], s['tx'], s['ty'], s['day'], s['hour'])
            self.same = self.same + 1 if key == self.last_state else 0
            self.last_state = key
            if self.same == 15:
                self.report('stuck', 'the game stood still for 15 actions: %s' % (s,), self.text()[-3000:].splitlines())
                self.do('key Escape; wait 2; key Escape; wait 2; ucmd uquickload:; wait 12')
                self.same = 0
                continue
            n += 1
            if n % 9 == 0:
                self.extra(s)
            elif s['mode'] == 1 and 0 <= s['place'] < 92 and s['menu'] and not s['goto'] and not s['enc']:
                self.town(s)
            elif s['mode'] == 1:
                self.card(s)
            elif s['mode'] == 2:
                self.map(s)
            elif s['mode'] == 3:
                self.screen(s)
            elif s['mode'] == 4:
                self.battle(s)
            else:
                self.do('key Return; wait 2; key Escape; wait 2')
            self.look()
        print('%d actions' % n)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--game', required=True)
    ap.add_argument('--exe', default=os.path.join(TOP, 'build', 'port', 'darklands'))
    ap.add_argument('--minutes', type=float, default=60)
    ap.add_argument('--seed', type=int, default=1)
    ap.add_argument('--out', default=os.path.join(TOP, 'build', 'soaktest'))
    ap.add_argument('--encounters', default=os.path.join(TOP, 'port', 'content', 'New encounters'))
    ap.add_argument('--load', default='', help='a saved game to start from')
    ap.add_argument('--name', default='soak', help='the session\'s name (two runs at once: two names)')
    a = ap.parse_args()
    global NAME
    NAME = a.name
    shutil.rmtree(a.out, ignore_errors=True)
    os.makedirs(a.out)
    game = os.path.join(a.out, 'game')                      # (the player's folder is never touched)
    shutil.copytree(a.game, game)
    if a.load:
        sd = next((os.path.join(game, n) for n in os.listdir(game) if n.upper() == 'SAVES'), os.path.join(game, 'SAVES'))
        os.makedirs(sd, exist_ok=True)
        shutil.copy(a.load, os.path.join(sd, os.path.basename(a.load).upper()))
        bsv = a.load[:-4] + ('.BSV' if a.load.endswith('.SAV') else '.bsv')       # (its battle part, when it has one)
        if os.path.exists(bsv):
            shutil.copy(bsv, os.path.join(sd, os.path.basename(bsv).upper()))
    if a.encounters and os.path.isdir(a.encounters):
        dst = os.path.join(game, 'mods', 'New encounters')
        shutil.rmtree(dst, ignore_errors=True)
        shutil.copytree(a.encounters, dst)
        ini = os.path.join(game, 'darklands_port.ini')
        old = open(ini, encoding='utf-8', errors='replace').read() if os.path.exists(ini) else ''
        if 'datamod.New encounters' not in old:
            open(ini, 'a', encoding='utf-8').write('\ndatamod.New encounters = 1\n')
    subprocess.run(NCTL + ['stop', NAME], capture_output=True)
    env = dict(os.environ, NCTL_SCREEN='1600x900', DL_TRAVEL_LOG='1', DL_ENC_LOG='1', DL_CITYNAV_LOG='1', DL_FIX_LOG='1', DL_CHECKS_LOG='1')
    subprocess.run(NCTL + ['start', NAME, '--x11', '--exe', a.exe, '--game', game], env=env, capture_output=True, check=True)
    read_towns(game)
    s = Soak(a)
    t0 = time.time()
    while time.time() - t0 < 900 and 'driver mgraphic' not in s.text():
        time.sleep(2)
    time.sleep(3)
    s.do('QUICKSTART; key Return; wait 12; key Return; wait 3; key Return; wait 3; key Return; wait 3; '
         + ('ucmd uload:%s; wait 20; ' % os.path.basename(a.load).upper() if a.load else '')
         + 'ucmd uoption:travel.fast=2; ucmd uoption:travel.camp_at_8=1; ucmd uquicksave:; wait 4')
    s.look()                                                # (the start and the load too: a check at the opening)
    try:
        s.run()
    finally:
        subprocess.run(NCTL + ['stop', NAME], capture_output=True)
        shutil.copy(s.log, os.path.join(a.out, 'log.txt'))
    with open(os.path.join(a.out, 'report.txt'), 'w', encoding='utf-8') as f:
        f.write('soaktest: %d found in %.0f minutes (seed %d)\n\n' % (len(s.found), a.minutes, a.seed))
        for kind, what, acts, lines, shot in s.found:
            f.write('== %s: %s\n   screenshot %s.png\n   the actions before:\n' % (kind, what, shot))
            for x in acts:
                f.write('     %s\n' % x)
            f.write('   the log:\n')
            for x in lines:
                f.write('     %s\n' % x)
            f.write('\n')
    print('%d found; the report is %s' % (len(s.found), os.path.join(a.out, 'report.txt')))
    return 1 if s.found else 0


if __name__ == '__main__':
    sys.exit(main())
