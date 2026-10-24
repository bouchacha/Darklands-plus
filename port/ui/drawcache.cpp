/* drawcache.cpp - see drawcache.h */
#include "drawcache.h"
#include "imgui_internal.h"
#include "backends/imgui_impl_sdlrenderer2.h"
#include <SDL.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
extern "C" SDL_Renderer *plat_renderer(void);

struct DcEnt { unsigned long long key; int w, h; SDL_Texture *t; int used; };
static std::vector<DcEnt> cache;
static int frame_no, usable = -1;

void dc_reset(void)
{
    for (DcEnt &e : cache) if (e.t) SDL_DestroyTexture(e.t);
    cache.clear();
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
    (void)made_now;
}

void dc_draw(ImDrawList *dl, ImVec2 lo, ImVec2 hi, unsigned long long key, const std::function<void(ImDrawList *)> &fn)
{
    SDL_Renderer *r = plat_renderer();
    if (usable < 0) {
        SDL_RendererInfo ri;
        usable = r && !getenv("DL_NO_DRAWCACHE") && SDL_GetRendererInfo(r, &ri) == 0 && (ri.flags & SDL_RENDERER_TARGETTEXTURE);
    }
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
