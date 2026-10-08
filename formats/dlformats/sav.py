"""Save games (``SAVES/DKSAVE*.SAV``, ``SAVES/DEFAULT``) and the runtime temp
files ``CHARACTR.TMP`` / ``CACHE.TMP`` that share parts of the layout.

Only ``SAVES/DEFAULT`` (the new-game template, 26349 bytes) ships with the
GOG copy, so every claim below is checked against that one file only; the
field names come from Merle's docs unless marked otherwise.  The companion
``.BSV`` battle/dungeon save is not present and not handled.

Top-level layout::

    0x000  header (0xEF bytes, SAVE_HEADER)
    0x0EF  u16 num_curr_characters, u16 num_characters
    0x0F3  s16 party_char_indices[5]       (-1 = empty)
    0x0FD  char party_images[5][4]         ('F60','A00',...; '0\\0' '0\\0' for empty)
    0x111  person_colors party_colors[5]   (8 RGB triplets each, 6-bit)
    0x189  character characters[num_characters]   (0x22A bytes each)
           u16 num_events,     event events[num_events]       (0x30 each)
           u16 num_locations,  location locations[...]        (DARKLAND.LOC record)
           u8  max_cache_slot (0x63), u8 num_caches, u16 cache_offsets[98]
           cache data (rest of file; kept raw)

``DEFAULT`` holds only 405 locations (DARKLAND.LOC has 414) -- it was made
with an older location list; the counts are therefore read from the file.

Corrections to Merle's docs found here: the event record has its words at
even offsets 0x1A..0x2E (the doc lists odd offsets 0x21/0x23/0x25), the save
header's "constant zero" block at 0x2C is not zero in DEFAULT, and the byte at
0x63 is 0x64 (doc: constant 0x19) -- they are kept as unknown raw bytes.
"""
from __future__ import annotations

import dataclasses
import struct

from .enm import ATTRS, SKILLS
from .loc import LOCATION
from .util import F, FormatError, Spec, read_file

DATE = Spec("date", [F("hour", "u16"), F("day", "u16"), F("month", "u16", doc="0-based"), F("year", "u16")])
DATE_YMD = Spec("date_ymdh", [F("year", "u16"), F("month", "u16", doc="0-based"), F("day", "u16"), F("hour", "u16")])
MONEY = Spec("money", [F("florins", "u16"), F("groschen", "u16"), F("pfennigs", "u16")])
RGB = Spec("rgb", [F("r", "u8"), F("g", "u8"), F("b", "u8")])
PERSON_COLORS = Spec("person_colors", [F("colors", "spec", count=8, sub=RGB,
                                         doc="1 hi/lo, 2 hi/med/lo, 3 hi/med/lo (Merle)")])

ITEM = Spec("item", [
    F("code", "u16", doc="LST item code", verified="cross-ref"),
    F("type", "u8", doc="LST item type (== LST[code].type)", verified="cross-ref"),
    F("quality", "u8", doc="quality", verified="doc"),
    F("quantity", "u8", doc="quantity", verified="doc"),
    F("weight", "u8", doc="weight", verified="doc"),
])

SAVE_HEADER = Spec("save_header", [
    F("curr_location_name", "str", 12, doc="name of current location", verified="structural"),
    F("unknown_0c", "bytes", 9),
    F("label", "str", 23, doc="save game label", verified="structural"),
    F("unknown_2c", "bytes", 0x37, doc="Merle: 18 zero bytes + 37 unknown; not zero in DEFAULT"),
    F("unknown_63", "u8", doc="Merle: constant 0x19; 0x64 in DEFAULT"),
    F("city_contents_seed", "u16", doc="seed for shop/saint/formula availability (Merle)", verified="doc"),
    F("unknown_66", "u16", doc="7"),
    F("date", "spec", sub=DATE_YMD, doc="current date: year, month(0-based), day, hour", verified="structural"),
    F("money", "spec", sub=MONEY, doc="cash", verified="doc"),
    F("unknown_76", "bytes", 4),
    F("reputation", "s16", doc="global reputation", verified="doc"),
    F("curr_location", "s16", doc="index into locations, -1 = wilderness", verified="doc"),
    F("x", "u16", doc="map x", verified="doc"),
    F("y", "u16", doc="map y", verified="doc"),
    F("curr_menu", "u16", doc="current card/menu id", verified="doc"),
    F("unknown_84", "bytes", 6),
    F("prev_menu", "u16", doc="previous card/menu id", verified="doc"),
    F("bank_notes", "u16", doc="bank notes (florins)", verified="doc"),
    F("unknown_8e", "bytes", 4),
    F("philosopher_stone", "u16", doc="philosopher's stone quality", verified="doc"),
    F("unknown_94", "bytes", 7),
    F("party_order", "u8", count=5, doc="marching order (0-based)", verified="doc"),
    F("unknown_a0", "u8"),
    F("party_leader", "u8", doc="leader index (0-based)", verified="doc"),
    F("unknown_a2", "bytes", 3),
    F("unknown_a5", "bytes", 74, doc="zero in DEFAULT"),
])
assert SAVE_HEADER.size == 0xEF

