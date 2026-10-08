#!/usr/bin/env python3
"""Dump every parsed Darklands data file to JSON and every image to PNG.

Usage:
    python3 -I tools/dump_all.py [OUTDIR] [--game GAMEDIR] [--no-sprites]

OUTDIR defaults to /home/claude/darklands-re/build/data_dump/ (must stay out
of version control: it contains game data).  GAMEDIR defaults to
$DARKLANDS_DIR or /home/claude/dl_raw/extracted/DARKLAND.

Output layout:
    json/<FILE>.json             parsed top-level data files
    json/<ARCHIVE>/index.json    catalog index (names, DOS dates, sizes)
    json/MSGFILES/<CARD>.json    card files (raw text + readable rendering)
    json/IMAPS.CAT/<MAP>.json    tactical maps (40x40 cell grid + raw tail)
    png/PICS/*.png, png/EINFO.CAT/*.png        PIC images
    png/sprites/<ARCHIVE>/<IMC>.png            IMC frame contact sheets
    png/fonts/*.png, png/map/DARKLAND.MAP.png, png/IMAPS.CAT/*.png
    manifest.json                every output and every error

Colours: PICs without their own palette (and all sprites) are rendered with
the standard VGA default palette as a fallback, so their colours are not
what the game shows.
"""
from __future__ import annotations

import argparse
import json
import os
import sys
import time
import traceback

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.dirname(HERE))

import dlformats as D  # noqa: E402
from dlformats import (cat, cty, enemypal, enm, exe_tables, fnt, imaps, imc, lst, lzss,  # noqa: E402
                       map as dlmap, misc, msg, pan, pic, sav)
from dlformats.util import dl_to_unicode, to_json  # noqa: E402

DEFAULT_OUT = "/home/claude/darklands-re/build/data_dump/"
DEFAULT_GAME = os.environ.get("DARKLANDS_DIR", "/home/claude/dl_raw/extracted/DARKLAND")


