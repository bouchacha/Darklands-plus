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
import os, re, hashlib, json, sys, struct
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


def compat600(obj):
    """Rewrite MSC 6.00A's memset intrinsic into the form found in DARKLAND.EXE.

    6.00A always fills with 'rep stosb'; the game's code (built with a different 6.00 release)
    fills even-sized blocks with 'rep stosw' and keeps the address in BX. Both forms have the
    same length, so the rewrite is purely byte-for-byte inside the code:
        mov dx,X / mov cx,2n / mov di,dx / push ds / pop es / rep stosb
     -> mov bx,X / mov cx,n  / mov di,bx / push ds / pop es / rep stosw
    plus a following 'mov di,Y / mov cx,2n / rep stosb' that reuses ES and AX, and
        mov di,X / mov cx,2n / push ds / pop es / rep stosb  ->  ... mov cx,n ... rep stosw
    Only applied where AX holds a byte-replicated value (sub ax,ax or mov ax,0FFFFh).
    The pattern is matched on each segment's data as a whole: MSC splits code into LEDATA
    records at arbitrary points."""
    out = bytearray(obj)
    recs = []                 # (segment index, segment offset, data start in obj, data length)
    p = 0
    while p < len(out):
        rt = out[p]
        ln = struct.unpack_from('<H', out, p + 1)[0]
        if rt == 0xa0:
            q0 = p + 3
            if out[q0] & 0x80:
                si = ((out[q0] & 0x7f) << 8) | out[q0 + 1]
                q0 += 2
            else:
                si = out[q0]
                q0 += 1
            off = struct.unpack_from('<H', out, q0)[0]
            q0 += 2
            recs.append((si, off, q0, p + 3 + ln - 1 - q0))
        p += 3 + ln
    by_seg = {}
    for si, off, q0, n in recs:
        by_seg.setdefault(si, []).append((off, q0, n))
    for si, parts in by_seg.items():
        size = max(off + n for off, q0, n in parts)
        buf = bytearray(size)
        for off, q0, n in parts:
            buf[off:off + n] = out[q0:q0 + n]
        orig = bytes(buf)
        _rewrite_memset(buf)
        if bytes(buf) != orig:
            for off, q0, n in parts:
                out[q0:q0 + n] = buf[off:off + n]
    return bytes(out)


