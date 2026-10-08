"""MicroProse LZ compression used inside Darklands catalogs.

Used for every ``*.IMC`` sprite file (A00C/C00C/E00C/F01C/F60C/M00C.CAT),
every ``IMAPS.CAT`` entry, every ``BC`` entry, every ``LCASTLE`` entry and
every top-level ``*.PAN`` animation (newly identified here).
(vvendigo's ``reader_drle.py`` calls it "DRLE"; it is actually an LZ77/LZSS
scheme whose bitstream is laid out like LZEXE 0.91's.)

Bitstream
---------
Control bits come from little-endian 16-bit words, LSB first.  The next
control word is fetched *immediately* after the 16th bit of the current one
has been consumed (i.e. before any data byte belonging to that token), so
control words are interleaved with data bytes.

====================  =====================================================
bits                  meaning
====================  =====================================================
``1``                 literal: copy next data byte
``0 0 a b`` + byte B  short match: length ``(a<<1|b) + 2`` (2..5),
                      distance ``256 - B`` (1..256)
``0 1`` + word W      long match: ``x = ((W >> 11) << 8) | (W & 0xFF)``,
                      distance ``0x2000 - x`` (1..8192),
                      ``n = (W >> 8) & 7``; if ``n``: length ``n + 2`` (3..9)
                      else read byte C: ``C == 0`` end of stream,
                      ``C == 1`` "segment" marker (no output; decoding
                      continues), otherwise length ``C + 1`` (3..256)
====================  =====================================================

Encoder (reconstructed; reproduces all 585 shipped streams -- 570 catalog
entries plus 15 PAN files -- byte-exactly):

* greedy; at each position take the longest match (max 253) among previous
  positions within 7939 bytes (= 8192 - 253, i.e. an 8 KiB ring buffer with a
  253-byte look-ahead), ties broken by the nearest distance;
* match length is computed against a *padded* input: bytes beyond the end of
  the data are the stale ring-buffer contents, i.e. ``data[n + k - 8192]``
  (or 0 when that is negative); the length is then clipped to the remaining
  input -- this only matters for the last token(s);
* a length-2 match is only used when its distance is <= 255 (short form);
  otherwise a literal is emitted.  Lengths 3..5 with distance <= 255 use the
  short form, everything else the long form (extra length byte if > 9);
* after a token that brings the output position to >= 0xA000 bytes past the
  previous segment marker (or the start) and more data follows, a segment
  marker (``W = 0xF000, C = 1``) is emitted -- this is how a 16-bit
  decompressor knows to renormalise its output pointer (seen in M40WKWH.IMC
  and in every ``*.PAN`` file);
* the stream ends with ``W = 0xF000, C = 0``; the last control word is
  flushed with its unused high bits zero.
"""
from __future__ import annotations

from .util import FormatError

RING = 8192
MAXLEN = 253
WINDOW = RING - MAXLEN      # 7939
SHORT_MAXDIST = 255
SEGMENT = 0xA000


def decompress(data: bytes, return_consumed: bool = False):
    d = data
    n = len(d)
    if n < 2:
        raise FormatError("LZ: stream too short")
    pos = 2
    ctrl = d[0] | (d[1] << 8)
    left = 16
    out = bytearray()

    def bit():
        nonlocal ctrl, left, pos
        b = ctrl & 1
        ctrl >>= 1
        left -= 1
        if left == 0:
            if pos + 2 > n:
                raise FormatError("LZ: truncated control word")
            ctrl = d[pos] | (d[pos + 1] << 8)
            pos += 2
            left = 16
        return b

    try:
        while True:
            if bit():
                out.append(d[pos])
                pos += 1
                continue
            if bit():
                w = d[pos] | (d[pos + 1] << 8)
                pos += 2
                length = (w >> 8) & 7
                dist = 0x2000 - (((w >> 11) << 8) | (w & 0xFF))
                if length == 0:
                    c = d[pos]
                    pos += 1
                    if c == 0:
                        break
                    if c == 1:
                        continue
                    length = c + 1
                else:
                    length += 2
            else:
                length = ((bit() << 1) | bit()) + 2
                dist = 256 - d[pos]
                pos += 1
            start = len(out) - dist
            if start < 0:
                raise FormatError("LZ: match before start of output")
            for i in range(length):
                out.append(out[start + i])
    except IndexError:
        raise FormatError("LZ: truncated stream") from None
    if return_consumed:
        return bytes(out), pos
    return bytes(out)


