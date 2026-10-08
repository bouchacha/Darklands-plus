#!/usr/bin/env python3
"""
Batch decompile source files with tools/dec16.py and check each with tools/fmatch.py.

  python3 -I tools/cardgen.py [--images s044-s135] [--jobs 8] [--only s070_19af]

Writes build/gen/<image>/<image>_<frame>.c and build/gen/report.json; prints a summary:
functions matched, files matched completely, most common decompiler failures and mismatches.
"""
import sys, os, json, re, collections, argparse, traceback
from concurrent.futures import ProcessPoolExecutor
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
REPO = os.path.dirname(HERE)
GEN = os.path.join(REPO, 'build', 'gen')

_prog = None


def work(job):
    global _prog
    import dec16, fmatch
    img, frame = job
    if _prog is None:
        _prog = dec16.Prog()
    out = {'image': img, 'frame': '%04x' % frame}
    path = os.path.join(GEN, img, '%s_%04x.c' % (img, frame))
    os.makedirs(os.path.dirname(path), exist_ok=True)
    try:
        fd, bodies = dec16.decompile_file(_prog, img, frame)
        text = dec16.render_file(_prog, img, frame, fd, bodies)
    except Exception as e:
        out['error'] = 'render: %s' % e
        out['trace'] = traceback.format_exc()[-800:]
        return out
    open(path, 'w').write(text)
    out['failed'] = fd.failed
    out['nfuncs'] = len(fd.funcs)
    try:
        r = fmatch.check(path, quiet=True)
    except Exception as e:
        out['error'] = 'fmatch: %s' % e
        out['trace'] = traceback.format_exc()[-800:]
        return out
    out['compile_error'] = r.get('error')
    out['functions'] = [(f['name'], f['match'], f['first_diff'], f['len'], f['orig_len']) for f in r['functions']]
    out['file_match'] = r['file_match']
    out['data_ok'] = r.get('data_ok')
    out['unresolved'] = r.get('unresolved')
    return out


def norm_reason(msg):
    msg = re.sub(r'0x[0-9a-f]+|\b[0-9a-f]{3,5}\b', 'N', msg)
    msg = re.sub(r'\d+', 'N', msg)
    return msg[:90]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--images', default='s044-s135')
    ap.add_argument('--jobs', type=int, default=8)
    ap.add_argument('--only')
    a = ap.parse_args()
    files = json.load(open(os.path.join(REPO, 'build', 'analysis', 'srcfiles.json')))
    lo, hi = a.images.split('-') if '-' in a.images else (a.images, a.images)

    def inrange(img):
        if img == 'root' or not img.startswith('s'):
            return lo == 'root'
        return int(lo[1:]) <= int(img[1:]) <= int(hi[1:])
    jobs = [(f['image'], int(f['frame'], 16)) for f in files if inrange(f['image'])]
    if a.only:
        jobs = [j for j in jobs if '%s_%04x' % j in a.only.split(',')]
    # warm the scan cache once
    import dec16
    dec16.Prog()._scan_calls()
    with ProcessPoolExecutor(a.jobs) as ex:
        res = list(ex.map(work, jobs, chunksize=2))
    json.dump(res, open(os.path.join(GEN, 'report.json'), 'w'), indent=0)
    nf = sum(r.get('nfuncs', 0) for r in res)
    nm = sum(sum(1 for f in r.get('functions', []) if f[1]) for r in res)
    nfile = sum(1 for r in res if r.get('file_match'))
    print('files %d, fully matching %d; functions %d, matching %d (%.1f%%)' % (len(res), nfile, nf, nm, 100.0 * nm / max(nf, 1)))
    fails = collections.Counter()
    for r in res:
        for fn, msg in (r.get('failed') or {}).items():
            fails[norm_reason(msg)] += 1
        if r.get('error'):
            fails['FILE ' + norm_reason(r['error'])] += 1
        if r.get('compile_error'):
            fails['COMPILE ' + norm_reason(r['compile_error'].split('\n')[0])] += 1
    print('decompiler failures:')
    for m, c in fails.most_common(25):
        print('  %5d  %s' % (c, m))
    diff = sum(1 for r in res for f in r.get('functions', []) if not f[1] and f[0] not in (r.get('failed') or {}))
    print('compiled but different: %d functions' % diff)


if __name__ == '__main__':
    main()
