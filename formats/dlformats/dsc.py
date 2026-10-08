"""DARKLAND.DSC -- one-line city descriptions (``$PlaceDesc`` in cards).

Layout::

    u8 header (0x5E = 94 in 483.07 -- NOT the record count)
    char description[92][80]       (record count = (filesize - 1) / 80)

The header byte does not match the 92 records actually present (the game
presumably ignores it, or uses a hard-coded count).  The reader therefore
derives the count from the file size and keeps the header byte verbatim.
Order matches DARKLAND.CTY (description 0 'a small North Sea port controlled
by Dutch nobles' = Groningen, city 0).
"""
from __future__ import annotations

import dataclasses

from .util import F, FormatError, Spec, read_file

DESC = Spec("city_description", [F("description", "str", 80, doc="description text", verified="structural")])


@dataclasses.dataclass
class Dsc:
    header: int
    descriptions: list[dict]


def parse(data: bytes) -> Dsc:
    if (len(data) - 1) % DESC.size:
        raise FormatError("DSC: size is not 1 + 80*n")
    n = (len(data) - 1) // DESC.size
    return Dsc(data[0], [DESC.parse(data, 1 + i * DESC.size) for i in range(n)])


def read(path) -> Dsc:
    return parse(read_file(path))


def build(d: Dsc) -> bytes:
    return bytes((d.header,)) + b"".join(DESC.build(x) for x in d.descriptions)
