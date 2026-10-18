// halloffame.cpp - the Honor Roll of Glory, a part of the game MicroProse left unfinished: the
// picture (pics\hallfame.pic, "Honor Roll of Glory" ... "Today's Tale"), the table
// (darkland.fam: 5 records of 0x6a bytes, int fame, int rank, int party seed, 4 x 25 bytes
// of names) and the code to show it (s038 16cd:00b0) and to enter the party (16cd:0404) are
// in the game, but nothing calls them, and 0404 never writes the table back (the manual
// speaks of a Hall of Fame; the game's README says there is none). Here a party enters the
// roll when its journey ends (and with F12 > Journal > "Enter the party now"), and the roll is
// shown then and from the Journal tab.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <string>
#include <vector>
#include <algorithm>
#include <SDL.h>
#include "imgui.h"
#include "ui.h"
#include "gamedata.h"
extern "C" {
#include "../runtime/cpu.h"
#include "../runtime/hle.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "../runtime/config.h"
u8 *hdart_decode_pic(const char *dosname, int *w, int *h, u32 *pal, u8 *mask);
}
extern ImFont *ui_font_small, *ui_font_text;

struct FameRec { int fame, rank, seed; std::string names[4]; };
static SDL_Renderer *hf_ren;
static SDL_Texture *hf_tex;
static int hf_w, hf_h;
static bool hf_open;
static std::vector<FameRec> roll;
static std::string hf_msg;

void halloffame_set_renderer(SDL_Renderer *r) { hf_ren = r; }

static std::string fam_path(void) { char p[1300]; dos_host_path("DARKLAND.FAM", p, sizeof p); return p; }
extern "C" void dos_host_path_write(const char *dos, char *out, size_t outn);
static std::string fam_path_write(void) { char p[1300]; dos_host_path_write("DARKLAND.FAM", p, sizeof p); return p; }

static void load_roll(void)
{
    roll.clear();
    FILE *f = fopen(fam_path().c_str(), "rb");
    if (!f) return;
    unsigned char b[0x212];
    size_t n = fread(b, 1, sizeof b, f);
    fclose(f);
    for (int i = 0; i < 5 && (size_t)(i + 1) * 0x6a <= n; i++) {
        const unsigned char *r = b + i * 0x6a;
        FameRec e; e.fame = (short)(r[0] | r[1] << 8); e.rank = (short)(r[2] | r[3] << 8); e.seed = (short)(r[4] | r[5] << 8);
        for (int k = 0; k < 4; k++) { char nm[26]; memcpy(nm, r + 6 + 25 * k, 25); nm[25] = 0; e.names[k] = game_text(nm, 25); }
        bool dup = false;          /* the shipped table repeats its last record */
        for (const FameRec &o : roll) if (o.seed == e.seed && o.fame == e.fame && o.names[0] == e.names[0]) dup = true;
        if (!dup && (e.fame || !e.names[0].empty())) roll.push_back(e);
    }
}

static bool save_roll(void)
{
    unsigned char b[0x212]; memset(b, 0, sizeof b);
    for (int i = 0; i < 5; i++) {
        const FameRec &e = roll[i < (int)roll.size() ? i : (int)roll.size() - 1];
        if (roll.empty()) break;
        unsigned char *r = b + i * 0x6a;
        r[0] = (unsigned char)e.fame; r[1] = (unsigned char)(e.fame >> 8); r[2] = (unsigned char)(i + 1); r[3] = 0;
        r[4] = (unsigned char)e.seed; r[5] = (unsigned char)(e.seed >> 8);
        for (int k = 0; k < 4; k++) { size_t l = e.names[k].size(); if (l > 24) l = 24; memcpy(r + 6 + 25 * k, e.names[k].c_str(), l); }
    }
    FILE *f = fopen(fam_path_write().c_str(), "wb");
    if (!f) return false;
    fwrite(b, 1, sizeof b, f); fclose(f);
    return true;
}

static FameRec current_party(void)
{
    FameRec e; e.fame = (s16)g_rw(GV_party_fame); e.rank = 0; e.seed = (s16)g_rw(G_PARTY_SEED);
    int k = 0;
    for (int m = 0; m < 5 && k < 4; m++) {
        if (!g_rb((u16)(G_MEMBER(m) + G_M_STATUS))) continue;
        char nm[26]; g_str(G_DGROUP, (u16)(G_MEMBER(m) + G_M_FULLNAME), nm, sizeof nm);
        if (nm[0]) e.names[k++] = nm;
    }
    return e;
}

/* the party joins the roll (or betters its own place): its fame against the four best */
extern "C" int halloffame_record(void)
{
    if (config_get_int("original", 0) || !config_get_int("assist.hall_of_fame", 1)) return 0;
    if (game_mode() == GM_OTHER) return 0;
    FameRec cur = current_party();
    if (cur.names[0].empty()) return 0;
    load_roll();
    bool found = false;
    for (FameRec &e : roll) if (e.seed == cur.seed && e.names[0] == cur.names[0]) { found = true; if (cur.fame > e.fame) e = cur; }
    if (!found) roll.push_back(cur);
    std::stable_sort(roll.begin(), roll.end(), [](const FameRec &a, const FameRec &b) { return a.fame > b.fame; });
    if (roll.size() > 5) roll.resize(5);
    bool in = false;
    for (int i = 0; i < 4 && i < (int)roll.size(); i++) if (roll[i].seed == cur.seed && roll[i].names[0] == cur.names[0]) in = true;
    bool ok = save_roll();
    fprintf(stderr, "hall of fame: %s with fame %d%s\n", cur.names[0].c_str(), cur.fame, ok ? (in ? ", on the roll" : ", not among the four best") : ", could not write DARKLAND.FAM");
    return in && ok;
}

