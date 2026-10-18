// bestiary.cpp - F12 > Records > Enemies: every foe of the game (DARKLAND.ENM: 71 kinds of 204
// bytes, then 82 named foes of 24 bytes; formats/dlformats/enm.py has the layout) with its
// attributes, weapon skills, armour (and how hard it is against which weapons), shield and
// weapons. The numbers can be changed and saved as a mod folder's DARKLAND.ENM; the game reads
// the file when it starts.
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
#include "../runtime/game.h"
#include "../mods/datamods.h"
}

enum { NT = 71, TSZ = 204, NE = 82, ESZ = 24 };
static std::vector<unsigned char> enm, orig;
static bool loaded;
static std::string msg;
static char filter[40];
static int open_type = -1;

static bool enm_or_lst_unchanged(void) { return enm == orig; }   /* edits not yet saved are kept */
static void load(void)
{
    loaded = true; enm.clear();
    char p[1300]; dos_host_path_read("DARKLAND.ENM", p, sizeof p);
    FILE *f = fopen(p, "rb");
    if (!f) { msg = "Could not read DARKLAND.ENM."; return; }
    unsigned char b[65536]; size_t n;
    while ((n = fread(b, 1, sizeof b, f)) > 0) enm.insert(enm.end(), b, b + n);
    fclose(f);
    if (enm.size() != (size_t)(NT * TSZ + NE * ESZ)) { msg = "DARKLAND.ENM has an unexpected size."; enm.clear(); return; }
    orig = enm;
}
static unsigned char *type_rec(int t) { return &enm[(size_t)t * TSZ]; }
static unsigned char *foe_rec(int e) { return &enm[(size_t)NT * TSZ + (size_t)e * ESZ]; }
static std::string str_at(const unsigned char *r, int len) { std::string s; for (int k = 0; k < len && r[k]; k++) s += (char)r[k]; return s; }

static const char *ATTR[7] = { "End", "Str", "Agl", "Per", "Int", "Chr", "DF" };
static const char *WSK[7] = { "Edged", "Impact", "Flails", "Polearms", "Thrown", "Bows", "Missile dev." };

/* an LST item by (type, code), or what the creature has instead */
static std::string item_name(int type, int code, const char *natural)
{
    if (code == 0xff || type == 0xff) return "";
    if (code < (int)gamedata.items.size() && gamedata.items[code].type == type && !gamedata.items[code].name.empty()) {
        std::string n = game_text(gamedata.items[code].name.c_str(), gamedata.items[code].name.size());
        if (n.size() > 2 && (n[0] == 'V' || n[0] == 'L') && n[1] == ':') n = n.substr(2);       /* "V:Scale" */
        return n;
    }
    return natural;
}
static int hardness(int type) { return type == 0xff ? 0 : rb(G_FAR20A5, (u16)(0x781e + type)); }
static int penetration(int type) { return type >= 0 && type <= 33 ? rb(G_FAR20A5, (u16)(0x76a8 + type)) & 15 : -1; }

static std::string weapons_of(const unsigned char *t)
{
    std::string s;
    for (int k = 0; k < 6; k++) {
        int ty = t[0xa0 + k], co = t[0xa6 + k];
        std::string n = item_name(ty, co, "natural weapons");
        if (n.empty() || s.find(n) != std::string::npos) continue;
        s += (s.empty() ? "" : ", ") + n;
    }
    return s;
}

