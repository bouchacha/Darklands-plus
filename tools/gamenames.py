#!/usr/bin/env python3
"""
Rewrite engine sources (src/c/root, src/c/s000..s043, src/wip/**) to use the readable names of
include/darklands/game.h and include/darklands/card.h.  The engine counterpart of
tools/cardnames.py (which stays the tool for the card scripts s044..s135).

  python3 -I tools/gamenames.py FILE.c [FILE.c ...]          rewrite in place
  python3 -I tools/gamenames.py --check FILE.c [...]         exit 1 if a file would change
  python3 -I tools/gamenames.py --diff FILE.c                show what would change
  python3 -I tools/gamenames.py --out DIR FILE.c [...]       write the results into DIR
  python3 -I tools/gamenames.py --raw FILE.c                 list the raw names left in a file
                                                             that have no readable name yet

Both headers only hold '#define readable RAW' lines, so the compiled code does not change: the
tool replaces each raw identifier (D_9c55, F_20a5_0000, sub_root_0617_045c, vec_1c2f ...) by its
readable name in code and comments (never inside string or character literals) and adds

    #include "darklands/card.h"
    #include "darklands/game.h"

after the comment lines at the top of the file (place:/orig:/flags:/status ...), which are left
as they are.  Declarations stay where they are (their order matters to MSC).  Running it again
changes nothing.  Also left alone:
  - directive comments that name functions (/* compat600-off: ... */, /* empty-ok: ... */);
  - the argument text of assert(...) (it is stringised into the program's data);
  - number literals (0x9c69 is not an identifier).
A file is refused (left unchanged and reported, exit status 2) when
  - a readable name is already used in it for something else (a local, a struct member, a
    label ...) - the macro would replace it; or
  - it #defines / #undefs a readable name of the headers (a raw name the file #defines
    itself is a file-local macro and simply keeps its raw spelling in that file); or
  - it defines a macro that stringises (#) or pastes (##) tokens; or
  - it is a card script (s044..s135: use tools/cardnames.py); or
  - it carries a '/* gamenames: off */' comment (a file whose code changes when game.h is
    included: MSC 6.00A's symbol-table order, see docs/ENGINE.md).
The refusal check needs the file in its raw form: once the include line is in, a readable name
is assumed to be the macro.  After adding names to game.h, convert again and re-run
tools/fmatch.py on the converted files (a new name may equal a local of a converted file).
"""
import sys, os, re, argparse, difflib

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import cardnames

REPO = os.path.dirname(HERE)
CARD_H = os.path.join(REPO, 'include', 'darklands', 'card.h')
GAME_H = os.path.join(REPO, 'include', 'darklands', 'game.h')
INCLUDES = '#include "darklands/card.h"\n#include "darklands/game.h"\n'
GAME_INCLUDE = re.compile(r'^[ \t]*#[ \t]*include[ \t]+"darklands/game\.h"[ \t]*\n', re.M)
CARD_INCLUDE = re.compile(r'^[ \t]*#[ \t]*include[ \t]+"darklands/card\.h"[ \t]*\n', re.M)

# strings, chars, comments, pp-numbers and identifiers, in that priority at each position
TOKEN = re.compile(r'"(?:\\.|[^"\\\n])*"|\'(?:\\.|[^\'\\\n])*\'|/\*.*?\*/|//[^\n]*'
                   r'|\.?[0-9](?:[eEpP][+-]|[A-Za-z0-9_.])*|[A-Za-z_]\w*', re.S)
IDENT = re.compile(r'[A-Za-z_]\w*')
RAW = re.compile(r'^(D_[0-9a-f]{4}|F_[0-9a-f]{4}_[0-9a-f]{1,4}|sub_\w+_[0-9a-f]{4}_[0-9a-f]{4}|vec_[0-9a-f]{4})$')
DIRECTIVE = re.compile(r'^/\*\s*(compat600-off|empty-ok)\s*:')
PP_DEFINE = re.compile(r'^[ \t]*#[ \t]*(define|undef)[ \t]+([A-Za-z_]\w*)(.*)$', re.M)
OPT_OUT = re.compile(r'/\*\s*gamenames:\s*off\b')
CARD_DIR = re.compile(r'(^|[\\/])s(04[4-9]|0[5-9]\d|1[0-2]\d|13[0-5])[\\/]')


