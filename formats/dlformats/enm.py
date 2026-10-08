"""DARKLAND.ENM -- enemy types and named enemies (combat / encounters).

Layout::

    enemy_type[71]   (204 bytes each, see ENEMY_TYPE)
    enemy[82]        (24 bytes each, see ENEMY), starting at 0x3894

The counts 71 and 82 are not stored in the file (they must be hard-coded in
the executable); the reader derives the split from ``71 * 204``.

Cross-references verified against DARKLAND.LST / ENEMYPAL.DAT in this work
(new compared with Merle's docs):

* the doc's skill offset was off by one: attributes are 0x14..0x1A, skills
  0x1B..0x2D (19 bytes), then the unknown bytes 0x2E..0x31;
* armour/shield/weapon fields come in (item *type*, item *code*) pairs: e.g.
  Sergeant1 0x92/0x94 = 72/41 = LST[41] 'V:Scale' (type 72), 0x93/0x95 =
  77/46 'L:Padded', 0x97/0x98 = 96/55 'Medium Shield'.  For animals and
  monsters the type bytes hold values no LST item uses (e.g. 0x55 'natural
  armour', 0x2C 'natural weapon') -- the meaning of those types must come
  from tables in the executable;
* 0x9A..0x9F is a missile kit: weapon type, weapon code, quality, ammo type,
  ammo code, ammo count (Sergeant1: Short Bow (28) + 12 Arrows (33));
* 0xA0..0xA5 are 6 candidate weapon *types* and 0xA6..0xAB the matching
  weapon *codes* (0xFF = none);
* the only exception to the type/code pairing: Skeleton1/2 pair natural
  weapon type 48 with item code 8 (Battle Axe) -- presumably the dropped item;
* ``palette_start``/``palette_count`` index ENEMYPAL.DAT; all lie within its
  71 records except Baphomet (start 71, count 1 = one past the end);
* image groups M08 (Undead), M12 (DemonKt) and M99 (Baphomet) have no IMC
  sprites in any catalog and no TACANIM.DB entry -- the executable must
  substitute other sprites (or never show them in tactical combat).

Fields still unknown are kept as raw bytes.
"""
from __future__ import annotations

import dataclasses

from .util import F, FormatError, Spec, read_file

ATTRS = Spec("attribute_set", [
    F(n, "u8", doc=d, verified="doc") for n, d in
    [("end", "Endurance"), ("str", "Strength"), ("agl", "Agility"), ("per", "Perception"),
     ("int", "Intelligence"), ("chr", "Charisma"), ("df", "Divine Favor")]])

SKILLS = Spec("skill_set", [
    F(n, "u8", doc=d, verified="doc") for n, d in
    [("wEdg", "Edged weapons"), ("wImp", "Impact weapons"), ("wFll", "Flails"), ("wPol", "Polearms"),
     ("wThr", "Thrown weapons"), ("wBow", "Bows"), ("wMsl", "Missile devices"), ("alch", "Alchemy"),
     ("relg", "Religion"), ("virt", "Virtue"), ("spkC", "Speak Common"), ("spkL", "Speak Latin"),
     ("r_w", "Read & Write"), ("heal", "Healing"), ("artf", "Artifice"), ("stlh", "Stealth"),
     ("strw", "Streetwise"), ("ride", "Riding"), ("wdws", "Woodwise")]])

