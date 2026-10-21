/* options.c - the table of Darklands+'s switches and the presets (options.h). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../runtime/config.h"
#include "options.h"
#include "mods.h"
#include "fixes.h"

/* the modules' setters, for the switches whose value a module keeps in a variable */
void menuview_set(int), titleview_set(int), cardview_set(int), barterview_set(int), sheetview_set(int), lootview_set(int);
void alchemyview_set(int), cacheview_set(int), lodgeview_set(int), prayview_set(int), createview_set(int), rosterview_set(int);
void infoview_set(int), marchview_set(int), enemyview_set(int), mapmsg_set(int), savesview_set(int);
void assist_set_chances(int), assist_set_best_leader(int), assist_set_card_help(int), assist_set_card_keys(int);
void autopause_set(int, int), autopause_set_foes(int), battle_speed_set(int), battle_fast_set(int);
void battlescroll_set(int, int), pathfix_set(int, int), travel_set(int, int), ammo_set(int), thrown_set(int);
void saves_enable(int), hdtext_set(int), hdart_set(int), sfx_set(int);
extern int autopause_down, autopause_hurt, battlescroll_on, battlescroll_rest, pathfix_party, pathfix_enemies;
extern int travel_enabled, travel_fast, orders_click_attack, loot_return_thrown;
extern int saves_auto_arrive, saves_auto_battle, saves_restart;

static void ap_down(int v) { autopause_set(v, autopause_hurt); }
static void ap_hurt(int v) { autopause_set(autopause_down, v); }
static void bs_paced(int v) { battlescroll_set(v, battlescroll_rest); }
static void bs_rest(int v) { battlescroll_set(battlescroll_on, v); }
static void pf_set(int v) { pathfix_set(v, v); }
static void tr_auto(int v) { travel_set(v, travel_fast); }
static void tr_fast(int v) { travel_set(travel_enabled, v); }
static void click_attack(int v) { orders_click_attack = v; }
static void return_thrown(int v) { loot_return_thrown = v; }
static void fast_key(int v) { if (!v) battle_fast_set(0); }
static void auto_arrive(int v) { saves_auto_arrive = v; }
static void auto_battle(int v) { saves_auto_battle = v; }
static void restart_end(int v) { saves_restart = v; }

static const int V_CHANCES[] = { 2, 1, 0 };
static const int V_PICTURE[] = { 0, 1, 2, 3 };
static const int V_CLOCK[] = { 0, 1 };
static const int V_SPEED[] = { 100, 150, 200, 300 };
static const int V_PACE[] = { 0, 1, 2 };
static const int V_RATE[] = { 25, 50, 100, 200 };

#define S_SCREENS "New screens"
#define S_CARDS   "Cards and towns"
#define S_PARTY   "The party and its items"
#define S_BATTLE  "Battles"
#define S_TRAVEL  "Travel"
#define S_CONTROL "Controls"
#define S_EDIT    "Editing tools"
#define S_OTHER   "Elsewhere"
#define S_RESTORED "Restored cut content"   /* their own page, F12 > Settings > Restored (experimental): off by default and in every preset */

