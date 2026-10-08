"""Read-only extraction of data tables embedded in DARKLAND.EXE (483.07).

These are *data that live in the executable*, so mods cannot change them by
editing data files.  Offsets are file offsets in the GOG 483.07
DARKLAND.EXE (1,675,637 bytes) and come from vvendigo's ``reader_exe.py``;
each table is checked against an expected first string, and extraction is
refused for other executables.

Only NUL-separated string tables are extracted; the binary tables next to
them (card dispatch, saint effects, item behaviour, ...) are left to the
code side of the project.
"""
from __future__ import annotations

from .util import FormatError

EXE_SIZE = 1675637

# name: (start, end, expected first string)
TABLES = {
    "attributes": (0x1881D9, 0x188222, "Endurance"),
    "skills": (0x188222, 0x1882F0, "Edged Wpns"),
    "occupations": (0x188350, 0x1884D3, "Recruit"),
    "family_backgrounds": (0x18879A, 0x188801, None),
    "card_variables": (0x188AF4, 0x188DD2, "PlaceName"),
    "first_names_male": (0x188E71, 0x1891A0, None),
    "first_names_female": (0x1891A0, 0x18945F, None),
    "surnames": (0x18945F, 0x189A20, None),
    "background_pictures": (0x18B710, 0x18C157, "common"),
    "card_files": (0x196502, 0x19783F, "$Initi00"),
}


def _split(data: bytes, a: int, b: int) -> list[str]:
    return [s.decode("latin-1") for s in data[a:b].split(b"\0")]


def extract(data: bytes) -> dict[str, list[str]]:
    if len(data) != EXE_SIZE or data[:2] != b"MZ":
        raise FormatError("not the 483.07 DARKLAND.EXE these offsets were taken from")
    out = {}
    for name, (a, b, first) in TABLES.items():
        strings = _split(data, a, b)
        if first is not None and strings[0] != first:
            raise FormatError(f"EXE table {name}: expected {first!r}, got {strings[0]!r}")
        out[name] = strings
    return out
