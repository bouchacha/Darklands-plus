// settings.cpp - the Darklands+ window's Settings page (F12), and its Mods and Reconstructed Content
// tabs. Each switch is a config key (runtime/config.h); a change marks the settings to be written
// when the window closes (ui_dirty_config). The sections on what things look like are in
// ui/settings_look.cpp, those on the computer's setup in ui/settings_setup.cpp (ui/settings_parts.h).
#include <SDL.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <string>
#include <vector>
#include <functional>
#include "imgui.h"
#include "padglyphs.h"
#include "../mods/fixgroups.h"
#include "skillicons.h"
#include "gamepics.h"
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
void soundpreview_start(const char *dev, int what); void soundpreview_stop(void); int soundpreview_playing(const char *dev, int what); int soundpreview_possible(void);
void musicpack_preview(int n); int musicpack_first_piece(void); void sfx_preview(int k); u32 plat_ticks_ms(void);
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
#include "settings_parts.h"

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
static void replacement_art(void)
{
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
        ui_hint("For artists: a file art%sNAME.png (any size; transparency where the picture has color 0) replaces the "
             "game's picture NAME wherever it is shown, at the window's resolution. Export writes every original picture "
             "to art%soriginal as a template, in its own colors. Whatever the game draws over a picture (text, the "
             "mouse pointer, other pictures) stays on top.", PATH_SEP, PATH_SEP);
        ui_hint("More crests: art%sSHIELDP.png, SHIELDQ.png ... up to SHIELDZ.png add crests after the game's fifteen "
             "(Heraldry, when making a party). A save with a member using one of these crests loads only in Darklands+.", PATH_SEP);
    }

}
static void option_row(const Option *o, int depth);
int ui_original_mode(void);
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
        else if (datamods_n > 1) ui_hint("Where two mods change the same file, the one lower in the list wins. Drag a mod by its handle (or use its arrows) to reorder.");
        int move_from = -1, move_to = -1;
        for (int i = 0; i < datamods_n; i++) {
            ImGui::PushID(1000 + i);
            ImGui::BeginGroup();
            if (datamods_n > 1) {                                /* the handle: dragged to another mod's place */
                float h = ImGui::GetFrameHeight();
                ImVec2 a = ImGui::GetCursorScreenPos();
                ImGui::InvisibleButton("##grip", ImVec2(h * 0.7f, h));
                bool hv = ImGui::IsItemHovered() || ImGui::IsItemActive();
                if (hv) ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
                ImDrawList *dl = ImGui::GetWindowDrawList();
                for (int k = 0; k < 3; k++) {
                    float y = a.y + h * (0.32f + 0.18f * k);
                    dl->AddLine(ImVec2(a.x + h * 0.12f, y), ImVec2(a.x + h * 0.58f, y), ImGui::GetColorU32(hv ? ImGuiCol_Text : ImGuiCol_TextDisabled), fmaxf(1.5f, h * 0.07f));
                }
                if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceNoPreviewTooltip)) {
                    ImGui::SetDragDropPayload("DL_MOD", &i, sizeof i);
                    ImGui::EndDragDropSource();
                }
                ImGui::SameLine();
                ImGui::BeginDisabled(i == 0);
                if (ImGui::ArrowButton("##up", ImGuiDir_Up)) { move_from = i; move_to = i - 1; }
                ImGui::EndDisabled();
                ImGui::SameLine();
                ImGui::BeginDisabled(i == datamods_n - 1);
                if (ImGui::ArrowButton("##down", ImGuiDir_Down)) { move_from = i; move_to = i + 1; }
                ImGui::EndDisabled();
                ImGui::SameLine();
            }
            bool on = datamods[i].on != 0;
            if (ImGui::Checkbox(datamods[i].title, &on)) { datamods_set(i, on); ui_dirty_config = 1; }
            ImGui::Indent(datamods_n > 1 ? ImGui::GetFrameHeight() * 2.7f + ImGui::GetStyle().ItemSpacing.x * 3 + UIS(34) : UIS(34));
            if (datamods[i].desc[0]) ui_hint("%s", datamods[i].desc);
            { std::string fp = datamods[i].path; for (char &c : fp) if (c == '/' || c == '\\') c = PATH_SEP[0]; ui_hint("folder: %s", fp.c_str()); }
            ImGui::Unindent(datamods_n > 1 ? ImGui::GetFrameHeight() * 2.7f + ImGui::GetStyle().ItemSpacing.x * 3 + UIS(34) : UIS(34));
            ImGui::EndGroup();
            if (ImGui::BeginDragDropTarget()) {                  /* (dropped on a mod: takes its place) */
                ImVec2 a = ImGui::GetItemRectMin(), b = ImGui::GetItemRectMax();
                if (const ImGuiPayload *pl = ImGui::AcceptDragDropPayload("DL_MOD", ImGuiDragDropFlags_AcceptNoDrawDefaultRect)) { move_from = *(const int *)pl->Data; move_to = i; }
                else if (const ImGuiPayload *pk = ImGui::GetDragDropPayload()) {
                    int from = *(const int *)pk->Data;              /* a gilt line where it will go */
                    float y = from < i ? b.y + 2 : a.y - 2;
                    ImGui::GetWindowDrawList()->AddLine(ImVec2(a.x, y), ImVec2(b.x, y), IM_COL32(220, 180, 90, 255), 3.0f);
                }
                ImGui::EndDragDropTarget();
            }
            ImGui::PopID();
        }
        if (move_from >= 0) { datamods_move(move_from, move_to); ui_dirty_config = 1; }
        {   /* mods with ENCOUNTERS folders (mods/encounters.c) */
            int ne = encounters_count(), nerr = encounters_errors();
            if (ne || nerr) {
                ImGui::Spacing();
                ImGui::TextWrapped("New encounters from mods: %d%s", ne, nerr ? "" : ".");
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
        ui_hint("A mod's text.txt replaces the texts built into the game's menus, one per line, e.g. Purchase an item => Kaufen (needs New text: F12 > Settings > Text).");
        if (msg[0]) ImGui::TextWrapped("%s", msg);
        ui_hint("Mods are applied in the order of the list above: where two change the same file, the lower one wins. "
             "Single files inside MSGFILES and the .CAT catalogs can be replaced by a folder of that name holding just those files. "
             "Card texts and pictures change at once; the item list, saints, cities and the like change the next time the game starts.");
    }
    ImGui::Spacing();
    ImGui::PushFont(ui_font_head); orn_septext("Replacement art (for artists)"); ImGui::PopFont();
    replacement_art();
    /* the built-in mods: easier play, no preset's (each its own switch, as a mod folder is) */
    ImGui::Spacing();
    ImGui::PushFont(ui_font_head); orn_septext("Built-in mods: easier play"); ImGui::PopFont();
    if (ui_original_mode()) { ui_hint("Off while playing the original game (Full Original): pick another preset to tick these."); return; }
    ui_hint("For the story more than the struggle. Each is its own switch; no preset turns them on, and Full Original and Purist turn them off. "
            "Some take effect only when the next new game starts (their note says so).");
    int n = options_count();
    for (int i = 0; i < n; i++) { const Option *o = options_get(i); if (o->cls == OC_EASY) option_row(o, 0); }
}

