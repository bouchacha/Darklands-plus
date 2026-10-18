// settings.cpp - the Darklands+ window's settings pages (F12): Mods, Helpers and Display. Each
// switch is a config key (runtime/config.h); a change marks the settings to be written when the
// window closes (ui_dirty_config). Split from ui.cpp, which holds the window and its other pages.
#include <SDL.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "imgui.h"
#include "tablesort.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_sdlrenderer2.h"
#include "gamedata.h"
#include "quests.h"

extern "C" {
#include "../runtime/cpu.h"
#include "../runtime/game.h"
#include "../runtime/config.h"
#include "../mods/mods.h"
#include "../mods/assist.h"
#include "../mods/cardview.h"
#include "../mods/travel.h"
#include "../mods/fixes.h"
#include "../mods/saves.h"
#include "../mods/hdtext.h"
#include "../mods/musicpack.h"
#include "../mods/hdart.h"
#include "../mods/sfx.h"
#include "../mods/datamods.h"
#include "../mods/lootview.h"
#include "../mods/sheetview.h"
#include "ui.h"
#include "money.h"
extern char game_dir[1024];
extern const unsigned char FONT_MEDIUM[], FONT_SEMIBOLD[], FONT_ALEGREYA[], FONT_FELL[], FONT_MEDIEVAL[], FONT_ATKINSON[], FONT_FRAKTUR[], FONT_GRENZE[];
extern const unsigned int FONT_MEDIUM_SIZE, FONT_SEMIBOLD_SIZE, FONT_ALEGREYA_SIZE, FONT_FELL_SIZE, FONT_MEDIEVAL_SIZE, FONT_ATKINSON_SIZE, FONT_FRAKTUR_SIZE, FONT_GRENZE_SIZE;
}

#include "uiinternal.h"

void ui_hint(const char *fmt, ...)
{
    va_list ap; va_start(ap, fmt);
    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
    ImGui::TextWrappedV(fmt, ap);
    ImGui::PopStyleColor();
    va_end(ap);
}

/* ------------------------------------------------------------------ tabs */
extern "C" int encounters_count(void);
extern "C" int opening_mode(void);                 /* mods/opening.c */
extern "C" void opening_set_mode(int);
extern "C" int encounters_errors(void);
void tab_mods(void)
{
    ImGui::PushFont(ui_font_head); ImGui::SeparatorText("Mod folders"); ImGui::PopFont();
    {
        static char msg[1400];
        char md[1100]; datamods_folder(md, sizeof md);
        extern char dlp_user_dir[];
        ImGui::TextWrapped("A mod is a folder in %s" PATH_SEP "mods with files laid out like the game folder: "
                           "while it is ticked the game reads them instead of its own (the game's files are never changed).", dlp_user_dir);
        if (!datamods_n) ui_hint("No mods found yet.");
        for (int i = 0; i < datamods_n; i++) {
            ImGui::PushID(1000 + i);
            bool on = datamods[i].on != 0;
            if (ImGui::Checkbox(datamods[i].title, &on)) { datamods_set(i, on); ui_dirty_config = 1; }
            ImGui::Indent(UIS(34));
            if (datamods[i].desc[0]) ui_hint("%s", datamods[i].desc);
            { std::string fp = datamods[i].path; for (char &c : fp) if (c == '/' || c == '\\') c = PATH_SEP[0]; ui_hint("folder: %s", fp.c_str()); }
            ImGui::Unindent(UIS(34));
            ImGui::PopID();
        }
        {   /* mods with ENCOUNTERS folders (mods/encounters.c) */
            int ne = encounters_count(), nerr = encounters_errors();
            if (ne || nerr) {
                ImGui::Spacing();
                ImGui::TextWrapped("New encounters in use: %d%s", ne, nerr ? "" : ".");
                if (nerr) { ImGui::SameLine(); ImGui::TextColored(ImVec4(1, 0.6f, 0.4f, 1), "(%d file%s with errors left out: see darklands_port.log)", nerr, nerr == 1 ? "" : "s"); }
                ImGui::Indent(UIS(34));
                int share = config_get_int("encounters.share", 100);
                ImGui::SetNextItemWidth(UIS(220));
                if (ImGui::SliderInt("How often they come up##encshare", &share, 0, 300, "%d %%")) { config_set_int("encounters.share", share); ui_dirty_config = 1; }
                ui_hint("100 %%: each counts as one more entry (or as many as its weight) of the game's own table of encounters for the terrain.");
                bool seal = config_get_int("encounters.seal", 1) != 0;
                if (ImGui::Checkbox("Mark them with a seal in the corner of the card", &seal)) { config_set_int("encounters.seal", seal); ui_dirty_config = 1; }
                ImGui::Unindent(UIS(34));
            }
        }
        if (ImGui::Button("Look for mods again")) { datamods_scan(); hdart_rescan(); msg[0] = 0; }
        ImGui::SameLine();
        static char newname[64] = "My mod";
        ImGui::SetNextItemWidth(UIS(200)); ImGui::InputText("##newmod", newname, sizeof newname);
        ImGui::SameLine();
        if (ImGui::Button("Make a new mod folder") && newname[0]) {
            char w[1300]; int r = datamods_new(newname, w, sizeof w);
            snprintf(msg, sizeof msg, r > 0 ? "Made %s (with a mod.txt that explains what goes where)." : r == 0 ? "%s is there already." : "Could not make %s.", w);
        }
        if (ImGui::Button("Export the card texts")) {
            char w[1300]; int r = datamods_export_cards(w, sizeof w);
            if (r >= 0) snprintf(msg, sizeof msg, "%d card files written to %s as text. Copy the ones you change into your mod's MSGFILES folder.", r, w);
            else snprintf(msg, sizeof msg, "Could not read MSGFILES.");
        }
        ImGui::SameLine(); ui_hint("every town and encounter text, editable, with its options");
        { extern int ui_collect_texts; bool ct = ui_collect_texts != 0;
          if (ImGui::Checkbox("Collect the texts shown (for a translation: mods" PATH_SEP "_export" PATH_SEP "texts_seen.txt)", &ct)) ui_collect_texts = ct; }
        ui_hint("A mod's text.txt says lines differently, one per line: Purchase an item => Kaufen (menus the game writes from its program; needs crisp text).");
        if (msg[0]) ImGui::TextWrapped("%s", msg);
        ui_hint("Mods are applied in alphabetical order of their folders: where two change the same file, the later one wins. "
             "Single files inside MSGFILES and the .CAT catalogues can be replaced by a folder of that name holding just those files. "
             "Card texts and pictures change at once; the item list, saints, cities and the like when the game starts.");
    }
    ImGui::PushFont(ui_font_head); ImGui::SeparatorText("Built-in changes"); ImGui::PopFont();
    ImGui::TextWrapped("Changes to the game, switched on and off here. They take effect at once "
                       "(\"new game\" mods when the next game starts).");
    ImGui::Spacing();
    const char *cat = "";
    for (int i = 0; i < MOD_COUNT; i++) {
        Mod &m = MODS[i];
        if (strcmp(cat, m.category)) {
            cat = m.category;
            ImGui::PushFont(ui_font_head); ImGui::SeparatorText(cat); ImGui::PopFont();
        }
        bool on = m.enabled;
        ImGui::PushID(i);
        if (ImGui::Checkbox(m.name, &on)) { mod_enable(i, on); ui_dirty_config = 1; }
        ImGui::Indent(UIS(34));
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        ImGui::TextWrapped("%s%s", m.desc, m.needs_new_game ? "  (New game.)" : "");
        ImGui::PopStyleColor();
        ImGui::Unindent(UIS(34));
        ImGui::PopID();
    }
}

