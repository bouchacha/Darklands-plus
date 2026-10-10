#!/usr/bin/env python3
"""
Record each matching C file's original extent in the file itself:

    /* orig: size=06f4 _DATA=6a76+0009 CONST=829e+0002 */

size  = bytes of original code the file replaces (from its placement address)
_DATA/CONST = DGROUP offset + size of the module's data segments

Run on files that match (the full build prints BYTE-IDENTICAL). 'build.py --mod' uses the stamp
to cut the original range out of the program even after the C has been edited and its code
no longer has the original length.

  python3 -I tools/stamp.py            stamp every src/c file that has no stamp yet
  python3 -I tools/stamp.py --force    re-stamp all files
"""
import sys, os, re, argparse
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import cport
from rtlink import RTLinkExe

REPO = os.path.dirname(HERE)
EXE = os.environ.get('DARKLAND_EXE', '/home/claude/dl_raw/extracted/DARKLAND/DARKLAND.EXE')


def stamp_line(c):
    parts = ['size=%04x' % c['length']]
    for n in sorted(c['datasegs']):
        parts.append('%s=%04x+%04x' % (n, c['data'][n], c['datasegs'][n]))
    return '/* orig: %s */' % ' '.join(parts)


def all_slots():
    """path -> (image, frame, offset, {seg: (dgroup offset, size)}) for every stamped file."""
    out = {}
    for sub in (os.path.join('src', 'c'), os.path.join('src', 'wip')):
        for x in cport.sources(REPO, sub=sub):
            if x['orig_slots']:
                out[x['path']] = (x['image'], x['frame'], x['offset'], x['orig_slots'])
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--force', action='store_true')
    ap.add_argument('--wip', action='store_true',
                    help='stamp complete but not (yet) matching files (e.g. src/wip/**): the original '
                         'range comes from the original source file, data placement from fmatch')
    ap.add_argument('files', nargs='*')
    a = ap.parse_args()
    exe = RTLinkExe(EXE)
    n = 0
    if a.wip:
        import fmatch, io, contextlib
        for f in a.files:
            with contextlib.redirect_stdout(io.StringIO()):
                r = fmatch.check(f, quiet=True)
            if r.get('error') or r.get('count_mismatch') or not r.get('data_ok'):
                print('skip %s: %s' % (f, r.get('error') and 'compile error' or
                                       r.get('count_mismatch') and 'function count %s' % (r['count_mismatch'],)
                                       or 'data differs'))
                continue
            fns = r['functions']
            if re.search(r'/\*\s*end:', open(f, encoding='latin-1').read(3000)):
                fns = fns[:-1]         # the last original 'function' runs into data
            stubs = [fn['name'] for fn in fns
                     if fn.get('orig_len') and fn['len'] * 4 < fn['orig_len'] and fn['orig_len'] > 24]
            if stubs:
                print('skip %s: unwritten (stub) functions %s' % (f, ', '.join(stubs)))
                t = open(f, encoding='latin-1').read()
                if cport.ORIG_LINE.search(t):          # an old stamp would let --functional use it
                    open(f, 'w', encoding='latin-1').write(cport.ORIG_LINE.sub('', t))
                continue
            text = open(f, encoding='latin-1').read()
            m = cport.PLACE.search(text)
            img, fr, off = m.group(1), int(m.group(2), 16), int(m.group(3), 16)
            ctx = fmatch.load_ctx()
            origs = fmatch.orig_functions(ctx, img, fr, off)
            start = fr * 16 + off
            size = origs[-1][2] - start
            me = re.search(r'/\*\s*end:\s*([0-9a-fA-F]{5})\s*\*/', text[:3000])
            if me:                     # the source file ends before the next one starts (data follows)
                size = int(me.group(1), 16) - start
            fm = cport.FLAGS.search(text[:3000])
            c = cport.compile_source({'path': f, 'image': img, 'frame': fr, 'offset': off,
                                      'flags': fm.group(1).split() if fm else list(cport.msc.DEFAULT)},
                                     os.path.join(REPO, 'build', 'cobj'))
            c['length'] = size
            c['data'] = {k: v for k, v in r['data_place'].items() if k in c['datasegs']}
            # refuse a placement that overlaps another module's data (fmatch may place CONST by
            # searching DGROUP for equal words, and segment words repeat)
            clash = []
            for other, (o_img, o_fr, o_off, slots) in all_slots().items():
                if os.path.abspath(other) == os.path.abspath(f) or \
                        (o_img, o_fr, o_off) == (img, fr, off):
                    continue
                for dn, d0 in c['data'].items():
                    d1 = d0 + c['datasegs'][dn]
                    for on, (s0, sl) in slots.items():
                        if d0 < s0 + sl and s0 < d1:
                            clash.append('%s %04x-%04x overlaps %s %s' % (dn, d0, d1, os.path.relpath(other, REPO), on))
            if clash:
                print('skip %s: data placement clashes (add a /* data: NAME=xxxx */ hint): %s' % (f, '; '.join(clash[:3])))
                continue
            line = stamp_line(c)
            text = cport.ORIG_LINE.sub('', text)
            m = cport.PLACE.search(text)
            eol = text.index('\n', m.end()) + 1
            open(f, 'w', encoding='latin-1').write(text[:eol] + line + '\n' + text[eol:])
            n += 1
            print('stamped %s: %s' % (f, line))
        return
    for src in cport.sources(REPO):
        if a.files and not any(os.path.samefile(src['path'], f) for f in a.files):
            continue
        text = open(src['path'], encoding='latin-1').read()
        if src['orig_size'] is not None and not a.force:
            continue
        c = cport.compile_source(src, os.path.join(REPO, 'build', 'cobj'))
        c['data'] = {}
        img = c['image']
        mem = exe.root_memory() if img == 'root' else exe.section_memory(int(img[1:]))
        lo = 0 if img == 'root' else exe.sections[int(img[1:])].load_seg * 16
        st = c['frame'] * 16 + c['offset']
        head = text[:3000]
        c['data'] = dict((k, int(v, 16)) for k, v in re.findall(r'/\*\s*data:\s*(\w+)=([0-9a-fA-F]{1,4})\s*\*/', head))
        cport.infer_data_placement(c, bytes(mem[st - lo: st - lo + c['length']]))
        line = stamp_line(c)
        text = cport.ORIG_LINE.sub('', text)
        m = cport.PLACE.search(text)
        eol = text.index('\n', m.end()) + 1
        text = text[:eol] + line + '\n' + text[eol:]
        open(src['path'], 'w', encoding='latin-1').write(text)
        n += 1
    print('stamped %d files' % n)


if __name__ == '__main__':
    main()
