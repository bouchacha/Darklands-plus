#!/usr/bin/env python3
"""
C sources that replace original functions (the matching decompilation).

Every file under src/c/ starts with a placement line:

    /* place: <image> <frame>:<offset> */          e.g.  /* place: root 0410:0008 */

and optionally a flags line (default /AM /G2 /Gs /Ox):

    /* flags: /AM /G2 /Gs /Ox */

The file is compiled with Microsoft C 6.00A (tools/msc.py). Its code must replace a
contiguous run of original functions starting at the placement address, in source order;
the build removes exactly that byte range from the generated assembly and links the
compiled object in its place.

Naming conventions understood by the linker for references from C:
  foo()          -> the assembly label 'foo' (C adds '_'; the linker strips it as a fallback),
                    so calling an unported function is   extern int far sub_root_047c_07ce();
  D_xxxx         -> DGROUP:xxxx            (extern int D_6eee;  extern char D_4f02[];)
  F_ssss_xxxx    -> far data ssss:xxxx     (extern char far F_290e_0123[];)
"""
import os, re, hashlib, json, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import msc

PLACE = re.compile(r'/\*\s*place:\s*(\w+)\s+([0-9a-fA-F]{4}):([0-9a-fA-F]{4})\s*\*/')
FLAGS = re.compile(r'/\*\s*flags:\s*([^*]+)\*/')


def sources(repo):
    out = []
    base = os.path.join(repo, 'src', 'c')
    for dp, dn, fn in os.walk(base):
        for f in sorted(fn):
            if not f.lower().endswith('.c'):
                continue
            p = os.path.join(dp, f)
            head = open(p, encoding='latin-1').read(2000)
            m = PLACE.search(head)
            if not m:
                continue
            fm = FLAGS.search(head)
            out.append({'path': p, 'image': m.group(1), 'frame': int(m.group(2), 16),
                        'offset': int(m.group(3), 16),
                        'flags': fm.group(1).split() if fm else list(msc.DEFAULT)})
    return sorted(out, key=lambda s: s['path'])


def compile_source(src, cache_dir, extra_headers=()):
    """Compile one placed source; returns dict with obj path, code segment, length, publics."""
    os.makedirs(cache_dir, exist_ok=True)
    seg = ('P_%s_%04x_%04x' % (src['image'], src['frame'], src['offset'])).upper()   # MSC upper-cases /NT
    flags = list(src['flags']) + ['/NT', seg]
    blob = open(src['path'], 'rb').read()
    for h in extra_headers:
        blob += open(h, 'rb').read()
    key = hashlib.md5(blob + ' '.join(flags).encode()).hexdigest()[:16]
    objp = os.path.join(cache_dir, '%s_%s.obj' % (seg, key))
    msg = ''
    if not os.path.exists(objp):
        obj, out, _ = msc.compile_c(src['path'], flags, extra_files=extra_headers)
        msg = out
        if obj is None or re.search(r'error [CD]\d+', out):
            raise SystemExit('compile failed: %s\n%s' % (src['path'], out))
        open(objp, 'wb').write(obj)
    segs, m = msc.obj_code(open(objp, 'rb').read())
    code = segs.get(seg)
    if code is None:
        raise SystemExit('%s: code segment %s not found (segments: %s)' % (src['path'], seg, list(segs)))
    for n, (b, fx) in segs.items():
        if n != seg and len(b):
            raise SystemExit('%s: segment %s is not empty - keep data in the original DGROUP '
                             '(use extern D_xxxx names instead of literals/initialised globals)' % (src['path'], n))
    si = [i for i in range(1, len(m.segs)) if m.segs[i][0] == seg][0]
    pubs = [(n, off) for n, s, off in m.pubs if s == si]
    return dict(src, obj=objp, segment=seg, length=len(code[0]), publics=pubs, messages=msg)
