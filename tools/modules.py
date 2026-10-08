#!/usr/bin/env python3
"""
Game-module map: which overlay section / root segment implements which part of Darklands.
Hand-curated from string references, data files opened, and the overlay call graph
(see build/analysis/section_strings.txt). Writes docs/MODULE_MAP.md with live statistics.

  python3 -I tools/modules.py <DARKLAND.EXE> build/analysis
"""
import sys, os, json, collections
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from rtlink import RTLinkExe

GROUPS = [
    ('core', 'Resident core (root)'),
    ('boot', 'Start-up'),
    ('combat', 'Tactical combat'),
    ('battlefield', 'Battlefield construction'),
    ('strategic', 'Strategic layer (overland, party, items)'),
    ('events', 'Town & event cards (location scripts)'),
    ('infra', 'Infrastructure (memory, overlay cache, protection)'),
]

ROOT = {
    '0000': ('core', 'Game shell: main loop helpers, pause, difficulty, quit-to-DOS'),
    '0265': ('core', 'Common game utilities'),
    '030b': ('core', 'PAN animation player ("player.c": FAB_* cel player, DOS memory)'),
    '03a1': ('core', 'Intro sequence & start-up file list (opening*.pan, msgfiles, darkland.loc)'),
    '03ec': ('core', 'Utility (single function)'),
    '0410': ('core', 'Random numbers & small math helpers (random_n)'),
    '044a': ('core', 'Low-level helpers (BIOS/DOS interrupts)'),
    '047c': ('core', 'Graphics/driver interface to MGRAPHIC.EXE (self-patched far-jump table)'),
    '05ba': ('core', 'Memory buffers ("Insufficient system memory")'),
    '05d9': ('core', 'Low-level helpers'),
    '05f8': ('infra', 'EMS driver interface ("EMS driver not present")'),
    '0617': ('core', 'File I/O helpers'),
    '06a1': ('infra', 'Microsoft C 6.0 run-time library (crt0 + medium-model LIBC)'),
    '09bf': ('infra', '(16-byte stub)'),
    '09c0': ('infra', 'RTLink/Plus overlay manager + 753 overlay call vectors'),
    '0d14': ('infra', 'RTLink/Plus overlay cache (EMS/XMS/disk)'),
}

SECTIONS = {
    0: ('boot', 'Start-up: config, sound & graphics driver loading, fonts; contains main()'),
    1: ('boot', 'Default party (pre-generated characters Gretchen, Gunther, Hans, Ebhard)'),
    2: ('combat', 'Battle entry/exit: tactical setup, results (victory/retreat/defeat/surrender)'),
    3: ('combat', 'Battle state save/restore (level files, .bsv, PKWARE DCL compression)'),
    4: ('battlefield', 'Battlefield loader: enemies (darkland.enm), tactical maps (imaps.cat), terrain'),
    5: ('battlefield', 'Battlefield generator core'),
    6: ('battlefield', 'Battlefield generator (procedural layout)'),
    7: ('battlefield', 'Castle/keep battlefield layouts'),
    8: ('battlefield', 'Wilderness battlefields'),
    9: ('battlefield', 'Mine battlefields (multi-level)'),
    10: ('battlefield', 'City battlefields'),
    11: ('battlefield', 'Templar fortress-monastery & tombs'),
    12: ('battlefield', 'Dragon cave'),
    13: ('battlefield', "Baphomet's citadel (lcastle)"),
    14: ('combat', 'Tactical combat engine (real-time-with-pause battle loop, AI, movement, attacks)'),
    15: ('combat', 'Combat: difficulty change'),
    16: ('combat', 'Combat: party movement modes, stairs/portals'),
    17: ('combat', 'Combat: in-battle interaction hook'),
    18: ('combat', 'Combat: sub-module'),
    19: ('combat', 'Combat: sub-module'),
    20: ('combat', 'Combat: enemy loading & buffer sizing'),
    21: ('combat', 'Combat: enemy information display (einfo.cat)'),
    22: ('combat', 'Combat: surrender, character data swapping'),
    23: ('combat', 'Combat: locks, traps, secret doors'),
    24: ('combat', 'Combat: enemy activation records (debug dump to tac.txt)'),
    25: ('combat', 'Combat: enemy palettes & sprites'),
    26: ('infra', 'Heap / swap-file manager (core)'),
    27: ('infra', 'Heap / swap-file manager'),
    28: ('infra', 'Heap / swap-file manager (heap.fng, TMP/TEMP, EMS)'),
    29: ('strategic', 'Strategic core: time, money, events, skill checks, city data (cty/dsc)'),
    30: ('strategic', 'Overland travel map (darkland.map, map icons, map messages)'),
    31: ('strategic', 'Inventory & equipment screen'),
    32: ('strategic', 'Barter / buying & selling'),
    33: ('strategic', 'Alchemy (darkland.alc, formula batches)'),
    34: ('strategic', 'Saints & prayer; party information screen'),
    35: ('strategic', 'Item cache (storage)'),
    36: ('strategic', 'Camping, inns, day labour'),
    37: ('strategic', 'Character creation; save & load game'),
    38: ('strategic', 'Character creation part 2 (family background, darkland.fam)'),
    39: ('events', 'Event-rules toolkit shared by all card scripts (180 functions)'),
    40: ('strategic', 'Loot distribution'),
    41: ('infra', 'Copy protection (manual symbol lookup)'),
    42: ('infra', '(empty section)'),
    43: ('events', 'Card/dialog engine: loads MSG cards, city descriptions; parent of all card scripts'),
}
EVENT_HINTS = {
    44: 'city gates, initial cards', 49: 'markets, merchants, arms outfitters', 50: 'physicians, city fights',
    56: 'mines, university, cathedral', 60: 'traders, Fugger bank', 61: 'Fugger, medics, Hanse',
    63: 'blacksmiths, soldiers, swordsmiths', 66: 'alchemists, artificers', 70: 'guilds, politics',
    80: 'robber knights (Raubritter)', 81: 'robber knights, dragon', 82: 'dragon quest', 83: 'dragon quest',
    93: 'mine encounters', 104: 'counts / castles', 106: 'quest McGuffins', 117: "witches' sabbat quest",
    118: "witches' sabbat quest", 130: "final quest (Baphomet)", 133: 'end game & finale animations',
}
for i in range(44, 136):
    SECTIONS[i] = ('events', 'Location/event card script' + (' - ' + EVENT_HINTS[i] if i in EVENT_HINTS else ''))


