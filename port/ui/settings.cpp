// settings.cpp - the Darklands+ window's settings pages (F12): Mods, Helpers and Display. Each
// switch is a config key (runtime/config.h); a change marks the settings to be written when the
// window closes (ui_dirty_config). Split from ui.cpp, which holds the window and its other pages.
#include <SDL.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <string>
#include <vector>
#include "imgui.h"
#include "ornament.h"
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
#include "../mods/options.h"
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
    ImGui::PushFont(ui_font_head); orn_septext("Mod folders"); ImGui::PopFont();
    {
        static char msg[1400];
        char md[1100]; datamods_folder(md, sizeof md);
        extern char dlp_user_dir[];
        ImGui::TextWrapped("A mod is a folder in %s" PATH_SEP "mods with files laid out like the game folder: "
                           "while the mod is ticked, the game reads the mod's files in place of the originals (the game folder is never changed).", dlp_user_dir);
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
                if (ImGui::SliderInt("How often new encounters come up##encshare", &share, 0, 300, "%d%%")) { config_set_int("encounters.share", share); ui_dirty_config = 1; }
                ui_hint("At 100%%, each new encounter is as likely as any one of the original encounters for that terrain (or more, if its file gives it a weight).");
                bool seal = config_get_int("encounters.seal", 1) != 0;
                if (ImGui::Checkbox("Mark new encounters with a seal in the corner of the card", &seal)) { config_set_int("encounters.seal", seal); ui_dirty_config = 1; }
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
          if (ImGui::Checkbox("Write every text shown to a file (for a translation: mods" PATH_SEP "_export" PATH_SEP "texts_seen.txt)", &ct)) ui_collect_texts = ct; }
        ui_hint("A mod's text.txt replaces the texts built into the game's menus, one per line, e.g. Purchase an item => Kaufen (needs crisp text).");
        if (msg[0]) ImGui::TextWrapped("%s", msg);
        ui_hint("Mods are applied in alphabetical order of their folders: where two change the same file, the later one wins. "
             "Single files inside MSGFILES and the .CAT catalogues can be replaced by a folder of that name holding just those files. "
             "Card texts and pictures change at once; the item list, saints, cities and the like change the next time the game starts.");
    }
    ImGui::Spacing();
    ui_hint("The built-in changes to the rules (swift travel, half-price inns, quick learning ...) are under Helpers > Easier rules.");
}

/* the presets (mods/options.h): "the original game" is Buggy Purist; the player's own choices are
   kept when a preset replaces them (Custom) */
int ui_original_mode(void) { return config_get_int("original", 0); }
bool ui_modding(void) { return config_get_int("editor.show", 0) && !ui_original_mode(); }
void ui_set_original(bool on)
{
    if (on) preset_apply(PRESET_BUGGY_PURIST);
    else if (config_get_int("custom.saved", 0)) preset_restore_custom();
    else preset_apply(PRESET_PLUS);
    ui_dirty_config = 1;
}
extern "C" void ui_set_original_c(int on)          /* (the test command uoriginal: 0 off, 1 on; 10 + n preset n) */
{
    if (on >= 10) preset_apply(on - 10); else ui_set_original(on != 0);
    fprintf(stderr, "original %d preset %d: fixes", ui_original_mode(), preset_current());
    for (int i = 0; i < FIX_COUNT; i++) fprintf(stderr, " %d", FIXES[i].on);
    fprintf(stderr, " mods");
    for (int i = 0; i < MOD_COUNT; i++) fprintf(stderr, " %d", MODS[i].enabled);
    fprintf(stderr, "\n");
}

/* a switch's words as one paragraph: the short title in bold, a colon, and what it does in grey,
   wrapped together at the page's width; true when the words are clicked (they switch it too) */
