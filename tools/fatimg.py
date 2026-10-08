#!/usr/bin/env python3
"""Minimal FAT12/FAT16 floppy-image extractor: fatimg.py <image> <out_dir>"""
import sys, os, struct


def extract(img_path, out):
    d = open(img_path, 'rb').read()
    bps, spc, rsv, nfat, nroot, tot16, media, spf = struct.unpack_from('<HBHBHHBH', d, 11)
    fat_off = rsv * bps
    root_off = fat_off + nfat * spf * bps
    data_off = root_off + nroot * 32
    nclusters = ((tot16 or len(d) // bps) * bps - data_off) // (spc * bps)
    fat12 = nclusters < 4085
    fat = d[fat_off:fat_off + spf * bps]

    def nxt(c):
        if fat12:
            v = struct.unpack_from('<H', fat, c * 3 // 2)[0]
            return v >> 4 if c & 1 else v & 0xfff
        return struct.unpack_from('<H', fat, c * 2)[0]

    def chain(c):
        out = []
        while 2 <= c < (0xff8 if fat12 else 0xfff8) and len(out) < 10000:
            out.append(c)
            c = nxt(c)
        return out

    def read_clusters(c, size=None):
        b = b''.join(d[data_off + (k - 2) * spc * bps: data_off + (k - 1) * spc * bps] for k in chain(c))
        return b if size is None else b[:size]

    def walk(entries, path):
        for i in range(0, len(entries), 32):
            e = entries[i:i + 32]
            if len(e) < 32 or e[0] == 0:
                break
            if e[0] == 0xe5 or e[11] == 0x0f or e[11] & 0x08:
                continue
            name = e[0:8].decode('latin-1').rstrip()
            ext = e[8:11].decode('latin-1').rstrip()
            fn = name + ('.' + ext if ext else '')
            if fn in ('.', '..'):
                continue
            clus = struct.unpack_from('<H', e, 26)[0]
            size = struct.unpack_from('<I', e, 28)[0]
            p = os.path.join(path, fn)
            if e[11] & 0x10:
                os.makedirs(p, exist_ok=True)
                yield from walk(read_clusters(clus), p)
            else:
                open(p, 'wb').write(read_clusters(clus, size) if size else b'')
                yield p
    os.makedirs(out, exist_ok=True)
    return list(walk(d[root_off:data_off], out))


if __name__ == '__main__':
    files = extract(sys.argv[1], sys.argv[2])
    print(len(files), 'files')
