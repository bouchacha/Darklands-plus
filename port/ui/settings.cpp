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

static void display_icons(void); static void look_text(void); static void look_ornaments(void); static void look_clock(void);
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
static void setup_screen(void); static void setup_intro(void); static void setup_sound(void); static void controller_prefs(void);
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

extern "C" { int mpu_out_count(void); const char *mpu_out_name(int i); int mt32_families(void); int mt32_switch_pending(void); int mt32_switch(void); const char *mt32_status(void); void mt32_rescan(void); int mt32_dir_count(void); const char *mt32_dir(int); extern int sound_music_volume; int mt32_available(void); void ui_show_mt32_help(void); }
/* a row of pictures to choose from (show, don't describe): n cards, each drawn by draw(dl, a, b,
   value) in its box, its label under it; a click sets the config key */
static void pic_choice(const char *id, const char *key, int def, const int *vals, const char *const *labels, int n, float cw, float ph,
                       const std::function<void(ImDrawList *, ImVec2, ImVec2, int)> &draw)
{
    int cur = config_get_int(key, def);
    float ch = ph + ImGui::GetTextLineHeightWithSpacing() * 1.3f;
    float avail = ImGui::GetContentRegionAvail().x;
    ImGui::PushID(id);
    float x = 0;
    for (int j = 0; j < n; j++) {
        if (j) { if (x + cw > avail) x = 0; else ImGui::SameLine(); }
        x += cw + ImGui::GetStyle().ItemSpacing.x;
        ImGui::PushID(j);
        ImVec2 p = ImGui::GetCursorScreenPos();
        if (ImGui::InvisibleButton("c", ImVec2(cw, ch))) { config_set_int(key, vals[j]); ui_dirty_config = 1; cur = vals[j]; }
        bool hv = ImGui::IsItemHovered();
        ImDrawList *dl = ImGui::GetWindowDrawList();
        dl->AddRectFilled(p, ImVec2(p.x + cw, p.y + ch), vals[j] == cur ? IM_COL32(140, 61, 33, 255) : hv ? IM_COL32(200, 170, 120, 255) : IM_COL32(60, 44, 30, 255), 6);
        float m = cw * 0.06f;
        dl->PushClipRect(ImVec2(p.x + m * 0.5f, p.y + m * 0.5f), ImVec2(p.x + cw - m * 0.5f, p.y + ph), true);
        draw(dl, ImVec2(p.x + m, p.y + m), ImVec2(p.x + cw - m, p.y + ph - m * 0.4f), vals[j]);
        dl->PopClipRect();
        ImVec2 ts = ImGui::CalcTextSize(labels[j]);
        float sc = ts.x > cw - 8 ? (cw - 8) / ts.x : 1.0f;
        dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() * sc, ImVec2(p.x + (cw - ts.x * sc) * 0.5f, p.y + ch - ImGui::GetTextLineHeightWithSpacing() * 1.1f), IM_COL32(246, 236, 214, 255), labels[j]);
        ImGui::PopID();
    }
    ImGui::PopID();
}

extern "C" void settings_helpers_jump(const char *name);
extern "C" void settings_display_jump(int k) { static const char *N[4] = { "Screen", "Intro", "Sound", "Controller" }; if (k >= 0 && k < 4) settings_helpers_jump(N[k]); }   /* (test: udsect) */
/* a game controller: always on (but in the original game's menus it is a mouse too); which one is
   connected, its buttons drawn as any of the three kinds, and the jobs of the main ones to change */
