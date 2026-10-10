# QA round 1 (2026-10-09)
OK in both builds: tavern intro, inn (after fix 1), market/barter, churches, side streets, character sheets, equipment, formulae, saints, overland travel and encounters, save game, Create a New World (first steps).
Bugs:
1. FIXED: 32 card scripts called sub_s014_0e76_199c instead of sub_s029_0e76_199c (sibling overlays share address 0e76:199c); identical bytes in the original layout, wrong target once s014 moved. dec16 now prefers names reachable from the caller.
2. OPEN: battle starts garbled (sidebar garbage, view off-party, not paused) when battlefield drafts (s002, s004-s013) and combat sub-module drafts (s015-s025) are both in. Candidates s020/s025. Untested split builds: build/qa/one_s2c15.EXE, one_b4c15.EXE.
3. OPEN: with the s014 drafts, loot after a battle lists garbage items: s014_0e76 end-of-battle or s014_18bf attack code.
Same-second runs share the RNG seed: start original and test build together for lockstep comparisons (tools/playtests/lockstep.py).

# QA round 2 (2026-10-09, functional build build/qa3/func.EXE, 71 drafts, vs original)
OK (identical screens, lockstep): Create a New World in full (s037/s038: name, family, attributes, occupations,
skills, add to party, heraldry, Begin the Adventure, pact cards); save game (F10 menu / Alt-S) and load
("The Story Continues", same save loaded into both builds; saves from both builds differ only in
uninitialised header padding); inn residence day labour (earn money), relax 8h; residence alchemy formula
book (s033: formula list, Use formula, batches, ingredient counts incl. bought ingredients; no potion actually
brewed - not enough ingredients/money); praying to a saint from a card (wolves -> St. Aidan, s034/s039);
potion thrown from a card; camping in the wilderness ('c' key, s036 mode 1: guard, pray, spend a day);
Party Information screen (F6); overland travel with ~8 encounter cards (pilgrims, wolves, bandits, refugees,
city walls); city tour: Obermarkt notices/gossip/jobs, Rathaus/Vogt, Schloss, Dom (mass, priest),
Jakobikirche (mass, confession), Berg-Akademie at night, guilds -> blacksmith, merchant quest offers;
barter buy/sell/other person.
Bugs:
4. OPEN (suspect: src/wip/s032/barter.c, sub_s032_1462_1d18 "shop_has_item"): shops stock far fewer items.
   Freiberg herbalist: original Pure Gold/Aqua Regia/Zinken/Choleric..., draft only Pure Gold; goods merchant
   Torch/Rope/Lantern/Flute vs Torch/Lantern. Confirmed with a single-draft build (only s032 = same bug) and a
   functional-minus-s032 build (stock identical to the original). Cause: the original reads an uninitialised
   [bp-2] (`size`) and then STORES the clamped size back into it, so from the 2nd call on [bp-2] holds the
   previous size and the city size (D_9785->size-1) is used; the draft keeps the computed size in DI and
   never writes [bp-2], so the stale value stays >=0x5c/-1 and size is always 1 (rarest stock table).
   Fix idea: one variable (`size = (size < 0x5c && size != -1) ? D_9785->size - 1 : 1;`), as the comment
   in the file already describes, even at the cost of the register-allocation mismatch.
   Recipe: tools/playtests/market_stock.txt.
5. (known bug 2 again) a night walk to the Freiberg crafts district started a street battle; functional
   build showed the garbled sidebar/view (build/qa3/shots/battle_night.png).
Not reproduced: one overland run diverged after camping (different encounter); the same steps rerun from
Quickstart stayed identical, and orig-vs-orig also stayed identical - most likely input timing.
Tester notes: keys (F10, Alt-S, F6, c) are ignored while the mouse pointer is over a menu option - park it
first (`at 150 195`); sending keys to one session at a time and retrying until the screen changes works.
Concurrent playtesters raced for X display :77 (two DOSBoxes on one display); use
tools/playtests/lockstep_sync.py (own display range + DOSBox started on a 3 s clock boundary).
A wrong answer to the copy-protection screen (s041, shown when leaving the city after loading a save) quits
the game: start overland tests from Quickstart instead. Saves used: build/qa3/shots/morning (Freiberg,
new-world party), build/qa3/shots/qs (Teschen, Quickstart party).
New recipes: new_world_full.txt, load_game.txt, camp_saint.txt, city_tour.txt, market_stock.txt, lockstep_sync.py.

