#!/usr/bin/env python3
"""
Infer the original source-file boundaries inside every code segment.

MSC 6 calls a far function defined in the same source file with 'push cs; call near' and
one defined elsewhere with 'call far'. Several source files can share one segment (same
/NT name), so inside a segment:
  - a near call between two functions  => same source file (no boundary between them)
  - a far call between two functions   => different files (some boundary between them)
Source files are contiguous, so the result is a partition of each segment's functions into
runs. Where the evidence doesn't pin a boundary down, the run is left merged (conservative:
a merged run can always be ported as one C file).

Output: build/analysis/srcfiles.json  [{image, frame, start, end, functions:[names]}]
"""
import sys, os, json, pickle, bisect, collections
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))


def main():
    an = sys.argv[1]
    d = pickle.load(open(os.path.join(an, 'analysis.pickle'), 'rb'))
    funcs = json.load(open(os.path.join(an, 'functions.json')))
    ins, calls, owner, seg_ranges = d['insns'], d['calls'], d['owner'], d['seg_ranges']
    by_region = collections.defaultdict(list)
    for r in funcs.values():
        img = r['image']
        lin = int(r['linear'], 16)
        st, fr = seg_ranges[img]
        i = max(0, bisect.bisect_right(st, lin) - 1)
        by_region[(img, fr[i], st[i])].append(lin)
    out = []
    stats = collections.Counter()
    for (img, frame, rstart), lins in sorted(by_region.items()):
        lins.sort()
        idx = {l: k for k, l in enumerate(lins)}
        n = len(lins)
        must_join = [False] * n      # must_join[k]: no boundary between function k-1 and k
        need_split = []              # (a, b): some boundary in (a, b]
        for (ci, cl), tg in calls.items():
            if ci != img:
                continue
            caller = owner.get((ci, cl))
            if caller not in idx:
                continue
            raw = ins[(ci, cl)][3]
            for t in tg:
                if t[0] != img or t[1] not in idx:
                    continue
                a, b = sorted((idx[caller], idx[t[1]]))
                if a == b:
                    continue
                if raw[:1] == b'\xe8':
                    for k in range(a + 1, b + 1):
                        must_join[k] = True
                elif raw[:1] == b'\x9a':
                    need_split.append((a, b))
        # boundaries: place one at the first allowed position for every far-call pair
        bounds = set()
        for a, b in sorted(need_split, key=lambda x: x[1]):
            if any(k in bounds for k in range(a + 1, b + 1)):
                continue
            cand = [k for k in range(a + 1, b + 1) if not must_join[k]]
            if cand:
                bounds.add(cand[-1])
            else:
                stats['conflict'] += 1
        cuts = [0] + sorted(bounds) + [n]
        for x, y in zip(cuts, cuts[1:]):
            fl = lins[x:y]
            names = [f for f in funcs if funcs[f]['image'] == img and int(funcs[f]['linear'], 16) in set(fl)]
            names.sort(key=lambda f: int(funcs[f]['linear'], 16))
            size = sum(funcs[f]['size'] for f in names)
            kinds = collections.Counter(funcs[f]['kind'] for f in names)
            out.append({'image': img, 'frame': '%04x' % frame, 'start': '%05x' % fl[0],
                        'functions': names, 'size': size, 'kinds': dict(kinds)})
            stats['files'] += 1
    json.dump(out, open(os.path.join(an, 'srcfiles.json'), 'w'), indent=0)
    sizes = [f['size'] for f in out]
    print('inferred source files:', len(out), 'conflicts:', stats['conflict'])
    print('median size %d bytes, largest %d' % (sorted(sizes)[len(sizes) // 2], max(sizes)))


if __name__ == '__main__':
    main()