extern "C" int plat_controller_kind(const char **name);
extern "C" int plat_pad_bind(int k);
extern "C" void plat_pad_bind_set(int k, int a);
int settings_pack_preview;                 /* (a pack previewed: ui/ui.cpp lets the music through) */
static void controller_prefs(void)
{
    const char *nm; int kind = plat_controller_kind(&nm);
    static const char *KIND[4] = { "", "Xbox", "PlayStation", "Nintendo" };
    if (kind) ImGui::Text("Connected: %s (%s style)", nm, KIND[kind]);
    else ImGui::TextUnformatted("No controller connected now: plug one in at any time. The buttons below can be set up beforehand.");
    static int view = 0;
    if (!view) view = kind ? kind : 1;
    ImGui::TextUnformatted("Show the buttons as:"); 
    for (int k = 1; k <= 3; k++) { ImGui::SameLine(); if (ImGui::RadioButton(KIND[k], view == k)) view = k; }
    static const char *JOB[9] = { "Click", "Right-click (the menus)", "Enter", "Esc", "Tab (the level map)", "Space (pause)", "F12 (the Darklands+ window)", "M (the world map)", "F11 (keys and clicks)" };
    int keep = ui_pad_style_force; ui_pad_style_force = view;
    float gh = ImGui::GetFrameHeight();
    if (ImGui::BeginTable("##padmap", 2, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("b", ImGuiTableColumnFlags_WidthFixed, UIS(70));
        ImGui::TableSetupColumn("j", ImGuiTableColumnFlags_WidthFixed, UIS(440));
        auto glyph = [&](int k, bool phys) {
            ImVec2 p = ImGui::GetCursorScreenPos();
            float w = ui_pad_glyph_width(gh * 0.9f, k);
            ImGui::Dummy(ImVec2(UIS(60), gh));
            if (phys) ui_pad_button_glyph(ImGui::GetWindowDrawList(), ImVec2(p.x + w * 0.5f + 4, p.y + gh * 0.5f), gh * 0.9f, k);
            else ui_pad_glyph(ImGui::GetWindowDrawList(), ImVec2(p.x + w * 0.5f + 4, p.y + gh * 0.5f), gh * 0.9f, k);
        };
        for (int k = PB_A; k <= PB_LS; k++) {
            ImGui::TableNextRow(); ImGui::TableNextColumn(); glyph(k, true);
            ImGui::TableNextColumn();
            int a = plat_pad_bind(k);
            ImGui::PushID(k); ImGui::SetNextItemWidth(UIS(290));
            if (ImGui::Combo("##job", &a, JOB, 9)) { plat_pad_bind_set(k, a); ui_dirty_config = 1; }
            ImGui::PopID();
        }
        struct F { int b; const char *what; } fixed[4] = { { PB_LSTICK, "the pointer (the left trigger held: slowly)" }, { PB_RSTICK, "scrolls lists; pressed: Z (fast-forward)" },
                                                            { PB_DPAD, "the arrow keys" }, { PB_RT, "held: Ctrl (Ctrl+click in the overland view travels there)" } };
        for (const F &f : fixed) { ImGui::TableNextRow(); ImGui::TableNextColumn(); glyph(f.b, true); ImGui::TableNextColumn(); ImGui::AlignTextToFramePadding(); ImGui::TextUnformatted(f.what); }
        ImGui::EndTable();
    }
    ui_pad_style_force = keep;
    if (ImGui::Button("Back to the usual buttons")) { for (int k = PB_A; k <= PB_LS; k++) plat_pad_bind_set(k, k); ui_dirty_config = 1; }
    /* the button prompts (ui/padglyphs.h): the controller's buttons on screen while it is in use */
    static const char *PM[5] = { "Match the device in use", "Keys and mouse always", "Xbox always", "PlayStation always", "Nintendo always" };
    int pm = config_get_int("input.prompts", 0); if (pm < 0 || pm > 4) pm = 0;
    ImGui::SetNextItemWidth(UIS(260));
    if (ImGui::Combo("Buttons shown in hints", &pm, PM, 5)) { config_set_int("input.prompts", pm); ui_dirty_config = 1; }
    ui_hint("While a controller is in use, the hints and on-screen buttons show its buttons. The \"always\" choices are for a Steam Deck played with its trackpads, which count as a mouse.");
}
static void display_icons(void)
{
    float cw = fminf((ImGui::GetContentRegionAvail().x - 3 * ImGui::GetStyle().ItemSpacing.x) / 4, UIS(170));
    ImGui::TextUnformatted("A potion's grade, on its flask");
    static const int PV[4] = { 4, 3, 1, 0 };
    static const char *PL[4] = { "Chevrons on a badge", "Chevrons", "Dots on a strip", "Small dots" };
    pic_choice("pgrade", "display.potion_grade", 4, PV, PL, 4, cw, cw * 0.62f, [](ImDrawList *dl, ImVec2 a, ImVec2 b, int v) {
        extern int potion_grade_preview; void ui_item_icon_tier(ImDrawList *dl, ImVec2 c, float s, int id, int tier);
        dl->AddRectFilled(a, b, IM_COL32(244, 234, 210, 255), 4);
        potion_grade_preview = v;
        float s = (b.y - a.y) * 0.62f;
        for (int k = 0; k < 3; k++) ui_item_icon_tier(dl, ImVec2(a.x + (b.x - a.x) * (0.2f + k * 0.3f), (a.y + b.y) * 0.5f), s, k == 0 ? 99 : k == 1 ? 95 : 104, k + 1);
        potion_grade_preview = -1;
    });
}

/* what things look like (shown among the switches, tab_helpers) */
/* the text: the game's own lettering, or the new text with all that goes with it (outlined letters,
   abbreviations spelled out, armor named by where it is worn, faint text darkened, icons beside the
   skills, the default typeface); only the money's look is a choice of its own */
extern "C" int hdtext_lettering(int font, const char *s, unsigned char *out, int w, int h);
extern "C" int hdtext_font_height(int font);
static void text_look_set(bool fresh)
{
    extern int ui_skill_icons, ui_text_contrast;
    hdtext_set(fresh ? 2 : 0);
    config_set_int("display.overlay_font_follow", 1); ui_overlay_font_update();
    ui_spell_out = fresh; config_set_int("display.spell_out", fresh);
    config_set_int("display.full_names", fresh); config_set_int("display.armour_worn", fresh); gamedata_names();
    ui_text_edge = fresh ? 2 : 0; config_set_int("display.text_edge", ui_text_edge);
    ui_skill_icons = fresh; config_set_int("display.skill_icons", fresh);
    ui_text_contrast = fresh; config_set_int("display.text_contrast", fresh);
    config_set_int("display.skill_icon_look", 2);
    ui_dirty_config = 1;
}
static void look_text(void)
{
    static const char *SAMPLE = "The gate of the town stands open.";
    float cw = fminf((ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) / 2, UIS(360));
    static const char *L[2] = { "The original lettering", "New text" };
    int cur = hdtext_mode ? 1 : 0;
    float ph = cw * 0.36f, ch = ph + ImGui::GetTextLineHeightWithSpacing() * 1.3f;
    for (int j = 0; j < 2; j++) {
        if (j) ImGui::SameLine();
        ImGui::PushID(300 + j);
        ImVec2 p = ImGui::GetCursorScreenPos();
        if (ImGui::InvisibleButton("tl", ImVec2(cw, ch)) && cur != j) { text_look_set(j != 0); cur = j; }
        bool hv = ImGui::IsItemHovered();
        ImDrawList *dl = ImGui::GetWindowDrawList();
        dl->AddRectFilled(p, ImVec2(p.x + cw, p.y + ch), j == cur ? IM_COL32(140, 61, 33, 255) : hv ? IM_COL32(200, 170, 120, 255) : IM_COL32(60, 44, 30, 255), 6);
        float m = cw * 0.04f;
        ImVec2 a(p.x + m, p.y + m), b(p.x + cw - m, p.y + ph - m * 0.4f);
        dl->AddRectFilled(a, b, IM_COL32(232, 214, 176, 255), 3);           /* (a card's parchment) */
        ImU32 ink = IM_COL32(52, 30, 14, 255);
        if (!j) {   /* the game's own 7-pixel letters (FONTS.FNT, the cards' font), as they look scaled up: each pixel a square */
            enum { PW = 220, PH = 12 };
            static unsigned char px[PW * PH]; static int pw = -1, fh;
            if (pw < 0) { fh = hdtext_font_height(3); if (fh > PH) fh = PH; pw = fh ? hdtext_lettering(3, SAMPLE, px, PW, PH) : 0; }
            if (pw > 0) {
                float k = fminf((b.x - a.x) * 0.92f / pw, (b.y - a.y) * 0.5f / fh);
                ImVec2 o(a.x + ((b.x - a.x) - pw * k) * 0.5f, a.y + ((b.y - a.y) - fh * k) * 0.5f);
                for (int y = 0; y < fh; y++) for (int x = 0; x < pw; x++)
                    if (px[y * PW + x]) dl->AddRectFilled(ImVec2(o.x + x * k, o.y + y * k), ImVec2(o.x + (x + 1) * k, o.y + (y + 1) * k), ink);
            } else dl->AddText(ImVec2(a.x + 8, a.y + 8), ink, "(the game's fonts were not found)");
        } else {    /* the same words in the new text */
            ImFont *f = ui_font_game[ui_game_font] ? ui_font_game[ui_game_font] : ImGui::GetFont();
            float fs = (b.y - a.y) * 0.36f;
            ImVec2 ts = f->CalcTextSizeA(fs, FLT_MAX, 0, SAMPLE);
            if (ts.x > (b.x - a.x) * 0.92f) { fs *= (b.x - a.x) * 0.92f / ts.x; ts = f->CalcTextSizeA(fs, FLT_MAX, 0, SAMPLE); }
            dl->AddText(f, fs, ImVec2(a.x + ((b.x - a.x) - ts.x) * 0.5f, a.y + ((b.y - a.y) - ts.y) * 0.5f), ink, SAMPLE);
        }
        ImVec2 ts = ImGui::CalcTextSize(L[j]);
        dl->AddText(ImVec2(p.x + (cw - ts.x) * 0.5f, p.y + ch - ImGui::GetTextLineHeightWithSpacing() * 1.1f), IM_COL32(246, 236, 214, 255), L[j]);
        ImGui::PopID();
    }
    if (cur) {
        ui_hint("New text: the same words drawn sharp at any window size, outlined over pictures and darkened where faint; "
                "the character sheet's abbreviations spelled out (wPol: Polearm Wpns), names in full where the game cut them short, "
                "armor named by where it is worn (Chainmail (body) for V:Chainmail), and icons beside the skills.");
        ImGui::SetNextItemWidth(UIS(380));
        if (ImGui::BeginCombo("Typeface", UI_GAME_FONT_NAMES[ui_game_font])) {      /* (the picture above follows) */
            for (int i = 0; i < UI_GAME_FONTS; i++) {
                ImGui::PushFont(ui_font_game[i]);
                if (ImGui::Selectable(UI_GAME_FONT_NAMES[i], i == ui_game_font)) { ui_game_font = i; config_set_int("display.text_font", i); ui_overlay_font_update(); ui_dirty_config = 1; }
                ImGui::PopFont();
            }
            ImGui::EndCombo();
        }
        bool co = money_coins();
        if (ImGui::Checkbox("Money with coins: 1" COIN_FL " 12" COIN_GR " 4" COIN_PF " instead of 1fl 12gr 4pf (florins, groschen, pfennigs)", &co)) { config_set_int("display.coins", co); ui_dirty_config = 1; }
    } else
        ui_hint("The game's own small blackletter, as in 1992: true to the original, but hard to read on a large screen.");
}
static void look_ornaments(void)
{
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
            float cw = fminf((ImGui::GetContentRegionAvail().x - 3 * ImGui::GetStyle().ItemSpacing.x) / 4, UIS(170));
            static const int V4[4] = { 0, 1, 2, 3 };
            static const char *FR[4] = { "Gold lines", "Red lines", "Pen flourishes", "Stamped leather" };
            static const char *BT[4] = { "Metal plaques", "Flat, gold-edged", "Rounded tabs", "Words only" };
            ImGui::TextUnformatted("Frames (tooltips, the card text, the new screens' pages)");
            pic_choice("ornfr", "display.frame_style", 0, V4, FR, 4, cw, cw * 0.62f, [](ImDrawList *dl, ImVec2 a, ImVec2 b, int v) {
                dl->AddRectFilled(a, b, IM_COL32(237, 222, 189, 255));
                orn_parchment_edges(dl, a, b, (b.y - a.y) * 0.12f);
                orn_frame_style_box(dl, a, b, (b.x - a.x) * 0.09f, v);
                dl->AddText(ImVec2(a.x + (b.x - a.x) * 0.2f, a.y + (b.y - a.y) * 0.35f), IM_COL32(60, 40, 20, 255), "St. Adrian");
            });
            ImGui::TextUnformatted("Buttons");
            pic_choice("ornbt", "display.button_style", 0, V4, BT, 4, cw, cw * 0.62f, [](ImDrawList *dl, ImVec2 a, ImVec2 b, int v) {
                dl->AddRectFilled(a, b, IM_COL32(242, 230, 200, 255), 4);
                static const ImU32 C[2] = { IM_COL32(112, 40, 22, 235), IM_COL32(40, 92, 34, 235) };
                static const char *L[2] = { "Leave", "Buy" };
                float h = (b.y - a.y) * 0.3f;
                for (int k = 0; k < 2; k++) {
                    ImVec2 p0(a.x + (b.x - a.x) * 0.15f, a.y + (b.y - a.y) * (0.14f + k * 0.46f)), p1(b.x - (b.x - a.x) * 0.15f, p0.y + h);
                    orn_button_style(dl, p0, p1, C[k], k == 1, true, v);
                    ImU32 tc = v == 3 ? (C[k] | IM_COL32(0, 0, 0, 255)) : IM_COL32(250, 240, 220, 255);
                    ImVec2 ts = ImGui::CalcTextSize(L[k]);
                    dl->AddText(ImVec2((p0.x + p1.x - ts.x) * 0.5f, (p0.y + p1.y - ts.y) * 0.5f), tc, L[k]);
                }
            });
        }
        ui_hint("The band around the windows, after the original's own: the cards' plaited copper, the party sheet's walnut and brass, "
                "the illuminated capitals' vermilion and gold, the stone niche around the figure on the character sheet. With any of them, the lines and corners are gilt "
                "and the frames and buttons are as chosen here.");
    }

}
static void look_clock(void)
{
    {   /* the clock shown while hours pass: a picture of each to choose from (ui/hourglass.cpp) */
        void ui_draw_clock_style(int style, ImDrawList *dl, ImVec2 tl, float kx, float k, int hour, int day, int month);
        const char *ui_clock_name(int k);
        int cur = config_get_int("display.clock", 1);
        float cw = fminf(ImGui::GetContentRegionAvail().x / 3 - ImGui::GetStyle().ItemSpacing.x, UIS(240)), k = cw / 140.0f, ch = 120 * k + ImGui::GetTextLineHeightWithSpacing() * 1.3f;
        static const int ORDER[3] = { 1, 3, 0 };
        if (cur == 2 || cur == 4) cur = 1;                          /* (dropped clocks: the tower dial) */
        if (cur == 5) cur = 3;                                      /* (and the copper astrolabe) */
        for (int j = 0; j < 3; j++) {
            int st = ORDER[j];
            if (j % 3) ImGui::SameLine();
            ImGui::PushID(st);
            ImVec2 p = ImGui::GetCursorScreenPos();
            if (ImGui::InvisibleButton("clk", ImVec2(cw, ch))) { config_set_int("display.clock", st); ui_dirty_config = 1; cur = st; }
            bool hv = ImGui::IsItemHovered();
            ImDrawList *dl = ImGui::GetWindowDrawList();
            dl->AddRectFilled(p, ImVec2(p.x + cw, p.y + ch), st == cur ? IM_COL32(140, 61, 33, 255) : hv ? IM_COL32(200, 170, 120, 255) : IM_COL32(60, 44, 30, 255), 6);
            dl->PushClipRect(p, ImVec2(p.x + cw, p.y + ch), true);
            ui_draw_clock_style(st, dl, ImVec2(p.x + 10 * k, p.y + 6 * k), k, k, (int)(ImGui::GetTime() * 1.5) % 24, 10, 11);   /* (moving: a day in sixteen seconds) */
            dl->PopClipRect();
            const char *n = ui_clock_name(st);
            ImVec2 ts = ImGui::CalcTextSize(n);
            dl->AddText(ImVec2(p.x + (cw - ts.x) * 0.5f, p.y + ch - ImGui::GetTextLineHeightWithSpacing() * 1.1f), IM_COL32(246, 236, 214, 255), n);
            ImGui::PopID();
        }
        ui_hint("Shown while hours pass (at the inn, waiting, traveling on a card). The tower dial follows the great clocks of about 1400 (Wells, Prague); "
                "the copper astrolabe is the game's own clock, drawn anew so that it stays sharp.");
    }

}

