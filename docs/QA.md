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
