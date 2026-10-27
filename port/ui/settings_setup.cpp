// settings_setup.cpp - the Settings page's sections on the game on this computer (ui/settings.cpp
// draws the page): Screen, Intro, Sound and Controller.
#include "../mods/playlist.h"
#include <SDL.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <string>
#include <vector>
#include <functional>
#include "imgui.h"
#include "padglyphs.h"
#include "../mods/fixgroups.h"
#include "skillicons.h"
#include "gamepics.h"
#include "ornament.h"
#include "tablesort.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_sdlrenderer2.h"
#include "gamedata.h"
#include "quests.h"

extern "C" {
#include "../runtime/cpu.h"
#include "../runtime/game.h"
#include "../runtime/config.h"
#include "../mods/mods.h"
#include "../mods/assist.h"
#include "../mods/cardview.h"
#include "../mods/travel.h"
#include "../mods/fixes.h"
#include "../mods/saves.h"
#include "../mods/hdtext.h"
#include "../mods/musicpack.h"
#include "../mods/hdart.h"
#include "../mods/sfx.h"
void soundpreview_start(const char *dev, int what); void soundpreview_stop(void); int soundpreview_playing(const char *dev, int what); int soundpreview_possible(void);
void musicpack_preview(int n); int musicpack_first_piece(void); void sfx_preview(int k); u32 plat_ticks_ms(void);
#include "../mods/datamods.h"
#include "../mods/lootview.h"
#include "../mods/sheetview.h"
#include "../mods/options.h"
#include "ui.h"
#include "money.h"
extern char game_dir[1024];
extern const unsigned char FONT_MEDIUM[], FONT_SEMIBOLD[], FONT_ALEGREYA[], FONT_FELL[], FONT_MEDIEVAL[], FONT_ATKINSON[], FONT_FRAKTUR[], FONT_GRENZE[];
extern const unsigned int FONT_MEDIUM_SIZE, FONT_SEMIBOLD_SIZE, FONT_ALEGREYA_SIZE, FONT_FELL_SIZE, FONT_MEDIEVAL_SIZE, FONT_ATKINSON_SIZE, FONT_FRAKTUR_SIZE, FONT_GRENZE_SIZE;
}

#include "uiinternal.h"

#include "settings_parts.h"
extern "C" bool screenpics_scaling(ImDrawList *dl, ImVec2 a, ImVec2 b, int mode);   /* ui/screenpics.cpp */
extern "C" int opening_mode(void); extern "C" void opening_set_mode(int);           /* mods/opening.c */

extern "C" { int mpu_out_count(void); const char *mpu_out_name(int i); int mt32_families(void); int mt32_switch_pending(void); int mt32_switch(void); const char *mt32_status(void); void mt32_rescan(void); int mt32_dir_count(void); const char *mt32_dir(int); extern int sound_music_volume; int mt32_available(void); void ui_show_mt32_help(void); }
extern "C" void settings_helpers_jump(const char *name);
extern "C" void settings_display_jump(int k) { static const char *N[4] = { "Screen", "Intro", "Sound", "Controller" }; if (k >= 0 && k < 4) settings_helpers_jump(N[k]); }   /* (test: udsect) */
/* a game controller: always on (but in the original game's menus it is a mouse too); which one is
   connected, its buttons drawn as any of the three kinds, and the jobs of the main ones to change */
