#!/usr/bin/env python3
"""arena.py - a combat test arena: battles set up to push Improved combat (battle.improved, mods/orders.c,
mods/battlerules.c) to its limits, run in the game (nctl, X11), logged and checked.

  arena.py --game DIR [--only NAME[,NAME]] [--out DIR] [--seeds N]

Each scenario: a battlefield and enemy groups (ufight), the Quickstart party armed as it asks (uarm:
member, missile weapon, ammunition), the fighters placed (uplace) and given orders (ushoot, ...) once
the battle is on, then the battle run (fast-forward, Z) for a while with the combatants' state logged
every few ticks (uarenalog). The checks, on the log:
  idle shooter   a party shooter with a Use Missile order, a foe standing and nothing beside it, that
                 neither shoots, walks nor changes its mind for a long time (idle, or "ready" and the shot never
                 coming)
  point blank    a party shooter still ordered to shoot with a foe beside it for a while
  flip-flop      a member's weapon put away and taken up again too often
  stuck walker   a member walking but not moving for a long time
  no end         the battle not over by the end of the scenario (when it should be)
  crash          the game gone
Prints one line a scenario and the problems found; the logs are kept in --out.
"""
import argparse, os, re, subprocess, sys, time, json

HERE = os.path.dirname(os.path.abspath(__file__))
NCTL = [sys.executable, '-I', os.path.join(HERE, 'nctl.py')]
START = ('QUICKSTART; key Return; wait 12; key Return; wait 3; key Return; wait 3; key Return; wait 3; '
         'key Return; wait 3; key Return; wait 3')

# battlefields (DARKLAND.ENM numbering, docs/MODDING.md): 0x2e clearing, 0x2f the wilds (trees), 0x30 surrounded
CLEARING, WILDS, SURROUNDED = 0x2e, 0x2f, 0x30
# enemies: 7 bandit, 2 archer, 0x0c thug, 0x0f mercenary, 0x2a wolf, 0x16 robber captain, 3 guard
BANDIT, ARCHER, THUG, MERC, WOLF, CAPTAIN, GUARD = 7, 2, 0x0c, 0x0f, 0x2a, 0x16, 3
QUICK_PARTY = ['Gretch', 'Gunther', 'Hans', 'Ebhard']          # (Quickstart's party, members 0-3, as the battle log names them)
BOW, LONGBOW, XBOW, ARBALEST, GUN, JAVELIN, KNIFE = 28, 27, 30, 29, 31, 24, 1

# a scenario: name, what it pushes, battlefield, seed, groups (type, quality, number) x3, arms
# [(member, weapon, ammo)], set (config before the battle), place [(i, dx, dy)] relative to the party's
# first member once the battle is on (None: as the game puts them), orders [ucmd ...], seconds, end
# (True: the battle should be over by then), expect [log regexes that must appear], shots [(member, at
# least so many shots loosed)]
S = []
def sc(name, what, bf=CLEARING, seed=11, groups=(BANDIT, 2, 6, 0, 1, 0, -1, 0, 0), arms=(), set=(), place=(), orders=(), secs=70, end=True,
       expect=(), shots=(), forbid=(), pre=()):
    S.append(dict(name=name, what=what, bf=bf, seed=seed, groups=groups, arms=list(arms), set=list(set), place=list(place),
                  orders=list(orders), secs=secs, end=end, expect=list(expect), shots=list(shots), forbid=list(forbid), pre=list(pre)))

sc('volley', 'one archer, three swords against six bandits in the open: shoot, then the sword when they close, the bow again after',
   arms=[(2, BOW, 30)])
sc('all_shooters', 'every member a shooter against six bandits: nobody shooting at point blank, nobody idle',
   arms=[(0, BOW, 30), (1, XBOW, 30), (2, LONGBOW, 30), (3, JAVELIN, 6)])