static void details(int ti)
{
    const unsigned char *t = type_rec(ti);
    ImGui::Text("Kind: %s (picture group %s)", str_at(t + 4, 10).c_str(), str_at(t, 4).c_str());
    std::string names;
    for (int e = 0; e < NE; e++) { const unsigned char *r = foe_rec(e); if ((r[0] | r[1] << 8) == ti) names += (names.empty() ? "" : ", ") + game_text((const char *)r + 2, strnlen((const char *)r + 2, 12)); }
    if (!names.empty()) ImGui::TextWrapped("Met as: %s", names.c_str());
    int hv = hardness(t[0x92]), hl = hardness(t[0x93]);
    static const char *HN[6] = { "none", "soft", "stiff", "scale", "mail", "plate" };
    std::string va = item_name(t[0x92], t[0x94], "natural armour"), la = item_name(t[0x93], t[0x95], "natural armour");
    ImGui::Text("Body: %s, hardness %d (%s); legs: %s, hardness %d (%s); armour %dq",
                va.empty() ? "nothing" : va.c_str(), hv, hv >= 0 && hv < 6 ? HN[hv] : "?",
                la.empty() ? "nothing" : la.c_str(), hl, hl >= 0 && hl < 6 ? HN[hl] : "?", t[0x96]);
    std::string through;
    for (int w = 0; w <= 33; w++) {
        int pen = penetration(w);
        if (pen <= hv) continue;
        for (const ItemDef &d : gamedata.items)
            if (d.type == w && !d.name.empty() && (d.kind == K_WEAPON || d.kind == K_MISSILE)) { through += (through.empty() ? "" : ", ") + game_text(d.name.c_str(), d.name.size()); break; }
    }
    ImGui::TextWrapped("Weapons that pierce its body armour: %s", through.empty() ? "none" : through.c_str());
    std::string sh = item_name(t[0x97], t[0x98], "a natural shield");
    if (!sh.empty()) ImGui::Text("Shield: %s, %dq", sh.c_str(), t[0x99]);
    std::string mw = item_name(t[0x9a], t[0x9b], "a natural missile");
    if (!mw.empty()) { std::string am = item_name(t[0x9d], t[0x9e], "ammunition"); ImGui::Text("Missile: %s, %dq, %d %s", mw.c_str(), t[0x9c], t[0x9f], am.c_str()); }
    ImGui::TextWrapped("Melee: %s, %dq", weapons_of(t).c_str(), t[0xac]);
}

