// iconsheets.cpp - the test command uiconstyles:N: sheets of the pictures drawn with ImGui shapes, to
// compare them across families and to look at new line art large (docs/REVIEW.md). 2: the towns',
// quests', careers', pages' and settings' sections' pictures, the inn's and camp's, the small signs
// (ui/marks.h), the age and the pointer drawn anew; 3: the battle
// orders', the ingredients and potions as items, the world map's; 4: the ingredients by family with
// their names; 7: the ingredients large; 8: the attributes, skills and standing large, in colour and
// heraldic; 1000+id: one ingredient window-large; any other: the skills and attributes in both looks.
#include <math.h>
#include "imgui.h"
#include "uitabs.h"
#include "ordericons.h"
#include "skillicons.h"
#include "townicons.h"
#include "quests.h"
#include "marks.h"
#include "pointer.h"

extern int ui_icon_disc;
void ui_icon_style_preview(int st);
void ui_page_icon(ImDrawList *dl, ImVec2 c, float s, int page); void ui_section_icon(ImDrawList *dl, ImVec2 c, float s, const char *name);
static int sheet_on;
extern "C" void ui_icon_styles_sheet(int on) { sheet_on = on; }
extern "C" const char *gm_skill_name(int k); extern "C" const char *gm_attr_name(int a);
void ui_lodge_act_icon(ImDrawList *, ImVec2, float, int); void ui_campfire_icon(ImDrawList *, ImVec2, float);   /* (the other sets, for the sheet) */
void draw_icon_seal(ImDrawList *, ImVec2, float, int); void layer_glyph(ImDrawList *, ImVec2, float, int, ImU32);
void ui_compass_icon(ImDrawList *, ImVec2, float, float); void ui_boot_icon(ImDrawList *, ImVec2, float, unsigned int);
void ui_draw_teach_mark(ImDrawList *, ImVec2, float, unsigned, unsigned); void ui_item_icon(ImDrawList *, ImVec2, float, int);
bool ui_draw_ingredient(ImDrawList *, ImVec2, float, int);   /* ui/alchemyicons.cpp */
extern "C" void ui_icon_styles_draw(void)
{
    if (!sheet_on) return;
    ImDrawList *dl = ImGui::GetForegroundDrawList(); ImVec2 ds = ImGui::GetIO().DisplaySize;
    dl->AddRectFilled(ImVec2(0, 0), ds, IM_COL32(246, 236, 212, 255));
    if (sheet_on == 2) {   /* every other set of icons, to look for one drawn twice for two things (uiconstyles:2) */
        static const char *SECT[] = { "Preset", "New screens", "Text", "Frames and clock", "Cards and towns", "The party and its items", "Battles", "Travel",
                                      "Fixes for bugs of the original", "Screen", "Intro", "Sound", "Controller" };
        float cell = fminf(ds.x / 14, ds.y / 9.1f), sz = cell * 0.5f; int n = 0;   /* (its ten rows all in the window) */
        auto at = [&](int k) { return ImVec2(cell * 0.5f + (k % 14) * cell, cell * 0.45f + (k / 14) * cell * 0.9f); };
        for (int i = 0; i < 40; i++) ui_draw_town_icon(dl, at(n++), sz, i);                 /* (the town icons: as many as there are) */
        n = (n + 13) / 14 * 14;
        for (int i = 1; i < 13; i++) ui_draw_quest_icon(dl, at(n++), sz, i);
        n = (n + 13) / 14 * 14;
        for (int i = 0; i < 8; i++) ui_draw_career_icon(dl, at(n++), sz, i);
        n = (n + 13) / 14 * 14;
        for (int i = 0; i < 10; i++) ui_page_icon(dl, at(n++), sz * 0.8f, i);
        n = (n + 13) / 14 * 14;
        for (const char *t : SECT) ui_section_icon(dl, at(n++), sz * 0.8f, t);
        n = (n + 13) / 14 * 14;                                                          /* the inn's and camp's day choices, the camp's fire */
        for (int i = 0; i < 7; i++) ui_lodge_act_icon(dl, at(n++), sz * 0.9f, i);
        ui_campfire_icon(dl, at(n++), sz * 0.8f);
        n = (n + 13) / 14 * 14;                                                          /* the small signs (ui/marks.h), the age */
        { float r = sz * 0.4f, th = fmaxf(1.0f, sz * 0.06f);
          ui_mark_star(dl, at(n++), r, IM_COL32(222, 168, 40, 255), IM_COL32(120, 70, 10, 255), th);
          ui_mark_aside(dl, at(n++), r, IM_COL32(120, 80, 36, 255), th);
          ui_mark_pentacle(dl, at(n++), r, IM_COL32(110, 60, 140, 255), th);
          ui_mark_falling_bars(dl, at(n++), r, IM_COL32(120, 80, 36, 255));
          ui_mark_lost(dl, at(n++), r, 72, th);
          ui_mark_stink(dl, at(n++), r, th);
          ui_mark_blast(dl, at(n++), r);
          ui_age_icon(dl, at(n++), sz * 0.8f); }
        n = (n + 13) / 14 * 14;                                                          /* the pointer drawn anew (ui/pointer.h): its hands and badges */
        { float pk = cell / 30.0f;
          for (int b = 0; b < 4; b++) { ImVec2 c = at(n++); ui_pointer_draw(dl, ImVec2(c.x - 13 * pk, c.y - 7 * pk), pk, PTR_POINT, b); }
          ImVec2 c = at(n++); ui_pointer_draw(dl, ImVec2(c.x - 13 * pk, c.y - 7 * pk), pk, PTR_GRAB, PTR_PLAIN); }
        return;
    }
    if (sheet_on >= 1000) {   /* one ingredient's sign as large as the window allows (uiconstyles:1000+id) */
        ui_draw_ingredient(dl, ImVec2(ds.x * 0.5f, ds.y * 0.5f), ds.y * 0.8f, (sheet_on - 1000) % 100);
        return;
    }
    if (sheet_on == 8) {   /* the attributes, skills and the party's standing large (uiconstyles:8): in colour, and heraldic beside */
        ImFont *f = ImGui::GetFont(); float cw = ds.x / 7, sz = fminf(cw * 0.36f, ds.y / 11.5f), fs = sz * 0.32f; int n = 0;
        auto cell = [&](int kind, int k, const char *name) {
            ImVec2 o(cw * (n % 7), (n / 7) * sz * 1.75f); n++;
            ImVec2 a(o.x + cw * 0.27f, o.y + sz * 0.75f), b(o.x + cw * 0.72f, o.y + sz * 0.75f);
            for (int st = 0; st < 2; st++) {
                ImVec2 c = st ? b : a; ui_icon_style_preview(st ? 2 : 0);
                if (kind == 0) ui_attr_icon(dl, c, sz, k); else if (kind == 1) ui_skill_icon(dl, c, sz, k); else ui_standing_icon(dl, c, sz, k);
            }
            ui_icon_style_preview(-1);
            float w = f->CalcTextSizeA(fs, 1e9f, 0, name).x;
            dl->AddText(f, fs, ImVec2(o.x + cw * 0.5f - w * 0.5f, o.y + sz * 1.38f), IM_COL32(60, 40, 20, 255), name);
        };
        for (int k = 0; k < 7; k++) cell(0, k, gm_attr_name(k));
        for (int k = 0; k < 19; k++) cell(1, k, gm_skill_name(k));
        cell(2, 0, "Reputation"); cell(2, 1, "Fame");
        return;
    }
    if (sheet_on == 7) {   /* the ingredients' signs large, to look at the strokes' joins (uiconstyles:7) */
        static const int IDS[] = { 91, 92, 93, 94, 72, 89, 87, 88, 78, 77, 73, 90, 79, 80, 85, 82, 74, 83, 75, 76, 81, 84, 86 };
        float sz = fminf(ds.y / 5.0f, ds.x / 8.75f); int n = 0;          /* (a roundel is 1.17 sizes across) */
        auto at = [&](int k) { return ImVec2(sz * 0.65f + (k % 7) * sz * 1.25f, sz * 0.65f + (k / 7) * sz * 1.22f); };
        for (int id : IDS) ui_draw_ingredient(dl, at(n++), sz, id);
        return;
    }
    if (sheet_on == 4) {   /* the ingredients by family, each sign with its name (uiconstyles:4) */
        struct Fam { const char *name; int ids[8]; };
        static const Fam FAM[] = {
            { "The four bases: the elements' triangles", { 91, 92, 93, 94, 0 } },
            { "Metals and their ores: the planets' signs, in pewter (gold in gold)", { 72, 89, 87, 88, 78, 77, 73, 0 } },
            { "Sulphurs, salts and poisons", { 90, 79, 80, 85, 0 } },
            { "Waters, oils and vapours", { 82, 74, 83, 75, 0 } },
            { "Herbs: no sign in the tables, one made as theirs are", { 76, 81, 84, 86, 0 } } };
        static const char *NM[23] = { "Pure Gold", "Manganes", "Naptha", "Marsh Vapor", "E. Black Bean", "Zincblende", "Antimoni", "Orpiment", "White Cinnabar",
                                      "Solanaceae", "Aqua Regia", "Gum", "Mondragora", "Alum", "Camomile", "Nikel", "Pitchblende", "Zinken", "Brimstone",
                                      "Choleric Base", "Phlegmatic Base", "Sanguine Base", "Melancholic Base" };
        ImFont *f = ImGui::GetFont(); const ImU32 INK = IM_COL32(60, 40, 20, 255), SOFT = IM_COL32(120, 90, 60, 255);
        const int COLS = 7; float cw = ds.x / COLS, fs = ds.y / 52, sz = fminf(cw * 0.5f, ds.y / 13.5f), rh = sz * 1.32f + fs * 1.3f, y = fs * 0.5f;
        for (const Fam &F : FAM) {
            dl->AddText(f, fs * 0.95f, ImVec2(fs, y), SOFT, F.name); y += fs * 1.25f;
            int k = 0;
            for (; F.ids[k]; k++) {
                float x = (k % COLS) * cw, yy = y + (k / COLS) * rh;
                ui_draw_ingredient(dl, ImVec2(x + cw * 0.5f, yy + sz * 0.6f), sz, F.ids[k]);
                const char *n = NM[F.ids[k] - 72]; float w = f->CalcTextSizeA(fs, 1e9f, 0, n).x;
                dl->AddText(f, fs, ImVec2(x + cw * 0.5f - w * 0.5f, yy + sz * 1.2f), INK, n);
            }
            y += ((k + COLS - 1) / COLS) * rh + fs * 0.3f;
        }
        return;
    }
    if (sheet_on == 3) {   /* the rest (uiconstyles:3): battle orders, the world map's, ingredients and potions */
        float cell = ds.x / 14, sz = cell * 0.5f; int n = 0;
        auto at = [&](int k) { return ImVec2(cell * 0.5f + (k % 14) * cell, cell * 0.45f + (k / 14) * cell * 0.9f); };
        for (int i = 0; i < OI_N; i++) ui_draw_order_icon(dl, at(n++), sz * 0.8f, i);
        for (int i : { 0, 1, 2, 3, 4, 7 }) ui_draw_style_icon(dl, at(n++), sz * 0.8f, i);   /* (the members' ways of fighting, and holding where sent: mods/styles.h) */
        n = (n + 13) / 14 * 14;                                                          /* the alchemists' ingredients (72-94) and potions (95-116) */
        for (int i = 72; i <= 116; i++) { if (i == 95) n = (n + 13) / 14 * 14; ui_item_icon(dl, at(n++), sz * 0.95f, i); }
        n = (n + 13) / 14 * 14;                                                          /* the world map: its places, its layers, a quest's way */
        for (int i = 0; i < 12; i++) draw_icon_seal(dl, at(n++), sz * 0.7f, i);
        for (int i = 0; i < 5; i++) layer_glyph(dl, at(n++), sz * 0.7f, i, IM_COL32(80, 50, 24, 255));
        ui_compass_icon(dl, at(n++), sz * 0.7f, 0.6f); ui_boot_icon(dl, at(n++), sz * 0.7f, IM_COL32(80, 50, 24, 255));
        ui_draw_teach_mark(dl, at(n++), sz * 0.9f, IM_COL32(80, 50, 24, 255), IM_COL32(252, 246, 228, 255));
        return;
    }
    static const char *TITLE[2] = { "Drawn in color (no disc)", "Heraldic" }; static const int ST[2] = { 0, 2 };
    float colw = ds.x / 2, row = ds.y / 27.5f, sz = row * 0.8f;
    for (int j = 0; j < 2; j++) {
        float x = j * colw + 20;
        dl->AddText(ImGui::GetFont(), row * 0.7f, ImVec2(x, row * 0.3f), IM_COL32(60, 40, 20, 255), TITLE[j]);
        ui_icon_style_preview(ST[j]); int keep = ui_icon_disc; ui_icon_disc = 0;
        for (int i = 0; i < 26; i++) {
            float y = row * (1.4f + i);
            ImVec2 c(x + sz * 0.6f, y + row * 0.5f);
            if (i < 7) ui_attr_icon(dl, c, sz, i); else ui_skill_icon(dl, c, sz, i - 7);
            dl->AddText(ImGui::GetFont(), row * 0.55f, ImVec2(x + sz * 1.5f, y + row * 0.2f), IM_COL32(60, 40, 20, 255), i < 7 ? gm_attr_name(i) : gm_skill_name(i - 7));
        }
        ui_icon_style_preview(-1); ui_icon_disc = keep;
    }
}