bool ui_title_desc(const char *title, const char *desc, bool dim, const char *id)
{
    ImDrawList *dl = ImGui::GetWindowDrawList();
    ImFont *fr = ImGui::GetFont(), *fb = ui_font_bold ? ui_font_bold : fr;
    float fs = ImGui::GetFontSize(), w = ImGui::GetContentRegionAvail().x, ls = ImGui::GetTextLineHeight() + UIS(1);   /* (as TextWrapped's lines) */
    ImVec2 p0 = ImGui::GetCursorScreenPos();
    ImU32 ink = ImGui::GetColorU32(dim ? ImGuiCol_TextDisabled : ImGuiCol_Text);
    ImU32 grey = ImGui::GetColorU32(ImGuiCol_TextDisabled, dim ? 0.75f : 1.0f);
    float x = 0, y = 0;
    auto words = [&](const char *s, ImFont *f, ImU32 col) {
        float sp = f->CalcTextSizeA(fs, FLT_MAX, 0, " ").x;
        while (*s) {
            while (*s == ' ') s++;
            const char *e = s; while (*e && *e != ' ') e++;
            if (e == s) break;
            float wd = f->CalcTextSizeA(fs, FLT_MAX, 0, s, e).x;
            if (x > 0 && x + wd > w) { x = 0; y += ls; }
            dl->AddText(f, fs, ImVec2(p0.x + x, p0.y + y), col, s, e);
            x += wd + sp; s = e;
        }
    };
    std::string t = std::string(title) + (desc && desc[0] ? ":" : "");
    words(t.c_str(), fb, ink);
    if (desc) words(desc, fr, grey);
    ImGui::SetCursorScreenPos(p0);
    bool clicked = ImGui::InvisibleButton(id, ImVec2(w > 1 ? w : 1, y + ImGui::GetTextLineHeight()));
    if (ImGui::IsItemHovered()) ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
    return clicked;
}

static void opt_set(const Option *o, int v) { option_set(o, v); ui_dirty_config = 1; }
static void option_row(const Option *o, int depth);
static void option_children(const Option *o, int depth)
{
    int n = options_count();
    for (int i = 0; i < n; i++) { const Option *c = options_get(i); if (c->parent && !strcmp(c->parent, o->key)) option_row(c, depth); }
}
/* a switch: the box, its words; a choice: its words, then the choices in a row (or a list); then
   what it means for saved games, then the switches that belong to it while it is on */
