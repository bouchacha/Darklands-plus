#!/usr/bin/env python3
"""Build the replacement-art checklist from a player's Darklands 483.07 files.

From the repository root, run python port/tools/gen_art_checklist.py GAME [--output FILE].
Only names, descriptions, dimensions and frame relationships go in the JSON.
"""
import argparse
from collections import Counter, defaultdict
import json
from pathlib import Path
import re
import struct
import sys
import tempfile

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[1]
sys.path.insert(0, str(ROOT / "formats"))
from dlformats import cat, enm, fnt, imc, lzss, pic


def group(category, name, source, route, assets, **fields):
    return dict(category=category, group=name, source=source, route=route, **fields, assets=assets)


def png(key, size=None, **fields):
    asset = {"file": key + ".png"}
    if size is not None:
        asset["native_px"] = size
    return {**asset, **fields}


def captions():
    result = {}
    for line in (ROOT / "port/tools/aiart_captions.txt").read_text(encoding="utf-8").splitlines():
        if ":" in line and not line.startswith("#"):
            name, subject = line.split(":", 1)
            result[name.strip().upper()] = subject.strip()
    return result


def picture_kind(key, size, caption, source):
    if source == "EINFO.CAT":
        return "figures", "enemy information pictures", "picture"
    if key in ("MAPICONS", "MAPICON2"):
        return "overland map", "terrain sheets", "native"
    if key in ("MOUSEPTR", "MAPCURSR"):
        return "interface", "original pointer sheets", "native"
    if re.fullmatch(r"[ACF]\d\d(SMALL|SHORT|STAT)", key):
        return "figures", "party portraits and figures", "picture"
    if re.fullmatch(r"WEAPON\d+", key):
        return "equipment", "weapons", "picture"
    if re.fullmatch(r"SHIELD[A-Z]|(SMALL|MEDIUM|LARGE)SH", key):
        return "equipment", "shields and crests", "picture"
    if re.fullmatch(r"[A-Z-]+(VIT|LIMS?)|BRIGSTUD|MAILCUIR", key):
        return "equipment", "armour", "picture"
    if re.fullmatch(r"X?MS\d+", key):
        return "scenes", "MS series illustrations", "picture"
    if caption:
        return "scenes", "captioned illustrations", "picture"
    if key.startswith("MAP"):
        return "overland map", "map interface", "picture"
    if caption is not None or size == (320, 200):
        return "screens", "screens and backgrounds", "picture"
    return "interface", "components and other pictures", "picture"


def picture_groups(files):
    pictures = sorted((p for p in files["PICS"].iterdir() if p.suffix.upper() == ".PIC"),
                      key=lambda p: p.name.upper())
    if not pictures:
        raise ValueError("No .PIC files in PICS")
    illustration_text = captions()
    assets = defaultdict(list)

    def add(source, name, data):
        image = pic.parse(data).image
        if image is None or not image.width or not image.height:
            assets["palettes", "picture palette", source, "reference"].append({"source_file": name})
            return
        key = Path(name).stem.upper()
        size = (image.width, image.height)
        subject = illustration_text.get(key)
        category, title, route = picture_kind(key, size, subject, source)
        row = dict(source_file=name, native_px=size)
        if route == "picture":
            row = png(key, size, source_file=name)
        if subject:
            row["subject"] = subject
        assets[category, title, source, route].append(row)

    for path in pictures:
        add("PICS", path.name.upper(), path.read_bytes())
    for entry in cat.read(files["EINFO.CAT"]).entries:
        add("EINFO.CAT", entry.name.upper(), entry.data)
    for kinds, rows in assets.items():
        yield group(*kinds, rows)


def port_labels(relative_path, table_name):
    """Read the code/name pairs used by the port's figure picker."""
    source = (ROOT / relative_path).read_text(encoding="utf-8")
    table = source.partition(table_name)[2].partition("};")[0]
    labels = dict(re.findall(r'\{\s*"([A-Z0-9]+)"\s*,\s*"([^"]+)"\s*\}', table))
    if not labels:
        raise ValueError(f"No labels in {relative_path} {table_name}")
    return labels


def port_groups(figures):
    portraits = [png(code + suffix, size, subject=name, figure=code)
                 for code, name in figures.items()
                 for suffix, size in [("SMALL", (34, 34)), ("SHORT", (11, 19))]]
    yield group("figures", "additional party portraits", "port/mods/looks.c", "generated_picture", portraits)
    crests = [png("SHIELD" + code, (11, 19)) for code in "PQRSTUVWXYZ"]
    yield group("equipment", "optional additional crests", "port/mods/hdart.c", "optional", crests)
    yield group("interface", "optional named artwork", "port/ui", "optional", [
        png("GAUNTLET", subject="Gauntlet cursor. Put the fingertip at the top left."),
        png("STARTLOGO", subject="Start menu lettering. Use transparency and close cropping."),
    ])