sc('friend_in_line', 'the archer behind a friend, the foe straight beyond: step aside for a line, then shoot',
   arms=[(2, BOW, 30)], groups=(BANDIT, 2, 2, 0, 1, 0, -1, 0, 0),
   place=[(2, 0, 0), (0, 16, 0), (5, 200, 0), (6, 210, 10)], orders=['ushoot:2,5'], end=False,
   expect=[r'battlerules: 2 (in place, shoots|has a line to) '], shots=[(2, 1)])
sc('moving_target', 'the archer must find a line while its mark walks behind the melee: wolves run fast and circle',
   arms=[(2, LONGBOW, 30)], groups=(WOLF, 2, 5, 0, 1, 0, -1, 0, 0), bf=WILDS)
sc('surrounded', 'enemies on every side, two shooters in the middle: put the bow away for a foe beside, help a friend set upon (a losing fight that can last: 100 s)',
   bf=SURROUNDED, arms=[(1, XBOW, 30), (2, BOW, 30)], groups=(BANDIT, 2, 8, 0, 1, 0, -1, 0, 0), secs=100)
sc('rescue', 'a friend set upon by three a few paces off while the archer shoots at another: go and help',
   arms=[(2, BOW, 30)], groups=(BANDIT, 1, 4, 0, 1, 0, -1, 0, 0),
   place=[(2, 0, 0), (0, 40, 0), (5, 52, 0), (6, 40, 8), (7, 28, -8), (8, 220, 0), (1, -150, 60), (3, -150, 70)], orders=['ushoot:2,8'])
sc('handgun', 'a slow handgun with foes closing: no putting away and taking up again over and over (each change starts the reloading anew)',
   arms=[(2, GUN, 20)], groups=(THUG, 2, 6, 0, 1, 0, -1, 0, 0))
sc('enemy_archers', 'enemy archers at range and swords coming: our archer duels theirs, our swords close in',
   arms=[(2, LONGBOW, 30)], groups=(ARCHER, 2, 3, BANDIT, 2, 4, -1, 0, 0), bf=WILDS, secs=100)   # (seven foes on four: a close fight can still be on at 70 s)
sc('thrown', 'javelins and throwing knives: thrown, then the melee weapon, picked up after the battle',
   arms=[(0, JAVELIN, 4), (2, KNIFE, 6)], groups=(BANDIT, 2, 5, 0, 1, 0, -1, 0, 0))
sc('crowded_line', 'three shooters in a column, each blocked by the one before: they spread out to shoot',
   arms=[(0, BOW, 30), (1, BOW, 30), (2, BOW, 30)], groups=(MERC, 2, 4, 0, 1, 0, -1, 0, 0),
   place=[(0, 0, 0), (1, -16, 0), (2, -32, 0), (3, 16, 0), (5, 240, 0), (6, 250, 8), (7, 240, -8), (8, 260, 0)],
   orders=['ushoot:0,5', 'ushoot:1,5', 'ushoot:2,5'])
sc('mark_falls', 'the mark dies before the shot: the next foe, the nearest with a clear line',
   arms=[(2, BOW, 30)], groups=(BANDIT, 0, 5, 0, 1, 0, -1, 0, 0), orders=['ushoot:2,5', 'ukill:5'])
sc('hemmed_in', 'the shooter boxed in by its own friends, no way out and no line: no freezing, no walking into walls',
   arms=[(2, BOW, 30)], groups=(BANDIT, 2, 3, 0, 1, 0, -1, 0, 0),
   place=[(2, 0, 0), (0, 16, 0), (1, -16, 0), (3, 0, 8), (5, 200, 0), (6, 200, 20), (7, 200, -20)], orders=['ushoot:2,5'])
sc('boss', 'a robber captain and his men in the wilds: long fight, many changes of target',
   arms=[(1, ARBALEST, 20), (2, BOW, 30)], groups=(CAPTAIN, 3, 1, BANDIT, 2, 6, -1, 0, 0), bf=WILDS, secs=180)   # (the captain's armor can hold out against the last two for a long while: still fighting is not a stall)
