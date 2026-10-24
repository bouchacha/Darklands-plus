#!/usr/bin/env python3
"""code_map.py - writes docs/CODE_MAP.md: every source file of the port with the first sentence of its
header comment (each file starts with one saying what it is), by folder, with its size. Run it after
adding or splitting files:  python3 port/tools/code_map.py > docs/CODE_MAP.md"""
import glob, os, re

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..')
PARTS = [('runtime', 'The runtime: the CPU and DOS the game runs on, the window, sound, saves, the configuration', ['runtime/*.c']),
         ('mods', 'The game side: hooks into the game\'s code, its rules and fixes, the models the screens draw from', ['mods/*.c']),
         ('ui', 'The interface: everything drawn (Dear ImGui), the new screens, margins, maps, settings', ['ui/*.cpp']),
         ('tools', 'Tools: tests, checks, the harness', ['tools/*.py'])]

def header(path):
    t = open(path, encoding='utf-8', errors='replace').read(60000)
    m = re.match(r'\s*/\*(.{0,1500}?)(\*/|\n\s*\*\s*\n)', t, re.S) if not path.endswith('.py') else re.search(r'"""(.*?)"""', t, re.S)
    if not m:
        m = re.search(r'((?://[^\n]*\n)+)', t)
        if not m: return ''
        s = re.sub(r'//\s?', '', m.group(1))
    else:
        s = m.group(1)
    s = re.sub(r'\s*\n\s*\*?\s*', ' ', s).strip()
    s = re.sub(r'^\S+\.(c|cpp|h|py)\s*[-:]\s*', '', s)
    m = re.match(r'(.+?\.)(\s|$)', s)
    s = m.group(1) if m and len(m.group(1)) > 30 else s[:220]
    return s.replace('|', '\\|')

print('# Where things are in the code\n')
print('Each source file of `port/`, with the first sentence of its header comment and its length. Made by')
print('`port/tools/code_map.py`; run it again after adding or splitting a file. The switches are listed in')
print('`docs/SWITCHES.md`; the original game\'s modules in `docs/MODULE_MAP.md`.\n')
for name, what, globs in PARTS:
    files = sorted(f for g in globs for f in glob.glob(os.path.join(ROOT, g)))
    print('## %s\n\n%s.\n\n| File | Lines | What it is |\n|---|---:|---|' % (name, what))
    for f in files:
        n = sum(1 for _ in open(f, encoding='utf-8', errors='replace'))
        print('| `%s` | %d | %s |' % (os.path.relpath(f, ROOT), n, header(f)))
    print()
