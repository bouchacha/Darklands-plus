#!/usr/bin/env python3
"""
build.py - regenerate the reassemblable source and rebuild DARKLAND.EXE from it.

  python3 -I tools/build.py <original DARKLAND.EXE> [--work build]

Steps
  1. analysis      tools/analyze.py           -> build/analysis/analysis.pickle
  2. layout        (here)                     -> build/layout.json  (linker script)
  3. emit          tools/emit_asm.py          -> build/asm/<image>/<region>.asm
  4. encode check  every distinct instruction text is assembled once; any whose JWasm
                   encoding differs from the original bytes is emitted as 'db' instead
                   (recorded in build/force_db.json)
  5. assemble      JWasm -> build/obj/*.obj
  6. link          tools/omflink.py           -> build/DARKLAND.EXE, compared with the original
"""
import sys, os, json, subprocess, argparse, collections, struct, re, hashlib
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from rtlink import RTLinkExe
import emit_asm, omflink, cport
REPO = os.path.dirname(HERE)

JWASM = os.environ.get('JWASM', '/home/claude/tools_ext/jwasm-bin')


def make_layout(exe, regions):
    L = {'images': {}, 'file_order': exe.file_order, 'reloc_order': {}, 'modules': {}}
    L['images']['root'] = {'base_seg': 0, 'mem_size': len(exe.root), 'file_size': len(exe.root)}
    for s in exe.sections:
        L['images']['s%03d' % s.index] = {
            'base_seg': s.load_seg, 'mem_size': s.mem_paras * 16, 'file_size': len(s.image),
            'section': s.index, 'parent_index': s.parent, 'flags': s.flags, 'w1': s.w1,
            'sect_id': s.sect_id}
    mz = exe.mz
    L['mz'] = {k: mz[k] for k in ('min_alloc', 'max_alloc', 'ss', 'sp', 'ip', 'cs')}
    L['manager'] = {'seg': exe.mgr_seg, 'table': exe.table_ptr}
    L['groups'] = {'DGROUP': 0x321a}
    # original relocation order and (frame) representation - cosmetic, for byte identity
    L['reloc_order']['root'] = [[seg * 16 + off, seg] for off, seg in exe.mz_relocs]
    for s in exe.sections:
        L['reloc_order']['s%03d' % s.index] = [[seg * 16 + off, seg] for off, seg in s.relocs]
    for r in regions:
        L['modules'][r['segment']] = {'image': r['image'], 'frame': r['frame']}
    return L


def run_jwasm(src, obj, lst=None, fmt='-omf'):
    cmd = [JWASM, '-q', '-Cp', fmt, '-Fo=' + obj]
    if lst:
        cmd.append('-Fl=' + lst)
    cmd.append(src)
    r = subprocess.run(cmd, capture_output=True, text=True)
    return r.returncode, r.stdout + r.stderr


def encoding_check(emitted, work):
    """Assemble each distinct label-free instruction text once; return set of (img, lin) to force to db."""
    texts = collections.defaultdict(list)
    for img, lin, txt, raw, symbolic in emitted:
        if symbolic:
            continue
        texts[txt].append((img, lin, raw))
    order = sorted(texts)
    src = os.path.join(work, 'enccheck.asm')  # JWasm writes <name>.err to cwd on errors
    with open(src, 'w') as f:
        f.write('.286\nOPTION NOLJMP\nT SEGMENT BYTE PUBLIC \'CODE\'\nASSUME CS:T,DS:NOTHING,ES:NOTHING,SS:NOTHING\n')
        for i, t in enumerate(order):
            f.write('L%d: %s\n' % (i, t))
        f.write('LEND:\nT ENDS\nEND\n')
    binf = os.path.join(work, 'enccheck.bin')
    lst = os.path.join(work, 'enccheck.lst')
    rc, out = run_jwasm(src, binf, lst, '-bin')
    bad_lines = set()
    for m in re.finditer(r'\((\d+)\) : Error', out):
        bad_lines.add(int(m.group(1)))
    force = set()
    if bad_lines:
        # drop failing lines and retry so offsets can be read
        good = []
        for i, t in enumerate(order):
            if (i + 5) in bad_lines:
                for img, lin, raw in texts[t]:
                    force.add((img, lin))
            else:
                good.append(t)
        sub, f2 = encoding_check_list(good, texts, work)
        return force | f2
    sub, f2 = encoding_check_list(order, texts, work)
    return force | f2


