#!/usr/bin/env python3
"""mkfuncmod.py: build a FUNCTIONAL build (all usable src/wip drafts) WITH a mod on top.
  python3 -I tools/playtests/mkfuncmod.py MODDIR WORKDIR OUT.EXE [--skip root/text.c ...]
Why: `build.py --mod --functional --overlay MODDIR` appends the src/wip drafts AFTER the
--overlay dirs, so a draft with the same /* place: */ replaces the mod's file (log line
"overlay: .../src/wip/s001/s001_0e76.c replaces mods/strong_party/s001/s001_0e76.c") and the
mod silently does nothing for any module that has a draft (found 2026-10-10).
This script makes WORKDIR/ovl = every stamped draft from src/wip that is not in src/wip/EXCLUDE,
minus the --skip list (default: root/text.c, which build.py --functional skips because root code
cannot grow), with the mod's files replacing drafts of the same place, and runs a plain
`build.py --mod --overlay WORKDIR/ovl` (work dir gets build/qa2/analysis + force_db.json).
Prints the last build line. Reliability: high (deterministic build).
"""
import sys, os, re, shutil, subprocess, argparse
REPO = '/home/claude/darklands-re'
EXE = '/home/claude/dl_raw/extracted/DARKLAND/DARKLAND.EXE'

def place(p):
    for l in open(p, errors='replace').read().splitlines()[:5]:
        m = re.match(r'/\* place: (\S+ \S+) \*/', l)
        if m: return m.group(1)
    return None

def stamped(p):
    return any(l.startswith('/* orig:') for l in open(p, errors='replace').read().splitlines()[:6])

if __name__ == '__main__':
    ap = argparse.ArgumentParser(); ap.add_argument('mod'); ap.add_argument('work'); ap.add_argument('out')
    ap.add_argument('--skip', action='append', default=None)
    a = ap.parse_args()
    skip = set(a.skip or ['root/text.c'])
    wip = os.path.join(REPO, 'src', 'wip')
    excl = set()
    ex = os.path.join(wip, 'EXCLUDE')
    if os.path.exists(ex):
        excl = {l.split('#')[0].strip() for l in open(ex)} - {''}
    modfiles = {}
    for dp, _, fs in os.walk(a.mod):
        for f in fs:
            if f.endswith('.c'):
                p = os.path.join(dp, f); modfiles[place(p)] = p
    ovl = os.path.join(a.work, 'ovl'); shutil.rmtree(ovl, ignore_errors=True); os.makedirs(ovl)
    n = 0
    for dp, _, fs in os.walk(wip):
        for f in fs:
            if not f.endswith('.c'): continue
            p = os.path.join(dp, f); rel = os.path.relpath(p, wip)
            if rel in excl or rel in skip or not stamped(p) or place(p) in modfiles: continue
            os.makedirs(os.path.join(ovl, os.path.dirname(rel)), exist_ok=True)
            shutil.copy(p, os.path.join(ovl, rel)); n += 1
    for pl, p in modfiles.items():
        d = os.path.join(ovl, 'mod_' + os.path.basename(os.path.dirname(p))); os.makedirs(d, exist_ok=True)
        shutil.copy(p, d)
    print('%d drafts + %d mod files in %s' % (n, len(modfiles), ovl))
    for x in ('analysis', 'force_db.json'):
        src = os.path.join(REPO, 'build', 'qa2', x); dst = os.path.join(a.work, x)
        if not os.path.exists(dst):
            (shutil.copytree if os.path.isdir(src) else shutil.copy)(src, dst)
    log = os.path.join(a.work, 'build.log')
    with open(log, 'w') as fo:
        subprocess.run(['python3', '-I', os.path.join(REPO, 'tools', 'build.py'), EXE, '--skip-analysis', '--mod',
                        '--overlay', ovl, '--work', a.work, '--out', a.out], cwd=REPO, stdout=fo, stderr=subprocess.STDOUT)
    print(open(log).read().strip().splitlines()[-1])
