#!/usr/bin/env python3
"""
omflink.py - a small OMF linker that produces an RTLink/Plus-layout DARKLAND.EXE.

The original game was linked with Pocket Soft's RTLink/Plus, which is not available. This
linker replaces it for the reconstruction: it consumes 16-bit OMF object files (from JWasm
today, from Microsoft C 6.0 later), places every module's segment at the link-time address
given by a layout file, resolves PUBDEF/EXTDEF symbols across all overlays, applies
FIXUPP records, regenerates every relocation table, rebuilds the RTLink section table and the
MZ header, and writes the executable.

Layout file (JSON, produced by tools/make_layout.py from the original EXE):
  images:   name -> {base_seg, mem_size, file_size, section, parent, flags, w1, sect_id}
  file_order: section indices in file order
  mz:       header template fields; manager: section table location
  reloc_order: per image, original relocation order (linear addresses) - only used to make
            the output byte-identical; relocations not in the hint are appended sorted.
  modules:  segment name -> {image, frame}

Supported OMF subset: THEADR, COMENT, LNAMES, SEGDEF(16), GRPDEF, PUBDEF(16/32), EXTDEF,
LEDATA(16/32), LIDATA(16), FIXUPP (threads + fixups; location types 0,1,2,3,4,5),
MODEND. One or more segments per module.
"""
import sys, os, json, struct, collections


class LinkError(Exception):
    pass


def read_index(b, p):
    v = b[p]
    if v & 0x80:
        return ((v & 0x7f) << 8) | b[p + 1], p + 2
    return v, p + 1


def read_name(b, p):
    n = b[p]
    return b[p + 1:p + 1 + n].decode('latin-1'), p + 1 + n


