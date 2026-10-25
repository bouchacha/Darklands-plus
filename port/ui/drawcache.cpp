/* drawcache.cpp - see drawcache.h */
#include "drawcache.h"
#include "imgui_internal.h"
#include "backends/imgui_impl_sdlrenderer2.h"
#include <SDL.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <vector>
extern "C" int config_get_int(const char *key, int def);
extern "C" SDL_Renderer *plat_renderer(void);

struct DcEnt { unsigned long long key; int w, h; SDL_Texture *t; int used; };
static std::vector<DcEnt> cache;
static int frame_no, usable = -1;
/* the way of keeping a drawing used by default (display.draw_cache 1 and 3): its triangles, made once,
   copied into the frame each time - the same pixels, none of the work of making them, no picture on the
   graphics card. A picture drawn into (2) showed as a wash over the inn, the camp and the margins on a
   player's machines, on Direct3D 11 first and then on a laptop: pictures only when asked for. Each of
   the drawing's commands keeps its own clip rectangle and texture */
struct DcGeo { unsigned long long key; std::vector<ImDrawVert> v; std::vector<ImDrawIdx> i; std::vector<int> cmd_n, cmd_vlo, cmd_vhi; std::vector<ImVec4> cmd_clip; std::vector<ImTextureID> cmd_tex; ImTextureID tex; int used; };
static std::vector<DcGeo> geos;
static int mode = -1;                          /* 0 none, 2 pictures, 3 triangles */

void dc_reset(void)
{
    for (DcEnt &e : cache) if (e.t) SDL_DestroyTexture(e.t);
    cache.clear();
}
static void geo_draw(ImDrawList *dl, ImVec2 lo, ImVec2 hi, unsigned long long key, const std::function<void(ImDrawList *)> &fn)
{
    DcGeo *g = nullptr;
    for (DcGeo &c : geos) if (c.key == key) { g = &c; break; }
    if (!g) {
        if (geos.size() >= 192) { dl->PushClipRect(lo, hi, true); fn(dl); dl->PopClipRect(); return; }
        ImDrawList tmp(ImGui::GetDrawListSharedData());
        tmp._ResetForNewFrame();
        tmp.PushClipRect(lo, hi);
        tmp.PushTextureID(ImGui::GetIO().Fonts->TexID);
        fn(&tmp);
        DcGeo n; n.key = key; n.tex = ImGui::GetIO().Fonts->TexID; n.used = frame_no;
        n.v.assign(tmp.VtxBuffer.Data, tmp.VtxBuffer.Data + tmp.VtxBuffer.Size);
        for (const ImDrawCmd &cmd : tmp.CmdBuffer) {
            if (cmd.UserCallback || cmd.ElemCount == 0) continue;
            int vlo = 1 << 30, vhi = -1;
            for (unsigned k = 0; k < cmd.ElemCount; k++) { int vi = (int)cmd.VtxOffset + tmp.IdxBuffer[cmd.IdxOffset + k]; vlo = vi < vlo ? vi : vlo; vhi = vi > vhi ? vi : vhi; }
            for (unsigned k = 0; k < cmd.ElemCount; k++) n.i.push_back((ImDrawIdx)((int)cmd.VtxOffset + tmp.IdxBuffer[cmd.IdxOffset + k] - vlo));
            n.cmd_n.push_back((int)cmd.ElemCount); n.cmd_vlo.push_back(vlo); n.cmd_vhi.push_back(vhi);
            n.cmd_clip.push_back(cmd.ClipRect); n.cmd_tex.push_back(cmd.TextureId);
        }
        geos.push_back(std::move(n));
        g = &geos.back();
    }
    g->used = frame_no;
    size_t ip = 0;
    for (size_t c = 0; c < g->cmd_n.size(); c++) {             /* (each of its commands: its own run of vertices, clip rectangle and texture) */
        int nv = g->cmd_vhi[c] - g->cmd_vlo[c] + 1, ni = g->cmd_n[c];
        const ImVec4 &cr = g->cmd_clip[c];
        dl->PushClipRect(ImVec2(fmaxf(cr.x, lo.x), fmaxf(cr.y, lo.y)), ImVec2(fminf(cr.z, hi.x), fminf(cr.w, hi.y)), true);
        dl->PushTextureID(g->cmd_tex[c]);
        dl->PrimReserve(ni, nv);
        ImDrawIdx base = (ImDrawIdx)dl->_VtxCurrentIdx;
        memcpy(dl->_VtxWritePtr, &g->v[g->cmd_vlo[c]], nv * sizeof(ImDrawVert));
        for (int k = 0; k < ni; k++) dl->_IdxWritePtr[k] = (ImDrawIdx)(base + g->i[ip + k]);
        dl->_VtxWritePtr += nv; dl->_IdxWritePtr += ni; dl->_VtxCurrentIdx += nv;
        ip += ni;
        dl->PopTextureID();
        dl->PopClipRect();
    }
}
/* (after the frame is built: pictures not used for a few seconds are let go; never during the
   frame, when the draw lists may still point at them) */