def encoding_check_list(order, texts, work):
    src = os.path.join(work, 'enccheck2.asm')
    with open(src, 'w') as f:
        f.write('.286\nOPTION NOLJMP\nT SEGMENT BYTE PUBLIC \'CODE\'\nASSUME CS:T,DS:NOTHING,ES:NOTHING,SS:NOTHING\n')
        f.write('PUBLIC L0\n')
        for i, t in enumerate(order):
            f.write('L%d: %s\n' % (i, t))
        f.write('LEND LABEL BYTE\n db 0\nT ENDS\nEND\n')
    binf = os.path.join(work, 'enccheck2.bin')
    lst = os.path.join(work, 'enccheck2.lst')
    rc, out = run_jwasm(src, binf, lst, '-bin')
    if rc != 0:
        raise SystemExit('encoding check failed to assemble:\n' + out[:2000])
    data = open(binf, 'rb').read()
    # offsets from listing: lines "OOOO ...  L<n>: ..."
    offs = {}
    for line in open(lst, encoding='latin-1'):
        m = re.match(r'^([0-9A-F]{4,8})\s.*?\bL(\d+):', line)
        if m:
            offs[int(m.group(2))] = int(m.group(1), 16)
    force = set()
    nbad = 0
    for i, t in enumerate(order):
        a = offs[i]
        b = offs.get(i + 1, len(data) - 1)
        got = data[a:b]
        for img, lin, raw in texts[t]:
            if got != raw:
                force.add((img, lin))
                nbad += 1
    return nbad, force


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('exe')
    ap.add_argument('--work', default='build')
    ap.add_argument('--skip-analysis', action='store_true')
    a = ap.parse_args()
    work = a.work
    os.makedirs(work, exist_ok=True)
    pk = os.path.join(work, 'analysis', 'analysis.pickle')
    if not a.skip_analysis or not os.path.exists(pk):
        subprocess.run([sys.executable, '-I', os.path.join(HERE, 'analyze.py'), a.exe,
                        os.path.join(work, 'analysis')], check=True)
    exe = RTLinkExe(a.exe)
    fpath = os.path.join(work, 'force_db.json')
    force = set(tuple(x) for x in json.load(open(fpath))) if os.path.exists(fpath) else set()
    asm_dir = os.path.join(work, 'asm')
    obj_dir = os.path.join(work, 'obj')
    os.makedirs(obj_dir, exist_ok=True)
    # ---- matching C: compile src/c/**.c with MSC 6.00A and cut their ranges out of the asm
    import pickle
    an = pickle.load(open(pk, 'rb'))
    entries = set(k for k in an['entries'] if k in an['insns'])
    cobjs, holes = [], []
    for src in cport.sources(REPO):
        c = cport.compile_source(src, os.path.join(work, 'cobj'))
        start = c['frame'] * 16 + c['offset']
        end = start + c['length']
        if (c['image'], start) not in entries:
            raise SystemExit('%s: placement %s %04x:%04x is not a function entry' % (
                c['path'], c['image'], c['frame'], c['offset']))
        im = exe.root_memory() if c['image'] == 'root' else exe.section_memory(int(c['image'][1:]))
        ilo = 0 if c['image'] == 'root' else exe.sections[int(c['image'][1:])].load_seg * 16
        cport.infer_data_placement(c, bytes(im[start - ilo: start - ilo + c['length']]))
        # the module's data (literals, segment words) replaces the same bytes of DGROUP
        for dn, doff in c['data'].items():
            dl = 0x321a0 + doff
            holes.append(('s136', dl, dl + c['datasegs'][dn], {}))
        names = {start + off: n for n, off in c['publics']}
        holes.append((c['image'], start, end, names))
        cobjs.append(c)
    if cobjs:
        print('C modules: %d files, %d functions, %d bytes' % (
            len(cobjs), sum(len(c['publics']) for c in cobjs), sum(c['length'] for c in cobjs)))
    for it in range(6):
        em = emit_asm.Emitter(a.exe, pk, force, holes)
        files, layout = em.emit_all(asm_dir)
        new = encoding_check(em.emitted, work)
        new -= force
        print('iteration %d: %d regions, %d instructions emitted symbolically, %d new force-db' % (
            it, len(files), len(em.emitted), len(new)))
        # assemble all
        errs = {}
        import concurrent.futures as cf
        def asm1(fn):
            b = os.path.splitext(os.path.basename(fn))[0]
            rc, out = run_jwasm(fn, os.path.join(obj_dir, b + '.obj'), os.path.join(obj_dir, b + '.lst'))
            return fn, rc, out
        if not new:
            with cf.ThreadPoolExecutor(4) as ex:
                for fn, rc, out in ex.map(asm1, files):
                    if rc != 0 or 'Error' in out:
                        errs[fn] = out
            for fn, out in errs.items():
                lines = open(fn).read().split('\n')
                for m in re.finditer(r'\((\d+)\) : Error', out):
                    ln = lines[int(m.group(1)) - 1]
                    mm = re.search(r'; @([0-9a-f]+) ', ln)
                    img = os.path.basename(os.path.dirname(fn))
                    if mm:
                        new.add((img, int(mm.group(1), 16)))
                    else:
                        print('unattributable error', fn, ln)
            print('  assembly errors in %d files -> %d force-db' % (len(errs), len(new)))
        force |= new
        json.dump(sorted(force), open(fpath, 'w'))
        if not new:
            break
    L = make_layout(exe, layout['regions'])
    per_obj = {}
    for c in cobjs:
        L['modules'][c['segment']] = {'image': c['image'], 'frame': c['frame'], 'offset': c['offset']}
        # data segments have common names (_DATA/CONST) - give each object its own mapping
        per_obj[c['obj']] = {n: {'image': 's136', 'frame': 0x321a, 'offset': off} for n, off in c['data'].items()}
    L['object_modules'] = per_obj
    json.dump(L, open(os.path.join(work, 'layout.json'), 'w'))
    objs = [os.path.join(obj_dir, os.path.splitext(os.path.basename(f))[0] + '.obj') for f in files]
    objs += [c['obj'] for c in cobjs]
    mods = [omflink.Module(o) for o in objs]
    lk = omflink.Linker(L)
    lk.place(mods)
    lk.apply(mods)
    out = os.path.join(work, 'DARKLAND.EXE')
    data = lk.write_exe(out)
    # symbol table for quick per-file checks (tools/cmatch.py)
    json.dump({n: list(v) for n, v in lk.symbols.items()}, open(os.path.join(work, 'symbols.json'), 'w'))
    ref = open(a.exe, 'rb').read()
    print('rebuilt %s: %d bytes md5 %s' % (out, len(data), hashlib.md5(data).hexdigest()))
    print('original      : %d bytes md5 %s' % (len(ref), hashlib.md5(ref).hexdigest()))
    if data == ref:
        print('BYTE-IDENTICAL')
    else:
        diffs = [i for i in range(min(len(ref), len(data))) if ref[i] != data[i]]
        print('DIFFERENT: %d differing bytes; first %s' % (len(diffs), [hex(x) for x in diffs[:16]]))
        # attribute differences to C modules (image-level compare)
        rexe = RTLinkExe(a.exe)
        for c in cobjs:
            st = c['frame'] * 16 + c['offset']
            img = c['image']
            orig = rexe.root_memory() if img == 'root' else rexe.section_memory(int(img[1:]))
            new = lk.images[img]['mem']
            lo = lk.images[img]['lo']
            seg = bytes(orig[st - lo: st - lo + c['length']]) == bytes(new[st - lo: st - lo + c['length']])
            print('  %-40s %s' % (os.path.relpath(c['path'], REPO), 'match' if seg else 'MISMATCH'))
        sys.exit(1)
    stats = {'regions': len(files), 'symbolic_instructions': len(em.emitted), 'forced_db': len(force)}
    json.dump(stats, open(os.path.join(work, 'build_stats.json'), 'w'))


if __name__ == '__main__':
    main()
