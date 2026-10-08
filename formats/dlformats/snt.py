"""DARKLAND.SNT -- saint descriptions (shown in the saints screen).

Layout::

    u8 num_saints (136)
    char description[num_saints][360]   (NUL-padded)

Order matches ``saint_full_names`` in DARKLAND.LST.  The *effects* of the
saints (which attribute/skill they modify, by how much, their divine-favour
cost, prerequisites) are NOT in this file -- only flavour text.  They must
live in DARKLAND.EXE.
"""
from __future__ import annotations

import dataclasses

from .util import F, FormatError, Spec, read_file

SAINT = Spec("saint_description", [F("description", "str", 360, doc="description text", verified="structural")])


@dataclasses.dataclass
class Snt:
    descriptions: list[dict]


def parse(data: bytes) -> Snt:
    n = data[0]
    if 1 + n * SAINT.size != len(data):
        raise FormatError("SNT: size mismatch")
    return Snt([SAINT.parse(data, 1 + i * SAINT.size) for i in range(n)])


def read(path) -> Snt:
    return parse(read_file(path))


def build(s: Snt) -> bytes:
    return bytes((len(s.descriptions),)) + b"".join(SAINT.build(d) for d in s.descriptions)
