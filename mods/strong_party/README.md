# Test mod: strong party (Quickstart party, overlay s001)

The four Quickstart characters (Gretchen, Gunther, Hans, Ebhard) start with 60 in every
attribute (current and maximum; divine favour stays 99) and 75 in all 19 skills. Equipment,
names and ages are unchanged. Use it to win fights in play-tests, e.g. to reach the loot
screen after a fight against human enemies.

- File: `s001/s001_0e76.c`, a copy of the draft `src/wip/s001/s001_0e76.c`.
- Function: `sub_s001_0e76_0000`, which builds the default party. A loop is added at its
  end. It writes the member records `D_9c69[m]`: `+0x49..0x4e` maximum attributes,
  `+0x50..0x55` current attributes (END STR AGL PER INT CHR), `+0x57..0x69` skills.

Build (plain mod build, every other module is the original code):

    python3 -I tools/build.py /home/claude/dl_raw/extracted/DARKLAND/DARKLAND.EXE --skip-analysis --mod \
        --overlay mods/strong_party --work build/mods_strong_party --out build/mods_strong_party/DARKLAND.EXE

Functional build (all drafts) WITH this mod: do NOT use `--functional --overlay mods/strong_party`.
build.py adds the src/wip drafts after the overlay directories, so src/wip/s001/s001_0e76.c
replaces this file (the build log says `overlay: .../src/wip/s001/s001_0e76.c replaces
mods/strong_party/s001/s001_0e76.c`), and the game starts with the normal party. Use:

    python3 -I tools/playtests/mkfuncmod.py mods/strong_party build/mods_strong_func2 build/mods_strong_func2/DARKLAND.EXE

Play-test (2026-10-10): the character sheet shows 60/60 for every attribute and 75 for every
skill. The sidebar shows 60 60 99. Load goes from 57/60 to 57/120. Both builds won a fight
against "ragged but determined bandits" without anyone giving orders. Loot results are in
docs/QA.md. Screens: build/mods_play/evidence/strong_party_sheet.png, loot_*.