sc('player_melee', 'the player sends the archer at a foe hand to hand: it fights, and does not take up the bow again by itself',
   arms=[(2, BOW, 30)], groups=(BANDIT, 2, 4, 0, 1, 0, -1, 0, 0), orders=['uattack:2,5'],
   forbid=[r'orders: member 2 (had no orders|has no foe beside it now|.s mark fell|stood idle with its bow)'])
sc('held', 'the archer told to hold, its line blocked by a friend: it stays put (no stepping aside) and does not freeze the others',
   arms=[(2, BOW, 30)], groups=(BANDIT, 2, 3, 0, 1, 0, -1, 0, 0),
   place=[(2, 0, 0), (0, 16, 0), (5, 200, 0), (6, 210, 10), (7, 200, -10)], orders=['ushoot:2,5', 'uhold:2,1'])
sc('no_ammo', 'endless ammunition off and three arrows: the archer shoots them, then draws the sword and walks to its mark',
   arms=[(2, BOW, 3)], set=['assist.endless=0'], groups=(BANDIT, 2, 4, 0, 1, 0, -1, 0, 0))
sc('blocked_and_moving', 'the line blocked by a friend while the mark walks across it, others closing from both sides: step aside, follow the mark, no dithering',
   arms=[(2, BOW, 30)], groups=(BANDIT, 2, 3, 0, 1, 0, -1, 0, 0),
   place=[(2, 0, 0), (0, 16, 0), (5, 180, 0), (6, 150, 80), (7, 150, -80)], orders=['ushoot:2,5'], end=False,
   expect=[r'battlerules: 2 (steps aside|walks|has no line)'], shots=[(2, 1)])
sc('mark_dies_on_the_way', 'the archer walks for a line and its mark dies before it gets there: a new mark, shooting, not standing',
   arms=[(2, BOW, 30)], groups=(BANDIT, 2, 3, 0, 1, 0, -1, 0, 0),
   place=[(2, 0, 0), (0, 16, 0), (5, 200, 0), (6, 220, 30), (7, 220, -30)], orders=['ushoot:2,5', 'wait 2', 'ukill:5'], end=False,
   shots=[(2, 1)])
sc('player_overrides', 'the player sends the archer at a foe hand to hand while it walks for a line: the order stands, no bow again by itself',
   arms=[(2, BOW, 30)], groups=(BANDIT, 2, 3, 0, 1, 0, -1, 0, 0),
   place=[(2, 0, 0), (0, 16, 0), (5, 200, 0), (6, 120, 40), (7, 220, -30)], orders=['ushoot:2,5', 'wait 2', 'uattack:2,6'], end=False,
   expect=[r'orders: test: 2 attacks 6 hand to hand'], forbid=[r'orders: member 2 (had no orders|has no foe beside it now|.s mark fell|stood idle with its bow)'])
sc('last_arrow', 'one arrow and no endless ammunition, the line blocked: step aside, loose it, then the sword',
   arms=[(2, BOW, 1)], set=['assist.endless=0'], groups=(BANDIT, 2, 2, 0, 1, 0, -1, 0, 0),
   place=[(2, 0, 0), (0, 16, 0), (5, 200, 0), (6, 210, 10)], orders=['ushoot:2,5'], end=False, shots=[(2, 1)])
sc('many_shooters_woods', 'four shooters against eight in the trees: lines blocked by trunks and friends, many changes of mark, the frame time steady',
   arms=[(0, BOW, 30), (1, XBOW, 30), (2, LONGBOW, 30), (3, BOW, 30)], groups=(BANDIT, 2, 8, 0, 1, 0, -1, 0, 0), bf=WILDS, secs=90)
sc('idle_join', 'a friend set upon a few paces off, two members with no orders nearby, one of them told to hold: the other joins in, the held one stays',
   arms=[(2, BOW, 30)], groups=(BANDIT, 2, 2, 0, 1, 0, -1, 0, 0),
   place=[(0, 0, 0), (1, -30, 10), (3, -30, -10), (2, -70, 0), (5, 14, 0), (6, 14, 8)], orders=['uhold:3,1'], end=False,
   expect=[r'orders: member 1 had no orders: joins the fight'], forbid=[r'orders: member 3 had no orders: joins'])
