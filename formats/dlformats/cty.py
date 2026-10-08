"""DARKLAND.CTY -- city definitions (overland map + town menus).

City *i* is location *i* in DARKLAND.LOC (same coordinates and size; the
only difference is Strassburg, x = 50 here vs 49 in LOC; LOC names are
abbreviated to fit 20 bytes).

Layout::

    u8 num_cities (92)
    city[num_cities]   (0x26E = 622 bytes each, see CITY)

All 18 string fields are 32-byte buffers whose bytes after the NUL contain
leftover memory from the original editor (x86 code / other strings); they are
preserved in ``<field>__pad`` so the file round-trips exactly.  An empty
building name is stored as a single space.

``contents`` bit meanings were re-derived here by correlating each bit with
the presence of the corresponding name string / dock destinations over all 92
cities (Merle's docs had them in the wrong order):

=====  ===================  ==========================================
bit    meaning              agreement with names (of 92)
=====  ===================  ==========================================
0      unknown (91 set)     --
1      town hall            92
2      fortress             91
3      unknown (all set)    --
4      cathedral            92
5      'unknown_zeughaus'   89 (name at 0x1CE: Zeughaus/-turm/-tor)
6      slums                91
7      kloster              92
8      university           91
9      pawnshop (Leihhaus)  91
10     'unknown_munz'       92 (name at 0x18E: Munzenplatz)
11     docks                91 (agrees with non-empty dock list)
12-15  never set            --
=====  ===================  ==========================================
"""
from __future__ import annotations

import dataclasses

from .util import F, FormatError, Spec, read_file

CONTENT_BITS = {1: "town_hall", 2: "fortress", 4: "cathedral", 5: "unknown_zeughaus", 6: "slums",
                7: "kloster", 8: "university", 9: "pawnshop", 10: "unknown_munz", 11: "docks"}

CITY_DATA = Spec("city_data", [
    F("city_size", "u16", doc="3..8", verified="cross-ref"),
    F("entry_x", "u16", doc="map coordinates of the city", verified="cross-ref"),
    F("entry_y", "u16", doc="", verified="cross-ref"),
    F("exit_x", "u16", doc="party position after leaving", verified="doc"),
    F("exit_y", "u16", doc="", verified="doc"),
    F("dock_destinations", "u16", count=4, doc="city indices reachable by ship, 0xFFFF none", verified="cross-ref"),
    F("coast", "u16", doc="0xFFFF inland, 0/1 (Merle: side of river; tidal)", verified="doc"),
    F("unknown_14", "u16", doc="always 4"),
    F("pseudo_ordinal", "u16", doc="0..91 with one gap and one duplicate"),
    F("city_type", "u16", doc="0 free city, 1 ruled, 2 capital (selects $CITYE00 card text, Merle)", verified="doc"),
    F("unknown_1a", "u16", doc="0..3"),
    F("unknown_1c", "u16", doc="always 0"),
    F("contents", "u16", doc="building bitmask, see CONTENT_BITS", verified="cross-ref"),
    F("unknown_20", "u16", doc="always 0"),
    F("qual_blacksmith", "u8", doc="shop quality, 0 = no shop (relative, Merle)", verified="doc"),
    F("qual_merchant", "u8", doc="", verified="doc"),
    F("qual_swordsmith", "u8", doc="", verified="doc"),
    F("qual_armorer", "u8", doc="", verified="doc"),
    F("qual_unknown_26", "u8", doc="unknown shop quality"),
    F("qual_bowyer", "u8", doc="", verified="doc"),
    F("qual_tinker", "u8", doc="", verified="doc"),
    F("qual_unknown_29", "u8", doc="unknown shop quality"),
    F("qual_clothing", "u8", doc="", verified="doc"),
    F("unknown_2b", "u8", doc="always 0"),
    F("unknown_2c", "u8", doc="1..255"),
    F("unknown_2d", "u8", doc="0 or 1"),
])

_NAMES = [("leader_name", "title of the city leader"), ("ruler_name", "overlord"),
          ("unknown_famous", "always 'Famous Place'"), ("polit_name", "town square"),
          ("town_hall_name", ""), ("fortress_name", ""), ("cathedral_name", ""), ("church_name", ""),
          ("market_name", ""), ("unknown_munz_name", "Munzenplatz (6 cities)"), ("slum_name", ""),
          ("unknown_zeughaus_name", "Zeughaus / -turm / -tor"), ("pawnshop_name", "Leihhaus"),
          ("kloster_name", ""), ("inn_name", ""), ("university_name", "")]

CITY = Spec("city", [
    F("short_name", "str", 32, doc="short name", verified="structural"),
    F("full_name", "str", 32, doc="full name", verified="structural"),
    F("data", "spec", sub=CITY_DATA, doc="see city_data"),
] + [F(n, "str", 32, doc=d, verified="structural") for n, d in _NAMES])


@dataclasses.dataclass
class Cty:
    cities: list[dict]


def contents_flags(city: dict) -> dict[str, bool]:
    v = city["data"]["contents"]
    return {name: bool(v >> bit & 1) for bit, name in CONTENT_BITS.items()}


def parse(data: bytes) -> Cty:
    n = data[0]
    if 1 + n * CITY.size != len(data):
        raise FormatError("CTY: size mismatch")
    return Cty([CITY.parse(data, 1 + i * CITY.size) for i in range(n)])


def read(path) -> Cty:
    return parse(read_file(path))


def build(c: Cty) -> bytes:
    return bytes((len(c.cities),)) + b"".join(CITY.build(x) for x in c.cities)
