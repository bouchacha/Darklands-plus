// itemedit.cpp - F12 > Records > Items: the item list (DARKLAND.LST: 200 records of 46
// bytes, then the saints' and formulas' names) as an editable table. Changes are saved as a mod
// folder's DARKLAND.LST, so the game's own file stays as it is; the game reads the list when it
// starts.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <string>
#include <vector>
#include "imgui.h"
#include "tablesort.h"
#include "ui.h"
#include "gamedata.h"
extern "C" {
#include "../runtime/hle.h"
#include "../mods/datamods.h"
}

static std::vector<unsigned char> lst, orig;
static bool loaded;
static std::string msg;
static char filter[40];

static bool enm_or_lst_unchanged(void) { return lst == orig; }   /* edits not yet saved are kept */
static void load(void)
{
    loaded = true; lst.clear();
    char p[1300]; dos_host_path_read("DARKLAND.LST", p, sizeof p);
    FILE *f = fopen(p, "rb");
    if (!f) { msg = "Could not read DARKLAND.LST."; return; }
    unsigned char b[65536]; size_t n;
    while ((n = fread(b, 1, sizeof b, f)) > 0) lst.insert(lst.end(), b, b + n);
    fclose(f);
    orig = lst;
}

static int nitems(void) { return lst.size() >= 3 ? lst[0] : 0; }
static unsigned char *rec(int i) { return &lst[3 + (size_t)i * 46]; }
static std::string field_str(const unsigned char *r, int off, int len) { std::string s; for (int k = 0; k < len && r[off + k]; k++) s += (char)r[off + k]; return s; }

void itemedit_tab(void)
{
    { static int gen = -1; if (gen != datamods_generation()) { gen = datamods_generation(); if (loaded && enm_or_lst_unchanged()) loaded = false; } }
    if (!loaded) load();
    if (lst.size() < 3) { ImGui::TextWrapped("%s", msg.c_str()); return; }
    ImGui::TextWrapped("Every item of the game. Change names, weight, quality, rarity and value here, then save the list "
                       "into a mod folder; it takes effect when the game is started again with that mod ticked.");
    ImGui::SetNextItemWidth(UIS(220)); ImGui::InputTextWithHint("##flt", "find an item", filter, sizeof filter);
    int changed = 0; for (int i = 0; i < nitems(); i++) if (memcmp(rec(i), &orig[3 + (size_t)i * 46], 46)) changed++;
    ImGui::SameLine(); ImGui::TextDisabled("%d changed", changed);
    if (ImGui::BeginTable("items", 8, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Sortable | ImGuiTableFlags_SortMulti,
                          ImVec2(0, ImGui::GetContentRegionAvail().y - 80))) {
        const char *H[8] = { "#", "Name", "Short name", "Kind", "Weight", "Quality", "Rarity", "Value (pf)" };
        for (int c = 0; c < 8; c++) ImGui::TableSetupColumn(H[c], c == 1 ? ImGuiTableColumnFlags_WidthStretch : 0);
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableHeadersRow();
        std::vector<int> rows;
        for (int i = 0; i < nitems(); i++) {
            std::string name = field_str(rec(i), 0, 20);
            if (name.empty()) continue;
            if (filter[0]) {
                std::string a = name, b = filter;
                for (char &c : a) c = (char)tolower((unsigned char)c);
                for (char &c : b) c = (char)tolower((unsigned char)c);
                if (a.find(b) == std::string::npos) continue;
            }
            rows.push_back(i);
        }
        static TableSort ts;
        auto key = [&](int i, int c) -> SortKey {
            unsigned char *r = rec(i);
            const ItemDef *d = i < (int)gamedata.items.size() ? &gamedata.items[i] : nullptr;
            switch (c) {
            case 1: return field_str(r, 0, 20);
            case 2: return field_str(r, 20, 10);
            case 3: return d ? kind_name(d->kind) : "";
            case 4: return (int)r[37];
            case 5: return (int)r[38];
            case 6: return (int)r[39];
            case 7: return r[44] | r[45] << 8;
            default: return i;
            }
        };
        for (int i : table_order(ts, rows, key)) {
            unsigned char *r = rec(i);
            ImGui::PushID(i);
            ImGui::TableNextRow();
            bool mod = memcmp(r, &orig[3 + (size_t)i * 46], 46) != 0;
            ImGui::TableNextColumn(); if (mod) ImGui::TextColored(ImVec4(0.6f, 0.2f, 0.1f, 1), "%d*", i); else ImGui::Text("%d", i);
            char nb[21]; memcpy(nb, r, 20); nb[20] = 0;
            ImGui::TableNextColumn(); ImGui::SetNextItemWidth(-1);
            if (ImGui::InputText("##n", nb, sizeof nb)) { memset(r, 0, 20); memcpy(r, nb, strlen(nb)); }
            char sb[11]; memcpy(sb, r + 20, 10); sb[10] = 0;
            ImGui::TableNextColumn(); ImGui::SetNextItemWidth(UIS(110));
            if (ImGui::InputText("##s", sb, sizeof sb)) { memset(r + 20, 0, 10); memcpy(r + 20, sb, strlen(sb)); }
            ImGui::TableNextColumn();
            const ItemDef *d = i < (int)gamedata.items.size() ? &gamedata.items[i] : nullptr;
            ImGui::TextDisabled("%s", d ? kind_name(d->kind) : "");
            int v;
            v = r[37]; ImGui::TableNextColumn(); ImGui::SetNextItemWidth(UIS(70)); if (ImGui::InputInt("##w", &v, 0)) r[37] = (unsigned char)(v < 0 ? 0 : v > 255 ? 255 : v);
            v = r[38]; ImGui::TableNextColumn(); ImGui::SetNextItemWidth(UIS(70)); if (ImGui::InputInt("##q", &v, 0)) r[38] = (unsigned char)(v < 0 ? 0 : v > 255 ? 255 : v);
            v = r[39]; ImGui::TableNextColumn(); ImGui::SetNextItemWidth(UIS(70)); if (ImGui::InputInt("##r", &v, 0)) r[39] = (unsigned char)(v < 0 ? 0 : v > 255 ? 255 : v);
            v = r[44] | r[45] << 8; ImGui::TableNextColumn(); ImGui::SetNextItemWidth(UIS(90));
            if (ImGui::InputInt("##v", &v, 0)) { if (v < 0) v = 0; if (v > 65535) v = 65535; r[44] = (unsigned char)v; r[45] = (unsigned char)(v >> 8); }
            ImGui::PopID();
        }
        ImGui::EndTable();
    }
    static char modname[64] = "My items";
    ImGui::SetNextItemWidth(UIS(220)); ImGui::InputText("##itemmod", modname, sizeof modname); ImGui::SameLine();
    if (ImGui::Button("Save the list into this mod folder") && modname[0]) {
        char w[1300]; int r = datamods_new(modname, w, sizeof w);
        std::string p = std::string(w) + "/DARKLAND.LST";
        FILE *f = r < 0 ? nullptr : fopen(p.c_str(), "wb");
        if (!f) msg = "Could not write " + p + ".";
        else { fwrite(lst.data(), 1, lst.size(), f); fclose(f); msg = "Saved " + p + ". Tick the mod (F12 > Settings > Mods) and start the game again."; }
    }
    ImGui::SameLine();
    if (ImGui::Button("Undo all changes")) { lst = orig; msg.clear(); }
    if (!msg.empty()) ImGui::TextWrapped("%s", msg.c_str());
}