def main():
    exe = RTLinkExe(sys.argv[1])
    an = sys.argv[2]
    funcs = json.load(open(os.path.join(an, 'functions.json')))
    per = collections.defaultdict(lambda: collections.Counter())
    for r in funcs.values():
        key = ('root', r['frame']) if r['image'] == 'root' else ('sec', int(r['image'][1:]))
        per[key]['funcs'] += 1
        per[key]['bytes'] += r['size']
        per[key][r['kind']] += 1
    rows = []
    for fr, (g, desc) in ROOT.items():
        c = per[('root', fr)]
        rows.append(('root ' + fr, g, desc, c['funcs'], c['bytes'], None))
    for s in exe.sections:
        if s.index == 136:
            continue
        g, desc = SECTIONS[s.index]
        c = per[('sec', s.index)]
        parent = '-' if s.parent == 0xffff else 's%03d' % s.parent
        rows.append(('s%03d' % s.index, g, desc, c['funcs'], c['bytes'], parent))
    tot = collections.Counter()
    nfun = collections.Counter()
    for r in rows:
        tot[r[1]] += r[4]
        nfun[r[1]] += r[3]
    out = ['# Darklands module map', '',
           'Which part of `DARKLAND.EXE` implements which part of the game. Generated by',
           '`tools/modules.py` from the function database; descriptions are curated from string',
           'references, data files opened, and the overlay call graph.', '',
           '## Summary by game module', '', '| module | functions | code bytes |', '|---|---:|---:|']
    for g, name in GROUPS:
        out.append('| %s | %d | %d |' % (name, nfun[g], tot[g]))
    out += ['', 'Section s136 is the preloaded data section (DGROUP and far data, about 133 KB). It holds no code.', '',
            'Towns are not a separate walk-around engine. Darklands towns are menu and card driven: the card',
            'engine (s043) shows a location card from the MSG files, and the matching card script overlay',
            '(s044-s135) decides the options, rolls and outcomes, using the shared rules toolkit in s039.', '',
            '## Overlay tree', '',
            'RTLink/Plus loads sections into a tree. Siblings share the same memory and are mutually',
            'exclusive. A child is only resident while its parent is.', '',
            '| unit | parent | module | functions | bytes | role |', '|---|---|---|---:|---:|---|']
    for unit, g, desc, nf, nb, parent in rows:
        out.append('| %s | %s | %s | %d | %d | %s |' % (unit, parent or '(resident)', g, nf, nb, desc))
    p = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), 'docs', 'MODULE_MAP.md')
    os.makedirs(os.path.dirname(p), exist_ok=True)
    open(p, 'w').write('\n'.join(out) + '\n')
    json.dump({'groups': dict(GROUPS), 'rows': rows}, open(os.path.join(an, 'modules.json'), 'w'), indent=0)
    for g, name in GROUPS:
        print('%-45s %5d funcs %8d bytes' % (name, nfun[g], tot[g]))


if __name__ == '__main__':
    main()
