#!/usr/bin/env python3
"""
Per-function matching check for one placed C file (see cport.py for the 'place:' line).

  python3 -I tools/fmatch.py FILE.c [-v]

Unlike cmatch.py (which compares the whole compiled range against the original bytes and so
stops being informative after the first function whose length differs), this compares each
compiled function with its original counterpart *after relocating it to the original address*:
calls between functions of the file, jump tables and data references are resolved as if every
function sat at its original offset. Functions are paired with the original source file's
functions in order (build/analysis/srcfiles.json).

Result: per function MATCH / DIFFERENT (+ first differing offset), plus string/CONST data.
"""
import sys, os, json, struct, re, bisect
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import cport, msc
from rtlink import RTLinkExe

REPO = os.path.dirname(HERE)
EXE = os.environ.get('DARKLAND_EXE', '/home/claude/dl_raw/extracted/DARKLAND/DARKLAND.EXE')
DG = 0x321a

_cache = {}


def load_ctx(exe=None):
    if 'ctx' not in _cache:
        exe = exe or RTLinkExe(EXE)
        syms = json.load(open(os.path.join(REPO, 'build', 'symbols.json')))
        files = json.load(open(os.path.join(REPO, 'build', 'analysis', 'srcfiles.json')))
        byfile = {}
        starts = {}
        for f in files:
            byfile[(f['image'], int(f['frame'], 16))] = f
            starts.setdefault(f['image'], []).append(int(f['start'], 16))
        # a file ends where the next one in the same image starts (or at the end of the image)
        for f in files:
            st = sorted(starts[f['image']])
            k = st.index(int(f['start'], 16))
            f['end'] = st[k + 1] if k + 1 < len(st) else None
        mems = {}
        _cache['ctx'] = dict(exe=exe, syms=syms, files=byfile, mems=mems)
    return _cache['ctx']


def image_mem(ctx, img):
    if img not in ctx['mems']:
        exe = ctx['exe']
        if img == 'root':
            ctx['mems'][img] = (exe.root_memory(), 0)
        else:
            i = int(img[1:])
            ctx['mems'][img] = (exe.section_memory(i), exe.sections[i].load_seg * 16)
    return ctx['mems'][img]


def resolve(name, syms):
    if name in syms:
        return syms[name]
    m = re.match(r'^_D_([0-9a-fA-F]{1,4})$', name)
    if m:
        return ['s136', DG * 16 + int(m.group(1), 16), DG]
    m = re.match(r'^_F_([0-9a-fA-F]{4})_([0-9a-fA-F]{1,4})$', name)
    if m:
        f = int(m.group(1), 16)
        return ['s136', f * 16 + int(m.group(2), 16), f]
    if name.startswith('_') and name[1:] in syms:
        return syms[name[1:]]
    return None


def orig_functions(ctx, img, frame, offset):
    """Original functions (name, linear start, linear end) of the source file containing img frame:offset."""
    f = ctx['files'].get((img, frame))
    if f is None:
        return None
    starts = []
    for fn in f['functions']:
        r = ctx['syms'].get(fn)
        if r is None:
            m = re.match(r'sub_(\w+)_([0-9a-f]{4})_([0-9a-f]{4})$', fn)
            r = [img, int(m.group(2), 16) * 16 + int(m.group(3), 16), int(m.group(2), 16)]
        starts.append((r[1], fn))
    starts.sort()
    end = f['end']
    if end is None:
        mem, lo = image_mem(ctx, img)
        end = lo + len(mem)
        while end > lo and mem[end - 1 - lo] == 0:
            end -= 1
    out = []
    for k, (lin, fn) in enumerate(starts):
        nxt = starts[k + 1][0] if k + 1 < len(starts) else end
        out.append((fn, lin, nxt))
    lo = frame * 16 + offset
    return [x for x in out if x[1] >= lo]


