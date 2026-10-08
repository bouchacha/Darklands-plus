"""DARKLAND.LST -- item definitions, saint names and formula names.

Layout::

    u8 num_item_slots (200)   u8 num_saints (136)   u8 num_formulae (66)
    item_definition[num_item_slots]          (46 bytes each, see ITEM)
    char* saint_full_names[num_saints]       (NUL-terminated)
    char* saint_short_names[num_saints]
    char* formula_full_names[num_formulae]
    char* formula_short_names[num_formulae]

The index of an item in the array is its *item code*, used everywhere else
(saves, ENM, ALC, quests).  The ``type`` word is a second id ("item type")
that the equipment slots and ENM use alongside the code (verified: e.g. ENM
Sergeant has vital armour code 41 'V:Scale' and type 72, which is exactly
LST[41].type).  Empty slots (codes 172..199 in 483.07) are all zero.

Flag observations (172 defined items): never set: 0x21 b7, 0x22 b3/b6,
0x23 b1/b6/b7, 0x24 b2..b6.  0x21 b0 = Torch, Candle, Rope, Iron Spike,
Lantern, Harp, Flute; 0x21 b1 = Clock, Grappling Hook, Lockpick, Harp, Flute
(Merle: pawnshop goods).  In byte 0x24 exactly one of bits 0/1/7 is set for
every item except non-clothing armour and the Superb/Fast Horse: bit 7 =
clothing, quest items, relics; bit 0 = one-handed weapons, shields, light/
tools, some potions; bit 1 = two-handed weapons, bows, missile weapons,
ammunition, components, other potions.  Meaning of 0/1 unknown.

Saints and formulae are referenced by their position in these lists
(DARKLAND.SNT / DARKLAND.ALC are in the same order).
"""
from __future__ import annotations

import dataclasses

from .util import F, FormatError, Spec, join_cstrings, read_file, split_cstrings

FLAG_BITS = [
    # byte 0x20
    ["is_edged", "is_impact", "is_polearm", "is_flail", "is_thrown", "is_bow", "is_metal_armor", "is_shield"],
    # byte 0x21
    ["flag21_0", "flag21_1", "is_component", "is_potion", "is_relic", "is_horse", "is_quest_1", "flag21_7"],
    # byte 0x22
    ["is_lockpicks", "is_light", "is_arrow", "flag22_3", "is_quarrel", "is_ball", "flag22_6", "is_quest_2"],
    # byte 0x23 -- Merle had bits 4..6 shifted: is_music is bit 5 (Harp, Flute), bit 4 is the
    # "mine chest?" group (Great Hammer, Maul, Military Hammer, V/L:Leather, 10 minerals)
    ["is_throw_potion", "flag23_1", "is_nonmetal_armor", "is_missile_weapon", "flag23_4", "is_music", "flag23_6", "flag23_7"],
    # byte 0x24
    ["flag24_0", "flag24_1", "flag24_2", "flag24_3", "flag24_4", "flag24_5", "flag24_6", "flag24_7"],
]

ITEM = Spec("item_definition", [
    F("name", "str", 20, doc="full name", verified="structural"),
    F("short_name", "str", 10, doc="short name", verified="structural"),
    F("type", "u16", doc="item type id (used with the code in equipment slots/ENM)", verified="cross-ref"),
    F("flags", "u8", count=5, doc="5 flag bytes, see FLAG_BITS / docs (bit names from Merle, re-checked against item lists)", verified="cross-ref"),
    F("weight", "u8", doc="weight", verified="doc"),
    F("quality", "u8", doc="default quality (25 normal, 99 quest items)", verified="doc"),
    F("rarity", "u8", doc="rarity 0..12", verified="doc"),
    F("unknown_28", "u16", doc="non-zero only for relics (6..80)"),
    F("unknown_2a", "u16", doc="non-zero only for relics and 'Residency Permit'"),
    F("value", "u16", doc="base value", verified="doc"),
])


@dataclasses.dataclass
class Lst:
    items: list[dict]
    saint_full_names: list[str]
    saint_short_names: list[str]
    formula_full_names: list[str]
    formula_short_names: list[str]


def item_flags(item: dict) -> dict[str, bool]:
    out = {}
    for byte, names in zip(item["flags"], FLAG_BITS):
        for bit, nm in enumerate(names):
            out[nm] = bool(byte >> bit & 1)
    return out


def parse(data: bytes) -> Lst:
    ni, ns, nf = data[0], data[1], data[2]
    pos = 3
    items = []
    for _ in range(ni):
        items.append(ITEM.parse(data, pos))
        pos += ITEM.size
    sf, pos = split_cstrings(data, ns, pos)
    ss, pos = split_cstrings(data, ns, pos)
    ff, pos = split_cstrings(data, nf, pos)
    fs, pos = split_cstrings(data, nf, pos)
    if pos != len(data):
        raise FormatError(f"LST: {len(data) - pos} trailing bytes")
    return Lst(items, sf, ss, ff, fs)


def read(path) -> Lst:
    return parse(read_file(path))


def build(lst: Lst) -> bytes:
    if len(lst.saint_full_names) != len(lst.saint_short_names):
        raise FormatError("LST: saint name lists differ in length")
    if len(lst.formula_full_names) != len(lst.formula_short_names):
        raise FormatError("LST: formula name lists differ in length")
    out = bytearray((len(lst.items), len(lst.saint_full_names), len(lst.formula_full_names)))
    for it in lst.items:
        out += ITEM.build(it)
    out += join_cstrings(lst.saint_full_names) + join_cstrings(lst.saint_short_names)
    out += join_cstrings(lst.formula_full_names) + join_cstrings(lst.formula_short_names)
    return bytes(out)