/* "play the original game": every change off, the player's choices kept to come back to */
int ui_original_mode(void) { return config_get_int("original", 0); }
extern "C" void ui_set_original_c(int on);
void ui_set_original(bool on)
{
    if (on) {
        char b[512]; int n = 0;
        n += snprintf(b + n, sizeof b - n, "v2 %d %d %d %d %d %d", assist_chances, journal_enabled(), pathfix_party, pathfix_enemies, travel_enabled, travel_fast);
        config_set_int("original.battle_speed", battle_speed);
        battle_speed_set(100);
        config_set_int("original.card_help", assist_card_help);
        assist_set_card_help(0);
        config_set_int("original.card_keys", assist_card_keys);
        assist_set_card_keys(0);
        config_set_int("original.pause_down", autopause_down); config_set_int("original.pause_hurt", autopause_hurt);
        autopause_set(0, 0);
        { extern int ui_margins_on; config_set_int("original.margins", ui_margins_on); ui_margins_on = 0; config_set_int("display.margins", 0); }
        n += snprintf(b + n, sizeof b - n, " %d", FIX_COUNT);            /* (v2: the counts, so that a later version with more fixes or mods reads it right) */
        for (int i = 0; i < FIX_COUNT; i++) n += snprintf(b + n, sizeof b - n, " %d", FIXES[i].on);
        n += snprintf(b + n, sizeof b - n, " %d", MOD_COUNT);
        for (int i = 0; i < MOD_COUNT; i++) n += snprintf(b + n, sizeof b - n, " %d", MODS[i].enabled);
        config_set("original.saved", b);
        config_set("original.sound", config_get("sound.device", "auto"));
        config_set_int("original.text", hdtext_mode);
        config_set_int("original.art", hdart_on);
        config_set_int("original.sfx", sfx_mode);
        config_set_int("original.saves", saves_on);
        saves_enable(0);
        assist_set_chances(0); journal_set(0); pathfix_set(0, 0); travel_set(0, travel_fast); hdtext_set(0); hdart_set(0); sfx_set(0);
        for (int i = 0; i < FIX_COUNT; i++) fix_enable(i, 0);
        for (int i = 0; i < MOD_COUNT; i++) if (MODS[i].enabled) mod_enable(i, 0);
        config_set("sound.device", "game");
        config_set_int("original", 1);
        datamods_scan(); hdart_rescan(); rules_apply(); sfx_reload(); musicpack_reload();
    } else {
        const char *p = config_get("original.saved", "");
        char *e;
        auto next = [&](int def) { long v = strtol(p, &e, 10); if (e == p) return def; p = e; return (int)v; };
        bool v2 = !strncmp(p, "v2", 2);
        if (v2) p += 2;
        int ch = next(2), jo = next(1), pp = next(1), pe = next(0), ta = next(1), tf = next(1);
        assist_set_chances(ch); journal_set(jo); pathfix_set(pp, pe); travel_set(ta, tf);
        int nf = v2 ? next(0) : FIX_COUNT;                  /* (fixes and mods added since are left as they start: fixes on, mods off) */
        for (int i = 0; i < nf; i++) { int on = next(1); if (i < FIX_COUNT) fix_enable(i, on); }
        for (int i = nf; i < FIX_COUNT; i++) fix_enable(i, 1);
        int nm = v2 ? next(0) : MOD_COUNT;
        for (int i = 0; i < nm; i++) { int on = next(0); if (i < MOD_COUNT && on) mod_enable(i, 1); }
        config_set("sound.device", config_get("original.sound", "auto"));
        hdtext_set(config_get_int("original.text", 2));
        hdart_set(config_get_int("original.art", 1));
        sfx_set(config_get_int("original.sfx", 1));
        saves_enable(config_get_int("original.saves", 1));
        battle_speed_set(config_get_int("original.battle_speed", 100));
        assist_set_card_help(config_get_int("original.card_help", 1));
        assist_set_card_keys(config_get_int("original.card_keys", 1));
        autopause_set(config_get_int("original.pause_down", 1), config_get_int("original.pause_hurt", 0));
        { extern int ui_margins_on; ui_margins_on = config_get_int("original.margins", 1); config_set_int("display.margins", ui_margins_on); }
        config_set_int("original", 0);
        datamods_scan(); hdart_rescan(); rules_apply(); sfx_reload(); musicpack_reload();
    }
    ui_dirty_config = 1;
}

extern "C" void ui_set_original_c(int on)          /* (the test command uoriginal) */
{
    ui_set_original(on != 0);
    fprintf(stderr, "original %d: fixes", on);
    for (int i = 0; i < FIX_COUNT; i++) fprintf(stderr, " %d", FIXES[i].on);
    fprintf(stderr, " mods");
    for (int i = 0; i < MOD_COUNT; i++) fprintf(stderr, " %d", MODS[i].enabled);
    fprintf(stderr, "\n");
}

