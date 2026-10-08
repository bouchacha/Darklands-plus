"""ENEMYPAL.DAT -- 16-colour palette patches for battle sprites (combat).

Layout: 71 records of 53 bytes::

    u8  start          byte offset into the VGA palette (index = start / 3)
    u8  rgb[16][3]     6-bit VGA colours
    u8  unknown[4]

DARKLAND.ENM ``palette_start`` / ``palette_count`` select records here
(verified: every enemy type's range lies inside the 71 records).
"""
from __future__ import annotations

import dataclasses

from .util import F, FormatError, Spec, read_file

RGB = Spec("rgb", [F("r", "u8"), F("g", "u8"), F("b", "u8")])
PATCH = Spec("enemy_palette", [
    F("start", "u8", doc="palette byte offset (/3 = first colour index)", verified="doc"),
    F("colors", "spec", count=16, sub=RGB, doc="16 colours, 6-bit", verified="doc"),
    F("unknown_31", "u8", count=4, doc="unknown"),
])


@dataclasses.dataclass
class EnemyPal:
    palettes: list[dict]


def parse(data: bytes) -> EnemyPal:
    if len(data) % PATCH.size:
        raise FormatError("ENEMYPAL: size not a multiple of 53")
    return EnemyPal([PATCH.parse(data, i) for i in range(0, len(data), PATCH.size)])


def read(path) -> EnemyPal:
    return parse(read_file(path))


def build(p: EnemyPal) -> bytes:
    return b"".join(PATCH.build(x) for x in p.palettes)


def first_index(rec: dict) -> int:
    return rec["start"] // 3
