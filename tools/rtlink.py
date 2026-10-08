#!/usr/bin/env python3
"""
RTLink/Plus (Pocket Soft) internal-overlay executable parser for DARKLAND.EXE.

Layout of the file (all verified by repack() producing a byte-identical EXE):

  [MZ header + MZ relocation table]          (header_paragraphs * 16 bytes)
  [root load module]                         (resident code, segments 0000..ovl area)
  [padding to paragraph]
  for each section in the overlay manager's section table (in file order):
      [section relocations: count * (u16 off, u16 seg)]   (seg is link-time, relative to load base)
      [padding to paragraph]
      [section image: file_paras * 16 bytes]
      (memory image is mem_paras * 16 bytes; the tail beyond file_paras is zero-filled BSS)

The section table lives in the overlay manager's code segment (the MZ entry CS). Its
descriptor block holds pointers: [mgr+0xb58] -> word count, [mgr+0xb5a] -> table,
[mgr+0xb5c] = entry size (0x12). Each 0x12-byte entry is 9 little-endian words:

  w0 load_seg   link-time segment the section is loaded at (relative to load base)
  w1 (const)    0x1763 in this file - looks like a file/handle descriptor id
  w2,w3         w2 | (w3 & 0xff) << 16 = file position in paragraphs; w3 >> 8 = flags
                (flags 0x04 = demand-loaded overlay, 0x01 = preloaded at startup)
  w4 mem_paras  size in memory, paragraphs
  w5 nrelocs    number of 4-byte relocation entries preceding the image
  w6 parent     index of the parent section in the overlay tree (0xffff = top level)
  w7 sect_id    section id (index + 2 in this file; 0/1 presumably the root parts)
  w8 file_paras size of the stored image, paragraphs (< mem_paras when the section has BSS)
"""
import struct, sys, json, hashlib
from dataclasses import dataclass, field, asdict


@dataclass
class Section:
    index: int
    load_seg: int
    w1: int
    file_para: int
    flags: int
    mem_paras: int
    nrelocs: int
    parent: int
    sect_id: int
    file_paras: int
    relocs: list = field(default_factory=list)     # list of (off, seg)
    image: bytes = b''                              # file image (file_paras*16)
    reloc_pad: bytes = b''                          # padding between relocs and image
    tail_pad: bytes = b''                           # padding after image up to next section

    @property
    def file_off(self):
        return self.file_para * 16

    @property
    def end_seg(self):
        return self.load_seg + self.mem_paras

    def depth(self, sections):
        d, p = 0, self.parent
        while p != 0xffff:
            d += 1
            p = sections[p].parent
        return d