void tab_helpers(void)
{
    bool orig = ui_original_mode();
    if (ImGui::Checkbox("Play the original game", &orig)) ui_set_original(orig);
    ui_hint("Switches everything below off and uses the game's own sound setup; your choices come back when you untick it.");
    if (orig) { ImGui::TextWrapped("Darklands as it was in 1993 (version 483.07). Only the Darklands+ menu itself remains."); return; }

    ImGui::PushFont(ui_font_head); ImGui::SeparatorText("Chances of success"); ImGui::PopFont();
    ImGui::TextWrapped("Town and encounter cards show the chance of success next to each option that has one, "
                       "and who would try it. (The original shows one at a time while you hold Shift.)");
    int c = assist_chances;
    if (ImGui::RadioButton("Always, as numbers", c == 2)) c = 2;
    if (ImGui::RadioButton("As the difficulty level allows (numbers, words or nothing)", c == 1)) c = 1;
    if (ImGui::RadioButton("Off", c == 0)) c = 0;
    if (c != assist_chances) { assist_set_chances(c); ui_dirty_config = 1; }

    ImGui::PushFont(ui_font_head); ImGui::SeparatorText("Cards, screens and lists"); ImGui::PopFont();
    { bool pp = partypanel_is_on() != 0;
      if (ImGui::Checkbox("The party panel drawn by Darklands+: each member's Endurance, Strength and Divine Favor as labelled bars with their numbers", &pp)) { partypanel_set(pp); ui_dirty_config = 1; }
      ImGui::Indent(UIS(34));
      ui_hint("Clicks on it still go to the game (a member's sheet; orders in a battle). A portrait of your own: put NAME.png (or .jpg) in a folder named portraits in the Darklands+ folder, NAME being the member's name. The battle figure is the little one the game shows beside a battle, in the colours chosen for the member.");
      int pic = partypanel_picture();
      static const char *PICS[] = { "Your portrait, else the battle figure, else the herald", "The battle figure, else the herald", "The herald", "No picture" };
      ImGui::SetNextItemWidth(ImGui::CalcTextSize(PICS[0]).x + ImGui::GetFrameHeight() + UIS(24));   /* (the longest whole) */
      if (ImGui::Combo("Picture##ppic", &pic, PICS, 4)) { partypanel_set_picture(pic); ui_dirty_config = 1; }
      ImGui::Unindent(UIS(34)); }
    { bool nm = menuview_is_on() != 0;
      if (ImGui::Checkbox("The game's menus (Game, Orders, Attack, Party ...) drawn by Darklands+: a right click or F10 opens them, a click chooses", &nm)) { menuview_set(nm); ui_dirty_config = 1; }
      ImGui::Indent(UIS(34)); ui_hint("They stay open until you choose or click elsewhere, instead of only while the right button is held (letting go of it over an entry still chooses that entry). The entries and what they do are the game's own. Off: the game's own menu bar."); ImGui::Unindent(UIS(34)); }
    { bool nc2 = cacheview_is_on() != 0;
      if (ImGui::Checkbox("Store rooms drawn by Darklands+: everything stored and everything the member carries in two lists", &nc2)) { cacheview_set(nc2); ui_dirty_config = 1; }
      ImGui::Indent(UIS(34)); ui_hint("Take and Put on each row (one or the whole stack), Put away for everything not in use. The storing is the game's own. Off: the game's screen, four things at a time."); ImGui::Unindent(UIS(34)); }
    { bool ss = config_get_int("store.shared", 1) != 0;
      if (ImGui::Checkbox("One store room for every inn: what is left with one innkeeper is there at every other", &ss)) { config_set_int("store.shared", ss); ui_dirty_config = 1; }
      ImGui::Indent(UIS(34)); ui_hint("Like a chest that follows the party. The stores already left in several towns join it the first time a store room is opened (as long as it has room: 100 kinds of things). Off: nothing more is joined; what is already shared stays so."); ImGui::Unindent(UIS(34)); }
    { bool ncr = createview_is_on() != 0;
      if (ImGui::Checkbox("Making a character drawn by Darklands+: each step on one page, with what a childhood or profession would bring", &ncr)) { createview_set(ncr); ui_dirty_config = 1; }
      ImGui::Indent(UIS(34)); ui_hint("The attributes after it (and the aging past thirty), each skill's gain and room, the virtues and saints learned. The rules are the game's own. Off: the game's screen."); ImGui::Unindent(UIS(34)); }
    { bool nro = rosterview_is_on() != 0;
      if (ImGui::Checkbox("Choosing the party of a new game drawn by Darklands+: the four places with their figures, the characters kept, the chosen one's numbers", &nro)) { rosterview_set(nro); ui_dirty_config = 1; }
      ImGui::Indent(UIS(34)); ui_hint("Click to choose, double-click to add or drop; the figure, herald and colours beside the chosen member. Adding, dropping and deleting are the game's own. Off: the game's screen."); ImGui::Unindent(UIS(34)); }
    { bool wf = config_get_int("party.new_member_looks", 1) != 0;
      ImGui::Indent(UIS(34));
      if (ImGui::Checkbox("Each member added gets a look of their own", &wf)) { config_set_int("party.new_member_looks", wf); ui_dirty_config = 1; }
      { bool kl = config_get_int("party.keep_looks", 1) != 0;
        if (ImGui::Checkbox("Characters keep their figure and colours when they join again (at an inn)", &kl)) { config_set_int("party.keep_looks", kl); ui_dirty_config = 1; } }
      ui_hint("The game gives everyone added the same figure (the tall man) in the same colours, so they are hard to tell apart in a battle. This gives a woman the woman figure and each member the next colours (Figure and Colours change them as before)."); ImGui::Unindent(UIS(34)); }
    { bool nt = titleview_is_on() != 0;
      if (ImGui::Checkbox("The start screen drawn by Darklands+: Continue (the newest save), Load, a new party, Quickstart, over the game's art", &nt)) { titleview_set(nt); ui_dirty_config = 1; } }
    { bool ni = infoview_is_on() != 0;
      if (ImGui::Checkbox("Party Info drawn by Darklands+: fame and its title, the time, where, the wealth, and the reputation in every city", &ni)) { infoview_set(ni); ui_dirty_config = 1; }
      ImGui::Indent(UIS(34)); ui_hint("The original names one town at a time under the pointer on its little map; this lists all of them, by reputation or by name. Off: the game's screen."); ImGui::Unindent(UIS(34)); }
    { bool nmo = marchview_is_on() != 0;
      if (ImGui::Checkbox("Change Marching Order as a list drawn by Darklands+: move members up and down, then Done", &nmo)) { marchview_set(nmo); ui_dirty_config = 1; }
      ImGui::Indent(UIS(34)); ui_hint("Instead of clicking each member in turn. The order is the game's own (single file follows it). Off: the game's way."); ImGui::Unindent(UIS(34)); }
    { bool nlo = lodgeview_is_on() != 0;
      if (ImGui::Checkbox("Inns and camps drawn by Darklands+: each member's day, its cost, the teachers and their fees, and \"Until rested\"", &nlo)) { lodgeview_set(nlo); ui_dirty_config = 1; } }
    { bool nei = enemyview_is_on() != 0;
      if (ImGui::Checkbox("Enemy Info (E in a battle) drawn by Darklands+: every kind of foe, its armour and weapons, and what your weapons do to it", &nei)) { enemyview_set(nei); ui_dirty_config = 1; } }
    { bool nmm = mapmsg_is_on() != 0;
      if (ImGui::Checkbox("The travel map's messages (a robber knight's land ...) in the cards' style", &nmm)) { mapmsg_set(nmm); ui_dirty_config = 1; } }
    { bool np = prayview_is_on() != 0;
      if (ImGui::Checkbox("Calling on a saint drawn by Darklands+: the statue, what the saint does, the favor to spend and the chance", &np)) { prayview_set(np); ui_dirty_config = 1; }
      ImGui::Indent(UIS(34)); ui_hint("The chance and the favor are the saint's own rules. Off: the game's prayer screen."); ImGui::Unindent(UIS(34)); }
    { bool nv = savesview_is_on() != 0;
      if (ImGui::Checkbox("The game's Save and Load drawn by Darklands+: the saves listed with where, when and who", &nv)) { savesview_set(nv); ui_dirty_config = 1; }
      ImGui::Indent(UIS(34)); ui_hint("Save asks for the comment on a page that lists the saves there are; Load lists them all to choose from (no question first). The saving and loading are the game's own. Off: the game's comment prompt and its list of nine."); ImGui::Unindent(UIS(34)); }
    { bool nc = cardview_on != 0;
      if (ImGui::Checkbox("Cards drawn by Darklands+: their text in the chosen typeface, the options as rows to click, chances beside them", &nc)) { cardview_set(nc); ui_dirty_config = 1; }
      ImGui::Indent(UIS(34)); ui_hint("The game still decides everything on a card; only the text and the choosing are new. An option that uses an item or calls on a saint lists, under it, who can. Off: the game draws its cards itself (with the crisp text over them)."); ImGui::Unindent(UIS(34)); }
    { bool ns = barterview_is_on() != 0;
      if (ImGui::Checkbox("Shops drawn by Darklands+: all the wares and all your things in two lists, Buy and Sell on each, what is in use marked", &ns)) { barterview_set(ns); ui_dirty_config = 1; }
      ImGui::Indent(UIS(34)); ui_hint("The merchant's stock and prices are the game's own. The bottom of the screen describes whatever the pointer is on (no tooltips over the lists). Off: the game's own shop screen."); ImGui::Unindent(UIS(34)); }
    { bool sv = sheetview_is_on() != 0;
      if (ImGui::Checkbox("Character sheets drawn by Darklands+: the paper doll, the things with Ready, Unequip, Drink and Drop, the skills on one page, the party's saints and formulae", &sv)) { sheetview_set(sv); ui_dirty_config = 1; }
      ImGui::Indent(UIS(34)); ui_hint("Drag a thing onto a member in the party panel to give it (Shift: one of a stack); a drop can be undone while the sheet is open. Equipping, giving, dropping and drinking are the game's own. Off: the game's own sheet.");
      if (sv) { bool sp = config_get_int("inventory.shared", 1) != 0;
        if (ImGui::Checkbox("One pack for the whole party: a member keeps only what they use; Ready or Drink takes from the party's pack, shops sell from it", &sp)) { config_set_int("inventory.shared", sp); ui_dirty_config = 1; }
        ui_hint("Who carries a spare thing no longer matters (the pack weighs nothing; only what is in use counts for the load). Potions stay each member's own. Where the game asks one member for a thing, it is handed to them first: lock picks at a lock, a horse or mule to whoever has none (each rides their best), arrows, quarrels and balls before a battle. Keep marks a thing never to be sold or dropped."); }
      ImGui::Unindent(UIS(34)); }
    { bool lv = lootview_is_on() != 0;
      if (ImGui::Checkbox("The loot drawn by Darklands+: the whole pile and each member's things in two lists, a whole stack in one click", &lv)) { lootview_set(lv); ui_dirty_config = 1; }
      ImGui::Indent(UIS(34)); ui_hint("The game's own G and P still move every thing. Off: the game's own loot screen.");
      if (lv) { bool lw = config_get_int("assist.loot_worth", 1) != 0;
        if (ImGui::Checkbox("Under the lists: about what a shop pays for the thing under the pointer", &lw)) { config_set_int("assist.loot_worth", lw); ui_dirty_config = 1; } }
      ImGui::Unindent(UIS(34)); }
    { bool av = alchemyview_is_on() != 0;
      if (ImGui::Checkbox("Alchemy drawn by Darklands+: every known formula in one list, its ingredients at hand, batches with - and +, the chance of success", &av)) { alchemyview_set(av); ui_dirty_config = 1; }
      ImGui::Indent(UIS(34)); ui_hint("The chance and the mixing are the game's own. Off: the game's own alchemy screen."); ImGui::Unindent(UIS(34));
      if (av) {
        ImGui::Indent(UIS(34));
        bool ws = config_get_int("assist.where_sold", 1) != 0;
        if (ImGui::Checkbox("Beside an ingredient that runs short: the nearest town seen selling it", &ws)) { config_set_int("assist.where_sold", ws); ui_dirty_config = 1; }
        if (ws) { bool wu = config_get_int("assist.where_sold_unvisited", 1) != 0;
          ImGui::Indent();
          if (ImGui::Checkbox("or, if none, the nearest town whose shops will have it", &wu)) { config_set_int("assist.where_sold_unvisited", wu); ui_dirty_config = 1; }
          ImGui::Unindent(); }
        ImGui::Unindent(UIS(34));
      } }
    { bool bl = assist_best_leader != 0;
      if (ImGui::Checkbox("The member best at it speaks for the party: a card's test of the leader is made by whoever does best, and the best charisma bargains in shops", &bl)) { assist_set_best_leader(bl); ui_dirty_config = 1; }
      ImGui::Indent(UIS(34)); ui_hint("No more changing the leader for each talk, lore or haggle; the chance shown on the card is that member's and their name is beside it. The leader you chose is back after the card."); ImGui::Unindent(UIS(34)); }
    bool ch = assist_card_help;
    if (ImGui::Checkbox("Show which saints and items work on a card, and who knows or carries them", &ch)) { assist_set_card_help(ch); ui_dirty_config = 1; }
    ImGui::Indent(UIS(34)); ui_hint("Like the Darklands Companion: the saints a card accepts are listed even when nobody in the party knows them yet."); ImGui::Unindent(UIS(34));
    { extern int ui_shop_info; bool si = ui_shop_info != 0;
      if (ImGui::Checkbox("Tooltips: what the item, saint or formula under the mouse is good for (shops, loot, character sheet, saints, formulae)", &si)) { ui_shop_info = si; config_set_int("assist.shop_info", si); ui_dirty_config = 1; } }
    { bool rt = config_get_int("assist.return_thrown", 1) != 0;
      if (ImGui::Checkbox("After a battle, javelins, throwing axes, knives and darts go back to whoever threw them", &rt)) { config_set_int("assist.return_thrown", rt); loot_return_thrown = rt; ui_dirty_config = 1; } }
    { bool ia = ammo_infinite != 0;
      if (ImGui::Checkbox("Arrows, quarrels and balls never run out (one in the pack is enough to shoot)", &ia)) { ammo_set(ia); ui_dirty_config = 1; }
      ImGui::Indent(UIS(34)); ui_hint("The party's shots take nothing from the pack and leave nothing to pick up after the battle. Enemies still shoot and lose theirs. Off: each shot uses one, and the game leaves 7 in 10 arrows and quarrels to be found in the loot."); ImGui::Unindent(UIS(34)); }
    { bool it = thrown_infinite != 0;
      if (ImGui::Checkbox("Javelins, throwing axes, throwing knives and darts never run out either (one in the pack is enough to throw)", &it)) { thrown_set(it); ui_dirty_config = 1; } }
    { bool la = config_get_int("assist.loot_all", 1) != 0;
      if (ImGui::Checkbox("Loot and shops: take the whole pile (A) or share it out (S); sell a whole stack (A), buy ten (T)", &la)) { config_set_int("assist.loot_all", la); ui_dirty_config = 1; } }
    { extern int ui_creation_guide; bool cg = ui_creation_guide != 0;
      if (ImGui::Checkbox("Character creation: a short guide to each step in the empty column on the left", &cg)) { ui_creation_guide = cg; config_set_int("assist.creation_guide", cg); ui_dirty_config = 1; } }
    { bool mf = config_get_int("party.more_figures", 1) != 0;
      if (ImGui::Checkbox("Character creation: Select Character Image also offers the enemies' human figures (sergeant, monk, knight, Templar ... 14 more)", &mf)) { config_set_int("party.more_figures", mf); ui_dirty_config = 1; }
      ImGui::Indent(UIS(34)); ui_hint("Their own walking, fighting and dying animations, with colours of their own for the 1st/2nd/3rd color keys. A figure without an animation for the weapon in hand swings the nearest one it has. Saves keep the figure; the original game cannot show these figures."); ImGui::Unindent(UIS(34));
      if (mf) { ImGui::Indent(UIS(34)); bool fp = config_get_int("party.figure_picker", 1) != 0;
        if (ImGui::Checkbox("Select Character Image shows them all at once to pick from (else it steps to the next, as in the original)", &fp)) { config_set_int("party.figure_picker", fp); ui_dirty_config = 1; }
        ImGui::Unindent(UIS(34)); } }
    { bool ff = config_get_int("assist.fast_fades", 1) != 0;
      if (ImGui::Checkbox("Cards show their options at once (no slow fade-in)", &ff)) { config_set_int("assist.fast_fades", ff); ui_dirty_config = 1; } }
    bool ck = assist_card_keys;
    if (ImGui::Checkbox("Number keys 1-9 pick a card's options", &ck)) { assist_set_card_keys(ck); ui_dirty_config = 1; }
    { bool wl = config_get_int("assist.wheel_lists", 1) != 0;
      if (ImGui::Checkbox("The mouse wheel moves in the game's lists (equipment, shops ...)", &wl)) { config_set_int("assist.wheel_lists", wl); ui_dirty_config = 1; } }
    { bool gp = config_get_int("input.gamepad", 1) != 0;
      if (ImGui::Checkbox("Game controller: the left stick moves the pointer, A clicks, B right-clicks (F1 lists the rest)", &gp)) { config_set_int("input.gamepad", gp); ui_dirty_config = 1; }
      ui_hint(plat_controller_connected() ? "A controller is connected." : "No controller connected now."); }
    { bool mc = config_get_int("assist.menu_click", 1) != 0;
      if (ImGui::Checkbox("A right click opens the menu bar (the next click chooses; Esc closes it)", &mc)) { config_set_int("assist.menu_click", mc); ui_dirty_config = 1; }
      if (ImGui::IsItemHovered()) ImGui::SetTooltip("The game shows its menus only while the right button is held down, and takes the entry under the pointer when\nit is let go. That still works; this adds the click. F10 opens the menus from the keyboard (arrows, Enter, Esc)."); }

    ImGui::PushFont(ui_font_head); ImGui::SeparatorText("Battles"); ImGui::PopFont();
    { bool d = autopause_down, h = autopause_hurt;
      if (ImGui::Checkbox("Pause the battle when someone in the party falls", &d)) { autopause_set(d, h); ui_dirty_config = 1; }
      if (ImGui::Checkbox("Pause the battle when someone is badly hurt (endurance below a quarter)", &h)) { autopause_set(d, h); ui_dirty_config = 1; }
      bool fo = autopause_foes != 0;
      if (ImGui::Checkbox("Pause the battle when foes waiting in a castle, dungeon or mine come out to fight", &fo)) { autopause_set_foes(fo); ui_dirty_config = 1; } }
    { extern int ui_battle_info, ui_battle_numbers; bool bi = ui_battle_info != 0, bn = ui_battle_numbers != 0;
    { bool ca = orders_click_attack;
      if (ImGui::Checkbox("In a battle, a click on an enemy sends the selected member at it (Std Attack), even before an attack is chosen", &ca)) { orders_click_attack = ca; config_set_int("battle.click_attack", ca); ui_dirty_config = 1; }
      { bool sp = battlescroll_on != 0, sr = battlescroll_rest != 0;
        if (ImGui::Checkbox("Battle map: scroll at a steady pace (Shift at the edge no longer flies to the end of the field)", &sp)) { battlescroll_set(sp, sr); ui_dirty_config = 1; }
        if (sp) { ImGui::Indent(); if (ImGui::Checkbox("the pointer resting on the edge scrolls, no Shift needed", &sr)) { battlescroll_set(sp, sr); ui_dirty_config = 1; } ImGui::Unindent(); } } }
      if (ImGui::Checkbox("Pointing at someone in a battle shows who it is, how hurt, attack, defence and weapon", &bi)) { ui_battle_info = bi; config_set_int("battle.info", bi); ui_dirty_config = 1; }
      if (bi) { ImGui::Indent(); if (ImGui::Checkbox("enemies' endurance as numbers", &bn)) { ui_battle_numbers = bn; config_set_int("battle.info_numbers", bn); ui_dirty_config = 1; } ImGui::Unindent(); }
      { extern int ui_battle_bars; bool bb = ui_battle_bars != 0;
        if (ImGui::Checkbox("A bar over each fighter: the endurance left (and the strength once it is down by half)", &bb)) { ui_battle_bars = bb; config_set_int("battle.bars", bb); ui_dirty_config = 1; } }
      { extern int ui_battle_log; bool bl = ui_battle_log != 0;
        if (ImGui::Checkbox("Battle log: every blow, parry, wound and fall, in the left margin on a wide screen, else over the field (F12 > Journal keeps it)", &bl)) { ui_battle_log = bl; config_set_int("battle.log", bl); ui_dirty_config = 1; } }
    }
    { bool lm = config_get_int("assist.level_map", 1) != 0, la = config_get_int("assist.level_map_all", 0) != 0;
      if (ImGui::Checkbox("Tab in battles, castles, mines and dungeons: a map of the level as far as the party has seen it", &lm)) { config_set_int("assist.level_map", lm); ui_dirty_config = 1; }
      if (lm) { ImGui::Indent(); if (ImGui::Checkbox("the whole level, not only what the party has seen", &la)) { config_set_int("assist.level_map_all", la); ui_dirty_config = 1; }
                bool lw = config_get_int("assist.level_map_walk", 1) != 0; if (ImGui::Checkbox("a click on it walks the party there", &lw)) { config_set_int("assist.level_map_walk", lw); ui_dirty_config = 1; } ImGui::Unindent(); } }
    { bool sp = config_get_int("battle.shoot_past_allies", 0) != 0;
      if (ImGui::Checkbox("The party's archers, crossbowmen and gunners shoot past their own fighters (no waiting while a friend stands in the line)", &sp)) { config_set_int("battle.shoot_past_allies", sp); ui_dirty_config = 1; }
      ui_hint("A missile never strikes anyone but its target in Darklands (a miss flies past everyone), yet the game makes a shooter wait ('b' on the panel) while anyone stands in the line of fire, unless the shooter's skill is 90 or more. Enemies in the line still block.");
    }
    { bool rs = config_get_int("battle.reposition_shooters", 1) != 0;
      if (ImGui::Checkbox("A shooter without a clear line steps aside or walks on to find one, instead of waiting; any foe can be chosen", &rs)) { config_set_int("battle.reposition_shooters", rs); ui_dirty_config = 1; }
      ui_hint("When something stands in the line of fire (a friend, a foe, a wall's corner), the archer, crossbowman or gunner walks to the nearest spot a few paces away from which the line to the target is clear, and shoots from there. Any foe can be chosen as the target (the game refused one with no clear line, with a barred circle and a beep); with no spot near, the shooter walks towards it until a line opens. A member told to hold stays put; any other order you give takes over.");
    }
    { bool fl = config_get_int("battle.foes_left", 1) != 0;
      if (ImGui::Checkbox("Under the enemies: the foes a castle, dungeon or mine still holds, not yet met, floor by floor", &fl)) { config_set_int("battle.foes_left", fl); ui_dirty_config = 1; }
      ui_hint("The defenders of a place wait in groups at their posts and step out when a member sees them. A castle's or dungeon's battle ends when its master has fallen and his fight is over, or when the party walks out (and flees): the margin marks the master, or names the floor where he waits.");
    }
    { bool ew = config_get_int("battle.end_words", 1) != 0;
      if (ImGui::Checkbox("The end of a battle said in words (\"Victory! The battle is won.\") for three seconds", &ew)) { config_set_int("battle.end_words", ew); ui_dirty_config = 1; }
      ui_hint("The game closes a battle with a box saying \"BattleResults - victory\" and holds the field still for about eight seconds, as if it had hung.");
    }
    bool pp = pathfix_party, pe = pathfix_enemies;
    if (ImGui::Checkbox("Better pathfinding for the party (no more getting stuck in mines)", &pp)) { pathfix_set(pp, pe); ui_dirty_config = 1; }
    if (ImGui::Checkbox("Better pathfinding for enemies too (harder battles)", &pe)) { pathfix_set(pp, pe); ui_dirty_config = 1; }
    ui_hint("Moves are planned around walls and furniture with a full route search instead of the "
                        "original wall-following, which gives up on long or winding routes.");
    {
        ImGui::TextUnformatted("Speed of battles and dungeons:"); ImGui::SameLine();
        static const int SPD[4] = { 100, 150, 200, 300 }; static const char *SPN[4] = { "as the original", "1.5x", "2x", "3x" };
        for (int i = 0; i < 4; i++) {
            if (i) ImGui::SameLine();
            if (ImGui::RadioButton(SPN[i], battle_speed == SPD[i])) { battle_speed_set(SPD[i]); ui_dirty_config = 1; }
        }
        ui_hint("Walking (also through dungeons, castles and mines), attacks and spells all go faster; pausing (Space) works as before.");
        bool fk = config_get_int("battle.fast_key", 1) != 0;
        if (ImGui::Checkbox("Z in a battle: fast forward (six times the pace), Z again back to this speed", &fk)) { config_set_int("battle.fast_key", fk); if (!fk) battle_fast_set(0); ui_dirty_config = 1; }
    }

    ImGui::PushFont(ui_font_head); ImGui::SeparatorText("Travel"); ImGui::PopFont();
    bool ta = travel_enabled; int tf = travel_fast;
    if (ImGui::Checkbox("Auto-travel along roads (Travel tab, or Ctrl+click on the map)", &ta)) { travel_set(ta, tf); ui_dirty_config = 1; }
    bool cg = config_get_int("assist.city_goto", 1);
    if (ImGui::Checkbox("Go to in cities: G on a city's card lists its places; the party walks there through the menus", &cg)) { config_set_int("assist.city_goto", cg); ui_dirty_config = 1; }
    { bool mn = config_get_int("map.names", 1) != 0;
      if (ImGui::Checkbox("Names of the towns and places on the travel map", &mn)) { config_set_int("map.names", mn); ui_dirty_config = 1; } }
    { bool qm = config_get_int("map.quest_marks", 1) != 0;
      if (ImGui::Checkbox("The quests' pictures above their places on the travel map, and a sign beside a town that bars the party or whose watch wants it", &qm)) { config_set_int("map.quest_marks", qm); ui_dirty_config = 1; } }
    {   /* the game's rate of encounters on the road (mods/travel.c) */
        static const int P[] = { 25, 50, 100, 200 };
        static const char *N[] = { "rare (a quarter)", "fewer (half)", "as the game", "more (double)" };
        int cur = config_get_int("travel.encounters", 100), sel = 2;
        for (int i = 0; i < 4; i++) if (P[i] == cur) sel = i;
        ImGui::SetNextItemWidth(UIS(200));
        if (ImGui::Combo("encounters on the road", &sel, N, 4)) { config_set_int("travel.encounters", P[sel]); ui_dirty_config = 1; }
        int cs = config_get_int("travel.street_encounters", 100), ss = 2;
        for (int i = 0; i < 4; i++) if (P[i] == cs) ss = i;
        ImGui::SetNextItemWidth(UIS(200));
        if (ImGui::Combo("thieves and the watch in town streets at night", &ss, N, 4)) { config_set_int("travel.street_encounters", P[ss]); ui_dirty_config = 1; }
        ui_hint("At night the side streets bring cutthroats about 8 times in 10 for a party with little Stealth and Streetwise, and the main street the watch now and then; \"rare\" makes both a quarter as likely.");
    }
    ImGui::TextUnformatted("Pace of auto-travel:"); ImGui::SameLine();
    if (ImGui::RadioButton("as walking", tf == 0)) { travel_set(ta, 0); ui_dirty_config = 1; } ImGui::SameLine();
    if (ImGui::RadioButton("4x", tf == 1)) { travel_set(ta, 1); ui_dirty_config = 1; } ImGui::SameLine();
    if (ImGui::RadioButton("at once", tf == 2)) { travel_set(ta, 2); ui_dirty_config = 1; }

    ImGui::PushFont(ui_font_head); ImGui::SeparatorText("Town journal"); ImGui::PopFont();
    bool j = journal_enabled();
    if (ImGui::Checkbox("Keep a journal of the towns the party visits", &j)) { journal_set(j); ui_dirty_config = 1; }
    ui_hint("Each town's shops, their wares, qualities and prices, noted on arrival (Journal tab).");
    if (j) {
        bool ch = config_get_int("journal.chronicle", 1) != 0;
        if (ImGui::Checkbox("Keep a chronicle of every text the cards show", &ch)) { config_set_int("journal.chronicle", ch); ui_dirty_config = 1; }
        ui_hint("Word for word, with the date, the place and the choice made: a clue read weeks ago can be found again (Journal > Chronicle).");
    }

    ImGui::PushFont(ui_font_head); ImGui::SeparatorText("Editor"); ImGui::PopFont();
    bool ed = config_get_int("editor.show", 0);
    if (ImGui::Checkbox("Show the character editor (Editor tab)", &ed)) { config_set_int("editor.show", ed); ui_dirty_config = 1; }
    ui_hint("Change money, names, attributes, skills, saints, formulae, items, and a character's colours, shield and "
         "figure, as the community's save-game editors do. For repairing characters hit by old bugs, or for cheating.");

    ImGui::PushFont(ui_font_head); ImGui::SeparatorText("Fixes for bugs of the original"); ImGui::PopFont();
    for (int i = 0; i < FIX_COUNT; i++) {
        bool on = FIXES[i].on;
        ImGui::PushID(i);
        if (ImGui::Checkbox(FIXES[i].name, &on)) { fix_enable(i, on); ui_dirty_config = 1; }
        ImGui::Indent(UIS(34)); ui_hint("%s", FIXES[i].desc); ImGui::Unindent(UIS(34));
        ImGui::PopID();
    }
}