def load_tables(card_h=CARD_H, game_h=GAME_H):
    """{raw: readable} of both headers; a raw or a readable name may appear only once."""
    a = cardnames.load_names(card_h)
    b = cardnames.load_names(game_h) if os.path.exists(game_h) else {}
    for raw, name in b.items():
        if raw in a:
            raise SystemExit('%s: %s is already named %s in card.h (define %s there only)' % (
                game_h, raw, a[raw], name))
    raw2name = dict(a)
    raw2name.update(b)
    seen = {}
    for raw, name in raw2name.items():
        if name in seen:
            raise SystemExit('readable name %s used for %s and %s' % (name, seen[name], raw))
        seen[name] = raw
    return raw2name


def assert_spans(text):
    """(start, end) of the argument text of every assert(...) call."""
    out = []
    for m in re.finditer(r'\bassert\s*\(', text):
        depth, i = 1, m.end()
        while i < len(text) and depth:
            c = text[i]
            if c == '(':
                depth += 1
            elif c == ')':
                depth -= 1
            elif c in '"\'':
                t = TOKEN.match(text, i)
                if t:
                    i = t.end()
                    continue
            i += 1
        out.append((m.end(), i))
    return out


def problems_of(text, raw2name, path=''):
    names = set(raw2name.values())
    probs = []
    if path and CARD_DIR.search(path.replace(os.sep, '/')):
        probs.append('card script: use tools/cardnames.py')
    if OPT_OUT.search(text):
        probs.append('marked /* gamenames: off */')
    has_include = GAME_INCLUDE.search(text) is not None
    for m in PP_DEFINE.finditer(text):
        kind, name, rest = m.groups()
        if name in names:
            probs.append('#%s of %s' % (kind, name))
        if kind == 'define' and '#' in rest.split('/*')[0]:
            probs.append('macro %s uses # or ##' % name)
    if not has_include:
        for m in TOKEN.finditer(text):
            s = m.group(0)
            if s[0] not in '"\'/' and not s[0].isdigit() and s[0] != '.' and s in names:
                probs.append('readable name %r is already used in the file' % s)
    return sorted(set(probs))


def convert(text, raw2name, path=''):
    """Return (new text, list of problems)."""
    probs = problems_of(text, raw2name, path)
    if probs:
        return text, probs
    has_include = GAME_INCLUDE.search(text) is not None
    # a raw name the file #defines itself (e.g. '#define F_20a5_0000 (F_20a5_000.cell)') is a
    # file-local macro: it keeps its raw name everywhere in that file
    local = set(n for k, n, r in PP_DEFINE.findall(text) if n in raw2name)
    if local:
        raw2name = dict((k, v) for k, v in raw2name.items() if k not in local)
    body_start = cardnames.header_end(text)
    head, body = text[:body_start], text[body_start:]
    protect = assert_spans(body)

    def protected(pos):
        return any(a <= pos < b for a, b in protect)

    def sub_ident(m):
        return raw2name.get(m.group(0), m.group(0))

    def tok(m):
        s = m.group(0)
        if s[0] in '"\'' or s[0].isdigit() or s[0] == '.':
            return s
        if s[0] == '/':
            if DIRECTIVE.match(s):
                return s
            return IDENT.sub(sub_ident, s)          # comments: rename too
        if s in raw2name and protected(m.start()):
            return s
        return raw2name.get(s, s)
    new_body = TOKEN.sub(tok, body)
    if new_body != body and not has_include:
        if head and not head.endswith('\n'):
            head += '\n'
        head += INCLUDES if not CARD_INCLUDE.search(text) else '#include "darklands/game.h"\n'
    return head + new_body, []


def raw_left(text, raw2name):
    """Raw names (code only) that have no readable name, with their counts."""
    out = {}
    for m in TOKEN.finditer(text):
        s = m.group(0)
        if RAW.match(s) and s not in raw2name:
            out[s] = out.get(s, 0) + 1
    return out


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('files', nargs='+')
    ap.add_argument('--check', action='store_true', help='report files that would change (exit 1)')
    ap.add_argument('--diff', action='store_true', help='print a unified diff instead of writing')
    ap.add_argument('--out', help='write converted files into this directory instead')
    ap.add_argument('--raw', action='store_true', help='list raw names without a readable name')
    ap.add_argument('--card-header', default=CARD_H)
    ap.add_argument('--header', default=GAME_H)
    a = ap.parse_args()
    raw2name = load_tables(a.card_header, a.header)
    changed = bad = 0
    for f in a.files:
        text = open(f, encoding='latin-1', newline='').read()
        if a.raw:
            left = raw_left(text, raw2name)
            print('%s: %d raw names left%s' % (f, len(left), ''.join(
                '\n  %-28s %d' % kv for kv in sorted(left.items(), key=lambda kv: -kv[1]))))
            continue
        new, problems = convert(text, raw2name, os.path.abspath(f))
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
