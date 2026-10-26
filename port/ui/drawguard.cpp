// drawguard.cpp - the last look at a frame before it goes to the graphics card: a triangle no screen of
// Darklands+ means to draw is taken out, and the first few of them are said in the log ("draw guard: ...").
//
// Why: on players' Windows machines the inn was washed over in the colour of the screens' text: letters
// drawn hundreds of times their size (their soft edges magnified). The cause was a skill icon's shield
// (ui/skillicons.cpp shield) writing one corner past its list, onto where Windows keeps the caller's
// registers: the inn's row positions and text sizes after a teacher's icon became whatever the corner
// was. Linux keeps nothing there, so it never showed here. A wash like that, or a triangle whose
// corners lie absurdly far away (graphics cards clip those to the window and fill it), is what a slip
// of that kind looks like on screen, so this guard stays: two kinds are taken out:
//   - a corner not a number, or farther than 100 000 pixels: nothing on screen is so;
//   - a letter (a triangle textured from the font atlas, not its white pixel) drawn more than 24 times
//     the size it was made at: the largest text of the screens is some 3 times.
// The check costs a pass over the vertices; only a draw list reaching past the window is looked at
// triangle by triangle (a scrolled list, the world map zoomed in).
#include "imgui.h"
#include "imgui_internal.h"
#include <math.h>
#include <stdio.h>
#include <initializer_list>
#include "../mods/nscreen.h"

static void say(const ImDrawList *L, int ci, const char *what, const ImDrawVert &a, const ImDrawVert &b, const ImDrawVert &c, int dropped)
{
    static int said; static float last[3];                 /* (each new one once: the same one comes every frame) */
    if (said >= 12 || (a.pos.x == last[0] && a.pos.y == last[1] && (float)ci == last[2])) return;
    said++; last[0] = a.pos.x; last[1] = a.pos.y; last[2] = (float)ci;
    ImU32 col = a.col;
    fprintf(stderr, "draw guard: %s in %s (command %d; %d triangles taken out; screen %d): corners %.0f,%.0f %.0f,%.0f %.0f,%.0f, colour %d,%d,%d,%d\n",
            what, L->_OwnerName ? L->_OwnerName : "?", ci, dropped, ns_screen, a.pos.x, a.pos.y, b.pos.x, b.pos.y, c.pos.x, c.pos.y,
            (int)(col & 255), (int)(col >> 8 & 255), (int)(col >> 16 & 255), (int)(col >> 24 & 255));
}

extern "C" void ui_draw_guard(ImDrawData *dd)
{
    if (!dd || !dd->Valid) return;
    ImVec2 ds = ImGui::GetIO().DisplaySize;
    ImFontAtlas *atlas = ImGui::GetIO().Fonts;
    ImTextureID font = atlas->TexID;
    ImVec2 white = atlas->TexUvWhitePixel;
    float tw = (float)atlas->TexWidth, th = (float)atlas->TexHeight;
    const float FAR = 100000.0f, MAGNIFIED = 24.0f;
    for (int li = 0; li < dd->CmdListsCount; li++) {
        ImDrawList *L = dd->CmdLists[li];
        /* (the fast way: every corner inside the window and its surroundings, none not a number) */
        bool look = false;
        for (const ImDrawVert &v : L->VtxBuffer)
            if (!(v.pos.x >= -ds.x && v.pos.x <= 2 * ds.x && v.pos.y >= -ds.y && v.pos.y <= 2 * ds.y)) { look = true; break; }
        bool fonts = false;                         /* (a letter far bigger than made: only text can be, so only its commands) */
        for (const ImDrawCmd &c : L->CmdBuffer) if (c.GetTexID() == font && c.ElemCount) { fonts = true; break; }
        if (!look && !fonts) continue;
        for (int ci = 0; ci < L->CmdBuffer.Size; ci++) {
            ImDrawCmd &c = L->CmdBuffer[ci];
            if (c.UserCallback || !c.ElemCount) continue;
            bool is_font = c.GetTexID() == font;
            if (!look && !is_font) continue;
            ImDrawIdx *idx = L->IdxBuffer.Data + c.IdxOffset;
            const ImDrawVert *vtx = L->VtxBuffer.Data + c.VtxOffset;
            int dropped = 0; const char *first = nullptr; int fa = 0, fb = 0, fc = 0;
            for (unsigned k = 0; k + 2 < c.ElemCount; k += 3) {
                const ImDrawVert &a = vtx[idx[k]], &b = vtx[idx[k + 1]], &cc = vtx[idx[k + 2]];
                const char *what = nullptr;
                if (look) {
                    for (const ImDrawVert *v : { &a, &b, &cc })
                        if (!(fabsf(v->pos.x) <= FAR && fabsf(v->pos.y) <= FAR)) { what = v->pos.x == v->pos.x && v->pos.y == v->pos.y ? "a corner absurdly far" : "a corner not a number"; break; }
                }
                if (!what && is_font && !(a.uv.x == white.x && a.uv.y == white.y)) {   /* (most triangles: a shape, from the atlas's white pixel) */
                    float uw = (fmaxf(fmaxf(a.uv.x, b.uv.x), cc.uv.x) - fminf(fminf(a.uv.x, b.uv.x), cc.uv.x)) * tw;
                    float uh = (fmaxf(fmaxf(a.uv.y, b.uv.y), cc.uv.y) - fminf(fminf(a.uv.y, b.uv.y), cc.uv.y)) * th;
                    if (uw > 1.0f && uh > 1.0f) {               /* (a letter: its picture in the atlas both ways) */
                        float sw = fmaxf(fmaxf(a.pos.x, b.pos.x), cc.pos.x) - fminf(fminf(a.pos.x, b.pos.x), cc.pos.x);
                        float sh = fmaxf(fmaxf(a.pos.y, b.pos.y), cc.pos.y) - fminf(fminf(a.pos.y, b.pos.y), cc.pos.y);
                        if (sw > uw * MAGNIFIED && sh > uh * MAGNIFIED) what = "a letter drawn many times its size";
                    }
                }
                if (!what) continue;
                if (!first) { first = what; fa = idx[k]; fb = idx[k + 1]; fc = idx[k + 2]; }
                idx[k + 1] = idx[k + 2] = idx[k];          /* (taken out: a triangle of no area) */
                dropped++;
            }
            if (first) say(L, ci, first, vtx[fa], vtx[fb], vtx[fc], dropped);
        }
    }
}

