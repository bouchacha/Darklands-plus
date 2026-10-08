#!/usr/bin/env python3
"""
Quick matching check for one C file in src/c (no full rebuild).

  python3 -I tools/cmatch.py src/c/root/random.c [--flags "/AM /G2 /Gs /Ox"] [-v]

Compiles the file with MSC 6.00A, places its code at the file's 'place:' address and compares
it with the original bytes. Fixups are resolved with build/symbols.json (written by
tools/build.py), so calls and data references are checked too, not just opcodes.
Prints a side-by-side disassembly from the first difference. Exit status 0 = exact match.
"""
import sys, os, json, struct, argparse, re
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import cport, msc
from rtlink import RTLinkExe

REPO = os.path.dirname(HERE)
EXE = os.environ.get('DARKLAND_EXE', '/home/claude/dl_raw/extracted/DARKLAND/DARKLAND.EXE')


def resolve(name, syms):
    if name in syms:
        return syms[name]
    m = re.match(r'^_D_([0-9a-fA-F]{1,4})$', name)
    if m:
        return ['s136', 0x321a0 + int(m.group(1), 16), 0x321a]
    m = re.match(r'^_F_([0-9a-fA-F]{4})_([0-9a-fA-F]{1,4})$', name)
    if m:
        f = int(m.group(1), 16)
        return ['s136', f * 16 + int(m.group(2), 16), f]
    if name.startswith('_') and name[1:] in syms:
        return syms[name[1:]]
    return None


def check(path, flags=None, verbose=False, exe=None):
    src = [s for s in cport.sources(REPO) if os.path.samefile(s['path'], path)]
    if not src:
        # allow files outside src/c with a place: line
        head = open(path, encoding='latin-1').read(2000)
        m = cport.PLACE.search(head)
        src = [{'path': path, 'image': m.group(1), 'frame': int(m.group(2), 16), 'offset': int(m.group(3), 16),
                'flags': list(msc.DEFAULT)}]
    s = dict(src[0])
    if flags:
        s['flags'] = flags.split()
    c = cport.compile_source(s, os.path.join(REPO, 'build', 'cobj'))
    syms = json.load(open(os.path.join(REPO, 'build', 'symbols.json')))
    exe = exe or RTLinkExe(EXE)
    img = s['image']
    mem = exe.root_memory() if img == 'root' else exe.section_memory(int(img[1:]))
    lo = 0 if img == 'root' else exe.sections[int(img[1:])].load_seg * 16
    start = s['frame'] * 16 + s['offset']
    segs, m = msc.obj_code(open(c['obj'], 'rb').read())
    code = bytearray(segs[c['segment']][0])
    seg_index = [i for i in range(1, len(m.segs)) if m.segs[i][0] == c['segment']][0]
    # apply fixups
    unresolved = []
    for si, off, loctype, M, frame, target, disp in m.fixups:
        if si != seg_index:
            continue
        tm, ti = target
        if tm == 0:
            if m.segs[ti][0] != c['segment']:
                unresolved.append('segment target %s' % m.segs[ti][0])
                continue
            tlin, tfr = start + disp, s['frame']
        elif tm == 2:
            r = resolve(m.exts[ti], syms)
            if r is None:
                unresolved.append(m.exts[ti])
                continue
            tlin, tfr = r[1] + disp, r[2]
        else:
            tlin, tfr = 0x321a0 + disp, 0x321a        # group DGROUP
        fm, fi = frame
        fr = tfr
        if fm == 1:
            fr = 0x321a
        elif fm == 4:
            fr = s['frame']
        loc = start + off
        if loctype in (1, 5):
            v = (tlin - fr * 16) if M else (tlin - (loc + 2))
            struct.pack_into('<H', code, off, (struct.unpack_from('<H', code, off)[0] + v) & 0xffff)
        elif loctype == 2:
            struct.pack_into('<H', code, off, fr)
        elif loctype == 3:
            struct.pack_into('<H', code, off, (tlin - fr * 16) & 0xffff)
            struct.pack_into('<H', code, off + 2, fr)
    orig = bytes(mem[start - lo: start - lo + len(code)])
    ok = orig == bytes(code) and not unresolved
    # which original functions does the range cover?
    print('%s: %d bytes at %s %04x:%04x -> %s' % (os.path.relpath(path, REPO), len(code), img, s['frame'],
                                                  s['offset'], 'MATCH' if ok else 'DIFFERENT'))
    if unresolved:
        print('  unresolved externals:', ', '.join(sorted(set(unresolved))))
    if not ok or verbose:
        import capstone
        md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_16)
        a = list(md.disasm(orig, s['offset']))
        b = list(md.disasm(bytes(code), s['offset']))
        first = next((k for k in range(min(len(orig), len(code))) if orig[k] != code[k]), min(len(orig), len(code)))
        print('  first difference at +%04x' % first)
        ia = ib = 0
        shown = 0
        rows = []
        for x in range(max(len(a), len(b))):
            xa = a[x] if x < len(a) else None
            xb = b[x] if x < len(b) else None
            fa = '%04x %-14s %s %s' % (xa.address, xa.bytes.hex(), xa.mnemonic, xa.op_str) if xa else ''
            fb = '%-14s %s %s' % (xb.bytes.hex(), xb.mnemonic, xb.op_str) if xb else ''
            mark = '  ' if (xa and xb and xa.bytes == xb.bytes) else '!!'
            rows.append('  %s %-50s | %s' % (mark, fa[:50], fb[:46]))
        # show context from a few lines before the first difference
        k0 = next((i for i, r in enumerate(rows) if r.startswith('  !!')), 0)
        lo_ = 0 if verbose else max(0, k0 - 4)
        for r in rows[lo_: lo_ + (len(rows) if verbose else 30)]:
            print(r)
    return ok


if __name__ == '__main__':
    ap = argparse.ArgumentParser()
    ap.add_argument('files', nargs='+')
    ap.add_argument('--flags')
    ap.add_argument('-v', action='store_true')
    a = ap.parse_args()
    exe = RTLinkExe(EXE)
    res = [check(f, a.flags, a.v, exe) for f in a.files]
    sys.exit(0 if all(res) else 1)
