"""FONTS.FNT / FONTS.UTL -- bitmap fonts.

Layout::

    u16 font_count                (FNT: 3, UTL: 4)
    u16 font_offset[font_count]   offset of each font's *bitmap*; the header
                                  and width table precede it
    per font (in order):
        u8 width[last-first+1]    pixel width of each glyph
        u8 first_char, last_char
        u8 bytes_per_row          (bytes per glyph row)
        u8 unknown_3 (0)
        u8 height
        u8 char_spacing (1)
        u8 line_spacing (1, or 0 for UTL font 3)
        u8 unknown_7 (0)
        u8 bitmap[height + line_spacing][glyph_count][bytes_per_row]
                                  row-major over all glyphs, MSB = leftmost

New finding: the bitmap holds ``height + line_spacing`` rows (the extra rows
are blank).  Without that the fonts appear to be separated by unexplained
gaps (256, 97, 194 bytes).  In FONTS.UTL one 0x00 byte precedes the 4th
font's width table (even-offset alignment?); it is kept in ``lead_pad``.
Both files reproduce exactly.

FONTS.UTL contains three fonts with the same metrics as FONTS.FNT's (a few
glyphs differ: font 0 chars 3,18,19,43,70,93,95; font 1 chars 73,95; font 2
chars 79,93,95) plus a fourth small font (probably for the DKED editor /
utilities).  Font 0 covers chars 0..127
(0..31 are UI symbols), fonts 1..2 cover 31..127 (31 = 'ä').
"""
from __future__ import annotations

import dataclasses
import struct

from .util import FormatError, read_file


@dataclasses.dataclass
class Font:
    first_char: int
    last_char: int
    bytes_per_row: int
    height: int
    char_spacing: int
    line_spacing: int
    widths: list[int]
    bitmap: bytes = dataclasses.field(repr=False)   # (height+line_spacing) * count * bytes_per_row
    unknown_3: int = 0
    unknown_7: int = 0
    lead_pad: bytes = b""    # bytes between the previous font and this one (FONTS.UTL font 3: one 0x00,
                             # apparently aligning the width table to an even offset)

    @property
    def count(self) -> int:
        return self.last_char - self.first_char + 1

    def glyph(self, ch: int) -> list[list[int]]:
        """Return the glyph as rows of 0/1 (height + line_spacing rows)."""
        i = ch - self.first_char
        if not 0 <= i < self.count:
            raise KeyError(ch)
        bw = self.bytes_per_row
        rows = []
        for y in range(self.height + self.line_spacing):
            o = (y * self.count + i) * bw
            bits = int.from_bytes(self.bitmap[o:o + bw], "big")
            rows.append([(bits >> (bw * 8 - 1 - x)) & 1 for x in range(self.widths[i])])
        return rows


@dataclasses.dataclass
class FontFile:
    fonts: list[Font]


def parse(data: bytes) -> FontFile:
    n, = struct.unpack_from("<H", data, 0)
    offs = struct.unpack_from(f"<{n}H", data, 2)
    fonts = []
    expect = 2 + 2 * n
    for o in offs:
        first, last, bw, u3, h, cs, ls, u7 = data[o - 8:o]
        cnt = last - first + 1
        wstart = o - 8 - cnt
        if wstart < expect:
            raise FormatError(f"FNT: font at {o} overlaps previous ({wstart} < {expect})")
        lead = data[expect:wstart]
        widths = list(data[wstart:o - 8])
        size = (h + ls) * cnt * bw
        bitmap = data[o:o + size]
        if len(bitmap) != size:
            raise FormatError("FNT: bitmap truncated")
        fonts.append(Font(first, last, bw, h, cs, ls, widths, bitmap, u3, u7, lead))
        expect = o + size
    if expect != len(data):
        raise FormatError(f"FNT: {len(data) - expect} trailing bytes")
    return FontFile(fonts)


def read(path) -> FontFile:
    return parse(read_file(path))


def build(ff: FontFile) -> bytes:
    n = len(ff.fonts)
    pos = 2 + 2 * n
    offs = []
    blobs = []
    for f in ff.fonts:
        if len(f.widths) != f.count:
            raise FormatError("FNT: width table size mismatch")
        pos += len(f.lead_pad)
        blobs.append(f.lead_pad)
        blob = bytes(f.widths) + bytes((f.first_char, f.last_char, f.bytes_per_row, f.unknown_3, f.height,
                                        f.char_spacing, f.line_spacing, f.unknown_7))
        offs.append(pos + len(blob))
        blob += f.bitmap
        blobs.append(blob)
        pos += len(blob)
    return struct.pack(f"<H{n}H", n, *offs) + b"".join(blobs)


def to_png(font: Font, path, scale: int = 2) -> None:
    """Render all glyphs in a 16-column grid (debug view)."""
    from PIL import Image
    cw = max(font.widths + [1]) + 1
    ch = font.height + font.line_spacing + 1
    cols = 16
    rows = (font.count + cols - 1) // cols
    im = Image.new("L", (cols * cw, rows * ch), 255)
    px = im.load()
    for i in range(font.count):
        g = font.glyph(font.first_char + i)
        ox, oy = (i % cols) * cw, (i // cols) * ch
        for y, r in enumerate(g):
            for x, v in enumerate(r):
                if v:
                    px[ox + x, oy + y] = 0
    if scale > 1:
        im = im.resize((im.width * scale, im.height * scale), Image.NEAREST)
    im.save(path)
