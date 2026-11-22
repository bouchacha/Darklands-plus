#!/usr/bin/env python3
"""List replacement art, plan a set of pictures, or check a Darklands+ mod's art folder."""
import argparse
from collections import Counter, defaultdict
import json
from pathlib import Path
import sys

MANIFEST = Path(__file__).with_name("darklands-art-checklist.json")
EXTENSIONS = {".png", ".jpg", ".bmp", ".tga"}
OK = {"present", "blank_original_fallback"}


def select_groups(catalogue, args):
    selected = []
    for group in catalogue["groups"]:
        if args.category and group["category"].casefold() != args.category.casefold():
            continue
        if args.group and group["group"].casefold() != args.group.casefold():
            continue
        assets = group["assets"]
        if args.town:
            assets = [asset for asset in assets if asset.get("town_types")]
        if assets:
            selected.append({**group, "assets": assets})
    if not selected:
        raise ValueError("No matching groups. Use --list-groups to find a category or group name.")
    return selected


def list_groups(groups):
    return [dict(category=g["category"], group=g["group"], route=g["route"], entries=len(g["assets"]),
                 named_files=sum("file" in asset for asset in g["assets"])) for g in groups]


def file_key(path):
    # The base game uses ASCII keys. Unicode filenames cannot stand in for a base game name.
    return path.stem.lower() if path.stem.isascii() else path.stem


def find_images(root):
    files = defaultdict(list)

    def scan(folder, depth):
        # hdart.c counts every non-hidden entry before it filters extensions or sorts names.
        entries = [p for p in folder.iterdir() if not p.name.startswith(".")]
        if len(entries) > 4096:
            raise ValueError(f"{folder} exceeds the game's limit of 4096 entries per folder. "
                             "Divide the art among subfolders, up to three levels deep.")
        for path in sorted(entries, key=lambda p: (p.name.lower(), p.name)):
            if path.is_dir():
                if depth < 3 and path.name.lower() != "original":
                    scan(path, depth + 1)
            elif path.suffix.lower() in EXTENSIONS:
                files[file_key(path)].append(path)

    scan(root, 0)
    return files


def check_image(asset, matches, root, image_reader):
    result = {"file": asset["file"]}
    blank = "ink_box" in asset and asset["ink_box"] is None
    if not matches:
        result["status"] = "blank_original_fallback" if blank else "missing"
    elif len(matches) > 1:
        result.update(status="duplicate", paths=[p.relative_to(root).as_posix() for p in matches])
    else:
        result["path"] = matches[0].relative_to(root).as_posix()
        try:
            with image_reader.open(matches[0], formats=["PNG", "JPEG", "BMP", "TGA"]) as image:
                image.load()
                visible = image.convert("RGBA").getchannel("A").getbbox() is not None
            if blank and visible:
                result["status"] = "review_originally_blank"
            elif not blank and not visible:
                result["status"] = "review_empty_image"
            else:
                result["status"] = "present"
        except (OSError, ValueError, image_reader.DecompressionBombError) as error:
            result.update(status="invalid_image", reason=str(error))
    return result


def check_pack(pack, catalogue, groups):
    root = pack / "art"
    if not root.is_dir():
        raise ValueError(f"Expected a mod folder containing art/: {pack}")
    try:
        from PIL import Image
    except ImportError:
        raise ValueError("Checking images needs Pillow. Install it with: python -m pip install Pillow") from None

    files = find_images(root)
    results = [check_image(asset, files.get(Path(asset["file"]).stem.lower(), []), root, Image)
               for g in groups for asset in g["assets"] if "file" in asset]
    if not results:
        raise ValueError("This selection has no named PNG targets. Use --plan to inspect its original files.")
    known = {Path(asset["file"]).stem.lower() for g in catalogue["groups"] for asset in g["assets"]
             if "file" in asset}
    unlisted = sorted(p.relative_to(root).as_posix() for key, paths in files.items()
                      if key not in known for p in paths)
    return {"selected": len(results), "counts": dict(Counter(r["status"] for r in results)),
            "unlisted_files": unlisted, "results": results}


def show_check(report):
    print(f"{report['selected']} named pictures in scope")
    for status, count in sorted(report["counts"].items()):
        print(f"  {status}: {count}")
    for row in report["results"]:
        if row["status"] not in OK:
            detail = row.get("reason") or ", ".join(row.get("paths", []))
            print(f"{row['status']}  {row['file']}" + (f"  ({detail})" if detail else ""))
    for path in report["unlisted_files"]:
        print(f"unlisted  {path}")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("mod_folder", type=Path, nargs="?", help="Mod folder containing art/ (needs Pillow)")
    mode.add_argument("--list-groups", action="store_true", help="Show group names, categories and counts")
    mode.add_argument("--plan", action="store_true", help="Print the selected catalogue entries as JSON")
    parser.add_argument("--town", action="store_true", help="Scenery from all nine town battle definitions")
    parser.add_argument("--category", help='Exact category, e.g. "scenes" or "battle scenery"')
    parser.add_argument("--group", help='Exact group, e.g. "CITY.FLC" or "E03WKSW"')
    parser.add_argument("--json", action="store_true", help="Machine-readable results on standard output")
    args = parser.parse_args()
    try:
        data = json.loads(MANIFEST.read_text(encoding="utf-8"))
        groups = select_groups(data, args)
        if args.list_groups:
            rows = list_groups(groups)
            if args.json:
                print(json.dumps(rows))
            else:
                for row in rows:
                    print(f"{row['category']}  /  {row['group']}  "
                          f"({row['named_files']} named pictures, {row['entries']} entries)")
            return 0
        if args.plan:
            plan = {"fields": data["fields"], "routes": data["routes"], "groups": groups}
            print(json.dumps(plan, separators=(",", ":")) if args.json else json.dumps(plan, indent=2))
            return 0
        report = check_pack(args.mod_folder, data, groups)
        if args.json:
            print(json.dumps(report))
        else:
            show_check(report)
        return int(any(status not in OK for status in report["counts"]))
    except (OSError, ValueError) as error:
        parser.exit(2, f"{error}\n")


if __name__ == "__main__":
    sys.exit(main())