PARTY_IMAGE = Spec("party_image", [F("image", "str", 4, doc="image group")])

PARTY = Spec("party", [
    F("party_char_indices", "s16", count=5, doc="character index per slot, -1 empty", verified="doc"),
    F("party_images", "spec", count=5, sub=PARTY_IMAGE, doc="image group per slot", verified="structural"),
])

CHARACTER = Spec("character", [
    F("unknown_00", "bytes", 18),
    F("age", "u16", doc="age", verified="doc"),
    F("unknown_14", "u8"),
    F("shield", "u8", doc="heraldic shield letter 'A'..'O' (PICS/SHIELD?.PIC)", verified="doc"),
    F("unknown_16", "bytes", 12),
    F("equip_missile_type", "u8", doc="LST type of equipped missile weapon (0xFF none)", verified="cross-ref"),
    F("unknown_23", "bytes", 2),
    F("full_name", "str", 25, doc="full name", verified="structural"),
    F("short_name", "str", 11, doc="nickname", verified="structural"),
    F("unknown_49", "bytes", 2),
    F("equip_vital_type", "u8", doc="LST type of vital armour", verified="cross-ref"),
    F("equip_leg_type", "u8", doc="LST type of leg armour", verified="cross-ref"),
    F("unknown_4d", "bytes", 2),
    F("equip_vital_q", "u8", doc="quality", verified="doc"),
    F("equip_leg_q", "u8", doc="quality", verified="doc"),
    F("equip_weapon_type", "u8", doc="LST type of weapon", verified="cross-ref"),
    F("unknown_52", "bytes", 6),
    F("equip_weapon_q", "u8", doc="quality", verified="doc"),
    F("unknown_59", "u8"),
    F("equip_missile_q", "u8", doc="quality", verified="doc"),
    F("equip_shield_q", "u8", doc="quality", verified="doc"),
    F("equip_shield_type", "u8", doc="LST type of shield (0xFF none)", verified="doc"),
    F("curr_attrs", "spec", sub=ATTRS, doc="current attributes", verified="doc"),
    F("max_attrs", "spec", sub=ATTRS, doc="maximum attributes", verified="doc"),
    F("skills", "spec", sub=SKILLS, doc="skills", verified="doc"),
    F("num_items", "u16", doc="items in use", verified="structural"),
    F("saints_known", "bytes", 20, doc="bit per saint (LST order)", verified="doc"),
    F("formulae_known", "bytes", 22, doc="byte per potion, 3 bits (Merle: q45/q35/q25)", verified="doc"),
    F("items", "spec", count=64, sub=ITEM, doc="inventory (unused slots zero)", verified="structural"),
])
assert CHARACTER.size == 0x22A

EVENT = Spec("event", [
    F("unknown_00", "s16", doc="RB quests: giver id (Merle)"),
    F("create_date", "spec", sub=DATE, doc="creation date", verified="doc"),
    F("date_0a", "spec", sub=DATE, doc="== create_date (Merle)"),
    F("expire_date", "spec", sub=DATE, doc="expiry date", verified="doc"),
    F("unknown_1a", "s16", doc="quests: location given / -2"),
    F("unknown_1c", "s16", doc="quests: target location"),
    F("unknown_1e", "s16", doc="quests: return location"),
    F("unknown_20", "s16"),
    F("unknown_22", "s16", doc="state? (RB quest 8/0x24/7, Merle)"),
    F("unknown_24", "s16"),
    F("unknown_26", "s16"),
    F("unknown_28", "s16"),
    F("unknown_2a", "s16"),
    F("unknown_2c", "s16"),
    F("unknown_2e", "s16", doc="quest item code (Merle)"),
])
assert EVENT.size == 0x30


@dataclasses.dataclass
class Save:
    header: dict
    num_curr_characters: int
    party: dict
    party_colors: list[dict]
    characters: list[dict]
    events: list[dict]
    locations: list[dict]
    max_cache_slot: int
    num_caches: int
    cache_offsets: list[int]
    cache_data: bytes      # raw remainder (caches), see parse_caches()


