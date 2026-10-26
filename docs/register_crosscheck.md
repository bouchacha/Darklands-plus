# Darklands Restoration's bug register, cross-checked

Arkana Mechanika keep a register of the bugs of 483.07 that their C# rewrite reproduces
(`docs/faithfulness/original_bugs.csv` in github.com/Arkana-Mechanika-Studios/darklands-restoration;
155 entries in October 2026, ids B-xxxxxx). Each was checked against the decompiled code here. This
is where Darklands+ stands on each: "fixed now" means fixed after the cross-check, "experimental"
that it is fixed but only reachable inside the experimental city politics or cities at war. The
fixes are described in `port/mods/fixes.c` (its header and each hook), the saints in `docs/SAINTS.md`.

| Id | Where (483.07) | Their name | Darklands+ | Switch |
|---|---|---|---|---|
| B-126118 | 18C8:0004 | CharacterCreation.Back | fixed now | fix.createback |
| B-047d71 | 0E76:4650 | GameState.HanseStanding | fixed | fix.standings |
| B-9224b7 | 1462:1630 | BrewingBench.TakeFromParty | fixed | fix.alchemy |
| B-9d2829 | 1462:1EC2 | Brewing.CallOff | fixed (the symptom) | fix.nullitem |
| B-459e15 | 1462:1D24; 1462:0082; 1462:147A | CampStay.TeacherList | fixed | fix.teachers |
| B-c0e7af | 1462:147A | CampStay.DoTheActivity | fixed | fix.teachcap |
| B-8a1b71 | 1462:1B80 | CampStay.GrowRisk | fixed | fix.campguard |
| B-ea8f8b | 0E76:4180 | CityPolitics.DailyPolitics | fixed (experimental) | rules.politics |
| B-074e99 | 1462:197E | DayJobs.static | fixed now | fix.dayjobs |
| B-b28ef4 | 18E7:171C | Inventory.BestQualityByFlags | fixed | fix.itemflags |
| B-d5aab5 | 18E7:01B4 | Inventory.AddStack | fixed | fix.partyitems |
| B-d0865f | 18E7:01B4 | Inventory.AddStack | fixed | fix.fullpack |
| B-e458c5 | 18E7:0AB8 | Inventory.KeepEach | fixed | fix.itemcount |
| B-42ba6c | 0E76:1DFE | LocationRules.Set | confirmed, unreachable |  |
| B-55afd6 | 1462:00BA | LocationRules.Hostile | fixed now | fix.cityalert |
| B-e63363 | 1462:28F2 | LocationRules.FarOfType | fixed | fix.altar |
| B-dc949d | 1462:0000 | LootSharing.ReplaceFallenLeader | confirmed, unreachable |  |
| B-a84745 | 0E76:01C0 | MemberRules.BestWeaponSkill | fixed | fix.bestweapon |
| B-2ffc47 | 0E76:0B84 | MemberRules.SetAttributeMaximum | confirmed, harmless |  |
| B-308359 | 0E76:08F2 | MemberRules.HealAttribute | fixed | fix.healboost |
| B-8b2837 | 0E76:4C04 | MemberRules.RaiseAttributeTemporarily | fixed now | fix.blessing |
| B-6eb045 | 0E76:4C04 | MemberRules.RaiseAttributeTemporarily | fixed | fix.blessing |
| B-c14d7c | 0E76:4B08 | MemberRules.RaiseSkillTemporarily | confirmed, unreachable |  |
| B-014a8d | 0E76:1696 | PartyQueries.MinAttributeSum | confirmed, unreachable |  |
| B-82a302 | 0E76:179C | PartyQueries.MaxAttributeSum | fixed now | fix.attrsum |
| B-3dcbd1 | 0E76:2278 | PartyRules.BattleSeed | confirmed, harmless |  |
| B-11a75a | 1462:10AE; 1462:43B2; 18E7:0474 | PotionEffects.Drink | fixed | fix.potions |
| B-23a975 | 1462:10AE; 1462:43B2; 18E7:0474 | PotionEffects.Drink | confirmed, kept |  |
| B-555b7a | 1462:10AE; 1462:43B2; 18E7:0474 | PotionEffects.Drink | fixed | fix.greatpower |
| B-d0b264 | 1462:10AE; 1462:43B2; 18E7:0474 | PotionEffects.Drink | fixed | fix.armourday |
| B-87887e | 1462:10AE; 1462:43B2; 18E7:0474 | PotionEffects.Drink | fixed | fix.armourday |
| B-82a169 | 1462:1E90 | Practice.ChargeFame | confirmed, unreachable |  |
| B-537f55 | 165C:1024 | SaintBook.StandingHereIfCarrying | fixed | fix.saintrep |
| B-6878fe | 165C:41FA | SaintBook.PragueStanding | fixed | fix.saintrep |
| B-b81c7e | 165C:0034 | SaintBook.new | fixed | fix.sainttexts |
| B-22d300 | 165C:0190 | SaintBook.new | fixed | fix.sainttexts |
| B-747355 | 165C:0792 | SaintBook.new | fixed | fix.sainttexts |
| B-49f1ff | 165C:0B82 | SaintBook.new | fixed | fix.sainttexts |
| B-defa50 | 165C:0C20 | SaintBook.new | fixed | fix.sainttexts |
| B-315695 | 165C:125A | SaintBook.new | fixed | fix.sainttexts + fix.armourday |
| B-623452 | 165C:13D4 | SaintBook.new | fixed | fix.sainttexts |
| B-4d4c99 | 165C:1868 | SaintBook.new | fixed | fix.armourday + fix.sainttexts |
| B-44e795 | 165C:1B02 | SaintBook.new | fixed | fix.sainttexts |
| B-5eb6aa | 165C:1CF2 | SaintBook.new | fixed | fix.sainttexts |
| B-465f6c | 165C:1F8C | SaintBook.new | fixed | fix.sainttexts |
| B-243faa | 165C:2052 | SaintBook.new | fixed | fix.armourday + fix.sainttexts |
| B-32871d | 165C:2208 | SaintBook.new | fixed now | fix.sainttexts |
| B-9b19b6 | 165C:23FE | SaintBook.new | fixed | fix.sainttexts |
| B-f32e0e | 165C:2628 | SaintBook.new | fixed | fix.sainttexts + fix.armourday |
| B-e8ecd2 | 165C:27C2 | SaintBook.new | fixed | fix.sainttexts |
| B-3947bd | 165C:76D4 | SaintBook.new | fixed | fix.sainttexts |
| B-f995af | 165C:2D58 | SaintBook.new | fixed | fix.sainttexts + fix.armourday |
| B-ec67c9 | 165C:2ECE | SaintBook.new | fixed now | fix.sainttexts |
| B-f2531c | 165C:307E | SaintBook.new | fixed | fix.sainttexts |
| B-41e624 | 165C:3612 | SaintBook.new | fixed | fix.sainttexts |
| B-2e94a4 | 165C:74EA | SaintBook.new | fixed | fix.sainttexts |
| B-cced91 | 165C:3FC2 | SaintBook.new | fixed now | fix.sainttexts |
| B-64dd08 | 165C:4320 | SaintBook.new | fixed | fix.sainttexts |
| B-48bb81 | 165C:43D8 | SaintBook.new | fixed now | fix.sainttexts |
| B-7fe039 | 165C:49E4 | SaintBook.new | fixed | fix.sainttexts |
| B-d7771e | 165C:4C46 | SaintBook.new | fixed | fix.sainttexts |
| B-38e50e | 165C:4D1A | SaintBook.new | fixed | fix.sainttexts + fix.armourday |
| B-06bdba | 165C:5296 | SaintBook.new | fixed | fix.sainttexts |
| B-723764 | 165C:5334 | SaintBook.new | fixed | fix.sainttexts + fix.armourday |
| B-a1a2bc | 165C:563A | SaintBook.new | fixed | fix.sainttexts |
| B-ce6144 | 165C:5A84 | SaintBook.new | fixed | fix.sainttexts + fix.armourday |
| B-a19878 | 165C:5B82 | SaintBook.new | fixed | fix.armourday + fix.sainttexts |
| B-148934 | 165C:5CCA; 165C:5D6C | SaintBook.new | fixed | fix.pauls |
| B-44772a | 165C:6122 | SaintBook.new | fixed | fix.armourday |
| B-99f176 | 165C:664A | SaintBook.new | fixed | fix.sainttexts |
| B-b8a1ad | 165C:671E | SaintBook.new | fixed | fix.sainttexts |
| B-bf6543 | 165C:6AE8 | SaintBook.new | fixed | fix.sainttexts |
| B-78a26c | 165C:6BB8 | SaintBook.new | fixed | fix.sainttexts |
| B-29f615 | 165C:7038 | SaintBook.new | fixed | fix.sainttexts |
| B-e64add | 165C:7288 | SaintBook.new | fixed | fix.sainttexts |
| B-61d9a9 | 165C:7342 | SaintBook.new | confirmed, kept |  |
| B-94efea | 0E76:1302 | Saints.Learn | fixed now | fix.patronsaint |
| B-7c4eb6 | 1462:10D2 | Storing.AddOne | fixed | fix.storeroom |
| B-2cf199 | 1462:0A46 | Storing.Get | fixed now | fix.fullpack |
| B-700b91 | 1462:1D18 | Trading.ShopHasItem | confirmed, harmless |  |
| B-fd421f | 1462:0F60 | Trading.Buy | fixed now | fix.fullpack |
| B-ed73b9 | 194B:0784; 194B:0736 | SideStreetAtNight.Run | fixed now | fix.cardslips |
| B-79145e | 1A44:0000 | CityWallsByNight._climbing | fixed | fix.nightwall |
| B-edab45 | 1A44:0B02; 1A44:0AE4 | CityWallsByNight.EveryoneChance | fixed | fix.nightwall |
| B-86d79f | 190E:04EE | RevolutionHouseAtNight.KnockChance | fixed (experimental) | rules.politics |
| B-9a79dc | 190E:03DE | RevolutionHouseAtNight.KnockAtTheBackDoor | confirmed, kept |  |
| B-82b312 | 190E:06F6 | RevolutionHouseAtNight.UseAPotion | fixed (experimental) | rules.politics |
| B-b4fddb | 1838:03D6 | RevolutionHouseByDay.GatherGossip | fixed (experimental) | rules.politics |
| B-0d7783 | 1838:0916 | RevolutionHouseByDay.UseAPotion | fixed (experimental) | rules.politics |
| B-169e5e | 1A06:0518; 1A06:0800 | FortressByDay.ReceiveACommission | fixed | fix.lordreward |
| B-86c8e5 | 1A06:05B8 | FortressByDay.SeekService | fixed | fix.appointment |
| B-a73a8e | 19F2:1276; 19F2:1258 | Docks.PassageHint | fixed now | fix.cardslips |
| B-e95c32 | 18C9:05D0 | InnByDay.VisitTheStables | fixed | fix.cardslips |
| B-ba6123 | 1924:0A5E | ClothmakersStreet_Cloth00.SneakIn | fixed now | fix.cardslips |
| B-556a53 | 1838:0C36; 1838:0C72; 1838:0CAE; 1838:0CEA | Footpads.ThrowAPotion | fixed | fix.cardslips |
| B-227a55 | 19DF:0888 | CouncilHall.AskForAnErrand | fixed | fix.appointment |
| B-0c78c7 | 1932:034C | CityWallSallyPort.ForceChance | fixed (experimental) | rules.war |
| B-83f56d | 1932:0196 | CityWallSallyPort.BlastWithPotion | fixed (experimental) | rules.war |
| B-77afe4 | 19C4:0EC2; 19C4:0E0A | CathedralByDay.AskForARelic | fixed | fix.bishop |
| B-7fade3 | 19C4:0EC2; 19C4:0E0A | CathedralByDay.AcceptTheRelic | fixed | fix.bishop |
| B-2fec94 | 18C0:0494 | University_Unive00.AskAboutFormulas | fixed | fix.university |
| B-d361c3 | 18D7:0342 | ChurchAtNight_CityC01.AskAboutMass | confirmed, unreachable |  |
| B-42f89c | 1838:0000 | CityChurchByDay.Run | fixed now | fix.cardopts |
| B-4d8935 | 1838:0956; 1838:0948 | CityChurchByDay.Confess | fixed | fix.confession |
| B-f30456 | 1838:0956; 1838:0948 | CityChurchByDay.GiveATenth | fixed | fix.membervalue |
| B-d962b2 | 1A02:0282 | HandbillThugs_Polit01.Fight | fixed (experimental) | rules.politics |
| B-0818ac | 191A:09BA | LeavingByTheGate.SettleStanding | fixed now | fix.cardslips |
| B-77a46d | 1A1F:1064 | LeavingByTheWall.Run | fixed | fix.cardopts |
| B-10641a | 1B0B:0008 | AlchemistFormulaSeller.FormulaOnOffer | confirmed, kept |  |
| B-dac2b0 | 1A29:02AC | BowyersGuildAtNight.KnockToBuy | fixed | fix.nightbowyer |
| B-ce730c | 19F9:0812; 19F9:0814 | ClimbingTheWallPastTheGuards.Run | fixed | fix.cardopts |
| B-fa0f93 | 1992:0624 | AffairsElsewhere.Run | confirmed, unreachable |  |
| B-31b0d3 | 1838:02A8 | NightInTheGuildLeadersHouse.RaiseTheHouse | fixed | fix.membervalue |
| B-2ac103 | 1981:0346 | BeforeTheCityWalls.Disgrace | confirmed, kept |  |
| B-097711 | 1A7E:0850 | SallyPortInTheCityWall.Run | fixed (experimental) | fix.cardopts + rules.war |
| B-23f155 | 1996:0378; 1996:0484 | VillageSchulz.AskForHelp | fixed | fix.cardslips |
| B-132edf | 1870:02EC; 1870:0324 | RobberKnightTaskRewarded.Run | fixed | fix.cardslips |
| B-166488 | 1870:02EC; 1870:0324 | RobberKnightTaskRewarded.Run | fixed | fix.cardslips |
| B-19435f | 1838:0536 | WolfPackOnTheTrail.RidingChance | fixed | fix.itemflags |
| B-ee49f7 | 18CE:02FC | CityLordsReward.Run | fixed (experimental) | rules.politics |
| B-418c61 | 18CE:02A8 | CityLordsReward.LearnTheFormula | fixed (experimental) | rules.politics |
| B-65bf20 | 18E0:08E6 | RebelMarch.Run | fixed (experimental) | rules.politics |
| B-45b1b4 | 1999:000E | RebelsBeforeTheFortress.Run | fixed | fix.cardopts |
| B-4df3ba | 188F:0222 | MerchantsOfTheCity.Trade | fixed | fix.bounty |
| B-805211 | 19BC:03E8 | SmallCaveUnderTheLedge.FightTheTatzelwurm | fixed | fix.cardslips |
| B-c6aabd | 1838:07CA | CryptUnderTheRuinedChurch.TakeTheItem | fixed | fix.tomb |
| B-7bfead | 18D3:041C | HussiteBand.ClaimFriendship | fixed | fix.cardslips |
| B-08a606 | 1A2B:02AC | TemplarWritingDesk.LearnAFormula | fixed | fix.cardslips |
| B-1eb8d6 | 18F7:0000; 18F7:079A; 18F7:07D6; 18F7:0812; 18F7:084E; 18F7:088A | CollectorsCommission.Run | fixed | fix.debtnote |
| B-586670 | 1838:03B8 | StrangerBringsAMob.FightAWave | fixed | fix.cardslips |
| B-759d5e | 190C:033E | CavernOfFireAndHail.CreepAcrossUnderArmour | fixed | fix.cardslips |
| B-f2c259 | 1838:0550; 1838:0692 | SpiderWarehousePoster.EnterTheWarehouse | fixed | fix.cardslips |
| B-ace31b | 0E76:3FB2 | EventExpiry.AlchemyResult | fixed | fix.alchrisk |
| B-024865 | 0E76:448A | EventExpiry.UndoBoost | confirmed, unreachable |  |
| B-2c6441 | 0E76:448A | EventExpiry.UndoBoost | fixed | fix.potions |
| B-dcf1be | 0E76:448A | EventExpiry.UndoBoost | fixed | fix.blessing |
| B-e1b48f | 1462:0100 | EventLog.Reload | fixed | fix.battlesaints |
| B-8f44e7 | 0E76:2C4E; 0E76:3034 | EventLog.Create | confirmed, kept |  |
| B-8d31a1 | 0E76:2E74 | EventLog.Update | confirmed, kept |  |
| B-2e264a | 18BF:0778 | Attacks.HitDamage | fixed | fix.fireward |
| B-2f5bf6 | 18BF:0778 | Attacks.HitDamage | fixed | fix.oilweapons |
| B-0d81e6 | 18BF:0778 | Attacks.HitDamage | fixed | fix.potions |
| B-08f8dd | 18BF:08CA | Attacks.QueueDamage | fixed | fix.pileup |
| B-a044f7 | 0E76:1D90 | BattlefieldForces.FixGroups | fixed | fix.enemygroups |
| B-8fdd39 | 15AF:1D78 | CastleGenerator.PlaceParty | fixed | fix.castlefreeze |
| B-c90740 | 15AF:0698 | CastleGenerator.OpenExitsAndDecorateWalls | fixed now | fix.corridors |
| B-ab0e37 | 1432:096A | FortressBattlefield.Fortress.SetUpRoom14 | confirmed, kept |  |
| B-e080d4 | 19CD:33C0 | LootStore.Merge | fixed now | fix.lootcount |
| B-fd2f1c | 1EF8:0914 | EffectField.DropWornOut | fixed | fix.armourwear |
| B-8e79e5 | 1EF8:0968 | EffectField.WearItem | fixed | fix.armourwear |
| B-3cc78a | 1EC9:0142 | EffectField.LineCrossesCloud | fixed now | fix.cloudline |
| B-bfb04e | 1EC9:0142 | EffectField.LineCrossesCloud | fixed now | fix.cloudline |
| B-61f351 | 0E76:00DC; 1EF8:059A; 0E76:0694 | TacticalBattle.WriteBattleFile | fixed | fix.castle |
| B-14d327 | 1C7D:09C6 | DarklandsGame.LoadInGame | avoided | saves.quick,  display.native_saves |
| B-f7799c | 03A1:000A | DarklandsGame.FightTheBattle | fixed now | fix.battledead |
| B-d7cb44 | 14F7:31E6 | CombatantSetup.BackToMember | confirmed, harmless (never read as a location; see uncertain_crosscheck.md) |  |

