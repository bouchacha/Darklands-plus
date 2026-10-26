// panelcheck.cpp - the checks, every frame, that no two of Darklands+'s panels lie over each other, and
// that the game's hand never shows over a Darklands+ window (two pointers). Panels (the
// world map's finders over its legend, a battle's tip over the order strip ...): each pair found goes
// to the log once, "panels overlap: A and B", and tools/uitour.py fails on it. A panel counts by what
// it shows (its content), not its whole window: a margin is a column of the screen's height with a
// few lines at its top.
//
// Left out: what stands at the back on purpose (the world map and the level map, each the window the
// others float on; the overlay layer over the game's picture), and what is meant to cover the rest
// while it is up (F12's window, the welcome and the first-start windows, an alert, a menu or modal).
#include "imgui.h"
#include "imgui_internal.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <SDL.h>

static bool named(const ImGuiWindow *w, const char *const *list)
{
    for (; *list; list++) if (!strncmp(w->Name, *list, strlen(*list))) return true;
    return false;
}

static const char *const BACK[] = { "World map", "Level map", "##overlay_layer", nullptr };
static const char *const OVER[] = { "Darklands+", "Welcome to Darklands+", "Saving options", "Keys and clicks", "Darklands sounds best", "##alert", "Debug##Default", nullptr };

static ImRect shown(const ImGuiWindow *w)
{
    ImVec2 pad = w->WindowPadding, sz(fminf(w->Size.x, w->ContentSize.x + pad.x * 2), fminf(w->Size.y, w->ContentSize.y + pad.y * 2 + w->TitleBarHeight));
    return ImRect(w->Pos, ImVec2(w->Pos.x + sz.x, w->Pos.y + sz.y));
}

void ui_panel_audit(void)
{
    static ImGuiID seen[64]; static int nseen = 0;
    ImGuiContext &g = *ImGui::GetCurrentContext();
    ImVector<ImGuiWindow *> up;
    for (ImGuiWindow *w : g.Windows) {
        if (!w->Active || w->Hidden || w->ParentWindow || (w->Flags & (ImGuiWindowFlags_Tooltip | ImGuiWindowFlags_Popup | ImGuiWindowFlags_ChildWindow | ImGuiWindowFlags_ChildMenu))) continue;
        if (named(w, OVER)) return;                              /* (a window over everything: the rest is under it on purpose) */
        if (named(w, BACK)) continue;
        up.push_back(w);
    }
    for (int i = 0; i < up.Size; i++) for (int j = i + 1; j < up.Size; j++) {
        ImRect a = shown(up[i]), b = shown(up[j]);
        float ox = fminf(a.Max.x, b.Max.x) - fmaxf(a.Min.x, b.Min.x), oy = fminf(a.Max.y, b.Max.y) - fmaxf(a.Min.y, b.Min.y);
        if (ox < 4 || oy < 4) continue;                          /* (frames touching are fine) */
        ImGuiID pair = up[i]->ID ^ (up[j]->ID * 2654435761u);
        bool known = false;
        for (int k = 0; k < nseen; k++) known = known || seen[k] == pair;
        if (known || nseen >= 64) continue;
        seen[nseen++] = pair;
        fprintf(stderr, "panels overlap: \"%s\" %.0f,%.0f-%.0f,%.0f and \"%s\" %.0f,%.0f-%.0f,%.0f\n", up[i]->Name, a.Min.x, a.Min.y, a.Max.x, a.Max.y,
                up[j]->Name, b.Min.x, b.Min.y, b.Max.x, b.Max.y);
    }
}

/* and one pointer at a time: the game's hand never over a window of Darklands+, where the system's
   arrow shows instead (ui/ui.cpp ui_cursor, windows_up): "the game's hand over a window: NAME" in the
   log, each window once - a window left out of windows_up shows two pointers */
void ui_pointer_audit(void)
{
    static ImGuiID seen[32]; static int nseen = 0;
    ImGuiContext &g = *ImGui::GetCurrentContext();
    ImGuiWindow *w = g.HoveredWindow;
    if (!w || SDL_ShowCursor(SDL_QUERY) == SDL_ENABLE || nseen >= 32) return;
    w = w->RootWindow;
    if (w->Flags & ImGuiWindowFlags_Tooltip) return;
    for (int k = 0; k < nseen; k++) if (seen[k] == w->ID) return;
    seen[nseen++] = w->ID;
    fprintf(stderr, "the game's hand over a window: \"%s\"\n", w->Name);
}