def parse(data: bytes) -> Save:
    hdr = SAVE_HEADER.parse(data, 0)
    ncur, nch = struct.unpack_from("<HH", data, 0xEF)
    party = PARTY.parse(data, 0xF3)
    colors = [PERSON_COLORS.parse(data, 0x111 + i * PERSON_COLORS.size) for i in range(5)]
    pos = 0x189
    chars = []
    for _ in range(nch):
        chars.append(CHARACTER.parse(data, pos))
        pos += CHARACTER.size
    ne, = struct.unpack_from("<H", data, pos)
    pos += 2
    events = []
    for _ in range(ne):
        events.append(EVENT.parse(data, pos))
        pos += EVENT.size
    nl, = struct.unpack_from("<H", data, pos)
    pos += 2
    locs = []
    for _ in range(nl):
        locs.append(LOCATION.parse(data, pos))
        pos += LOCATION.size
    if pos + 2 + 196 > len(data):
        raise FormatError("SAV: truncated cache table")
    mx, nc = data[pos], data[pos + 1]
    offs = list(struct.unpack_from("<98H", data, pos + 2))
    pos += 2 + 196
    return Save(hdr, ncur, party, colors, chars, events, locs, mx, nc, offs, data[pos:])


def read(path) -> Save:
    return parse(read_file(path))


def build(s: Save) -> bytes:
    out = bytearray(SAVE_HEADER.build(s.header))
    out += struct.pack("<HH", s.num_curr_characters, len(s.characters))
    out += PARTY.build(s.party)
    out += b"".join(PERSON_COLORS.build(c) for c in s.party_colors)
    out += b"".join(CHARACTER.build(c) for c in s.characters)
    out += struct.pack("<H", len(s.events)) + b"".join(EVENT.build(e) for e in s.events)
    out += struct.pack("<H", len(s.locations)) + b"".join(LOCATION.build(r) for r in s.locations)
    out += bytes((s.max_cache_slot, s.num_caches)) + struct.pack("<98H", *s.cache_offsets)
    out += s.cache_data
    return bytes(out)


def parse_caches(num_caches: int, offsets: list[int], cache_data: bytes, base: int = 200) -> list[dict]:
    """Best-effort decoding of inn item caches (``u8 n; {u16 code; u8 q; u8 qty}[n]``).

    Merle: offsets are relative to ``max_cache_slot`` (i.e. the start of the
    cache table, 200 bytes before ``cache_data``).  Unverified: no shipped
    file contains a cache.
    """
    out = []
    for i in range(num_caches):
        p = offsets[i] - base
        if not 0 <= p < len(cache_data):
            out.append({"error": "offset out of range", "offset": offsets[i]})
            continue
        n = cache_data[p]
        items = [dict(zip(("code", "quality", "quantity"), struct.unpack_from("<HBB", cache_data, p + 1 + 4 * k)))
                 for k in range(n)]
        out.append({"offset": offsets[i], "items": items})
    return out


# ---------------------------------------------------------------------------
# CHARACTR.TMP / CACHE.TMP (runtime files written by the game)
# ---------------------------------------------------------------------------

@dataclasses.dataclass
class CharacterTmp:
    """CHARACTR.TMP = u16 num_characters + PARTY block + characters.

    (i.e. the save layout from 0xF1 without the colour table.)"""
    party: dict
    characters: list[dict]


def parse_charactr_tmp(data: bytes) -> CharacterTmp:
    n, = struct.unpack_from("<H", data, 0)
    party = PARTY.parse(data, 2)
    pos = 2 + PARTY.size
    if pos + n * CHARACTER.size != len(data):
        raise FormatError("CHARACTR.TMP: size mismatch")
    chars = [CHARACTER.parse(data, pos + i * CHARACTER.size) for i in range(n)]
    return CharacterTmp(party, chars)


def build_charactr_tmp(c: CharacterTmp) -> bytes:
    return struct.pack("<H", len(c.characters)) + PARTY.build(c.party) + b"".join(CHARACTER.build(x) for x in c.characters)


@dataclasses.dataclass
class CacheTmp:
    """CACHE.TMP = the save's cache section (max slot, count, 98 offsets, data)."""
    max_cache_slot: int
    num_caches: int
    cache_offsets: list[int]
    cache_data: bytes


def parse_cache_tmp(data: bytes) -> CacheTmp:
    return CacheTmp(data[0], data[1], list(struct.unpack_from("<98H", data, 2)), data[198:])


def build_cache_tmp(c: CacheTmp) -> bytes:
    return bytes((c.max_cache_slot, c.num_caches)) + struct.pack("<98H", *c.cache_offsets) + c.cache_data
