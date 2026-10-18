// partypanel.cpp - the party panel drawn by Darklands+ (display.native_panel; "Play the original
// game" keeps the game's). Wherever the game shows its panel (the five boxes left of the
// picture), each member's box is drawn anew over it: the name (gold for the leader, grey for
// one out of action), a picture, and Endurance, Strength and Divine Favor as labelled bars with
// their numbers; in a battle the order being carried out. The picture is the member's own
// portrait if there is one (portraits\<name>.png or .jpg in the Darklands+ folder), else the
// figure the game shows beside a battle, in the member's colours, or the herald
// (display.panel_picture: 0 portrait, figure or herald; 1 figure or herald; 2 herald; 3 none). The game still takes the clicks there (a member's sheet, orders
// in a battle); native screens frame the member they are about (partypanel_focus) and find a
// member under the pointer (partypanel_member_at, for dragging things onto them).
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <string>
#include <map>
#include <vector>
#include <sys/stat.h>
#include <ctype.h>
#include <SDL.h>
#include "nsdraw.h"
#include "gamepics.h"
extern "C" {
#include "../runtime/cpu.h"
#include "../runtime/hle.h"
#include "../runtime/game.h"
#include "../runtime/config.h"
#include "../mods/gamemodel.h"
#include "../third_party/stb/stb_image.h"
u8 *hdart_decode_pic(const char *dosname, int *w, int *h, u32 *pal, u8 *mask);
extern char dlp_user_dir[], game_dir[];
}
static std::string user_folder(void) { return dlp_user_dir[0] ? dlp_user_dir : game_dir; }

static int pp_on = 1, pp_pic = 0;
int partypanel_focus = -1;                       /* the member a native screen is about */
static bool shown_now;

extern "C" int partypanel_is_on(void) { return pp_on; }
extern "C" void partypanel_set(int on) { pp_on = on; config_set_int("display.native_panel", on); }
extern "C" int partypanel_picture(void) { return pp_pic; }
extern "C" void partypanel_set_picture(int v) { pp_pic = v; config_set_int("display.panel_picture", v); }
extern "C" void partypanel_init(void) { pp_on = config_get_int("display.native_panel", 1); pp_pic = config_get_int("display.panel_picture", 0); }
extern "C" int partypanel_shown(void) { return shown_now; }

/* messages the game writes into a member's box (a skill grown at a battle's end): shown on the box
 * for a while (battlelog.cpp takes them from the game) */
struct PNote { std::string text; u32 t; bool good; };
static std::vector<PNote> pnotes[6];                 /* [5]: the whole panel */
extern "C" u32 plat_ticks_ms(void);
extern "C" void partypanel_note(int row, const char *text, int good)
{
    int k = row >= 0 && row < 5 ? row : 5;
    std::vector<PNote> &v = pnotes[k];
    v.push_back({ text, plat_ticks_ms(), good != 0 });
    if (v.size() > 3) v.erase(v.begin());
}
static void draw_notes(NsDraw &P, int k, float x0, float y0, float x1, float y1)
{
    std::vector<PNote> &v = pnotes[k];
    u32 now = plat_ticks_ms();
    while (!v.empty() && now - v.front().t > 9000) v.erase(v.begin());
    if (v.empty()) return;
    float y = y1;
    for (int i = (int)v.size() - 1; i >= 0; i--) {
        const PNote &n = v[i];
        float a = now - n.t > 8000 ? (9000 - (now - n.t)) / 1000.0f : 1.0f;
        float th = P.fo->CalcTextSizeA(P.size(P.fo, 4.3f), FLT_MAX, (x1 - x0 - 4) * P.kx, n.text.c_str()).y / P.k + 2.4f;
        float h = k < 5 ? fmaxf(th, (y1 - y0) / 3.0f) : th;    /* (on a member: at least a bar's row - half a row covered looked broken) */
        if (y - h < y0) break;
        y -= h;
        P.rect(x0, y, x1, y + h - 0.4f, IM_COL32(24, 16, 8, (int)(225 * a)), 1.2f);
        P.para(P.fo, 4.3f, x0 + 2, y + 1.0f + (h - th) * 0.5f, x1 - x0 - 4, n.good ? IM_COL32(250, 214, 110, (int)(255 * a)) : IM_COL32(240, 120, 100, (int)(255 * a)), n.text);
    }
}

