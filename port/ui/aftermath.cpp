/* The victory loot shows what happened to the party, using the game's actual
 * recovery, practice rolls and armor wear. The report is frozen when loot opens. */
#include <algorithm>
#include <array>
#include <cstdlib>
#include <cstdio>
#include <string>
#include <vector>
#include "effects.h"
#include "nsdraw.h"
#include "../mods/aftermath.h"
#include "../mods/foefx.h"
#include "../mods/gamemodel.h"
#include "../mods/lootview.h"
#include "../runtime/config.h"
extern "C" {
#include "../runtime/dispatch.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "../runtime/hooks.h"
}

namespace {
constexpr int MEMBERS = 5, SKILLS = 19, ATTRIBUTES = 7;
constexpr u16 LOOT_EXISTS = 0x1b98, LOOT_UNDER_MEMBER = 0xc81c, LOOT_PILES = 0x59c8;
constexpr u16 BATTLE_DRAWING = 0x07d8;
constexpr ImU32 BLUE = IM_COL32(41, 70, 105, 255);
const char *const TEMPORARY = "Temporary effects";
using SkillBoosts = std::array<std::array<int, SKILLS>, MEMBERS>;

struct Member {
    std::string name;
    int status = 0;
    std::array<int, SKILLS> skills;
    std::array<int, ATTRIBUTES> maxima;
};
struct Wear {
    int member, item, before, after, count;
};
struct Battle {
    bool tracking = false, checking_end = false, report_next_loot = false;
    int deferred_loot = -1;
    std::array<Member, MEMBERS> before;
    std::array<int, MEMBERS> fallen;
    SkillBoosts temporary_skill_writes;
    std::vector<Eff> effects;
    std::vector<Wear> wear;
} battle = {};

struct Entry {
    std::string text;
    ImU32 color;
    int item = -1;
};
struct ReportMember {
    int slot;
    std::string name;
    std::vector<Entry> entries;
};
std::vector<ReportMember> changes, temporary;
int changes_scroll = 0, temporary_scroll = 0;

bool enabled()
{
    return lootview_is_on() && ns_active() && config_get_int("battle.aftermath", 1);
}

/* The combat-only percentage penalty is kept apart from the timed events.
 * Dungeon loot can open while that penalty is still on the skill. */
int skill_without_field_penalty(int m, int k)
{
    int skill = gm_member_skill(m, k);
    u16 modifiers = GV_cbt_mods + 0x12 * m;
    if (k < 7 && (s16)g_rw(modifiers + 0x0e) != -536)
        skill += g_rb(modifiers + 2 + k);
    return skill;
}

void snapshot()
{
    battle.effects = effects_snapshot(MEMBERS);
    for (int m = 0; m < MEMBERS; m++) {
        Member &member = battle.before[m];
        member.status = gm_member_status(m);
        if (!member.status) continue;
        char name[24];
        gm_member_name(m, name, sizeof name);
        member.name = ns_gtext(name);
        for (int k = 0; k < SKILLS; k++) member.skills[k] = skill_without_field_penalty(m, k);
        for (int k = 0; k < ATTRIBUTES; k++) {
            int current;
            gm_member_attr(m, k, &current, &member.maxima[k]);
        }
    }
    battle.fallen.fill(0);
    battle.temporary_skill_writes = {};
    battle.wear.clear();
}

void record_fall(int m, int previous)
{
    int status = gm_member_status(m);
    if ((status == 2 || status == 3) && status != previous)
        battle.fallen[m] = std::max(battle.fallen[m], status);
}

void damage_hook()
{
    int m = (s16)rw(SS, SP + 4);
    bool ours = battle.tracking && m >= 0 && m < MEMBERS;
    int previous = ours ? gm_member_status(m) : 0;
    hook_call_original();
    if (ours && !unw_active) record_fall(m, previous);
}

void recover_hook()
{
    if (battle.tracking)
        for (int m = 0; m < MEMBERS; m++) record_fall(m, battle.before[m].status);
    hook_call_original();
}

void temporary_skill_hook()
{
    int k = (s16)rw(SS, SP + 6);
    if (!battle.tracking || k < 0 || k >= SKILLS) {
        hook_call_original();
        return;
    }
    std::array<int, MEMBERS> before;
    for (int m = 0; m < MEMBERS; m++) before[m] = gm_member_skill(m, k);
    hook_call_original();
    if (!unw_active)
        for (int m = 0; m < MEMBERS; m++)
            battle.temporary_skill_writes[m][k] += gm_member_skill(m, k) - before[m];
}

void over_hook()
{
    battle.checking_end = battle.tracking;
    hook_call_original();
    battle.checking_end = false;
}

/* The same two early exits as the original loot_pile_open. Only a pile that
 * will really show the screen may take over the original practice messages. */
bool pile_opens(int m)
{
    if (g_rb(LOOT_EXISTS) != 1) return false;
    int pile = (s8)g_rb(LOOT_UNDER_MEMBER + m);
    return pile < 0 || rb(G_FAR20A5, LOOT_PILES + 12 * pile + 2) != 0;
}

void pile_hook()
{
    int m = (s16)rw(SS, SP + 4);
    if (!battle.checking_end || !enabled() || m < 0 || m >= MEMBERS || !pile_opens(m)) {
        hook_call_original();
        return;
    }
    /* combat_over opens the victory pile before combat_main rolls practice.
     * Open terminal victory loot after that cleanup, while the battle's own
     * loot buffers and item handling are still live. A dungeon can resume
     * exploration here, so its intermediate victory piles stay where they are. */
    if (!(g_rb(GV_combat_flags) & 1) || g_rb(GV_place_won) == 1 || g_rb(GV_demo_mode)) {
        battle.deferred_loot = m;
        hook_return_far(0);
        return;
    }
    battle.report_next_loot = true;
    hook_call_original();
    battle.report_next_loot = false;
    if (!unw_active) snapshot();
}

SkillBoosts skill_boosts(const std::vector<Eff> &effects)
{
    SkillBoosts totals = {};
    for (const Eff &e : effects)
        if (e.kind == 0x49) totals[e.m][e.k] += e.amount;
    return totals;
}

std::string with_shield(int key, const std::string &text)
{
    return std::string("{") + ui_key_name(key) + "} " + text;
}

std::string delta(int before, int after)
{
    return (after >= before ? "+" : "-") + std::to_string(abs(after - before)) + " (now " + std::to_string(after) + ")";
}

int bounded_skill(int skill) { return std::max(0, std::min(skill, 99)); }

void lasting_changes(ReportMember &row, const SkillBoosts &old_boost)
{
    int m = row.slot, status = gm_member_status(m);
    const Member &before = battle.before[m];
    if (battle.fallen[m] == 3) row.entries.push_back({ "Died in battle", NsDraw::RED });
    else if (battle.fallen[m] == 2)
        row.entries.push_back({ status == 1 ? "Fell unconscious, recovered" : "Fell unconscious", NsDraw::RED });

    for (const Wear &wear : battle.wear) {
        if (wear.member != m) continue;
        std::string text = ns_item_name(wear.item);
        if (wear.count > 1) text += " (" + std::to_string(wear.count) + ")";
        std::string was = std::to_string(wear.before) + "q";
        if (wear.after > 0) text += " " + was + " to " + std::to_string(wear.after) + "q";
        else text += " destroyed (was " + was + ")";
        row.entries.push_back({ text, NsDraw::RED, wear.item });
    }
    if (status != 1 && status != 2) return;

    for (int k = 0; k < ATTRIBUTES; k++) {
        int current, maximum;
        gm_member_attr(m, k, &current, &maximum);
        if (maximum < before.maxima[k])
            row.entries.push_back({ with_shield(k, std::string(gm_attr_name(k)) + " max " + delta(before.maxima[k], maximum)), NsDraw::RED });
    }
    for (int k = 0; k < SKILLS; k++) {
        int was = bounded_skill(before.skills[k] - old_boost[m][k]);
        /* The event can record a larger penalty than the game could apply at zero.
         * Subtract what was actually written, even for successive opposing effects. */
        int change = skill_without_field_penalty(m, k) - before.skills[k] - battle.temporary_skill_writes[m][k];
        int now = bounded_skill(was + change);
        if (was != now)
            row.entries.push_back({ with_shield(ATTRIBUTES + k, std::string(gm_skill_name(k)) + " " + delta(was, now)),
                                    now > was ? NsDraw::GREEN : NsDraw::RED });
    }
}

bool same_effect(const Eff &a, const Eff &b)
{
    int amount_a = a.recorded_amount ? a.recorded_amount : a.amount;
    int amount_b = b.recorded_amount ? b.recorded_amount : b.amount;
    return a.m == b.m && a.kind == b.kind && a.k == b.k && amount_a == amount_b && a.until == b.until;
}

void temporary_changes(ReportMember &row, const std::vector<Eff> &effects, std::vector<bool> &matched)
{
    for (const Eff &e : effects) {
        if (e.m != row.slot || e.kind == 0x43) continue;
        size_t k = 0;
        for (; k < battle.effects.size(); k++)
            if (!matched[k] && same_effect(e, battle.effects[k])) { matched[k] = true; break; }
        if (k < battle.effects.size() || !e.amount) continue;
        std::string text = e.what + " (" + std::to_string(e.left) + " h left)";
        if (e.kind == 0x48 || e.kind == 0x49) text = with_shield(e.k + (e.kind == 0x49 ? ATTRIBUTES : 0), text);
        else if (e.icon >= 0) text = rich_effect(e.icon) + " " + text;
        row.entries.push_back({ text, BLUE });
    }

    FoeFx field[3];
    int n = foefx_of(row.slot, field, 3);
    for (int k = 0; k < n; k++) {
        const FoeFx &f = field[k];
        const char *what = f.what == FX_BLIND ? "Blinded" : f.what == FX_WEAKENED ? "Weakened" : "Weapon skills lowered";
        row.entries.push_back({ std::string(what) + " (" + std::to_string((f.left + 59) / 60) + "s left in battle)", BLUE, f.potion });
    }
}

/* Pack whole entries across the page. A long entry wraps on its own line and
 * a member who needs another line keeps the same left-hand name column. */
constexpr float LEFT = 63, RIGHT = 317, TOP = 20;
constexpr float HEAD_H = 10, MAX_BODY_H = 36, GAP = 5, ITEM_W = 8.5f;
struct Line {
    const ReportMember *member;
    size_t first, last;
    float height;
};
struct Section {
    float left, right, details_x, height = 0;
    std::vector<Line> lines;
};

float entry_width(const NsDraw &p, const Entry &entry)
{
    return p.width(p.fo, NsDraw::FS_BODY, entry.text) + (entry.item >= 0 ? ITEM_W : 0);
}

Section layout_section(const NsDraw &p, const std::vector<ReportMember> &members, float left, float right)
{
    Section section = { left, right, left + 12 };
    for (const ReportMember &member : members) {
        float end_of_name = left + 12 + p.width(p.f, NsDraw::FS_NAME, member.name);
        section.details_x = std::max(section.details_x, end_of_name);
    }
    section.details_x += 5;
    float width = right - 4 - section.details_x;
    float row_height = std::max(7.5f, p.size(p.fo, NsDraw::FS_BODY) / p.k + 1.5f);
    for (const ReportMember &member : members) {
        size_t start = 0;
        while (start < member.entries.size()) {
            const Entry &entry = member.entries[start];
            float used = entry_width(p, entry);
            float height = row_height;
            size_t end = start + 1;
            if (used > width) {
                float icon = entry.item >= 0 ? ITEM_W : 0;
                height = std::max(height, p.para_height(p.fo, NsDraw::FS_BODY, width - icon, entry.text) + 2);
            } else {
                for (; end < member.entries.size(); end++) {
                    float next = GAP + entry_width(p, member.entries[end]);
                    if (used + next > width) break;
                    used += next;
                }
            }
            section.lines.push_back({ &member, start, end, height });
            section.height += height;
            start = end;
        }
    }
    return section;
}

void draw_line(NsDraw &p, const Section &section, const Line &line, float y, bool show_name)
{
    const ReportMember &member = *line.member;
    if (show_name || line.first == 0) {
        ui_member_head(p.dl, member.slot, NsDraw::w(section.left + 4, y + 0.8f), 6.0f * p.k);
        p.text(p.f, NsDraw::FS_NAME, section.left + 12, y + 0.8f, NsDraw::INK, member.name);
    }
    float x = section.details_x;
    for (size_t k = line.first; k < line.last; k++) {
        const Entry &entry = member.entries[k];
        if (k != line.first) {
            p.dl->AddLine(NsDraw::w(x + GAP * 0.5f, y + 2), NsDraw::w(x + GAP * 0.5f, y + 6.5f), IM_COL32(146, 112, 66, 150));
            x += GAP;
        }
        if (entry.item >= 0) {
            p.icon(x + 3.2f, y + 3.9f, 5.1f, entry.item);
            x += ITEM_W;
        }
        p.para(p.fo, NsDraw::FS_BODY, x, y + 0.8f, section.right - 4 - x, entry.color, entry.text);
        x += p.width(p.fo, NsDraw::FS_BODY, entry.text);
    }
}

void draw_section(NsDraw &p, const Section &section, float bottom, float body, bool is_temporary, bool single, int &scroll)
{
    if (section.lines.empty()) return;
    float left = section.left, right = section.right;
    int last = (int)section.lines.size() - 1;
    for (float height = section.lines[last].height; last > 0 && height + section.lines[last - 1].height <= body + 0.01f; )
        height += section.lines[--last].height;
    if (p.mx >= left && p.mx < right && p.my >= TOP && p.my < bottom && ns_in.wheel) {
        scroll -= ns_in.wheel > 0 ? 1 : -1;
        ns_in.wheel = 0;
    }
    scroll = std::max(0, std::min(last, scroll));

    ImU32 head = is_temporary && !single ? IM_COL32(222, 223, 211, 255) : IM_COL32(229, 212, 176, 255);
    p.rect(left, TOP, right, bottom, IM_COL32(249, 239, 215, 255), 1.5f);
    p.rect(left + 0.6f, TOP + 0.6f, right - 0.6f, TOP + HEAD_H, head, 1);
    p.frame(left, TOP, right, bottom, IM_COL32(125, 87, 43, 120), 1.5f);
    const char *title = is_temporary && !single ? TEMPORARY : "Battle aftermath";
    p.text(p.f, NsDraw::FS_HEAD, left + 4, TOP + 1, is_temporary && !single ? BLUE : NsDraw::INK, title);
    if (is_temporary && single) {
        float label_x = left + 9 + p.width(p.f, NsDraw::FS_HEAD, title);
        p.text(p.fo, NsDraw::FS_BODY, label_x, TOP + 2.2f, BLUE, TEMPORARY);
    }
    if (last > 0) {
        ImU32 button = IM_COL32(120, 84, 40, 235);
        if (p.arrow_button(right - 24, TOP + 1.3f, right - 15, TOP + 8.7f, true, scroll > 0, button)) scroll--;
        if (p.arrow_button(right - 13, TOP + 1.3f, right - 4, TOP + 8.7f, false, scroll < last, button)) scroll++;
    }

    float y = TOP + HEAD_H;
    for (size_t k = scroll; k < section.lines.size() && y + section.lines[k].height <= bottom - 1.5f; k++) {
        draw_line(p, section, section.lines[k], y, k == (size_t)scroll);
        y += section.lines[k].height;
    }
}
} // namespace

