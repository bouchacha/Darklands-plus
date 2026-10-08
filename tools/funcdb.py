#!/usr/bin/env python3
"""
Build the function database from the analysis pickle.

Output: build/analysis/functions.json  (one record per function)
        build/analysis/functions.csv   (flat summary)
        build/analysis/strings.json    (DGROUP / far-data strings with referencing functions)

Per function: image, frame:offset, linear, size, instruction count, how it is reached
(near/far/vector/pointer), callers/callees, prologue/epilogue shape, a C-vs-assembly
classification with the evidence used, and the strings it references.
"""
import sys, os, json, pickle, collections, struct, re, csv, bisect
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from rtlink import RTLinkExe

DGROUP = 0x321a
FAR_STRING_SEGS = (0x286b, 0x290e, 0x2caf)
ASM_ONLY = {'in', 'out', 'cli', 'sti', 'lahf', 'sahf', 'daa', 'das', 'xlatb', 'stc', 'clc', 'cmc',
            'pushf', 'popf', 'rcl', 'rcr', 'rol', 'ror', 'loop', 'jcxz', 'lodsb', 'lodsw', 'std',
            'int3', 'iret', 'stosb', 'stosw', 'movsb', 'movsw', 'cld', 'hlt', 'int'}
# note: MSC 6 emits 'rep movsw/movsb/stosw' for struct copies and memset-like code, and 'int'
# only via inline/intrinsics; the plain (non-rep) string ops and the flag/port/rotate ops above
# are strong hand-written-assembly markers.


def load():
    d = pickle.load(open(sys.argv[2], 'rb'))
    exe = RTLinkExe(sys.argv[1])
    return d, exe


