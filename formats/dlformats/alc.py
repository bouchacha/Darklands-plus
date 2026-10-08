"""DARKLAND.ALC -- alchemical formulae (alchemy screen / inventory).

Layout::

    u8 num_formulae (66)
    formula_definition[num_formulae]   (104 bytes each, see FORMULA)

Order matches ``formula_full_names`` in DARKLAND.LST.  The 66 formulae come
in 22 groups of three consecutive entries for the same potion (different
named authors, e.g. "al-Razi's / Gerard C's / Petrus Bs' Noxious Aroma").
Saves keep 22 bytes of ``formulae_known`` with 3 bits each, so presumably
formula ``3k+j`` maps to bit ``j`` of byte ``k`` (unverified).  Ingredient
``item_code`` values index the LST item table (the test suite checks they
are all LST items flagged ``is_component``).  Merle's note that ingredients
are listed in ascending code order is false for 15 of the 66 formulae.
"""
from __future__ import annotations

import dataclasses

from .util import F, FormatError, Spec, read_file

INGREDIENT = Spec("ingredient", [
    F("quantity", "u16", doc="amount needed (1..5); 0 = unused slot", verified="doc"),
    F("item_code", "u16", doc="LST item code of the component", verified="cross-ref"),
])

FORMULA = Spec("formula_definition", [
    F("description", "str", 80, doc="description text", verified="structural"),
    F("mystic_number", "u16", doc="base difficulty", verified="doc"),
    F("risk_factor", "u16", doc="0 low, 1 medium, 2 high", verified="doc"),
    F("ingredients", "spec", count=5, sub=INGREDIENT, doc="ingredients (usually, not always, ascending code), zero-filled", verified="cross-ref"),
])


@dataclasses.dataclass
class Alc:
    formulae: list[dict]


def parse(data: bytes) -> Alc:
    n = data[0]
    if 1 + n * FORMULA.size != len(data):
        raise FormatError("ALC: size mismatch")
    return Alc([FORMULA.parse(data, 1 + i * FORMULA.size) for i in range(n)])


def read(path) -> Alc:
    return parse(read_file(path))


def build(alc: Alc) -> bytes:
    return bytes((len(alc.formulae),)) + b"".join(FORMULA.build(f) for f in alc.formulae)