class Module:
    def __init__(self, path):
        self.path = path
        self.name = None
        self.lnames = [None]
        self.segs = [None]          # (name, length)
        self.groups = [None]
        self.exts = [None]          # names
        self.pubs = []              # (name, seg index, offset)
        self.data = []              # (seg index, offset, bytes)
        self.fixups = []            # (seg index, location offset in seg, loc type, M, frame (m, idx), target (m, idx), disp)
        self.parse(open(path, 'rb').read())

    def parse(self, b):
        p = 0
        last_data = None            # (seg index, offset) of last LEDATA/LIDATA
        frame_threads = [None] * 4
        target_threads = [None] * 4
        while p < len(b):
            rt = b[p]
            ln = struct.unpack_from('<H', b, p + 1)[0]
            rec = b[p + 3:p + 3 + ln - 1]
            p += 3 + ln
            if rt == 0x80:
                self.name, _ = read_name(rec, 0)
            elif rt == 0x96:
                q = 0
                while q < len(rec):
                    n, q = read_name(rec, q)
                    self.lnames.append(n)
            elif rt in (0x98, 0x99):
                attr = rec[0]
                q = 1
                if (attr >> 5) == 0:
                    q += 3
                if rt == 0x98:
                    length = struct.unpack_from('<H', rec, q)[0]
                    q += 2
                    if attr & 2:
                        length = 0x10000
                else:
                    length = struct.unpack_from('<I', rec, q)[0]
                    q += 4
                ni, q = read_index(rec, q)
                self.segs.append((self.lnames[ni], length))
            elif rt == 0x9a:
                ni, q = read_index(rec, 0)
                members = []
                while q < len(rec):
                    assert rec[q] == 0xff
                    si, q = read_index(rec, q + 1)
                    members.append(si)
                self.groups.append((self.lnames[ni], members))
            elif rt in (0x8c, 0xb4):
                q = 0
                while q < len(rec):
                    n, q = read_name(rec, q)
                    _, q = read_index(rec, q)
                    self.exts.append(n)
            elif rt in (0x90, 0x91, 0xb6, 0xb7):
                gi, q = read_index(rec, 0)
                si, q = read_index(rec, q)
                if si == 0:
                    q += 2
                while q < len(rec):
                    n, q = read_name(rec, q)
                    if rt in (0x90, 0xb6):
                        off = struct.unpack_from('<H', rec, q)[0]
                        q += 2
                    else:
                        off = struct.unpack_from('<I', rec, q)[0]
                        q += 4
                    _, q = read_index(rec, q)
                    self.pubs.append((n, si, off))
            elif rt in (0xa0, 0xa1):
                si, q = read_index(rec, 0)
                if rt == 0xa0:
                    off = struct.unpack_from('<H', rec, q)[0]
                    q += 2
                else:
                    off = struct.unpack_from('<I', rec, q)[0]
                    q += 4
                self.data.append((si, off, bytes(rec[q:])))
                last_data = (si, off)
            elif rt == 0xa2:
                si, q = read_index(rec, 0)
                off = struct.unpack_from('<H', rec, q)[0]
                q += 2
                blob, q = self.lidata(rec, q)
                self.data.append((si, off, blob))
                last_data = (si, off)
            elif rt in (0x9c, 0x9d):
                q = 0
                while q < len(rec):
                    h = rec[q]
                    if not (h & 0x80):                       # THREAD
                        d = (h >> 6) & 1
                        method = (h >> 2) & 7
                        thr = h & 3
                        q += 1
                        idx = None
                        if (d == 0 and method < 4) or (d == 1 and method < 3):
                            idx, q = read_index(rec, q)
                        if d == 0:
                            target_threads[thr] = (method & 3, idx)
                        else:
                            frame_threads[thr] = (method, idx)
                        continue
                    locat = (h << 8) | rec[q + 1]
                    q += 2
                    M = (locat >> 14) & 1
                    loctype = (locat >> 10) & 0xf
                    dofs = locat & 0x3ff
                    fd = rec[q]
                    q += 1
                    if fd & 0x80:
                        frame = frame_threads[(fd >> 4) & 3]
                    else:
                        fm = (fd >> 4) & 7
                        fi = None
                        if fm < 3:
                            fi, q = read_index(rec, q)
                        frame = (fm, fi)
                    P = (fd >> 2) & 1
                    if fd & 0x08:
                        tmeth, tidx = target_threads[fd & 3]
                    else:
                        tmeth = fd & 3
                        tidx, q = read_index(rec, q)
                    disp = 0
                    if not P:
                        if rt == 0x9c:
                            disp = struct.unpack_from('<H', rec, q)[0]
                            q += 2
                        else:
                            disp = struct.unpack_from('<I', rec, q)[0]
                            q += 4
                    si, base = last_data
                    self.fixups.append((si, base + dofs, loctype, M, frame, (tmeth, tidx), disp))
            elif rt in (0x8a, 0x8b, 0x88):
                pass
            else:
                raise LinkError('%s: unsupported OMF record %02x' % (self.path, rt))

    def lidata(self, rec, q):
        def block(q):
            rep = struct.unpack_from('<H', rec, q)[0]
            cnt = struct.unpack_from('<H', rec, q + 2)[0]
            q += 4
            if cnt == 0:
                n = rec[q]
                content = bytes(rec[q + 1:q + 1 + n])
                q += 1 + n
            else:
                content = b''
                for _ in range(cnt):
                    c, q = block(q)
                    content += c
            return content * rep, q
        out = b''
        while q < len(rec):
            c, q = block(q)
            out += c
        return out, q


