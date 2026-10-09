#!/usr/bin/env python3
"""
Rewrite card-script sources (src/c/s044..s135) to use the readable names of
include/darklands/card.h.

  python3 -I tools/cardnames.py FILE.c [FILE.c ...]          rewrite in place
  python3 -I tools/cardnames.py --check FILE.c [...]         exit 1 if a file would change
  python3 -I tools/cardnames.py --diff FILE.c                show what would change
  python3 -I tools/cardnames.py --out DIR FILE.c [...]       write the results into DIR

The header only holds '#define readable RAW' lines, so the compiled code does not change:
the tool replaces each raw identifier (D_e48a, sub_s043_150b_000a, ...) by its readable
name in code and comments (never inside string literals) and adds

    #include "darklands/card.h"

after the comment lines at the top of the file (place:/orig:/flags:/...), on its own line.
Declarations stay where they are (their order matters to MSC).  Running it again changes
nothing.  A file in which a readable name is already used for something else (a local,
a struct member ...) would be broken by the macro: such a file is left unchanged and
reported (exit status 2).  tools/cport.py resolves the include from include/ and makes the
header part of the compile cache key.
"""
import sys, os, re, argparse, difflib

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
HEADER = os.path.join(REPO, 'include', 'darklands', 'card.h')
INCLUDE = '#include "darklands/card.h"'

DEFINE = re.compile(r'^\s*#\s*define\s+([A-Za-z_]\w*)\s+([A-Za-z_]\w*)\b', re.M)
# string/char literals and comments are tokens of their own so they are not touched
TOKEN = re.compile(r'"(?:\\.|[^"\\\n])*"|\'(?:\\.|[^\'\\\n])*\'|/\*.*?\*/|//[^\n]*|[A-Za-z_]\w*', re.S)
IDENT = re.compile(r'[A-Za-z_]\w*')
INCLUDE_LINE = re.compile(r'^[ \t]*#[ \t]*include[ \t]+"darklands/card\.h"[ \t]*\n', re.M)


def load_names(path=HEADER):
    """{raw: readable} from the header's #define lines."""
    text = open(path, encoding='latin-1').read()
    text = re.sub(r'/\*.*?\*/', lambda m: '\n' * m.group(0).count('\n'), text, flags=re.S)
    raw2name = {}
    for name, raw in DEFINE.findall(text):
        if raw in raw2name:
            raise SystemExit('%s: %s defined twice (%s, %s)' % (path, raw, raw2name[raw], name))
        raw2name[raw] = name
    return raw2name


def header_end(text):
    """Offset just after the last of the comment lines at the top of the file."""
    pos = end = 0
    in_comment = False
    for line in text.splitlines(True):
        s = line.strip()
        pos += len(line)
        if in_comment:
            if '*/' in s:
                in_comment = False
                end = pos
            continue
        if not s:
            continue
        if s.startswith('/*'):
            in_comment = not s.endswith('*/')
            if not in_comment:
                end = pos
            continue
        break
    return end


def convert(text, raw2name):
    """Return (new text, list of problems)."""
    names = set(raw2name.values())
    problems = []
    has_include = INCLUDE_LINE.search(text) is not None
    # in a file without the include, an identifier that equals a readable name is something
    # else (a local, a struct member ...) and would be macro-expanded: refuse the file
    if not has_include:
        for m in TOKEN.finditer(text):
            s = m.group(0)
            if s[0] not in '"\'/' and s in names:
                problems.append('readable name %r is already used in the file' % s)
    if problems:
        return text, sorted(set(problems))
    body_start = header_end(text)
    head, body = text[:body_start], text[body_start:]

    def sub_ident(m):
        return raw2name.get(m.group(0), m.group(0))

    def tok(m):
        s = m.group(0)
        if s[0] in '"\'':
            return s
        if s[0] == '/':
            return IDENT.sub(sub_ident, s)          # comments: rename too
        return raw2name.get(s, s)
    new_body = TOKEN.sub(tok, body)
    if new_body != body and not has_include:
        if head and not head.endswith('\n'):
            head += '\n'
        head += INCLUDE + '\n'
    return head + new_body, []


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('files', nargs='+')
    ap.add_argument('--check', action='store_true', help='report files that would change (exit 1)')
    ap.add_argument('--diff', action='store_true', help='print a unified diff instead of writing')
    ap.add_argument('--out', help='write converted files into this directory instead')
    ap.add_argument('--header', default=HEADER)
    a = ap.parse_args()
    raw2name = load_names(a.header)
    changed = bad = 0
    for f in a.files:
        text = open(f, encoding='latin-1', newline='').read()
        new, problems = convert(text, raw2name)
        if problems:
            bad += 1
            print('%s: NOT converted: %s' % (f, '; '.join(problems)), file=sys.stderr)
            continue
        if new == text:
            continue
        changed += 1
        if a.check:
            print('%s: would change' % f)
        elif a.diff:
            sys.stdout.writelines(difflib.unified_diff(text.splitlines(True), new.splitlines(True), f, f + ' (readable)'))
        else:
            dst = os.path.join(a.out, os.path.basename(f)) if a.out else f
            if a.out:
                os.makedirs(a.out, exist_ok=True)
            open(dst, 'w', encoding='latin-1', newline='').write(new)
            print('%s: converted%s' % (f, ' -> ' + dst if a.out else ''))
    if bad:
        sys.exit(2)
    if a.check and changed:
        sys.exit(1)


if __name__ == '__main__':
    main()