/* key, section, parent, title, description, class, original, Darklands+, Easy, choices, values, apply, note, flags */
static Option OPTIONS[] = {
    { "display.native_panel", S_SCREENS, 0, "New party panel", "each member's Endurance, Strength and Divine Favor as labelled bars with numbers. Clicks work as in the original (open a character sheet; give orders in a battle). For a portrait of your own, put NAME.png (or .jpg) in a folder named portraits in the Darklands+ folder, NAME being the member's name.", OC_SHOW, 0, 1, 1 },
    { "display.panel_picture", S_SCREENS, "display.native_panel", "Picture in the party panel", "beside each member", OC_TASTE, 0, 0, 0, "Your portrait, else the battle sprite, else the shield emblem|The battle sprite, else the shield emblem|The shield emblem|No picture", V_PICTURE, 0, 0, OF_COMBO },
    { "display.native_menu", S_SCREENS, 0, "New menus", "the game's menus (Game, Orders, Attack, Party ...) open with a right-click or F10 and stay open until you choose or click elsewhere; the entries and what they do are unchanged. When off: the original menu bar, open only while the right button is held.", OC_SHOW, 0, 1, 1, 0, 0, menuview_set },
    { "display.native_title", S_SCREENS, 0, "New start screen", "Continue (loads the newest save), Load, New party and Quickstart, over the original game's art.", OC_SHOW, 0, 1, 1, 0, 0, titleview_set },
    { "display.native_cards", S_SCREENS, 0, "New card screens", "card text in the typeface you choose, the options as rows to click with their chances beside them; an option that uses an item or a saint lists who can. The game still decides every outcome. When off: the original cards (with crisp text drawn over them).", OC_SHOW, 0, 1, 1, 0, 0, cardview_set },
    { "display.native_shops", S_SCREENS, 0, "New shop screen", "all the wares and all your items in two lists, with Buy and Sell on each row and the items in use marked; the bottom of the screen describes whatever is under the pointer. Stock and prices are unchanged.", OC_SHOW, 0, 1, 1, 0, 0, barterview_set },
    { "display.native_sheet", S_SCREENS, 0, "New character sheet", "the paper doll and the items with Ready, Unequip, Drink and Drop, all the skills on one page, the party's saints and formulae. Drag an item onto a member in the party panel to give it (hold Shift to give one from a stack). Equipping, giving and drinking work as in the original.", OC_SHOW, 0, 1, 1, 0, 0, sheetview_set },
    { "inventory.shared", S_SCREENS, "display.native_sheet", "One shared pack for the party", "each member carries only what they use; Ready and Drink take from the shared pack, and selling in a shop takes from it. Spare items weigh nothing (only what is in use counts toward the load); potions stay with each member. When the game needs a particular member to have an item (lock picks at a lock, a mount, arrows before a battle), it is handed to them first. Keep marks an item never to be sold or dropped.", OC_PLUS, 0, 1, 1 },
    { "display.native_loot", S_SCREENS, 0, "New loot screen", "the whole pile and each member's items in two lists; a whole stack moves in one click. The original keys G and P still move every item.", OC_SHOW, 0, 1, 1, 0, 0, lootview_set },
    { "assist.loot_worth", S_SCREENS, "display.native_loot", "Shop value in the loot screen", "under the lists, about what a shop would pay for the item under the pointer.", OC_SHOW, 0, 1, 1 },
    { "display.native_alchemy", S_SCREENS, 0, "New alchemy screen", "every known formula in one list, with its ingredients at hand, batches set with - and +, and the chance of success. The chance and the brewing are unchanged.", OC_SHOW, 0, 1, 1, 0, 0, alchemyview_set },
    { "assist.where_sold", S_SCREENS, "display.native_alchemy", "Where to buy missing ingredients", "beside an ingredient that runs short, the nearest town seen selling it.", OC_SHOW, 0, 1, 1 },
    { "display.native_cache", S_SCREENS, 0, "New store room screen", "everything stored and everything the member carries in two lists, with Take and Put on each row (one or the whole stack), and Put away for everything not in use. When off: the original screen, four items at a time.", OC_SHOW, 0, 1, 1, 0, 0, cacheview_set },
    { "store.shared", S_SCREENS, "display.native_cache", "One store room shared by every inn", "what you leave with one innkeeper is there at every other inn, like a chest that follows the party (100 kinds of items at most). Items already stored in several towns are put together the first time a store room is opened.", OC_PLUS, 0, 1, 1 },
    { "display.native_lodge", S_SCREENS, 0, "New inn and camp screen", "each member's activity for the day, its cost, the teachers and their fees, and \"Until rested\".", OC_SHOW, 0, 1, 1, 0, 0, lodgeview_set },
    { "display.native_pray", S_SCREENS, 0, "New prayer screen", "the saint's statue, what the saint does, the Divine Favor to spend and the chance (by the original game's rules).", OC_SHOW, 0, 1, 1, 0, 0, prayview_set },
    { "display.native_create", S_SCREENS, 0, "New character creation", "each step on one page, with what a childhood or profession would give: the attributes afterwards (and the aging past thirty), each skill's gain and remaining room, the virtues and saints learned. The rules are unchanged.", OC_SHOW, 0, 1, 1, 0, 0, createview_set },
    { "assist.creation_guide", S_SCREENS, "display.native_create", "Step-by-step guide", "a short explanation of each step, in the empty column on the left.", OC_SHOW, 0, 1, 1 },
    { "display.native_roster", S_SCREENS, 0, "New party selection screen", "the four party slots with their sprites, the saved characters and the selected one's numbers; click to select, double-click to add or remove.", OC_SHOW, 0, 1, 1, 0, 0, rosterview_set },
    { "display.native_info", S_SCREENS, 0, "New Party Info screen", "fame and its title, the date, the place, the wealth, and the party's reputation in every town at once (the original shows one town at a time, under the pointer).", OC_SHOW, 0, 1, 1, 0, 0, infoview_set },
    { "display.native_march", S_SCREENS, 0, "New marching order screen", "a list to move members up and down, then Done, instead of clicking each member in turn.", OC_SHOW, 0, 1, 1, 0, 0, marchview_set },
    { "display.native_enemyinfo", S_SCREENS, 0, "New Enemy Info screen", "E in a battle: every kind of enemy on the field, its armour and weapons, and how well your weapons work against it.", OC_SHOW, 0, 1, 1, 0, 0, enemyview_set },
    { "display.native_mapmsg", S_SCREENS, 0, "New travel map messages", "the travel map's messages (a robber knight's land and the like) drawn in the style of the cards.", OC_SHOW, 0, 1, 1, 0, 0, mapmsg_set },
    { "display.native_saves", S_SCREENS, 0, "New Save and Load screens", "the saves listed with place, date and party: Save asks for the comment on a page that lists the saves, and Load lists all of them (the original's list holds nine).", OC_SHOW, 0, 1, 1, 0, 0, savesview_set },

    { "assist.chances", S_CARDS, 0, "Chances of success", "beside each option of a town or encounter card that has one, with the member who would try it (the original shows one at a time while Shift is held).", OC_SHOW, 0, 2, 2, "Always, as numbers|As the difficulty level allows|Off", V_CHANCES, assist_set_chances },
    { "assist.best_leader", S_CARDS, 0, "The best member speaks for the party", "when a card tests the leader, the member best at it makes the test, and the best Charisma bargains in shops; the chance shown is that member's, with their name. The leader you chose is back after the card.", OC_EASE, 0, 1, 1, 0, 0, assist_set_best_leader },
    { "assist.card_help", S_CARDS, 0, "Saints and items on cards", "which saints and items a card accepts, and who knows or carries them (as the Darklands Companion did: saints are listed even when nobody knows them yet).", OC_SHOW, 0, 1, 1, 0, 0, assist_set_card_help },
    { "assist.shop_info", S_CARDS, 0, "Tooltips", "what the item, saint or formula under the pointer is good for (in shops, the loot, character sheets, saints and formulae).", OC_SHOW, 0, 1, 1 },
    { "assist.fast_fades", S_CARDS, 0, "Cards without the fade-in", "a card shows its options at once, without the slow fade-in.", OC_SHOW, 0, 1, 1 },
    { "assist.card_keys", S_CARDS, 0, "Number keys for card options", "1 to 9 choose a card's options.", OC_SHOW, 0, 1, 1, 0, 0, assist_set_card_keys },
    { "assist.goto_work", S_CARDS, "assist.city_goto", "Who has work", "beside the Fugger, the Medici, the Hanse, the cathedral, the council, the lord and the market stalls: the chance that asking brings a task, or the date until which they have none (or that a task for them is under way, or its reward is waiting).", OC_SHOW, 0, 1, 1 },
    { "assist.teachers", S_CARDS, 0, "Teachers", "a lectern icon beside a card option that brings a teacher to the inn, with what, how well and at what fee; the teachers each town can offer, in the margin and on the map.", OC_SHOW, 0, 1, 1 },
    { "assist.city_goto", S_CARDS, 0, "Go to (G)", "G on a town's card lists its places; pick one and the party walks there through the menus.", OC_EASE, 0, 1, 1 },
    { "journal.enabled", S_CARDS, 0, "Town journal", "each town's shops, their wares, qualities and prices, noted on arrival (F12 > Journal).", OC_SHOW, 0, 1, 1 },
    { "journal.chronicle", S_CARDS, "journal.enabled", "Chronicle", "every text the cards show, word for word, with the date, the place and the choice made, so a clue read weeks ago can be found again (F12 > Journal > Chronicle).", OC_SHOW, 0, 1, 1 },

    { "assist.return_thrown", S_PARTY, 0, "Thrown weapons come back", "after a battle, javelins, throwing axes, knives and darts go back to whoever threw them.", OC_EASE, 0, 1, 1, 0, 0, return_thrown },
    { "assist.infinite_ammo", S_PARTY, 0, "Endless ammunition", "arrows, quarrels and balls never run out (one in the pack is enough to shoot); enemies still use up theirs. When off: each shot uses one, and the game leaves 7 in 10 arrows and quarrels to be found in the loot.", OC_PLUS, 0, 1, 1, 0, 0, ammo_set },
    { "assist.infinite_thrown", S_PARTY, 0, "Endless thrown weapons", "javelins, throwing axes, throwing knives and darts never run out either (one in the pack is enough to throw).", OC_PLUS, 0, 1, 1, 0, 0, thrown_set },
    { "assist.loot_all", S_PARTY, 0, "Loot and shop keys", "in the loot, A takes the whole pile and S shares it out; in a shop, A sells a whole stack and T buys ten.", OC_SHOW, 0, 1, 1 },
    { "party.new_member_looks", S_PARTY, 0, "A sprite of their own for each new member", "each member who joins gets a battle sprite and colours of their own (the original gives everyone the tall man in the same colours, hard to tell apart in a battle; the game's Figure and Colours options still change them).", OC_SHOW, 0, 1, 1 },
    { "party.keep_looks", S_PARTY, 0, "Characters keep their sprite and colours", "when they join the party again at an inn.", OC_SHOW, 0, 1, 1 },
    { "party.more_figures", S_PARTY, 0, "Enemy sprites for your own characters", "Select Character Image also offers the enemies' human sprites (sergeant, monk, knight, Templar and 14 more), with their own animations and colours. A sprite with no animation for the weapon in hand swings with the closest one it has.", OC_PLUS, 0, 1, 1, 0, 0, 0,
      "Saved games: a save with a member using one of these sprites loads only in Darklands+ (the original game on DOS stops at the missing picture)." },
    { "rules.aging", S_PARTY, 0, "Aging during play", "the years take their toll in play too, as they do during character creation: from thirty on, a little each month, more with age (by seventy, 25 Endurance, 21 Strength and Agility, 10 Perception and Charisma and 5 Intelligence in all). The original ages characters only during creation.", OC_PLUS, 0, 1, 0 },
    { "assist.loss_alerts", S_PARTY, 0, "Warn about permanent losses", "anything lost for good (an attribute's maximum, a skill, armour eaten by acid or fire, the toll of the years) pauses the game with a window saying exactly what was lost, and by whom.", OC_SHOW, 0, 1, 1 },
    { "party.figure_picker", S_PARTY, "party.more_figures", "Sprite picker", "Select Character Image shows all the sprites at once to pick from. When off: it steps to the next one, as in the original.", OC_SHOW, 0, 1, 1 },

    { "battle.pause_down", S_BATTLE, 0, "Pause when someone falls", "the battle pauses when a party member falls.", OC_EASE, 0, 1, 1, 0, 0, ap_down },
    { "battle.pause_hurt", S_BATTLE, 0, "Pause when badly hurt", "the battle pauses when a party member's Endurance drops below a quarter.", OC_EASE, 0, 0, 0, 0, 0, ap_hurt },
    { "battle.pause_foes", S_BATTLE, 0, "Pause when enemies come out", "the battle pauses when enemies waiting in a castle, dungeon or mine come out to fight.", OC_EASE, 0, 1, 1, 0, 0, autopause_set_foes },
    { "battle.click_attack", S_BATTLE, 0, "Click an enemy to attack it", "a click on an enemy sends the selected member to attack it (Std Attack), even before an attack is chosen.", OC_EASE, 0, 1, 1, 0, 0, click_attack },
    { "battle.click_best", S_BATTLE, "battle.click_attack", "Shoot first", "a member with a bow, crossbow, gun or thrown weapon shoots (Use Missile) when you click an enemy, unless the enemy is within about five paces or the line of fire is blocked: then they close in with their melee weapon. This is each member's starting choice; the bow or sword on their box switches it. When off: everyone starts with the melee weapon.", OC_EASE, 0, 1, 1 },
    { "battle.auto_vulnerable", S_BATTLE, "battle.click_attack", "Vulnerable against armour", "a click on an enemy whose body armour the member's weapon would not pierce attacks Vulnerable (aimed at the gaps: 1 to 4 more penetration, but slower and a little less likely to land) when that is enough to get through; otherwise Std Attack.", OC_EASE, 0, 1, 1 },
    { "battle.volley_toggle", S_BATTLE, "battle.click_attack", "Shoot or melee on each member's box", "in a battle, a member with a missile weapon in hand has a bow or a sword on their picture in the party panel: how a click on an enemy sends them, shooting or hand to hand. Click it to switch; each character keeps their choice.", OC_SHOW, 0, 1, 1 },
    { "battle.volley_melee", S_BATTLE, 0, "Shooters fight hand to hand once the enemies close in", "a member shooting (Use Missile) draws their melee weapon and fights when an enemy comes within reach of a blow, when nothing is left to shoot, or when the line of fire is blocked by a melee their target is already in. When off: they go on shooting, or wait for a clear line, until given another order, as in the original.", OC_EASE, 0, 1, 1 },
    { "battle.panel_select", S_BATTLE, 0, "Click a member's box to give them orders", "in a battle, a click on a member's box in the party panel selects them for orders, as 1 to 5 do (a double-click also moves the view to them); a right-click on the box opens their character sheet. When off: a click on the box opens the sheet, as in the original.", OC_EASE, 0, 1, 1 },
    { "battle.quick_orders", S_BATTLE, 0, "One click gives an order", "with a member selected, a click on an enemy or on the ground gives the order at once, and so do the orders that need no target (Halt, a door, the stairs). An attack chosen from the Orders menu or the buttons (Std Attack, Vulnerable, Berserk, Parry) for a member already fighting an enemy on screen is given against that enemy. If selecting the member paused the battle, the battle resumes with the order. When off: a click only marks the target, and the order is given by Enter, by Space or by selecting another member, as in the original.", OC_EASE, 0, 1, 1 },
    { "battle.next_member", S_BATTLE, "battle.quick_orders", "Then select the next member without orders", "while the battle is paused, giving an order selects the next member on screen who has no order, so at the start of a battle each order is a single click on an enemy. Space resumes the battle.", OC_EASE, 0, 1, 1 },
    { "battle.order_bar", S_BATTLE, 0, "Order buttons over the field", "when the window has no room for a right margin (narrower than 16:9), the order buttons appear in a strip at the bottom of the battlefield while a member is selected, so no right-click menu is needed.", OC_SHOW, 0, 1, 1 },
    { "battle.order_icons", S_BATTLE, 0, "Pictures on the order buttons", "each order button in the right margin (or over the field) shows a picture beside its name: a sword for Std Attack, a shield for Parry, footprints for Walk towards, a white flag for Surrender.", OC_SHOW, 0, 1, 1 },
    { "battle.click_hint", S_BATTLE, 0, "Say what a click will do", "with a member selected, a short label by the pointer says what a click there does (attack, shoot, walk here) or why it can't (no way there, no clear shot).", OC_SHOW, 0, 1, 1 },
    { "battle.drag_pan", S_BATTLE, 0, "Drag to move the view", "hold the right (or middle) button and move the mouse to drag the battle view; a right-click without moving still opens the menus.", OC_SHOW, 0, 1, 1 },
    { "battle.select_stays", S_BATTLE, 0, "Selecting a member doesn't move the view", "1 to 5 (or a click on a member's box) select a member without moving the view; press twice to move the view to the member and follow them. A member on another floor is still shown.", OC_SHOW, 0, 1, 1 },
    { "battle.smooth_scroll", S_BATTLE, 0, "Smooth scrolling", "each step of the battle view slides to its new place instead of jumping (a step is 32 pixels across, 16 up or down).", OC_SHOW, 0, 1, 1 },
    { "battle.scroll_paced", S_BATTLE, 0, "Steady scrolling", "the battle map scrolls at a steady pace (Shift at the edge no longer races to the end of the field).", OC_SHOW, 0, 1, 1, 0, 0, bs_paced },
    { "battle.scroll_rest", S_BATTLE, "battle.scroll_paced", "Edge scrolling without Shift", "resting the pointer on the edge of the screen scrolls the battle map; no Shift needed.", OC_SHOW, 0, 1, 1, 0, 0, bs_rest },
    { "battle.info", S_BATTLE, 0, "Info under the pointer", "pointing at someone shows who it is, how badly hurt they are, their melee rating (attack and defence hand to hand), their aim with a missile weapon, and their weapons.", OC_SHOW, 0, 1, 1 },
    { "battle.info_numbers", S_BATTLE, "battle.info", "Enemy Endurance as numbers", "the enemies' Endurance shown as numbers.", OC_EASY, 0, 0, 1 },
    { "battle.bars", S_BATTLE, 0, "Health bars over the fighters", "the Endurance left (and the Strength, once it is down by half).", OC_SHOW, 0, 1, 1 },
    { "battle.bars_hurt", S_BATTLE, "battle.bars", "Bars only over the wounded", "no bar over a fighter still unhurt (hold Alt to show them all), so the field stays clear.", OC_SHOW, 0, 1, 1 },
    { "battle.foe_arrow", S_BATTLE, 0, "Arrow to the enemies", "when no enemy of the fight is on the screen, an arrow at the edge of the field points to the nearest; a click on it shows them.", OC_EASE, 0, 1, 1 },
    { "battle.skill_gains", S_BATTLE, 0, "Show skill gains", "a skill raised in a fight rises in gold over the fighter's head (+2 Edged), and the note in the member's box flashes.", OC_SHOW, 0, 1, 1 },
    { "battle.reload_bar", S_BATTLE, 0, "Reload bar", "a thin amber line under a member reloading and aiming a bow, crossbow, gun or the next thrown weapon, filling up until the next shot.", OC_SHOW, 0, 1, 1 },
    { "battle.log", S_BATTLE, 0, "Battle log", "every blow, parry, wound and fall, in the left margin on a wide window, otherwise over the field (F12 > Journal keeps it).", OC_SHOW, 0, 1, 1 },
    { "assist.level_map", S_BATTLE, 0, "Level map (Tab)", "Tab in battles, castles, mines and dungeons: a map of the level as far as the party has seen it.", OC_SHOW, 0, 1, 1 },
    { "assist.level_map_all", S_BATTLE, "assist.level_map", "Show the whole level", "the whole level on the map, not only what the party has seen.", OC_EASY, 0, 0, 1 },
    { "assist.level_map_walk", S_BATTLE, "assist.level_map", "Click the map to walk there", "a click on the level map walks the party there.", OC_EASE, 0, 1, 1 },
    { "assist.more_jobs", S_PARTY, 0, "More day jobs", "the jobs the game keeps for marked towns, which no town ever is (mercenary, guard sergeant, bodyguard, courtier, scribe, clerk, troubadour, entertainer, preaching friar), are found where the town has what they need: soldiers where there is an arsenal or a fortress, a lord's household at a fortress, the council's clerks at a town hall, the great houses of a big city, friars by a monastery. Each member still gets the best paid job their attributes and skills fit.", OC_EASE, 0, 1, 1 },
    { "assist.stairs_together", S_BATTLE, 0, "Stairs: the party goes together", "outside a fight, when one member takes the stairs the others on that floor follow, and U with nobody on the stairs walks the party to the nearest stairs (the original moved only whoever stood on them).", OC_EASE, 0, 1, 1 },
    { "battle.foes_left", S_BATTLE, 0, "Enemies still waiting", "under the enemy list, how many enemies a castle, dungeon or mine still holds that the party has not met, floor by floor, with the master of the place marked, or the floor where he waits (the party could not know this: off except in the Easy preset).", OC_EASY, 0, 0, 1 },
    { "battle.end_words", S_BATTLE, 0, "Shorter end of battle", "when the last enemy falls, the field stands still for one second instead of three and a half before the loot, and \"Victory! The battle is won.\" (or whatever the outcome) shows for three seconds instead of eight, during which the game looked frozen.", OC_SHOW, 0, 1, 1 },
    { "battle.retarget_near", S_BATTLE, 0, "Next enemy within reach", "when a member's enemy falls, the member turns on an enemy close by, instead of walking around the whole melee to help the most wounded friend.", OC_EASE, 0, 1, 1 },
    { "battle.shoot_past_allies", S_BATTLE, 0, "Shoot past anyone in the way", "the party's archers, crossbowmen, gunners and throwers shoot past friends and enemies alike; only walls and furniture block. In Darklands a missile never hits anyone but its target, yet the game makes a shooter wait while anyone stands in the line of fire (unless the shooter's skill is 90 or more). When off: they wait for a clear line, as in the original.", OC_PLUS, 0, 1, 1 },
    { "battle.reposition_shooters", S_BATTLE, 0, "Shooters move for a clear shot", "experimental: a shooter without a clear line of fire steps aside or moves forward to find one instead of waiting, and any enemy can be targeted. A member ordered to hold stays put.", OC_EXPERIMENT, 0, 0, 0 },
    { "fix.pathfinding", S_BATTLE, 0, "Better pathfinding", "moves are planned with a full route search around walls, furniture and the other fighters, instead of the original's wall-following, which gives up on long or winding routes (members stuck in mines). For the party and the enemies alike: they find their way around corners and each other (harder battles); in a melee crowd, a fighter whose way is blocked looks for another route at once instead of standing still for a couple of seconds, and enemies at the back wait or pick another target rather than taking a long detour.", OC_FIX, 0, 1, 1, 0, 0, pf_set },
    { "battle.speed", S_BATTLE, 0, "Battle speed", "walking (in dungeons, castles and mines too), attacks and spells all run faster; pausing (Space) works as before.", OC_TASTE, 100, 100, 100, "as the original|1.5x|2x|3x", V_SPEED, battle_speed_set },
    { "battle.fast_key", S_BATTLE, 0, "Fast forward (Z)", "Z in a battle runs it six times faster; Z again goes back to the chosen speed.", OC_SHOW, 0, 1, 1, 0, 0, fast_key },

    { "travel.auto", S_TRAVEL, 0, "Auto-travel", "follows the roads to a place chosen on the world map (M: double-click, or Travel there) or with Ctrl+click on the travel map; any key or click stops it.", OC_EASE, 0, 1, 1, 0, 0, tr_auto },
    { "travel.fast", S_TRAVEL, "travel.auto", "Auto-travel speed", "", OC_TASTE, 0, 1, 1, "as walking|4x|instant", V_PACE, tr_fast },
    { "map.names", S_TRAVEL, 0, "Place names on the map", "the towns and places named on the travel map.", OC_SHOW, 0, 1, 1 },
    { "map.quest_marks", S_TRAVEL, 0, "Quest markers on the map", "each quest's picture above its place on the travel map, and a sign beside a town that bars the party or whose watch is after it.", OC_SHOW, 0, 1, 1 },
    { "map.offers_unvisited", S_TRAVEL, 0, "What unvisited places offer", "the saints a library teaches, the formulae an alchemist sells and a shop's goods, also for the towns and monasteries the party has not been to: on the world map, on the character sheet (where a saint is taught, where a formula is sold) and beside an ingredient that runs short. When off: only what the party has seen. (How good each town's smiths and other shops are is common knowledge, shown either way.)", OC_EASY, 0, 0, 1 },
    { "map.key_m", S_TRAVEL, 0, "World map (M)", "M opens a map of the whole land (the towns, the quests, where saints and formulae are found).", OC_SHOW, 0, 1, 1 },
    { "travel.encounters", S_TRAVEL, 0, "Encounters on the road", "how often travel brings an encounter.", OC_EASY, 100, 100, 50, "rare (a quarter)|fewer (half)|as the game|more (double)", V_RATE },
    { "travel.street_encounters", S_TRAVEL, 0, "Night encounters in the streets", "thieves and the watch at night in a town: the side streets bring cutthroats about 8 times in 10 for a party with little Stealth and Streetwise, and the main street the watch now and then.", OC_EASY, 100, 100, 50, "rare (a quarter)|fewer (half)|as the game|more (double)", V_RATE },

    { "assist.wheel_lists", S_CONTROL, 0, "Mouse wheel in lists", "scrolls the original game's lists (equipment, shops ...).", OC_SHOW, 0, 1, 1 },
    { "assist.menu_click", S_CONTROL, 0, "Right-click for the menus", "a right-click opens the menu bar, the next click chooses and Esc closes it (the original shows its menus only while the right button is held; that still works). F10 opens them from the keyboard.", OC_SHOW, 0, 1, 1 },
    { "input.gamepad", S_CONTROL, 0, "Game controller", "the left stick moves the pointer, A clicks, B right-clicks (F1 lists the rest).", OC_SHOW, 0, 1, 1 },

    { "editor.show", S_EDIT, 0, "Editing tools", "the Editor tab (a character's money, names, attributes, skills, saints, formulae, items, colours, shield and sprite, as the community's save editors do: for repairing characters hit by old bugs, or for cheating), and editing weapons, items and enemies in F12 > Records for a mod.", OC_TOOL, 0, 0, 0 },

    /* on other pages (Display, Sound, Saves): the presets set them */
    { "display.crisp_text", S_OTHER, 0, "Crisp text", "", OC_TASTE, 0, 2, 2, 0, 0, hdtext_set, 0, OF_ELSEWHERE },
    { "display.hd_art", S_OTHER, 0, "Pictures", "", OC_TASTE, 0, 1, 1, 0, 0, hdart_set, 0, OF_ELSEWHERE },
    { "sound.effects", S_OTHER, 0, "Sound effects", "", OC_TASTE, 0, 1, 1, 0, 0, sfx_set, 0, OF_ELSEWHERE },
    { "display.margins", S_OTHER, 0, "Margins", "", OC_SHOW, 0, 1, 1, 0, 0, 0, 0, OF_ELSEWHERE },
    { "display.full_names", S_OTHER, 0, "Names in full", "", OC_SHOW, 0, 1, 1, 0, 0, 0, 0, OF_ELSEWHERE },
    { "display.armour_worn", S_OTHER, 0, "Where armour is worn", "", OC_SHOW, 0, 1, 1, 0, 0, 0, 0, OF_ELSEWHERE },
    { "saves.quick", S_OTHER, 0, "Quick saves", "", OC_EASE, 0, 1, 1, 0, 0, saves_enable, 0, OF_ELSEWHERE },
    { "saves.autosave_arrive", S_OTHER, 0, "Autosave on arriving", "", OC_EASE, 0, 1, 1, 0, 0, auto_arrive, 0, OF_ELSEWHERE },
    { "saves.autosave_battle", S_OTHER, 0, "Autosave before a battle", "", OC_EASE, 0, 1, 1, 0, 0, auto_battle, 0, OF_ELSEWHERE },
    { "saves.restart_on_end", S_OTHER, 0, "Start again at the end", "", OC_EASE, 0, 1, 1, 0, 0, restart_end, 0, OF_ELSEWHERE },
    /* what MicroProse wrote, coded or left unfinished and never switched on, put back: experimental, off by default and in every preset */
    { "rules.fehde", S_RESTORED, 0, "\"Fehde!\" ambush", "an ambush on the road by soldiers of a private war, written for the mercenaries' ambush and never used: half the times that ambush comes up it is the Fehde instead, with its own texts and the same chances and battle (the enemies are Soldiers, led by a Knight).", OC_EXPERIMENT, 0, 0, 0 },
    { "rules.hospice", S_RESTORED, 0, "Town hospice", "a town's hospice by day, written with no code: give alms, work a day beside the nuns (Healing), have the party's wounds tended for a fee, or beg to be tended for nothing. The church district leads there, and Go to lists it.", OC_EXPERIMENT, 0, 0, 0 },
    { "rules.war", S_RESTORED, 0, "Cities at war", "coded and never switched on. Now and then a city prepares for war for a few weeks, and some then stand a siege: prices rise, the news tells of it, Soldiers' Road and its market open, the docks are guarded or barricaded, and the gate turns travellers away (talk, bluff, sneak, a farmer's hay cart, or the walls, where patrols take you for spies). A save made during a war takes it into the original game.", OC_EXPERIMENT, 0, 0, 0 },
    { "rules.politics", S_RESTORED, 0, "City politics", "the campaign written and never switched on. Now and then a city has a season of unrest: rumours in the news and the guild quarters lead to the rebels' leader or the government's man, the party takes a side, posts handbills, sways the guilds, and a war council sets the day of a march on the town hall or of the arrests. Never played by anyone in 1992: expect rough edges (docs/cut_politics.md).", OC_EXPERIMENT, 0, 0, 0 },
    { "battle.black_cloud", S_RESTORED, 0, "Black Cloud in battle", "the party can throw Black Cloud in a battle too. The manual left it out (\"the screen takes too long\" on the machines of 1992), but its smoke is in the battle's rules: shots through it hit a fifth as often, and fighting from inside it costs 15 to hit. Enemies and cards use it.", OC_EXPERIMENT, 0, 0, 0 },
    { "assist.hall_of_fame", S_RESTORED, 0, "Honor Roll of Glory (hall of fame)", "the hall of fame left unfinished in the game (its picture and its table, which nothing called): a party enters it when its journey ends, and F12 > Journal shows it.", OC_EXPERIMENT, 0, 0, 0 },
};
#define NSTATIC ((int)(sizeof OPTIONS / sizeof OPTIONS[0]))