void bestiary_tab(void)
{
    if (!gamedata.loaded) { ImGui::TextUnformatted("The item list (DARKLAND.LST) could not be read."); return; }
    { static int gen = -1; if (gen != datamods_generation()) { gen = datamods_generation(); if (loaded && enm_or_lst_unchanged()) loaded = false; } }
    if (!loaded) load();
    if (enm.empty()) { ImGui::TextWrapped("%s", msg.c_str()); return; }
    ImGui::TextWrapped("Every foe of the game with its numbers as the game file has them (a fight adds the luck of the dice and "
                       "the difficulty). Click a name for its armour and weapons. The numbers can be changed and saved into a "
                       "mod folder; kinds are shared, so a change applies to every foe of that kind.");
    ImGui::SetNextItemWidth(UIS(220)); ImGui::InputTextWithHint("##eflt", "find a foe", filter, sizeof filter);
    int changed = 0; for (int i = 0; i < NT; i++) if (memcmp(type_rec(i), &orig[(size_t)i * TSZ], TSZ)) changed++;
    ImGui::SameLine(); ImGui::TextDisabled("%d kind%s changed", changed, changed == 1 ? "" : "s");
    if (ImGui::BeginTable("foes", 16, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Sortable | ImGuiTableFlags_SortMulti,
                          ImVec2(0, ImGui::GetContentRegionAvail().y - (open_type >= 0 ? UIS(270) : UIS(80))))) {
        ImGui::TableSetupColumn("Foe", ImGuiTableColumnFlags_WidthStretch);
        for (int a = 0; a < 7; a++) ImGui::TableSetupColumn(ATTR[a]);
        for (int k = 0; k < 7; k++) ImGui::TableSetupColumn(WSK[k]);
        ImGui::TableSetupColumn("Armour");
        ImGui::TableSetupScrollFreeze(1, 1);
        ImGui::TableHeadersRow();
        std::vector<int> rows;
        for (int e = 0; e < NE; e++) {
            unsigned char *r = foe_rec(e);
            int ti = r[0] | r[1] << 8;
            if (ti < 0 || ti >= NT) continue;
            std::string name = game_text((const char *)r + 2, strnlen((const char *)r + 2, 12));
            if (name.empty()) continue;
            if (filter[0]) {
                std::string a = name, b = filter;
                for (char &c : a) c = (char)tolower((unsigned char)c);
                for (char &c : b) c = (char)tolower((unsigned char)c);
                if (a.find(b) == std::string::npos) continue;
            }
            rows.push_back(e);
        }
        static TableSort ts;
        auto key = [&](int e, int c) -> SortKey {
            unsigned char *r = foe_rec(e), *t = type_rec(r[0] | r[1] << 8);
            if (c == 0) return game_text((const char *)r + 2, strnlen((const char *)r + 2, 12));
            if (c <= 7) return (int)t[0x14 + c - 1];
            if (c <= 14) return (int)t[0x1b + c - 8];
            return (int)t[0x96];
        };
        for (int e : table_order(ts, rows, key)) {
            unsigned char *r = foe_rec(e);
            int ti = r[0] | r[1] << 8;
            std::string name = game_text((const char *)r + 2, strnlen((const char *)r + 2, 12));
            unsigned char *t = type_rec(ti);
            bool mod = memcmp(t, &orig[(size_t)ti * TSZ], TSZ) != 0;
            ImGui::PushID(e);
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            if (ImGui::Selectable((name + (mod ? " *" : "")).c_str(), open_type == ti, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap))
                open_type = open_type == ti ? -1 : ti;
            for (int a = 0; a < 7; a++) {
                ImGui::TableNextColumn(); int v = t[0x14 + a];
                ImGui::SetNextItemWidth(UIS(38)); ImGui::PushID(a);
                if (ImGui::DragInt("##a", &v, 0.2f, 0, 99)) t[0x14 + a] = (unsigned char)v;
                ImGui::PopID();
            }
            for (int k = 0; k < 7; k++) {
                ImGui::TableNextColumn(); int v = t[0x1b + k];
                ImGui::SetNextItemWidth(UIS(38)); ImGui::PushID(10 + k);
                if (ImGui::DragInt("##s", &v, 0.2f, 0, 99)) t[0x1b + k] = (unsigned char)v;
                ImGui::PopID();
            }
            ImGui::TableNextColumn(); int q = t[0x96];
            ImGui::SetNextItemWidth(UIS(38));
            if (ImGui::DragInt("##q", &q, 0.2f, 0, 99)) t[0x96] = (unsigned char)q;
            ImGui::PopID();
        }
        ImGui::EndTable();
    }
    if (open_type >= 0 && open_type < NT) {
        ImGui::BeginChild("foe", ImVec2(0, UIS(190)), ImGuiChildFlags_Border);
        details(open_type);
        ImGui::EndChild();
    }
    static char modname[64] = "My foes";
    ImGui::SetNextItemWidth(UIS(220)); ImGui::InputText("##enmmod", modname, sizeof modname); ImGui::SameLine();
    if (ImGui::Button("Save the foes into this mod folder") && modname[0]) {
        char w[1300]; int r = datamods_new(modname, w, sizeof w);
        std::string p = std::string(w) + "/DARKLAND.ENM";
        FILE *f = r < 0 ? nullptr : fopen(p.c_str(), "wb");
        if (!f) msg = "Could not write " + p + ".";
        else { fwrite(enm.data(), 1, enm.size(), f); fclose(f); msg = "Saved " + p + ". Tick the mod (F12 > Settings > Mods) and start the game again."; }
    }
    ImGui::SameLine();
    if (ImGui::Button("Undo all changes##enm")) { enm = orig; msg.clear(); }
    if (!msg.empty()) ImGui::TextWrapped("%s", msg.c_str());
}
