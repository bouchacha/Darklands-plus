# Example mod: half-price inn (residence screen, overlay s036)

Taking up residence in an inn (and the other "spend a day" lodgings handled by overlay s036)
costs half (rounded up). The price text on the residence screen, the money check and the
amount actually charged at the end of the day all use the halved price. The screen shows a
small "(half-price inn mod)" label next to "You decide to...".

- File: `s036/s036_1462.c`, a copy of the draft `src/wip/s036/s036_1462.c`.
- Change: a wrapper `static int inn_price(a, b)` that returns `(vec_2099(a, b) + 1) / 2`,
  plus `#define vec_2099 inn_price` so every lodging-price call below it (in
  `sub_s036_1462_10d0` = price text, `sub_s036_1462_147a` = spend the day,
  `sub_s036_1462_1858` = total cost of the day) uses it. `vec_2099` is the game's lodging price rule.
- The label is new text, and DGROUP is full, so it is a far string:
  `static char far s_cheap_inn[] = "(half-price inn mod)";` copied into a local buffer with
  `_fstrcpy((char far *)buf, s_cheap_inn)` before `sub_root_047c_055a(g, 0xa0, 0xa4, buf)`
  (the text routine takes a near pointer). This builds and shows correctly.
- Not changed: the "relax with a good meal ... for 2 groschen" option on the inn CARD
  (that price is in the card script, not in s036).

Build:

    python3 -I tools/build.py /home/claude/dl_raw/extracted/DARKLAND/DARKLAND.EXE --skip-analysis --mod \
        --overlay mods/cheap_inn --work build/mods_cheap_inn --out build/mods_cheap_inn/DARKLAND.EXE

Note: s036_1462.c already declares `_fstrcpy`; declaring it a second time
(`int far _fstrcpy();`) fails with `error C2086: '_fstrcpy' : redefinition`.

Play-test (2026-10-10, original vs mod, same seed): Quickstart (Stuttgart), inn, take up
residence: original "staying here today, at 2gr, 0pf. That will leave you with 0fl, 8gr, 10pf",
mod "at 1gr, 0pf ... 0fl, 9gr, 10pf" plus the label. After "Spend a day" the purse is
8gr 10pf (original) vs 9gr 10pf (mod). Screens: build/mods_play/evidence/cheap_inn_*.

Far strings only work here because this module's CONST still has a spare segment slot. A far
name needs a segment word in the module's CONST. When CONST is already full, the build fails
(tested with s014_18bf.c: `CONST data grew from 22 to 24 bytes, but DGROUP is full; move new
strings to far data`). A code-segment string (`_based(_segname("_CODE"))`) compiles, but the
link fails (`omflink.LinkError: ...: overlapping data at s036:156fc`). See docs/QA.md, round 4.
