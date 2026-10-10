# Example mod: heavy blows (combat, overlay s014)

Every hit on an enemy does double damage. The party's hits (and enemies hitting each other)
queue twice the endurance damage, and the strength part, which is computed from it, doubles
too. Hits on the party are unchanged.

- File: `s014/s014_18bf.c`, a copy of the draft `src/wip/s014/s014_18bf.c` (attack resolution).
- Function: `sub_s014_18bf_08ca(d, t, pen, arm)`, which rolls the damage of one hit on
  combatant `t` and queues it in `D_d70c[t]`. Combatants 0..4 are the party, 5..12 the
  enemies. Added before the damage is queued:

      if (t >= 5)
          dmg *= 2;

  `sub_s014_18bf_098e` still caps the endurance loss per tick at 42.

Build:

    python3 -I tools/build.py /home/claude/dl_raw/extracted/DARKLAND/DARKLAND.EXE --skip-analysis --mod \
        --overlay mods/heavy_blows --work build/mods_heavy_blows --out build/mods_heavy_blows/DARKLAND.EXE

s014 grows by 6 paragraphs, so the build moves all its child overlays (s015..s025) and the
following sections. That is expected (`relayout: s014 0e76+1082 -> 0e76+1088`).

Play-test (2026-10-10, original vs mod, lockstep_sync.py + hunt.py + autobattle.py, unattended
Quickstart party): bandit-soldiers ambush ("ignore the warning and press onward"). The
original lost ("You awaken in a gully, stripped of arms..."). The mod party won in about half
the time, with hits of -20 on the enemies, and got a normal loot screen ("The party finds 0fl,
8gr, 2pf in cash", Mace/Cuirboulli/Studded Lthr/Medium Shield, quality 15). The two
battlefields differed from the first frame (input timing), so this is not a same-seed
comparison of the same fight. Screens: build/mods_play/evidence/heavy_blows_*.
