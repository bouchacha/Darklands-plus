"""DARKLAND.LOC -- map locations (overland map, also copied into saves).

Layout::

    u16 num_locations (414 = 0x19E in 483.07)
    location[num_locations]   (58 bytes each, see LOCATION)

The same record layout is used for the location table inside save games and
for the runtime copy ``LOCS.TMP``.  The first 92 records are the cities, in
DARKLAND.CTY order.

Offsets corrected against Merle's docs (which had the name at 0x25 and
overlapping words at 0x19/0x1B): the name is at 0x26 and is followed by
nothing; all other fields are listed below.  ``menu`` is a *card/menu id*
interpreted by the executable (cities 4, villages 133, castles 242, ...); the
id -> card table is in DARKLAND.EXE, not in any data file.
"""
from __future__ import annotations

import dataclasses
import struct

from .util import F, FormatError, Spec, read_file

LOCATION_ICONS = {
    0x00: "city", 0x01: "castle (lord)", 0x02: "castle (raubritter)", 0x03: "monastery",
    0x04: "Teufelstein-like (tomb/pagan altar look)", 0x05: "cave", 0x06: "mines", 0x08: "village",
    0x09: "ruined village", 0x0A: "village (alt, unused?)", 0x0D: "tomb", 0x0F: "dragon's lair",
    0x10: "spring", 0x11: "lake", 0x12: "shrine", 0x13: "cave (alt)", 0x14: "pagan altar",
    0x15: "witch sabbat", 0x16: "Templar castle", 0x17: "Hochkonig (Baphomet castle)",
    0x18: "alpine cave", 0x19: "lady of the lake", 0x1A: "ruined raubritter castle",
}

LOCATION = Spec("location", [
    F("icon", "u16", doc="map icon / location kind (see LOCATION_ICONS, from Merle)", verified="doc"),
    F("unknown_02", "u16", doc="0 for cities; 8..14 otherwise"),
    F("x", "u16", doc="map x (0..326)", verified="structural"),
    F("y", "u16", doc="map y (0..930; one record has 931 = off-map, Merle: Marienburg is patched at new game)", verified="structural"),
    F("unknown_08", "u16", doc="5 for cities, 1/9/10 otherwise (raubritter strength?)"),
    F("unknown_0a", "u16", doc="0..5, 99 for pagan altars"),
    F("menu", "u16", doc="card/menu id shown on entering (EXE table)", verified="doc"),
    F("unknown_0e", "u16", doc="0x62, 0x92 for raubritter castles"),
    F("unknown_10", "u8", doc="always 0xFF in DARKLAND.LOC"),
    F("city_size", "u8", doc="3..8 for cities, 1 otherwise", verified="cross-ref"),
    F("local_rep", "s16", doc="local reputation (0 in DARKLAND.LOC, set in saves)", verified="doc"),
    F("unknown_14", "u8", doc="0 = active; 0x04 destroyed, 0x20/0x24 seen in saves (Merle)"),
    F("unknown_15", "u8", count=3, doc="always 0x19 0x19 0x19"),
    F("inn_cache_idx", "u16", doc="0xFFFF, or index into the save's cache_offsets", verified="doc"),
    F("unknown_1a", "u16", doc="always 0"),
    F("unknown_1c", "u16", doc="0, except 0xC0 for one location"),
    F("unknown_1e", "bytes", 8, doc="always zero"),
    F("name", "str", 20, doc="name (DL charset)", verified="structural"),
])


@dataclasses.dataclass
class Loc:
    locations: list[dict]


def parse_records(data: bytes, pos: int, n: int) -> tuple[list[dict], int]:
    out = []
    for _ in range(n):
        out.append(LOCATION.parse(data, pos))
        pos += LOCATION.size
    return out, pos


def parse(data: bytes) -> Loc:
    n, = struct.unpack_from("<H", data, 0)
    locs, pos = parse_records(data, 2, n)
    if pos != len(data):
        raise FormatError(f"LOC: {len(data) - pos} trailing bytes")
    return Loc(locs)


def read(path) -> Loc:
    return parse(read_file(path))


def build(loc: Loc) -> bytes:
    return struct.pack("<H", len(loc.locations)) + b"".join(LOCATION.build(r) for r in loc.locations)