/* the fixes and the built-in changes, appended as they are */
static Option *dyn; static int ndyn = -1;
static char (*dyn_key)[48];
static void fix_apply(int i, int v) { fix_enable(i, v); }
static void mod_apply(int i, int v) { mod_enable(i, v); }
static void build_dyn(void)
{
    if (ndyn >= 0) return;
    int n = FIX_COUNT + MOD_COUNT;
    dyn = calloc((size_t)(n ? n : 1), sizeof *dyn); dyn_key = calloc((size_t)(n ? n : 1), sizeof *dyn_key);
    ndyn = 0;
    if (!dyn || !dyn_key) return;
    for (int i = 0; i < MOD_COUNT; i++) {            /* (the Easy preset takes the ones that make the game easier, not the new-game strong party or swift travel) */
        Option *o = &dyn[ndyn]; snprintf(dyn_key[ndyn], sizeof dyn_key[0], "mod.%s", MODS[i].id);
        int easy = !strcmp(MODS[i].id, "cheap_inn") || !strcmp(MODS[i].id, "quick_learning") || !strcmp(MODS[i].id, "heavy_blows") || !strcmp(MODS[i].id, "rich_start");
        o->key = dyn_key[ndyn]; o->section = "Easier rules"; o->title = MODS[i].name; o->desc = MODS[i].desc;
        o->cls = OC_EASY; o->orig = 0; o->def = 0; o->easy = easy; o->arg = i; o->apply_i = mod_apply;
        o->note = MODS[i].needs_new_game ? "Takes effect when the next new game starts." : 0;
        ndyn++;
    }
    for (int i = 0; i < FIX_COUNT; i++) {
        Option *o = &dyn[ndyn]; snprintf(dyn_key[ndyn], sizeof dyn_key[0], "fix.%s", FIXES[i].id);
        o->key = dyn_key[ndyn]; o->section = "Fixes for bugs of the original"; o->title = FIXES[i].name; o->desc = FIXES[i].desc;
        o->cls = OC_FIX; o->orig = 0; o->def = 1; o->easy = 1; o->arg = i; o->apply_i = fix_apply;
        ndyn++;
    }
}
int options_count(void) { build_dyn(); return NSTATIC + ndyn; }
const Option *options_get(int i)
{
    build_dyn();
    if (i < 0) return 0;
    if (i < NSTATIC) return &OPTIONS[i];
    return i - NSTATIC < ndyn ? &dyn[i - NSTATIC] : 0;
}
const Option *options_find(const char *key)
{
    int n = options_count();
    for (int i = 0; i < n; i++) if (!strcmp(options_get(i)->key, key)) return options_get(i);
    return 0;
}
int option_value(const Option *o) { return config_get_int(o->key, o->def); }