/* with a screenshot (Ctrl+F12): the frame's every draw - its list, texture, colour and where it reaches -
   written beside the picture (name.txt), so that a wrong picture on a player's machine can be read back
   here draw by draw */
extern "C" void ui_draw_dump(const char *path)
{
    ImDrawData *dd = ImGui::GetDrawData();
    FILE *f = dd && dd->Valid ? fopen(path, "w") : nullptr;
    if (!f) return;
    ImFontAtlas *atlas = ImGui::GetIO().Fonts;
    ImVec2 ds = ImGui::GetIO().DisplaySize;
    fprintf(f, "the frame's draws: window %.0fx%.0f, %d lists, %d vertices, font atlas %dx%d (texture %p), screen %d\n",
            ds.x, ds.y, dd->CmdListsCount, dd->TotalVtxCount, atlas->TexWidth, atlas->TexHeight, (void *)atlas->TexID, ns_screen);
    fprintf(f, "list command: texture, triangles, clip; corners from-to; uv from-to (font: in atlas pixels); colour of the first corner\n");
    for (int li = 0; li < dd->CmdListsCount; li++) {
        const ImDrawList *L = dd->CmdLists[li];
        for (int ci = 0; ci < L->CmdBuffer.Size; ci++) {
            const ImDrawCmd &c = L->CmdBuffer[ci];
            if (c.UserCallback || !c.ElemCount) continue;
            const ImDrawVert *vtx = L->VtxBuffer.Data + c.VtxOffset;
            const ImDrawIdx *idx = L->IdxBuffer.Data + c.IdxOffset;
            float x0 = 1e30f, y0 = 1e30f, x1 = -1e30f, y1 = -1e30f, u0 = 1e30f, v0 = 1e30f, u1 = -1e30f, v1 = -1e30f;
            for (unsigned k = 0; k < c.ElemCount; k++) {
                const ImDrawVert &v = vtx[idx[k]];
                x0 = fminf(x0, v.pos.x); y0 = fminf(y0, v.pos.y); x1 = fmaxf(x1, v.pos.x); y1 = fmaxf(y1, v.pos.y);
                u0 = fminf(u0, v.uv.x); v0 = fminf(v0, v.uv.y); u1 = fmaxf(u1, v.uv.x); v1 = fmaxf(v1, v.uv.y);
            }
            bool font = c.GetTexID() == atlas->TexID;
            float kx = font ? (float)atlas->TexWidth : 1, ky = font ? (float)atlas->TexHeight : 1;
            ImU32 col = vtx[idx[0]].col;
            fprintf(f, "%s %d: %s%p, %u, clip %.0f,%.0f-%.0f,%.0f; %.0f,%.0f-%.0f,%.0f; uv %.3g,%.3g-%.3g,%.3g; %d,%d,%d,%d\n",
                    L->_OwnerName ? L->_OwnerName : "?", ci, font ? "font " : "", (void *)c.GetTexID(), c.ElemCount / 3,
                    c.ClipRect.x, c.ClipRect.y, c.ClipRect.z, c.ClipRect.w, x0, y0, x1, y1, u0 * kx, v0 * ky, u1 * kx, v1 * ky,
                    (int)(col & 255), (int)(col >> 8 & 255), (int)(col >> 16 & 255), (int)(col >> 24 & 255));
        }
    }
    fclose(f);
}
