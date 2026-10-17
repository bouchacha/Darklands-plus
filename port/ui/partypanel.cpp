// partypanel.cpp - the party panel drawn by Darklands+ (display.native_panel; "Play the original
// game" keeps the game's). Wherever the game shows its panel (the five boxes left of the
// picture), each member's box is drawn anew over it: the name (gold for the leader, grey for
// one out of action), a picture, and Endurance, Strength and Divine Favor as labelled bars with
// their numbers; in a battle the order being carried out. The picture is the member's own
// portrait if there is one (portraits\<name>.png or .jpg in the Darklands+ folder), else the
// figure the game shows in battles (once a battle has shown it), or the herald
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
#ifdef _WIN32
#include <direct.h>
#define mkdir(p) _mkdir(p)
#endif
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
#include "../third_party/stb/stb_image_write.h"
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

/* a member's figure as the game shows it in battles (PICS\<code>STAT.PIC, its clothes in the
 * colours chosen for the member, which the game sets only there): taken from the game's own
 * panel in a battle, where the PIC's shape lies at (6, 40 m + 9), and kept for the other screens */
struct Figure { SDL_Texture *tex = nullptr; int w = 0, h = 0; std::string code; bool looked = false; };
static std::map<std::string, Figure> figures;          /* by full name */
/* kept between sessions in the Darklands+ folder: cache/figures/<full name>-<code>.png */
static std::string figure_file(const std::string &who, const char *code)
{
    std::string n;
    for (char c : who) n += (isalnum((unsigned char)c) || c == ' ' || c == '-') ? c : '_';
    return user_folder() + "/cache/figures/" + n + "-" + code + ".png";
}
static void figure_load(const std::string &who, int m)
{
    Figure &f = figures[who];
    if (f.tex || f.looked || !pp_ren) return;
    f.looked = true;
    char code[8]; gm_member_herald(m, code, sizeof code);
    if (!code[0]) return;
    int w, h, n; unsigned char *px = stbi_load(figure_file(who, code).c_str(), &w, &h, &n, 4);
    if (!px) return;
    int dark = 0, all = 0;   /* (one taken black by an earlier version is left for a new one) */
    for (int i = 0; i < w * h; i++) {
        const unsigned char *c = px + 4 * i;
        if (!c[3]) continue;
        all++; if (c[0] + c[1] + c[2] < 90) dark++;
    }
    if (!all || dark * 10 > all * 9) { stbi_image_free(px); return; }
    f.tex = SDL_CreateTexture(pp_ren, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STATIC, w, h);
    if (f.tex) { SDL_UpdateTexture(f.tex, 0, px, w * 4); SDL_SetTextureBlendMode(f.tex, SDL_BLENDMODE_BLEND); SDL_SetTextureScaleMode(f.tex, SDL_ScaleModeNearest); f.w = w; f.h = h; f.code = code; }
    stbi_image_free(px);
}
static void figure_save(const std::string &who, const char *code, const std::vector<u32> &argb, int w, int h)
{
    std::string dir = user_folder() + "/cache";
#ifdef _WIN32
    mkdir(dir.c_str()); mkdir((dir + "/figures").c_str());
#else
    mkdir(dir.c_str(), 0755); mkdir((dir + "/figures").c_str(), 0755);
#endif
    std::vector<unsigned char> rgba((size_t)w * h * 4);
    for (size_t i = 0; i < (size_t)w * h; i++) { u32 c = argb[i]; rgba[4 * i] = (u8)(c >> 16); rgba[4 * i + 1] = (u8)(c >> 8); rgba[4 * i + 2] = (u8)c; rgba[4 * i + 3] = (u8)(c >> 24); }
    stbi_write_png(figure_file(who, code).c_str(), w, h, 4, rgba.data(), w * 4);
}
static void take_figure(int m, const std::string &who)
{
    char code[8]; gm_member_herald(m, code, sizeof code);
    if (!code[0] || !pp_ren) return;
    Figure &f = figures[who];
    if (f.tex && f.code == code) return;
    static u32 pal[256]; static u8 mask[256];
    int w = 0, h = 0;
    {   /* not while the game's panel still shows the herald there (the start of a battle) */
        char fig = gm_member_figure(m), fn[32];
        snprintf(fn, sizeof fn, "PICS\\SHIELD%c.PIC", fig ? fig : 'a');
        u8 *her = hdart_decode_pic(fn, &w, &h, pal, mask);
        if (her) {
            int same = 0, all = 0;
            for (int y = 0; y < h; y++) for (int x = 0; x < w; x++) {
                if (!her[y * w + x] || 6 + x >= 60) continue;
                all++; if (rb(0xa000, (u16)((40 * m + 9 + y) * 320 + 6 + x)) == her[y * w + x]) same++;
            }
            free(her);
            if (all && same * 10 >= all * 6) return;
        }
    }
    u8 *img = hdart_decode_pic((std::string("PICS\\") + code + "SHORT.PIC").c_str(), &w, &h, pal, mask);
    if (!img) return;
    std::vector<u32> rgba((size_t)w * h, 0);
    for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++) {
            if (!img[y * w + x]) continue;
            int vx = 6 + x, vy = 40 * m + 9 + y;
            if (vx >= 60 || vy >= 200) continue;
            const u8 *c = vga_dac + rb(0xa000, (u16)(vy * 320 + vx)) * 3;
            u32 r = c[0] & 63, g = c[1] & 63, b = c[2] & 63;
            rgba[(size_t)y * w + x] = 0xff000000u | ((r << 2 | r >> 4) << 16) | ((g << 2 | g >> 4) << 8) | (b << 2 | b >> 4);
        }
    free(img);
    /* only a figure that has stayed the same for a while and is not dark (a battle's palette is
       still being set: the figures came out black under Wine) */
    int dark = 0, all = 0; u32 hash = 2166136261u;
    for (u32 c : rgba) {
        hash = (hash ^ c) * 16777619u;
        if (!c) continue;
        all++; if (((c >> 16) & 255) + ((c >> 8) & 255) + (c & 255) < 90) dark++;
    }
    struct Pend { u32 hash, t; };
    static std::map<std::string, Pend> pend;
    u32 now = SDL_GetTicks();
    Pend &pd = pend[who];
    if (pd.hash != hash) { pd.hash = hash; pd.t = now; return; }
    if (now - pd.t < 700 || !all || dark * 10 > all * 9) return;
    if (f.tex) SDL_DestroyTexture(f.tex);
    f = Figure(); f.code = code; f.w = w; f.h = h;
    f.tex = SDL_CreateTexture(pp_ren, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, w, h);
    if (f.tex) { SDL_UpdateTexture(f.tex, 0, rgba.data(), w * 4); SDL_SetTextureBlendMode(f.tex, SDL_BLENDMODE_BLEND); SDL_SetTextureScaleMode(f.tex, SDL_ScaleModeNearest); }
    f.looked = true;
    figure_save(who, code, rgba, w, h);
}