/* the presets (mods/options.h): "the original game" is Full Original; the player's own choices are
   kept when a preset replaces them (Custom) */
int ui_original_mode(void) { return config_get_int("original", 0); }
bool ui_modding(void) { return !ui_original_mode(); }      /* (the Editor: always but in the original game; editor.show retired) */
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

int ui_option_pics(const char *key, const char **caps);                           /* ui/optpreview.cpp */
void ui_option_pic(const char *key, int i, ImDrawList *dl, ImVec2 a, ImVec2 b);
static const char *pcaps[4];
extern "C" bool screenpics_draw(ImDrawList *dl, ImVec2 a, ImVec2 b);   /* ui/screenpics.cpp */
extern "C" bool screenpics_scaling(ImDrawList *dl, ImVec2 a, ImVec2 b, int mode);
extern "C" bool screenpics_map_draw(ImDrawList *dl, ImVec2 a, ImVec2 b);
extern "C" bool screenpics_land_draw(ImDrawList *dl, ImVec2 a, ImVec2 b);
extern "C" void saves_prefs(void);   /* ui/saves.cpp */
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
    if (!strcmp(o->key, "display.native_screens")) {     /* pictures of the new screens, from this game (ui/screenpics.cpp) */
        ImGui::Indent(ti);
        float pw = fminf(UIS(480), ImGui::GetContentRegionAvail().x), ph = pw * 9 / 16;
        ImVec2 a = ImGui::GetCursorScreenPos();
        if (screenpics_draw(ImGui::GetWindowDrawList(), a, ImVec2(a.x + pw, a.y + ph))) ImGui::Dummy(ImVec2(pw, ph + UIS(4)));
        else ui_hint("Pictures of the new screens appear here as you meet them in your game: a card, a shop, the character sheet, the inn, alchemy.");
        ImGui::Unindent(ti);
    } else if (!strcmp(o->key, "map.key_m")) {          /* the world map as the player last saw it (ui/screenpics.cpp) */
        ImGui::Indent(ti);
        float pw = fminf(UIS(420), ImGui::GetContentRegionAvail().x), ph = pw * 9 / 16;
        ImVec2 a = ImGui::GetCursorScreenPos();
        if (screenpics_map_draw(ImGui::GetWindowDrawList(), a, ImVec2(a.x + pw, a.y + ph))) ImGui::Dummy(ImVec2(pw, ph + UIS(4)));
        else ui_hint("A picture of the world map appears here once you have opened it (M) in your game.");
        ImGui::Unindent(ti);
    } else if (!strcmp(o->key, "map.names")) {          /* the overland view as the player last saw it, names and quest marks on it */
        ImGui::Indent(ti);
        float pw = fminf(UIS(420), ImGui::GetContentRegionAvail().x), ph = pw * 9 / 16;
        ImVec2 a = ImGui::GetCursorScreenPos();
        if (screenpics_land_draw(ImGui::GetWindowDrawList(), a, ImVec2(a.x + pw, a.y + ph))) ImGui::Dummy(ImVec2(pw, ph + UIS(4)));
        else ui_hint("A picture of the overland view, with the names and quest markers on it, appears here once the party has been out in the land.");
        ImGui::Unindent(ti);
    } else if (int np = ui_option_pics(o->key, pcaps)) {   /* what it turns on, in pictures */
        ImGui::Indent(ti);
        float gap = UIS(10), pw = fminf(UIS(np == 1 ? 260 : 170), (ImGui::GetContentRegionAvail().x - gap * (np - 1)) / np), ph = pw * 0.6f;
        for (int k = 0; k < np; k++) {
            if (k) ImGui::SameLine(0, gap);
            ImVec2 a = ImGui::GetCursorScreenPos();
            ImGui::Dummy(ImVec2(pw, ph + ImGui::GetTextLineHeightWithSpacing()));
            ImDrawList *dl = ImGui::GetWindowDrawList();
            ui_option_pic(o->key, k, dl, a, ImVec2(a.x + pw, a.y + ph));
            dl->AddRect(ImVec2(a.x - 1, a.y - 1), ImVec2(a.x + pw + 1, a.y + ph + 1), IM_COL32(120, 100, 80, 160), 3);
            ImVec2 ts = ImGui::CalcTextSize(pcaps[k]);
            float sc = ts.x > pw ? pw / ts.x : 1.0f;
            dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() * sc, ImVec2(a.x + (pw - ts.x * sc) * 0.5f, a.y + ph + 2), ImGui::GetColorU32(ImGuiCol_Text), pcaps[k]);
        }
        ImGui::Unindent(ti);
    }
    if (o->note) {
        ImGui::Indent(ti);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.62f, 0.32f, 0.08f, 1));
        ImGui::TextWrapped("%s", o->note);
        ImGui::PopStyleColor();
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
                       "what Darklands+ records on top (what the party has learned of the witches' Sabbats, items marked Keep, sprites and colors) goes into separate files of its own. Switching presets "
                       "or single switches at any time is safe.");
    ImGui::PopStyleColor();
    ImGui::TextColored(ImVec4(0.62f, 0.32f, 0.08f, 1), "One exception:");
    ImGui::SameLine();
    ImGui::TextWrapped("a save with a member using an enemy sprite or an added crest (Darklands+ offers both) loads only in Darklands+.");
}