extern "C" int plat_controller_kind(const char **name);
extern "C" int plat_pad_bind(int k);
extern "C" void plat_pad_bind_set(int k, int a);
int settings_pack_preview;                 /* (a pack previewed: ui/ui.cpp lets the music through) */
void controller_prefs(void)
{
    const char *nm; int kind = plat_controller_kind(&nm);
    static const char *KIND[4] = { "", "Xbox", "PlayStation", "Nintendo" };
    if (kind) ImGui::Text("Connected: %s (%s style)", nm, KIND[kind]);
    else ImGui::TextWrapped("No controller connected now: plug one in at any time. The buttons below can be set up beforehand.");
    static int view = 0;
    if (!view) view = kind ? kind : 1;
    ImGui::TextUnformatted("Show the buttons as:"); 
    for (int k = 1; k <= 3; k++) { ImGui::SameLine(); if (ImGui::RadioButton(KIND[k], view == k)) view = k; }
    static const char *JOB[9] = { "Click", "Right-click (the menus)", "Enter", "Esc", "Tab (the level map)", "Space (pause)", "F12 (the Darklands+ window)", "M (the world map)", "F11 (keys and clicks)" };
    int keep = ui_pad_style_force; ui_pad_style_force = view;
    float gh = ImGui::GetFrameHeight();
    if (ImGui::BeginTable("##padmap", 2, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("b", ImGuiTableColumnFlags_WidthFixed, UIS(70));
        ImGui::TableSetupColumn("j", ImGuiTableColumnFlags_WidthFixed, UIS(440));
        auto glyph = [&](int k, bool phys) {
            ImVec2 p = ImGui::GetCursorScreenPos();
            float w = ui_pad_glyph_width(gh * 0.9f, k);
            ImGui::Dummy(ImVec2(UIS(60), gh));
            if (phys) ui_pad_button_glyph(ImGui::GetWindowDrawList(), ImVec2(p.x + w * 0.5f + 4, p.y + gh * 0.5f), gh * 0.9f, k);
            else ui_pad_glyph(ImGui::GetWindowDrawList(), ImVec2(p.x + w * 0.5f + 4, p.y + gh * 0.5f), gh * 0.9f, k);
        };
        for (int k = PB_A; k <= PB_LS; k++) {
            ImGui::TableNextRow(); ImGui::TableNextColumn(); glyph(k, true);
            ImGui::TableNextColumn();
            int a = plat_pad_bind(k);
            ImGui::PushID(k); ImGui::SetNextItemWidth(UIS(290));
            if (ImGui::Combo("##job", &a, JOB, 9)) { plat_pad_bind_set(k, a); ui_dirty_config = 1; }
            ImGui::PopID();
        }
        struct F { int b; const char *what; } fixed[4] = { { PB_LSTICK, "the pointer (the left trigger held: slowly)" }, { PB_RSTICK, "scrolls lists; pressed: Z (fast-forward)" },
                                                            { PB_DPAD, "the arrow keys" }, { PB_RT, "held: Ctrl (Ctrl+click in the overland view travels there)" } };
        for (const F &f : fixed) { ImGui::TableNextRow(); ImGui::TableNextColumn(); glyph(f.b, true); ImGui::TableNextColumn(); ImGui::AlignTextToFramePadding(); ImGui::TextUnformatted(f.what); }
        ImGui::EndTable();
    }
    ui_pad_style_force = keep;
    if (ImGui::Button("Back to the usual buttons")) { for (int k = PB_A; k <= PB_LS; k++) plat_pad_bind_set(k, k); ui_dirty_config = 1; }
    /* the button prompts (ui/padglyphs.h): the controller's buttons on screen while it is in use */
    static const char *PM[5] = { "Match the device in use", "Keys and mouse always", "Xbox always", "PlayStation always", "Nintendo always" };
    int pm = config_get_int("input.prompts", 0); if (pm < 0 || pm > 4) pm = 0;
    ImGui::SetNextItemWidth(UIS(260));
    if (ImGui::Combo("Buttons shown in hints", &pm, PM, 5)) { config_set_int("input.prompts", pm); ui_dirty_config = 1; }
    ui_hint("While a controller is in use, the hints and on-screen buttons show its buttons. The \"always\" choices are for a Steam Deck played with its trackpads, which count as a mouse.");
}
/* the setup of the game on this computer: four sections of the Settings page (tab_helpers) */
void setup_screen(void)
{
    {   /* the game's picture scaled up: three ways, each shown on a piece of the screen now (ui/screenpics.cpp);
           (the picture is always 4:3, as on a 1992 monitor) */
        int cur = ui_settings.scaler ? 1 : ui_settings.smooth ? 2 : 0;
        static const char *L[3] = { "Sharp pixels", "Rounded edges", "Soft" };
        float cw = fminf((ImGui::GetContentRegionAvail().x - 2 * ImGui::GetStyle().ItemSpacing.x) / 3, UIS(240)), ph = cw * 0.625f, ch = ph + ImGui::GetTextLineHeightWithSpacing() * 1.3f;
        for (int j = 0; j < 3; j++) {
            if (j) ImGui::SameLine();
            ImGui::PushID(400 + j);
            ImVec2 p = ImGui::GetCursorScreenPos();
            if (ImGui::InvisibleButton("sc", ImVec2(cw, ch))) {
                ui_settings.scaler = j == 1 ? 2 : 0; ui_settings.smooth = j != 0; ui_settings.integer = 0;
                config_set_int("display.scaler", ui_settings.scaler); config_set_int("display.smooth", ui_settings.smooth); config_set_int("display.integer_scale", 0);
                ui_dirty_config = 1; cur = j;
            }
            bool hv = ImGui::IsItemHovered();
            ImDrawList *dl = ImGui::GetWindowDrawList();
            dl->AddRectFilled(p, ImVec2(p.x + cw, p.y + ch), j == cur ? IM_COL32(140, 61, 33, 255) : hv ? IM_COL32(200, 170, 120, 255) : IM_COL32(60, 44, 30, 255), 6);
            float m = cw * 0.04f;
            if (!screenpics_scaling(dl, ImVec2(p.x + m, p.y + m), ImVec2(p.x + cw - m, p.y + ph - m * 0.4f), j))
                dl->AddText(ImVec2(p.x + 10, p.y + 10), IM_COL32(246, 236, 214, 255), "(no picture yet)");
            ImVec2 ts = ImGui::CalcTextSize(L[j]);
            dl->AddText(ImVec2(p.x + (cw - ts.x) * 0.5f, p.y + ch - ImGui::GetTextLineHeightWithSpacing() * 1.1f), IM_COL32(246, 236, 214, 255), L[j]);
            ImGui::PopID();
        }
        ui_hint(cur == 0 ? "Every pixel a crisp square, as the game drew it."
              : cur == 1 ? "The stair-steps of diagonal edges rounded off, without blurring. On a computer without a proper graphics card, the game slows down with this."
                         : "Blurred a little, as an old monitor would.");
    }
    { extern int ui_margins_on; bool mg = ui_margins_on != 0, click = false;   /* (a long line: the words wrapped beside the box, not one line wider than the page) */
      ImGui::PushID("margins");
      if (ImGui::Checkbox("##mg", &mg)) click = true;
      ImGui::SameLine();
      if (ui_title_desc("Side margins", "on wide screens, the date, place, purse, quests, town and effects at the sides (in battles: the log and everyone's condition).")) { mg = !mg; click = true; }
      if (click) { ui_margins_on = mg; config_set_int("display.margins", mg); ui_dirty_config = 1; }
      ImGui::PopID(); }
    {   /* grows with the picture; this scales it */
        static const int P[] = { 70, 80, 90, 100, 115, 130, 150, 175, 200 };
        int cur = config_get_int("display.text_size", 100), sel = 3;
        for (int i = 0; i < 9; i++) if (P[i] == cur) sel = i;
        char lab[16]; snprintf(lab, sizeof lab, "%d%%", P[sel]);
        ImGui::SetNextItemWidth(UIS(160));
        if (ImGui::BeginCombo("text size in the margins, tooltips and place names", lab)) {
            for (int i = 0; i < 9; i++) { char t[16]; snprintf(t, sizeof t, "%d%%", P[i]); if (ImGui::Selectable(t, i == sel)) { config_set_int("display.text_size", P[i]); ui_dirty_config = 1; } }
            ImGui::EndCombo();
        }
    }
    { bool pd = config_get_int("display.pointer_drawn", 1) != 0;
      if (ImGui::Checkbox("The game's gauntlet pointer drawn anew, crisp at any size", &pd)) { config_set_int("display.pointer_drawn", pd); ui_dirty_config = 1; }
      if (ImGui::IsItemHovered()) ImGui::SetTooltip("A fist while something is dragged; on its cuff the pause in a paused battle, crossed blades over a foe, an hourglass while Go to walks. Off: the game's little picture."); }
    { bool sc = ui_sys_cursor != 0;
      if (ImGui::Checkbox("Show the system mouse pointer over the game too", &sc)) { ui_sys_cursor = sc; config_set_int("display.system_cursor", sc); ui_dirty_config = 1; } }
    {   /* the menus' size (fonts are baked at start-up) */
        static const int P[] = { 0, 100, 125, 150, 175, 200, 250 };
        static const char *N[] = { "automatic (the system's scale)", "100%", "125%", "150%", "175%", "200%", "250%" };
        int cur = config_get_int("display.ui_scale", 0), sel = 0;
        for (int i = 0; i < 7; i++) if (P[i] == cur) sel = i;
        ImGui::SetNextItemWidth(UIS(260));
        if (ImGui::Combo("size of these menus (after a restart)", &sel, N, 7)) { config_set_int("display.ui_scale", P[sel]); ui_dirty_config = 1; }
    }
    { bool pu = config_get_int("display.pause_unfocused", 1) != 0;
      if (ImGui::Checkbox("Pause the game while its window is in the background", &pu)) { config_set_int("display.pause_unfocused", pu); ui_dirty_config = 1; } }
    bool fs = (SDL_GetWindowFlags(ui_window) & SDL_WINDOW_FULLSCREEN_DESKTOP) != 0;
    if (ImGui::Checkbox("Full screen (Alt+Enter)", &fs)) SDL_SetWindowFullscreen(ui_window, fs ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
}
void setup_intro(void)
{
    {   /* the opening at start (mods/opening.c): the first start plays it whole */
        static const char *N[] = { "None: straight to the start screen", "Only the last scene: the lady of the Darklands and her welcome", "The whole intro" };
        int m = opening_mode();
        for (int i = 0; i < 3; i++) if (ImGui::RadioButton(N[i], m == i)) { opening_set_mode(i); ui_dirty_config = 1; }
        ui_hint("Shown each time the game starts (the very first start plays it whole). A key or a click skips ahead.");
    }
}
void setup_sound(void)
{
    /* two previews, at the top right of the section: what the sound chosen below sounds like now (the
       second tune of the overland view, or ten seconds of a real battle's sounds), through the device chosen, or the
       custom pack when it is ticked (mods/soundpreview.c, mods/musicpack.c, mods/sfx.c) */
    const char *cur = config_get("sound.device", "auto");
    bool can_hear = soundpreview_possible() != 0;
    static u32 pack_until; static bool pack_on;
    static u32 seq_t; static int seq_k = -1;
    static const int SEQ[] = { 1, 2, 0, 3, 5, 4 };   /* parry, hit, miss, knock-back, bow, death */
    if (pack_on && plat_ticks_ms() > pack_until) { musicpack_preview(-1); pack_on = false; }
    if (seq_k >= 0 && plat_ticks_ms() >= seq_t) { sfx_preview(SEQ[seq_k]); seq_t = plat_ticks_ms() + 650; if (++seq_k >= 6) seq_k = -1; }
    settings_pack_preview = pack_on || seq_k >= 0;
    {
        const char *dev = !strcmp(cur, "auto") || !strcmp(cur, "game") ? (mt32_available() ? "mt32" : "fm") : cur;
        bool off = !strcmp(cur, "off");
        bool pack_music = musicpack_on && musicpack_versions(20) > 0, pack_sfx = sfx_mode != 0 || sfx_mod_count() > 0;
        float bw = ImGui::CalcTextSize("Preview: Battle Sounds").x + ImGui::GetStyle().FramePadding.x * 2;
        float bm = ImGui::CalcTextSize("Preview: Music").x + ImGui::GetStyle().FramePadding.x * 2;
        float x = ImGui::GetContentRegionMax().x - bw - bm - ImGui::GetStyle().ItemSpacing.x;
        ImGui::SetCursorPosX(x);
        bool spk = !strcmp(dev, "speaker") && !pack_music;            /* (the game plays no music on the PC speaker: its driver only beeps the battle sounds) */
        ImGui::BeginDisabled(off || spk || (!can_hear && !pack_music));
        bool mon = pack_music ? pack_on : soundpreview_playing(dev, 0) != 0;
        if (ImGui::Button(mon ? "Stop##pm" : "Preview: Music", ImVec2(bm, 0))) {
            if (mon) { if (pack_on) { musicpack_preview(-1); pack_on = false; } else soundpreview_stop(); }
            else if (pack_music) { musicpack_preview(20); pack_on = true; pack_until = plat_ticks_ms() + 15000; }
            else soundpreview_start(dev, 0);
        }
        ImGui::EndDisabled();
        if (spk && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) ImGui::SetTooltip("The game plays no music on the PC speaker, only the battle sounds.");
        ImGui::SameLine();
        ImGui::BeginDisabled(off || (!can_hear && !pack_sfx));
        bool bon = pack_sfx ? seq_k >= 0 : soundpreview_playing(dev, 1) != 0;
        if (ImGui::Button(bon ? "Stop##pb" : "Preview: Battle Sounds", ImVec2(bw, 0))) {
            if (bon) { if (seq_k >= 0) seq_k = -1; else soundpreview_stop(); }
            else if (pack_sfx) { seq_k = 0; seq_t = 0; }
            else soundpreview_start(dev, 1);
        }
        ImGui::EndDisabled();
        if (!can_hear && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) ImGui::SetTooltip("A preview needs a game under way, outside a battle.");
    }
    /* the devices, in three tiers: the best (the MT-32, as composed), an OK one (the Sound Blaster,
       as most heard it in 1992), and those for history's sake */
    auto device = [&](const char *id, const char *title, const char *what, bool chosen) {
        ImGui::PushID(id);
        if (ImGui::RadioButton(title, chosen)) { config_set("sound.device", id); ui_dirty_config = 1; }
        if (what) { ImGui::Indent(UIS(30)); ui_hint("%s", what); ImGui::Unindent(UIS(30)); }
        ImGui::PopID();
    };
    auto tier = [&](const char *t) { ImGui::Spacing(); ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(120, 52, 20, 255)); ImGui::TextUnformatted(t); ImGui::PopStyleColor(); };
    tier("Best");
    device("mt32", "Roland MT-32", "Roland's synthesizer of 1987, the one Darklands was written for. Music and battle sounds alike are notes played on it: the music as composed, the blows as near to steel as a synthesizer gets.",
           !strcmp(cur, "mt32") || !strcmp(cur, "auto"));
    {
        ImGui::Indent(UIS(30));
        bool roms = mt32_available() != 0;
        static bool more;
        if (!roms) {      /* the one thing that pokes out: in red, a click for the rest */
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(170, 30, 20, 255));
            if (ImGui::Selectable(more ? "MT-32 ROMs not found (click to close)" : "MT-32 ROMs not found (click for how to get it working)", more, 0, ImVec2(ImGui::CalcTextSize("MT-32 ROMs not found (click for how to get it working)").x, 0))) more = !more;
            ImGui::PopStyleColor();
        }
        if (roms) { ImGui::SetNextItemOpen(more); more = ImGui::TreeNode("MT-32 details"); if (more) ImGui::TreePop(); }
        if (more) {
            ImGui::Indent(UIS(14));
            if (!roms) {
                ImGui::TextWrapped("The MT-32 was a Roland synthesizer of 1987; Darklands' music and battle sounds were written for it (both are notes played on it), and every other choice here is a "
                                   "lesser version. Darklands+ has it built in (Munt's imitation of it) and only needs two files copied from a real one's memory (its ROM files), which belong to Roland "
                                   "and can't be included. If you have used Munt or DOSBox with an MT-32 you already have them. Until they are found, the Sound Blaster plays.");
                if (ImGui::Button("Where to find them...")) ui_show_mt32_help();
            } else ui_hint("%s", mt32_status());
            static char romdir[1024]; static bool init;
            if (!init) { snprintf(romdir, sizeof romdir, "%s", config_get("sound.mt32_roms", "")); init = true; }
            ImGui::SetNextItemWidth(UIS(420));
            if (ImGui::InputTextWithHint("##mt32roms", "the folder with the ROM files", romdir, sizeof romdir)) { config_set("sound.mt32_roms", romdir); ui_dirty_config = 1; }
            ImGui::SameLine();
            if (ImGui::Button("Look again##mt32")) { mt32_rescan(); mt32_switch(); }
            ui_hint("Darklands+ finds them by their contents, whatever their names: in that folder, in the game's folder and its \"roms\" folder, and where Munt and DOSBox keep them.");
            {   bool lcd = config_get_int("sound.mt32_lcd", 1), click = false;
                ImGui::PushID("lcd");
                if (ImGui::Checkbox("##lcd", &lcd)) click = true;
                ImGui::SameLine();
                if (ui_title_desc("The MT-32's display", "its little screen at the top right, when the game puts a message on it.")) { lcd = !lcd; click = true; }
                if (click) { config_set_int("sound.mt32_lcd", lcd); ui_dirty_config = 1; }
                ImGui::PopID(); }
            ImGui::Unindent(UIS(14));
        }
        ImGui::Unindent(UIS(30));
    }
    tier("OK");
    device("fm", "Sound Blaster Pro", "The sound card most players had in 1992: the music arranged for its simpler synthesizer, thinner and buzzier than the MT-32.", !strcmp(cur, "fm"));
    device("midi", "MIDI", "The music and battle sounds sent out of Darklands+ as MIDI: for a real MT-32 or CM-32L connected to the computer, or Munt (an MT-32 imitation) installed on its own. "
           "An ordinary synthesizer, such as the one built into Windows, plays the wrong instruments.", !strcmp(cur, "midi"));
    if (!strcmp(cur, "midi")) {
        int n = mpu_out_count();
        const char *out = config_get("sound.midi_out", "");
        if (!n) ui_hint("No MIDI outputs (connections to a synthesizer) on this computer.");
        else {
            ImGui::SetNextItemWidth(UIS(420));
            if (ImGui::BeginCombo("MIDI output", out[0] ? out : "the system's default")) {
                if (ImGui::Selectable("the system's default", !out[0])) { config_set("sound.midi_out", ""); ui_dirty_config = 1; }
                for (int i = 0; i < n; i++) { const char *nm = mpu_out_name(i); if (nm[0] && ImGui::Selectable(nm, !strcmp(nm, out))) { config_set("sound.midi_out", nm); ui_dirty_config = 1; } }
                ImGui::EndCombo();
            }
        }
    }
    tier("For history's sake");
    device("adlib", "AdLib (an older sound card)", "Much the same music; the battle sounds are rough imitations too.", !strcmp(cur, "adlib"));
    device("speaker", "PC speaker", "Beeps for the battle sounds; the game plays no music on it.", !strcmp(cur, "speaker"));
    device("game", "As set in the original game's INSTALL program", 0, !strcmp(cur, "game"));
    device("off", "Off", 0, !strcmp(cur, "off"));
    ui_hint("A change is heard from the next battle, or the next start.");
    {
        int v = sound_music_volume;
        ImGui::SetNextItemWidth(UIS(260));
        if (ImGui::SliderInt("Volume of the music", &v, 0, 150, "%d%%")) { sound_music_volume = v; config_set_int("sound.music_volume", v); ui_dirty_config = 1; }
    }

    /* the soundtrack: every piece of the game's music, through the device chosen above (mods/playlist.c) */
    {
        ImGui::Spacing();
        const char *pdev = !strcmp(cur, "auto") || !strcmp(cur, "game") ? (mt32_available() ? "mt32" : "fm") : cur;
        bool can = soundpreview_possible() && strcmp(cur, "off") != 0;
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(120, 52, 20, 255)); ImGui::TextUnformatted("Soundtrack"); ImGui::PopStyleColor();
        ui_hint(can ? "Every piece of the game's music, played through the device chosen above. Click a piece to hear it."
                    : "Every piece of the game's music, played through the device chosen above: it needs a game under way, outside a battle.");
        auto mmss = [](int ms) { char t[16]; int s2 = ms / 1000; snprintf(t, sizeof t, "%d:%02d", s2 / 60, s2 % 60); return std::string(t); };
        int now = playlist_now(), len = now >= 0 ? playlist_length_ms(now) : -1;
        bool paused = playlist_paused() != 0, meas = playlist_measuring() != 0;
        ImGui::BeginDisabled(!can || now < 0 || meas);
        if (ImGui::Button(now >= 0 && !paused ? "Pause##st" : "Play##st", ImVec2(UIS(64), 0))) playlist_pause(!paused);
        ImGui::SameLine();
        if (ImGui::Button("Stop##st", ImVec2(UIS(56), 0))) playlist_stop();
        ImGui::EndDisabled();
        ImGui::SameLine();
        if (now >= 0) {
            const char *d = musicpack_desc(now);
            ImGui::Text("%d  %s", now, d[0] ? d : "");
            static int drag = -1;
            int pos = drag >= 0 ? drag : playlist_pos_ms();
            ImGui::TextUnformatted(meas ? "-:--" : mmss(pos).c_str());
            ImGui::SameLine();
            ImGui::SetNextItemWidth(UIS(300));
            int v = pos;
            ImGui::BeginDisabled(meas || len <= 0);
            if (ImGui::SliderInt("##seek", &v, 0, len > 0 ? len : 1, "")) drag = v;
            if (ImGui::IsItemDeactivatedAfterEdit() && drag >= 0) { playlist_seek(drag); drag = -1; }
            else if (!ImGui::IsItemActive()) drag = -1;
            ImGui::EndDisabled();
            ImGui::SameLine();
            if (meas) ui_hint("finding its length (silent, a few seconds)");
            else ImGui::TextUnformatted(len > 0 ? (mmss(len) + (musicpack_loops(now) ? ", then again" : "")).c_str() : len == -2 ? "repeats" : "");
        } else ImGui::TextDisabled("no piece playing");
        ImGui::BeginDisabled(!can);
        if (ImGui::BeginChild("##pieces", ImVec2(0, UIS(210)), ImGuiChildFlags_Borders)) {
            for (int k = 8; k <= 47; k++) {
                const char *d = musicpack_desc(k);
                ImGui::PushID(k);
                float x0 = ImGui::GetCursorPosX();
                if (ImGui::Selectable("##piece", now == k)) playlist_play(pdev, k);
                ImGui::SameLine(x0 + UIS(4)); ImGui::TextDisabled("%d", k);   /* (the numbers in a column of their own) */
                ImGui::SameLine(x0 + UIS(40)); ImGui::TextUnformatted(d[0] ? d : "(a piece without a known place)");
                ImGui::PopID();
                int l = playlist_length_dev_ms(pdev, k);
                if (l > 0 || l == -2) {
                    std::string t = l > 0 ? mmss(l) : std::string("repeats");
                    ImGui::SameLine(ImGui::GetContentRegionMax().x - ImGui::CalcTextSize(t.c_str()).x - UIS(6));
                    ImGui::TextDisabled("%s", t.c_str());
                }
            }
        }
        ImGui::EndChild();
        ImGui::EndDisabled();
    }

    /* custom packs: recordings in place of the synthesizer's music, or of the battle sounds (folded away) */
    ImGui::Spacing();
    extern char dlp_user_dir[];
    if (ImGui::TreeNode("Custom sound packs")) {
        ui_hint("Recordings that replace the synthesizer's music or battle sounds. A battle sounds pack comes with Darklands+ as a mod (F12 > Mods); no music pack does. The previews above play them when ticked.");
        {   /* music packs (mods/musicpack.c) */
            bool mp = musicpack_on;
            if (ImGui::Checkbox("Music from a music pack (recordings in a \"music\" folder)", &mp)) { musicpack_set(mp); ui_dirty_config = 1; }
            int n = musicpack_count();
            if (n) {
                const char *f0 = ""; for (int k = 0; k < 60 && !f0[0]; k++) f0 = musicpack_from(k);
                ui_hint("%d piece%s in the pack (%s); the synthesizer plays the others.", n, n == 1 ? "" : "s", strcmp(f0, "music") ? f0 : "the music folder");
            }
            else ui_hint("No music pack found: a mod's music folder, or %s%smusic, with a recording for each piece named by its number (10.ogg, \"11 market.ogg\").", dlp_user_dir, PATH_SEP);
            {   int fp = 0, now = musicpack_now(&fp);              /* (the piece playing: to make a pack by listening) */
                if (now > 0) { const char *d = musicpack_desc(now); ui_hint("The game's music now: piece %d%s%s%s, %s (silent while Settings is open).", now, d[0] ? " (" : "", d, d[0] ? ")" : "", fp ? "from the pack" : "by the synthesizer"); } }
            if (ImGui::TreeNode("The pieces")) {
                ui_hint("A recording replaces a piece when its file name starts with the number (up to four for one number, played in turn: \"27 a.ogg\", \"27 b.ogg\" ...). "
                        "Pieces that loop in the game loop in the pack; while a piece of the pack is on, the synthesizer is silent.");
                for (int k = 8; k <= 47; k++) {
                    const char *f = musicpack_file(k), *d = musicpack_desc(k);
                    const char *bn = f + strlen(f); while (bn > f && bn[-1] != '/' && bn[-1] != '\\') bn--;
                    ui_hint("%2d  %s%s%s", k, d, d[0] ? ", " : "", musicpack_loops(k) ? "loops" : "plays once");
                    if (f[0]) { ImGui::SameLine(); int nv = musicpack_versions(k); if (nv > 1) ImGui::Text("  %s and %d more, in turn", bn, nv - 1); else ImGui::Text("  %s", bn); }
                }
                if (ImGui::SmallButton("Look again##music")) musicpack_reload();
                ImGui::TreePop();
            }
        }
        {   /* battle sounds packs (mods/sfx.c) */
            ImGui::Spacing();
            bool sp = sfx_mode;   /* (hits: the fix "Ordinary hits make a sound") */
            int nm = sfx_mod_count(), n = sfx_user_count() - nm;
            if (ImGui::Checkbox("Battle sounds from a sound pack (the \"sounds\" folder)", &sp)) { sfx_set(sp); ui_dirty_config = 1; }
            if (nm) ui_hint("A mod enabled in F12 > Mods gives %d battle sound%s: they play in place of the synthesizer's (switch the mod off there for the game's own).", nm, nm == 1 ? "" : "s");
            if (n > 0) ui_hint("%d sound%s in %s%ssounds%s; the game's own sounds for the rest.", n, n == 1 ? "" : "s", dlp_user_dir, PATH_SEP, sp ? "" : " (played once this is ticked)");
            else if (!nm) ui_hint("Recorded blows, parries and cries in place of the synthesizer's: a pack in %s%ssounds, or the \"Battle sounds\" mod (F12 > Mods).", dlp_user_dir, PATH_SEP);
            if (sp || nm) {
                ImGui::Indent(UIS(30));
                int v = sfx_volume;
                ImGui::SetNextItemWidth(UIS(260));
                if (ImGui::SliderInt("Volume of the pack", &v, 0, 150, "%d%%")) { sfx_set_volume(v); ui_dirty_config = 1; }
                static char msg[400];
                if (ImGui::Button("Look again##sfx")) { sfx_reload(); snprintf(msg, sizeof msg, "%d sound%s found.", sfx_user_count(), sfx_user_count() == 1 ? "" : "s"); }
                if (msg[0]) ui_hint("%s", msg);
                ImGui::Unindent(UIS(30));
            }
            if (ImGui::TreeNode("File names for a sound pack")) {
                for (int k = 0; k < SFX_COUNT; k++) ui_hint("%s.wav or .ogg: %s", SFX_NAMES[k], SFX_DESC[k]);
                ui_hint("Several versions of one sound (parry1.wav, parry2.wav ...) are played in turn at random.");
                ImGui::TreePop();
            }
        }
        ImGui::TreePop();
    }
}