static void option_row(const Option *o, int depth)
{
    ImGui::PushID(o->key);
    float ind = depth * (ImGui::GetFrameHeight() + ImGui::GetStyle().ItemInnerSpacing.x);
    if (ind > 0) ImGui::Indent(ind);
    int v = option_value(o);
    if (!o->choices) {
        bool on = v != 0;
        if (ImGui::Checkbox("##o", &on)) opt_set(o, on);
        ImGui::SameLine();
        if (ui_title_desc(o->title, o->desc)) opt_set(o, option_value(o) ? 0 : 1);
    } else {
        ImGui::Dummy(ImVec2(ImGui::GetFrameHeight(), ImGui::GetFrameHeight())); ImGui::SameLine();
        ui_title_desc(o->title, o->desc);
        ImGui::Indent(ImGui::GetFrameHeight() + ImGui::GetStyle().ItemInnerSpacing.x);
        int nc = option_nchoices(o), sel = -1;
        for (int k = 0; k < nc; k++) if (o->vals[k] == v) sel = k;
        char lab[96];
        if (o->flags & OF_COMBO) {
            float wmax = 0; for (int k = 0; k < nc; k++) wmax = fmaxf(wmax, ImGui::CalcTextSize(option_choice(o, k, lab, sizeof lab)).x);
            ImGui::SetNextItemWidth(wmax + ImGui::GetFrameHeight() + UIS(24));
            if (ImGui::BeginCombo("##c", sel >= 0 ? option_choice(o, sel, lab, sizeof lab) : "")) {
                for (int k = 0; k < nc; k++) if (ImGui::Selectable(option_choice(o, k, lab, sizeof lab), k == sel)) opt_set(o, o->vals[k]);
                ImGui::EndCombo();
            }
        } else {
            float right = ImGui::GetContentRegionMax().x;
            for (int k = 0; k < nc; k++) {
                option_choice(o, k, lab, sizeof lab);
                float bw = ImGui::GetFrameHeight() + ImGui::GetStyle().ItemInnerSpacing.x + ImGui::CalcTextSize(lab).x;
                if (k) { ImGui::SameLine(); if (ImGui::GetCursorPosX() + bw > right) ImGui::NewLine(); }
                ImGui::PushID(k);
                if (ImGui::RadioButton(lab, k == sel)) opt_set(o, o->vals[k]);
                ImGui::PopID();
            }
        }
        ImGui::Unindent(ImGui::GetFrameHeight() + ImGui::GetStyle().ItemInnerSpacing.x);
    }
    float ti = ImGui::GetFrameHeight() + ImGui::GetStyle().ItemInnerSpacing.x;
    if (o->note) {
        ImGui::Indent(ti);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.62f, 0.32f, 0.08f, 1));
        ImGui::TextWrapped("%s", o->note);
        ImGui::PopStyleColor();
        ImGui::Unindent(ti);
    }
    if (!strcmp(o->key, "input.gamepad")) {
        ImGui::Indent(ti); ui_hint(plat_controller_connected() ? "A controller is connected." : "No controller connected now.");
        /* the button prompts (ui/padglyphs.h): the controller's buttons on screen while it is in use */
        static const char *PM[5] = { "Match the device in use", "Keys and mouse always", "Xbox always", "PlayStation always", "Nintendo always" };
        int pm = config_get_int("input.prompts", 0); if (pm < 0 || pm > 4) pm = 0;
        ImGui::SetNextItemWidth(UIS(260));
        if (ImGui::Combo("Button prompts", &pm, PM, 5)) { config_set_int("input.prompts", pm); ui_dirty_config = 1; }
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("While a controller is in use, the hints and on-screen buttons show its buttons (Xbox, PlayStation or Nintendo, matching the controller). The \"always\" choices are for a Steam Deck played with its trackpads, which count as a mouse.");
        ImGui::Unindent(ti);
    }
    if (ind > 0) ImGui::Unindent(ind);
    if (option_value(o)) option_children(o, depth + 1);
    ImGui::Spacing();
    ImGui::PopID();
}

/* the presets, as switches are: a name in bold, what it is in grey; the one in force marked, or
   "Custom" with the nearest and how many switches differ */
