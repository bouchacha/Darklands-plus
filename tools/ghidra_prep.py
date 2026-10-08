#!/usr/bin/env python3
"""
Prepare per-image inputs for the Ghidra headless export (tools/ghidra/DarklandsExport.java).

For every code image (root, overlay sections) writes:
  <out>/in/<img>.bin   flat memory snapshot (linear 0 .. end of DGROUP) as seen while that image
                       is loaded: root + overlay ancestors + the image + the data section
  <out>/in/<img>.json  {"own": [[seg, off, name], ...], "labels": [[seg, off, name], ...]}
                       own = functions to create and decompile, labels = names for code in
                       other images (ancestors/root) and RTLink vector stubs
"""
import sys, os, json, pickle, bisect
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from rtlink import RTLinkExe
from emit_asm import load_names


def main():
    exe_path, pk, out = sys.argv[1:4]
    exe = RTLinkExe(exe_path)
    d = pickle.load(open(pk, 'rb'))
    names = load_names()
    seg_ranges = d['seg_ranges']
    entries = d['entries']
    insns = d['insns']
    vectors = d['vectors']
    os.makedirs(os.path.join(out, 'in'), exist_ok=True)
    total = 0x42000
    imgs = {'root': (0, exe.root_memory(), None)}
    for s in exe.sections:
        imgs['s%03d' % s.index] = (s.load_seg * 16, exe.section_memory(s.index),
                                   None if s.parent == 0xffff else 's%03d' % s.parent)

    def fname(img, lin):
        st, fr = seg_ranges[img]
        i = max(0, bisect.bisect_right(st, lin) - 1)
        f = fr[i]
        gen = 'sub_%s_%04x_%04x' % (img, f, lin - f * 16)
        return f, lin - f * 16, names.get(gen, gen)

    fdb = json.load(open(os.path.join(os.path.dirname(pk), 'functions.json')))
    conv = {}
    for r in fdb.values():
        rets = r['returns']
        if 'ret' in rets and 'retf' not in rets:
            c = '__cdecl16near'
        elif r['asm_markers'] and 'retf n' in r['asm_markers']:
            c = '__stdcall16far'
        else:
            c = '__cdecl16far'
        conv[(r['image'], int(r['linear'], 16))] = c
    funcs_by_img = {}
    for (img, lin) in entries:
        if (img, lin) in insns:
            funcs_by_img.setdefault(img, []).append(lin)
    for img in imgs:
        if img in ('s136', 's042'):
            continue
        chain = [img]
        p = imgs[img][2]
        while p:
            chain.append(p)
            p = imgs[p][2]
        if 'root' not in chain:
            chain.append('root')
        chain.append('s136')
        flat = bytearray(total)
        for c in chain:
            lo, mem, _ = imgs[c]
            flat[lo:lo + len(mem)] = mem
        open(os.path.join(out, 'in', img + '.bin'), 'wb').write(flat)
        own = [list(fname(img, l)) + [conv.get((img, l), '__cdecl16far')] for l in sorted(funcs_by_img.get(img, []))]
        # only label what this image's code actually calls (keeps each Ghidra run small)
        called = set()
        for (ci, cl), tg in d['calls'].items():
            if ci != img:
                continue
            raw = insns[(ci, cl)][3]
            if raw[:1] == b'\x9a':
                toff, tseg = int.from_bytes(raw[1:3], 'little'), int.from_bytes(raw[3:5], 'little')
                if tseg == 0x9c0 and toff in vectors:
                    called.add(('vec', toff))
                    continue
            for t in tg:
                if t[0] != img:
                    called.add(t)
        labels = []
        for t in sorted(x for x in called if x[0] != 'vec'):
            labels.append(list(fname(t[0], t[1])) + [conv.get(t, '__cdecl16far')])
        for _, o in sorted(x for x in called if x[0] == 'vec'):
            tname, tlin, tseg = vectors[o]
            f, off, nm = fname(tname, tlin)
            labels.append([0x9c0, o, 'vec_' + (nm if not nm.startswith('sub_') else nm[4:]),
                           conv.get((tname, tlin), '__cdecl16far')])
        json.dump({'own': own, 'labels': labels}, open(os.path.join(out, 'in', img + '.json'), 'w'))
    print('prepared', len([i for i in imgs if i not in ('s136', 's042')]), 'images')


if __name__ == '__main__':
    main()