/* the frames, the buttons, the clock and the potions' grade marks: one section */
static void look_all(void)
{
    ImGui::TextUnformatted("The band around the windows");
    look_ornaments();
    ImGui::Spacing(); ImGui::TextUnformatted("The clock while hours pass");
    look_clock();
    ImGui::Spacing();
    display_icons();
}
static const char *const LOOKS[] = { "Text", "Frames and clock" };
static void (*const LOOK_FN[])(void) = { look_text, look_all };
static int look_index(const char *s) { for (int i = 0; i < 2; i++) if (!strcmp(s, LOOKS[i])) return i; return -1; }
void ui_section_icon(ImDrawList *dl, ImVec2 c, float s, const char *name);   /* ui/skillicons.cpp */
static char h_jump_to[64];
extern "C" void settings_helpers_jump(const char *name) { snprintf(h_jump_to, sizeof h_jump_to, "%s", name); }   /* (test: uhsect) */
void tab_helpers(void)
{
    /* the sections, from the switches' table: bookmarks at the left (a click opens the section and
       brings it to the top), each section folding away under its heading; the bookmark of the
       section at the top of the view is marked */
    std::vector<const char *> sect; sect.push_back("Preset");
    bool orig = ui_original_mode();
    static const char *const SETUP[] = { "Screen", "Intro", "Sound", "Controller" };   /* (the computer's side: shown in every preset) */
    static void (*const SETUP_FN[])(void) = { setup_screen, setup_intro, setup_sound, controller_prefs };
    auto setup_index = [](const char *s) { for (int i = 0; i < 4; i++) if (!strcmp(s, SETUP[i])) return i; return -1; };
    if (!orig) {
        int n = options_count();
        for (int i = 0; i < n; i++) {
            const Option *o = options_get(i);
            if ((o->flags & OF_ELSEWHERE) || option_bundled(o) || o->cls == OC_EASY || !strcmp(o->section, "Reconstructed Content")) continue;   /* (those: their own pages, tab_restored, tab_mods) */
            bool seen = false; for (const char *s : sect) if (!strcmp(s, o->section)) seen = true;
            if (!seen) sect.push_back(o->section);
            if (!seen && !strcmp(o->section, "New screens"))      /* what things look like, after the screens */
                for (const char *l : LOOKS) sect.push_back(l);
        }
    }
    for (const char *l : SETUP) sect.push_back(l);
    const int NSECT = (int)sect.size();
    static int jump = -1, at_top = 0;
    if (h_jump_to[0]) { for (int i = 0; i < NSECT; i++) if (!strcmp(sect[i], h_jump_to)) jump = i; h_jump_to[0] = 0; }
    static bool fold_all = false;
    ImGui::BeginChild("##hnav", ImVec2(UIS(230), 0), ImGuiChildFlags_Borders);
    float ih = ImGui::GetTextLineHeight();
    for (int i = 0; i < NSECT; i++) {
        const char *s = sect[i]; char lab[80];
        snprintf(lab, sizeof lab, "%s###nav%d", !strcmp(s, "Fixes for bugs of the original") ? "Fixes" : s, i);
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ih * 1.5f);           /* (room for the section's picture) */
        if (ImGui::Selectable(lab, i == at_top)) jump = i;
        ImVec2 a = ImGui::GetItemRectMin(), b = ImGui::GetItemRectMax();
        ui_section_icon(ImGui::GetWindowDrawList(), ImVec2(a.x - ih * 0.8f, (a.y + b.y) * 0.5f), ih * 1.1f, s);
    }
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
    }
    {
        int n = options_count();
        for (int k = 1; k < NSECT; k++) {
            ImGui::Spacing();
            if (!section(sect[k], true)) continue;
            {   int l = look_index(sect[k]);
                if (l >= 0) { LOOK_FN[l](); continue; } }
            {   int l = setup_index(sect[k]);
                if (l >= 0) { SETUP_FN[l](); continue; } }
            bool fixes = !strcmp(sect[k], "Fixes for bugs of the original");
            if (fixes) {                                  /* (a long list, each its own switch since what counts as a bug is open to argument: folded) */
                int nf = 0, on = 0;
                for (int i = 0; i < n; i++) { const Option *o = options_get(i); if (o->cls == OC_FIX && !strcmp(o->section, sect[k])) { nf++; on += option_value(o) != 0; } }
                ui_hint("The original game's bugs put right (%d of %d on). Each is its own switch, since what counts as a bug (and how to mend it) is open to argument; "
                        "they are sorted from the plainly broken to the arguable.", on, nf);
                for (int g = 0; g < FIX_NGROUPS; g++) {     /* (fixgroups.c) */
                    int gn = 0, gon = 0;
                    for (int i = 0; i < n; i++) { const Option *o = options_get(i); if (o->cls == OC_FIX && !strcmp(o->section, sect[k]) && fix_group(o->key + 4) == g) { gn++; gon += option_value(o) != 0; } }
                    if (!gn) continue;
                    char lab[128]; snprintf(lab, sizeof lab, "%s (%d of %d on)###fixg%d", FIX_GROUPS[g].name, gon, gn, g);
                    if (!ImGui::TreeNodeEx(lab, ImGuiTreeNodeFlags_SpanAvailWidth)) continue;
                    ui_hint("%s", FIX_GROUPS[g].what);
                    for (int i = 0; i < n; i++) {
                        const Option *o = options_get(i);
                        if (o->cls != OC_FIX || strcmp(o->section, sect[k]) || o->parent || fix_group(o->key + 4) != g) continue;
                        option_row(o, 0);
                    }
                    ImGui::TreePop();
                }
                continue;
            }
            for (int i = 0; i < n; i++) {
                const Option *o = options_get(i);
                if ((o->flags & OF_ELSEWHERE) || option_bundled(o) || o->parent || strcmp(o->section, sect[k])) continue;
                option_row(o, 0);
            }
        }
    }
    at_top = top_found < 0 ? 0 : top_found;
    fold_all = false;
    ImGui::EndChild();
}

