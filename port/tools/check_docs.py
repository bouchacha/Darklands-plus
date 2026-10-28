#!/usr/bin/env python3
"""check_docs.py - what the player-facing texts say against the code, so that no text drifts from it.

  check_docs.py [--fix]    (from anywhere; exit 1 on a mismatch; release_check.py runs it first)

The same things are said in several places - README.md, the changelog's newest section, the release
notes (docs/release_notes.md, what is above "New in 1.2" and from "What it adds" on), the README of
the download (tools/release_readme.txt), docs/MODDING.md and the game's own welcome and news windows
(ui/ui_windows.cpp). Older sections of the changelog and release notes are history and left alone.
Checked:
  - a skill's or attribute's shield is never drawn by a fixed number (it stands for that skill only);
  - counts: "N fixes" and "(N in all)" are the fixes of the original's bugs (mods/fixes.c FIXES, less
    "Skip the manual check", which is no bug); "N more fixed" is the count less the last release's
    (git tag of the version before); "N saints that" is mods/sainttexts.c's table;
  - the changelog's newest section and the release notes' "New in" section are the same text;
  - every "F12 > A > B ..." names a real place: A one of the F12 window's tabs (ui/ui.cpp), B under
    Settings one of its sections (ui/settings.cpp, mods/options.c), anything further a label the code
    has (a string in ui/ or mods/);
  - the presets named (*Full Original*, *Purist*, *Darklands+* ...) are the presets (mods/options.c);
  - the docs made from the code are as it makes them now: docs/CODE_MAP.md (tools/code_map.py),
    docs/COPY.md (tools/copy_catalog.py) and runtime/gamevars.h (tools/gen_gamevars.py, from the
    decompilation's names: a name added there by hand would be lost at the next run). With --fix they are written anew instead of reported. And
    every source file starts with a header comment saying what it is (CODE_MAP.md's description).
"""
import os, re, subprocess, sys

TOP = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
PORT = os.path.join(TOP, 'port')


def rd(p):
    return open(os.path.join(TOP, p), encoding='utf-8', errors='replace').read()


def c_strings(text):
    out = []
    for m in re.finditer(r'"((?:[^"\\\n]|\\.)*)"', text):
        out.append(m.group(1).replace('\\"', '"'))
    return out


def fixes_count(src):
    body = src[src.index('Fix FIXES[] = {'):]
    body = body[:body.index('\n};')]
    ids = re.findall(r'^    \{ "([a-z0-9_ ]+)",', body, re.M)
    return len([i for i in ids if i != 'copyprot'])


def sections_of_docs():
    """(name, text) of each player-facing text, the current part only"""
    out = [('README.md', rd('README.md'))]
    ch = rd('CHANGELOG.md')
    secs = [m.group(0) for m in re.finditer(r'^## .*?(?=^## |\Z)', ch, re.M | re.S)]
    out.append(('CHANGELOG.md (newest section)', secs[0] if secs else ''))
    out.append(('CHANGELOG.md (newest release)', next((t for t in secs if 'not yet released' not in t.splitlines()[0]), '')))   # (what the release notes say)
    rn = rd('docs/release_notes.md')
    i = rn.find('**New in 1.2**'); j = rn.find('### What it adds')
    out.append(('docs/release_notes.md', rn[:i if i >= 0 else len(rn)] + (rn[j:] if j >= 0 else '')))
    out.append(('tools/release_readme.txt', rd('port/tools/release_readme.txt')))
    out.append(('docs/MODDING.md', rd('docs/MODDING.md')))
    out.append(('ui/ui_windows.cpp (welcome, news)', '\n'.join(c_strings(rd('port/ui/ui_windows.cpp')))))
    return out


GENERATED = [('docs/CODE_MAP.md', 'port/tools/code_map.py'), ('docs/COPY.md', 'port/tools/copy_catalog.py'),
             ('port/runtime/gamevars.h', 'port/tools/gen_gamevars.py --stdout'),
             ('docs/MERGING.md', 'port/tools/switch_map.py')]   # (its section 4: every switch and the files that read it)