sc('join_melee', 'one of the party fights a lone foe hand to hand, three shooters a few paces off: they put the bows away and join in, not loose into the melee',
   arms=[(1, BOW, 30), (2, BOW, 30), (3, XBOW, 30)], groups=(MERC, 3, 1, 0, 1, 0, -1, 0, 0),
   place=[(0, 0, 0), (5, 14, 0), (1, -30, 12), (2, -28, -12), (3, -38, 0)], end=False,
   expect=[r'orders: member 1 .*goes to help', r'orders: member 2 .*goes to help', r'orders: member 3 .*goes to help'])
sc('second_battle', "a later battle of the session, the last battle's clock of its last order still set (the game clears the battle's clock, not that): the shooters open fire at once, not when this battle's clock reaches it",
   arms=[(1, BOW, 30), (2, BOW, 30), (3, XBOW, 30)], groups=(BANDIT, 2, 3, 0, 1, 0, -1, 0, 0), orders=['uww:c867,30000'],
   place=[(1, 0, 0), (2, 0, 30), (3, 0, -30), (0, -40, 0), (5, 230, 0), (6, 240, 30), (7, 240, -30)], secs=30, end=False,
   shots=[(1, 1), (2, 1), (3, 1)])
sc('wolf_pack', 'eight wolves on four swords in the open: fast foes swarming, nobody stuck walking in place, no stutter',
   groups=(WOLF, 2, 8, 0, 1, 0, -1, 0, 0), secs=90)
sc('big_brawl', 'eight mercenaries and a captain against the party in the trees, two of ours shooting: a long crowded melee, routes round trunks and bodies',
   arms=[(1, XBOW, 30), (2, BOW, 30)], groups=(MERC, 3, 8, CAPTAIN, 3, 1, -1, 0, 0), bf=WILDS, secs=120)
sc('original', 'Improved combat off: the game\'s own way, as a check that nothing of ours runs (no crash, no orders of ours)',
   arms=[(2, BOW, 30)], set=['battle.improved=0'], end=False)

def nctl(*a, timeout=600):
    return subprocess.run(NCTL + list(a), capture_output=True, text=True, timeout=timeout)

def run(s, game, exe, out):
    name = 'ar_' + s['name']
    nctl('stop', name)
    env = dict(os.environ, DL_ORDERS_LOG='1', DL_BATTLE_LOG='1')
    r = subprocess.run(NCTL + ['start', name, '--game', game, '--x11'] + (['--exe', exe] if exe else []), capture_output=True, text=True, env=env, timeout=200)
    steps = [START, 'ucmd uoption:battle.pause_down=0', 'ucmd uoption:battle.pause_hurt=0', 'ucmd uoption:battle.pause_foes=0']   # (no automatic pauses: nobody to resume)
    steps += ['ucmd uoption:%s' % kv for kv in s['set']]
    steps += ['ucmd uarm:%d,%d,%d' % a for a in s['arms']]
    steps += ['ucmd %s' % p for p in s.get('pre', [])]          # (test commands before the battle: the state an earlier one leaves)
    g = ','.join(str(x) for x in s['groups'])
    steps += ['ucmd ufight:%d,%d,%s' % (s['bf'], s['seed'], g), 'ucmd ugoto:24', 'wait 14', 'ucmd ucbt:']
    if s['place']:
        place = list(s['place'])
        if any(i < 5 for i, dx, dy in place):          # (party members not placed: out of the way behind, not where the battle happened to put them)
            placed = {i for i, dx, dy in place}
            place += [(m, -170, -40 + 26 * m) for m in range(4) if m not in placed]
        steps += ['ucmd uarenaplace:%s' % '/'.join('%d,%d,%d' % p for p in place)]
    steps += [o if o.startswith('wait ') else 'ucmd %s' % o for o in s['orders']]   # ('wait N': a pause between orders)
    steps += ['ucmd uarenalog:10', 'ucmd uunpause:', 'wait 1', 'key z']
    secs = s['secs']
    for k in range(0, secs, 10):
        steps += ['wait 10', 'ucmd uunpause:', 'ucmd ucbt:' if k % 30 == 20 else 'ucmd uparty:']
    steps += ['wshot %s_end' % name]
    t0 = time.time()
    r = nctl('do', name, '; '.join(steps), timeout=secs * 3 + 400)
    logf = '/tmp/nctl/%s/out.txt' % name
    log = open(logf, errors='replace').read() if os.path.exists(logf) else ''
    alive = nctl('stop', name).returncode == 0
    os.makedirs(out, exist_ok=True)
    open(os.path.join(out, name + '.log'), 'w').write(log)
    return check(s, log), time.time() - t0