extern "C" void aftermath_begin(void)
{
    battle = {};
    battle.tracking = enabled();
    if (battle.tracking) snapshot();
}

extern "C" void aftermath_end(void)
{
    if (!unw_active && battle.deferred_loot >= 0) {
        CPU registers = R;
        int tail = tail_pending;
        u8 drawing = g_rb(BATTLE_DRAWING);
        battle.report_next_loot = true;
        ns_call(GF_loot_pile_open, 1, battle.deferred_loot);
        if (!ns_unwinding) {
            g_wb(BATTLE_DRAWING, drawing);   /* loot_pile_open turns battle drawing back on */
            R = registers;
            tail_pending = tail;
        }
    }
    battle = {};
}

extern "C" int aftermath_victory_loot_pending(void) { return battle.deferred_loot >= 0; }

extern "C" void aftermath_armor_worn(int m, int id, int before, int after, int count)
{
    if (!battle.tracking) return;
    for (auto it = battle.wear.rbegin(); it != battle.wear.rend(); ++it) {
        if (it->member == m && it->item == id && it->after == before && it->count == count) {
            it->after = after;
            return;
        }
    }
    battle.wear.push_back({ m, id, before, after, count });
}

extern "C" void aftermath_open(void)
{
    changes.clear();
    temporary.clear();
    changes_scroll = temporary_scroll = 0;
    if (!battle.report_next_loot) return;
    battle.report_next_loot = false;
    auto effects = effects_snapshot(MEMBERS);
    auto old_boost = skill_boosts(battle.effects);
    std::vector<bool> matched(battle.effects.size());
    for (int m = 0; m < MEMBERS; m++) {
        const Member &member = battle.before[m];
        if (!member.status) continue;
        ReportMember lasting = { m, member.name };
        lasting_changes(lasting, old_boost);
        if (!lasting.entries.empty()) changes.push_back(lasting);
        int status = gm_member_status(m);
        if (status != 1 && status != 2) continue;
        ReportMember current = { m, member.name };
        temporary_changes(current, effects, matched);
        if (!current.entries.empty()) temporary.push_back(current);
    }
    std::stable_sort(changes.begin(), changes.end(), [](const ReportMember &a, const ReportMember &b) {
        return battle.fallen[a.slot] > battle.fallen[b.slot];     /* casualties first, even in a long report */
    });
    if (getenv("DL_LOOT_LOG")) {
        for (const auto &row : changes) for (const Entry &entry : row.entries)
            fprintf(stderr, "aftermath: %s: %s\n", row.name.c_str(), rich_plain(entry.text.c_str()).c_str());
        for (const auto &row : temporary) for (const Entry &entry : row.entries)
            fprintf(stderr, "aftermath (temporary): %s: %s\n", row.name.c_str(), rich_plain(entry.text.c_str()).c_str());
    }
}

extern "C" void aftermath_init(void)
{
    hook_install(GF_combat_over, over_hook);
    hook_install(GF_combat_recover, recover_hook);
    hook_install(GF_damage_apply, damage_hook);
    hook_install(GF_lower_skill_temp, temporary_skill_hook);
    hook_install(GF_loot_pile_open, pile_hook);
}

float aftermath_draw(NsDraw &p)
{
    if (changes.empty() && temporary.empty()) return 0;
    bool split = !changes.empty() && !temporary.empty();
    const float middle = LEFT + (RIGHT - LEFT) * 0.58f;
    Section lasting = layout_section(p, changes, LEFT, split ? middle - 1 : RIGHT);
    Section timed = layout_section(p, temporary, split ? middle + 1 : LEFT, RIGHT);
    float body = std::min(std::max(lasting.height, timed.height), MAX_BODY_H);
    float bottom = TOP + HEAD_H + body + 2;
    draw_section(p, lasting, bottom, body, false, !split, changes_scroll);
    draw_section(p, timed, bottom, body, true, !split, temporary_scroll);
    return bottom - TOP + 4;
}