static void (*listener)(const char *key, int v);
void options_set_listener(void (*fn)(const char *key, int v)) { listener = fn; }
void option_set(const Option *o, int v)
{
    config_set_int(o->key, v);
    if (o->apply) o->apply(v);
    if (o->apply_i) o->apply_i(o->arg, v);
    if (listener) listener(o->key, v);
}
int option_nchoices(const Option *o)
{
    if (!o->choices) return 0;
    int n = 1; for (const char *c = o->choices; *c; c++) n += *c == '|';
    return n;
}
const char *option_choice(const Option *o, int k, char *buf, int n)
{
    const char *c = o->choices ? o->choices : "";
    for (; k > 0 && *c; c++) if (*c == '|') k--;
    int len = 0; while (c[len] && c[len] != '|') len++;
    snprintf(buf, (size_t)n, "%.*s", len, c);
    return buf;
}

/* ---------------------------------------------------------------- presets */
const Preset PRESETS[PRESET_COUNT] = {
    { "Buggy Purist", "Darklands as it was in 1993 (version 483.07), bugs and all: everything Darklands+ adds is off, and the original game's sound setup is used.",
      "Saves load in the original game and back." },
    { "Purist", "The original game with its bugs fixed: its own screens, rules and graphics.",
      "Saves load in the original game and back." },
    { "Purist + interface", "The original rules and odds, with the new screens, crisp text, and on screen what the original keeps behind keys or in the manual (chances, tooltips, the journal, battle info). Nothing changes what happens.",
      "Saves load in the original game and back." },
    { "Semi-Purist", "Purist + interface, plus what saves clicks and waiting: auto-travel, Go to, the best member speaking for the party, thrown weapons returned after a battle, pauses in battles, autosaves. Only what you could do by hand.",
      "Saves load in the original game and back." },
    { "Darklands+", "The standard set: Semi-Purist plus one shared pack for the party, one store room shared by every inn, endless ammunition, and enemy sprites for your own characters.",
      "Saves load in the original game and back, unless a member uses an enemy sprite." },
    { "Easy", "For the story more than the struggle: fewer encounters on the road and in the streets, quick learning, half-price inns, double damage against enemies, a rich start, shooters firing past friends; and what the party could not know: the whole level on the map, the enemies a castle still holds, what unvisited places offer, the enemies' Endurance as numbers.",
      "Saves load in the original game and back, unless a member uses an enemy sprite." },
};

