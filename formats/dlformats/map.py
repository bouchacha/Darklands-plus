"""DARKLAND.MAP -- the overland (wilderness) map.

Layout::

    u16be width  (327)          <- big-endian!
    u16be height (931)
    u32   row_offset[height]    absolute file offsets (rows are contiguous)
    rows: RLE bytes  ccc b rrrr
          ccc  = repeat count 1..7
          b    = tile bank: 0 -> MAPICONS.PIC, 1 -> MAPICON2.PIC
          rrrr = tile row within the bank

A cell value here is ``b << 4 | rrrr`` (0..31); see TERRAIN_LABELS for the
names the artists wrote next to each row of the tile sheets.  The encoder is greedy (runs
of up to 7, split at row ends); it reproduces the shipped file exactly.

The *column* of the tile graphic (its variant) is not stored: the game picks
it from the four diagonal neighbours (odd rows are shifted half a tile to
the right).  Merle's documented rule did not reproduce the game according to
vvendigo, who found a "same-kind" adjacency rule with special cases (rivers
join bridges, wet tiles join each other).  That rule lives in DARKLAND.EXE.
Locations (cities, castles, ...) are not in this file -- see DARKLAND.LOC.
"""
from __future__ import annotations

import dataclasses
import struct

from .util import FormatError, read_file


#: Labels painted by the artists next to each tile row in MAPICONS.PIC (bank 0)
#: and MAPICON2.PIC (bank 1); None where the row has no label.  Cell value =
#: bank * 16 + row.
TERRAIN_LABELS = {
    0: "plains", 1: "ocean", 2: "MjRvr (major river)", 3: "MnRvr (minor river)", 4: "TdlMrs (tidal marsh)",
    5: "Marsh", 6: "Geest", 7: "Gst1", 8: "Farm0", 9: "Farm1", 10: "F/W0", 11: "F/W1", 12: "LtW1",
    13: "LtW2", 14: "Frst0", 15: "Frst1", 16: "Frst2", 17: "Frst3", 18: "Rck0", 19: "Rck1", 20: "Rck2",
    21: "Rck3", 22: "Alp4", 23: "Alp5", 24: None, 25: "Ford", 26: None, 27: "Brdge", 28: "Cstle",
    29: "Cty", 30: None, 31: None,
}


@dataclasses.dataclass
class Map:
    width: int
    height: int
    rows: list[bytes]     # each row: width cell values (bank<<4 | row)

    def cell(self, x: int, y: int) -> int:
        return self.rows[y][x]


def parse(data: bytes) -> Map:
    w, h = struct.unpack_from(">HH", data, 0)
    offs = struct.unpack_from(f"<{h}I", data, 4)
    rows = []
    for y in range(h):
        p = offs[y]
        row = bytearray()
        while len(row) < w:
            b = data[p]
            p += 1
            c = b >> 5
            if c == 0:
                raise FormatError(f"MAP: zero run at row {y}")
            row += bytes((b & 0x1F,)) * c
        if len(row) != w:
            raise FormatError(f"MAP: row {y} overruns width")
        rows.append(bytes(row))
    return Map(w, h, rows)


def read(path) -> Map:
    return parse(read_file(path))


def encode_row(row: bytes) -> bytes:
    out = bytearray()
    i = 0
    while i < len(row):
        v = row[i]
        if v > 0x1F:
            raise FormatError("MAP: cell value > 31")
        j = i
        while j < len(row) and row[j] == v and j - i < 7:
            j += 1
        out.append(((j - i) << 5) | v)
        i = j
    return bytes(out)


def build(m: Map) -> bytes:
    enc = [encode_row(r) for r in m.rows]
    pos = 4 + 4 * m.height
    offs = []
    for e in enc:
        offs.append(pos)
        pos += len(e)
    return struct.pack(">HH", m.width, m.height) + struct.pack(f"<{m.height}I", *offs) + b"".join(enc)


def to_png(m: Map, path) -> None:
    """Debug rendering: one pixel per cell, arbitrary colours per value."""
    from PIL import Image
    im = Image.frombytes("P", (m.width, m.height), b"".join(m.rows))
    pal = []
    for v in range(256):
        pal += [(v * 97) % 256, (v * 57 + 80) % 256, (v * 151 + 40) % 256]
    im.putpalette(pal)
    im.save(path)