class Linker:
    def __init__(self, layout):
        self.L = layout
        self.images = {}
        for name, im in layout['images'].items():
            self.images[name] = {'mem': bytearray(im['mem_size']), 'written': bytearray(im['mem_size']),
                                 'lo': im['base_seg'] * 16, 'relocs': {}}
        self.symbols = {}           # name -> (image, linear, frame)
        self.placed = []            # (module, seg index, image, frame)

    def place(self, mods):
        segmap = self.L['modules']
        for m in mods:
            for si in range(1, len(m.segs)):
                sname, slen = m.segs[si]
                if sname not in segmap:
                    raise LinkError('%s: segment %s not in layout' % (m.path, sname))
                img, frame = segmap[sname]['image'], segmap[sname]['frame']
                base = segmap[sname].get('offset', 0)    # e.g. a compiled C _TEXT placed mid-segment
                m.__dict__.setdefault('segplace', {})[si] = (img, frame, base)
            for si, off, blob in m.data:
                img, frame, base = m.segplace[si]
                off += base
                I = self.images[img]
                lin = frame * 16 + off - I['lo']
                if lin < 0 or lin + len(blob) > len(I['mem']):
                    raise LinkError('%s: data outside image %s at %x' % (m.path, img, frame * 16 + off))
                for k, bt in enumerate(blob):
                    if I['written'][lin + k] and I['mem'][lin + k] != bt:
                        raise LinkError('%s: overlapping data at %s:%x' % (m.path, img, frame * 16 + off + k))
                    I['mem'][lin + k] = bt
                    I['written'][lin + k] = 1
            for n, si, off in m.pubs:
                if si == 0:
                    continue                                # absolute public (EQU) - not needed
                img, frame, base = m.segplace[si]
                if n in self.symbols:
                    raise LinkError('duplicate symbol %s (%s)' % (n, m.path))
                self.symbols[n] = (img, frame * 16 + base + off, frame)

    def group_frame(self, m, gi):
        """Groups (e.g. DGROUP from compiled C) map to a fixed frame given in layout['groups']."""
        gname = m.groups[gi][0]
        g = self.L.get('groups', {}).get(gname)
        if g is None:
            raise LinkError('%s: group %s not in layout' % (m.path, gname))
        return g

    def resolve_ext(self, name):
        if name in self.symbols:
            return self.symbols[name]
        if name.startswith('FRM_abs_'):
            f = int(name[8:], 16)
            return (None, f * 16, f)
        raise LinkError('unresolved external %s' % name)

    def apply(self, mods):
        for m in mods:
            for si, loc_off, loctype, M, frame, target, disp in m.fixups:
                limg, lframe, lbase = m.segplace[si]
                I = self.images[limg]
                loc_lin = lframe * 16 + lbase + loc_off
                p = loc_lin - I['lo']
                tm, ti = target
                if tm == 0:
                    timg, tframe, tbase = m.segplace[ti]
                    tlin, tfr = tframe * 16 + tbase + disp, tframe
                elif tm == 2:
                    timg, slin, tfr = self.resolve_ext(m.exts[ti])
                    tlin = slin + disp
                elif tm == 1:
                    gf = self.group_frame(m, ti)
                    timg, tlin, tfr = None, gf * 16 + disp, gf
                else:
                    raise LinkError('target method %d' % tm)
                fm, fi = frame
                if fm == 0:
                    fr = m.segplace[fi][1]
                elif fm == 1:
                    fr = self.group_frame(m, fi)
                elif fm == 2:
                    fr = self.resolve_ext(m.exts[fi])[2]
                elif fm == 4:
                    fr = lframe
                elif fm == 5:
                    fr = tfr
                else:
                    raise LinkError('frame method %d' % fm)
                mem = I['mem']
                if loctype in (1, 5):
                    old = struct.unpack_from('<H', mem, p)[0]
                    if M:
                        v = tlin - fr * 16
                    else:
                        v = tlin - (loc_lin + 2)
                    struct.pack_into('<H', mem, p, (old + v) & 0xffff)
                elif loctype == 2:
                    old = struct.unpack_from('<H', mem, p)[0]
                    struct.pack_into('<H', mem, p, (old + fr) & 0xffff)
                    I['relocs'][loc_lin] = lframe
                elif loctype == 3:
                    old = struct.unpack_from('<H', mem, p)[0]
                    struct.pack_into('<H', mem, p, (old + tlin - fr * 16) & 0xffff)
                    old = struct.unpack_from('<H', mem, p + 2)[0]
                    struct.pack_into('<H', mem, p + 2, (old + fr) & 0xffff)
                    I['relocs'][loc_lin + 2] = lframe
                elif loctype in (0, 4):
                    v = (tlin - fr * 16) if M else (tlin - (loc_lin + 1))
                    if loctype == 4:
                        v >>= 8
                    mem[p] = (mem[p] + v) & 0xff
                else:
                    raise LinkError('location type %d' % loctype)

    # ------------------------------------------------------------------ EXE writer
    def write_exe(self, path=None):
        L = self.L
        mz = L['mz']
        mgr = L['manager']
        imgs = L['images']
        root = self.images['root']
        rootmem = root['mem']
        root_size = imgs['root']['file_size']

        def ordered_relocs(name):
            I = self.images[name]
            hint = L['reloc_order'].get(name, [])
            have = dict(I['relocs'])
            out = []
            for lin in hint:
                if lin in have:
                    out.append((lin, have.pop(lin)))
            out.extend(sorted(have.items()))
            return [((lin - f * 16), f) for lin, f in out]

        # section file positions
        root_relocs = ordered_relocs('root')
        hdr_bytes = 0x20 + 4 * len(root_relocs)
        hdr_size = (hdr_bytes + 511) // 512 * 512
        pos = hdr_size + root_size
        pos = (pos + 15) & ~15
        sect_blobs = []
        table = {}
        for idx in L['file_order']:
            name = 's%03d' % idx
            im = imgs[name]
            rel = ordered_relocs(name)
            fpara = pos // 16
            blob = bytearray()
            for off, seg in rel:
                blob += struct.pack('<HH', off, seg)
            blob += bytes((-len(blob)) % 16)
            body = self.images[name]['mem'][:im['file_size']]
            blob += body
            # sections are paragraph aligned in the file; the last one is not padded
            if idx != L['file_order'][-1]:
                blob += bytes((-len(blob)) % 16)
            sect_blobs.append(blob)
            table[idx] = (im['base_seg'], im['w1'], fpara & 0xffff, (im['flags'] << 8) | (fpara >> 16),
                          im['mem_size'] // 16, len(rel), im['parent_index'], im['sect_id'],
                          (im['file_size'] + 15) // 16)
            pos += len(blob)
        # patch section table in the manager segment
        tbase = mgr['seg'] * 16 + mgr['table']
        for idx, w in table.items():
            struct.pack_into('<9H', rootmem, tbase + idx * 0x12, *w)
        # MZ header
        total = hdr_size + root_size
        hdr = bytearray(hdr_size)
        pages = (total + 511) // 512
        last = total % 512
        struct.pack_into('<2s13H', hdr, 0, b'MZ', last, pages, len(root_relocs), hdr_size // 16,
                         mz['min_alloc'], mz['max_alloc'], mz['ss'], mz['sp'], 0, mz['ip'], mz['cs'],
                         0x20, 0)
        for k, (off, seg) in enumerate(root_relocs):
            struct.pack_into('<HH', hdr, 0x20 + 4 * k, off, seg)
        out = bytearray(hdr)
        out += rootmem[:root_size]
        out += bytes((-len(out)) % 16)
        for blob in sect_blobs:
            out += blob
        if path:
            open(path, 'wb').write(out)
        return bytes(out)


def main():
    import argparse
    ap = argparse.ArgumentParser()
    ap.add_argument('layout')
    ap.add_argument('out')
    ap.add_argument('objs', nargs='+')
    ap.add_argument('--compare')
    a = ap.parse_args()
    layout = json.load(open(a.layout))
    objs = []
    for o in a.objs:
        if o.endswith('.txt'):
            objs += [l.strip() for l in open(o) if l.strip()]
        else:
            objs.append(o)
    mods = [Module(o) for o in objs]
    lk = Linker(layout)
    lk.place(mods)
    lk.apply(mods)
    data = lk.write_exe(a.out)
    print('linked %d modules, %d symbols -> %s (%d bytes)' % (len(mods), len(lk.symbols), a.out, len(data)))
    if a.compare:
        ref = open(a.compare, 'rb').read()
        if ref == data:
            print('IDENTICAL to', a.compare)
        else:
            diffs = [i for i in range(min(len(ref), len(data))) if ref[i] != data[i]]
            print('DIFFERENT: size %d vs %d, %d differing bytes, first at %s' % (
                len(data), len(ref), len(diffs), [hex(x) for x in diffs[:10]]))
            sys.exit(1)


if __name__ == '__main__':
    main()