int preset_value(const Option *o, int p)
{
    if (o->cls == OC_TOOL) return PRESET_KEEP;
    if (o->cls == OC_TASTE) return p <= PRESET_PURIST ? o->orig : PRESET_KEEP;
    if (p == PRESET_EASY && o->cls != OC_EXPERIMENT) return o->easy;
    int on = 0;
    switch (o->cls) {
    case OC_FIX: on = p >= PRESET_PURIST; break;
    case OC_SHOW: on = p >= PRESET_PURIST_UI; break;
    case OC_EASE: on = p >= PRESET_SEMI; break;
    case OC_PLUS: on = p >= PRESET_PLUS; break;
    default: on = 0; break;
    }
    return on ? o->def : o->orig;
}
static int preset_flag_ok(int p) { return (config_get_int("original", 0) != 0) == (p == PRESET_BUGGY_PURIST); }
int preset_differences(int p)
{
    int n = options_count(), d = preset_flag_ok(p) ? 0 : 1;
    for (int i = 0; i < n; i++) {
        const Option *o = options_get(i);
        int pv = preset_value(o, p);
        if (pv == PRESET_KEEP) continue;
        if (option_value(o) != pv) d++;
    }
    return d;
}
int preset_current(void)
{
    if (config_get_int("original", 0)) return PRESET_BUGGY_PURIST;
    for (int p = PRESET_PURIST; p < PRESET_COUNT; p++) if (!preset_differences(p)) return p;
    return PRESET_CUSTOM;
}
int preset_nearest(int *diffs)
{
    int best = PRESET_PLUS, bd = 1 << 30;
    for (int p = PRESET_PURIST; p < PRESET_COUNT; p++) { int d = preset_differences(p); if (d < bd || (d == bd && p == PRESET_PLUS)) { bd = d; best = p; } }
    if (diffs) *diffs = bd;
    return best;
}

