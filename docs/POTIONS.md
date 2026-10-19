# Potions: what the code does, what the manual says, what Darklands+ changes

Grades 1, 2 and 3 are the qualities 25, 35 and 45 a brew can have ("All potions are rated quality
25, 35 or 45", manual). Drunk and applied potions go through `use_potion` (s031 1462:43b2); thrown
ones through the battle's effect code (s019). "A day" in the code is an event of 19 to 21 hours
(`rand_mod(3) + 24 - 5`). The switches are in F12 > Settings > Helpers > Fixes.

| Potion (item) | 483.07 code | Manual | Darklands+ |
|---|---|---|---|
| Noxious Aroma (95) | thrown: a cloud up to 4×4; the one hit loses ½ of Agility, Perception and weapon skills, the others in it ¼, friend or foe | lowers agility, perception, skills; 5-15 s | as the code |
| Eyeburn (96) | thrown: one target −⅓ Agl/Per/weapon skills and blinded; neighbours −¼ | blinds, immobilises; 8-25 s | as the code |
| Black Cloud (97) | thrown: smoke; shots through it hit ⅕ as often, fighting inside −15; never offered in the party's battle menu | "cannot be used in battle" | as the code |
| Fleadust (98) | thrown: weapon skills −5 × (body + limb armour) | itching (the Clue Book gives the formula) | as the code |
| Eater-Water (99) | thrown: armour quality −(1d8 + 4/6/8); wounds through no armour; opens locks | quality −3 to 15 | as the code |
| Breath of Death (100) | thrown: poison cloud 2×2, armour no help, worse for the weak | a deadly mist | as the code |
| Sunburst (101) | thrown: every foe facing it blinded, friends spared | blinds enemies, 12-25 s | as the code |
| Thunderbolt (102) | thrown: 1d8 + 4/6/8, penetration 6/7/8, splash, wears armour | a cannon's blast, hurts armour | as the code |
| Arabian Fire (103) | thrown: 1d6 + 4/6/8 through armour (less a tenth of armour quality and of Firewall), wears armour | armour no help | as the code |
| Stone-tar (104) | thrown: a pool up to 4×4 slows to ½, ⅜, ¼ pace | slows; hardens after about 4 min | as the code |
| Deadly Blade (105) | member's bonus byte (party +0x34) high nibble +3/4/5: every hit with any weapon; a kind 0x4a event that is never undone, so permanent, stacking, wrapping past 15 | damage; all weapons but impact and flails; a day | `potions`: a day, capped at 15 |
| Strongedge (106) | low nibble +3/4/5 (penetration), likewise permanent; hit_damage's (bonus + weapon) & 15 wraps | penetration; blades and polearms; a day | `potions`: a day, penetration stops at 15 |
| Great Power (107) | adds 14/22/30 to the whole byte: +14 pen; +1 dmg +6 pen; +1 dmg +14 pen (with carries), permanent | the weapon's quality up, to 99; impact, flails, handguns; a day | `greatpower`: the quality the member's blows count (combatant +0x0d) +14/22/30, to 99, a day |
| Trueflight (108) | Bows, Thrown, Missile Devices +14/22/30 (a 0x49 event: wears off) | missile accuracy; a day | as the code |
| Hardarmor (109) | armour byte (party +0x41) low nibble +1/2/3, no event: permanent, stacking (past 15 into the fire bits) | armour quality +20-30 and thickness +1-2; a day | `armourday`: a day, to 15 |
| Transformation (110) | drunk: +1 florin; its use is on cards | purifies; metal into one florin | as the code |
| Truesight (111) | Perception +9-14 / 15-21 / 22-29 above the maximum (0x48: wears off) | 10-30; a day | as the code |
| New-wind (112) | Endurance +6-11 / 8-13 / 10-15 | 7-16; a day | as the code |
| Ironarm (113) | Strength +6-11 / 8-13 / 10-15 | 7-16; a day | as the code |
| Quickmove (114) | Agility +6-11 / 8-15 / 10-17 | 7-18; a day | as the code |
| Essence o'Grace (115) | heals 50/70/90 % of lost Strength (heal_attr_pct: from above the maximum it lowers, so Ironarm's boost was mostly lost), Endurance +9-29 for about 30 h | restores Endurance and Strength | `healboost`: healing only raises |
| Firewall (116) | fire protection (party +0x10) 50/70/90, no event: permanent; and +1 to the high nibble of the armour byte: +16 armour for good | armour against fire only; a day | `armourday`: a day, no high-nibble change; `fireward`: those bits never count as armour |

The switch "Oils as the manual says" (`battle.oils_manual`, off by default) leaves out, in each hit, what the
weapon used may not have by the manual (port/mods/battlerules.c).

Saints: St. Dunstan adds +1 and St. Patrick +2 to the same armour nibble as Hardarmor, for good
(`armourday`: a day). A battle puts the event list away (cards_shutdown, s043 1462:054e) and reads
it back after (cards_startup, 1462:078e) into its first slots, over what the battle added: a
saint's help called in a battle was never undone (`battlesaints`), and a potion's day ended with
the battle (the potion fixes carry those events over).

Online, players report what the code explains: "bows do 3-5 damage after Great Power" (the
overflow), Deadly Blade and Strongedge "never wear off", "Ironarm interacts poorly with Essence of
Grace", Firewall with St. Polycarp "never wears off, making your characters invulnerable".