void dc_frame_end(void)
{
    frame_no++;
    int made_now = 0;
    for (size_t i = 0; i < cache.size();)
        if (frame_no - cache[i].used > 120 || (cache.size() > 160 && frame_no - cache[i].used > 2)) { SDL_DestroyTexture(cache[i].t); cache[i] = cache.back(); cache.pop_back(); }
        else i++;
    for (size_t i = 0; i < geos.size();)
        if (frame_no - geos[i].used > 120 || (geos.size() > 160 && frame_no - geos[i].used > 2)) { geos[i] = std::move(geos.back()); geos.pop_back(); }
        else i++;
    (void)made_now;
}

void dc_draw(ImDrawList *dl, ImVec2 lo, ImVec2 hi, unsigned long long key, const std::function<void(ImDrawList *)> &fn)
{
    SDL_Renderer *r = plat_renderer();
    if (mode < 0) {                            /* display.draw_cache: 1 (the default) and 3 triangles; 0 off; 2 pictures */
        SDL_RendererInfo ri; int have = r && SDL_GetRendererInfo(r, &ri) == 0;
        int want = getenv("DL_NO_DRAWCACHE") ? 0 : getenv("DL_DRAWCACHE") ? atoi(getenv("DL_DRAWCACHE")) : config_get_int("display.draw_cache", 1);
        if (want == 1) want = 3;
        if (want == 2 && !(have && (ri.flags & SDL_RENDERER_TARGETTEXTURE))) want = 3;
        mode = want;
        fprintf(stderr, "drawcache: %s\n", mode == 0 ? "off" : mode == 2 ? "pictures" : "triangles");
    }
    if (mode == 0) { dl->PushClipRect(lo, hi, true); fn(dl); dl->PopClipRect(); return; }
    if (mode == 3) { geo_draw(dl, lo, hi, dc_key(dc_key(dc_key(key, lo.x), lo.y), hi.x * 7 + hi.y * 13), fn); return; }
    if (usable < 0) usable = 1;
    float x0 = floorf(lo.x), y0 = floorf(lo.y);
    int w = (int)ceilf(hi.x) - (int)x0, h = (int)ceilf(hi.y) - (int)y0;
    auto direct = [&]() { dl->PushClipRect(ImVec2(x0, y0), ImVec2(x0 + w, y0 + h), true); fn(dl); dl->PopClipRect(); };   /* (drawn as it is, kept to its box) */
    if (!usable || w <= 0 || h <= 0 || w > 4096 || h > 4096) { direct(); return; }
    key = dc_key(dc_key(dc_key(key, w), h * 7919.0), (lo.x - x0) * 31 + (lo.y - y0) * 977);
    DcEnt *e = nullptr;
    for (DcEnt &c : cache) if (c.key == key && c.w == w && c.h == h) { e = &c; break; }
    if (!e) {
        /* a picture whose size changes from frame to frame (a tooltip's frame growing) would make a new one each
           frame: at most a few new ones a frame, and not past the cap; else drawn as it is */
        static int made_frame = -1, made_n = 0;
        if (made_frame != frame_no) { made_frame = frame_no; made_n = 0; }
        if (cache.size() >= 192 || made_n >= 8) { direct(); return; }
        SDL_Texture *t = SDL_CreateTexture(r, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, w, h);
        if (!t) { direct(); return; }
        made_n++;
        SDL_SetTextureBlendMode(t, SDL_BLENDMODE_BLEND);
        ImDrawList tmp(ImGui::GetDrawListSharedData());
        tmp._ResetForNewFrame();
        tmp.PushClipRect(ImVec2(x0, y0), ImVec2(x0 + w, y0 + h));
        tmp.PushTextureID(ImGui::GetIO().Fonts->TexID);
        fn(&tmp);
        for (ImDrawVert &v : tmp.VtxBuffer) { v.pos.x -= x0; v.pos.y -= y0; }   /* (the backend draws from 0,0; the clip rects it moves itself) */
        ImDrawData dd;
        dd.Valid = true; dd.CmdLists.push_back(&tmp); dd.CmdListsCount = 1;
        dd.TotalVtxCount = tmp.VtxBuffer.Size; dd.TotalIdxCount = tmp.IdxBuffer.Size;
        dd.DisplayPos = ImVec2(x0, y0); dd.DisplaySize = ImVec2((float)w, (float)h); dd.FramebufferScale = ImVec2(1, 1);
        SDL_Texture *old = SDL_GetRenderTarget(r);
        float sx, sy; SDL_RenderGetScale(r, &sx, &sy);
        if (SDL_SetRenderTarget(r, t) != 0) {                   /* (the renderer cannot draw into it after all: never again) */
            SDL_DestroyTexture(t); usable = 0; fprintf(stderr, "drawcache: cannot draw into a picture (%s): drawn directly\n", SDL_GetError());
            direct(); return;
        }
        SDL_RenderSetScale(r, 1, 1);
        SDL_SetRenderDrawColor(r, 0, 0, 0, 0); SDL_RenderClear(r);
        ImGui_ImplSDLRenderer2_RenderDrawData(&dd, r);
        SDL_SetRenderTarget(r, old);
        SDL_RenderSetScale(r, sx, sy);
        cache.push_back(DcEnt{ key, w, h, t, frame_no });
        e = &cache.back();
    }
    e->used = frame_no;
    dl->AddImage((ImTextureID)(intptr_t)e->t, ImVec2(x0, y0), ImVec2(x0 + w, y0 + h));
}