/* the player's own choices, kept before a preset replaces them: "custom.KEY" for each switch not at
   its Darklands+ value (and the sound device); "custom.saved" says there are some */
static void save_custom(void)
{
    char k[96];
    int n = options_count();
    for (int i = 0; i < n; i++) {
        const Option *o = options_get(i); int v = option_value(o);
        snprintf(k, sizeof k, "custom.%s", o->key);
        if (v != o->def) config_set_int(k, v); else config_unset(k);
    }
    config_set("custom.sound", config_get("sound.device", "auto"));
    config_set_int("custom.saved", 1);
}
int preset_custom_saved(void) { return config_get_int("custom.saved", 0) && preset_current() != PRESET_CUSTOM; }
void preset_restore_custom(void)
{
    if (!config_get_int("custom.saved", 0)) return;
    char k[96];
    config_set_int("original", 0);
    int n = options_count();
    for (int i = 0; i < n; i++) {
        const Option *o = options_get(i);
        snprintf(k, sizeof k, "custom.%s", o->key);
        int v = config_get_int(k, o->def);
        if (v != option_value(o)) option_set(o, v);
    }
    config_set("sound.device", config_get("custom.sound", "auto"));
    void datamods_scan(void), musicpack_reload(void); datamods_scan(); musicpack_reload();
    if (listener) listener("preset", PRESET_CUSTOM);
}

