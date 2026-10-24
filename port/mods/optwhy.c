/* optwhy.c - why each switch exists, one line each: beside what it does (the table's description,
 * mods/options.c), the reason for it, for whoever carries a feature into another engine (the list
 * docs/SWITCHES.md is made from both: DL_DUMP_SWITCHES, runtime/main.c). The fixes need none: each
 * puts right a bug of the original (their descriptions say which). A switch's key is its name for
 * good: saved settings, presets and other engines know it by that; a switch retired is listed in
 * RETIRED, never reused. */
#include <string.h>
#include "options.h"

static const struct { const char *key, *why; } WHY[] = {
    /* groups */
    { "display.native_screens", "The original screens are hard to read when scaled up and hide much behind keys; one switch, since nobody wants half the screens new and half old." },
    { "battle.controls", "The original needs several clicks and menus per order; one click per order makes real-time battles playable. One switch: the parts depend on one another." },
    { "battle.helpers", "The original shows almost nothing of a battle's state on the field; these show what the game already knows, nothing more." },
    { "battle.pauses", "Things that need the player's attention happen while the battle runs on." },
    { "battle.scrolling", "The original's view jumps and races, which loses the player's bearings." },
    { "assist.card_helpers", "The original hides the odds and what a card accepts behind Shift and the manual; the game's own numbers, shown." },
    /* screens */
    { "display.shop_compare", "Comparing a ware with what is worn meant opening the sheet." },
    { "display.native_panel", "The original panel shows bars without numbers." },
    { "display.panel_picture", "A matter of taste: portrait, sprite or crest." },
    { "display.tooltip_delay", "A matter of taste." },
    { "display.native_menu", "The original menu bar is open only while the right button is held." },
    { "display.native_title", "Loading the newest save was several screens away." },
    { "display.native_cards", "The cards' text is the heart of the game and was the hardest to read." },
    { "display.native_shops", "The original shows a few items at a time and no comparison." },
    { "display.native_sheet", "The original splits the sheet over several screens of small text." },
    { "display.native_loot", "The original moves one item per click." },
    { "assist.loot_worth", "Deciding what to carry needs what it is worth." },
    { "display.native_alchemy", "The original brews one formula at a time and hides the chance." },
    { "assist.where_sold", "Missing ingredients meant searching the towns by memory." },
    { "display.native_cache", "The original shows four stored items at a time." },
    { "display.native_lodge", "The original hides the cost and the teachers' terms." },
    { "display.native_pray", "The original hides what a saint does and the chance." },
    { "display.native_create", "The original hides what each choice will give." },
    { "assist.creation_guide", "Character creation is the first wall for new players." },
    { "display.native_roster", "The original picks party members one click at a time without details." },
    { "display.native_info", "The original shows one town's reputation at a time." },
    { "display.native_march", "The original sets the order by clicking each member in turn." },
    { "display.native_enemyinfo", "The original Enemy Info is one enemy at a time, without armor or weapons." },
    { "display.native_mapmsg", "The travel map's messages were in the old style among the new screens." },
    { "display.native_saves", "The original lists only nine saves, without their place or party." },
    /* the party and its items */
    { "inventory.shared", "Juggling items between members is busywork the original enforces only through weight; the pack keeps what is worn counting." },
    { "store.shared", "A store room per town made stored items as good as lost." },
    { "assist.inn_earn", "A rested member relaxing gains nothing; earning was the better choice the player always had to make by hand." },
    { "assist.best_leader", "The original tests only the leader, so the player swaps leaders before each card by hand." },
    { "assist.chances", "The original shows a chance only while Shift is held, one option at a time." },
    { "assist.card_help", "Which saints and items a card accepts was known only from the Darklands Companion." },
    { "assist.shop_info", "What an item, saint or formula is good for was in the manual only." },
    { "assist.fast_fades", "The fade-in is a wait each card." },
    { "assist.card_keys", "Choosing with the keyboard is quicker than clicking." },
    { "assist.goto_work", "Finding which office has work meant visiting each." },
    { "assist.teachers", "Teachers are easy to miss: nothing in the original says an option brings one." },
    { "assist.city_goto", "Walking through a town's menus to one place is many clicks the player could do by hand." },
    { "assist.docks_map", "Boat destinations are names without a map." },
    { "journal.enabled", "The player otherwise keeps notes on paper." },
    { "journal.chronicle", "A record of the journey the original does not keep." },
    { "assist.return_thrown", "Gathering thrown weapons after each battle is busywork." },
    { "assist.infinite_ammo", "Counting arrows is bookkeeping, not play; enemies still run out." },
    { "assist.infinite_thrown", "The same for thrown weapons." },
    { "assist.loot_all", "Taking or sharing the whole loot was one click per item." },
    { "party.new_member_looks", "In the original every member looks the same and cannot be told apart in battle." },
    { "party.keep_looks", "A member rejoining lost their looks." },
    { "party.more_figures", "Four party sprites are few; the enemies' human sprites fit." },
    { "party.figure_picker", "Choosing among many sprites needs to see them." },
    { "rules.aging", "Creation ages characters but play does not, so old characters were free; this keeps age a cost." },
    { "assist.loss_alerts", "Permanent losses happen silently in the original." },
    { "assist.more_jobs", "Jobs written into the game's tables and never offered." },
    { "assist.stairs_together", "The original moves only whoever stands on the stairs." },
    /* battles */
    { "battle.pause_down", "(part of Pauses in battle)" },
    { "battle.pause_hurt", "(part of Pauses in battle)" },
    { "battle.pause_foes", "(part of Pauses in battle)" },
    { "battle.improved", "Between orders the original's members stand idle or fight poorly (shooters walk into melee, freeze without a line of fire); this is what a player would order each time." },
    { "battle.click_attack", "(part of Easier battle controls)" },
    { "battle.volley_toggle", "(part of Easier battle controls)" },
    { "battle.panel_select", "(part of Easier battle controls)" },
    { "battle.quick_orders", "(part of Easier battle controls)" },
    { "battle.next_member", "(part of Easier battle controls)" },
    { "battle.order_bar", "(part of Easier battle controls)" },
    { "battle.order_icons", "(part of Easier battle controls)" },
    { "battle.drag_pan", "(part of Easier battle controls)" },
    { "battle.select_stays", "(part of Easier battle controls)" },
    { "battle.end_words", "(part of Easier battle controls) The original's long wait at the end looked like a freeze." },
    { "battle.smooth_scroll", "(part of Smooth, steady scrolling)" },
    { "battle.scroll_paced", "(part of Smooth, steady scrolling)" },
    { "battle.scroll_rest", "(part of Smooth, steady scrolling)" },
    { "battle.info", "(part of Battle info on screen)" },
    { "battle.los_line", "(part of Battle info on screen)" },
    { "battle.click_hint", "(part of Battle info on screen)" },
    { "battle.foe_arrow", "(part of Battle info on screen)" },
    { "battle.skill_gains", "(part of Battle info on screen)" },
    { "battle.reload_bar", "(part of Battle info on screen)" },
    { "battle.log", "(part of Battle info on screen)" },
    { "battle.info_numbers", "Easier: exact numbers the party could not know." },
    { "battle.foes_left", "Easier: knowledge the party could not have." },
    { "assist.level_map", "Dungeons and castles are hard to find one's way in without a map." },
    { "assist.level_map_all", "Easier: the parts not yet seen." },
    { "assist.level_map_walk", "Walking across a level is many clicks." },
    { "fix.pathfinding", "The original's wall-following gets members stuck in mines and long rooms." },
    { "battle.speed", "A matter of taste: the original's battles are slow on today's machines' timing." },
    { "battle.fast_key", "Battles already won take long to finish." },
    /* travel */
    { "travel.auto", "Steering across the map tile by tile is many key presses the player could do by hand." },
    { "travel.fast", "A matter of taste." },
    { "map.names", "The original map names nothing." },
    { "map.quest_marks", "Where a quest leads was in the player's notes only." },
    { "map.offers_unvisited", "Easier: knowledge the party could not have." },
    { "map.key_m", "The original has no map of the whole land." },
    { "travel.encounters", "Easier: fewer interruptions." },
    { "travel.street_encounters", "Easier: the original's night streets are near-certain ambushes." },
    /* controls, tools, elsewhere */
    { "assist.wheel_lists", "The mouse wheel did nothing in the original." },
    { "assist.menu_click", "The original menu bar needs the right button held." },
    { "input.gamepad", "To play from a couch or a handheld PC." },
    { "editor.show", "To repair characters hit by old bugs, to cheat, and to edit data for mods." },
    { "display.crisp_text", "The original 7-pixel lettering is hard to read scaled up." },
    { "display.hd_art", "Replacement pictures, for those who make them." },
    { "sound.effects", "A matter of taste." },
    { "display.margins", "Wide screens have room the original left black." },
    { "display.full_names", "The original cut names short to fit its screens." },
    { "display.armour_worn", "V:/L: prefixes are cryptic." },
    { "saves.quick", "Saving took several menus." },
    { "saves.autosave_arrive", "A crash or a bad fight cost hours." },
    { "saves.autosave_battle", "A lost battle cost what came before it." },
    { "saves.restart_on_end", "The original quits to DOS at the end." },
    /* restored cut content */
    { "rules.fehde", "Written by MicroProse and never used." },
    { "rules.hospice", "Written by MicroProse and never made playable." },
    { "rules.war", "Coded by MicroProse and never switched on." },
    { "rules.politics", "Written by MicroProse and never switched on." },
    { "battle.black_cloud", "Left out in 1992 for the machines' speed; the rules handle it." },
    { "assist.hall_of_fame", "Left unfinished in the game's files." },
};

/* switches no longer offered: their keys are not to be used for anything else (settings may still
   hold them; nothing reads them) */
const char *const OPTIONS_RETIRED[] = {
    "display.panel_losses",          /* the party panel's pale losses: always shown with the new panel */
    "battle.bars", "battle.bars_hurt",   /* bars over every fighter: now only over wounded enemies (battle.info) */
    "map.icon_set",                  /* the world map's place pictures: the seals only */
    "map.boat_style",                /* boat routes: red ink with ships only */
    "display.card_numbers",          /* the card options' key marks: red numerals only */
    0 };

const char *option_why(const Option *o)
{
    if (o->cls == OC_FIX) return "Puts right a bug of the original (the description says which).";
    if (o->cls == OC_LOOK) return "A matter of taste, or of the computer it runs on; no preset touches it.";
    if (!strncmp(o->key, "mod.", 4)) return "Easier: for the story more than the struggle (a built-in mod).";
    for (size_t i = 0; i < sizeof WHY / sizeof WHY[0]; i++) if (!strcmp(WHY[i].key, o->key)) return WHY[i].why;
    return "";
}
