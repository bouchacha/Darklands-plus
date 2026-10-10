# Example mod: swift travel (overland map, overlay s030)

Overland travel takes a third of the time. Every step on the overland map adds a terrain
dependent number of minutes to a counter, and each full 60 minutes advances the clock by an
hour. The mod divides that per-step cost by three (rounded up, so a step costs at least
1 minute). Nothing else changes: routes, encounter checks and the map look the same.

- File: `s030/overland.c`, a copy of the draft `src/wip/s030/overland.c` (the overland map is
  not in byte-exact C yet; the draft plays the same as the original, QA round 2).
- Function: `sub_s030_1462_2ac8(terrain)`, the minutes per step for each terrain type
  (6 for a road, up to 0xc5 for the slowest terrain). One added line before the
  "whole days" fold-down: `v_di = (v_di + 2) / 3;`.

Build:

    python3 -I tools/build.py /home/claude/dl_raw/extracted/DARKLAND/DARKLAND.EXE --skip-analysis --mod \
        --overlay mods/swift_travel --work build/mods_swift_travel --out build/mods_swift_travel/DARKLAND.EXE

(Copy `build/qa2/analysis` and `build/qa2/force_db.json` into the work directory first, or drop
`--skip-analysis`.) The build log says `relayout: s030 1462+0317 -> 1462+0318 paragraphs`.

Play-test (2026-10-10, original vs mod, same seed, lockstep_sync.py): Quickstart in Bromberg,
leave the city, one click east along the river (`at 290 112; click; wait 30`), then F6 (Party
Information). Same position, the original shows a dark moon and **August 27, Matins 2**, the
mod a crescent moon and **August 25, Compline 23**: 27 hours less on the same ~40-hour walk,
i.e. one third of the time (start ~Aug 25, 9-10 h). Screens: build/mods_play/evidence/swift_*.
