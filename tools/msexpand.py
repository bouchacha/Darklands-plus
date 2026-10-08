#!/usr/bin/env python3
"""Expand Microsoft COMPRESS files (SZDD and the older 'SZ ' QBasic/MSC 6 variant).
   msexpand.py <in> <out>"""
import sys, struct


def expand(data):
    if data[:8] == b'SZDD\x88\xf0\x27\x33':
        n = struct.unpack_from('<I', data, 10)[0]
        p, pos = 14, 4096 - 16
    elif data[:8] == b'SZ \x88\xf0\x27\x33\xd1':
        n = struct.unpack_from('<I', data, 8)[0]
        p, pos = 12, 4096 - 18
    else:
        raise ValueError('not an SZ/SZDD file')
    win = bytearray(b' ' * 4096)
    out = bytearray()
    while p < len(data) and len(out) < n:
        flags = data[p]
        p += 1
        for bit in range(8):
            if p >= len(data) or len(out) >= n:
                break
            if flags & (1 << bit):
                c = data[p]
                p += 1
                out.append(c)
                win[pos] = c
                pos = (pos + 1) & 4095
            else:
                if p + 1 >= len(data):
                    break
                b0, b1 = data[p], data[p + 1]
                p += 2
                off = b0 | ((b1 & 0xf0) << 4)
                ln = (b1 & 0x0f) + 3
                for k in range(ln):
                    c = win[(off + k) & 4095]
                    out.append(c)
                    win[pos] = c
                    pos = (pos + 1) & 4095
    return bytes(out[:n])


if __name__ == '__main__':
    open(sys.argv[2], 'wb').write(expand(open(sys.argv[1], 'rb').read()))