void preset_apply(int p)
{
    if (p < 0 || p >= PRESET_COUNT) return;
    if (preset_current() == PRESET_CUSTOM) save_custom();
    int purist = p <= PRESET_PURIST;
    int n = options_count();
    for (int i = 0; i < n; i++) {
        const Option *o = options_get(i);
        int v = option_value(o), pv = preset_value(o, p);
        char kk[96]; snprintf(kk, sizeof kk, "keep.%s", o->key);
        if (o->cls == OC_TASTE) {                   /* the player's own taste is kept for when a modern preset comes back */
            if (purist) { if (v != o->orig) config_set_int(kk, v); }
            else if (v == o->orig) pv = config_get_int(kk, o->def);
            else pv = PRESET_KEEP;
        }
        if (pv != PRESET_KEEP && pv != v) option_set(o, pv);
    }
    const char *dev = config_get("sound.device", "auto");    /* the purists hear the game's own sound setup */
    if (purist) { if (strcmp(dev, "game")) { config_set("keep.sound.device", dev); config_set("sound.device", "game"); } }
    else if (!strcmp(dev, "game")) config_set("sound.device", config_get("keep.sound.device", "auto"));
    config_set_int("original", p == PRESET_BUGGY_PURIST);
    config_set_int("preset", p);
    void datamods_scan(void), musicpack_reload(void); datamods_scan(); musicpack_reload();
    if (listener) listener("preset", p);
}
