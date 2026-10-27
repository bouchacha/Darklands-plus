// money.cpp - see money.h.
#include "imgui.h"
#include "imgui_internal.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <vector>
#include "money.h"
#include "../runtime/config.h"

bool money_coins(void) { return config_get_int("display.coins", 1) != 0; }

std::string money_fgp(long fl, long gr, long p)
{
    bool c = money_coins();
    std::string s;
    auto part = [&](long v, const char *coin, const char *abbr) {
        if (!s.empty()) s += " ";
        s += std::to_string(v);
        s += c ? coin : abbr;
    };
    if (fl) part(fl, COIN_FL, "fl");
    if (gr) part(gr, COIN_GR, "gr");
    if (p || s.empty()) part(p, COIN_PF, "pf");
    return s;
}

std::string money_str(long pf)
{
    if (pf < 0) return "-" + money_str(-pf);
    return money_fgp(pf / 240, pf % 240 / 12, pf % 12);
}

std::string money_with_total(long pf)
{
    std::string s = money_str(pf);
    if (pf >= 12) s += "  (" + std::to_string(pf) + (money_coins() ? COIN_PF ")" : " pf)");
    return s;
}

extern "C" void money_cstr(long pf, char *out, size_t n) { snprintf(out, n, "%s", money_str(pf).c_str()); }

std::string money_total(long pf)
{
    char b[128];
    if (money_coins()) snprintf(b, sizeof b, "%ld" COIN_PF " in all (1" COIN_FL " florin = 20" COIN_GR " groschen = 240" COIN_PF " pfennigs)", pf);   /* (the coins' names: here) */
    else snprintf(b, sizeof b, "%ld pfennig%s in all (1 florin = 20 groschen = 240 pfennigs)", pf, pf == 1 ? "" : "s");
    return b;
}

std::string money_words(const std::string &s)
{
    if (!money_coins() || s.find_first_of("0123456789") == std::string::npos) return s;
    static const struct { const char *w; const char *coin; bool word; } W[] = {
        { "florins", COIN_FL, true }, { "florin", COIN_FL, true }, { "groschen", COIN_GR, true }, { "pfennigs", COIN_PF, true },
        { "pfennig", COIN_PF, true }, { "pfs", COIN_PF, false }, { "fl", COIN_FL, false }, { "gr", COIN_GR, false }, { "pf", COIN_PF, false },
    };
    std::string out;
    size_t i = 0, n = s.size();
    while (i < n) {
        if (!isdigit((unsigned char)s[i]) || (i > 0 && isalnum((unsigned char)s[i - 1]))) { out += s[i++]; continue; }
        size_t j = i; while (j < n && isdigit((unsigned char)s[j])) j++;
        out.append(s, i, j - i);
        bool sp = j < n && s[j] == ' ';
        size_t k = sp ? j + 1 : j;
        i = j;
        for (const auto &w : W) {
            size_t l = strlen(w.w);
            if ((sp && !w.word) || k + l > n || strncasecmp(s.c_str() + k, w.w, l) || (k + l < n && isalpha((unsigned char)s[k + l]))) continue;
            out += w.coin; i = k + l;
            break;
        }
    }
    return out;
}

void money_tooltip(long pf)
{
    if (pf < 12 || !ImGui::IsItemHovered()) return;
    ImGui::SetTooltip("%s", money_total(pf).c_str());
}

/* ---------------------------------------------------------------- the coins in the fonts */
struct CoinRect { ImFont *font; int rect[3]; float d; };
static std::vector<CoinRect> coin_rects;

void money_glyphs_add(ImFontAtlas *atlas)
{
    coin_rects.clear();
    for (ImFont *f : atlas->Fonts) {
        if (!f || !f->ConfigData) continue;
        float S = f->ConfigData->SizePixels;
        CoinRect cr; cr.font = f; cr.d = roundf(S * 0.56f);
        if (cr.d < 6) cr.d = 6;
        int box = (int)cr.d + 2;
        for (int k = 0; k < 3; k++)
            cr.rect[k] = atlas->AddCustomRectFontGlyph(f, (ImWchar)(0xE000 + k), box, box, cr.d + S * 0.14f, ImVec2(S * 0.03f - 1, 0));
        coin_rects.push_back(cr);
    }
    for (CoinRect &cr : coin_rects)
        for (int k = 0; k < 3; k++) atlas->CustomRects[cr.rect[k]].GlyphColored = 1;   /* (drawn in their own colours, not the text's) */
}

/* a coin's colour at (nx, ny), in its radius from its middle: a disc lit from the upper left with a
   darker rim and (ring: big enough to show it) a raised inner ring */
static void coin_rgb(int kind, float nx, float ny, bool ring, float col[3])
{
    static const float BASE[3][3] = { { 226, 176, 46 }, { 192, 196, 204 }, { 190, 104, 56 } };
    static const float RIM[3][3] = { { 132, 88, 14 }, { 100, 104, 114 }, { 110, 52, 22 } };
    static const float HI[3][3] = { { 255, 238, 160 }, { 250, 251, 253 }, { 246, 182, 132 } };
    float t = sqrtf(nx * nx + ny * ny);                                      /* 0 centre .. 1 edge */
    float lit = 0.5f - (nx + ny) / 2.83f;                                    /* 1 upper left .. 0 lower right */
    for (int c = 0; c < 3; c++) {
        float v = BASE[kind][c] + (HI[kind][c] - BASE[kind][c]) * fmaxf(0, lit - 0.35f) * 1.2f
                                - (BASE[kind][c] - RIM[kind][c]) * fmaxf(0, 0.3f - lit) * 1.4f;
        float rim = fmaxf(0, (t - 0.78f) / 0.22f);                           /* the rim */
        v = v + (RIM[kind][c] - v) * fminf(1.0f, rim) * 0.85f;
        if (ring && fabsf(t - 0.62f) < 0.06f) v = v + (RIM[kind][c] - v) * 0.35f;   /* the raised ring */
        col[c] = v < 0 ? 0 : v > 255 ? 255 : v;
    }
}

