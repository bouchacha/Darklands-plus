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
static void pf_party(int v) { pathfix_set(v, pathfix_enemies); }
static void pf_enemies(int v) { pathfix_set(pathfix_party, v); }
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
static const int V_SPEED[] = { 100, 150, 200, 300 };
static const int V_PACE[] = { 0, 1, 2 };
static const int V_RATE[] = { 25, 50, 100, 200 };

#define S_SCREENS "Screens drawn anew"
#define S_CARDS   "Cards and towns"
#define S_PARTY   "The party and its things"
#define S_BATTLE  "Battles"
#define S_TRAVEL  "Travel"
#define S_CONTROL "Controls"
#define S_EDIT    "Editing tools"
#define S_OTHER   "Elsewhere"

/* key, section, parent, title, description, class, original, Darklands+, Easy, choices, values, apply, note, flags */
static Option OPTIONS[] = {
    { "display.native_panel", S_SCREENS, 0, "Party panel", "each member's Endurance, Strength and Divine Favor as labelled bars with their numbers. Clicks on it still go to the game (a member's sheet; orders in a battle). A portrait of your own: NAME.png (or .jpg) in a folder named portraits in the Darklands+ folder, NAME being the member's name.", OC_SHOW, 0, 1, 1 },
    { "display.panel_picture", S_SCREENS, "display.native_panel", "Picture", "beside each member", OC_TASTE, 0, 0, 0, "Your portrait, else the battle figure, else the herald|The battle figure, else the herald|The herald|No picture", V_PICTURE, 0, 0, OF_COMBO },
    { "display.native_menu", S_SCREENS, 0, "Menus", "the game's menus (Game, Orders, Attack, Party ...) open with a right click or F10 and stay open until you choose or click elsewhere; the entries and what they do are the game's own. Off: the game's menu bar, open only while the right button is held.", OC_SHOW, 0, 1, 1, 0, 0, menuview_set },
    { "display.native_title", S_SCREENS, 0, "Start screen", "Continue (the newest save), Load, a new party and Quickstart, over the game's art.", OC_SHOW, 0, 1, 1, 0, 0, titleview_set },
    { "display.native_cards", S_SCREENS, 0, "Cards", "their text in the chosen typeface, the options as rows to click with the chances beside them; an option that uses an item or calls on a saint lists who can. The game still decides everything on a card. Off: the game draws its cards itself (with the crisp text over them).", OC_SHOW, 0, 1, 1, 0, 0, cardview_set },
    { "display.native_shops", S_SCREENS, 0, "Shops", "all the wares and all your things in two lists, Buy and Sell on each, what is in use marked; the bottom of the screen describes whatever the pointer is on. The stock and the prices are the game's own.", OC_SHOW, 0, 1, 1, 0, 0, barterview_set },
    { "display.native_sheet", S_SCREENS, 0, "Character sheets", "the paper doll, the things with Ready, Unequip, Drink and Drop, the skills on one page, the party's saints and formulae. Drag a thing onto a member in the party panel to give it (Shift: one of a stack). Equipping, giving and drinking are the game's own.", OC_SHOW, 0, 1, 1, 0, 0, sheetview_set },
    { "inventory.shared", S_SCREENS, "display.native_sheet", "One pack for the party", "a member keeps only what they use; Ready or Drink takes from the party's pack, and shops sell from it. Spare things weigh nothing (only what is in use counts for the load); potions stay each member's own. Where the game asks one member for a thing (lock picks at a lock, a mount, arrows before a battle) it is handed to them first; Keep marks a thing never to be sold or dropped.", OC_PLUS, 0, 1, 1 },
    { "display.native_loot", S_SCREENS, 0, "Loot", "the whole pile and each member's things in two lists, a whole stack in one click; the game's own G and P still move every thing.", OC_SHOW, 0, 1, 1, 0, 0, lootview_set },
    { "assist.loot_worth", S_SCREENS, "display.native_loot", "Worth", "under the lists, about what a shop pays for the thing under the pointer.", OC_SHOW, 0, 1, 1 },
    { "display.native_alchemy", S_SCREENS, 0, "Alchemy", "every known formula in one list, its ingredients at hand, batches with - and +, the chance of success. The chance and the mixing are the game's own.", OC_SHOW, 0, 1, 1, 0, 0, alchemyview_set },
    { "assist.where_sold", S_SCREENS, "display.native_alchemy", "Where sold", "beside an ingredient that runs short, the nearest town seen selling it.", OC_SHOW, 0, 1, 1 },
    { "display.native_cache", S_SCREENS, 0, "Store rooms", "everything stored and everything the member carries in two lists, Take and Put on each row (one or the whole stack), Put away for everything not in use. Off: the game's screen, four things at a time.", OC_SHOW, 0, 1, 1, 0, 0, cacheview_set },
    { "store.shared", S_SCREENS, "display.native_cache", "One store room for every inn", "what is left with one innkeeper is there at every other, like a chest that follows the party (100 kinds of things at most). Stores already left in several towns join it the first time a store room is opened.", OC_PLUS, 0, 1, 1 },
    { "display.native_lodge", S_SCREENS, 0, "Inns and camps", "each member's day, its cost, the teachers and their fees, and \"Until rested\".", OC_SHOW, 0, 1, 1, 0, 0, lodgeview_set },
    { "display.native_pray", S_SCREENS, 0, "Calling on a saint", "the statue, what the saint does, the favor to spend and the chance (the saint's own rules).", OC_SHOW, 0, 1, 1, 0, 0, prayview_set },
    { "display.native_create", S_SCREENS, 0, "Making a character", "each step on one page, with what a childhood or profession would bring: the attributes after it (and the aging past thirty), each skill's gain and room, the virtues and saints learned. The rules are the game's own.", OC_SHOW, 0, 1, 1, 0, 0, createview_set },
    { "assist.creation_guide", S_SCREENS, "display.native_create", "Guide", "a short guide to each step, in the empty column on the left.", OC_SHOW, 0, 1, 1 },
    { "display.native_roster", S_SCREENS, 0, "Choosing the party", "the four places with their figures, the characters kept, the chosen one's numbers; click to choose, double-click to add or drop.", OC_SHOW, 0, 1, 1, 0, 0, rosterview_set },
    { "display.native_info", S_SCREENS, 0, "Party Info", "fame and its title, the time, where, the wealth, and the reputation in every town at once (the original names one town at a time under the pointer).", OC_SHOW, 0, 1, 1, 0, 0, infoview_set },
    { "display.native_march", S_SCREENS, 0, "Marching order", "a list to move members up and down, then Done, instead of clicking each member in turn.", OC_SHOW, 0, 1, 1, 0, 0, marchview_set },
    { "display.native_enemyinfo", S_SCREENS, 0, "Enemy Info", "E in a battle: every kind of foe on the field, its armour and weapons, and what your weapons do to it.", OC_SHOW, 0, 1, 1, 0, 0, enemyview_set },
    { "display.native_mapmsg", S_SCREENS, 0, "Travel map messages", "a robber knight's land and the like, in the cards' style.", OC_SHOW, 0, 1, 1, 0, 0, mapmsg_set },
    { "display.native_saves", S_SCREENS, 0, "Save and Load", "the saves listed with where, when and who: Save asks for the comment on a page that lists them, Load lists them all (the game's own list holds nine).", OC_SHOW, 0, 1, 1, 0, 0, savesview_set },
    { "assist.hall_of_fame", S_SCREENS, 0, "Honor Roll of Glory", "the hall of fame MicroProse left unfinished in the game: a party enters it when its journey ends (and from F12 > Journal).", OC_SHOW, 0, 1, 1 },

    { "assist.chances", S_CARDS, 0, "Chances of success", "beside each option of a town or encounter card that has one, with who would try it (the original shows one at a time while Shift is held).", OC_SHOW, 0, 2, 2, "Always, as numbers|As the difficulty level allows|Off", V_CHANCES, assist_set_chances },
    { "assist.best_leader", S_CARDS, 0, "The best speaks for the party", "a card's test of the leader is made by whoever does it best, and the best Charisma bargains in shops; the chance shown is that member's, with their name. The leader you chose is back after the card.", OC_EASE, 0, 1, 1, 0, 0, assist_set_best_leader },
    { "assist.card_help", S_CARDS, 0, "Saints and items on cards", "which a card accepts, and who knows or carries them (as the Darklands Companion did: the saints are listed even when nobody knows them yet).", OC_SHOW, 0, 1, 1, 0, 0, assist_set_card_help },
    { "assist.shop_info", S_CARDS, 0, "Tooltips", "what the item, saint or formula under the pointer is good for (shops, loot, character sheet, saints, formulae).", OC_SHOW, 0, 1, 1 },
    { "assist.fast_fades", S_CARDS, 0, "Quick cards", "a card shows its options at once, without the slow fade-in.", OC_SHOW, 0, 1, 1 },
    { "assist.card_keys", S_CARDS, 0, "Number keys", "1 to 9 pick a card's options.", OC_SHOW, 0, 1, 1, 0, 0, assist_set_card_keys },
    { "assist.goto_work", S_CARDS, "assist.city_goto", "Who has work", "beside the Fugger, the Medici, the Hanse, the cathedral, the council or the lord, and the market stalls: the chance that asking brings a task, or until when they will give none (or that a task for them runs, or its reward waits).", OC_SHOW, 0, 1, 1 },
    { "assist.teachers", S_CARDS, 0, "Teachers", "a lectern beside a card's option that brings a teacher to the inn, with what, how well and for how much; the teachers each town can offer, in the margin and on the map.", OC_SHOW, 0, 1, 1 },
    { "assist.city_goto", S_CARDS, 0, "Go to", "G on a town's card lists its places; the party walks there through the menus.", OC_EASE, 0, 1, 1 },
    { "journal.enabled", S_CARDS, 0, "Town journal", "each town's shops, their wares, qualities and prices, noted on arrival (F12 > Journal).", OC_SHOW, 0, 1, 1 },
    { "journal.chronicle", S_CARDS, "journal.enabled", "Chronicle", "every text the cards show, word for word, with the date, the place and the choice made: a clue read weeks ago can be found again (F12 > Journal > Chronicle).", OC_SHOW, 0, 1, 1 },

    { "assist.return_thrown", S_PARTY, 0, "Thrown weapons come back", "after a battle, javelins, throwing axes, knives and darts go back to whoever threw them.", OC_EASE, 0, 1, 1, 0, 0, return_thrown },
    { "assist.infinite_ammo", S_PARTY, 0, "Endless ammunition", "arrows, quarrels and balls never run out (one in the pack is enough to shoot); enemies still use up theirs. Off: each shot uses one, and the game leaves 7 in 10 arrows and quarrels to be found in the loot.", OC_PLUS, 0, 1, 1, 0, 0, ammo_set },
    { "assist.infinite_thrown", S_PARTY, 0, "Endless thrown weapons", "javelins, throwing axes, throwing knives and darts never run out either (one in the pack is enough to throw).", OC_PLUS, 0, 1, 1, 0, 0, thrown_set },
    { "assist.loot_all", S_PARTY, 0, "Loot and shop keys", "take the whole pile (A) or share it out (S); in a shop, sell a whole stack (A) or buy ten (T).", OC_SHOW, 0, 1, 1 },
    { "party.new_member_looks", S_PARTY, 0, "A look for each", "each member added gets a figure and colours of their own (the game gives everyone the tall man in the same colours, hard to tell apart in a battle; Figure and Colours change them as before).", OC_SHOW, 0, 1, 1 },
    { "party.keep_looks", S_PARTY, 0, "Looks kept", "characters keep their figure and colours when they join again at an inn.", OC_SHOW, 0, 1, 1 },
    { "party.more_figures", S_PARTY, 0, "Enemies' figures", "Select Character Image also offers the enemies' human figures (sergeant, monk, knight, Templar ... 14 more), with their own animations and colours. A figure without an animation for the weapon in hand swings the nearest one it has.", OC_PLUS, 0, 1, 1, 0, 0, 0,
      "Saved games: a save with a member in one of these figures loads only in Darklands+ (the original game on DOS stops at the missing picture)." },
    { "rules.aging", S_PARTY, 0, "Aging", "the years take their toll in play too, as they do while a character is made: from thirty on, a little each month, more as the years go by (by seventy, 25 Endurance, 21 Strength and Agility, 10 Perception and Charisma and 5 Intellect in all). The original ages characters only while they are made.", OC_PLUS, 0, 1, 0 },
    { "assist.loss_alerts", S_PARTY, 0, "Losses told", "anything lost for good (an attribute's maximum, a skill, armour eaten by acid or fire, the years' toll) stops the game in a window saying exactly what, and whose.", OC_SHOW, 0, 1, 1 },
    { "party.figure_picker", S_PARTY, "party.more_figures", "Figure picker", "Select Character Image shows all the figures at once to pick from (else it steps to the next, as in the original).", OC_SHOW, 0, 1, 1 },

    { "battle.pause_down", S_BATTLE, 0, "Pause when someone falls", "the battle stops when someone in the party falls.", OC_EASE, 0, 1, 1, 0, 0, ap_down },
    { "battle.pause_hurt", S_BATTLE, 0, "Pause when badly hurt", "the battle stops when someone's endurance drops below a quarter.", OC_EASE, 0, 0, 0, 0, 0, ap_hurt },
    { "battle.pause_foes", S_BATTLE, 0, "Pause when foes come out", "the battle stops when foes waiting in a castle, dungeon or mine come out to fight.", OC_EASE, 0, 1, 1, 0, 0, autopause_set_foes },
    { "battle.click_attack", S_BATTLE, 0, "Click to attack", "a click on an enemy sends the selected member at it (Std Attack), even before an attack is chosen.", OC_EASE, 0, 1, 1, 0, 0, click_attack },
    { "battle.click_best", S_BATTLE, "battle.click_attack", "With the better weapon", "a member better with a bow, crossbow, gun or thrown weapon than with the weapon for blows shoots from where they stand (Use Missile), unless the foe is already within reach of a blow or the line to it is blocked.", OC_EASE, 0, 1, 1 },
    { "battle.drag_pan", S_BATTLE, 0, "Drag the field", "the right (or middle) button held and moved pulls the battle's view along; a right click without moving still opens the menus.", OC_SHOW, 0, 1, 1 },
    { "battle.select_stays", S_BATTLE, 0, "Choosing doesn't move the view", "1 to 5 choose a member without moving the view while the member is in sight; pressed twice, the view goes to the member and follows.", OC_SHOW, 0, 1, 1 },
    { "battle.smooth_scroll", S_BATTLE, 0, "Smooth scrolling", "a step of the battle's view slides the field to its new place instead of jumping (a whole step at once: 32 pixels across, 16 up or down).", OC_SHOW, 0, 1, 1 },
    { "battle.scroll_paced", S_BATTLE, 0, "Steady scrolling", "the battle map scrolls at a steady pace (Shift at the edge no longer flies to the end of the field).", OC_SHOW, 0, 1, 1, 0, 0, bs_paced },
    { "battle.scroll_rest", S_BATTLE, "battle.scroll_paced", "Edge scrolling", "the pointer resting on the edge scrolls, no Shift needed.", OC_SHOW, 0, 1, 1, 0, 0, bs_rest },
    { "battle.info", S_BATTLE, 0, "Battle info", "pointing at someone shows who it is, how hurt, attack, defence and weapon.", OC_SHOW, 0, 1, 1 },
    { "battle.info_numbers", S_BATTLE, "battle.info", "Numbers", "the enemies' endurance as numbers.", OC_EASY, 0, 0, 1 },
    { "battle.bars", S_BATTLE, 0, "Bars over the fighters", "the endurance left (and the strength once it is down by half).", OC_SHOW, 0, 1, 1 },
    { "battle.bars_hurt", S_BATTLE, "battle.bars", "Only the hurt", "no bar over a fighter still unhurt (all of them while Alt is held): the field stays clear.", OC_SHOW, 0, 1, 1 },
    { "battle.skill_gains", S_BATTLE, 0, "Skill gains", "a skill raised in a fight rises in gold over the fighter's head (+2 Edged), and the note in the member's box flashes.", OC_SHOW, 0, 1, 1 },
    { "battle.reload_bar", S_BATTLE, 0, "Reloading", "a thin amber line under a member who reloads and aims a bow, crossbow, gun or the next thrown weapon, filling up until the next shot.", OC_SHOW, 0, 1, 1 },
    { "battle.log", S_BATTLE, 0, "Battle log", "every blow, parry, wound and fall, in the left margin on a wide window, else over the field (F12 > Journal keeps it).", OC_SHOW, 0, 1, 1 },
    { "assist.level_map", S_BATTLE, 0, "Level map", "Tab in battles, castles, mines and dungeons: a map of the level as far as the party has seen it.", OC_SHOW, 0, 1, 1 },
    { "assist.level_map_all", S_BATTLE, "assist.level_map", "Whole level", "not only what the party has seen.", OC_EASY, 0, 0, 1 },
    { "assist.level_map_walk", S_BATTLE, "assist.level_map", "Walk there", "a click on the map walks the party there.", OC_EASE, 0, 1, 1 },
    { "battle.foes_left", S_BATTLE, 0, "Foes left", "under the enemies, the foes a castle, dungeon or mine still holds, not yet met, floor by floor; the master of the place marked, or the floor where he waits (what the party cannot know: off but in the Easy preset).", OC_EASY, 0, 0, 1 },
    { "battle.end_words", S_BATTLE, 0, "A quick end", "when the last foe falls the field stands still a second, not three and a half, before the loot; then \"Victory! The battle is won.\" (or how else it ended) for three seconds, not eight, as if the game had hung.", OC_SHOW, 0, 1, 1 },
    { "battle.black_cloud", S_BATTLE, 0, "Black Cloud in battle", "the party can throw Black Cloud in a battle too. The manual left it out (\"the screen takes too long\" on the machines of 1992), but its smoke is in the battle's rules: shots through it hit a fifth as often, and fighting from inside it costs 15 to hit. Enemies and cards use it.", OC_PLUS, 0, 1, 1 },
    { "battle.retarget_near", S_BATTLE, 0, "Next foe within reach", "when a member's foe falls, the member turns on a foe close by, rather than walking around the whole melee to help the most wounded friend.", OC_EASE, 0, 1, 1 },
    { "battle.shoot_past_allies", S_BATTLE, 0, "Shoot past friends", "the party's archers, crossbowmen and gunners shoot past their own fighters. A missile never strikes anyone but its target in Darklands, yet the game makes a shooter wait while a friend stands in the line (unless the shooter's skill is 90 or more). Enemies in the line still block.", OC_EASY, 0, 0, 1 },
    { "battle.reposition_shooters", S_BATTLE, 0, "Shooters look for a line", "experimental: a shooter without a clear line steps aside or walks on to find one instead of waiting, and any foe can be chosen. A member told to hold stays put.", OC_EXPERIMENT, 0, 0, 0 },
    { "fix.pathfinding", S_BATTLE, 0, "Better pathfinding", "moves are planned around walls, furniture and the other fighters with a full route search, instead of the original's wall-following, which gives up on long or winding routes (members stuck in mines).", OC_FIX, 0, 1, 1, 0, 0, pf_party },
    { "fix.pathfinding_enemies", S_BATTLE, 0, "Better pathfinding for enemies", "the same for the foes: they find their way to the party around corners and each other (harder battles).", OC_FIX, 0, 1, 1, 0, 0, pf_enemies },
    { "battle.speed", S_BATTLE, 0, "Speed of battles and dungeons", "walking (also through dungeons, castles and mines), attacks and spells all go faster; pausing (Space) works as before.", OC_TASTE, 100, 100, 100, "as the original|1.5x|2x|3x", V_SPEED, battle_speed_set },
    { "battle.fast_key", S_BATTLE, 0, "Fast forward", "Z in a battle: six times the pace; Z again back to the chosen speed.", OC_SHOW, 0, 1, 1, 0, 0, fast_key },

    { "travel.auto", S_TRAVEL, 0, "Auto-travel", "along the roads, to a place chosen on the world map (M: double-click, or Travel there) or with Ctrl+click on the map; any key or click stops.", OC_EASE, 0, 1, 1, 0, 0, tr_auto },
    { "travel.fast", S_TRAVEL, "travel.auto", "Pace", "of auto-travel.", OC_TASTE, 0, 1, 1, "as walking|4x|at once", V_PACE, tr_fast },
    { "map.names", S_TRAVEL, 0, "Place names", "the towns and places named on the travel map.", OC_SHOW, 0, 1, 1 },
    { "map.quest_marks", S_TRAVEL, 0, "Quest marks", "the quests' pictures above their places on the travel map, and a sign beside a town that bars the party or whose watch wants it.", OC_SHOW, 0, 1, 1 },
    { "map.offers_unvisited", S_TRAVEL, 0, "What unvisited places offer", "the saints a library teaches, the formulae an alchemist sells and the goods a shop will have, for the towns and monasteries the party has not been to as well: on the world map, on the character sheet (where a saint is taught, where a formula is sold) and beside an ingredient that runs short. Off: only what the party has seen.", OC_EASY, 0, 0, 1 },
    { "map.key_m", S_TRAVEL, 0, "World map", "M opens the map of the whole land (the towns, the quests, where saints and formulae are found).", OC_SHOW, 0, 1, 1 },
    { "travel.encounters", S_TRAVEL, 0, "Encounters on the road", "how often the road brings an encounter.", OC_EASY, 100, 100, 50, "rare (a quarter)|fewer (half)|as the game|more (double)", V_RATE },
    { "travel.street_encounters", S_TRAVEL, 0, "Thieves and the watch at night", "in a town's streets: the side streets bring cutthroats about 8 times in 10 for a party with little Stealth and Streetwise, and the main street the watch now and then.", OC_EASY, 100, 100, 50, "rare (a quarter)|fewer (half)|as the game|more (double)", V_RATE },

    { "assist.wheel_lists", S_CONTROL, 0, "Mouse wheel", "moves in the game's lists (equipment, shops ...).", OC_SHOW, 0, 1, 1 },
    { "assist.menu_click", S_CONTROL, 0, "Right click for the menus", "opens the menu bar, the next click chooses, Esc closes it (the game shows its menus only while the right button is held; that still works). F10 opens them from the keyboard.", OC_SHOW, 0, 1, 1 },
    { "input.gamepad", S_CONTROL, 0, "Game controller", "the left stick moves the pointer, A clicks, B right-clicks (F1 lists the rest).", OC_SHOW, 0, 1, 1 },

    { "editor.show", S_EDIT, 0, "Editing tools", "the Editor tab (a character's money, names, attributes, skills, saints, formulae, things, colours, shield and figure, as the community's save editors do: for mending characters hit by old bugs, or for cheating), and changing weapons, items and foes in F12 > Records for a mod.", OC_TOOL, 0, 0, 0 },

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
    { "Buggy Purist", "Darklands as it was in 1993 (version 483.07), bugs and all: everything Darklands+ adds is off, and the game's own sound setup is used.",
      "Saves load in the original game and back." },
    { "Purist", "The original game with its bugs fixed: its own screens, rules and looks.",
      "Saves load in the original game and back." },
    { "Purist + interface", "The game's own rules and odds, with the screens drawn anew, crisp text, and what the original hides behind keys and the manual shown (chances, tooltips, the journal, battle info). Nothing changes what happens.",
      "Saves load in the original game and back." },
    { "Semi-Purist", "As Purist + interface, with what saves clicks and waiting: auto-travel, Go to, the best member speaking for the party, thrown weapons picked up, pauses in battles, autosaves. Only what you could do by hand.",
      "Saves load in the original game and back." },
    { "Darklands+", "The standard set: also one pack for the party, one store room for every inn, endless ammunition, the foes waiting in a castle shown, and the enemies' figures for your characters.",
      "Saves load in the original game and back, unless a member has one of the enemies' figures." },
    { "Easy", "For the story more than the struggle: fewer encounters on the road and in the streets, quick learning, half-price inns, heavy blows, a rich start, shooters firing past friends, the whole level on the map.",
      "Saves load in the original game and back, unless a member has one of the enemies' figures." },
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
