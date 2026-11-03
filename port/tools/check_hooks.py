#!/usr/bin/env python3
"""check_hooks.py - a hook that reads or writes its arguments by SP after calling the game's function.

At a hook's entry SP points at the far return address and the arguments follow it (SP + 4, SP + 6, ...).
hook_call_original() runs the game's function, whose return pops that address: afterwards SP is 4 higher,
and SP + 6 is no longer the second argument but a word of the caller's own frame. relics.c wrote the favor
back that way after asking a saint's chance (1.4.5 to 1.4.6-RC5): the word was saint_finish's saved BP, the
prayer screen's caller came back with BP the favor, and every prayer by a relic's bearer crashed.

Take the address before the call (u16 at = SP + 6; ... ww(SS, at, v)), or read the arguments into
locals first. This lists every SS:SP + k access (rw/ww/rb/wb, or a file's ARG(k) macro) that comes after
a hook_call_original() on the same path of a function in port/mods, port/ui and port/runtime.

  check_hooks.py [PORT_DIR]          exit status 1 when it finds any
"""
import re, sys, glob, os

ACCESS = re.compile(r'\b(rw|ww|rb|wb)\s*\(\s*SS\s*,\s*\(u16\)\s*\(\s*SP\s*\+|\bARG\s*\(')


def strip(t):
    """comments and string contents blanked (newlines kept), so braces and words in them do not count"""
    blank = lambda m: re.sub(r'[^\n]', ' ', m.group(0))
    t = re.sub(r'/\*.*?\*/', blank, t, flags=re.S)
    t = re.sub(r'//[^\n]*', blank, t)
    t = re.sub(r'"(\\.|[^"\\\n])*"', '""', t)
    return re.sub(r"'(\\.|[^'\\\n])'", "''", t)


def scan(path):
    raw = open(path, encoding='utf-8', errors='replace').read()
    lines, rawl = strip(raw).split('\n'), raw.split('\n')
    found = []
    depth = 0
    called = None            # (the brace depth of the block where the original was called, a return seen in it)
    for i, code in enumerate(lines):
        if depth == 0:
            called = None
        # a statement at a time is enough here: these files put a hook's call and its uses on lines of their own
        for stmt in re.split(r'(?<=;)', code):
            if 'hook_call_original()' in stmt:
                called = [depth + stmt.count('{') - stmt.count('}'), False]
            elif called and re.match(r'[\s{}]*return\b', stmt) and depth + stmt.count('{') - stmt.count('}') >= called[0]:
                called[1] = True     # (a return of its own: "if (x) return;" leaves the path going on)
            elif called and ACCESS.search(stmt) and not called[1]:
                found.append('%s:%d: %s' % (os.path.relpath(path), i + 1, rawl[i].strip()[:140]))
            depth += stmt.count('{') - stmt.count('}')
            if called and depth < called[0]:
                called = None if called[1] else [depth, False]   # (the block that called it closed: past a return, that path is over)
    return found


def main():
    port = sys.argv[1] if len(sys.argv) > 1 else os.path.join(os.path.dirname(os.path.abspath(__file__)), '..')
    files = sorted(glob.glob(os.path.join(port, 'mods', '*.c')) + glob.glob(os.path.join(port, 'ui', '*.cpp')) +
                   glob.glob(os.path.join(port, 'runtime', '*.c')))
    found = [f for p in files for f in scan(p)]
    for f in found:
        print(f)
    print('hooks: %d files, %s' % (len(files), 'no argument read by SP after the original' if not found else '%d reads by SP after the original' % len(found)))
    return 1 if found else 0


if __name__ == '__main__':
    sys.exit(main())