extern "C" { int mpu_out_count(void); const char *mpu_out_name(int i); int mt32_families(void); int mt32_switch_pending(void); int mt32_switch(void); const char *mt32_status(void); void mt32_rescan(void); int mt32_dir_count(void); const char *mt32_dir(int); extern int sound_music_volume; }
void tab_display(void)
{
    bool s = ui_settings.smooth, n = ui_settings.integer;   /* (the picture is always 4:3, as on a 1992 monitor) */
    if (ImGui::Checkbox("Smooth scaling", &s)) { ui_settings.smooth = s; config_set_int("display.smooth", s); ui_dirty_config = 1; }
    if (ImGui::Checkbox("Whole-number scaling (sharpest pixels)", &n)) { ui_settings.integer = n; config_set_int("display.integer_scale", n); ui_dirty_config = 1; }
    {
        ImGui::TextUnformatted("Pixel edges:"); ImGui::SameLine();
        static const char *SC[3] = { "as drawn", "smoothed (Scale2x)", "smoothed more (Scale4x)" };
        for (int i = 0; i < 3; i++) {
            if (i) ImGui::SameLine();
            if (ImGui::RadioButton(SC[i], ui_settings.scaler == i)) { ui_settings.scaler = i; config_set_int("display.scaler", i); ui_dirty_config = 1; }
        }
        ui_hint("Rounds off the stair-steps of diagonal edges in the pictures and figures without blurring them "
             "(the EPX/Scale2x rule: a pixel's corner takes its neighbours' colour where two of them agree). "
             "Scale4x needs a graphics card: without one (a virtual machine, say) the game slows down.");
    }
    { extern int ui_margins_on; bool mg = ui_margins_on != 0;
      if (ImGui::Checkbox("Wide screens: date, place, purse, tasks, the town and effects in the side margins (in battles: the log and everyone's condition)", &mg)) { ui_margins_on = mg; config_set_int("display.margins", mg); ui_dirty_config = 1; } }
    {   /* grows with the picture; this scales it */
        static const int P[] = { 70, 80, 90, 100, 115, 130, 150, 175, 200 };
        int cur = config_get_int("display.text_size", 100), sel = 3;
        for (int i = 0; i < 9; i++) if (P[i] == cur) sel = i;
        char lab[16]; snprintf(lab, sizeof lab, "%d %%", P[sel]);
        ImGui::SetNextItemWidth(UIS(160));
        if (ImGui::BeginCombo("text size in the margins, tooltips and on the map", lab)) {
            for (int i = 0; i < 9; i++) { char t[16]; snprintf(t, sizeof t, "%d %%", P[i]); if (ImGui::Selectable(t, i == sel)) { config_set_int("display.text_size", P[i]); ui_dirty_config = 1; } }
            ImGui::EndCombo();
        }
    }
    { bool sc = ui_sys_cursor != 0;
      if (ImGui::Checkbox("Show the system mouse pointer over the game too", &sc)) { ui_sys_cursor = sc; config_set_int("display.system_cursor", sc); ui_dirty_config = 1; } }
    {   /* the menus' size (fonts are baked at start-up) */
        static const int P[] = { 0, 100, 125, 150, 175, 200, 250 };
        static const char *N[] = { "automatic (the system's scale)", "100 %", "125 %", "150 %", "175 %", "200 %", "250 %" };
        int cur = config_get_int("display.ui_scale", 0), sel = 0;
        for (int i = 0; i < 7; i++) if (P[i] == cur) sel = i;
        ImGui::SetNextItemWidth(UIS(260));
        if (ImGui::Combo("size of these menus (after a restart)", &sel, N, 7)) { config_set_int("display.ui_scale", P[sel]); ui_dirty_config = 1; }
    }
    { bool ir = idle_rest != 0;
      if (ImGui::Checkbox("Let the computer rest while the game waits for a click or a key (less CPU, quieter fans)", &ir)) { idle_rest = ir; config_set_int("display.idle_rest", ir); ui_dirty_config = 1; } }
    { bool pu = config_get_int("display.pause_unfocused", 1) != 0;
      if (ImGui::Checkbox("Pause the game while its window is in the background", &pu)) { config_set_int("display.pause_unfocused", pu); ui_dirty_config = 1; } }
    {   /* the opening at start (mods/opening.c): the first start plays it whole */
        static const char *N[] = { "nothing: straight to the title", "the last scene: the lady of the Darklands and her welcome", "the whole opening (logo and animations)" };
        int m = opening_mode();
        ImGui::SetNextItemWidth(UIS(420));
        if (ImGui::Combo("at start", &m, N, 3)) { opening_set_mode(m); ui_dirty_config = 1; }
        ImGui::SameLine(); ImGui::TextDisabled("(a key or a click ends it)");
    }
    bool fs = (SDL_GetWindowFlags(ui_window) & SDL_WINDOW_FULLSCREEN_DESKTOP) != 0;
    if (ImGui::Checkbox("Full screen (Alt+Enter)", &fs)) SDL_SetWindowFullscreen(ui_window, fs ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
    ImGui::PushFont(ui_font_head); ImGui::SeparatorText("Text"); ImGui::PopFont();
    int hm = hdtext_mode;
    if (ImGui::RadioButton("Crisp text on story cards only", hm == 1)) { hdtext_set(1); ui_dirty_config = 1; }
    if (ImGui::RadioButton("Crisp text everywhere (cards, party, sheets, menus, shops)", hm == 2)) { hdtext_set(2); ui_dirty_config = 1; }
    if (ImGui::RadioButton("The game's own lettering", hm == 0)) { hdtext_set(0); ui_dirty_config = 1; }
    if (hm) {
        ImGui::SetNextItemWidth(UIS(380));
        if (ImGui::BeginCombo("Typeface", UI_GAME_FONT_NAMES[ui_game_font])) {
            for (int i = 0; i < UI_GAME_FONTS; i++) {
                ImGui::PushFont(ui_font_game[i]);
                if (ImGui::Selectable(UI_GAME_FONT_NAMES[i], i == ui_game_font)) { ui_game_font = i; config_set_int("display.text_font", i); ui_overlay_font_update(); ui_dirty_config = 1; }
                ImGui::PopFont();
            }
            ImGui::EndCombo();
        }
        { bool of = config_get_int("display.overlay_font_follow", 1) != 0;
          if (ImGui::Checkbox("The same typeface for the side panels, tooltips and names on the maps", &of)) { config_set_int("display.overlay_font_follow", of); ui_overlay_font_update(); ui_dirty_config = 1; } }
        { bool so = ui_spell_out != 0;
          if (ImGui::Checkbox("Spell out the character sheet's abbreviations (wPol -> Polearm Wpns, R&W -> Read & Write)", &so)) { ui_spell_out = so; config_set_int("display.spell_out", so); ui_dirty_config = 1; } }
        { bool co = money_coins();
          if (ImGui::Checkbox("Money with coins: 1" COIN_FL " 12" COIN_GR " 4" COIN_PF " (florins, groschen, pfennigs) instead of 1fl 12gr 4pf; the total in pfennigs beside a price described", &co)) { config_set_int("display.coins", co); ui_dirty_config = 1; } }
        ImGui::TextUnformatted("Letter edges:"); ImGui::SameLine();
        static const char *E[3] = { "plain", "shadow", "outline" };
        for (int i = 0; i < 3; i++) {
            if (i) ImGui::SameLine();
            if (ImGui::RadioButton(E[i], ui_text_edge == i)) { ui_text_edge = i; config_set_int("display.text_edge", i); ui_dirty_config = 1; }
        }
        { extern int ui_skill_icons; bool si = ui_skill_icons != 0;
          if (ImGui::Checkbox("Small pictures beside the skills of the character sheet", &si)) { ui_skill_icons = si; config_set_int("display.skill_icons", si); ui_dirty_config = 1; } }
        { extern int ui_text_contrast; bool tc = ui_text_contrast != 0;
          if (ImGui::Checkbox("Darken (or lighten) faint text against its background", &tc)) { ui_text_contrast = tc; config_set_int("display.text_contrast", tc); ui_dirty_config = 1; } }
    }
    ui_hint("The original 7-pixel blackletter is hard to read when scaled up. Crisp text draws the same words at "
         "the window's resolution; anything it cannot match exactly is left as the game drew it.");

    ImGui::PushFont(ui_font_head); ImGui::SeparatorText("Pictures"); ImGui::PopFont();
    bool ha = hdart_on;
    if (ImGui::Checkbox("Use replacement art from the \"art\" folder", &ha)) { hdart_set(ha); ui_dirty_config = 1; }
    {
        extern char game_dir[1024];
        int n = hdart_count();
        extern char dlp_user_dir[];
        if (n) ui_hint("%d replacement picture%s in %s%sart.", n, n == 1 ? "" : "s", dlp_user_dir, PATH_SEP);
        else ui_hint("No replacement pictures yet (%s%sart).", dlp_user_dir, PATH_SEP);
        static char msg[600];
        if (ImGui::Button("Look again")) { hdart_rescan(); snprintf(msg, sizeof msg, "%d replacement pictures found.", hdart_count()); }
        ImGui::SameLine();
        if (ImGui::Button("Export the original pictures")) hdart_export(msg, sizeof msg);
        if (msg[0]) ui_hint("%s", msg);
        bool sn = hdart_show_names;
        if (ImGui::Checkbox("Show the name of the picture under the mouse", &sn)) { hdart_show_names = sn; config_set_int("display.art_names", sn); ui_dirty_config = 1; }
        ui_hint("For artists: a file art%sNAME.png (any size; transparency where the picture has colour 0) replaces the "
             "game's picture NAME wherever it is shown, at the window's resolution. Export writes every original picture "
             "to art%soriginal as a template, in its own colours. Whatever the game draws over a picture (text, the "
             "mouse pointer, other pictures) stays on top.", PATH_SEP, PATH_SEP);
        ui_hint("More crests: art%sSHIELDP.png, SHIELDQ.png ... up to SHIELDZ.png add crests after the fifteen of the game "
             "(Heraldry, when making a party). A character with one of them needs this version of the game to be loaded.", PATH_SEP);
    }

    ImGui::PushFont(ui_font_head); ImGui::SeparatorText("Sound"); ImGui::PopFont();
    static const char *ids[] = { "auto", "mt32", "fm", "adlib", "midi", "speaker", "off", "game" };
    static const char *labels[] = {
        "Automatic: the Roland MT-32 when its ROM files are found, otherwise the Sound Blaster - recommended",
        "Roland MT-32 / LAPC-I, emulated (Munt; needs the MT-32 or CM-32L ROM files)",
        "Sound Blaster Pro (OPL-3 FM, emulated)",
        "AdLib / Sound Blaster (OPL-2 FM, emulated)",
        "Roland MIDI to your system's MIDI device (a real MT-32, or Munt installed as a device)",
        "PC speaker",
        "Off",
        "As in the game's own setup (INSTALL)" };
    const char *cur = config_get("sound.device", "auto");
    for (int i = 0; i < 8; i++)
        if (ImGui::RadioButton(labels[i], !strcmp(cur, ids[i]))) { config_set("sound.device", ids[i]); ui_dirty_config = 1; }
    ui_hint("Takes effect after the next battle, or when the game is started again.");
    if (!strcmp(cur, "midi")) {                     /* which MIDI output */
        ImGui::Indent(UIS(30));
        int n = mpu_out_count();
        const char *out = config_get("sound.midi_out", "");
        if (!n) ui_hint("No MIDI outputs on this system (Windows lists them here).");
        else {
            ImGui::SetNextItemWidth(UIS(420));
            if (ImGui::BeginCombo("MIDI output", out[0] ? out : "Windows' default")) {
                if (ImGui::Selectable("Windows' default", !out[0])) { config_set("sound.midi_out", ""); ui_dirty_config = 1; }
                for (int i = 0; i < n; i++) {
                    const char *nm = mpu_out_name(i);
                    if (nm[0] && ImGui::Selectable(nm, !strcmp(nm, out))) { config_set("sound.midi_out", nm); ui_dirty_config = 1; }
                }
                ImGui::EndCombo();
            }
            ui_hint("A real MT-32 on a MIDI interface, or a synthesizer installed as a MIDI device (Munt's mt32emu driver, "
                 "a Sound Canvas ...). The game's Roland driver loads its instruments when it starts, so the choice takes "
                 "effect after the next battle, or at the next start.");
        }
        ImGui::Unindent(UIS(30));
    }
    {
        int v = sound_music_volume;
        ImGui::SetNextItemWidth(UIS(260));
        if (ImGui::SliderInt("Volume of the music", &v, 0, 150, "%d%%")) { sound_music_volume = v; config_set_int("sound.music_volume", v); ui_dirty_config = 1; }
    }
    if (!strcmp(cur, "auto") || !strcmp(cur, "mt32")) {
        ImGui::Indent(UIS(30));
        ui_hint("%s", mt32_status());
        static char romdir[1024]; static bool init;
        if (!init) { snprintf(romdir, sizeof romdir, "%s", config_get("sound.mt32_roms", "")); init = true; }
        ImGui::SetNextItemWidth(UIS(420));
        if (ImGui::InputTextWithHint("##mt32roms", "the folder with the ROM files, if they are elsewhere", romdir, sizeof romdir)) { config_set("sound.mt32_roms", romdir); ui_dirty_config = 1; }
        ImGui::SameLine();
        if (ImGui::Button("Look again##mt32")) { mt32_rescan(); mt32_switch(); }
        if (mt32_families() == 3) {                /* both: which one */
            const char *cur = config_get("sound.mt32_model", "mt32");
            ImGui::TextUnformatted("Both found; play on the:");
            ImGui::Indent();
            if (ImGui::RadioButton("MT-32 (recommended: battle sounds as blows and grunts)", !strcmp(cur, "mt32"))) { config_set("sound.mt32_model", "mt32"); mt32_rescan(); mt32_switch(); ui_dirty_config = 1; }
            if (ImGui::RadioButton("CM-32L / LAPC-I (its extra sound effects turn the battle sounds into chimes)", !strcmp(cur, "cm32l"))) { config_set("sound.mt32_model", "cm32l"); mt32_rescan(); mt32_switch(); ui_dirty_config = 1; }
            ImGui::Unindent();
        }
        {   bool lcd = config_get_int("sound.mt32_lcd", 1);
            if (ImGui::Checkbox("Show the synthesizer's display when the game writes to it", &lcd)) { config_set_int("sound.mt32_lcd", lcd); ui_dirty_config = 1; } }
        if (ImGui::TreeNode("About the MT-32")) {
            ImGui::TextWrapped("Darklands was composed for the Roland MT-32 and LAPC-I: its Roland driver loads its own "
                               "instruments into the synthesizer and plays the music and the battle sounds with them. Munt's "
                               "emulator of the MT-32, CM-32L and LAPC-I is built into Darklands+, but it needs the synthesizer's "
                               "two ROM images (control and PCM), which belong to Roland and cannot come with Darklands+. If you "
                               "use Munt or DOSBox with an MT-32, you already have them: Darklands+ finds them by their contents "
                               "(any file names) in these folders, a CM-32L / LAPC-I pair first:");
            for (int i = 0; i < mt32_dir_count(); i++) ui_hint("%s", mt32_dir(i));
            ImGui::TreePop();
        }
        ImGui::Unindent(UIS(30));
    }
    {
        ImGui::Spacing();
        bool sp = sfx_mode, hs = sfx_added_hits, sy = sfx_synth;
        extern char game_dir[1024];
        if (ImGui::Checkbox("Battle sounds from a sound pack (the \"sounds\" folder)", &sp)) { sfx_set(sp); ui_dirty_config = 1; }
        int n = sfx_user_count();
        extern char dlp_user_dir[];
        if (n) ui_hint("%d sound%s in %s%ssounds; the game's own sounds for the rest.", n, n == 1 ? "" : "s", dlp_user_dir, PATH_SEP);
        else ui_hint("No sound pack in use: put one into %s%ssounds, or tick a sound mod in F12 > Settings > Mods (\"Battle sounds\" comes with Darklands+).", dlp_user_dir, PATH_SEP);
        if (sp) {
            ImGui::Indent(UIS(30));
            if (ImGui::Checkbox("Blows that land make a sound too (the original is silent there)", &hs)) { sfx_set_hits(hs); ui_dirty_config = 1; }
            if (ImGui::Checkbox("Experimental: synthesized sounds where the pack has none", &sy)) { sfx_set_synth(sy); ui_dirty_config = 1; }
            int v = sfx_volume;
            ImGui::SetNextItemWidth(UIS(260));
            if (ImGui::SliderInt("Volume of the pack", &v, 0, 150, "%d%%")) { sfx_set_volume(v); ui_dirty_config = 1; }
            static char msg[400];
            if (ImGui::Button("Look again##sfx")) { sfx_reload(); snprintf(msg, sizeof msg, "%d sounds found.", sfx_user_count()); }
            ImGui::SameLine();
            if (ImGui::Button("Write the synthesized sounds as .wav")) sfx_export(msg, sizeof msg);
            if (msg[0]) ui_hint("%s", msg);
            ImGui::Unindent(UIS(30));
        }
        if (ImGui::TreeNode("File names for a sound pack")) {
            for (int k = 0; k < SFX_COUNT; k++) ui_hint("%s.wav or .ogg: %s", SFX_NAMES[k], SFX_DESC[k]);
            ui_hint("Several versions of one sound (parry1.wav, parry2.wav ...) are played in turn at random.");
            ImGui::TreePop();
        }
    }
    {   /* music packs (mods/musicpack.c) */
        ImGui::Spacing();
        bool mp = musicpack_on;
        extern char dlp_user_dir[];
        if (ImGui::Checkbox("Music from a music pack (recordings in a \"music\" folder)", &mp)) { musicpack_set(mp); ui_dirty_config = 1; }
        int n = musicpack_count();
        if (n) {
            const char *f0 = ""; for (int k = 0; k < 60 && !f0[0]; k++) f0 = musicpack_from(k);
            ui_hint("%d piece%s in the pack (%s); the synthesizer plays the others.", n, n == 1 ? "" : "s", strcmp(f0, "music") ? f0 : "the music folder");
        }
        else ui_hint("No music pack: a mod's music folder, or %s%smusic, with a recording for each piece named by its number "
                  "(10.ogg, \"11 market.ogg\", .ogg or .wav). Darklands+ comes with none.", dlp_user_dir, PATH_SEP);
        int fp = 0, now = musicpack_now(&fp);
        if (now >= 0) {
            const char *d = musicpack_desc(now);
            ui_hint("The last piece started: %d%s%s%s, from the %s.", now, d[0] ? " (" : "", d, d[0] ? ")" : "", fp ? "music pack" : "synthesizer");
        }
        if (ImGui::TreeNode("The pieces")) {
            ui_hint("A recording replaces a piece when its file name starts with the number (up to four for one number, "
                 "played in turn: \"27 a.ogg\", \"27 b.ogg\" ...). The pieces that loop in the game "
                 "loop in the pack (the whole file again), the others play once. While a piece of the pack is on, the "
                 "synthesizer is silent; a piece the pack lacks is played by the synthesizer.");
            for (int k = 8; k <= 47; k++) {
                const char *f = musicpack_file(k), *d = musicpack_desc(k);
                const char *b = f + strlen(f); while (b > f && b[-1] != '/' && b[-1] != '\\') b--;
                ui_hint("%2d  %s%s%s", k, d, d[0] ? ", " : "", musicpack_loops(k) ? "loops" : "plays once");
                if (f[0]) { ImGui::SameLine(); int nv = musicpack_versions(k); if (nv > 1) ImGui::Text("  %s and %d more, in turn", b, nv - 1); else ImGui::Text("  %s", b); }
            }
            ui_hint("The pieces without a name are heard elsewhere in towns and on the road; the line above names each piece as it starts.");
            if (ImGui::SmallButton("Look again##music")) musicpack_reload();
            ImGui::TreePop();
        }
    }
    ImGui::TextWrapped("The GOG release plays everything through its Roland driver, which on a General MIDI synthesizer "
                       "sounds like \"random notes rather than clashing swords\": it is meant for an MT-32. With the MT-32's "
                       "ROM files Darklands+ plays it as composed; without them, the Sound Blaster setting uses the game's "
                       "own FM drivers and digitized effects, as most players heard it in 1992.");
}