/* the game's panel is on the screen: the rope frames left and right of its boxes (three
 * browns, or cyan round the member a screen is about) */
static bool frame_colour(u8 v)
{
    const u8 *c = vga_dac + v * 3;
    int r = c[0] & 63, g = c[1] & 63, b = c[2] & 63;
    return (r == 10 && g == 8 && b == 3) || (r == 18 && g == 14 && b == 6) || (r == 39 && g == 24 && b == 14) || (r == 21 && g == 63 && b == 63);
}
static bool game_panel_shown(void)
{
    int rows = 0;
    for (int i = 0; i < 5; i++) {
        int good = 0, all = 0;
        for (int dy = 4; dy < 40; dy += 8)
            for (int x : { 0, 1, 2, 57, 58, 59 }) { all++; if (frame_colour(rb(0xa000, (u16)((40 * i + dy) * 320 + x)))) good++; }
        if (good * 5 >= all * 4) rows++;
    }
    return rows >= 3;
}

/* a member's own portrait, from the portraits folder (by short or full name) */
struct Portrait { SDL_Texture *tex = nullptr; int w = 0, h = 0; long mtime = -1; };
static std::map<std::string, Portrait> portraits;
static SDL_Renderer *pp_ren;
extern "C" void partypanel_set_renderer(SDL_Renderer *r) { pp_ren = r; }
static const Portrait *portrait(const std::string &name)
{
    if (name.empty() || !pp_ren) return nullptr;
    static const char *EXT[] = { ".png", ".jpg", ".jpeg", ".PNG", ".JPG" };
    std::string path; struct stat st; long mt = -1;
    for (const char *e : EXT) {
        path = user_folder() + "/portraits/" + name + e;
        if (stat(path.c_str(), &st) == 0) { mt = (long)st.st_mtime; break; }
    }
    Portrait &p = portraits[name];
    if (mt < 0) { if (p.tex) SDL_DestroyTexture(p.tex); p = Portrait(); return nullptr; }
    if (p.mtime == mt) return p.tex ? &p : nullptr;
    if (p.tex) SDL_DestroyTexture(p.tex);
    p = Portrait(); p.mtime = mt;
    int n; unsigned char *px = stbi_load(path.c_str(), &p.w, &p.h, &n, 4);
    if (!px) return nullptr;
    p.tex = SDL_CreateTexture(pp_ren, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STATIC, p.w, p.h);
    if (p.tex) { SDL_UpdateTexture(p.tex, 0, px, p.w * 4); SDL_SetTextureBlendMode(p.tex, SDL_BLENDMODE_BLEND); SDL_SetTextureScaleMode(p.tex, SDL_ScaleModeLinear); }
    stbi_image_free(px);
    return p.tex ? &p : nullptr;
}

/* a figure as the game shows it (PICS\<code><suffix>.PIC: SHORT beside a battle, SMALL the creation
 * menus' portrait): its colours 0xeb..0xf2 are the member's own (the colour keys, member_palette),
 * the rest the battle's common colours (in the PIC's own palette, else in another figure's, else
 * the screen's). Kept per cache slot (made again when the code or the colours change). */