extern "C" void halloffame_open(void) { load_roll(); hf_open = true; }
extern "C" int halloffame_is_open(void) { return hf_open; }
extern "C" void halloffame_close(void) { hf_open = false; }

static void upload(void)
{
    if (hf_tex || !hf_ren) return;
    u32 pal[256]; u8 mask[256]; memset(mask, 0, sizeof mask);
    u8 *img = hdart_decode_pic("PICS\\HALLFAME.PIC", &hf_w, &hf_h, pal, mask);
    if (!img) return;
    static const u8 EGA[16][3] = { {0,0,0},{0,0,42},{0,42,0},{0,42,42},{42,0,0},{42,0,42},{42,21,0},{42,42,42},{21,21,21},{21,21,63},{21,63,21},{21,63,63},{63,21,21},{63,21,63},{63,63,21},{63,63,63} };
    bool game = game_mode() != GM_OTHER;
    for (int i = 0; i < 256; i++) if (!mask[i]) {
        u32 r, g, b;
        if (game) { r = vga_dac[i * 3] & 63; g = vga_dac[i * 3 + 1] & 63; b = vga_dac[i * 3 + 2] & 63; }
        else if (i < 16) { r = EGA[i][0]; g = EGA[i][1]; b = EGA[i][2]; } else { r = g = b = 0; }
        pal[i] = 0xff000000u | ((r << 2 | r >> 4) << 16) | ((g << 2 | g >> 4) << 8) | (b << 2 | b >> 4);
    }
    std::vector<u32> px((size_t)hf_w * hf_h);
    for (int i = 0; i < hf_w * hf_h; i++) px[i] = pal[img[i]];
    free(img);
    hf_tex = SDL_CreateTexture(hf_ren, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, hf_w, hf_h);
    if (hf_tex) { SDL_UpdateTexture(hf_tex, 0, px.data(), hf_w * 4); SDL_SetTextureScaleMode(hf_tex, SDL_ScaleModeNearest); }
}

extern "C" void halloffame_draw(void)
{
    if (!hf_open) return;
    upload();
    ImGuiIO &io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(io.DisplaySize);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("Honor Roll", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::PopStyleVar();
    ImDrawList *dl = ImGui::GetWindowDrawList();
    ImVec2 o = ImGui::GetWindowPos(), sz = io.DisplaySize;
    dl->AddRectFilled(o, ImVec2(o.x + sz.x, o.y + sz.y), IM_COL32(10, 8, 6, 255));
    /* the picture at 4:3 */
    float k = fminf(sz.x / 320.0f, sz.y / 240.0f);
    float pw = 320 * k, ph = 240 * k, px0 = o.x + (sz.x - pw) * 0.5f, py0 = o.y + (sz.y - ph) * 0.5f;
    auto G = [&](float x, float y) { return ImVec2(px0 + x * pw / 320.0f, py0 + y * ph / 200.0f); };
    if (hf_tex) dl->AddImage((ImTextureID)(intptr_t)hf_tex, G(0, 0), G(320, 200));
    ImFont *f = ui_font_text ? ui_font_text : ImGui::GetFont();
    float line = 8.5f * ph / 200.0f;                /* engraved: 8.5 of the picture's 200 rows */
    const ImU32 ink = IM_COL32(52, 30, 12, 255), shine = IM_COL32(255, 240, 200, 120);
    auto text = [&](ImVec2 p, float size, const std::string &t, bool right) {
        ImVec2 ts = f->CalcTextSizeA(size, FLT_MAX, 0, t.c_str());
        if (right) p.x -= ts.x;
        dl->AddText(f, size, ImVec2(p.x + 1, p.y + 1), shine, t.c_str());
        dl->AddText(f, size, p, ink, t.c_str());
    };
    auto names_of = [](const FameRec &e) { std::string s; for (int i = 0; i < 4; i++) if (!e.names[i].empty()) { if (!s.empty()) s += ", "; s += e.names[i]; } return s; };
    /* the five plaques (the picture's rows 48-70, 72-94, 97-118, 121-142 and, Today's Tale, 148-169):
       the text centred on each */
    static const float PY[5] = { 59 - 4.6f, 83 - 4.6f, 107.5f - 4.6f, 131.5f - 4.6f, 158.5f - 4.6f };
    for (int i = 0; i < 4; i++) {
        if (i >= (int)roll.size()) break;
        const FameRec &e = roll[i];
        char n[16]; snprintf(n, sizeof n, "%d.", i + 1);
        text(G(30, PY[i]), line, n, false);
        text(G(44, PY[i]), line, names_of(e), false);
        char fm[24]; snprintf(fm, sizeof fm, "fame %d", e.fame);
        text(G(290, PY[i]), line, fm, true);
    }
    if (game_mode() != GM_OTHER) {
        FameRec cur = current_party();
        if (!cur.names[0].empty()) {
            text(G(44, PY[4]), line, names_of(cur), false);
            char fm[24]; snprintf(fm, sizeof fm, "fame %d", cur.fame);
            text(G(290, PY[4]), line, fm, true);
        }
    }
    /* Exit (the picture's button, 249,179 - 283,190), Esc, or a click anywhere */
    if ((ImGui::IsMouseClicked(0) && ImGui::IsWindowHovered()) || ImGui::IsKeyPressed(ImGuiKey_Escape) || ImGui::IsKeyPressed(ImGuiKey_Enter)) hf_open = false;
    ImGui::End();
}