static void presets_block(void)
{
    int cur = preset_current();
    for (int p = 0; p < PRESET_COUNT; p++) {
        ImGui::PushID(p);
        if (ImGui::RadioButton("##p", cur == p)) { preset_apply(p); ui_dirty_config = 1; cur = preset_current(); }
        ImGui::SameLine();
        std::string nm = std::string(PRESETS[p].name) + (p == PRESET_PLUS ? " (recommended)" : "");
        if (ui_title_desc(nm.c_str(), PRESETS[p].desc)) { preset_apply(p); ui_dirty_config = 1; cur = preset_current(); }
        ImGui::PopID();
    }
    ImGui::PushID("custom");
    bool kept = config_get_int("custom.saved", 0) != 0;
    if (cur == PRESET_CUSTOM) {
        int d = 0, near = preset_nearest(&d), shown = 0;
        std::string t = std::string("your own choices: ") + PRESETS[near].name + ", but ";
        int n = options_count();
        for (int i = 0; i < n; i++) {                   /* (the switches that differ, by name: the first few) */
            const Option *o = options_get(i); int pv = preset_value(o, near), v = option_value(o);
            if (pv == PRESET_KEEP || v == pv) continue;
            if (shown < 3) {
                char lab[96]; std::string val = o->choices ? option_choice(o, 0, lab, sizeof lab) : (v ? "on" : "off");
                if (o->choices) for (int k = 0; k < option_nchoices(o); k++) if (o->vals[k] == v) val = option_choice(o, k, lab, sizeof lab);
                t += std::string(shown ? ", " : "") + o->title + " " + val;
            }
            shown++;
        }
        if (config_get_int("original", 0) && near != PRESET_BUGGY_PURIST) { t += shown ? ", " : ""; t += "the original game"; shown++; }
        if (shown > 3) { char b[48]; snprintf(b, sizeof b, " and %d more", shown - 3); t += b; }
        t += ".";
        ImGui::RadioButton("##p", true); ImGui::SameLine(); ui_title_desc("Custom", t.c_str());
    } else if (kept) {
        bool c = false;
        if (ImGui::RadioButton("##p", false)) c = true;
        ImGui::SameLine();
        if (ui_title_desc("Custom", "your own choices from before you picked a preset, kept: click to restore them.") || c) { preset_restore_custom(); ui_dirty_config = 1; }
    }
    ImGui::PopID();
    ImGui::Spacing();
    ImGui::TextColored(ImVec4(0.18f, 0.40f, 0.16f, 1), "Saved games stay compatible with the original game both ways, whatever the preset.");
    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
    ImGui::TextWrapped("Darklands+ saves in the original game's format: its saves load in the original (on DOS too), and the original's saves load here; "
                       "what Darklands+ records on top (Sabbats learned, items marked Keep, sprites and colours) goes into separate files of its own. Switching presets "
                       "or single switches at any time is safe.");
    ImGui::PopStyleColor();
    ImGui::TextColored(ImVec4(0.62f, 0.32f, 0.08f, 1), "One exception:");
    ImGui::SameLine();
    ImGui::TextWrapped("a save with a member using an enemy sprite (Darklands+ and Easy offer them) loads only in Darklands+.");
}

void tab_helpers(void)
{
    /* the sections, from the switches' table: bookmarks at the left (a click opens the section and
       brings it to the top), each section folding away under its heading; the bookmark of the
       section at the top of the view is marked */
    std::vector<const char *> sect; sect.push_back("Preset");
    bool orig = ui_original_mode();
    if (!orig) {
        int n = options_count();
        for (int i = 0; i < n; i++) {
            const Option *o = options_get(i);
            if ((o->flags & OF_ELSEWHERE) || !strcmp(o->section, "Restored cut content")) continue;   /* (that one: its own page, tab_restored) */
            bool seen = false; for (const char *s : sect) if (!strcmp(s, o->section)) seen = true;
            if (!seen) sect.push_back(o->section);
        }
    }
    const int NSECT = (int)sect.size();
    static int jump = -1, at_top = 0;
    static bool fold_all = false;
    ImGui::BeginChild("##hnav", ImVec2(UIS(210), 0), ImGuiChildFlags_Borders);
    ImGui::TextDisabled("Sections");
    for (int i = 0; i < NSECT; i++) {
        const char *s = sect[i]; char lab[64];
        snprintf(lab, sizeof lab, "%s", !strcmp(s, "Fixes for bugs of the original") ? "Fixes" : s);
        if (ImGui::Selectable(lab, i == at_top)) jump = i;
    }
    ImGui::Spacing();
    if (NSECT > 1 && ImGui::SmallButton("Fold all")) fold_all = true;
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("##hbody", ImVec2(0, 0));
    int sec_i = 0, top_found = -1; float view_top = ImGui::GetWindowPos().y + UIS(30);   /* (a heading brought to the top counts as in view) */
    auto section = [&](const char *name, bool foldable) -> bool {
        int i = sec_i++;
        ImGui::PushID(name);
        if (jump == i) ImGui::SetNextItemOpen(true);
        else if (fold_all && foldable) ImGui::SetNextItemOpen(false);
        ImGui::PushFont(ui_font_head);
        bool open = foldable ? ImGui::CollapsingHeader(name, ImGuiTreeNodeFlags_DefaultOpen) : (orn_septext(name), true);
        ImGui::PopFont();
        if (jump == i) { ImGui::SetScrollHereY(0.0f); jump = -1; }
        if (ImGui::GetItemRectMin().y <= view_top) top_found = i;       /* (the last heading above the top: the section in view) */
        ImGui::PopID();
        return open;
    };
    section("Preset", false);
    presets_block();
    if (orig) {
        ImGui::Spacing();
        ui_hint("Everything Darklands+ adds is off, and its switches are hidden: pick another preset to change single switches.");
    } else {
        int n = options_count();
        for (int k = 1; k < NSECT; k++) {
            ImGui::Spacing();
            if (!section(sect[k], true)) continue;
            if (!strcmp(sect[k], "Easier rules")) ui_hint("Rule changes that make the game easier; the Easy preset turns on the ones marked for it. Some take effect only when the next new game starts (their note says so).");
            for (int i = 0; i < n; i++) {
                const Option *o = options_get(i);
                if ((o->flags & OF_ELSEWHERE) || o->parent || strcmp(o->section, sect[k])) continue;
                option_row(o, 0);
            }
        }
    }
    at_top = top_found < 0 ? 0 : top_found;
    fold_all = false;
    ImGui::EndChild();
}

