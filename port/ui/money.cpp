// money.cpp - see money.h.
#include "imgui.h"
#include "imgui_internal.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
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
    if (pf >= 12) s += "  (" + std::to_string(pf) + " pf)";
    return s;
}

std::string money_total(long pf)
{
    char b[64]; snprintf(b, sizeof b, "%ld pfennig%s in all (1 florin = 20 groschen = 240 pfennigs)", pf, pf == 1 ? "" : "s");
    return b;
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

/* one coin, d pixels across, into the RGBA atlas at (x0, y0): a disc lit from the upper left
   with a darker rim and a raised inner ring */
static void paint_coin(unsigned char *px, int pitch, int x0, int y0, int box, float d, int kind)
{
    static const float BASE[3][3] = { { 226, 176, 46 }, { 192, 196, 204 }, { 190, 104, 56 } };
    static const float RIM[3][3] = { { 132, 88, 14 }, { 100, 104, 114 }, { 110, 52, 22 } };
    static const float HI[3][3] = { { 255, 238, 160 }, { 250, 251, 253 }, { 246, 182, 132 } };
    float r = d * 0.5f, cx = x0 + 1 + r, cy = y0 + 1 + r;
    for (int y = y0; y < y0 + box; y++)
        for (int x = x0; x < x0 + box; x++) {
            float fx = x + 0.5f - cx, fy = y + 0.5f - cy, dist = sqrtf(fx * fx + fy * fy);
            float a = r + 0.5f - dist; a = a < 0 ? 0 : a > 1 ? 1 : a;          /* (anti-aliased edge) */
            unsigned char *p = px + (size_t)y * pitch + (size_t)x * 4;
            if (a <= 0) { p[0] = p[1] = p[2] = 255; p[3] = 0; continue; }
            float t = dist / r;                                                  /* 0 centre .. 1 edge */
            float lit = 0.5f - (fx + fy) / (2.83f * r);                          /* 1 upper left .. 0 lower right */
            float col[3];
            for (int c = 0; c < 3; c++) {
                float v = BASE[kind][c] + (HI[kind][c] - BASE[kind][c]) * fmaxf(0, lit - 0.35f) * 1.2f
                                        - (BASE[kind][c] - RIM[kind][c]) * fmaxf(0, 0.3f - lit) * 1.4f;
                float rim = fmaxf(0, (t - 0.78f) / 0.22f);                       /* the rim */
                v = v + (RIM[kind][c] - v) * rim * 0.85f;
                float ring = fabsf(t - 0.62f) < 0.06f ? 1.0f : 0.0f;              /* the raised ring */
                if (d >= 12) v = v + (RIM[kind][c] - v) * ring * 0.35f;
                col[c] = v < 0 ? 0 : v > 255 ? 255 : v;
            }
            p[0] = (unsigned char)col[0]; p[1] = (unsigned char)col[1]; p[2] = (unsigned char)col[2];
            p[3] = (unsigned char)(a * 255);
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