def _rewrite_memset(b):
    end = len(b)
    q = 0
    while q < end - 9:
        if q + 12 <= end and b[q] == 0xba and b[q + 3] == 0xb9 and b[q + 6:q + 8] == b'\x8b\xfa' and \
                b[q + 8:q + 12] == b'\x1e\x07\xf3\xaa':
            n = struct.unpack_from('<H', b, q + 4)[0]
            prev = bytes(b[max(0, q - 3):q])
            if n % 2 == 0 and (prev.endswith(b'\xb8\xff\xff') or prev.endswith(b'\x2b\xc0')):
                b[q] = 0xbb
                struct.pack_into('<H', b, q + 4, n // 2)
                b[q + 6:q + 8] = b'\x8b\xfb'
                b[q + 11] = 0xab
                r = q + 12
                while r + 8 <= end and b[r] == 0xbf and b[r + 3] == 0xb9 and b[r + 6:r + 8] == b'\xf3\xaa':
                    m = struct.unpack_from('<H', b, r + 4)[0]
                    if m % 2:
                        break
                    struct.pack_into('<H', b, r + 4, m // 2)
                    b[r + 7] = 0xab
                    r += 8
                q = r
                continue
        # mov di,X / mov cx,2n / push ds / pop es / rep stosb  (AX byte-replicated)
        if q + 10 <= end and b[q] == 0xbf and b[q + 3] == 0xb9 and b[q + 6:q + 10] == b'\x1e\x07\xf3\xaa':
            n = struct.unpack_from('<H', b, q + 4)[0]
            prev = bytes(b[max(0, q - 3):q])
            if n % 2 == 0 and (prev.endswith(b'\xb8\xff\xff') or prev.endswith(b'\x2b\xc0')):
                struct.pack_into('<H', b, q + 4, n // 2)
                b[q + 9] = 0xab
                q += 10
                continue
        q += 1


def compile_source(src, cache_dir, extra_headers=()):
    """Compile one placed source; returns dict with obj path, code segment, length, publics."""
    os.makedirs(cache_dir, exist_ok=True)
    seg = ('P_%s_%04x_%04x' % (src['image'], src['frame'], src['offset'])).upper()   # MSC upper-cases /NT
    flags = list(src['flags']) + ['/NT', seg]
    blob = open(src['path'], 'rb').read()
    for h in extra_headers:
        blob += open(h, 'rb').read()
    key = hashlib.md5(blob + ' '.join(flags).encode() + b'compat600-v3').hexdigest()[:16]
    objp = os.path.join(cache_dir, '%s_%s.obj' % (seg, key))
    msg = ''
    if not os.path.exists(objp):
        obj, out, _ = msc.compile_c(src['path'], flags, extra_files=extra_headers)
        msg = out
        if obj is None or re.search(r'error [CD]\d+', out):
            raise SystemExit('compile failed: %s\n%s' % (src['path'], out))
        open(objp, 'wb').write(compat600(obj))
    segs, m = msc.obj_code(open(objp, 'rb').read())
    code = segs.get(seg)
    if code is None:
        raise SystemExit('%s: code segment %s not found (segments: %s)' % (src['path'], seg, list(segs)))
    # data contributions (string literals -> _DATA, far-segment words -> CONST) must land where
    # the original module's data sits in DGROUP. Placement is inferred from the original code
    # (see infer_data_placement), or given with a '/* data: _DATA=xxxx */' hint.
    head = open(src['path'], encoding='latin-1').read(3000)
    hints = dict((k, int(v, 16)) for k, v in re.findall(r'/\*\s*data:\s*(\w+)=([0-9a-fA-F]{1,4})\s*\*/', head))
    datasegs = {}
    for i in range(1, len(m.segs)):
        n = m.segs[i][0]
        if n == seg:
            continue
        ext = max([off + len(blob) for s_, off, blob in m.data if s_ == i] or [0])
        if ext == 0:
            continue
        if n not in ('_DATA', 'CONST'):
            raise SystemExit('%s: unsupported non-empty segment %s' % (src['path'], n))
        datasegs[n] = ext
    si = [i for i in range(1, len(m.segs)) if m.segs[i][0] == seg][0]
    pubs = [(n, off) for n, s, off in m.pubs if s == si]
    return dict(src, obj=objp, segment=seg, length=len(code[0]), publics=pubs, messages=msg,
                data=dict(hints), datasegs=datasegs)


def infer_data_placement(c, orig):
    """Fill c['data'] (segment name -> DGROUP offset) from the original code: every offset
    fixup in the compiled code that targets a data segment points at an operand whose value
    in the original binary is the DGROUP offset of (segment start + displacement)."""
    segs, m = msc.obj_code(open(c['obj'], 'rb').read())
    code = segs[c['segment']][0]
    si = [i for i in range(1, len(m.segs)) if m.segs[i][0] == c['segment']][0]
    votes = {}
    for fsi, off, loctype, M, frame, target, disp in m.fixups:
        if fsi != si or loctype not in (1, 5) or not M or target[0] != 0:
            continue
        tn = m.segs[target[1]][0]
        if tn not in c['datasegs'] or tn in c['data']:
            continue
        addend = struct.unpack_from('<H', code, off)[0]
        ow = struct.unpack_from('<H', orig, off)[0]
        place = (ow - disp - addend) & 0xffff
        votes.setdefault(tn, {}).setdefault(place, 0)
        votes[tn][place] += 1
    for tn, v in votes.items():
        c['data'][tn] = max(v.items(), key=lambda kv: kv[1])[0]
    missing = [n for n in c['datasegs'] if n not in c['data']]
    if missing:
        raise SystemExit('%s: cannot place data segment(s) %s; add a /* data: NAME=xxxx */ hint'
                         % (c['path'], ', '.join(missing)))
    return c


_DG = None


def dgroup_bytes():
    """Initialised part of the original DGROUP (offset 0 = DGROUP:0000)."""
    global _DG
    if _DG is None:
        from rtlink import RTLinkExe
        exe = RTLinkExe(os.environ.get('DARKLAND_EXE', '/home/claude/dl_raw/extracted/DARKLAND/DARKLAND.EXE'))
        s = exe.sections[136]
        _DG = bytes(s.image[0x321a0 - s.load_seg * 16:])
    return _DG