/* what MicroProse wrote, coded or left unfinished and never switched on, put back by Darklands+:
   a page of its own, experimental, every switch off by default and in every preset */
void tab_restored(void)
{
    ImGui::PushFont(ui_font_head); orn_septext("Restored cut content"); ImGui::PopFont();
    ImGui::TextColored(ImVec4(0.75f, 0.35f, 0.1f, 1), "Experimental.");
    ImGui::SameLine();
    ImGui::TextWrapped("What MicroProse wrote, coded or left unfinished and never switched on, put back by Darklands+. Nobody played it in 1992: "
                       "expect rough edges. Every switch here is off unless you tick it, and every preset turns them off again.");
    ui_hint("What each one was in the game's files, and what Darklands+ had to add or put right, is in docs/cut_content.md (and docs/cut_politics.md).");
    ImGui::Spacing();
    if (ui_original_mode()) { ui_hint("Off while playing the original game (Buggy Purist): pick another preset to tick these."); return; }
    int n = options_count();
    for (int i = 0; i < n; i++) {
        const Option *o = options_get(i);
        if ((o->flags & OF_ELSEWHERE) || o->parent || strcmp(o->section, "Restored cut content")) continue;
        option_row(o, 0);
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
        ui_hint("Rounds off the stair-steps of diagonal edges in the pictures and sprites without blurring them "
             ""
             "Scale4x needs a graphics card: without one (a virtual machine, say) the game slows down.");
    }
    { extern int ui_margins_on; bool mg = ui_margins_on != 0;
      if (ImGui::Checkbox("On wide screens, show the date, place, purse, tasks, town and effects in the side margins (in battles: the log and everyone's condition)", &mg)) { ui_margins_on = mg; config_set_int("display.margins", mg); ui_dirty_config = 1; } }
    {   /* grows with the picture; this scales it */
        static const int P[] = { 70, 80, 90, 100, 115, 130, 150, 175, 200 };
        int cur = config_get_int("display.text_size", 100), sel = 3;
        for (int i = 0; i < 9; i++) if (P[i] == cur) sel = i;
        char lab[16]; snprintf(lab, sizeof lab, "%d%%", P[sel]);
        ImGui::SetNextItemWidth(UIS(160));
        if (ImGui::BeginCombo("text size in the margins, tooltips and on the map", lab)) {
            for (int i = 0; i < 9; i++) { char t[16]; snprintf(t, sizeof t, "%d%%", P[i]); if (ImGui::Selectable(t, i == sel)) { config_set_int("display.text_size", P[i]); ui_dirty_config = 1; } }
            ImGui::EndCombo();
        }
    }
    { bool sc = ui_sys_cursor != 0;
      if (ImGui::Checkbox("Show the system mouse pointer over the game too", &sc)) { ui_sys_cursor = sc; config_set_int("display.system_cursor", sc); ui_dirty_config = 1; } }
    {   /* the menus' size (fonts are baked at start-up) */
        static const int P[] = { 0, 100, 125, 150, 175, 200, 250 };
        static const char *N[] = { "automatic (the system's scale)", "100%", "125%", "150%", "175%", "200%", "250%" };
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
        static const char *N[] = { "nothing: straight to the start screen", "the last scene: the lady of the Darklands and her welcome", "the whole opening (a click skips to the lady's welcome)" };
        int m = opening_mode();
        ImGui::SetNextItemWidth(UIS(420));
        if (ImGui::Combo("at start-up", &m, N, 3)) { opening_set_mode(m); ui_dirty_config = 1; }
        ImGui::SameLine(); ImGui::TextDisabled(m == 2 ? "(a click skips to the welcome, another to the start screen)" : "(a key or a click ends it)");
    }
    bool fs = (SDL_GetWindowFlags(ui_window) & SDL_WINDOW_FULLSCREEN_DESKTOP) != 0;
    if (ImGui::Checkbox("Full screen (Alt+Enter)", &fs)) SDL_SetWindowFullscreen(ui_window, fs ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
    ImGui::PushFont(ui_font_head); orn_septext("Text"); ImGui::PopFont();
    int hm = hdtext_mode;
    if (ImGui::RadioButton("Crisp text on story cards only", hm == 1)) { hdtext_set(1); ui_dirty_config = 1; }
    if (ImGui::RadioButton("Crisp text everywhere (cards, party, sheets, menus, shops)", hm == 2)) { hdtext_set(2); ui_dirty_config = 1; }
    if (ImGui::RadioButton("The original lettering", hm == 0)) { hdtext_set(0); ui_dirty_config = 1; }
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
        { bool fn = config_get_int("display.full_names", 1) != 0, aw = config_get_int("display.armour_worn", 1) != 0;
          if (ImGui::Checkbox("Names in full where the game cut them short (Studded Leather, Illuminated Gospel, St. Gregory Thaumaturgus, Raimundus Lullus' Sunburst)", &fn)) {
              config_set_int("display.full_names", fn); gamedata_names(); ui_dirty_config = 1; }
          ImGui::Indent(); if (!fn) ImGui::BeginDisabled();
          if (ImGui::Checkbox("Armour by where it is worn: Chainmail (body) and Chainmail (legs), instead of V:Chainmail and L:Chainmail", &aw)) {
              config_set_int("display.armour_worn", aw); gamedata_names(); ui_dirty_config = 1; }
          if (!fn) ImGui::EndDisabled(); ImGui::Unindent(); }
        { bool co = money_coins();
          if (ImGui::Checkbox("Money with coins: 1" COIN_FL " 12" COIN_GR " 4" COIN_PF " (florins, groschen, pfennigs) instead of 1fl 12gr 4pf; in a description, a price also shows its total in pfennigs", &co)) { config_set_int("display.coins", co); ui_dirty_config = 1; } }
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

    ImGui::PushFont(ui_font_head); orn_septext("Pictures"); ImGui::PopFont();
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
        ui_hint("More crests: art%sSHIELDP.png, SHIELDQ.png ... up to SHIELDZ.png add crests after the game's fifteen "
             "(Heraldry, when making a party). A save with a character using one of these crests loads only in Darklands+.", PATH_SEP);
    }

    ImGui::PushFont(ui_font_head); orn_septext("Ornaments"); ImGui::PopFont();
    {   /* the band round the windows (ui/ornament.cpp): a picture of each to choose from */
        int cur = orn_style();
        float cw = fminf((ImGui::GetContentRegionAvail().x - 4 * ImGui::GetStyle().ItemSpacing.x) / 5, UIS(170)), ch = cw * 0.78f + ImGui::GetTextLineHeightWithSpacing() * 1.3f;
        static const int ORDER[5] = { 1, 2, 3, 4, 0 };
        for (int j = 0; j < 5; j++) {
            int st = ORDER[j];
            if (j) ImGui::SameLine();
            ImGui::PushID(100 + st);
            ImVec2 p = ImGui::GetCursorScreenPos();
            if (ImGui::InvisibleButton("orn", ImVec2(cw, ch))) { config_set_int("display.ornate", st); ui_dirty_config = 1; cur = st; }
            bool hv = ImGui::IsItemHovered();
            ImDrawList *dl = ImGui::GetWindowDrawList();
            dl->AddRectFilled(p, ImVec2(p.x + cw, p.y + ch), st == cur ? IM_COL32(140, 61, 33, 255) : hv ? IM_COL32(200, 170, 120, 255) : IM_COL32(60, 44, 30, 255), 6);
            float m = cw * 0.08f;
            extern ImFont *ui_font_overlay;
            orn_preview(dl, ImVec2(p.x + m, p.y + m), ImVec2(p.x + cw - m, p.y + cw * 0.78f - m * 0.4f), st, ui_font_overlay);
            const char *n = orn_style_name(st);
            ImVec2 ts = ImGui::CalcTextSize(n);
            dl->AddText(ImVec2(p.x + (cw - ts.x) * 0.5f, p.y + ch - ImGui::GetTextLineHeightWithSpacing() * 1.1f), IM_COL32(246, 236, 214, 255), n);
            ImGui::PopID();
        }
        {   /* the frames round tooltips and panels, and the buttons (with any band) */
            static const char *FR[4] = { "a double gilt rule", "rubricated (a red rule, as a manuscript)", "pen flourishes at the corners", "blind tooling (as a leather binding)" };
            static const char *BT[4] = { "metal plaques", "rubrics (flat, a fine gold line)", "seal tabs (rounded)", "lettered (the word, underlined)" };
            int fr = config_get_int("display.frame_style", 0) & 3, bt = config_get_int("display.button_style", 0) & 3;
            ImGui::AlignTextToFramePadding(); ImGui::TextUnformatted("Frames:"); ImGui::SameLine(); ImGui::SetNextItemWidth(UIS(330));
            if (ImGui::BeginCombo("##ornfr", FR[fr])) { for (int k = 0; k < 4; k++) if (ImGui::Selectable(FR[k], fr == k)) { config_set_int("display.frame_style", k); ui_dirty_config = 1; } ImGui::EndCombo(); }
            ImGui::SameLine(); ImGui::TextUnformatted("Buttons:"); ImGui::SameLine(); ImGui::SetNextItemWidth(UIS(300));
            if (ImGui::BeginCombo("##ornbt", BT[bt])) { for (int k = 0; k < 4; k++) if (ImGui::Selectable(BT[k], bt == k)) { config_set_int("display.button_style", k); ui_dirty_config = 1; } ImGui::EndCombo(); }
            static const char *NC[4] = { "acanthus leaves", "tracery (a quatrefoil)", "a shield in a roundel", "a carved rose" };
            int nc = config_get_int("display.niche_corners", 0) & 3;
            ImGui::AlignTextToFramePadding(); ImGui::TextUnformatted("The character sheet's arch, its corners:"); ImGui::SameLine(); ImGui::SetNextItemWidth(UIS(260));
            if (ImGui::BeginCombo("##ornnc", NC[nc])) { for (int k = 0; k < 4; k++) if (ImGui::Selectable(NC[k], nc == k)) { config_set_int("display.niche_corners", k); ui_dirty_config = 1; } ImGui::EndCombo(); }
        }
        ui_hint("The band round the windows, after the original's own: the cards' plaited copper, the party sheet's walnut and brass, "
                "the illuminated capitals' vermilion and gold, the paper doll's carved stone. With any of them, rules and corners are gilt "
                "and the frames and buttons are as chosen below the pictures.");
    }

    ImGui::PushFont(ui_font_head); orn_septext("Clock"); ImGui::PopFont();
    {   /* the clock shown while hours pass: a picture of each to choose from (ui/hourglass.cpp) */
        void ui_draw_clock_style(int style, ImDrawList *dl, ImVec2 tl, float kx, float k, int hour, int day, int month);
        const char *ui_clock_name(int k);
        int cur = config_get_int("display.clock", 1);
        float cw = fminf(ImGui::GetContentRegionAvail().x / 3 - ImGui::GetStyle().ItemSpacing.x, UIS(240)), k = cw / 140.0f, ch = 120 * k + ImGui::GetTextLineHeightWithSpacing() * 1.3f;
        static const int ORDER[6] = { 1, 3, 4, 5, 2, 0 };
        for (int j = 0; j < 6; j++) {
            int st = ORDER[j];
            if (j % 3) ImGui::SameLine();
            ImGui::PushID(st);
            ImVec2 p = ImGui::GetCursorScreenPos();
            if (ImGui::InvisibleButton("clk", ImVec2(cw, ch))) { config_set_int("display.clock", st); ui_dirty_config = 1; cur = st; }
            bool hv = ImGui::IsItemHovered();
            ImDrawList *dl = ImGui::GetWindowDrawList();
            dl->AddRectFilled(p, ImVec2(p.x + cw, p.y + ch), st == cur ? IM_COL32(140, 61, 33, 255) : hv ? IM_COL32(200, 170, 120, 255) : IM_COL32(60, 44, 30, 255), 6);
            dl->PushClipRect(p, ImVec2(p.x + cw, p.y + ch), true);
            ui_draw_clock_style(st, dl, ImVec2(p.x + 10 * k, p.y + 6 * k), k, k, 9, 10, 11);
            dl->PopClipRect();
            const char *n = ui_clock_name(st);
            ImVec2 ts = ImGui::CalcTextSize(n);
            dl->AddText(ImVec2(p.x + (cw - ts.x) * 0.5f, p.y + ch - ImGui::GetTextLineHeightWithSpacing() * 1.1f), IM_COL32(246, 236, 214, 255), n);
            ImGui::PopID();
        }
        ui_hint("Shown while hours pass (at the inn, waiting, travelling on a card). The tower dial follows the great clocks of about 1400 (Wells, Prague); "
                "the game's own clock is an armillary sphere, here also drawn anew at the window's resolution.");
    }

    ImGui::PushFont(ui_font_head); orn_septext("Sound"); ImGui::PopFont();
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
            ImGui::TextUnformatted("Both ROM sets found; play on the:");
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
        if (n) ui_hint("%d sound%s in %s%ssounds; the original sounds for the rest.", n, n == 1 ? "" : "s", dlp_user_dir, PATH_SEP);
        else ui_hint("No sound pack in use: put one into %s%ssounds, or tick a sound mod in F12 > Settings > Mods (\"Battle sounds\" comes with Darklands+).", dlp_user_dir, PATH_SEP);
        if (sp) {
            ImGui::Indent(UIS(30));
            if (ImGui::Checkbox("Blows that land make a sound too (the original is silent there)", &hs)) { sfx_set_hits(hs); ui_dirty_config = 1; }
            if (ImGui::Checkbox("Experimental: synthesized sounds where the pack has none", &sy)) { sfx_set_synth(sy); ui_dirty_config = 1; }
            int v = sfx_volume;
            ImGui::SetNextItemWidth(UIS(260));
            if (ImGui::SliderInt("Volume of the pack", &v, 0, 150, "%d%%")) { sfx_set_volume(v); ui_dirty_config = 1; }
            static char msg[400];
            if (ImGui::Button("Look again##sfx")) { sfx_reload(); snprintf(msg, sizeof msg, "%d sound%s found.", sfx_user_count(), sfx_user_count() == 1 ? "" : "s"); }
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
        else ui_hint("No music pack found. A music pack is a mod's music folder, or %s%smusic, with a recording for each piece named by its number "
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

