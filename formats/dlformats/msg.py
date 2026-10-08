"""MSG card files (``$*.MSG`` inside the MSGFILES catalog) and DARKLAND.MSG.

Card files (dialog / event "cards")
-----------------------------------
::

    u8 num_cards
    num_cards x {
        u8  text_y        top text margin   (Merle/vvendigo)
        u8  text_x        left text margin
        u8  unknown_2     0 except in $MCGUF07.MSG / one card in $RAUBI05.MSG
        u8  text_max_x    right text edge (text width = text_max_x - text_x)
        u8  unknown_4     as unknown_2
        char text[]       NUL-terminated
    }

All 419 card files parse with no trailing bytes.

Text mark-up (control bytes inside ``text``):

========  ==============================================================
byte      meaning
========  ==============================================================
0x0A      line break
0x14      paragraph / spacing break (usually ``0x0A 0x14``)
0x15      start of a normal option (``0x0A 0x14 0x15 ... 0x1D text``)
0x16      option that opens the saint list (pray)
0x10      option that opens a list (potions / formulae)
0x06      option that starts a battle
0x1D      separator between the ``...`` lead-in and the option text
0x01      start of a conditional block, ended by 0x13 -- shown or hidden by
          the executable (e.g. "In addition, ... ruled by $NamedOneName.")
0x13      end of conditional block
0x19      seen only as ``<0x19>ou decide to`` in $MONAS00/01 -- a glyph
          (FONTS.FNT char 25) used as a drop-cap 'Y'? (unverified)
0x1F      'ä' (Darklands charset); ``{`` = ö, ``|`` = ü
``$Name`` substitution variable resolved by the executable
          ($PlaceName, $LeaderName, $Money1, $ChosenOneName, $He/$him...)
========  ==============================================================

Options that are not used by a given screen are filled with dummy text
("0", "1", ...).

What is NOT in the data (lives in DARKLAND.EXE): which card file/card is
shown for a given situation (the menu-id -> card table, e.g. LOC ``menu``
values), what each option *does* (option index -> handler), the conditions
for 0x01..0x13 blocks, success/failure rolls, rewards, and the values of
``$`` variables.  The card files are pure presentation.

DARKLAND.MSG
------------
A different, simpler file: ``u8 count (10)`` followed by ``count`` fixed
400-byte NUL-padded text records (error message title, raubritter cairn
text, wasteland text, ...).
"""
from __future__ import annotations

import dataclasses
import re

from .util import F, FormatError, Spec, read_file

CARD_HEADER = Spec("card_header", [
    F("text_y", "u8", doc="top text margin", verified="doc"),
    F("text_x", "u8", doc="left text margin", verified="doc"),
    F("unknown_2", "u8", doc="0 for nearly all cards"),
    F("text_max_x", "u8", doc="right text edge", verified="doc"),
    F("unknown_4", "u8", doc="0 for nearly all cards"),
])

OPTION_MARKERS = {0x15: "option", 0x16: "saint_option", 0x10: "list_option", 0x06: "battle_option"}


@dataclasses.dataclass
class CardFile:
    cards: list[dict]     # header fields + "text" (raw latin-1 string)


def parse(data: bytes) -> CardFile:
    n = data[0]
    pos = 1
    cards = []
    for i in range(n):
        if pos + CARD_HEADER.size > len(data):
            raise FormatError(f"MSG: card {i} header truncated")
        c = CARD_HEADER.parse(data, pos)
        pos += CARD_HEADER.size
        z = data.find(b"\0", pos)
        if z < 0:
            raise FormatError(f"MSG: card {i} text not terminated")
        c["text"] = data[pos:z].decode("latin-1")
        pos = z + 1
        cards.append(c)
    if pos != len(data):
        raise FormatError(f"MSG: {len(data) - pos} trailing bytes")
    return CardFile(cards)


def read(path) -> CardFile:
    return parse(read_file(path))


def build(cf: CardFile) -> bytes:
    if len(cf.cards) > 255:
        raise FormatError("MSG: too many cards")
    out = bytearray((len(cf.cards),))
    for c in cf.cards:
        out += CARD_HEADER.build(c)
        t = c["text"].encode("latin-1")
        if b"\0" in t:
            raise FormatError("MSG: NUL inside card text")
        out += t + b"\0"
    return bytes(out)


_VAR = re.compile(r"\$[A-Za-z][A-Za-z0-9]*")


def analyse_card(text: str) -> dict:
    """Split a card text into preamble paragraphs and options.

    Returns ``{"body": str, "options": [{"kind", "text"}], "variables": [...],
    "has_conditional": bool}``.  This is a convenience view; :func:`build`
    always works from the raw ``text``.
    """
    options = []
    body_end = len(text)
    for m in re.finditer(r"[\x15\x16\x10\x06]", text):
        rest = text[m.end():]
        # an option is "<marker>...<0x1D>text" or a dummy "<marker>N"
        nxt = re.search(r"\n|$", rest)
        seg = rest[:nxt.start()]
        if body_end == len(text):
            body_end = m.start()
        kind = OPTION_MARKERS[ord(m.group())]
        label = seg.split("\x1d", 1)[1] if "\x1d" in seg else seg
        options.append({"kind": kind, "text": label.strip("\x14")})
    return {
        "body": text[:body_end],
        "options": options,
        "variables": sorted(set(_VAR.findall(text))),
        "has_conditional": "\x01" in text,
    }


def readable(text: str) -> str:
    """Human-readable rendering of card mark-up (for dumps)."""
    from .util import dl_to_unicode
    rep = {"\x14": "\n", "\x15": "[opt] ", "\x16": "[saint-opt] ", "\x10": "[list-opt] ",
           "\x06": "[battle-opt] ", "\x1d": " ", "\x01": "{IF:", "\x13": "}", "\x19": "<19>"}
    t = "".join(rep.get(ch, ch) for ch in text)
    return dl_to_unicode(t)


# ---------------------------------------------------------------------------
# DARKLAND.MSG
# ---------------------------------------------------------------------------

SYSTEXT = Spec("system_text", [F("text", "str", 400, doc="message text", verified="structural")])


@dataclasses.dataclass
class SystemMessages:
    messages: list[dict]


def parse_darkland_msg(data: bytes) -> SystemMessages:
    n = data[0]
    if 1 + n * SYSTEXT.size != len(data):
        raise FormatError("DARKLAND.MSG: size mismatch")
    return SystemMessages([SYSTEXT.parse(data, 1 + i * SYSTEXT.size) for i in range(n)])


def build_darkland_msg(m: SystemMessages) -> bytes:
    return bytes((len(m.messages),)) + b"".join(SYSTEXT.build(x) for x in m.messages)