class _BitWriter:
    __slots__ = ("out", "slot", "ctrl", "n")

    def __init__(self):
        self.out = bytearray(b"\0\0")
        self.slot = 0
        self.ctrl = 0
        self.n = 0

    def bit(self, b):
        self.ctrl |= (b & 1) << self.n
        self.n += 1
        if self.n == 16:
            self.out[self.slot] = self.ctrl & 0xFF
            self.out[self.slot + 1] = self.ctrl >> 8
            self.slot = len(self.out)
            self.out += b"\0\0"
            self.ctrl = 0
            self.n = 0

    def finish(self) -> bytes:
        self.out[self.slot] = self.ctrl & 0xFF
        self.out[self.slot + 1] = self.ctrl >> 8
        return bytes(self.out)


def compress(data: bytes) -> bytes:
    """Compress exactly like the original MicroProse tool (see module doc)."""
    data = bytes(data)
    n = len(data)
    pad = bytes(data[n + k - RING] if n + k - RING >= 0 else 0 for k in range(MAXLEN + 2))
    pd = data + pad
    w = _BitWriter()
    out = w.out
    chains: dict[bytes, list[int]] = {}
    pos = 0
    segbase = 0
    while pos < n:
        bestl = 0
        bestd = 0
        lst = chains.get(pd[pos:pos + 2])
        if lst:
            for i in range(len(lst) - 1, -1, -1):
                q = lst[i]
                dist = pos - q
                if dist > WINDOW:
                    break
                if pd[q + 2] != pd[pos + 2]:
                    l = 2
                else:
                    l = 3
                    while l < MAXLEN and pd[q + l] == pd[pos + l]:
                        l += 1
                if l > bestl:
                    bestl, bestd = l, dist
                    if l == MAXLEN:
                        break
        if bestl > n - pos:
            bestl = n - pos
        if bestl < 2 or (bestl == 2 and bestd > SHORT_MAXDIST):
            w.bit(1)
            out.append(data[pos])
            L = 1
        else:
            L = bestl
            if L <= 5 and bestd <= SHORT_MAXDIST:
                w.bit(0)
                w.bit(0)
                k = L - 2
                w.bit(k >> 1)
                w.bit(k & 1)
                out.append((256 - bestd) & 0xFF)
            else:
                x = 0x2000 - bestd
                w.bit(0)
                w.bit(1)
                if L <= 9:
                    v = ((x >> 8) << 11) | ((L - 2) << 8) | (x & 0xFF)
                    out += bytes((v & 0xFF, v >> 8))
                else:
                    v = ((x >> 8) << 11) | (x & 0xFF)
                    out += bytes((v & 0xFF, v >> 8, L - 1))
        for p in range(pos, pos + L):
            key = pd[p:p + 2]
            c = chains.get(key)
            if c is None:
                chains[key] = [p]
            else:
                c.append(p)
        pos += L
        if pos - segbase >= SEGMENT and pos < n:
            w.bit(0)
            w.bit(1)
            out += b"\x00\xF0\x01"
            segbase = pos
    w.bit(0)
    w.bit(1)
    out += b"\x00\xF0\x00"
    return w.finish()


def is_compressed_name(name: str) -> bool:
    """Heuristic used by the dump tool: which catalog entries are LZ streams."""
    u = name.upper()
    ext = u.rsplit(".", 1)[-1] if "." in u else ""
    if ext == "IMC":
        return True
    if ext.isdigit() and len(ext) == 3:          # IMAPS.CAT entries
        return True
    if ext in ("FLC", "FFC", "FRC", "NWC", "WWC", "NFC", "WFC"):   # BC
        return True
    if u.startswith("LC_"):                       # LCASTLE
        return True
    return False