def generated(fix):
    """the docs made from the code: (what is stale, as mismatches) - or, with fix, written anew"""
    out = []
    for doc, tool in GENERATED:
        now = subprocess.run([sys.executable, '-I', os.path.join(TOP, tool.split()[0])] + tool.split()[1:], capture_output=True, text=True, cwd=TOP).stdout
        if not now.strip(): out.append('%s: %s made nothing' % (doc, tool)); continue
        if doc.endswith('CODE_MAP.md'):              # (every file starts with a header saying what it is)
            for f in re.findall(r'^\| `([^`]+)` \| \d+ \| \s*\|$', now, re.M):
                out.append('port/%s: no header comment saying what it is (the first thing in the file)' % f)
        if now == rd(doc): continue
        if fix:
            open(os.path.join(TOP, doc), 'w', encoding='utf-8').write(now)
            print('  %s written anew' % doc)
        else:
            out.append('%s: not as %s makes it now (python3 %s > %s, or check_docs.py --fix)' % (doc, tool.split()[0], tool, doc))
    return out


def main():
    bad = generated('--fix' in sys.argv[1:])
    fixes_src = rd('port/mods/fixes.c')
    n_fix = fixes_count(fixes_src)
    st = rd('port/mods/sainttexts.c')
    st = st[st.index('SAINTS[] = {'):]; st = st[:st.index('\n};')]
    n_saints = len(re.findall(r'\{ \d+, 0x[0-9a-f]+,', st))
    # the last release's count (the newest tag before this version)
    prev = None
    tags = subprocess.run(['git', '-C', TOP, 'tag', '--sort=-creatordate'], capture_output=True, text=True).stdout.split()
    if tags:
        old = subprocess.run(['git', '-C', TOP, 'show', '%s:port/mods/fixes.c' % tags[0]], capture_output=True, text=True).stdout
        if 'Fix FIXES[] = {' in old: prev = fixes_count(old)

    # the F12 window: its tabs, and the Settings page's sections
    ui = rd('port/ui/ui.cpp')
    tabs = set(re.findall(r'\{ "([^"]+)", tab_', ui))
    sett = rd('port/ui/settings.cpp'); opts = rd('port/mods/options.c')
    sections = {'Preset', 'Fixes'}
    for arr in re.findall(r'(?:LOOKS|SETUP)\[\] = \{([^}]*)\}', sett): sections |= set(re.findall(r'"([^"]+)"', arr))
    sdef = dict(re.findall(r'#define (S_[A-Z_]+)\s+"([^"]+)"', opts))
    for key, sec in re.findall(r'^    \{ "([a-z_.0-9]+)", (S_[A-Z_]+|"[^"]*"),', opts, re.M):
        sections.add(sdef.get(sec, sec.strip('"')))
    labels = set()
    for d in ('port/ui', 'port/mods'):
        for f in os.listdir(os.path.join(TOP, d)):
            if f.endswith(('.c', '.cpp', '.h')):
                for s in c_strings(rd(os.path.join(d, f))): labels.add(re.split(r'##', s)[0].strip())
    presets = re.findall(r'^    \{ "([^"]+)", "', opts[opts.index('PRESETS[PRESET_COUNT] = {'):], re.M)

    texts = sections_of_docs()
    unreleased = 'not yet released' in texts[1][1].splitlines()[0] if texts[1][1] else False
    for name, t in texts:
        flat = re.sub(r'\s+', ' ', t.replace('**', '').replace('\\n', ' '))
        if unreleased and name in ('CHANGELOG.md (newest release)', 'docs/release_notes.md'):
            flat = re.sub(r'\d+ fixes\b|\(\d+ in all\)|\d+ more fixed|\d+ saints that', '', flat)   # (a release's own counts: what its download held)
        for m in re.finditer(r'(\d+) fixes\b', flat):
            if int(m.group(1)) != n_fix: bad.append('%s: "%s" - the code has %d fixes' % (name, m.group(0), n_fix))
        for m in re.finditer(r'\((\d+) in all\)', flat):
            if 'fix' in flat[max(0, m.start() - 40):m.start()] and int(m.group(1)) != n_fix: bad.append('%s: "%s" - the code has %d fixes' % (name, m.group(0), n_fix))
        for m in re.finditer(r'(\d+) more fixed', flat):
            if prev is not None and int(m.group(1)) != n_fix - prev: bad.append('%s: "%s" - %d now, %d in %s: %d more' % (name, m.group(0), n_fix, prev, tags[0], n_fix - prev))
        for m in re.finditer(r'(\d+) saints that', flat):
            if int(m.group(1)) != n_saints: bad.append('%s: "%s" - mods/sainttexts.c has %d' % (name, m.group(0), n_saints))
        for m in re.finditer(r'F12 > ([^.;:()]*)', flat):
            parts = [p.replace('"', '').strip().strip("'*") for p in m.group(1).split(' > ')]
            path = []
            for k, p in enumerate(parts):
                if k == 0: known = tabs
                elif path == ['Settings'] and k == 1: known = sections
                else: known = labels
                # the longest known name the words start with
                hit = max((x for x in known if x and (p == x or p.startswith(x + ' ') or p.startswith(x + "'") or p.startswith(x + ','))), key=len, default=None)
                if not hit:
                    bad.append('%s: "F12 > %s" - no %s "%s"' % (name, ' > '.join(parts[:k + 1]), ['tab', 'section' if path == ['Settings'] else 'label'][min(k, 1)] if k < 2 else 'label', p.split(' ')[0]))
                    break
                path.append(hit)
                if hit != p: break              # (the path ends where the sentence goes on)
        for m in re.finditer(r'\*([A-Z][A-Za-z+ ]+)\*', t.replace('**', '')):
            w = m.group(1).strip()
            if 'preset' in t[m.end():m.end() + 200].lower() or 'preset' in t[max(0, m.start() - 200):m.start()].lower():
                if w not in presets and w + ' (recommended)' not in presets and not any(w == p.split(' (')[0] for p in presets):
                    bad.append('%s: preset "%s" - the presets are %s' % (name, w, ', '.join(presets)))

    # the changelog's newest section and the release notes' "New in" section: the same text
    ch = texts[2][1]                     # (the newest released version: the release notes are the download's)
    rn = rd('docs/release_notes.md')
    a = rn.find('**New in '); b = rn.find('**New in ', a + 1)
    def body(s):
        lines = [l.rstrip() for l in s.splitlines() if l.strip()]
        k = next((i for i, l in enumerate(lines) if l.startswith('**') and not l.startswith('**New in')), len(lines))
        return lines[k:]
    if a >= 0 and body(ch) != body(rn[a:b if b > 0 else len(rn)]):
        x, y = body(ch), body(rn[a:b if b > 0 else len(rn)])
        d = next((i for i in range(min(len(x), len(y))) if x[i] != y[i]), min(len(x), len(y)))
        bad.append('CHANGELOG.md newest section and docs/release_notes.md "New in": differ from line %d:\n      %s\n      %s' % (d + 1, x[d][:100] if d < len(x) else '(end)', y[d][:100] if d < len(y) else '(end)'))

    # a skill's or attribute's shield stands for that skill or attribute only: never one drawn by a fixed
    # number for something else (the camp's guard once borrowed Polearms); a constant is named for its skill
    for d in ('port/ui', 'port/mods'):
        for f in sorted(os.listdir(os.path.join(TOP, d))):
            if not f.endswith(('.c', '.cpp')): continue
            for i, line in enumerate(rd(os.path.join(d, f)).splitlines()):
                if re.search(r'ui_(skill|attr)_icon\([^;]*,\s*\d+\s*\)', line):
                    bad.append('%s/%s:%d: a shield drawn by a fixed number (name the skill: a constant, or the thing it shows)' % (d, f, i + 1))

    # a shield written into a text ({Strength}, {Edged:+3}: ui/richtext.h) names an attribute or skill the
    # pictures know (ui_key_of in ui/skillicons.cpp); any other name would be printed in its braces
    gm = rd('port/mods/gamemodel.c')
    known = set(re.findall(r'"([^"]+)"', gm[gm.index('const char *gm_attr_name'):gm.index('/* the months')]))
    known |= {'Latin', 'Reputation', 'Fame', 'Age', 'Missile Device', 'Missile Weapons', 'Edged Weapon', 'Impact Weapon', 'Flail', 'Polearm', 'Thrown Weapon', 'Bow'}
    known |= set(re.findall(r'"([^"]+)"', gm[gm.index('const char *gm_skill_short'):][:400]))
    for d in ('port/ui', 'port/mods'):
        for f in sorted(os.listdir(os.path.join(TOP, d))):
            if not f.endswith(('.c', '.cpp')): continue
            for i, line in enumerate(rd(os.path.join(d, f)).splitlines()):
                for lit in re.findall(r'"((?:[^"\\]|\\.)*)"', line):
                    for m in re.finditer(r'\{([A-Z][a-z][A-Za-z &]*)(?::[^{}]*)?\}', lit):   # (not a controller button's {A}, {LSTICK})
                        if m.group(1) not in known:
                            bad.append('%s/%s:%d: "{%s}" - no attribute or skill of that name for its shield (ui/richtext.h)' % (d, f, i + 1, m.group(1)))

    for b_ in bad: print('  ' + b_)
    print('docs: %s (%d fixes, %d saints, %d texts)' % ('as the code' if not bad else '%d mismatches' % len(bad), n_fix, n_saints, len(texts)))
    sys.exit(1 if bad else 0)


if __name__ == '__main__':
    main()
