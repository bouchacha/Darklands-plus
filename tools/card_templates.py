#!/usr/bin/env python3
"""
Group the event/card-script functions (overlays s044-s135) by code template.

Two functions share a template when their instruction sequences are identical after
replacing immediates and memory displacements with parameters; far-call targets are kept
(resolved to function names), so a template is "the same C code with different constants".
For every template the varying constants of each instance are extracted.

Output: build/analysis/card_templates.json and a coverage summary on stdout.
This is the starting point both for generating matching C mechanically and for turning
the card scripts into an editable data format (see docs/ROADMAP.md).
"""
import sys, os, re, json, pickle, collections, bisect
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from emit_asm import load_names

NUM = re.compile(r'0x[0-9a-f]+|(?<![\w])\d+(?![\w])')


def main():
    an = sys.argv[1]
    d = pickle.load(open(os.path.join(an, 'analysis.pickle'), 'rb'))
    names = load_names()
    ins, owner, calls, seg_ranges, vectors = d['insns'], d['owner'], d['calls'], d['seg_ranges'], d['vectors']

    def fname(img, lin):
        st, fr = seg_ranges[img]
        i = max(0, bisect.bisect_right(st, lin) - 1)
        g = 'sub_%s_%04x_%04x' % (img, fr[i], lin - fr[i] * 16)
        return names.get(g, g)

    funcs = collections.defaultdict(list)
    for (img, l), e in owner.items():
        if img.startswith('s') and 44 <= int(img[1:]) <= 135:
            funcs[(img, e)].append(l)
    groups = collections.defaultdict(list)
    for key, ls in funcs.items():
        ls.sort()
        shape, params = [], []
        for l in ls:
            r = ins[(key[0], l)]
            mn, ops = r[1], r[2]
            tgt = None
            if mn in ('lcall', 'call') and (key[0], l) in calls:
                t = sorted(calls[(key[0], l)])[0]
                # calls into the script's own overlay are local: keep only "local call"
                tgt = 'local' if t[0] == key[0] else fname(*t)
            if tgt is not None:
                shape.append('%s %s' % (mn, tgt))
                if tgt == 'local':
                    params.append(fname(key[0], sorted(calls[(key[0], l)])[0][1]))
                continue
            if mn.startswith('j') or mn in ('loop', 'jcxz'):
                shape.append(mn)          # branch targets follow from the shape
                continue
            shape.append(mn + ' ' + NUM.sub('#', ops))
            params.extend(NUM.findall(ops))
        groups[tuple(shape)].append((key, params))
    ranked = sorted(groups.items(), key=lambda kv: -len(kv[1]))
    total = sum(len(v) for v in groups.values())
    out = []
    cum = 0
    for rank, (shape, inst) in enumerate(ranked):
        cum += len(inst)
        # which parameter positions actually vary across instances
        cols = list(zip(*[p for _, p in inst])) if inst and all(len(p) == len(inst[0][1]) for _, p in inst) else []
        varying = [i for i, c in enumerate(cols) if len(set(c)) > 1]
        out.append({
            'rank': rank + 1, 'instances': len(inst), 'cumulative_share': round(cum / total, 4),
            'instructions': len(shape), 'template': list(shape), 'varying_param_positions': varying,
            'instances_detail': [{'function': fname(k[0], k[1]), 'overlay': k[0],
                                  'params': [p[i] for i in varying]} for k, p in inst],
        })
    json.dump({'functions': total, 'templates': len(groups), 'groups': out},
              open(os.path.join(an, 'card_templates.json'), 'w'), indent=0)
    print('card-script functions: %d, templates: %d' % (total, len(groups)))
    for g in out[:10]:
        print('#%-2d %5d instances (cum %4.1f%%) %3d insns, %d varying constants: %s' % (
            g['rank'], g['instances'], 100 * g['cumulative_share'], g['instructions'],
            len(g['varying_param_positions']), ' | '.join(g['template'][:6])[:110]))
    for n in (10, 50, 100, 200):
        if n <= len(out):
            print('top %3d templates cover %4.1f%% of card-script functions' % (n, 100 * out[n - 1]['cumulative_share']))


if __name__ == '__main__':
    main()