def town_selections():
    """Records selected by the nine town battlefield definitions in src/wip/s010/s010_1432.c."""
    town = defaultdict(set)

    def add(bank, records, types):
        for record in records:
            town[bank, record].update(types)

    for first in (0, 6):
        add("CITY.FLC", (first, first + 1), (1, 2, 3, 5, 6))
    for first in (2, 4, 8):
        add("CITY.FLC", (first, first + 1), (2, 3, 4))
    add("CITY.FFC", [0, 16, 17, 18, 19], range(1, 7))
    add("CITY.FRC", [0, 1, 2, 3, 4, 5, 9], range(1, 7))
    for index in range(6):
        types = [6] if index == 5 else [1, 2, 3, 4]
        if index in (3, 4):
            types.append(5)
        for axis in ("NW", "WW"):
            add(f"CITY{axis}1{index}.{axis}C", range(8 if index == 5 else 6), types)
    for bank, records in {"OUT.FLC": [0, 1, 2, 9], "OUT.FFC": range(18, 24),
                          "OUT.NWC": [3], "OUT.WWC": [3], "OUTGRPA.FRC": [7, 8, 9, 30, 31, 32, 33]}.items():
        add(bank, records, [7])
    for bank, both, type8, type9 in [
        ("KEEP.FLC", [], [0], [1]), ("KEEP.FFC", [9, 10, 11], [], []),
        ("KEEP1.NWC", [2], [1], [0, 6, 8, 9]), ("KEEP1.WWC", [5], [1], [0, 6]),
        ("KEEP.NFC", [], [], [0, 8, 14, 10]), ("KEEP.WFC", [], [], [0]),
        ("KEEP.FRC", [0, 1, 2, 3, 4, 5, 8, 9, 10, 11, 12, 16, 17], [], []),
    ]:
        add(bank, both, [8, 9])
        add(bank, type8, [8])
        add(bank, type9, [9])
    return town


def sprite_asset(key, frame, **fields):
    # Bounds of nonzero source pixels; padding belongs to the canvas on every frame.
    left, top, right, bottom = frame.width, frame.height, 0, 0
    for y, (skip, pixels) in enumerate(frame.rows):
        end = len(pixels.rstrip(b"\0"))
        if end:
            left = min(left, skip + len(pixels) - len(pixels.lstrip(b"\0")))
            top = min(top, y)
            right = max(right, skip + end)
            bottom = y + 1
    box = [left, top, right, bottom] if bottom else None
    return png(key, (frame.width, frame.height), **fields, ink_box=box)


# Directory record sizes and byte positions from the battlefield loader (s002_0e76.c).
# Each view offset is an absolute byte position in the decompressed scenery bank.
SCENERY = {
    "FLC": ("floor", 4, 0, (2,)), "FFC": ("floor detail", 6, 0, (2, 4)),
    "NWC": ("north wall", 10, 0, (2, 4, 6, 8)), "WWC": ("west wall", 10, 0, (2, 4, 6, 8)),
    "NFC": ("north wall detail", 10, 0, (2, 4)), "WFC": ("west wall detail", 10, 0, (2, 4)),
    "FRC": ("furniture", 40, 34, (38,)),
}


