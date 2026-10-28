#!/usr/bin/env python3
"""switch_map.py - docs/MERGING.md with its section 4 (every switch and the files that read it) made anew
from docs/SWITCHES.md (the switches, as the game writes them) and the sources (a file reads a switch when
its key is written in it, in quotes). Prints the whole document; tools/check_docs.py --fix writes it.

  python3 -I port/tools/switch_map.py > docs/MERGING.md
"""
import os, re, sys

TOP = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
TABLES = {'options.c', 'optwhy.c'}                  # (the switches' own tables: every key is in them)


def sources():
    out = []
    for d in ('port/mods', 'port/ui', 'port/runtime'):
        for f in sorted(os.listdir(os.path.join(TOP, d))):
            if f.endswith(('.c', '.cpp', '.h')) and f not in TABLES:
                out.append((f, open(os.path.join(TOP, d, f), encoding='utf-8', errors='replace').read()))
    return out


def main():
    doc = open(os.path.join(TOP, 'docs', 'MERGING.md'), encoding='utf-8').read()
    sw = open(os.path.join(TOP, 'docs', 'SWITCHES.md'), encoding='utf-8').read()
    src = sources()
    opts = open(os.path.join(TOP, 'port', 'mods', 'options.c'), encoding='utf-8').read()
    groups = set(re.findall(r'^    \{ "([a-z_.0-9]+)", \{ "', opts, re.M))   # (BUNDLES[]: a group switch and its members)
    rows = ['| Switch | What | Class | Where |', '|---|---|---|---|']
    for part in re.split(r'^## ', sw, flags=re.M)[1:]:
        title = part.split('\n', 1)[0].strip()
        if title.lower().startswith('retired'):
            continue
        entries = re.findall(r'^\| `([^`]+)` \| ([^|]*) \|.*\| ([a-z]+) \| [^|]* \|$', part, re.M)
        if not entries:
            continue
        rows.append('| **%s** | | | |' % title)
        for key, name, cls in entries:
            look = key.split('.', 1)[1] if key.startswith(('fix.', 'mod.')) else key   # (a fix or built-in mod: read by its id, fix_is_on("id"))
            where = [f for f, text in src if ('"%s"' % key in text or '"%s"' % look in text) and f not in ('fixgroups.c',)]
            if key in groups: where = ['a group of switches: options.c (BUNDLES)'] + where
            rows.append('| `%s` | %s | %s | %s |' % (key, name.strip(), cls, ', '.join(where) if where else '?'))
    head = '## 4. The switches, and where each lives\n'
    i = doc.index(head) + len(head)
    j = doc.index('\n| Switch | What | Class | Where |', i) + 1
    k = j
    lines = doc[j:].split('\n')
    n = 0
    while n < len(lines) and lines[n].startswith('|'):
        n += 1
    k = j + len('\n'.join(lines[:n]))
    sys.stdout.write(doc[:j] + '\n'.join(rows) + doc[k:])


if __name__ == '__main__':
    main()
