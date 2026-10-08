"""dlformats -- readers/writers for the data files of Darklands (MicroProse, 1992).

Built on the community documentation by Merle (wallace.net/darklands),
Joel "Quadko" McIntyre (PIC), Olemars (IMC/LZ decoding) and vvendigo
(github.com/vvendigo/Darklands); every format here was re-verified against
the GOG 483.07 files, and several of those docs were corrected (see
docs/FORMATS.md).

Each module exposes ``parse(bytes) -> object`` and, where a writer exists,
``build(object) -> bytes`` with ``build(parse(x)) == x`` for every shipped
file.  :data:`REGISTRY` maps top-level game files to their handlers.
"""
from __future__ import annotations

import fnmatch

from . import (alc, cat, cty, dsc, enemypal, enm, exe_tables, fnt, imaps, imc, loc, lst, lzss, map,  # noqa: A004
               misc, msg, pan, pic, sav, snt)

__all__ = ["alc", "cat", "cty", "dsc", "enemypal", "enm", "exe_tables", "fnt", "imaps", "imc", "loc", "lst",
           "lzss", "map", "misc", "msg", "pan", "pic", "sav", "snt", "REGISTRY", "handler_for"]


def _rec(name):
    return (lambda d: misc.parse_records(name, d)), (lambda r: misc.build_records(name, r))


#: (glob pattern relative to the game directory, format name, parse, build or None)
REGISTRY = [
    ("*.CAT", "cat", cat.parse, cat.build),
    ("MSGFILES", "cat", cat.parse, cat.build),
    ("BC", "cat", cat.parse, cat.build),
    ("LCASTLE", "cat", cat.parse, cat.build),
    ("DARKLAND.LST", "lst", lst.parse, lst.build),
    ("DARKLAND.ALC", "alc", alc.parse, alc.build),
    ("DARKLAND.SNT", "snt", snt.parse, snt.build),
    ("DARKLAND.DSC", "dsc", dsc.parse, dsc.build),
    ("DARKLAND.ENM", "enm", enm.parse, enm.build),
    ("DARKLAND.LOC", "loc", loc.parse, loc.build),
    ("LOCS.TMP", "loc", loc.parse, loc.build),
    ("DARKLAND.CTY", "cty", cty.parse, cty.build),
    ("DARKLAND.MSG", "darkland_msg", msg.parse_darkland_msg, msg.build_darkland_msg),
    ("DARKLAND.MAP", "map", map.parse, map.build),
    ("SAVES/DEFAULT", "sav", sav.parse, sav.build),
    ("SAVES/*.SAV", "sav", sav.parse, sav.build),
    ("CHARACTR.TMP", "charactr_tmp", sav.parse_charactr_tmp, sav.build_charactr_tmp),
    ("CACHE.TMP", "cache_tmp", sav.parse_cache_tmp, sav.build_cache_tmp),
    ("FONTS.FNT", "fnt", fnt.parse, fnt.build),
    ("FONTS.UTL", "fnt", fnt.parse, fnt.build),
    ("PICS/*.PIC", "pic", pic.parse, pic.build),
    ("ENEMYPAL.DAT", "enemypal", enemypal.parse, enemypal.build),
    ("BATTLEGR.IMG", "img", imc.parse_img, imc.build_img),
    ("COMMONSP.IMG", "img", imc.parse_img, imc.build_img),
    ("BKGNDPAL.DAT", "bkgndpal", misc.parse_bkgndpal, misc.build_bkgndpal),
    ("BANNER.DAT", "banner", misc.parse_banner, misc.build_banner),
    ("*.PAN", "pan", pan.parse, pan.build),
] + [(n, "records:" + n, *_rec(n)) for n in misc.RECORD_FILES]

#: catalog-entry handlers: (glob on entry name, format, parse, build)
CAT_ENTRY_REGISTRY = [
    ("*.MSG", "msg", msg.parse, msg.build),
    ("*.PIC", "pic", pic.parse, pic.build),
    ("*.IMC", "imc", imc.parse, imc.build),
    ("I*.[0-9][0-9][0-9]", "imap", imaps.parse, imaps.build),
    # BC and LCASTLE entries: LZ streams with (mostly) undecoded payloads
    ("*.F[LFR]C", "lz", lzss.decompress, lzss.compress),
    ("*.[NW][WF]C", "lz", lzss.decompress, lzss.compress),
    ("LC_*", "lz", lzss.decompress, lzss.compress),
]


def handler_for(relpath: str):
    rp = relpath.replace("\\", "/").upper()
    for pat, name, p, b in REGISTRY:
        if fnmatch.fnmatchcase(rp, pat):
            return name, p, b
    return None


def entry_handler_for(name: str):
    u = name.upper()
    for pat, fmt, p, b in CAT_ENTRY_REGISTRY:
        if fnmatch.fnmatchcase(u, pat):
            return fmt, p, b
    return None
