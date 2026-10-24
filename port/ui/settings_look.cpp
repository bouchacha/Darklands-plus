// settings_look.cpp - the Settings page's sections on what things look like (ui/settings.cpp draws
// the page): Text (the original lettering or the new text, the typeface, the coins) and Frames and
// clock (the band around the windows, the frames, the buttons, the clock, the potions' grade marks).
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

/* a row of pictures to choose from (show, don't describe): n cards, each drawn by draw(dl, a, b,
   value) in its box, its label under it; a click sets the config key */
void pic_choice(const char *id, const char *key, int def, const int *vals, const char *const *labels, int n, float cw, float ph,
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

void display_icons(void)
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
void look_text(void)
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
void look_ornaments(void)
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
void look_clock(void)
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