def parse(log):
    ticks = []
    for line in log.splitlines():
        if not line.startswith('ARENA t='):
            continue
        parts = line.split(' |')
        t = int(parts[0][8:])
        cb = {}
        for p in parts[1:]:
            m = re.match(r'(\d+):(-?\d+),(-?\d+) o([0-9a-f]+) t(-?\d+) a(\d+) s(\d+) m(\d+) w(-?\d+) e(-?\d+) b(\d+)', p)
            if m:
                v = [int(x) if k != 3 else int(x, 16) for k, x in enumerate(m.groups())]
                cb[v[0]] = dict(x=v[1], y=v[2], o=v[3], t=v[4], a=v[5], s=v[6], m=v[7], w=v[8], e=v[9], b=v[10])
        ticks.append((t, cb))
    return ticks

def check(s, log):
    probs = []
    if 'Segmentation' in log or 'SIGSEGV' in log or 'fatal' in log.lower() and 'fatal error' in log.lower():
        probs.append('crash')
    ticks = parse(log)
    if not ticks:
        return ['no battle log (the battle never started?)']
    # the battlefield's layout varies from run to run: foes placed behind a wall or a stand of trees never
    # move, and no line reaches them; that is the layout, not the shooting (main tries another seed)
    first = {j: (c['x'], c['y']) for j, c in ticks[0][1].items() if j >= 5}
    still = {j for j in first if all(cb[j]['x'] == first[j][0] and cb[j]['y'] == first[j][1] for t, cb in ticks if j in cb)}
    standing_at_end = {j for j in ticks[-1][1] if j >= 5}
    if first and s.get('foes_move', True) and (len(still) == len(first) or (len(still) * 2 >= len(first) and still & standing_at_end)):
        return ['layout: %d of the %d foes never moved (walled off from the party?)' % (len(still), len(first))]   # (half or more, and some of them still there at the end: the battle cannot end for them)
    # per party member: runs of the same state
    for i in range(5):
        idle = blank = stuck = bowidle = 0; last = None; flips = []; worst_idle = worst_blank = worst_stuck = worst_bowidle = 0; nshots = 0
        prev_w_ord = None
        for t, cb in ticks:
            c = cb.get(i)
            if not c:
                idle = blank = stuck = 0; last = None; continue
            foes = [j for j in cb if j >= 5]
            shooting_order = c['o'] == 0x80
            if shooting_order and c['b'] == 0 and foes and c['m'] == 0 and c['a'] in (0, 7):   # (7: ready, and the loosing never comes)
                idle += 10; worst_idle = max(worst_idle, idle)
            else:
                idle = 0
            if shooting_order and c['b'] > 0:
                blank += 10; worst_blank = max(worst_blank, blank)
            else:
                blank = 0
            # a missile weapon and no orders, standing, no foe beside, foes about: Improved combat should have it shoot
            if c['w'] >= 0 and c['o'] == 0 and c['m'] == 0 and c['b'] == 0 and foes:
                bowidle += 10; worst_bowidle = max(worst_bowidle, bowidle)
            else:
                bowidle = 0
            if c['a'] in (9, 13) and (not last or last['a'] not in (9, 13)):   # (13: the last shot, its ammunition gone)
                nshots += 1
            if c['m'] and last and (c['x'], c['y']) == (last['x'], last['y']):
                stuck += 10; worst_stuck = max(worst_stuck, stuck)
            else:
                stuck = 0
            ordk = 'shoot' if shooting_order else 'other'
            if prev_w_ord and ordk != prev_w_ord:
                flips.append(t)
            prev_w_ord = ordk
            last = c
        if worst_idle >= 150:
            probs.append('member %d idle with a shooting order for %d ticks' % (i, worst_idle))
        if worst_blank >= 60:
            probs.append('member %d kept a shooting order with a foe beside it for %d ticks' % (i, worst_blank))
        held_i = any(o == 'uhold:%d,1' % i for o in s['orders'])   # (told to hold with its line blocked: standing is right, it may not step aside)
        if worst_bowidle >= 150 and re.sub(r'_\d+$', '', s['name']) != 'original' and not held_i:   # (the seeds' runs too: original_1 ...)
            probs.append('member %d stood with its missile weapon and no orders for %d ticks' % (i, worst_bowidle))
        # (a shot's draw and loosing can fall between two samples: the battle log's "<name>'s shot" lines count too)
        if i < len(QUICK_PARTY): nshots = max(nshots, len(re.findall(r"battle: %s's shot" % QUICK_PARTY[i], log)))
        for m, n in s.get('shots', []):
            if m == i and nshots < n:
                probs.append('member %d loosed %d shots (expected at least %d)' % (i, nshots, n))
        held = any(o == 'uhold:%d,1' % i for o in s['orders'])          # (one told to hold: its walk toward a foe out of reach goes nowhere, as in the original)
        if worst_stuck >= 150 and not held:
            probs.append('member %d walking without moving for %d ticks' % (i, worst_stuck))
        # flip-flop: more than 6 changes between shooting and not within 600 ticks
        for k in range(len(flips)):
            n = sum(1 for f in flips if flips[k] <= f < flips[k] + 600)
            if n > 6:
                probs.append('member %d switched between shooting and not %d times in 600 ticks (from t=%d)' % (i, n, flips[k])); break
    for rx in s.get('expect', []):
        if not re.search(rx, log):
            probs.append('never logged: %s' % rx)
    for rx in s.get('forbid', []):
        m = re.search(rx, log)
        if m:
            probs.append('logged what should not happen: %s' % m.group(0))
    last = ticks[-1][1]
    foes_left = [j for j in last if j >= 5]
    party_left = [j for j in last if j < 5]
    ended = 'battle end:' in log
    if s['end'] and not ended and foes_left and party_left and ticks[-1][0] < 40 * s['secs']:
        probs.append('slow run: the battle clock reached only %d in %ds (the machine busy?): not judged' % (ticks[-1][0], s['secs']))
    elif s['end'] and not ended and foes_left and party_left:
        probs.append('battle not over after %ds (%d foes and %d of the party still standing)' % (s['secs'], len(foes_left), len(party_left)))
    return probs

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--game', required=True); ap.add_argument('--exe'); ap.add_argument('--only'); ap.add_argument('--out', default='/tmp/arena')
    ap.add_argument('--seeds', type=int, default=1, help='run each scenario with this many battle seeds')
    a = ap.parse_args()
    todo = [s for s in S if not a.only or s['name'] in a.only.split(',')]
    total = 0
    for s in todo:
        for k in range(a.seeds):
            s2 = dict(s, seed=s['seed'] + k * 7, name=s['name'] + ('' if k == 0 else '_%d' % k))
            probs, dt = run(s2, a.game, a.exe, a.out)
            for retry in range(2):                         # (a layout that walls the foes off: another seed)
                if not any(p.startswith('layout:') for p in probs): break
                print('%-22s        %s: another seed' % (s2['name'], probs[0]), flush=True)
                s2 = dict(s2, seed=s2['seed'] + 3)
                probs, dt = run(s2, a.game, a.exe, a.out)
            total += len(probs)
            print('%-22s %4.0fs  %s' % (s2['name'], dt, 'OK' if not probs else '; '.join(probs)), flush=True)
    print('%d problem%s' % (total, '' if total == 1 else 's'))

if __name__ == '__main__':
    main()