def check(path, verbose=False, flags=None, quiet=False, only=None):
    ctx = load_ctx()
    src = [s for s in cport.sources(REPO) if os.path.samefile(s['path'], path)]
    if not src:
        head = open(path, encoding='latin-1').read(3000)
        m = cport.PLACE.search(head)
        fm = cport.FLAGS.search(head)
        src = [{'path': path, 'image': m.group(1), 'frame': int(m.group(2), 16), 'offset': int(m.group(3), 16),
                'flags': fm.group(1).split() if fm else list(msc.DEFAULT)}]
    s = dict(src[0])
    if flags:
        s['flags'] = flags.split()
    img, frame = s['image'], s['frame']
    res = {'path': path, 'functions': [], 'file_match': False, 'error': None}
    try:
        c = cport.compile_source(s, os.path.join(REPO, 'build', 'cobj'))
    except SystemExit as e:
        res['error'] = str(e)[:2000]
        if not quiet:
            print(res['error'])
        return res
    segs, m = msc.obj_code(open(c['obj'], 'rb').read())
    code = bytearray(segs[c['segment']][0])
    si = [i for i in range(1, len(m.segs)) if m.segs[i][0] == c['segment']][0]
    pubs = sorted((off, n) for n, s_, off in m.pubs if s_ == si)
    origs = orig_functions(ctx, img, frame, s['offset'])
    mem, lo = image_mem(ctx, img)
    # pair compiled functions with original ones in order
    comp = []
    for k, (off, n) in enumerate(pubs):
        end = pubs[k + 1][0] if k + 1 < len(pubs) else len(code)
        comp.append((n, off, end))
    pairs = list(zip(comp, origs or []))
    cstarts = [p[0][1] for p in pairs]

    def comp_to_orig(t):
        k = bisect.bisect_right(cstarts, t) - 1
        if k < 0:
            return None
        (cn, coff, cend), (on, olin, oend) = pairs[k]
        return olin + (t - coff)

    pubaddr = {n: comp_to_orig(off) for off, n in pubs}
    # data placement votes (global over the file)
    data_place = dict(c['data'])
    votes = {}
    for fsi, off, loctype, M, fr, target, disp in m.fixups:
        if fsi != si or loctype not in (1, 5) or not M or target[0] != 0:
            continue
        tn = m.segs[target[1]][0]
        if tn not in c['datasegs'] or tn in data_place:
            continue
        loc = comp_to_orig(off)
        if loc is None:
            continue
        addend = struct.unpack_from('<H', code, off)[0]
        ow = struct.unpack_from('<H', mem, loc - lo)[0]
        if off >= 1 and code[off - 1] != mem[loc - lo - 1]:
            continue
        place = (ow - disp - addend) & 0xffff
        votes.setdefault(tn, {}).setdefault(place, 0)
        votes[tn][place] += 1
    for tn, v in votes.items():
        data_place[tn] = max(v.items(), key=lambda kv: kv[1])[0]
    # the vote can be misled by functions that do not match; prefer a place where the original
    # DGROUP holds exactly the compiled data
    dmem0 = ctx['exe'].section_memory(136)
    dlo0 = ctx['exe'].sections[136].load_seg * 16
    for tn in list(c['datasegs']):
        if tn in c['data']:
            continue        # explicit hint
        di = [i for i in range(1, len(m.segs)) if m.segs[i][0] == tn][0]
        dbuf = bytearray(segs[tn][0][:c['datasegs'][tn]])
        for si2, off2, lt2, M2, fr2, tg2, dp2 in m.fixups:
            if si2 == di and lt2 == 2 and tg2[0] == 2:
                r = resolve(m.exts[tg2[1]], ctx['syms'])
                if r:
                    struct.pack_into('<H', dbuf, off2, r[2])
        if not any(dbuf):
            continue
        base = DG * 16 - dlo0
        hay = bytes(dmem0[base:base + 0x10000])
        v = data_place.get(tn)
        if v is not None and hay[v:v + len(dbuf)] == bytes(dbuf):
            continue
        occ = []
        p0 = hay.find(bytes(dbuf))
        while p0 >= 0 and len(occ) < 5000:
            occ.append(p0)
            p0 = hay.find(bytes(dbuf), p0 + 1)
        if not occ:
            continue
        v = data_place.get(tn)
        if v not in occ:
            data_place[tn] = min(occ, key=lambda o: abs(o - (v if v is not None else o)))
    unresolved = set()
    bad_at = set()
    for fsi, off, loctype, M, fr, target, disp in m.fixups:
        if fsi != si:
            continue
        loc = comp_to_orig(off)
        if loc is None:
            continue
        tm, ti = target
        if tm == 0:
            tn = m.segs[ti][0]
            if tn == c['segment']:
                t = comp_to_orig(disp)
                tlin, tfr = (t if t is not None else 0), frame
            elif tn in data_place:
                tlin, tfr = DG * 16 + data_place[tn] + disp, DG
            else:
                unresolved.add('segment ' + tn)
                bad_at.add(off)
                continue
        elif tm == 2:
            nm = m.exts[ti]
            if nm in pubaddr:
                tlin, tfr = pubaddr[nm] + disp, frame
            else:
                r = resolve(nm, ctx['syms'])
                if r is None:
                    unresolved.add(nm)
                    bad_at.add(off)
                    continue
                tlin, tfr = r[1] + disp, r[2]
        else:
            tlin, tfr = DG * 16 + disp, DG
        fm_, fi = fr
        f_ = tfr
        if fm_ == 1:
            f_ = DG
        elif fm_ == 4:
            f_ = frame
        if loctype in (1, 5):
            v = (tlin - f_ * 16) if M else (tlin - (loc + 2))
            struct.pack_into('<H', code, off, (struct.unpack_from('<H', code, off)[0] + v) & 0xffff)
        elif loctype == 2:
            struct.pack_into('<H', code, off, f_)
        elif loctype == 3:
            struct.pack_into('<H', code, off, (tlin - f_ * 16) & 0xffff)
            struct.pack_into('<H', code, off + 2, f_)
    allok = True
    for (cn, coff, cend), (on, olin, oend) in pairs:
        cb = bytes(code[coff:cend])
        ob = bytes(mem[olin - lo: oend - lo])
        ok = cb == ob and not any(coff <= b < cend for b in bad_at)
        first = next((k for k in range(min(len(cb), len(ob))) if cb[k] != ob[k]), min(len(cb), len(ob)))
        res['functions'].append({'name': on, 'cname': cn, 'match': ok, 'len': len(cb), 'orig_len': len(ob),
                                 'first_diff': None if ok else first})
        allok &= ok
    if len(comp) != len(origs or []):
        res['count_mismatch'] = (len(comp), len(origs or []))
    # data segments
    dmem = ctx['exe'].section_memory(136)
    dlo = ctx['exe'].sections[136].load_seg * 16
    res['data_ok'] = True
    for dn, doff in data_place.items():
        if dn not in c['datasegs']:
            continue
        di = [i for i in range(1, len(m.segs)) if m.segs[i][0] == dn][0]
        dbuf = bytearray(segs[dn][0][:c['datasegs'][dn]])
        for si2, off2, lt2, M2, fr2, tg2, dp2 in m.fixups:
            if si2 == di and lt2 == 2 and tg2[0] == 2:
                r = resolve(m.exts[tg2[1]], ctx['syms'])
                if r:
                    struct.pack_into('<H', dbuf, off2, r[2])
        o = bytes(dmem[DG * 16 + doff - dlo: DG * 16 + doff - dlo + len(dbuf)])
        if o != bytes(dbuf):
            res['data_ok'] = False
            res.setdefault('data_diff', []).append((dn, doff, dbuf.hex(), o.hex()))
    res['data_place'] = data_place
    res['unresolved'] = sorted(unresolved)
    # whole-file match = every original function from the placement on is covered and matches
    res['file_match'] = allok and res['data_ok'] and len(comp) == len(origs or []) and not unresolved
    res['prefix_match'] = 0
    for f in res['functions']:
        if not f['match']:
            break
        res['prefix_match'] += 1
    if not quiet:
        nm = sum(f['match'] for f in res['functions'])
        print('%s: %d/%d functions match%s%s' % (os.path.relpath(path, REPO), nm, len(origs or []),
              '' if res['data_ok'] else ', DATA DIFFERS', ' (FILE MATCH)' if res['file_match'] else ''))
        if unresolved:
            print('  unresolved:', ', '.join(sorted(unresolved)))
        for dd in res.get('data_diff', []):
            print('  data %s at %04x: %s\n                 orig %s' % dd)
        for f, ((cn, coff, cend), (on, olin, oend)) in zip(res['functions'], pairs):
            if f['match'] and not verbose:
                continue
            if only and not any(o in on for o in only):
                continue
            print('  %-28s %-8s len %d/%d' % (on, 'MATCH' if f['match'] else 'DIFF@%x' % f['first_diff'],
                                              f['len'], f['orig_len']))
            if verbose and not f['match']:
                show_diff(bytes(mem[olin - lo: oend - lo]), bytes(code[coff:cend]), olin - frame * 16)
    return res


