"""IMAPS.CAT entries (``ICITY.000``, ``IMINEGEN.0xx``, ``IWILDGEN.1xx`` ...) --
tactical-combat / dungeon maps (combat module).

Each entry is an LZ stream (:mod:`dlformats.lzss`) that decompresses to
exactly 13308 bytes (all 143 entries).  Partial layout (inferred from
rendering ICITY.000, where city blocks become visible at a 40-cell stride):

    0x0000  u32 cell[40][40]     0x80000000 appears to mark "outside/void";
                                 other values look like tile/wall codes
    0x1900  6908 bytes            unknown tables (contain runs of small
                                 increasing values -- probably object/spawn
                                 or tile-graphics lists)

LCASTLE's ``LC_*`` maps (no extension) decompress to 13310 bytes and look
similar (two extra bytes).  The cell bit layout is NOT understood; readers
keep everything raw so files round-trip exactly.
"""
from __future__ import annotations

import dataclasses
import struct

from . import lzss
from .util import FormatError

GRID_W = GRID_H = 40


@dataclasses.dataclass
class IMap:
    cells: list[int]          # 1600 u32 values, row-major 40x40
    tail: bytes = dataclasses.field(repr=False)

    def cell(self, x: int, y: int) -> int:
        return self.cells[y * GRID_W + x]


def parse_raw(o: bytes) -> IMap:
    if len(o) < GRID_W * GRID_H * 4:
        raise FormatError("IMAP: too short")
    cells = list(struct.unpack_from(f"<{GRID_W * GRID_H}I", o, 0))
    return IMap(cells, o[GRID_W * GRID_H * 4:])


def build_raw(m: IMap) -> bytes:
    return struct.pack(f"<{GRID_W * GRID_H}I", *m.cells) + m.tail


def parse(compressed: bytes) -> IMap:
    return parse_raw(lzss.decompress(compressed))


def build(m: IMap) -> bytes:
    return lzss.compress(build_raw(m))


def to_png(m: IMap, path, scale: int = 8) -> None:
    """Debug view: void cells black, others coloured by low 16 bits."""
    from PIL import Image
    im = Image.new("RGB", (GRID_W, GRID_H))
    px = im.load()
    for y in range(GRID_H):
        for x in range(GRID_W):
            v = m.cell(x, y)
            if v == 0x80000000:
                px[x, y] = (0, 0, 0)
            elif v == 0:
                px[x, y] = (255, 255, 255)
            else:
                px[x, y] = ((v * 53) & 255, (v >> 8) * 40 & 255, 128 + ((v >> 4) & 127))
    im.resize((GRID_W * scale, GRID_H * scale), Image.NEAREST).save(path)
