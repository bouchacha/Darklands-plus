"""Round-trip and invariant tests against a real Darklands installation.

Run:  python3 -I -m unittest discover -s tests -v      (from formats/)
  or  python3 -I tests/test_roundtrip.py

The game directory defaults to /home/claude/dl_raw/extracted/DARKLAND and can
be overridden with the DARKLANDS_DIR environment variable.  Tests are skipped
when it is missing.  No game data is stored in the repository.

For every format with a writer, ``build(parse(x)) == x`` is checked on every
matching file, including catalog entries (MSG cards, EINFO PICs, IMC sprites,
IMAPS maps, BC/LCASTLE streams).  Read-only formats get parse + invariant
tests.  A per-format summary is printed at the end.
"""
from __future__ import annotations

import collections
import fnmatch
import os
import struct
import sys
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.dirname(HERE))

import dlformats as D  # noqa: E402
from dlformats import (alc, cat, cty, dsc, enemypal, enm, exe_tables, fnt, imaps, imc, loc, lst, lzss,  # noqa: E402
                       map as dlmap, misc, msg, pan, pic, sav, snt)

GAME = os.environ.get("DARKLANDS_DIR", "/home/claude/dl_raw/extracted/DARKLAND")
HAVE_GAME = os.path.isdir(GAME)

RESULTS: dict[str, collections.Counter] = collections.defaultdict(collections.Counter)


def gpath(*p):
    return os.path.join(GAME, *p)


def gread(*p) -> bytes:
    with open(gpath(*p), "rb") as f:
        return f.read()


def game_files():
    for root, _dirs, files in os.walk(GAME):
        for f in sorted(files):
            yield os.path.relpath(os.path.join(root, f), GAME).replace(os.sep, "/")


_CATS = {}


def catalog(name) -> cat.Catalog:
    if name not in _CATS:
        _CATS[name] = cat.parse(gread(name))
    return _CATS[name]


@unittest.skipUnless(HAVE_GAME, "game directory not found")
class RoundTripTopLevel(unittest.TestCase):
    """build(parse(file)) == file for every registered top-level file."""

    def test_all_registered_files(self):
        seen = 0
        for rel in game_files():
            h = D.handler_for(rel)
            if h is None:
                continue
            name, parse, build = h
            data = gread(rel)
            with self.subTest(file=rel, fmt=name):
                obj = parse(data)
                if build is None:
                    RESULTS[name]["parsed (read-only)"] += 1
                    continue
                ok = build(obj) == data
                RESULTS[name]["round-trip ok" if ok else "round-trip FAIL"] += 1
                self.assertTrue(ok, f"{rel}: rebuilt bytes differ")
            seen += 1
        self.assertGreater(seen, 500)


@unittest.skipUnless(HAVE_GAME, "game directory not found")
class RoundTripCatalogEntries(unittest.TestCase):
    """Every catalog entry with a handler round-trips (MSG, PIC, IMC, IMAP, LZ)."""

    def test_entries(self):
        for arch in cat.ARCHIVES:
            for e in catalog(arch):
                h = D.entry_handler_for(e.name)
                if h is None:
                    continue
                fmt, parse, build = h
                with self.subTest(archive=arch, entry=e.name, fmt=fmt):
                    obj = parse(e.data)
                    ok = build(obj) == e.data
                    RESULTS["entry:" + fmt]["round-trip ok" if ok else "round-trip FAIL"] += 1
                    self.assertTrue(ok)

    def test_every_nonempty_entry_has_a_handler(self):
        missing = [(a, e.name) for a in cat.ARCHIVES for e in catalog(a)
                   if e.data and D.entry_handler_for(e.name) is None]
        self.assertEqual(missing, [])


@unittest.skipUnless(HAVE_GAME, "game directory not found")
class CatInvariants(unittest.TestCase):
    def test_contiguous_and_dates(self):
        for arch in cat.ARCHIVES:
            c = catalog(arch)
            pos = 2 + 24 * len(c.entries)
            for e in c.entries:
                self.assertEqual(e.offset, pos, (arch, e.name))
                pos += len(e.data)
                self.assertIsNotNone(e.date, (arch, e.name, e.timestamp))
                self.assertTrue("1990" < e.date < "1994", e.date)
            self.assertEqual(pos, os.path.getsize(gpath(arch)))

    def test_rebuild_after_replace(self):
        c = cat.parse(gread("MSGFILES"))
        cat.replace(c, c.entries[0].name, b"\x00")
        c2 = cat.parse(cat.build(c))
        self.assertEqual(c2.entries[0].data, b"\x00")
        self.assertEqual([e.data for e in c2.entries[1:]], [e.data for e in c.entries[1:]])