/* the setup of the game on this computer: four sections of the Settings page (tab_helpers) */
static void setup_screen(void)
{
    {   /* the game's picture scaled up: three ways, each shown on a piece of the screen now (ui/screenpics.cpp);
           (the picture is always 4:3, as on a 1992 monitor) */
        int cur = ui_settings.scaler ? 1 : ui_settings.smooth ? 2 : 0;
        static const char *L[3] = { "Sharp pixels", "Rounded edges", "Soft" };
        float cw = fminf((ImGui::GetContentRegionAvail().x - 2 * ImGui::GetStyle().ItemSpacing.x) / 3, UIS(240)), ph = cw * 0.625f, ch = ph + ImGui::GetTextLineHeightWithSpacing() * 1.3f;
        for (int j = 0; j < 3; j++) {
            if (j) ImGui::SameLine();
            ImGui::PushID(400 + j);
            ImVec2 p = ImGui::GetCursorScreenPos();
            if (ImGui::InvisibleButton("sc", ImVec2(cw, ch))) {
                ui_settings.scaler = j == 1 ? 2 : 0; ui_settings.smooth = j != 0; ui_settings.integer = 0;
                config_set_int("display.scaler", ui_settings.scaler); config_set_int("display.smooth", ui_settings.smooth); config_set_int("display.integer_scale", 0);
                ui_dirty_config = 1; cur = j;
            }
            bool hv = ImGui::IsItemHovered();
            ImDrawList *dl = ImGui::GetWindowDrawList();
            dl->AddRectFilled(p, ImVec2(p.x + cw, p.y + ch), j == cur ? IM_COL32(140, 61, 33, 255) : hv ? IM_COL32(200, 170, 120, 255) : IM_COL32(60, 44, 30, 255), 6);
            float m = cw * 0.04f;
            if (!screenpics_scaling(dl, ImVec2(p.x + m, p.y + m), ImVec2(p.x + cw - m, p.y + ph - m * 0.4f), j))
                dl->AddText(ImVec2(p.x + 10, p.y + 10), IM_COL32(246, 236, 214, 255), "(no picture yet)");
            ImVec2 ts = ImGui::CalcTextSize(L[j]);
            dl->AddText(ImVec2(p.x + (cw - ts.x) * 0.5f, p.y + ch - ImGui::GetTextLineHeightWithSpacing() * 1.1f), IM_COL32(246, 236, 214, 255), L[j]);
            ImGui::PopID();
        }
        ui_hint(cur == 0 ? "Every pixel a crisp square, as the game drew it."
              : cur == 1 ? "The stair-steps of diagonal edges rounded off, without blurring. On a computer without a proper graphics card, the game slows down with this."
                         : "Blurred a little, as an old monitor would.");
    }
    { extern int ui_margins_on; bool mg = ui_margins_on != 0;
      if (ImGui::Checkbox("On wide screens, show the date, place, purse, quests, town and effects in the side margins (in battles: the log and everyone's condition)", &mg)) { ui_margins_on = mg; config_set_int("display.margins", mg); ui_dirty_config = 1; } }
    {   /* grows with the picture; this scales it */
        static const int P[] = { 70, 80, 90, 100, 115, 130, 150, 175, 200 };
        int cur = config_get_int("display.text_size", 100), sel = 3;
        for (int i = 0; i < 9; i++) if (P[i] == cur) sel = i;
        char lab[16]; snprintf(lab, sizeof lab, "%d%%", P[sel]);
        ImGui::SetNextItemWidth(UIS(160));
        if (ImGui::BeginCombo("text size in the margins, tooltips and place names", lab)) {
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
    { bool pu = config_get_int("display.pause_unfocused", 1) != 0;
      if (ImGui::Checkbox("Pause the game while its window is in the background", &pu)) { config_set_int("display.pause_unfocused", pu); ui_dirty_config = 1; } }
    bool fs = (SDL_GetWindowFlags(ui_window) & SDL_WINDOW_FULLSCREEN_DESKTOP) != 0;
    if (ImGui::Checkbox("Full screen (Alt+Enter)", &fs)) SDL_SetWindowFullscreen(ui_window, fs ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
}
static void setup_intro(void)
{
    {   /* the opening at start (mods/opening.c): the first start plays it whole */
        static const char *N[] = { "None: straight to the start screen", "Only the last scene: the lady of the Darklands and her welcome", "The whole intro" };
        int m = opening_mode();
        for (int i = 0; i < 3; i++) if (ImGui::RadioButton(N[i], m == i)) { opening_set_mode(i); ui_dirty_config = 1; }
        ui_hint("Shown each time the game starts (the very first start plays it whole). A key or a click skips ahead.");
    }
}
static void setup_sound(void)
{
    /* two previews, at the top right of the section: what the sound chosen below sounds like now (the
       second tune of the overland view, or a few battle sounds), through the device chosen, or the
       custom pack when it is ticked (mods/soundpreview.c, mods/musicpack.c, mods/sfx.c) */
    const char *cur = config_get("sound.device", "auto");
    bool can_hear = soundpreview_possible() != 0;
    static u32 pack_until; static bool pack_on;
    static u32 seq_t; static int seq_k = -1;
    static const int SEQ[] = { 1, 2, 0, 3, 5, 4 };   /* parry, hit, miss, knock-back, bow, death */
    if (pack_on && plat_ticks_ms() > pack_until) { musicpack_preview(-1); pack_on = false; }
    if (seq_k >= 0 && plat_ticks_ms() >= seq_t) { sfx_preview(SEQ[seq_k]); seq_t = plat_ticks_ms() + 650; if (++seq_k >= 6) seq_k = -1; }
    settings_pack_preview = pack_on || seq_k >= 0;
    {
        const char *dev = !strcmp(cur, "auto") || !strcmp(cur, "game") ? (mt32_available() ? "mt32" : "fm") : cur;
        bool off = !strcmp(cur, "off");
        bool pack_music = musicpack_on && musicpack_versions(20) > 0, pack_sfx = sfx_mode != 0;
        float bw = ImGui::CalcTextSize("Preview: Battle Sounds").x + ImGui::GetStyle().FramePadding.x * 2;
        float bm = ImGui::CalcTextSize("Preview: Music").x + ImGui::GetStyle().FramePadding.x * 2;
        float x = ImGui::GetContentRegionMax().x - bw - bm - ImGui::GetStyle().ItemSpacing.x;
        ImGui::SetCursorPosX(x);
        ImGui::BeginDisabled(off || (!can_hear && !pack_music));
        bool mon = pack_music ? pack_on : soundpreview_playing(dev, 0) != 0;
        if (ImGui::Button(mon ? "Stop##pm" : "Preview: Music", ImVec2(bm, 0))) {
            if (mon) { if (pack_on) { musicpack_preview(-1); pack_on = false; } else soundpreview_stop(); }
            else if (pack_music) { musicpack_preview(20); pack_on = true; pack_until = plat_ticks_ms() + 15000; }
            else soundpreview_start(dev, 0);
        }
        ImGui::EndDisabled();
        ImGui::SameLine();
        ImGui::BeginDisabled(off || (!can_hear && !pack_sfx));
        bool bon = pack_sfx ? seq_k >= 0 : soundpreview_playing(dev, 1) != 0;
        if (ImGui::Button(bon ? "Stop##pb" : "Preview: Battle Sounds", ImVec2(bw, 0))) {
            if (bon) { if (seq_k >= 0) seq_k = -1; else soundpreview_stop(); }
            else if (pack_sfx) { seq_k = 0; seq_t = 0; }
            else soundpreview_start(dev, 1);
        }
        ImGui::EndDisabled();
        if (!can_hear && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) ImGui::SetTooltip("A preview needs a game under way, outside a battle.");
    }
    /* the devices, in three tiers: the best (the MT-32, as composed), an OK one (the Sound Blaster,
       as most heard it in 1992), and those for history's sake */
    auto device = [&](const char *id, const char *title, const char *what, bool chosen) {
        ImGui::PushID(id);
        if (ImGui::RadioButton(title, chosen)) { config_set("sound.device", id); ui_dirty_config = 1; }
        if (what) { ImGui::Indent(UIS(30)); ui_hint("%s", what); ImGui::Unindent(UIS(30)); }
        ImGui::PopID();
    };
    auto tier = [&](const char *t) { ImGui::Spacing(); ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(120, 52, 20, 255)); ImGui::TextUnformatted(t); ImGui::PopStyleColor(); };
    tier("Best");
    device("mt32", "Roland MT-32", "Roland's synthesizer of 1987, the one Darklands' music and battle sounds were written for: they sound as intended.",
           !strcmp(cur, "mt32") || !strcmp(cur, "auto"));
    {
        ImGui::Indent(UIS(30));
        bool roms = mt32_available() != 0;
        static bool more;
        if (!roms) {      /* the one thing that pokes out: in red, a click for the rest */
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(170, 30, 20, 255));
            if (ImGui::Selectable(more ? "MT-32 ROMs not found (click to close)" : "MT-32 ROMs not found (click for how to get it working)", more, 0, ImVec2(ImGui::CalcTextSize("MT-32 ROMs not found (click for how to get it working)").x, 0))) more = !more;
            ImGui::PopStyleColor();
        }
        if (roms) { ImGui::SetNextItemOpen(more); more = ImGui::TreeNode("MT-32 details"); if (more) ImGui::TreePop(); }
        if (more) {
            ImGui::Indent(UIS(14));
            if (!roms) {
                ImGui::TextWrapped("The MT-32 was a Roland synthesizer of 1987; Darklands' music and battle sounds were written for it, and every other choice here is a "
                                   "lesser version. Darklands+ has it built in (Munt's imitation of it) and only needs two files copied from a real one's memory (its ROM files), which belong to Roland "
                                   "and can't be included. If you have used Munt or DOSBox with an MT-32 you already have them. Until they are found, the Sound Blaster plays.");
                if (ImGui::Button("Where to find them...")) ui_show_mt32_help();
            } else ui_hint("%s", mt32_status());
            static char romdir[1024]; static bool init;
            if (!init) { snprintf(romdir, sizeof romdir, "%s", config_get("sound.mt32_roms", "")); init = true; }
            ImGui::SetNextItemWidth(UIS(420));
            if (ImGui::InputTextWithHint("##mt32roms", "the folder with the ROM files", romdir, sizeof romdir)) { config_set("sound.mt32_roms", romdir); ui_dirty_config = 1; }
            ImGui::SameLine();
            if (ImGui::Button("Look again##mt32")) { mt32_rescan(); mt32_switch(); }
            ui_hint("Darklands+ finds them by their contents, whatever their names, in that folder and in:");
            for (int i = 0; i < mt32_dir_count(); i++) ui_hint("    %s", mt32_dir(i));
            if (mt32_families() == 3) {                /* both: which one */
                const char *m = config_get("sound.mt32_model", "mt32");
                if (ImGui::RadioButton("MT-32 (the battle sounds as blows and grunts, as composed)", !strcmp(m, "mt32"))) { config_set("sound.mt32_model", "mt32"); mt32_rescan(); mt32_switch(); ui_dirty_config = 1; }
                if (ImGui::RadioButton("CM-32L / LAPC-I (later Roland models), a curiosity: chimes and odd noises for the battle sounds", !strcmp(m, "cm32l"))) { config_set("sound.mt32_model", "cm32l"); mt32_rescan(); mt32_switch(); ui_dirty_config = 1; }
            }
            {   bool lcd = config_get_int("sound.mt32_lcd", 1);
                if (ImGui::Checkbox("Show the MT-32's little display (top right) when the game puts a message on it", &lcd)) { config_set_int("sound.mt32_lcd", lcd); ui_dirty_config = 1; } }
            device("midi", "A synthesizer outside Darklands+ (MIDI)", "For a real MT-32 connected to the computer, or Munt (an MT-32 imitation) installed on its own. "
                   "An ordinary synthesizer, such as the one built into Windows, plays the wrong instruments.", !strcmp(cur, "midi"));
            if (!strcmp(cur, "midi")) {
                int n = mpu_out_count();
                const char *out = config_get("sound.midi_out", "");
                if (!n) ui_hint("No MIDI outputs (connections to a synthesizer) on this computer.");
                else {
                    ImGui::SetNextItemWidth(UIS(420));
                    if (ImGui::BeginCombo("MIDI output", out[0] ? out : "the system's default")) {
                        if (ImGui::Selectable("the system's default", !out[0])) { config_set("sound.midi_out", ""); ui_dirty_config = 1; }
                        for (int i = 0; i < n; i++) { const char *nm = mpu_out_name(i); if (nm[0] && ImGui::Selectable(nm, !strcmp(nm, out))) { config_set("sound.midi_out", nm); ui_dirty_config = 1; } }
                        ImGui::EndCombo();
                    }
                }
            }
            ImGui::Unindent(UIS(14));
        }
        ImGui::Unindent(UIS(30));
    }
    tier("OK");
    device("fm", "Sound Blaster Pro", "The sound card most players had in 1992: the music arranged for its simpler synthesizer, thinner and buzzier than the MT-32.", !strcmp(cur, "fm"));
    tier("For history's sake");
    device("adlib", "AdLib (an older sound card)", "Much the same music; the battle sounds are rough imitations too.", !strcmp(cur, "adlib"));
    device("speaker", "PC speaker", "Beeps.", !strcmp(cur, "speaker"));
    device("game", "As set in the original game's INSTALL program", 0, !strcmp(cur, "game"));
    device("off", "Off", 0, !strcmp(cur, "off"));
    ui_hint("A change is heard from the next battle, or the next start.");
    {
        int v = sound_music_volume;
        ImGui::SetNextItemWidth(UIS(260));
        if (ImGui::SliderInt("Volume of the music", &v, 0, 150, "%d%%")) { sound_music_volume = v; config_set_int("sound.music_volume", v); ui_dirty_config = 1; }
    }

    /* custom packs: recordings in place of the synthesizer's music, or of the battle sounds (folded away) */
    ImGui::Spacing();
    extern char dlp_user_dir[];
    if (ImGui::TreeNode("Custom sound packs")) {
        ui_hint("Recordings that replace the synthesizer's music or battle sounds. A battle sounds pack comes with Darklands+ as a mod (F12 > Mods); no music pack does. The previews above play them when ticked.");
        {   /* music packs (mods/musicpack.c) */
            bool mp = musicpack_on;
            if (ImGui::Checkbox("Music from a music pack (recordings in a \"music\" folder)", &mp)) { musicpack_set(mp); ui_dirty_config = 1; }
            int n = musicpack_count();
            if (n) {
                const char *f0 = ""; for (int k = 0; k < 60 && !f0[0]; k++) f0 = musicpack_from(k);
                ui_hint("%d piece%s in the pack (%s); the synthesizer plays the others.", n, n == 1 ? "" : "s", strcmp(f0, "music") ? f0 : "the music folder");
            }
            else ui_hint("No music pack found: a mod's music folder, or %s%smusic, with a recording for each piece named by its number (10.ogg, \"11 market.ogg\").", dlp_user_dir, PATH_SEP);
            if (ImGui::TreeNode("The pieces")) {
                ui_hint("A recording replaces a piece when its file name starts with the number (up to four for one number, played in turn: \"27 a.ogg\", \"27 b.ogg\" ...). "
                        "Pieces that loop in the game loop in the pack; while a piece of the pack is on, the synthesizer is silent.");
                for (int k = 8; k <= 47; k++) {
                    const char *f = musicpack_file(k), *d = musicpack_desc(k);
                    const char *bn = f + strlen(f); while (bn > f && bn[-1] != '/' && bn[-1] != '\\') bn--;
                    ui_hint("%2d  %s%s%s", k, d, d[0] ? ", " : "", musicpack_loops(k) ? "loops" : "plays once");
                    if (f[0]) { ImGui::SameLine(); int nv = musicpack_versions(k); if (nv > 1) ImGui::Text("  %s and %d more, in turn", bn, nv - 1); else ImGui::Text("  %s", bn); }
                }
                if (ImGui::SmallButton("Look again##music")) musicpack_reload();
                ImGui::TreePop();
            }
        }
        {   /* battle sounds packs (mods/sfx.c) */
            ImGui::Spacing();
            bool sp = sfx_mode;   /* (hits: the fix "Ordinary hits make a sound") */
            if (ImGui::Checkbox("Battle sounds from a sound pack (the \"sounds\" folder)", &sp)) { sfx_set(sp); ui_dirty_config = 1; }
            int n = sfx_user_count();
            if (n) ui_hint("%d sound%s in %s%ssounds; the game's own sounds for the rest.", n, n == 1 ? "" : "s", dlp_user_dir, PATH_SEP);
            else ui_hint("Recorded blows, parries and cries in place of the synthesizer's: a pack in %s%ssounds, or the \"Battle sounds\" mod (F12 > Mods).", dlp_user_dir, PATH_SEP);
            if (sp) {
                ImGui::Indent(UIS(30));
                int v = sfx_volume;
                ImGui::SetNextItemWidth(UIS(260));
                if (ImGui::SliderInt("Volume of the pack", &v, 0, 150, "%d%%")) { sfx_set_volume(v); ui_dirty_config = 1; }
                static char msg[400];
                if (ImGui::Button("Look again##sfx")) { sfx_reload(); snprintf(msg, sizeof msg, "%d sound%s found.", sfx_user_count(), sfx_user_count() == 1 ? "" : "s"); }
                if (msg[0]) ui_hint("%s", msg);
                ImGui::Unindent(UIS(30));
            }
            if (ImGui::TreeNode("File names for a sound pack")) {
                for (int k = 0; k < SFX_COUNT; k++) ui_hint("%s.wav or .ogg: %s", SFX_NAMES[k], SFX_DESC[k]);
                ui_hint("Several versions of one sound (parry1.wav, parry2.wav ...) are played in turn at random.");
                ImGui::TreePop();
            }
        }
        ImGui::TreePop();
    }
}