# QA round 2 (2026-10-09, build/qa2)
Root cause of bug 2 and of the s001 issue: WRONG CONST STAMPS. `stamp.py --wip` places a draft's CONST
(far segment slots) by searching DGROUP for the compiled words; segment words are not unique, so a draft
that needs MORE slots than the original (two far names where the original had one) is stamped wherever
the pattern happens to occur, and `build.py --mod` then writes its segment words over another module's
slots. Found with tools/playtests/constcheck.py (compares the slots the original code loads with the
compiled C and the stamp) - run it on every new --wip stamp:
- s025 (enemy placement): F_20a5_763b/763c as two far names -> 6 slots, stamped at 7d1c over s001's
  (7d18-7d22) and s002's (7d22-7d28) slots. FIXED: one far array, hint CONST=7e0a, re-stamped 7e0a+000a.
  This was bug 2 (s002 battle entry with wrong segments: view off the party, not paused) AND the s001
  "Hand Axe 00q" (s001's slots overwritten).
- s017 (dungeon view): floor cells + floor art as two far names -> 17 slots, stamped at 7dc4 over
  s018/s019's slots (with s015 below: garbage sidebar). FIXED: one struct, hint 7dbc, re-stamped
  7dbc+0020. Also fixed two real bugs found while comparing: object record stride 13 (pad[4]; original
  12) and the cell-code table passed as F_20a5_589 (20a5:0589) instead of 20a5:5890.
- s015: CONST stamped at 7cf8 (the original uses 7db8). FIXED: hint, re-stamped 7db8+0002.
Bisection (all with s002+s004-s013 drafts): +s015-s019 ok, +s020+s022 ok, +s024 ok, +s025 view off/not
paused, +s023-s025 same; full functional build: also garbage sidebar.
1. s001: only s001_0e76.c overlaid -> Quickstart equipment correct; also correct in the full functional
   build after the s025 fix. REMOVED from src/wip/EXCLUDE.
2. FIXED (see above). Retest (build/qa2/func3.EXE, func4.EXE: all drafts): wolves, bandits, soldiers,
   city guards, schrats - every battle starts with the party in view, paused, normal sidebar.
3. Garbage loot: three bugs in the s014 drafts, all "declared differently from the original data":
   - s014_0e76: `extern struct item6 far F_2e38_003e[][64], F_2e38_0dbe[][16];` - in MSC `far` belongs
     to the declarator, so F_2e38_0dbe (the enemies' items) was a NEAR array: a dead enemy's pile was
     copied from DS:0bde+i*0x60. Same for `char far F_2e38_0bfc, F_2e38_0bfd;` (reinforcement count
     written to DS:0bfd). FIXED (0bfc/0bfd as one far array so the CONST slots stay as in the original).
     tools/playtests/farcheck.py finds this pattern (none left in src/wip, src/c).
   - s014_0e76 + s014_19cd: D_c291 is a POINTER (s002: D_c291 = &D_a895) but was declared
     `struct loot D_c291[]`, so loot lists were copied over DGROUP c291.. and written to the loot file.
     FIXED. tools/playtests/ptrcheck.py finds address-vs-variable mismatches (none left).
   Retest: func3.EXE (c291 fix only) still showed garbage loot after a won bandit fight (the far bug);
   func4.EXE has all three fixes - retest below.
Not a bug: "Edged Wpns changed from 11 to 15" style messages pause the battle (space resumes).
Note: the Quickstart party loses most fights against bandit-soldiers/guards/schrats in the original-
code build too (build with only the s014_0e76/19cd drafts lost to bandit-soldiers as well) - not a draft bug.
OPEN (new): build/qa2/loot.EXE (ONLY s014_0e76 + s014_19cd drafts, rest original asm) quits to DOS
without a message early in a battle - seen twice (night 'cutthroat thieves', bandit-soldiers ambush;
evidence build/qa2/sess/T/a.png, b.png). The all-drafts builds func3/func4 ran ~10 battles to the
end without this, so it is probably an interaction of these two drafts with the asm s014 siblings
(calls/data between s014 files); bisect with mkbuild.sh (0e76 alone, 19cd alone).
Loot retest still OPEN: no won fight against human enemies with func4 in this round (the party lost
or the fights were against animals); the three fixes are verified only statically (cdis/farcheck/
ptrcheck: the code now addresses 2e38:0bde+i*0x60 and the D_c291 pointer like the original).
lockstep.py now keeps DOSBox open after the game exits (no 'exit' in the conf; a quit shows the DOS
prompt) and logs DOSBox's output to $LOCKSTEP_DIR/NAME/dosbox.log.
Tools added (tools/playtests/): mkbuild.sh (overlay-only build in its own work dir, parallel-safe),
hunt.py (travel until an encounter card), battle_start.sh (start + Quickstart + leave city + hunt),
cdis.py (full disassembly of draft vs original per function), constcheck.py, farcheck.py, ptrcheck.py.
Recommendation (tools/, not changed here): stamp.py --wip / fmatch data placement should refuse a CONST
placement that overlaps another stamped file or that differs from the slots the original code loads.

# QA round 3 (2026-10-09, build/qa4/func.EXE = full functional build incl. root/root_0000.c, vs original)
Sessions/shots: build/qa4/sess (O = original, X = func, Y = b0e76.EXE, Z = b19cd.EXE).
4. FIXED (shop stock, s032 barter.c sub_s032_1462_1d18): `volatile int size` keeps size in memory at
   [bp-2] like the original (read the stale slot, store the clamped size back), so from the 2nd call on
   the city size - 1 is used again. fmatch still 8/11 (1d18 now 248/244: frame 8, q not in DI); module
   1e0c -> 1e0e bytes (fits); re-stamped --wip, constcheck ok. Verified with market_stock.txt on the
   Freiberg save (build/qa3/shots/morning): herbalist Pure Gold/Aqua Regia/Zinken/Choleric Base and goods
   merchant Torch/Rope/Lantern/Flute, prices/qualities/weights identical to the original.
Root game shell (root_0000.c), all identical to the original: main menu + "The Story Continues" load;
F10 Game menu (open, keyboard Down/Right/Return navigation, Difficulty submenu ticks, choosing Expert);
Alt-D cycle message "Difficulty is now set to Basic"; Alt-C "Show saintly changes OFF"; Pause from the
menu ("Game Paused / Press a key to resume", resumes on a key); Alt-Q "Quit to DOS? y/n": n returns, y
exits to the DOS prompt (DK.TMP removed, temp files identical to the original's).
Not a bug: the difficulty/toggle message boxes stay ~3 s (catch them with a shot <1 s after the key);
Alt-D presses that got through unseen cycle the difficulty, so check the tick in the submenu.
Not a bug: the copy-protection book (s041) also shows after losing a battle (party stripped, "Well, now
what?"); any stray key answers it wrongly and the game quits to DOS (happened to X once).
Combat with func.EXE: wolves (won, both nights), spiders (won), bandit-soldiers ambush (lost), witch +
wolves at the sod hut (lost): every battle starts with party in view, paused, normal sidebar, runs to
the end card; no crash. LOOT AFTER A WON FIGHT AGAINST HUMANS: STILL UNTESTED - the unattended
Quickstart party lost both human fights (it needs a tester issuing orders, or a stronger party/save).
Quit-to-DOS bisect (drafts as of ~22:10; s014_0e76.c/19cd.c were edited again at 22:51/22:55 by others):
  b0e76.EXE (only s014_0e76.c): boars, schrats, spiders (all lost): 3 battles to the end, no quit. b19cd.EXE (only s014_19cd.c): schrats (won, no loot - animals/schrats drop none),
  no quit. bloot.EXE (both) built but not run. The qa2 quit (loot.EXE) did not reproduce in either
  single-draft build; next: run bloot.EXE through several night/bandit fights.
New scripts (tools/playtests/): keyretry.py (press a key in one session until the screen changes - keys
reach the game only every 1-6 tries), autobattle.py (let a battle run, press space when paused, stop
at a card/loot screen or the DOS prompt).

# QA round 4 (2026-10-10): example mods + loot after a won fight vs humans
Mods (mods/<name>/README.md: what, file/function, build, result). Each was built with
`build.py --mod --overlay mods/<name>` and play-tested against the original (lockstep_sync.py, same city).
Evidence: build/mods_play/evidence/.
- swift_travel (src/wip/s030/overland.c, sub_s030_1462_2ac8 minutes per step / 3): OK. Same walk east of
  Bromberg: the original's Party Information shows Aug 27 Matins 2 and the mod Aug 25 Compline 23 (the walk
  took ~1/3 of the time); the moon icon differs too.
- cheap_inn (src/wip/s036/s036_1462.c, vec_2099 lodging price halved through a wrapper + #define): OK.
  Residence: "at 2gr, 0pf ... leave 0fl 8gr 10pf" vs "at 1gr, 0pf ... 0fl 9gr 10pf"; after a day 8gr10 vs 9gr10.
  The new label is a `static char far` string + _fstrcpy into a local buffer, and it displays correctly.
- heavy_blows (src/wip/s014/s014_18bf.c, sub_s014_18bf_08ca: damage x2 when t >= 5): OK. Bandit-soldiers ambush,
  unattended Quickstart party: the original lost, the mod won (hits of -20), and the loot screen was normal (real names).
- strong_party (src/wip/s001/s001_0e76.c: attributes 60, skills 75 for the 4 Quickstart members): OK
  (character sheet 60/60, 75; load 57/120).
LOOT AFTER A WON FIGHT VS HUMANS (the open item from rounds 2/3): OK in the full functional build.
build/mods_strong_func2/DARKLAND.EXE (68 drafts + strong_party) vs build/mods_strong_party/DARKLAND.EXE (plain
--mod + strong_party), started in the same second. The bandit ambush near Vordingbord ("Ragged but determined
bandits...", press onward) was won by both. Functional loot screen: "The party finds 0fl, 5gr, 10pf in cash",
Long Sword (5) 15-Qual, V:Leather (4), L:Leather (4), V:Scale (1). "Get an item" moved a Long Sword to Gretch
(count 5 -> 4; it is listed under "Gretch currently has..." and after Leave in the Equipment list as "Long Sword
15q (1)"). Party Information wealth: 10gr + 5gr10pf = 15gr 10pf (correct). The plain build behaves the same way
(Club/Leather/Leather/Long Sword, 7gr 1pf).
The battlefields of the two sessions differed from the first frame, so the two fights (and loot) were not identical.
Not verified: a frame-for-frame same-seed fight, and Distribute/Put on the loot screen.
Observation (not confirmed as a bug): after these different fights, the Vordingbord gate card says "you will
be respected here" in the plain build but "unknown here" in the functional build. This could be a
reputation change after the fight (card script / s029 rule draft), or just the different fights. Check it
with a fight that is identical in both builds.
Bugs / tool problems:
5. OPEN (tools/build.py): `--mod --functional --overlay DIR` lets the src/wip drafts OVERRIDE the mod.
   The wip list is appended after the user overlays (build.py ~line 177, `a.overlay + [('__list__', wip)]`, last
   one wins). The log says "overlay: .../src/wip/s001/s001_0e76.c replaces mods/strong_party/s001/s001_0e76.c", and
   the game started with the normal party. Fix: put the wip list first. Workaround:
   tools/playtests/mkfuncmod.py (copies the functional draft set + the mod into one --overlay dir).
6. OPEN (far strings in mods): `static char far s[] = "..."` needs a segment word in the module's CONST. It
   works in s036 (the draft has a spare CONST slot). In a module whose CONST is full it fails. With s014_18bf.c
   the exact error is: "build/mods_fartest/mod/s014/s014_18bf.c: CONST data grew from 22 to 24 bytes, but
   DGROUP is full; move new strings to far data (static char far s[] = "...";)". The message suggests the very
   thing that failed. Code-segment strings (`static char _based(_segname("_CODE")) s[] = "...";`) compile
   but the link fails: "omflink.LinkError: build/mods_fartest2/cobj/P_S036_1462_0000_....obj: overlapping data
   at s036:156fc" (also s014:19590). cport/build place only the code record, not the extra _CODE data after it.
   The CARDS.md worry about X5_DATA did not come up: the far segment is accepted and placed after the code.
   Workaround for full-CONST modules: fill a local char buffer one character at a time (no data at all).
7. Tester hazard: `lockstep.py stop NAME` without LOCKSTEP_DIR uses build/qa/sess/NAME.json. This round a
   stale O.json from 2026-10-09 19:00 got its PIDs 26934/26925 killed (whatever process they belonged to at that
   moment) and its /tmp/.X77 lock removed. Always export LOCKSTEP_DIR. Better: lockstep.stop should check that
   the PID is still a dosbox/Xvfb process.
Same-seed battles: orig vs orig (bandit ambush) and orig vs a build with only the UNMODIFIED s014_18bf draft
(build/mods_ctl18bf, mkbuild.sh; schrats) had identical battlefields and identical first frames. So the
battlefield differences above (orig vs heavy_blows, plain vs functional strong_party) came from the mods (the
party or the damage) or from input timing, and not from the s014 relayout. In the ctl18bf schrats fight the
end state differed a little (Gretch STR 7 vs 12, Hans 2 vs 1; both lost). That is probably autobattle's
per-session space timing, and it is not confirmed as a draft bug.
Functional build + strong_party + heavy_blows (build/mods_combo_func, mkfuncmod.py with both files): a gargoyle
fight ran 24 autobattle rounds to the defeat card. No crash, normal sidebar and messages ("Edged Wpns changed
from 75 to 76"). The party lost: the unattended party only fights back slowly.
Mod build notes: a mod build takes 3.5-8 min (2 CPUs shared). Copy build/qa2/analysis + force_db.json into each
--work dir (as mkbuild.sh does). The s014 change moves all children s015-s025 (relayout), and the game still
plays normally.
New in tools/playtests/: mods.txt (recipes for the mod play-tests), mkfuncmod.py (functional build + mod).