struct FigTex { GamePic pic; std::string key; };
static std::map<int, FigTex> figtex;
const GamePic *ns_figure_pic(int cache, const char *code, const unsigned char *mp, const char *suffix)
{
    if (!code || !code[0] || !pp_ren) return nullptr;
    std::string key = std::string(code) + suffix + std::string((const char *)mp, 24);
    FigTex &f = figtex[cache];
    if (f.key == key) return f.pic.tex ? &f.pic : nullptr;
    if (f.pic.tex) SDL_DestroyTexture(f.pic.tex);
    f = FigTex(); f.key = key;
    static u32 pal[256], common[256]; static u8 mask[256], cmask[256]; static bool have_common;
    if (!have_common) {                                 /* the common colours: from the game's own figures */
        have_common = true;
        for (const char *c : { "F01", "F60", "A00" }) {
            int w, h; u32 p2[256]; u8 m2[256];
            memset(m2, 0, sizeof m2);
            u8 *im = hdart_decode_pic((std::string("PICS\\") + c + "SHORT.PIC").c_str(), &w, &h, p2, m2);
            free(im);
            for (int i = 0; i < 256; i++) if (m2[i] && !cmask[i]) { common[i] = p2[i]; cmask[i] = 1; }
        }
    }
    int w = 0, h = 0;
    memset(mask, 0, sizeof mask);
    u8 *img = hdart_decode_pic((std::string("PICS\\") + code + suffix + ".PIC").c_str(), &w, &h, pal, mask);
    if (!img) return nullptr;
    auto c8 = [](int v) { v &= 63; return (u32)(v << 2 | v >> 4); };
    std::vector<u32> argb((size_t)w * h, 0);
    for (int i = 0; i < w * h; i++) {
        int p = img[i];
        if (!p) continue;
        u32 c;
        if (p >= 0xeb && p <= 0xf2) { const unsigned char *q = mp + 3 * (p - 0xeb); c = c8(q[0]) << 16 | c8(q[1]) << 8 | c8(q[2]); }
        else if (mask[p]) c = pal[p] & 0xffffff;
        else if (cmask[p]) c = common[p] & 0xffffff;
        else { const u8 *q = vga_dac + p * 3; c = c8(q[0]) << 16 | c8(q[1]) << 8 | c8(q[2]); }
        argb[i] = 0xff000000u | c;
    }
    free(img);
    f.pic.w = w; f.pic.h = h;
    f.pic.tex = SDL_CreateTexture(pp_ren, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, w, h);
    if (f.pic.tex) { SDL_UpdateTexture(f.pic.tex, 0, argb.data(), w * 4); SDL_SetTextureBlendMode(f.pic.tex, SDL_BLENDMODE_BLEND); SDL_SetTextureScaleMode(f.pic.tex, SDL_ScaleModeNearest); }
    return f.pic.tex ? &f.pic : nullptr;
}
/* a member's figure beside a battle */
static const GamePic *figure(int m)
{
    char code[8]; gm_member_figure_code(m, code, sizeof code);
    unsigned char mp[24]; gm_member_palette(m, mp);
    return ns_figure_pic(m, code, mp, "SHORT");
}

/* the order, in the words of the game's Orders menu: the word on the card and, in its tooltip, the sentence */
static const char *order_word(char c, int place, bool shortw)
{
    #define W(s, l) return shortw ? s : l
    switch (c) {
    case 'A': W("attacks", "attacks (a standard attack)");
    case 'V': W("all-out", "attacks all-out: harder blows, a weaker defence");
    case 'B': W("berserk", "is berserk: attacks without thought of defence");
    case 'P':
        if (place == 3) W("picks", "picks the lock of the door given");
        if (place == 6) W("picks", "picks the lock of the chest given");
        W("parries", "parries: defends and does not strike");
    case 'M': W("shoots", "shoots at the target given");
    case 'T': W("throws", "throws a potion at the target given");
    case 'W': W("walks", "walks to the spot given");
    case 'F': W("flees", "flees to the spot given: does not stop to fight, nor strike back");
    case 'U': if (place == 2) W("stairs", "takes the stairs given"); W("door", "opens or shuts the door given");
    case 'D': W("unlocks", "dissolves the lock given");
    case 'O': W("opens", "opens the chest given");
    case 'X': W("leaves", "leaves the battlefield by the way out given");
    case 'H': W("halts", "halts and stands where they are");
    case 'b':
        if (config_get_int("battle.reposition_shooters", 0) && !config_get_int("original", 0))
            W("no line", "has no clear line of fire to the target: steps aside or nearer until there is one");
        W("no line", "has no clear line of fire to the target and waits for one (or give another order)");
    case 'e': W("no shot", "has nothing left to shoot or throw: give another order");
    case '-': W("waits", "waits for an order: click them here or on the field, then a foe, a spot or a choice of the Orders menu");
    }
    #undef W
    return nullptr;
}