def main():
    d, exe = load()
    out_dir = os.path.dirname(sys.argv[2])
    insns = d['insns']
    owner = d['owner']
    entries = d['entries']
    calls = d['calls']
    seg_ranges = d['seg_ranges']
    jt = d['jump_tables']

    # images' memory for string lookups
    s136 = exe.section_memory(136)
    s136_lo = exe.sections[136].load_seg * 16
    s136_file = len(exe.sections[136].image)

    def cstring_at(seg, off, minlen=3):
        lin = seg * 16 + off - s136_lo
        if lin <= 0 or lin >= s136_file:
            return None
        if s136[lin - 1] != 0:
            return None
        end = s136.find(b'\0', lin, lin + 400)
        if end < 0 or end - lin < minlen:
            return None
        s = bytes(s136[lin:end])
        printable = sum(32 <= c < 127 or c in (9, 10, 13) for c in s)
        if printable < len(s) * 0.9:
            return None
        return s.decode('latin-1')

    funcs = {}
    by_func = collections.defaultdict(list)
    for (img, lin), e in owner.items():
        by_func[(img, e)].append(lin)
    names = {}

    def fname(img, lin):
        st, fr = seg_ranges[img]
        i = max(0, bisect.bisect_right(st, lin) - 1)
        f = fr[i]
        return 'sub_%s_%04x_%04x' % (img, f, lin - f * 16), f

    callers = collections.defaultdict(list)
    for (img, lin), tg in calls.items():
        r = insns[(img, lin)]
        kind = 'near' if r[3][0] == 0xe8 else 'far'
        src = owner.get((img, lin))
        for t in tg:
            callers[t].append({'from': fname(img, src)[0] if src is not None else None, 'kind': kind})

    string_refs = collections.defaultdict(set)
    for (img, e), lins in by_func.items():
        lins.sort()
        name, frame = fname(img, e)
        n = len(lins)
        end = max(l + insns[(img, l)][0] for l in lins)
        mn = collections.Counter(insns[(img, l)][1] for l in lins)
        first = [insns[(img, l)] for l in lins[:4]]
        rets = collections.Counter(insns[(img, l)][1] for l in lins if insns[(img, l)][1] in ('ret', 'retf', 'iret'))
        # prologue shape
        p0 = first[0][3]
        if p0[:1] == b'\xc8':
            prologue = 'enter'
        elif p0[:1] == b'\x55' and len(first) > 1 and first[1][3][:2] == b'\x8b\xec':
            prologue = 'push bp/mov bp,sp'
        else:
            prologue = 'none'
        asm_marks = sorted(k for k in mn if k in ASM_ONLY)
        uses_leave = mn.get('leave', 0) > 0
        # MSC 6 /G2 idioms
        msc_marks = []
        if prologue == 'enter':
            msc_marks.append('enter')
        if uses_leave:
            msc_marks.append('leave')
        pc = sum(1 for l in lins if insns[(img, l)][3][:1] == b'\x0e' and insns.get((img, l + 1), (0, ''))[1] == 'call')
        if pc:
            msc_marks.append('push cs;call near')
        if any(insns[(img, l)][1] == 'mov' and insns[(img, l)][2] == 'sp, bp' for l in lins):
            msc_marks.append('mov sp,bp')
        # classification -------------------------------------------------------------
        # MSC 6 (/AL /G2 /Ox) never emits port I/O, flag games, lods/plain stos, loop/jcxz,
        # xlat or near 'ret' in large model; it does emit rep movs/stos, plain movsw/movsb
        # (struct copies), rcl/rcr (long shifts) and frameless functions (no args/locals).
        strong = set()
        for l in lins:
            r = insns[(img, l)]
            m = r[1]
            if m in ('in', 'out', 'cli', 'sti', 'lahf', 'sahf', 'iret', 'pushf', 'popf', 'stc', 'clc',
                     'cmc', 'daa', 'das', 'xlatb', 'loop', 'jcxz', 'lodsb', 'lodsw', 'int3', 'std', 'cld'):
                strong.add(m)
            elif m in ('stosb', 'stosw'):
                strong.add(m)
            elif m == 'ret':
                strong.add('near ret')
            elif m == 'retf' and r[2]:
                strong.add('retf n')
        has_int = 'int' in mn
        if img == 'root' and frame in (0x6a1,):
            kind = 'lib'            # Microsoft C 6 run-time library (crt0 + LLIBCE)
        elif img == 'root' and frame in (0x9c0, 0xd14):
            kind = 'rtlink'         # RTLink/Plus overlay manager + cache
        elif strong:
            kind = 'asm'
        elif has_int:
            kind = 'C+asm?'         # C with inline _asm / intrinsic, or small asm helper
        else:
            kind = 'C'
        asm_marks = sorted(strong | ({'int'} if has_int else set()))
        # strings
        strs = []
        for l in lins:
            r = insns[(img, l)]
            raw = r[3]
            if raw[:1] == b'\x68' and len(raw) == 3 or (0xb8 <= raw[0] <= 0xbf and len(raw) == 3):
                v = struct.unpack_from('<H', raw, 1)[0]
                s = cstring_at(DGROUP, v)
                if s:
                    strs.append(s)
                    string_refs[('DGROUP', v)].add(name)
        rec = {
            'name': name, 'image': img, 'frame': '%04x' % frame, 'offset': '%04x' % (e - frame * 16),
            'linear': '%05x' % e, 'size': end - e, 'insns': n,
            'reached_by': sorted(entries.get((img, e), [])),
            'callers': len(callers.get((img, e), [])),
            'far_callers': sum(1 for c in callers.get((img, e), []) if c['kind'] == 'far'),
            'returns': dict(rets), 'prologue': prologue, 'kind': kind,
            'asm_markers': asm_marks, 'msc_markers': msc_marks,
            'strings': strs[:40],
        }
        callees = set()
        for l in lins:
            for t in calls.get((img, l), []):
                callees.add(fname(t[0], t[1])[0])
        rec['callees'] = sorted(callees)
        funcs[name] = rec

    json.dump(funcs, open(os.path.join(out_dir, 'functions.json'), 'w'), indent=0)
    with open(os.path.join(out_dir, 'functions.csv'), 'w', newline='') as f:
        w = csv.writer(f)
        w.writerow(['name', 'image', 'frame', 'offset', 'linear', 'size', 'insns', 'kind', 'prologue',
                    'callers', 'far_callers', 'n_callees', 'first_string'])
        for r in sorted(funcs.values(), key=lambda r: (r['image'], r['linear'])):
            w.writerow([r['name'], r['image'], r['frame'], r['offset'], r['linear'], r['size'], r['insns'],
                        r['kind'], r['prologue'], r['callers'], r['far_callers'], len(r['callees']),
                        (r['strings'][0] if r['strings'] else '')[:60]])
    kinds = collections.Counter(r['kind'] for r in funcs.values())
    kb = collections.Counter()
    for r in funcs.values():
        kb[r['kind']] += r['size']
    print('functions', len(funcs), dict(kinds), 'bytes by kind', dict(kb))
    by_img = collections.defaultdict(collections.Counter)
    for r in funcs.values():
        by_img[r['image']][r['kind']] += 1
    json.dump({k: dict(v) for k, v in by_img.items()}, open(os.path.join(out_dir, 'kinds_by_image.json'), 'w'), indent=0)


if __name__ == '__main__':
    main()
