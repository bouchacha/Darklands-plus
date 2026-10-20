# The C# rewrite's uncertain readings, cross-checked

Arkana Mechanika's C# rewrite of Darklands (github.com/Arkana-Mechanika-Studios/darklands-restoration)
marks each guess about the original 483.07 with an `// UNCERTAIN:` comment: what a field means, what a
function does in a case its authors could not see. In October 2026 there were 304 of them. Each was
checked against the decompiled code here (the matching C in `src/c/`, the drafts in `src/wip/`, the
original's disassembly in `build/asm/`, and the game's data), by reading only.

The answers are in [`uncertain_crosscheck.csv`](uncertain_crosscheck.csv), one row per comment, in the
order of their register: `id` (theirs), `area`, `file`, `line` and `member` (where the comment is in
their code), `original` (the address it concerns), `verdict`, `answer`, `evidence` (files and lines
here).

| Verdict | Rows | Meaning |
|---|---|---|
| confirmed | 225 | their reading is right |
| partly | 40 | right in part; the answer says which part |
| corrected | 29 | the original does something else; the answer says what |
| unknown | 10 | reading the code cannot settle it (a value left on the stack, a DOS or driver routine outside the game); the answer says what would |

## The corrections that matter most

- **The to-hit rolls** (s014 18bf:02a8): a blow (an order with bit 2: Std Attack, Vulnerable,
  Berserk, Parry) goes to 18bf:0592, which reads combatant +0x20 on both sides (the "fighting value",
  attack and defence hand to hand) and the smaller shield values; a shot or a throw is 02a8's own
  branch, with +0x21 (the "aiming value") against the larger shield values and twice the Agility.
  Their "fighting/aiming value" names are right; this repository had 0592 named `missile_to_hit`
  (now `melee_to_hit`), and Darklands+ followed the wrong name in its battle info and tooltips
  (fixed, see CHANGELOG.md). +0x24 is fire protection, not "piercing resistance".
- **DS:A76C/A76E/A770** are the three enemy groups' qualities (battle_run's own log calls them fqual),
  not types; the fortress's "quality 2" picks each kind's next weaker template, groups keep their size.
- **Battlefield level records** +0x3350 / +0x3382 are not door tables: the original's log calls them
  FurnRemove and WallRemove (trigger zones that hand control to a card, then remove furniture or a
  wall); +0x300E holds the level's chests.
- **DS:160C** is the citadel court's state in the last castle (the door the party comes back through,
  and the rooms finished), the same variable as their separate CourtProgress.
- **DS:A891** is never 1 when a card reads it ($Raubi02's `== 1` branch is dead): 3 means "came
  straight from the overland map".
- **The `$NamedOneName` table** (290E:2107) is a fixed table in the EXE's data, not filled at run time.
- **The sprite format's first byte** is the width itself (their `Width = byte0 + 1` is one too many).
- **A city record read with cur_loc -1** starts at the file's beginning, but the description read
  beside it does not: `city_desc` gets bytes 512..591 of DARKLAND.DSC (the end of Wismar's text).
- **Adding a character at an inn** recomputes +0x35 (the carried weight) and resets the slot's colours.
- **Sound driver commands:** 1 stops at once, 6 fades out, 2/3 mute and restore, 7 does nothing in the
  town drivers. (Darklands+'s music packs had 1 and 6 the other way round: fixed.)
- **Several "stale stack" values are fixed**, because the card loop calls an option's hint just before
  its action at the same stack depth (CityGateInWartime, AskForTheNextMass, RevolutionHouseAtNight);
  the brewing event's missing tenth argument is `alchemy_brew`'s saved SI, the key that started it.

## Behaviour of the original found on the way

Candidates for their register of original bugs (none of them changed in Darklands+ yet):

- $MineB01: the lock-pick line can never succeed (its hint returns -1, the action rolls
  `rand_mod(100) <= -1`).
- An outrun chance shown as 1% succeeds 2% of the time (a roll of 0 or 1).
- A card's text says the horses are driven away, but no horse is removed.
- The dwarf card's "hand over the potions" works only in the same visit (nothing creates the event it
  checks on arrival).
- The voyage cards 0x10C-0x10E are never reached, and would leave the party at the gate it sailed from.
- HussiteBand's potion pop-up runs the previous card's handler; GiantSpiderNest's setup clears the
  saints the card had just set (the saint line is greyed on arrival).
- JewelersGuildAtNight writes a skill value past the five party records 11% of the time.
- WildHunt and HandbillThugs: the unset chance is probably the card's saved frame pointer, negative, so
  the roll never succeeds.
- VillageGraveyard: block 0 has three option lines; the third follows a stale flag and leads to card
  0x62, which ends the game.
- StrangerBringsAMob's ointment offer between the Sabbat's waves has every line switched off: the
  screen waits for ever (Darklands+ already fixes this: fix.cardslips).

## Their register entry B-d7cb44

The battle writes the combatant's screen x into the party record's +0x00 word on every frame (the
combatant record is the party record), and nothing puts the location back. It is never read as a
location: the inn's roster reads +0x00 only for characters outside the party, whose word
`party_remove` sets to the town as they leave. Harmless aliasing, no visible effect. On the way it
showed a real fault in Darklands+'s own inn screen, which listed every character kept, out of step
with the game's list (fixed: see CHANGELOG.md).
