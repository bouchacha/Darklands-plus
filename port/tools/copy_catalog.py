#!/usr/bin/env python3
"""copy_catalog.py - every text a player can read that Darklands+ itself writes (not the game's own),
by file and line: docs/COPY.md. For proofreading and translation: one place to read all the
words. Strings in logs, settings keys, file names and ImGui ids are left out.
    python3 port/tools/copy_catalog.py > docs/COPY.md"""
import glob, os, re, sys

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..')
SKIP_LINE = re.compile(r'fprintf|LOG\(|config_(get|set|unset)|getenv|strcmp|strcasecmp|strncmp|snprintf\(\s*(key|kk|k|p|path|fp|dir|nm|name)\b|fopen|opendir|#include|hook_|ns_call|ImGui::PushID|PushID\(|BeginChild\("##|"##')
LIT = re.compile(r'"((?:[^"\\\n]|\\.)*)"')

def strings(path):
    out = []
    lines = open(path, encoding='utf-8', errors='replace').read().split('\n')
    i = 0
    while i < len(lines):
        l = lines[i]
        if SKIP_LINE.search(l) or l.lstrip().startswith(('//', '/*', '*', '#')):
            i += 1; continue
        # adjacent literals across lines are one text
        parts = LIT.findall(l)
        j = i
        while parts and lines[j].rstrip().endswith('"') and j + 1 < len(lines) and lines[j + 1].lstrip().startswith('"'):
            j += 1; parts += LIT.findall(lines[j])
        text = ''.join(parts) if len(parts) > 1 and all(not p.endswith(('|',)) for p in parts) and l.count('"') % 2 == 0 and lines[j].rstrip().endswith((');', '",', '"', ');', '),')) and j > i else None
        for t in ([text] if text else parts):
            t = t.replace('\\"', '"').replace('\\n', ' ')
            if len(t) < 12 or ' ' not in t: continue                      # (keys, names, short labels)
            if re.fullmatch(r'[\w.%/\-: ]+', t) and '.' in t and ' ' not in t.strip(): continue
            if t.count('%') > 3 and len(t) < 40: continue
            out.append((i + 1, t))
        i = j + 1
    return out

GROUPS = [('Settings: the switches (mods/options.c)', ['mods/options.c']), ('Settings: why each switch (mods/optwhy.c)', ['mods/optwhy.c']),
          ('Fixes (mods/fixes.c)', ['mods/fixes.c']), ('The interface (ui/)', sorted(glob.glob(os.path.join(ROOT, 'ui/*.cpp')))),
          ('The game side (mods/)', [f for f in sorted(glob.glob(os.path.join(ROOT, 'mods/*.c'))) if not f.endswith(('options.c', 'optwhy.c', 'fixes.c'))])]
print('# Every text Darklands+ shows\n')
print('Made by `port/tools/copy_catalog.py` from the source: the words Darklands+ itself writes (the game\'s own texts are')
print('in its files). One row a text, with where it is. Words in **Terms** below are used the same way everywhere.\n')
print('## Terms\n')
print('- **equipped**: worn or held (weapons, armor, shield, a missile weapon); never "in use".')
print('- **the party\'s pack** (Darklands+, *One shared pack*): one pack for the whole party; spare items are in it, not on a member.')
print('- **party member** (or member): one of the characters in the party.')
print('- **Endurance, Strength, Divine Favor, Virtue**: capitalized, as the game has them.')
print('- **Full Original, Purist, Darklands+**: the presets; **Custom**: own choices.')
print('- **Darklands+** marks what the original game does not have; the Guide colors it.\n')
n = 0
for title, files in GROUPS:
    rows = []
    for f in files:
        f = f if os.path.isabs(f) else os.path.join(ROOT, f)
        for ln, t in strings(f):
            rows.append((os.path.relpath(f, ROOT), ln, t))
    if not rows: continue
    print('## %s\n\n| Where | Text |\n|---|---|' % title)
    for f, ln, t in rows:
        print('| `%s:%d` | %s |' % (f, ln, t.replace('|', '\\|')))
        n += 1
    print()
sys.stderr.write('%d texts\n' % n)
