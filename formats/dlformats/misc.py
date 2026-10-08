"""Small data files.  Each has ``parse_*`` / ``build_*`` (round-trip exact).

==============  ============================================================
file            content
==============  ============================================================
TACANIM.DB      39 x 14 bytes: char group[4] ('A00', 'E03', 'M21'...),
                u16 unknown[3], u8 unknown[4] -- per sprite group tactical
                animation parameters (vvendigo: editing the last 4 bytes
                had no visible effect)
HITSEQ.DTA      56 x 15 bytes of small values (0..7, some 0x8x) -- combat
                hit/animation sequences? (unknown)
COMNCLRS.DAT    72 RGB triplets (6-bit) -- "common colours" palette fragment
                (target palette range unknown)
BKGNDPAL.DAT    11 x 71 RGB triplets (6-bit) -- background palette fragments
                (11 variants; magenta 3f/00/3f entries look like unused
                slots; target range unknown)
DARKLAND.FAM    5 x 106 bytes: u16 unknown_00, u16 unknown_02 (1..4),
                u16 unknown_04, char names[4][25] -- possibly a hall of
                fame (party names + score?); unverified
BANNER.DAT      u16 offsets[20] + 20 NUL-terminated strings: the DOS
                start-up banner / memory-check messages of the loader
CFILE.DAT       4 x {u16, u16, u8, u8} -- unknown
DARKLAND.CFG    90 bytes: char sound_driver[13] ('nsound.dl'),
                char graphics_driver[13] ('mgraphic.exe'), char path[64]
                ('e:\\') -- install configuration (field split inferred
                from the NUL-padded strings)
CONFIG.DRK      8 bytes (4 x u16) -- runtime options, unknown
DARKLAND.CCH    106 bytes -- unknown (starts 0x63 like the cache table)
==============  ============================================================

Not parsed (raw only): TERRAIN.FIL and LCASTLE's ``*.FIL`` (196-byte
terrain/tile set lists with names such as 'mine'/'lastcast' and garbage
after the NULs), ``*.ATV`` (32 bytes), LEVEL0.ENM (291-byte runtime list of
the current battle's enemies: a count byte then an instance record that
embeds an ENM-like attribute/skill block), LOCS.TMP (runtime copy of
DARKLAND.LOC), DK.TMP / MEM.LOG (runtime scratch).

``*.DGT`` (OPENDARK, ENDDARK1/2) are raw headerless 8-bit unsigned PCM
(mean 127.5, full 0..255 range); the playback rate is not stored in the file.
"""
from __future__ import annotations

import dataclasses
import struct

from .util import F, FormatError, Spec, join_cstrings, split_cstrings

RGB = Spec("rgb", [F("r", "u8"), F("g", "u8"), F("b", "u8")])

TACANIM_REC = Spec("tacanim", [
    F("group", "str", 4, doc="image group", verified="cross-ref"),
    F("unknown_04", "u16", count=3, doc="unknown"),
    F("unknown_0a", "u8", count=4, doc="unknown (editing had no visible effect, vvendigo)"),
])

HITSEQ_REC = Spec("hitseq", [F("values", "u8", count=15, doc="unknown")])

FAM_REC = Spec("fam_record", [
    F("unknown_00", "u16"), F("unknown_02", "u16"), F("unknown_04", "u16"),
    F("names", "spec", count=4, sub=Spec("name25", [F("name", "str", 25)]), doc="four names"),
])

CFILE_REC = Spec("cfile", [F("unknown_00", "u16"), F("unknown_02", "u16"), F("unknown_04", "u8"), F("unknown_05", "u8")])

CFG = Spec("darkland_cfg", [
    F("sound_driver", "str", 13, doc="e.g. 'nsound.dl'", verified="structural"),
    F("graphics_driver", "str", 13, doc="e.g. 'mgraphic.exe'", verified="structural"),
    F("path", "str", 64, doc="install path", verified="structural"),
])


def _records(spec: Spec, data: bytes, name: str) -> list[dict]:
    if len(data) % spec.size:
        raise FormatError(f"{name}: size {len(data)} not a multiple of {spec.size}")
    return [spec.parse(data, i) for i in range(0, len(data), spec.size)]


def _join(spec: Spec, recs: list[dict]) -> bytes:
    return b"".join(spec.build(r) for r in recs)


# generic fixed-record files -------------------------------------------------
RECORD_FILES = {
    "TACANIM.DB": TACANIM_REC,
    "HITSEQ.DTA": HITSEQ_REC,
    "COMNCLRS.DAT": RGB,
    "DARKLAND.FAM": FAM_REC,
    "CFILE.DAT": CFILE_REC,
    "DARKLAND.CFG": CFG,
}


def parse_records(name: str, data: bytes) -> list[dict]:
    return _records(RECORD_FILES[name], data, name)


def build_records(name: str, recs: list[dict]) -> bytes:
    return _join(RECORD_FILES[name], recs)


# BKGNDPAL.DAT ---------------------------------------------------------------
BKGND_COLORS = 71


def parse_bkgndpal(data: bytes) -> list[list[tuple[int, int, int]]]:
    if len(data) % (3 * BKGND_COLORS):
        raise FormatError("BKGNDPAL: size not a multiple of 213")
    pals = []
    for p in range(0, len(data), 3 * BKGND_COLORS):
        pals.append([tuple(data[p + 3 * i:p + 3 * i + 3]) for i in range(BKGND_COLORS)])
    return pals


def build_bkgndpal(pals) -> bytes:
    return b"".join(bytes(c) for pal in pals for c in pal)


# BANNER.DAT -----------------------------------------------------------------
@dataclasses.dataclass
class Banner:
    strings: list[str]


def parse_banner(data: bytes) -> Banner:
    first, = struct.unpack_from("<H", data, 0)
    n = first // 2
    offs = struct.unpack_from(f"<{n}H", data, 0)
    strings, pos = split_cstrings(data, n, first)
    # verify the offsets describe the strings contiguously
    p = first
    for o, s in zip(offs, strings):
        if o != p:
            raise FormatError("BANNER: non-contiguous strings")
        p += len(s.encode("latin-1")) + 1
    if pos != len(data):
        raise FormatError("BANNER: trailing bytes")
    return Banner(strings)


def build_banner(b: Banner) -> bytes:
    n = len(b.strings)
    offs = []
    p = 2 * n
    for s in b.strings:
        offs.append(p)
        p += len(s.encode("latin-1")) + 1
    return struct.pack(f"<{n}H", *offs) + join_cstrings(b.strings)


# DGT ------------------------------------------------------------------------
def dgt_to_wav(data: bytes, path, rate: int) -> None:
    """Wrap raw 8-bit unsigned PCM in a WAV container.  ``rate`` must be
    supplied by the caller -- the file does not store it."""
    import wave
    with wave.open(str(path), "wb") as w:
        w.setnchannels(1)
        w.setsampwidth(1)
        w.setframerate(rate)
        w.writeframes(data)