/* what MicroProse wrote, coded or left unfinished and never switched on, reconstructed by
   Darklands+: a page of its own, experimental, every switch off by default and in every preset;
   in two parts, by how much Darklands+ had to supply (docs/cut_content.md) */
void tab_restored(void)
{
    ImGui::PushFont(ui_font_head); orn_septext("Reconstructed Content"); ImGui::PopFont();
    ImGui::TextColored(ImVec4(0.75f, 0.35f, 0.1f, 1), "Experimental.");
    ImGui::SameLine();
    ImGui::TextWrapped("What MicroProse wrote or coded for Darklands and never switched on, put back by Darklands+. Nobody played it in 1992: "
                       "expect rough edges. Every switch here is off unless you tick it, and every preset turns them off again.");
    ui_hint("What each one was in the game's files, and what Darklands+ had to add or put right, is in docs/cut_content.md (and docs/cut_politics.md).");
    ImGui::Spacing();
    if (ui_original_mode()) { ui_hint("Off while playing the original game (Full Original): pick another preset to tick these."); return; }
    static const char *const FINISHED[] = { "rules.politics", "rules.war", "battle.black_cloud", 0 };   /* (the rest: rules.fehde, rules.hospice, assist.hall_of_fame) */
    auto finished = [](const char *k) { for (int i = 0; FINISHED[i]; i++) if (!strcmp(FINISHED[i], k)) return true; return false; };
    int n = options_count();
    for (int part = 0; part < 2; part++) {
        ImGui::PushFont(ui_font_head); orn_septext(part ? "Written, but never finished" : "Finished, but never switched on"); ImGui::PopFont();
        ui_hint(part ? "The texts or pictures are in the game's files, but the game never shows them: Darklands+ supplies the missing pieces."
                     : "Written and coded by MicroProse, only never turned on (or, for Black Cloud, left out of a menu): Darklands+ turns it on.");
        for (int i = 0; i < n; i++) {
            const Option *o = options_get(i);
            if ((o->flags & OF_ELSEWHERE) || option_bundled(o) || o->parent || strcmp(o->section, "Reconstructed Content") || finished(o->key) != (part == 0)) continue;
            option_row(o, 0);
        }
        ImGui::Spacing();
    }
}