class Dumper:
    def __init__(self, game, out, sprites=True):
        self.game, self.out, self.sprites = game, out, sprites
        self.manifest = {"game_dir": game, "outputs": [], "errors": [], "skipped": []}

    # -- helpers -------------------------------------------------------------
    def path(self, *p):
        full = os.path.join(self.out, *p)
        os.makedirs(os.path.dirname(full), exist_ok=True)
        return full

    def write_json(self, rel, obj):
        p = self.path(rel)
        with open(p, "w", encoding="utf-8") as f:
            f.write(to_json(obj))
        self.manifest["outputs"].append(rel)

    def err(self, what, e):
        self.manifest["errors"].append({"item": what, "error": f"{type(e).__name__}: {e}"})
        print(f"  ! {what}: {e}", file=sys.stderr)

    def read(self, rel):
        with open(os.path.join(self.game, rel), "rb") as f:
            return f.read()

    # -- enrichment of some formats for readability -------------------------
    def enrich(self, name, rel, obj):
        if name == "lst":
            for it in obj.items:
                it["flag_names"] = [k for k, v in lst.item_flags(it).items() if v]
        elif name == "cty":
            for c in obj.cities:
                c["contents_flags"] = [k for k, v in cty.contents_flags(c).items() if v]
        elif name == "enemypal":
            for p in obj.palettes:
                p["first_index"] = enemypal.first_index(p)
        elif name == "sav":
            obj = {"save": obj, "caches_decoded": sav.parse_caches(obj.num_caches, obj.cache_offsets, obj.cache_data)}
        elif name == "pan":
            obj = {"magic": obj.magic, "count": obj.count, "decompressed_size": len(obj.data),
                   "trailer": obj.trailer, "note": "inner animation format not decoded"}
        elif name == "img":
            obj = {"frame_offsets": obj.frame_offsets,
                   "frames": [{"width": f.width, "height": f.height} for f in obj.frames()]}
        elif name == "map":
            obj = {"width": obj.width, "height": obj.height,
                   "rows": [list(r) for r in obj.rows]}
        elif name == "fnt":
            obj = {"fonts": [{k: v for k, v in vars(f).items() if k != "bitmap"} for f in obj.fonts]}
        return obj

    # -- top level -------------------------------------------------------------
    def dump_top_level(self):
        for root, _dirs, files in os.walk(self.game):
            for f in sorted(files):
                rel = os.path.relpath(os.path.join(root, f), self.game).replace(os.sep, "/")
                h = D.handler_for(rel)
                if h is None:
                    self.manifest["skipped"].append(rel)
                    continue
                name, parse, _build = h
                try:
                    data = self.read(rel)
                    obj = parse(data)
                    if name == "pic":
                        self.dump_pic(obj, "png/" + rel[:-4] + ".png")
                        continue
                    if name == "cat":
                        self.dump_catalog(rel, obj)
                        continue
                    if name == "fnt":
                        for i, font in enumerate(obj.fonts):
                            fnt.to_png(font, self.path(f"png/fonts/{f}.{i}.png"))
                            self.manifest["outputs"].append(f"png/fonts/{f}.{i}.png")
                    if name == "map":
                        dlmap.to_png(obj, self.path("png/map/DARKLAND.MAP.png"))
                        self.manifest["outputs"].append("png/map/DARKLAND.MAP.png")
                    if name == "img" and self.sprites:
                        imc.frames_to_png(obj.frames(), self.path(f"png/sprites/{f}.png"), columns=20)
                        self.manifest["outputs"].append(f"png/sprites/{f}.png")
                    self.write_json(f"json/{rel}.json", {"format": name, "file": rel,
                                                         "data": self.enrich(name, rel, obj)})
                except Exception as e:  # keep going, record the error
                    self.err(rel, e)
                    traceback.print_exc()

    def dump_pic(self, p, rel):
        try:
            if pic.to_png(p, self.path(rel)):
                self.manifest["outputs"].append(rel)
            else:
                self.manifest["skipped"].append(rel + " (no image)")
        except Exception as e:
            self.err(rel, e)

    # -- catalogs --------------------------------------------------------------
    def dump_catalog(self, arch, c):
        self.write_json(f"json/{arch}/index.json", {
            "archive": arch,
            "entries": [{"name": e.name, "dos_datetime": e.timestamp, "date": e.date, "size": len(e.data),
                         "offset": e.offset, "lz_compressed": lzss.is_compressed_name(e.name)} for e in c]})
        for e in c:
            h = D.entry_handler_for(e.name)
            if h is None:
                self.manifest["skipped"].append(f"{arch}/{e.name}")
                continue
            fmt, parse, _ = h
            try:
                obj = parse(e.data)
                if fmt == "msg":
                    cards = []
                    for i, cd in enumerate(obj.cards):
                        a = msg.analyse_card(cd["text"])
                        cards.append({"index": i, **cd, "readable": msg.readable(cd["text"]),
                                      "options": a["options"], "variables": a["variables"],
                                      "has_conditional": a["has_conditional"]})
                    self.write_json(f"json/{arch}/{e.name}.json", {"format": "msg", "cards": cards})
                elif fmt == "pic":
                    self.dump_pic(obj, f"png/{arch}/{e.name[:-4]}.png")
                elif fmt == "imc":
                    info = {k: v for k, v in vars(obj).items() if k != "data"}
                    frames = obj.frames()
                    info["frames"] = [{"width": f.width, "height": f.height} for f in frames]
                    self.write_json(f"json/{arch}/{e.name}.json", {"format": "imc", "data": info})
                    if self.sprites:
                        rel = f"png/sprites/{arch}/{e.name[:-4]}.png"
                        imc.frames_to_png(frames, self.path(rel), columns=max(1, obj.frame_count))
                        self.manifest["outputs"].append(rel)
                elif fmt == "imap":
                    self.write_json(f"json/{arch}/{e.name}.json",
                                    {"format": "imap", "grid_40x40": [obj.cells[y * 40:(y + 1) * 40] for y in range(40)],
                                     "tail": obj.tail})
                    imaps.to_png(obj, self.path(f"png/{arch}/{e.name}.png"))
                    self.manifest["outputs"].append(f"png/{arch}/{e.name}.png")
                elif fmt == "lz":
                    self.write_json(f"json/{arch}/{e.name}.json",
                                    {"format": "lz", "decompressed_size": len(obj),
                                     "note": "payload format not decoded", "head": obj[:64]})
            except Exception as ex:
                self.err(f"{arch}/{e.name}", ex)

    # -- executable tables -----------------------------------------------------
    def dump_exe(self):
        try:
            t = exe_tables.extract(self.read("DARKLAND.EXE"))
            self.write_json("json/DARKLAND.EXE.tables.json",
                            {k: [dl_to_unicode(s) for s in v] for k, v in t.items()})
        except Exception as e:
            self.err("DARKLAND.EXE tables", e)

    def run(self):
        t = time.time()
        os.makedirs(self.out, exist_ok=True)
        self.dump_top_level()
        self.dump_exe()
        self.manifest["seconds"] = round(time.time() - t, 1)
        with open(os.path.join(self.out, "manifest.json"), "w") as f:
            json.dump(self.manifest, f, indent=1)
        print(f"{len(self.manifest['outputs'])} outputs, {len(self.manifest['errors'])} errors, "
              f"{len(self.manifest['skipped'])} skipped, {self.manifest['seconds']}s -> {self.out}")
        return 1 if self.manifest["errors"] else 0


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("outdir", nargs="?", default=DEFAULT_OUT)
    ap.add_argument("--game", default=DEFAULT_GAME)
    ap.add_argument("--no-sprites", action="store_true", help="skip IMC/IMG contact sheets")
    a = ap.parse_args(argv)
    if not os.path.isdir(a.game):
        ap.error(f"game directory not found: {a.game}")
    return Dumper(a.game, a.outdir, sprites=not a.no_sprites).run()


if __name__ == "__main__":
    sys.exit(main())