ENEMY_TYPE = Spec("enemy_type", [
    F("image_group", "str", 4, doc="sprite group, e.g. 'E00' (E00C.CAT / M00C.CAT, TACANIM.DB)", verified="cross-ref"),
    F("type_name", "str", 10, doc="internal name, e.g. 'Sergeant1'", verified="structural"),
    F("num_variants", "u8", doc="number of variants for the first record of a group, 0xFF for the others", verified="structural"),
    F("palette_count", "u8", doc="number of ENEMYPAL.DAT palettes usable", verified="cross-ref"),
    F("unknown_10", "u8", doc="1..3"),
    F("palette_start", "u8", doc="first ENEMYPAL.DAT palette", verified="cross-ref"),
    F("unknown_12", "u16", doc="0..2"),
    F("attrs", "spec", sub=ATTRS, doc="attributes", verified="doc"),
    F("skills", "spec", sub=SKILLS, doc="skills", verified="doc"),
    F("unknown_2e", "u8", doc="8..20"),
    F("unknown_2f", "u8", doc="always 0"),
    F("unknown_30", "u8", doc="0, 0x01, 0x20, 0x2B, 0x40 (skeletons/gnomes/vulcan/hellhound/dragons)"),
    F("unknown_31", "u8", doc="0..2"),
    F("potion_kit", "bytes", 0x42, doc="22 triplets; non-zero only for some types (Merle: potion odds/quality)"),
    F("component_kit", "bytes", 0x1E, doc="10 triplets (Merle: component odds?)"),
    F("vital_armor_type", "u8", doc="LST item type of vital armour (or natural-armour type)", verified="cross-ref"),
    F("limb_armor_type", "u8", doc="LST item type of limb armour", verified="cross-ref"),
    F("vital_armor_code", "u8", doc="LST item code of vital armour, 0xFF none", verified="cross-ref"),
    F("limb_armor_code", "u8", doc="LST item code of limb armour, 0xFF none", verified="cross-ref"),
    F("armor_quality", "u8", doc="armour quality (effective AC for unarmoured creatures)", verified="doc"),
    F("shield_type", "u8", doc="LST item type of shield, 0xFF none", verified="cross-ref"),
    F("shield_code", "u8", doc="LST item code of shield, 0xFF none", verified="cross-ref"),
    F("shield_quality", "u8", doc="shield quality", verified="doc"),
    F("missile_type", "u8", doc="LST type of missile weapon, 0xFF none", verified="cross-ref"),
    F("missile_code", "u8", doc="LST code of missile weapon", verified="cross-ref"),
    F("missile_quality", "u8", doc="quality of missile weapon"),
    F("ammo_type", "u8", doc="LST type of ammunition", verified="cross-ref"),
    F("ammo_code", "u8", doc="LST code of ammunition", verified="cross-ref"),
    F("ammo_count", "u8", doc="amount of ammunition (6..12 for sergeants)"),
    F("weapon_types", "u8", count=6, doc="candidate melee weapon LST types (0xFF none)", verified="cross-ref"),
    F("weapon_codes", "u8", count=6, doc="matching LST codes (0xFF none)", verified="cross-ref"),
    F("weapon_quality", "u8", doc="weapon quality", verified="doc"),
    F("unknown_ad", "bytes", 12, doc="mostly 0xFE/0xFF, occasional small values"),
    F("unknown_b9", "bytes", 19, doc="mostly zero; small values / 0xFFFF words"),
])

ENEMY = Spec("enemy", [
    F("enemy_type", "u16", doc="index into enemy_types", verified="structural"),
    F("name", "str", 12, doc="display name", verified="structural"),
    F("unknown_0e", "bytes", 8, doc="always zero"),
    F("unknown_16", "u16", doc="0,1,3,7,0x3F,0x4F,0x81; same for all enemies of a type"),
])

NUM_TYPES = 71
NUM_ENEMIES = 82


@dataclasses.dataclass
class Enm:
    enemy_types: list[dict]
    enemies: list[dict]


def parse(data: bytes) -> Enm:
    if len(data) != NUM_TYPES * ENEMY_TYPE.size + NUM_ENEMIES * ENEMY.size:
        raise FormatError("ENM: unexpected size")
    types = [ENEMY_TYPE.parse(data, i * ENEMY_TYPE.size) for i in range(NUM_TYPES)]
    base = NUM_TYPES * ENEMY_TYPE.size
    enemies = [ENEMY.parse(data, base + i * ENEMY.size) for i in range(NUM_ENEMIES)]
    return Enm(types, enemies)


def read(path) -> Enm:
    return parse(read_file(path))


def build(e: Enm) -> bytes:
    return b"".join(ENEMY_TYPE.build(t) for t in e.enemy_types) + b"".join(ENEMY.build(x) for x in e.enemies)