def show_diff(orig, code, base, ctxlines=4, maxlines=30):
    import capstone
    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_16)
    a = list(md.disasm(orig, base))
    b = list(md.disasm(code, base))
    rows = []
    for x in range(max(len(a), len(b))):
        xa = a[x] if x < len(a) else None
        xb = b[x] if x < len(b) else None
        fa = '%04x %-12s %s %s' % (xa.address, xa.bytes.hex(), xa.mnemonic, xa.op_str) if xa else ''
        fb = '%-12s %s %s' % (xb.bytes.hex(), xb.mnemonic, xb.op_str) if xb else ''
        mark = '  ' if (xa and xb and xa.bytes == xb.bytes) else '!!'
        rows.append('    %s %-48s | %s' % (mark, fa[:48], fb[:44]))
    k0 = next((i for i, r in enumerate(rows) if r.startswith('    !!')), 0)
    for r in rows[max(0, k0 - ctxlines): max(0, k0 - ctxlines) + maxlines]:
        print(r)


if __name__ == '__main__':
    import argparse
    ap = argparse.ArgumentParser()
    ap.add_argument('files', nargs='+')
    ap.add_argument('-v', action='store_true')
    ap.add_argument('--flags')
    ap.add_argument('--func', action='append', help='only show these functions (substring)')
    a = ap.parse_args()
    rs = [check(f, a.v, a.flags, only=a.func) for f in a.files]
    sys.exit(0 if all(r['file_match'] for r in rs) else 1)