extern "C" int partypanel_member_at(int gx, int gy)
{
    if (!shown_now || gx < 0 || gx >= 60 || gy < 0 || gy >= 200) return -1;
    int m = gy / 40;
    return m < gm_party_size() && gm_member_present(m) ? m : -1;
}

extern "C" void partypanel_draw(void)
{
    shown_now = false;
    if (!pp_on || config_get_int("original", 0) || game_mode() == GM_OTHER || !game_panel_shown()) return;
    NsDraw P;
    if (!P.f) { P.f = ui_font_game[ui_game_font] ? ui_font_game[ui_game_font] : ui_font_text; P.fo = ui_font_overlay ? ui_font_overlay : P.f; }
    if (!P.f) return;
    ImVec2 o = NsDraw::w(0, 0), e = NsDraw::w(320, 200);
    P.k = (e.y - o.y) / 200.0f; P.kx = (e.x - o.x) / 320.0f;
    if (P.k * 6 * 1.18f > 44 && ui_font_game_hi[ui_game_font]) P.f = ui_font_game_hi[ui_game_font];
    P.fk = ui_face_k(P.f); P.fok = ui_face_k(P.fo);
    P.dl = ImGui::GetBackgroundDrawList();
    { void plat_mouse_game(int *x, int *y); plat_mouse_game(&P.mx, &P.my); }   /* (the pointer now: ns_in is read only while a native screen runs) */
    shown_now = true;

    const ImU32 BG = IM_COL32(18, 13, 9, 255), CARD = IM_COL32(38, 29, 20, 255), EDGE = IM_COL32(112, 82, 46, 255);
    const ImU32 CREAM = IM_COL32(238, 226, 200, 255), GOLD = IM_COL32(240, 198, 92, 255), GREY = IM_COL32(150, 138, 124, 255), DIM = IM_COL32(176, 160, 136, 255);
    P.rect(0, 0, 60, 200, BG);
    int size = gm_party_size(), lead = gm_leader();
    bool battle = gm_in_battle() && game_mode() == GM_BATTLE;      /* (not the loot after it) */
    for (int m = 0; m < 5; m++) {
        float y0 = 40.0f * m;
        bool here = m < size && gm_member_present(m);
        if (!here) continue;                                    /* (an empty place: nothing) */
        P.rect(1.2f, y0 + 1.2f, 58.8f, y0 + 38.8f, CARD, 2);
        int st = here ? gm_member_status(m) : 0;
        bool focus = m == partypanel_focus && here, sel = battle && here && gm_member_selected(m);
        P.frame(1.2f, y0 + 1.2f, 58.8f, y0 + 38.8f, focus ? IM_COL32(110, 230, 236, 255) : sel ? IM_COL32(250, 236, 170, 255) : EDGE, 2);
        bool out = st == 3 || st == 5 || gm_member_elsewhere(m);
        char nm[32]; gm_member_name(m, nm, sizeof nm);
        std::string name = ns_gtext(nm);
        /* the order (in a battle), a word right of the name */
        char oc = battle && !out ? gm_member_order(m) : 0;
        int place = oc ? gm_member_order_place(m) : 0;
        float name_end = 56.5f;
        if (oc && order_word(oc, place, true)) {
            const char *wd = order_word(oc, place, true);
            float ww = P.width(P.fo, NsDraw::FS_SMALL, wd);
            P.text(P.fo, NsDraw::FS_SMALL, 0, y0 + 2.9f, DIM, wd, 56.5f);
            name_end = 56.5f - ww - 1.5f;
            if (P.over(56.5f - ww, y0 + 2.0f, 56.5f, y0 + 8.6f)) ImGui::SetTooltip("%s %s", name.c_str(), order_word(oc, place, false));
        }
        P.text(P.f, NsDraw::FS_NAME, 3.2f, y0 + 1.8f, out ? GREY : m == lead ? GOLD : CREAM, name, -1, name_end);
        /* the picture */
        float px0 = 2.6f, py0 = y0 + 9.6f, px1 = 17.4f, py1 = y0 + 37.0f;
        P.rect(px0, py0, px1, py1, IM_COL32(24, 20, 30, 255), 1.2f);
        bool drawn = false;
        if (pp_pic == 0) {
            char full[40]; gm_member_fullname(m, full, sizeof full);
            const Portrait *pt = portrait(nm); if (!pt) pt = portrait(full);
            if (pt) {
                ImVec2 a = NsDraw::w(px0, py0), b = NsDraw::w(px1, py1);
                float bw = b.x - a.x, bh = b.y - a.y, s = fmaxf(bw / pt->w, bh / pt->h);
                float uw = bw / (pt->w * s), uh = bh / (pt->h * s);     /* the middle of it, filling the box */
                P.dl->AddImageRounded((ImTextureID)(intptr_t)pt->tex, a, b, ImVec2(0.5f - uw * 0.5f, 0.5f - uh * 0.5f), ImVec2(0.5f + uw * 0.5f, 0.5f + uh * 0.5f), IM_COL32_WHITE, 1.2f * P.k);
                drawn = true;
            }
        }
        if (!drawn && pp_pic != 3) {
            SDL_Texture *t = nullptr; int tw = 0, th = 0;
            const GamePic *fi = pp_pic != 2 ? figure(m) : nullptr;
            if (fi) { t = fi->tex; tw = fi->w; th = fi->h; }
            char fig = gm_member_herald_letter(m);                          /* the herald: PICS\SHIELD<letter>.PIC */
            if (!t && fig) { char fn[32]; snprintf(fn, sizeof fn, "PICS\\SHIELD%c.PIC", fig); const GamePic *g = gamepic(fn); if (g) { t = g->tex; tw = g->w; th = g->h; } }
            if (t) {
                float s = fminf((px1 - px0 - 1.5f) / tw, (py1 - py0 - 1.5f) / th);
                float cx = (px0 + px1) * 0.5f, cy = (py0 + py1) * 0.5f, hw = tw * s * 0.5f, hh = th * s * 0.5f;
                P.dl->AddImage((ImTextureID)(intptr_t)t, NsDraw::w(cx - hw, cy - hh), NsDraw::w(cx + hw, cy + hh));
            }
        }
        /* a heavy load: an iron weight on the picture's corner, amber (heavy) or red (overloaded);
           the game's classes (s029 0e76:013a): above endurance + strength heavy, above one and a half times that overloaded */
        int band = st == 1 || st == 2 ? gm_member_load_band(m) : 0;
        if (band >= 2 && !out) {
            float wx = px1 - 3.4f, wy = py1 - 3.6f, r = 3.0f;
            ImU32 fill = band == 3 ? IM_COL32(200, 60, 44, 255) : IM_COL32(222, 168, 60, 255);
            P.dl->AddCircleFilled(NsDraw::w(wx, wy), (r + 0.6f) * P.k, IM_COL32(18, 13, 9, 235));
            ImVec2 a = NsDraw::w(wx - 2.2f, wy - 0.8f), b = NsDraw::w(wx + 2.2f, wy - 0.8f), c2 = NsDraw::w(wx + 2.8f, wy + 2.2f), d = NsDraw::w(wx - 2.8f, wy + 2.2f);
            P.dl->AddQuadFilled(a, b, c2, d, fill);                     /* the weight */
            P.dl->AddCircle(NsDraw::w(wx, wy - 1.9f), 1.1f * P.k, fill, 0, fmaxf(1.0f, 0.5f * P.k));   /* its ring */
            if (P.over(wx - r, wy - r, wx + r, wy + r)) {
                int e, em, s2, sm; gm_member_attr(m, 0, &e, &em); gm_member_attr(m, 1, &s2, &sm);
                ImGui::SetTooltip("%s carries %s: %d against %d (endurance + strength).\nIt costs agility in card tests and in fending off blows, and slows the swings. A click opens the sheet.",
                                  name.c_str(), band == 3 ? "far too much" : "a heavy load", gm_member_load(m), e + s2);
            }
        }
        if (st == 3) {
            P.rect(px0, py0, px1, py1, IM_COL32(20, 10, 8, 170), 1.2f);
            P.dl->AddLine(NsDraw::w(px0 + 2, py0 + 2), NsDraw::w(px1 - 2, py1 - 2), IM_COL32(170, 40, 30, 230), fmaxf(1.5f, 0.6f * P.k));
            P.dl->AddLine(NsDraw::w(px1 - 2, py0 + 2), NsDraw::w(px0 + 2, py1 - 2), IM_COL32(170, 40, 30, 230), fmaxf(1.5f, 0.6f * P.k));
        }
        /* Endurance, Strength, Divine Favor */
        static const char *LBL[3] = { "End", "Str", "DF" };
        static const int ATT[3] = { 0, 1, 6 };
        static const ImU32 COL[3] = { IM_COL32(104, 132, 244, 255), IM_COL32(96, 184, 76, 255), IM_COL32(230, 200, 76, 255) };
        for (int r = 0; r < 3; r++) {
            float ry = y0 + 10.0f + r * 9.3f, bx0 = 19.5f, bx1 = 56.6f;
            int cur, mx; gm_member_attr(m, ATT[r], &cur, &mx);
            char v[24];
            if (r == 2) snprintf(v, sizeof v, "%d", cur); else snprintf(v, sizeof v, "%d/%d", cur, mx);
            P.text(P.fo, NsDraw::FS_SMALL, bx0, ry - 0.6f, DIM, LBL[r]);
            P.text(P.fo, NsDraw::FS_SMALL, 0, ry - 0.6f, out ? GREY : CREAM, v, bx1);
            float f = mx > 0 ? fminf(1.0f, fmaxf(0.0f, (float)cur / mx)) : 0;
            P.rect(bx0, ry + 5.0f, bx1, ry + 7.2f, IM_COL32(64, 52, 40, 255), 0.8f);
            if (f > 0) P.rect(bx0, ry + 5.0f, bx0 + (bx1 - bx0) * f, ry + 7.2f, out ? IM_COL32(120, 112, 104, 255) : COL[r], 0.8f);
            if (cur > mx && mx > 0) P.text(P.fo, NsDraw::FS_SMALL, bx0 + P.width(P.fo, NsDraw::FS_SMALL, LBL[r]) + 0.8f, ry - 0.6f, GOLD, "+");
        }
        if (st == 5 || gm_member_elsewhere(m)) P.text(P.fo, NsDraw::FS_SMALL, 3.2f, y0 + 31.5f, GREY, st == 5 ? "away" : "elsewhere");
        draw_notes(P, m, 1.6f, y0 + 9.0f, 58.4f, y0 + 38.4f);              /* (a skill grown ... over the box's lower part) */
    }
    draw_notes(P, 5, 1.6f, 60.0f, 58.4f, 140.0f);
}