/* one coin, d pixels across, into the RGBA atlas at (x0, y0) */
static void paint_coin(unsigned char *px, int pitch, int x0, int y0, int box, float d, int kind)
{
    float r = d * 0.5f, cx = x0 + 1 + r, cy = y0 + 1 + r;
    for (int y = y0; y < y0 + box; y++)
        for (int x = x0; x < x0 + box; x++) {
            float fx = x + 0.5f - cx, fy = y + 0.5f - cy, dist = sqrtf(fx * fx + fy * fy);
            float a = r + 0.5f - dist; a = a < 0 ? 0 : a > 1 ? 1 : a;          /* (anti-aliased edge) */
            unsigned char *p = px + (size_t)y * pitch + (size_t)x * 4;
            if (a <= 0) { p[0] = p[1] = p[2] = 255; p[3] = 0; continue; }
            float col[3]; coin_rgb(kind, fx / r, fy / r, d >= 12, col);
            p[0] = (unsigned char)col[0]; p[1] = (unsigned char)col[1]; p[2] = (unsigned char)col[2];
            p[3] = (unsigned char)(a * 255);
        }
}

/* the same coin drawn at any size (a picture made of coins, as an inn's wages): rings of vertices
   coloured as the glyph's pixels are, and a soft edge one pixel wide */
void money_coin_draw(ImDrawList *dl, ImVec2 c, float r, int kind)
{
    static const float T[] = { 0, 0.25f, 0.5f, 0.555f, 0.565f, 0.675f, 0.685f, 0.78f, 0.86f, 0.93f, 1.0f };
    const int NT = (int)(sizeof T / sizeof T[0]), N = 36;
    bool ring = r * 2 >= 12;
    float fringe = 1.0f / fmaxf(r, 1.0f);
    int nv = 1 + (NT - 1) * N + N, ni = (N * 3) + (NT - 2) * N * 6 + N * 6;
    dl->PrimReserve(ni, nv);
    ImVec2 uv = dl->_Data->TexUvWhitePixel;
    unsigned base = dl->_VtxCurrentIdx;
    auto vtx = [&](float nx, float ny, float a) {
        float col[3]; coin_rgb(kind < 0 ? 0 : kind > 2 ? 2 : kind, nx, ny, ring, col);
        dl->PrimWriteVtx(ImVec2(c.x + nx * r, c.y + ny * r), uv, IM_COL32((int)col[0], (int)col[1], (int)col[2], (int)(a * 255)));
    };
    vtx(0, 0, 1);
    for (int k = 1; k < NT; k++)
        for (int i = 0; i < N; i++) { float an = i * 6.2831853f / N; vtx(cosf(an) * T[k], sinf(an) * T[k], 1); }
    for (int i = 0; i < N; i++) { float an = i * 6.2831853f / N; vtx(cosf(an) * (1 + fringe), sinf(an) * (1 + fringe), 0); }
    auto ringv = [&](int k, int i) { return (ImDrawIdx)(base + 1 + (k - 1) * N + (i % N)); };
    for (int i = 0; i < N; i++) { dl->PrimWriteIdx((ImDrawIdx)base); dl->PrimWriteIdx(ringv(1, i)); dl->PrimWriteIdx(ringv(1, i + 1)); }
    for (int k = 1; k < NT; k++)                                            /* (the last: to the soft edge) */
        for (int i = 0; i < N; i++) {
            ImDrawIdx a = ringv(k, i), b = ringv(k, i + 1), c2 = ringv(k + 1, i), d = ringv(k + 1, i + 1);
            dl->PrimWriteIdx(a); dl->PrimWriteIdx(c2); dl->PrimWriteIdx(d);
            dl->PrimWriteIdx(a); dl->PrimWriteIdx(d); dl->PrimWriteIdx(b);
        }
}

void money_glyphs_paint(ImFontAtlas *atlas)
{
    unsigned char *px; int w, h;
    atlas->GetTexDataAsRGBA32(&px, &w, &h);
    for (CoinRect &cr : coin_rects) {
        float S = cr.font->FontSize;
        for (int k = 0; k < 3; k++) {
            const ImFontAtlasCustomRect *r = atlas->GetCustomRectByIndex(cr.rect[k]);
            if (!r->IsPacked()) continue;
            paint_coin(px, w * 4, r->X, r->Y, r->Width, cr.d, k);
            /* on the line: the coin's foot a little below the baseline, its middle near that of
               the small letters */
            ImFontGlyph *g = (ImFontGlyph *)cr.font->FindGlyphNoFallback((ImWchar)(0xE000 + k));
            if (g) { float hgt = g->Y1 - g->Y0; g->Y0 = cr.font->Ascent + S * 0.11f - hgt; g->Y1 = g->Y0 + hgt; }
            if (getenv("DL_COIN_LOG")) fprintf(stderr, "coin %d: font %.0f ascent %.1f glyph %p y %.1f..%.1f\n", k, S, cr.font->Ascent, (void *)g, g ? g->Y0 : 0, g ? g->Y1 : 0);
        }
    }
}