## Not on the register

Bugs of 483.07 that Darklands+ fixes and the register does not list (also as CSV: `docs/not_in_register.csv`).

| Where (483.07) | Area | What the original does | Darklands+ | Switch |
|---|---|---|---|---|
| DARKLAND.LOC record 373 | data | One location (a cave) lies at y 931, one row below the 931-row map: quests that send the party there cannot be finished. | Moved onto the last row the party can step on (929: a step keeps the marker off row 930). | fix.offmap |
| s036 1462:0082 | screens | The lodging screen disables Save and Load (DS:9920, DS:9934 = 2) and re-enables them on most exits but not all (a click on an empty party slot returns at once): Save/Load stay grey until a character sheet is opened. | Enabled again on the way out. | fix.innsave |
| (not located) | screens | The 'neon hair' bug: characters' colours (and figures) change after encounters. | We only watch and put them back each frame; the cause is not found. A save that shows it would help. | fix.colours |
| s044 1838:0000 ($Initi00, card 0) | cards | A card that goes back to where it came from (prev_screen) goes to card 0 when it came from none; card 0 is the developers' test card, whose picture (pics\xhouse3.pic) is missing: the game stops. | Goes on to the map or the town. | fix.testcard |
| s053 1924:0004 ($Cloth00) | cards | Clothmakers' Street by day hides its shopfronts (opt_flag_0 = 0), unlike every other guild street by day; its handler 02be would open the clothmaker's shop as at night. | Shown. | fix.cardopts |
| s134 1838:0000 ($DPost00) | cards | The spider warehouse's owner is $NamedOneName, which nothing on the way sets: 'I am . Are ye hardy adventurers?' (or whoever was named last). | Given a name. | fix.cardslips |
| s084 1838:0536 ($meetw00) | cards | Besides item_defs[0] (register B-19435f), the mask asked for is 0x02000000, which no item has; the mounts' LST flag is 0x2000. | Mapped to 0x2000. | fix.itemflags |
| s039 165c:2208 (St. Edward) | saints | The price, a quarter of the money, is money_pfennigs() cast to unsigned and shifted: a quarter of the purse cut to 16 bits, so a party with more than 273 florins pays less. | A true quarter. | fix.sainttexts |
| s039 165c (39 saints before the register, 43 now) | saints | Where a saint's coded help departs from DARKLAND.SNT; the register has most of them. Not on it: St. Vitus (6e62), St. Martin of Tours (50cc), St. Florian (2b30) and St. Hildegard (3934). | docs/SAINTS.md has the table, saint by saint. | fix.sainttexts |
| s039 1462:0558 | rules | The season's own record (kind 8) doubles as fact 0 (loc_fact matches kind 8 too) and the setup fills it with the city's number instead of 0: the agitators' guild master never offers the job (except in city 0). | Set to 0 after the setup. | rules.politics |
| s039 1462:0558 | rules | No guild leader is ever given a secret: the setup never fills fact 8, only the cards clear it. | About one in three gets one. | rules.politics |
| s039 1462:0558 | rules | The agitators' guild (fact 0x17) can be the local merchants, who have no guild card; and the setup loops for ever in a city without a guild street. | Moved to a guild with a street; such cities get no season. | rules.politics |
| s039 1462:conflict_outcome_impl | rules | conflict_outcome marks the city overthrown (+0x14 /= 0x80) whoever won, and checks whether the party deserted (facts 0x24, 0x25) after clearing the season's facts. | The first put right; the second left. | rules.politics |
| $Revol04 (0xbf), $Revol09 (0xb8), $Snuck00 | cards | $Revol04 never sets option 0, shows four unwritten lines ('5' to '8') and does nothing for 'government activities' and 'rebel activities'; $Revol09 shows lines '2' to '5'; $Snuck00 a blank line whose handler is empty. | Hidden or written. | rules.politics |
| $Consi00 (s072 1838) | cards | The war council never shows its first text (the tally) and fills the option table with stubs ('Sorry, this option isn't working yet.'). | Written from its texts. | rules.politics |
| $CityN00 (s069 195d:0358) | cards | 'Discuss the political situation here' goes to the general news, like 'gossip about the situation': the street gossip ($Revol04) has no way in. | Leads to $Revol04 during a season. | rules.politics |
| $Polit01 (s057 1a02:053c) | cards | 'Eat your handbills': the thugs walk off but the card asks again; the party is never let go. | Goes on to the street. | rules.politics |
| $Revol13 (s090) | cards | The bribe's flag is overwritten with the saint's (register B-65bf20, from the other side): no money needed. | Needs the money. | rules.politics |
| s029 0e76:4180 | rules | The season's day (register B-ea8f8b): besides cur_loc, the strike it sets (facts 0x15/0x16) lasts 480 hours, is made again every week while it holds, and nothing settles it if the party is away. | A day like the war council's: the next day, twelve hours, settled when the party comes back. | rules.politics |
| byte +0x14 of a city's location | rules | Cities preparing for war (1) and at war (2) are read all over (prices, the news, Soldier's Road, the docks, the war's city gate, card 0x73) but nothing sets them, and DARKLAND.LOC has 0 for every city. | port/mods/war.c; its cards' own slips listed there. | rules.war |
| s072 1981 (card 0x73, the war's city gate) | cards | The card names $CityE00 and loads no card at all, so its menu is the last card's and its texts 7 to 9 do not exist. | Given its text. | rules.war |
| s014 0e76:2cca (the approach); 0e76:1624 (cbt_step) | battles | A walker re-plans every 30 to 120 ticks without a good step, but the step it had lined up from the old route (D_ca30[i]) is still taken first, and the new route then goes on from its second step: one step off the new route, often into somebody. And a walker blocked by somebody waits for that re-plan time (up to 2.5 s), though the one in its way stopped to fight. | The new route starts from its first step; a blocked walker re-plans within 12 to 24 ticks, around whoever blocked it. | fix.pathfinding |