class RTLinkExe:
    MZ_FIELDS = ('sig', 'last_page', 'pages', 'nrelocs', 'hdr_paras', 'min_alloc', 'max_alloc',
                 'ss', 'sp', 'csum', 'ip', 'cs', 'reloc_off', 'ovl_no')

    def __init__(self, path):
        self.path = path
        self.data = data = open(path, 'rb').read()
        hdr = struct.unpack_from('<2s13H', data, 0)
        self.mz = dict(zip(self.MZ_FIELDS, hdr))
        mz = self.mz
        self.mz_size = (mz['pages'] - 1) * 512 + (mz['last_page'] or 512)
        self.hdr_size = mz['hdr_paras'] * 16
        self.header_bytes = data[:self.hdr_size]
        self.mz_relocs = [struct.unpack_from('<HH', data, mz['reloc_off'] + 4 * i)
                          for i in range(mz['nrelocs'])]          # (off, seg)
        self.root = data[self.hdr_size:self.mz_size]
        # overlay manager descriptor block in entry CS
        self.mgr_seg = mz['cs']
        mgr = self.hdr_size + self.mgr_seg * 16
        self.mgr_file_off = mgr
        rd = lambda o: struct.unpack_from('<H', data, mgr + o)[0]
        self.count_ptr, self.table_ptr, self.entsize = rd(0xb58), rd(0xb5a), rd(0xb5c)
        assert self.entsize == 0x12, hex(self.entsize)
        self.mgr_desc = {k: rd(o) for k, o in (
            ('flags', 0xb68), ('stack_seg', 0xb6a), ('count', self.count_ptr),
            ('w_b76', 0xb76), ('w_b78', 0xb78))}
        n = rd(self.count_ptr)
        self.sections = []
        for i in range(n):
            w = struct.unpack_from('<9H', data, mgr + self.table_ptr + i * self.entsize)
            s = Section(index=i, load_seg=w[0], w1=w[1], file_para=w[2] | ((w[3] & 0xff) << 16),
                        flags=w[3] >> 8, mem_paras=w[4], nrelocs=w[5], parent=w[6],
                        sect_id=w[7], file_paras=w[8])
            off = s.file_off
            s.relocs = [struct.unpack_from('<HH', data, off + 4 * k) for k in range(s.nrelocs)]
            rl_end = off + 4 * s.nrelocs
            img_off = (rl_end + 15) & ~15
            s.reloc_pad = data[rl_end:img_off]
            s.image = data[img_off:img_off + s.file_paras * 16]
            self.sections.append(s)
        # file order & padding bookkeeping
        order = sorted(self.sections, key=lambda s: s.file_off)
        self.file_order = [s.index for s in order]
        first = order[0].file_off
        self.root_tail_pad = data[self.mz_size:first]
        for a, b in zip(order, order[1:] + [None]):
            img_end = a.file_off + ((4 * a.nrelocs + 15) & ~15) + len(a.image)
            nxt = b.file_off if b else len(data)
            a.tail_pad = data[img_end:nxt]

    # ---------------------------------------------------------------- repack
    def repack(self):
        out = bytearray(self.header_bytes)
        out += self.root
        out += self.root_tail_pad
        for i in self.file_order:
            s = self.sections[i]
            assert len(out) == s.file_off, (i, hex(len(out)), hex(s.file_off))
            for off, seg in s.relocs:
                out += struct.pack('<HH', off, seg)
            out += s.reloc_pad
            out += s.image
            out += s.tail_pad
        return bytes(out)

    # ---------------------------------------------------------------- memory views
    def root_memory(self, base=0):
        """Resident image (MZ load module) with MZ relocs applied for load segment `base`."""
        m = bytearray(self.root)
        for off, seg in self.mz_relocs:
            p = seg * 16 + off
            v = struct.unpack_from('<H', m, p)[0]
            struct.pack_into('<H', m, p, (v + base) & 0xffff)
        return m

    def section_memory(self, idx, base=0):
        """Section image in memory (file image + zero BSS), relocs applied for load segment `base`."""
        s = self.sections[idx]
        m = bytearray(s.image) + bytes(s.mem_paras * 16 - len(s.image))
        for off, seg in s.relocs:
            p = (seg - s.load_seg) * 16 + off
            v = struct.unpack_from('<H', m, p)[0]
            struct.pack_into('<H', m, p, (v + base) & 0xffff)
        return m

    def reloc_sites(self, idx=None):
        """Linear (link-time) addresses of relocated words: root if idx is None, else section."""
        if idx is None:
            return sorted(seg * 16 + off for off, seg in self.mz_relocs)
        return sorted(seg * 16 + off for off, seg in self.sections[idx].relocs)

    def summary(self):
        mz = self.mz
        root_paras = len(self.root) // 16
        return {
            'file': self.path, 'size': len(self.data),
            'md5': hashlib.md5(self.data).hexdigest(),
            'mz': {k: (v.decode() if isinstance(v, bytes) else v) for k, v in mz.items()},
            'mz_load_module_bytes': len(self.root),
            'entry': '%04x:%04x' % (mz['cs'], mz['ip']),
            'stack': '%04x:%04x' % (mz['ss'], mz['sp']),
            'overlay_manager_seg': '%04x' % self.mgr_seg,
            'section_count': len(self.sections),
            'manager_descriptor': {k: '%04x' % v for k, v in self.mgr_desc.items()},
        }


def tree_lines(exe):
    secs = exe.sections
    kids = {}
    for s in secs:
        kids.setdefault(s.parent, []).append(s.index)
    lines = []

    def walk(i, d):
        s = secs[i]
        lines.append('%s[%3d] seg %04x-%04x  mem %6d B  file %6d B  relocs %4d  flags %02x  @file 0x%06x' % (
            '  ' * d, i, s.load_seg, s.end_seg, s.mem_paras * 16, s.file_paras * 16, s.nrelocs, s.flags, s.file_off))
        for k in kids.get(i, []):
            walk(k, d + 1)
    for r in kids.get(0xffff, []):
        walk(r, 0)
    return lines


if __name__ == '__main__':
    exe = RTLinkExe(sys.argv[1])
    cmd = sys.argv[2] if len(sys.argv) > 2 else 'summary'
    if cmd == 'summary':
        print(json.dumps(exe.summary(), indent=1))
    elif cmd == 'tree':
        print('\n'.join(tree_lines(exe)))
    elif cmd == 'verify':
        rebuilt = exe.repack()
        ok = rebuilt == exe.data
        print('repack byte-identical:', ok, 'size', len(rebuilt), 'md5', hashlib.md5(rebuilt).hexdigest())
        sys.exit(0 if ok else 1)
    elif cmd == 'json':
        out = []
        for s in exe.sections:
            d = asdict(s)
            d.pop('image'); d.pop('reloc_pad'); d.pop('tail_pad'); d.pop('relocs')
            out.append(d)
        print(json.dumps(out, indent=0))