@unittest.skipUnless(HAVE_GAME, "game directory not found")
class LzInvariants(unittest.TestCase):
    def test_streams_consume_exactly(self):
        n = 0
        for arch in ("A00C.CAT", "IMAPS.CAT", "BC", "LCASTLE"):
            for e in catalog(arch):
                if e.data and lzss.is_compressed_name(e.name):
                    out, used = lzss.decompress(e.data, return_consumed=True)
                    self.assertEqual(used, len(e.data), e.name)
                    n += 1
        self.assertGreater(n, 200)

    def test_compress_random_data(self):
        import random
        rnd = random.Random(1)
        for size in (0, 1, 2, 3, 100, 5000, 70000):
            data = bytes(rnd.choice(b"\x00\x00\x05\x07ab") for _ in range(size))
            self.assertEqual(lzss.decompress(lzss.compress(data)), data)


@unittest.skipUnless(HAVE_GAME, "game directory not found")
class DataInvariants(unittest.TestCase):
    """Cross-file checks backing the 'cross-ref' claims in docs/FORMATS.md."""

    @classmethod
    def setUpClass(cls):
        cls.lst = lst.parse(gread("DARKLAND.LST"))
        cls.items = cls.lst.items
        cls.enm = enm.parse(gread("DARKLAND.ENM"))
        cls.cty = cty.parse(gread("DARKLAND.CTY"))
        cls.loc = loc.parse(gread("DARKLAND.LOC"))

    def test_lst_counts(self):
        self.assertEqual(len(self.items), 200)
        self.assertEqual(len(self.lst.saint_full_names), 136)
        self.assertEqual(len(self.lst.formula_full_names), 66)
        self.assertTrue(all(s.startswith("St.") for s in self.lst.saint_full_names))
        self.assertTrue(all(s.startswith("S.") for s in self.lst.saint_short_names))
        music = [it["name"] for it in self.items if it["name"] and lst.item_flags(it)["is_music"]]
        self.assertEqual(music, ["Harp", "Flute"])          # 0x23 bit 5 (Merle: bit 6)
        for it in self.items:
            if it["name"]:
                f = lst.item_flags(it)
                n = f["flag24_0"] + f["flag24_1"] + f["flag24_7"]
                self.assertLessEqual(n, 1, it["name"])

    def test_snt_alc_match_lst(self):
        self.assertEqual(len(snt.parse(gread("DARKLAND.SNT")).descriptions), 136)
        a = alc.parse(gread("DARKLAND.ALC"))
        self.assertEqual(len(a.formulae), 66)
        unsorted = 0
        for f in a.formulae:
            codes = [i["item_code"] for i in f["ingredients"] if i["quantity"]]
            unsorted += codes != sorted(codes)      # Merle: "ascending" -- not true for 15 formulae
            for c in codes:
                self.assertTrue(lst.item_flags(self.items[c])["is_component"], self.items[c]["name"])
            self.assertIn(f["risk_factor"], (0, 1, 2))
        self.assertEqual(unsorted, 15)

    def test_enm_item_type_code_pairs(self):
        pairs = 0
        for t in self.enm.enemy_types:
            for tf, cf in (("vital_armor_type", "vital_armor_code"), ("limb_armor_type", "limb_armor_code"),
                           ("shield_type", "shield_code"), ("missile_type", "missile_code"),
                           ("ammo_type", "ammo_code")):
                if t[cf] != 0xFF and t[tf] != 0xFF:
                    self.assertEqual(self.items[t[cf]]["type"], t[tf], (t["type_name"], cf))
                    pairs += 1
            for ty, co in zip(t["weapon_types"], t["weapon_codes"]):
                if co != 0xFF:
                    if t["type_name"].startswith("Skeleton"):
                        # only exception: natural-weapon type 48 paired with code 8 (Battle Axe)
                        self.assertEqual((ty, co), (48, 8))
                        continue
                    self.assertEqual(self.items[co]["type"], ty, (t["type_name"], co))
                    pairs += 1
        self.assertGreater(pairs, 100)

    def test_enm_palettes_and_groups(self):
        pals = enemypal.parse(gread("ENEMYPAL.DAT")).palettes
        self.assertEqual(len(pals), 71)
        groups = set()
        for t in self.enm.enemy_types:
            # Baphomet (start 71, count 1) points one past the last ENEMYPAL record
            limit = len(pals) + (1 if t["type_name"] == "Baphomet" else 0)
            self.assertLessEqual(t["palette_start"] + t["palette_count"], limit, t["type_name"])
            groups.add(t["image_group"])
        tac = {r["group"] for r in misc.parse_records("TACANIM.DB", gread("TACANIM.DB"))}
        sprites = {e.name[:3] for a in ("E00C.CAT", "M00C.CAT") for e in catalog(a)}
        self.assertEqual(sprites, tac - {"A00", "C00", "F01", "F60"})
        # three ENM groups have neither sprites nor TACANIM entries: the EXE must remap them
        self.assertEqual(groups - sprites, {"M08", "M12", "M99"})
        for e in self.enm.enemies:
            self.assertLess(e["enemy_type"], 71)

    def test_cty_vs_loc_dsc(self):
        cities = self.cty.cities
        self.assertEqual(len(cities), 92)
        self.assertEqual(len(dsc.parse(gread("DARKLAND.DSC")).descriptions), 92)
        mismatched = []
        for i, c in enumerate(cities):
            L = self.loc.locations[i]
            self.assertEqual(L["icon"], 0)
            if (L["x"], L["y"]) != (c["data"]["entry_x"], c["data"]["entry_y"]):
                mismatched.append(c["short_name"])
            self.assertEqual(L["city_size"], c["data"]["city_size"])
            for d in c["data"]["dock_destinations"]:
                self.assertTrue(d == 0xFFFF or d < 92)
        self.assertEqual(mismatched, ["Strassburg"])   # LOC x=49, CTY x=50

    def test_cty_content_bits(self):
        """Bits re-derived in cty.py agree with the name strings for >= 89/92 cities."""
        names = {"town_hall": "town_hall_name", "fortress": "fortress_name", "cathedral": "cathedral_name",
                 "slums": "slum_name", "kloster": "kloster_name", "university": "university_name",
                 "pawnshop": "pawnshop_name", "unknown_munz": "unknown_munz_name",
                 "unknown_zeughaus": "unknown_zeughaus_name"}
        for flag, field in names.items():
            agree = sum(cty.contents_flags(c)[flag] == (c[field].strip() != "") for c in self.cty.cities)
            self.assertGreaterEqual(agree, 89, flag)
        docks = sum(cty.contents_flags(c)["docks"] == any(d != 0xFFFF for d in c["data"]["dock_destinations"])
                    for c in self.cty.cities)
        self.assertGreaterEqual(docks, 91)

    def test_save_default(self):
        s = sav.parse(gread("SAVES", "DEFAULT"))
        self.assertEqual(len(s.characters), 4)
        for ch in s.characters:
            for it in ch["items"][:ch["num_items"]]:
                self.assertEqual(self.items[it["code"]]["type"], it["type"], ch["full_name"])
            for it in ch["items"][ch["num_items"]:]:
                self.assertEqual(it, {"code": 0, "type": 0, "quality": 0, "quantity": 0, "weight": 0})
        ct = sav.parse_charactr_tmp(gread("CHARACTR.TMP"))
        self.assertEqual(ct.characters, s.characters)

    def test_map(self):
        m = dlmap.parse(gread("DARKLAND.MAP"))
        self.assertEqual((m.width, m.height), (327, 931))
        for L in self.loc.locations:
            self.assertLess(L["x"], m.width)
            self.assertLessEqual(L["y"], m.height)   # one location sits at y == height (off-map)

    def test_msg_cards(self):
        n = 0
        for e in catalog("MSGFILES"):
            cf = msg.parse(e.data)
            for c in cf.cards:
                msg.analyse_card(c["text"])
                n += 1
        self.assertGreater(n, 2000)
        RESULTS["entry:msg"]["cards parsed"] = n

    def test_pic_images(self):
        n = 0
        for rel in game_files():
            if fnmatch.fnmatch(rel, "PICS/*.PIC"):
                p = pic.parse(gread(rel))
                img = p.image
                self.assertIsNotNone(img, rel)
                self.assertGreaterEqual(len(img.pixels), img.width * img.height)
                n += 1
        self.assertGreater(n, 500)

    def test_pic_encode_new_image(self):
        px = bytes((x * 7 + y) % 256 for y in range(50) for x in range(64))
        p = pic.make(px, 64, 50, pic.Palette(16, [(i % 64, 0, 63 - i % 64) for i in range(240)]))
        q = pic.parse(pic.build(p))
        self.assertEqual(q.image.pixels, px)

    def test_imc_frames(self):
        frames = 0
        for arch in ("A00C.CAT", "C00C.CAT", "E00C.CAT", "F01C.CAT", "F60C.CAT", "M00C.CAT"):
            for e in catalog(arch):
                m = imc.parse(e.data)
                fr = m.frames()
                self.assertEqual(len(fr), 8 * m.frame_count)
                for f in fr:
                    self.assertEqual(f.height, m.height, e.name)
                frames += len(fr)
        RESULTS["entry:imc"]["frames decoded"] = frames
        self.assertGreater(frames, 18000)

    def test_imc_regenerated_layout(self):
        m = imc.parse(catalog("A00C.CAT").entries[0].data)
        m2 = imc.with_frames(m, m.frames())
        self.assertEqual([f.to_pixels() for f in imc.parse_raw(imc.build_raw(m2)).frames()],
                         [f.to_pixels() for f in m.frames()])

    def test_img_banks(self):
        for name, n in (("BATTLEGR.IMG", 200), ("COMMONSP.IMG", 13)):
            b = imc.parse_img(gread(name))
            self.assertEqual(len(b.frames()), n)

    def test_imaps(self):
        for e in catalog("IMAPS.CAT"):
            raw = lzss.decompress(e.data)
            self.assertEqual(len(raw), 13308, e.name)
            imaps.parse_raw(raw)

    def test_fonts(self):
        f = fnt.parse(gread("FONTS.FNT"))
        self.assertEqual(len(f.fonts), 3)
        u = fnt.parse(gread("FONTS.UTL"))
        self.assertEqual(len(u.fonts), 4)
        for a, b in zip(f.fonts, u.fonts):      # same metrics, a few glyphs differ
            self.assertEqual((a.first_char, a.last_char, a.height, a.bytes_per_row),
                             (b.first_char, b.last_char, b.height, b.bytes_per_row))
        for font in f.fonts:
            extra = font.bitmap[font.height * font.count * font.bytes_per_row:]
            self.assertEqual(extra.count(0), len(extra))   # line-spacing rows are blank

    def test_pan(self):
        n = 0
        for rel in game_files():
            if rel.endswith(".PAN"):
                p = pan.parse(gread(rel))
                self.assertEqual(p.magic, pan.MAGIC)
                n += 1
        self.assertEqual(n, 15)

    def test_exe_tables(self):
        t = exe_tables.extract(gread("DARKLAND.EXE"))
        self.assertEqual(t["attributes"][:7], ["Endurance", "Strength", "Agility", "Perception",
                                               "Intelligence", "Charisma", "Divine Favor"])
        self.assertEqual(len([s for s in t["skills"] if s]), 19)
        RESULTS["exe_tables (read-only)"]["tables extracted"] = len(t)

    def test_dl_charset(self):
        from dlformats.util import dl_to_unicode
        self.assertEqual(dl_to_unicode("J\x1fger L|beck L{we"), "Jäger Lübeck Löwe")


def tearDownModule():
    if not RESULTS:
        return
    print("\n\nPer-format results")
    print("-" * 60)
    for fmt in sorted(RESULTS):
        print(f"{fmt:32s} " + ", ".join(f"{k}: {v}" for k, v in sorted(RESULTS[fmt].items()) if v))


if __name__ == "__main__":
    unittest.main(verbosity=2)