def scenery_group(entry, town, invalid, omitted):
    name = entry.name.upper()
    bank, ext = name.split(".")
    kind, stride, size_field, view_fields = SCENERY[ext]
    raw = lzss.decompress(entry.data)

    def word(pos):
        return struct.unpack_from("<H", raw, pos)[0]

    header = word(view_fields[0])
    if header % stride or header > len(raw):
        raise ValueError(f"{name}: invalid scenery directory")
    assets = []
    for record in range(header // stride):
        pos = record * stride
        size = word(pos + size_field)
        offsets = [word(pos + field) for field in view_fields]
        base = offsets[0]
        if not size:
            omitted["zero_size_records"] += 1
            continue
        if base < header or base + size > len(raw):
            invalid.append(dict(bank=name, record=record, reason="record outside bank data"))
            continue
        for view, offset in enumerate(offsets):
            if not offset:
                continue
            if offset in offsets[view + 1:]:
                omitted["shared_view_slots"] += 1
                continue
            try:
                if not base <= offset < base + size - 1:
                    raise ValueError("pointer outside source record")
                frame, end = imc.decode_frame(raw, offset)
                if end > base + size:
                    raise ValueError("frame exceeds source record")
            except (ValueError, IndexError) as error:
                invalid.append(dict(bank=name, record=record, view=view, reason=str(error)))
                continue
            if not frame.width or not frame.height:
                omitted["zero_dimension_views"] += 1
                continue
            row = sprite_asset(f"{bank}_{ext}_{record:02d}_{view}", frame, record=record, view=view)
            if (name, record) == ("OUTGRPA.FRC", 20):
                row["subject"] = "Tree"
            if (name, record) in town:
                row["town_types"] = sorted(town[name, record])
            assets.append(row)
    return group("battle scenery", name, "BC/" + name, "battle", assets,
                 kind=kind, source_records=header // stride)


def fighter_groups(files, figures):
    weapons = port_labels("port/ui/figures.cpp", "W[][2] =")
    subjects = defaultdict(set)
    enemies = enm.read(files["DARKLAND.ENM"])
    for enemy in enemies.enemies:
        code = enemies.enemy_types[enemy["enemy_type"]]["image_group"]
        subjects[code].add(enemy["name"])
    actions = {"WK": "walk", "CB": "combat", "DY": "fall"}
    for archive in (name for name in cat.ARCHIVES if name.endswith("C.CAT")):
        for entry in cat.read(files[archive]).entries:
            stem = Path(entry.name).stem.upper()
            anim = imc.parse(entry.data)
            slots_by_offset = defaultdict(list)
            for slot, offset in enumerate(anim.frame_offsets):
                slots_by_offset[offset].append(slot)
            frames = []
            # hdart.c names a segment on each registration. For a shared segment the last slot wins.
            for offset, slots in sorted(slots_by_offset.items(), key=lambda item: item[1][-1]):
                frame, _ = imc.decode_frame(anim.data, 16 * offset)
                frames.append(sprite_asset(f"{stem}_{slots[-1]}", frame, slots=slots))
            code, action, weapon = stem[:3], actions[stem[3:5]], stem[5:]
            yield group("battle figures", stem, archive + "/" + entry.name, "battle", frames,
                        figure=code, subjects=sorted(subjects[code]) or [figures.get(code, code)],
                        action=action, weapon=weapons.get(weapon, weapon or None), slots=len(anim.frame_offsets))


def reference_groups(files):
    for name in ("BATTLEGR.IMG", "COMMONSP.IMG"):
        frames = imc.parse_img(files[name].read_bytes()).frames()
        yield group("original sprite banks", name, name, "native",
                    [dict(index=i, native_px=(f.width, f.height)) for i, f in enumerate(frames)])
    yield group("cutscenes", "animation files", "game folder", "native",
                [dict(source_file=n) for n in sorted(files) if n.endswith(".PAN")])
    for name in ("FONTS.FNT", "FONTS.UTL"):
        for i, font in enumerate(fnt.read(files[name]).fonts):
            glyphs = [dict(character_code=code, native_px=(width, font.height))
                      for code, width in enumerate(font.widths, font.first_char)]
            yield group("bitmap fonts", f"{name} font {i}", name, "native", glyphs)
    yield group("palettes", "palette tables", "game folder", "reference",
                [dict(source_file=name) for name in ("BKGNDPAL.DAT", "ENEMYPAL.DAT", "COMNCLRS.DAT")])
    for sheet, rows, first in [("MAPICONS", range(1, 16), 0), ("MAPICON2", range(16), 16)]:
        cells = [dict(row=r, column=c, source_rect=[c * 16, r * 12, 16, 12], terrain=first + r)
                 for r in rows for c in range(16)]
        yield group("overland map", sheet + " cells", f"PICS/{sheet}.PIC", "sheet_region", cells)


def catalogue(game):
    files = {p.name.upper(): p for p in game.iterdir()}
    figures = port_labels("port/mods/looks.c", "KNOWN[] =")
    town = town_selections()
    invalid = []
    omitted = Counter()
    groups = [*picture_groups(files), *port_groups(figures), *fighter_groups(files, figures),
              *reference_groups(files)]
    groups.extend(scenery_group(entry, town, invalid, omitted) for entry in cat.read(files["BC"]).entries)
    groups.sort(key=lambda g: (g["category"], g["group"]))

    layouts = defaultdict(list)
    for name in sorted(cat.read(files["IMAPS.CAT"]).names()):
        if re.fullmatch(r"ICITY\.[0-8][0-9]{2}", name):
            layouts[int(name[6]) + 1].append(name)

    counts = Counter()
    filenames = set()
    town_records = set()
    town_count = 0
    for g in groups:
        counts[g["category"]] += len(g["assets"])
        for asset in g["assets"]:
            if "file" in asset:
                key = asset["file"].casefold()
                if key in filenames:
                    raise ValueError(f"Repeated replacement name {key}")
                filenames.add(key)
            if "town_types" in asset:
                town_count += 1
                town_records.add((g["group"], asset["record"]))
    if town_records != set(town):
        raise ValueError(f"Town scenery records missing from BC: {sorted(set(town) - town_records)}")

    meta = {
        "title": "Darklands+ art checklist", "version": 1, "game_version": "Darklands 483.07",
        "guide": "docs/MODDING.md#art-pack-checklist",
        "scope": "Named replacements and original-format art in the base game. A group describes where its assets come from and how to use them. A pack can cover any subset.",
        "routes": {
            "picture": "Named PIC when drawn through the replacement-art renderer.",
            "battle": "Named picture used by the battlefield renderer.",
            "generated_picture": "Optional named portrait generated for an available figure.",
            "optional": "Additional named picture supported by Darklands+. Extra crests follow the picker in order.",
            "native": "Original-format file or entry within a file.",
            "sheet_region": "Cell within an original map sheet.",
            "reference": "Colour data used by other pictures.",
        },
        "fields": {
            "file": "Preferred PNG filename inside the mod's art folder. Only entries with this field count towards PNG coverage.",
            "native_px": "Original canvas [width, height] in game pixels. High-resolution artwork can use any size. For the display aspect, scale width by 5 and height by 6.",
            "ink_box": "Visible native sprite bounds [left, top, right, bottom], with right and bottom exclusive. Null means the original is entirely transparent.",
            "slots": "On a fighter asset, every animation slot sharing the named drawing. Pose = slot // 8, facing index = slot % 8. Keep the feet aligned across poses.",
            "town_types": "Town battlefield definitions that can select this scenery. town_layouts lists the maps for each type.",
            "source_rect": "Region within a native sheet [left, top, width, height].",
        },
        "templates": "Export personal-use templates in F12 > Mods. Look in art/original for the matching filename. Battle templates depend on which pictures have been drawn. The JSON contains metadata only.",
        "open_ended": [
            {"category": "character portraits", "pattern": "portraits/<character name>.png", "recommended_px": [360, 800]},
            {"category": "anonymous memory pictures", "pattern": "M<7 hex digits>.png", "lookup": "Show the picture name in F12 > Mods."},
            {"category": "new encounters and mod-added figures", "lookup": "Use the picture name chosen by the mod."},
            {"category": "native interface and drawn overlays", "lookup": "See the individual named exceptions."},
        ],
        "evidence": ["port/mods/hdart.c", "src/wip/s014/s014_0e76.c", "src/wip/s002/s002_0e76.c",
                     "src/wip/s010/s010_1432.c", "port/mods/worldmap.c", "formats/dlformats",
                     "port/tools/aiart_captions.txt"],
        "town_layouts": dict(sorted(layouts.items())), "invalid_source_views": invalid,
        "omitted_empty_directory_entries": dict(omitted),
        "summary": {"groups": len(groups), "entries_by_category": dict(sorted(counts.items())),
                    "named_png_targets": len(filenames), "total_catalogue_entries": sum(counts.values()),
                    "fighter_source_slots": sum(g["slots"] for g in groups if g["category"] == "battle figures"),
                    "town_scenery": town_count},
    }
    return meta, groups


def save(path, meta, groups):
    # One asset per line keeps individual names searchable and future changes readable in Git.
    lines = [json.dumps(meta, ensure_ascii=False, indent=2)[:-2] + ',\n  "groups": [']
    for i, g in enumerate(groups):
        header = {key: value for key, value in g.items() if key != "assets"}
        lines.append("    " + json.dumps(header, ensure_ascii=False)[:-1] + ', "assets": [')
        lines.append(",\n".join("      " + json.dumps(asset, ensure_ascii=False) for asset in g["assets"]))
        lines.append("    ]}" + ("," if i + 1 < len(groups) else ""))
    lines.append("  ]\n}\n")
    with tempfile.NamedTemporaryFile(dir=path.parent, delete=False) as tmp:
        temp_path = Path(tmp.name)
    try:
        temp_path.write_bytes("\n".join(lines).encode("utf-8"))
        temp_path.replace(path)
    finally:
        temp_path.unlink(missing_ok=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("game", type=Path, help="Base game directory containing DARKLAND.ENM and PICS")
    parser.add_argument("--output", type=Path, default=HERE / "darklands_art_checklist.json")
    args = parser.parse_args()
    try:
        meta, groups = catalogue(args.game)
        save(args.output, meta, groups)
    except KeyError as exc:
        parser.exit(2, f"Missing game file or unrecognized record {exc}. Use the base DARKLAND folder.\n")
    except (OSError, ValueError, IndexError, struct.error) as exc:
        parser.exit(2, f"Could not build the art checklist: {exc}\n")
    print(f"Wrote {meta['summary']['named_png_targets']} named pictures in the checklist at {args.output}")


if __name__ == "__main__":
    main()