static const char *order_word(char c)
{
    switch (c) {
    case 'A': return "attacks"; case 'V': return "attacks (vulnerable)"; case 'B': return "berserk"; case 'P': return "parries";
    case 'M': return "shoots"; case 'W': return "walks"; case 'F': return "follows"; case '-': return "waits";
    }
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
    P.mx = ns_in.x; P.my = ns_in.y;
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
        /* the order (in a battle), right of the name */
        char oc = battle ? gm_member_order(m) : 0;
        float name_end = 56.5f;
        if (oc) {
            char b[2] = { oc, 0 };
            float bw = 6.0f;
            P.rect(56.8f - bw, y0 + 2.6f, 56.8f, y0 + 8.6f, IM_COL32(86, 60, 30, 255), 1.2f);
            P.text(P.fo, NsDraw::FS_SMALL, 0, y0 + 2.9f, CREAM, b, 56.8f - bw * 0.5f + P.width(P.fo, NsDraw::FS_SMALL, b) * 0.5f);
            name_end = 56.8f - bw - 1;
            if (P.over(56.8f - bw, y0 + 2.6f, 56.8f, y0 + 8.6f) && order_word(oc)) ImGui::SetTooltip("%s %s", name.c_str(), order_word(oc));
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
        char full[40]; gm_member_fullname(m, full, sizeof full);
        if (battle && st == 1 && !gm_member_elsewhere(m)) take_figure(m, full);
        else figure_load(full, m);
        if (!drawn && pp_pic != 3) {
            SDL_Texture *t = nullptr; int tw = 0, th = 0;
            auto fi = figures.find(full);
            if (pp_pic != 2 && fi != figures.end() && fi->second.tex) { t = fi->second.tex; tw = fi->second.w; th = fi->second.h; }
            char fig = gm_member_figure(m);                          /* the herald: PICS\SHIELD<letter>.PIC */
            if (!t && fig) { char fn[32]; snprintf(fn, sizeof fn, "PICS\\SHIELD%c.PIC", fig); const GamePic *g = gamepic(fn); if (g) { t = g->tex; tw = g->w; th = g->h; } }
            if (t) {
                float s = fminf((px1 - px0 - 1.5f) / tw, (py1 - py0 - 1.5f) / th);
                float cx = (px0 + px1) * 0.5f, cy = (py0 + py1) * 0.5f, hw = tw * s * 0.5f, hh = th * s * 0.5f;
                P.dl->AddImage((ImTextureID)(intptr_t)t, NsDraw::w(cx - hw, cy - hh), NsDraw::w(cx + hw, cy + hh));
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
    }
}
