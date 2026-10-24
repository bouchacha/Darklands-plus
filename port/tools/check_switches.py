#!/usr/bin/env python3
"""check_switches.py OLD.md NEW.md - fails if a switch key in OLD (docs/SWITCHES.md as committed) is
missing from NEW (as the game now writes it) without being listed under "Retired keys" there: a
switch's key is its name for good (saved settings, presets and other engines know it by it)."""
import re, sys

def keys(path):
    live, retired, sect = set(), set(), None
    for l in open(path, encoding='utf-8'):
        if l.startswith('## '): sect = l[3:].strip()
        m = re.match(r'\| `([^`]+)`', l)
        if m: live.add(m.group(1))
        m = re.match(r'- `([^`]+)`', l)
        if m and sect == 'Retired keys': retired.add(m.group(1))
    return live, retired

old, _ = keys(sys.argv[1])
new, retired = keys(sys.argv[2])
gone = sorted(k for k in old - new if k not in retired)
reused = sorted(k for k in new & retired)
for k in gone: print('switch key gone without being retired:', k)
for k in reused: print('retired key in use again:', k)
print('%d switches, %d retired: %s' % (len(new), len(retired), 'OK' if not gone and not reused else 'FAILED'))
sys.exit(1 if gone or reused else 0)
