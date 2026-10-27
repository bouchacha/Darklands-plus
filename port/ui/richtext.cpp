// richtext.cpp - text with the attributes' and skills' shields in it (richtext.h): the text cut into words
// and shields, laid out a line at a time like ImGui's own wrapped text, each shield the size of the
// type's capitals (UI_SHIELD_W wide), its value after it; shields side by side (a group sharing one value)
// joined by a bracket under them; a member's head ({@m}) square, the type's height. With display.coins on, an
// amount in words is given its coin first (money_words). The shields and heads drawn last are kept, so that the
// pointer on one names it.
#include "richtext.h"
#include "imgui_internal.h"
#include "skillicons.h"
#include "money.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <float.h>
#include <vector>
#include <algorithm>
#include <ctype.h>

namespace {
struct Atom { int key; std::string text; bool space; bool br; };   /* key -1: words; br: a line break before it */
struct Hit { ImVec2 a, b; int key; };
std::vector<Hit> hits;                                              /* the shields of the last rich_draw */
/* a shield's {Name} or {Name:value} at s[i] ('{'): its key, the value after the shield ("" none) and the end; -1 none */
int token_at(const char *s, size_t i, std::string *value, size_t *end)
{
    const char *e = strchr(s + i, '}');
    if (!e) return -1;
    const char *colon = (const char *)memchr(s + i, ':', (size_t)(e - (s + i)));
    const char *ne = colon ? colon : e;
    int key = ne == s + i + 3 && s[i + 1] == '@' && s[i + 2] >= '0' && s[i + 2] <= '4' ? RICH_MEMBER + (s[i + 2] - '0')
                                                                                     : ui_key_of(s + i + 1, (int)(ne - (s + i + 1)));
    if (key < 0) return -1;
    *value = colon ? std::string(colon + 1, e) : std::string();
    *end = (size_t)(e - s) + 1;
    return key;
}
std::vector<Atom> atoms_of(const char *s0)
{
    std::string coined = money_words(s0);
    const char *s = coined.c_str();
    std::vector<Atom> out;
    bool space = false, br = false;
    std::string word;
    auto flush = [&]() { if (!word.empty()) { out.push_back({ -1, word, space, br }); space = br = false; word.clear(); } };
    for (size_t i = 0; s[i]; ) {
        char c = s[i];
        if (c == '\n') { flush(); br = true; space = false; i++; continue; }
        if (c == ' ') { flush(); space = true; i++; continue; }
        if (c == '{') {
            std::string value; size_t end;
            int key = token_at(s, i, &value, &end);
            if (key >= 0) { flush(); out.push_back({ key, value, space, br }); space = br = false; i = end; continue; }
        }
        word += c; i++;
    }
    flush();
    return out;
}
float shield_size(float fs) { return fs * 0.66f; }
float pic_width(float fs, int key) { return key >= RICH_MEMBER ? fs * 0.92f / 1.2f : UI_SHIELD_W * shield_size(fs); }   /* a head; a shield */
float atom_width(ImFont *f, float fs, const Atom &a)
{
    float w = a.text.empty() ? 0 : f->CalcTextSizeA(fs, FLT_MAX, 0, a.text.c_str()).x;
    if (a.key < 0) return w;
    return pic_width(fs, a.key) + (a.text.empty() ? 0 : shield_size(fs) * 0.25f + w);
}
/* a shield alone, straight after another (no space): one of a group */
bool grouped(const std::vector<Atom> &at, size_t i) { return i + 1 < at.size() && at[i].key >= 0 && at[i].text.empty() && at[i + 1].key >= 0 && !at[i + 1].space && !at[i + 1].br; }
/* lay the atoms out: each one's place (x from the left, the line) and the whole size */
ImVec2 layout(ImFont *f, float fs, float wrap, const std::vector<Atom> &at, std::vector<ImVec2> *pos)
{
    float sp = f->CalcTextSizeA(fs, FLT_MAX, 0, " ").x, x = 0, wmax = 0;
    int line = 0;
    bool first = true;
    for (size_t i = 0; i < at.size(); i++) {
        const Atom &a = at[i];
        float w = atom_width(f, fs, a);
        if (a.br) { line++; x = 0; first = true; }
        float gap = first ? 0 : a.space ? sp : a.key >= 0 && i > 0 && at[i - 1].key >= 0 ? fs * 0.06f : 0;
        if (!first && wrap > 0 && x + gap + w > wrap && a.space) { line++; x = 0; gap = 0; }   /* (only where there is a space: "{Strength}," stays together) */
        if (pos) pos->push_back(ImVec2(x + gap, line * fs));
        x += gap + w; first = false;
        if (x > wmax) wmax = x;
    }
    return at.empty() ? ImVec2(0, fs) : ImVec2(wmax, (line + 1) * fs);
}
}

bool rich_has(const char *s)
{
    for (const char *p = s ? strchr(s, '{') : nullptr; p; p = strchr(p + 1, '{')) {
        std::string value; size_t end;
        if (token_at(s, (size_t)(p - s), &value, &end) >= 0) return true;
    }
    return s && money_coins() && money_words(s) != s;
}

std::string rich_member(int m, const std::string &value)
{
    return "{@" + std::to_string(m) + (value.empty() ? "" : ":" + value) + "}";
}
std::string rich_key_name(int key) { return key >= RICH_MEMBER ? ui_member_name(key - RICH_MEMBER) : ui_key_name(key); }

ImVec2 rich_size(ImFont *f, float fs, float wrap, const char *s)
{
    std::vector<Atom> at = atoms_of(s);
    return layout(f, fs, wrap, at, nullptr);
}

ImVec2 rich_draw(ImDrawList *dl, ImFont *f, float fs, ImVec2 pos, float wrap, ImU32 col, const char *s)
{
    std::vector<Atom> at = atoms_of(s);
    std::vector<ImVec2> ps;
    ImVec2 size = layout(f, fs, wrap, at, &ps);
    float is = shield_size(fs);
    hits.clear();
    for (size_t i = 0; i < at.size(); i++) {
        const Atom &a = at[i];
        ImVec2 p(pos.x + ps[i].x, pos.y + ps[i].y);
        if (a.key < 0) { dl->AddText(f, fs, p, col, a.text.c_str()); continue; }
        float sw = pic_width(fs, a.key);
        int v0 = dl->VtxBuffer.Size;
        if (a.key >= RICH_MEMBER) ui_member_head(dl, a.key - RICH_MEMBER, ImVec2(p.x, p.y + fs * 0.04f), fs * 0.92f);
        else {
            extern int ui_icon_disc; int disc = ui_icon_disc; ui_icon_disc = 0;
            ui_key_icon(dl, ImVec2(p.x + sw * 0.5f, p.y + fs * 0.52f), is, a.key);
            ui_icon_disc = disc;
        }
        unsigned al = (col >> IM_COL32_A_SHIFT) & 255;
        if (al < 250) ui_dim_since(dl, v0, al / 255.0f);          /* (a greyed or fading text: its shields with it) */
        hits.push_back({ p, ImVec2(p.x + sw, p.y + fs), a.key });
        if (!a.text.empty()) dl->AddText(f, fs, ImVec2(p.x + sw + is * 0.25f, p.y), col, a.text.c_str());
        if (grouped(at, i) && (i == 0 || !grouped(at, i - 1))) {    /* a group's first shield: a bracket under the group */
            size_t j = i; while (grouped(at, j) && ps[j + 1].y == ps[i].y) j++;
            float x0 = p.x + sw * 0.15f, x1 = pos.x + ps[j].x + pic_width(fs, at[j].key) * 0.85f, y = p.y + fs * 1.03f, t = fmaxf(1.0f, fs * 0.07f), tk = fs * 0.14f;
            ImU32 bc = (col & ~IM_COL32_A_MASK) | ((ImU32)(((col >> IM_COL32_A_SHIFT) & 255) * 0.75f) << IM_COL32_A_SHIFT);
            ImVec2 br[4] = { ImVec2(x0, y - tk), ImVec2(x0, y), ImVec2(x1, y), ImVec2(x1, y - tk) };
            dl->AddPolyline(br, 4, bc, 0, t);
        }
    }
    return size;
}

int rich_key_at(ImVec2 m)
{
    for (const Hit &h : hits) if (m.x >= h.a.x && m.x < h.b.x && m.y >= h.a.y && m.y < h.b.y) return h.key;
    return -1;
}

std::string rich_plain(const char *s)
{
    std::string out;
    for (size_t i = 0; s[i]; ) {
        std::string value; size_t end;
        int key = s[i] == '{' ? token_at(s, i, &value, &end) : -1;
        if (key < 0) { out += s[i++]; continue; }
        const char *colon = (const char *)memchr(s + i, ':', end - i);
        out += key >= RICH_MEMBER ? rich_key_name(key) : std::string(s + i + 1, colon ? colon : s + end - 1);   /* (the name as written) */
        if (!value.empty()) out += " " + value;
        i = end;
    }
    return out;
}

std::string rich_auto(const char *s)
{
    static std::vector<std::string> names;                      /* every name a key answers to, the longest first */
    if (names.empty()) {
        static const char *const MORE[] = { "Edged", "Impact", "Thrown", "Latin", "Missile Devices" };
        for (int k = 0; k < UI_KEY_REP; k++) names.push_back(ui_key_name(k));
        for (const char *m : MORE) names.push_back(m);
        std::sort(names.begin(), names.end(), [](const std::string &a, const std::string &b) { return a.size() > b.size(); });
    }
    std::string out;
    int depth = 0;
    for (size_t i = 0; s[i]; ) {
        if (s[i] == '{') depth++; else if (s[i] == '}' && depth) depth--;
        bool start = i == 0 || !isalnum((unsigned char)s[i - 1]);
        const std::string *hit = nullptr;
        if (!depth && start && isupper((unsigned char)s[i]))
            for (const std::string &n : names)
                if (!strncmp(s + i, n.c_str(), n.size()) && !isalnum((unsigned char)s[i + n.size()])) { hit = &n; break; }
        if (hit && ui_key_of(hit->c_str(), (int)hit->size()) >= 0) { out += "{" + *hit + "}"; i += hit->size(); continue; }
        out += s[i++];
    }
    return out;
}

void ui_rich_text(const char *s)
{
    if (!rich_has(s)) { ImGui::TextUnformatted(s); return; }
    ImGuiWindow *w = ImGui::GetCurrentWindow();
    if (w->SkipItems) return;
    ImVec2 pos = ImGui::GetCursorScreenPos();
    float wrap = w->DC.TextWrapPos >= 0 ? ImGui::CalcWrapWidthForPos(pos, w->DC.TextWrapPos) : 0;
    ImVec2 size = rich_draw(w->DrawList, ImGui::GetFont(), ImGui::GetFontSize(), pos, wrap, ImGui::GetColorU32(ImGuiCol_Text), s);
    ImGui::ItemSize(size);
    if (!(w->Flags & ImGuiWindowFlags_Tooltip) && ImGui::IsWindowHovered()) {   /* (a shield under the pointer: its name; not inside a tooltip) */
        int k = rich_key_at(ImGui::GetIO().MousePos);
        if (k >= 0) ui_rich_tip_later(k);
    }
}

void ui_rich_textf(const char *fmt, ...)
{
    char b[2048];
    va_list ap; va_start(ap, fmt); vsnprintf(b, sizeof b, fmt, ap); va_end(ap);
    ui_rich_text(b);
}

void ui_rich_textf_c(const ImVec4 &col, const char *fmt, ...)
{
    char b[2048];
    va_list ap; va_start(ap, fmt); vsnprintf(b, sizeof b, fmt, ap); va_end(ap);
    ImGui::PushStyleColor(ImGuiCol_Text, col); ui_rich_text(b); ImGui::PopStyleColor();
}

void ui_rich_textf_dim(const char *fmt, ...)
{
    char b[2048];
    va_list ap; va_start(ap, fmt); vsnprintf(b, sizeof b, fmt, ap); va_end(ap);
    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled)); ui_rich_text(b); ImGui::PopStyleColor();
}

static int tip_key = -1, tip_frame = -1, taken_frame = -1;
void ui_rich_tip_later(int key) { tip_key = key; tip_frame = ImGui::GetFrameCount(); }
void ui_rich_tip_taken(void) { taken_frame = ImGui::GetFrameCount(); }
void ui_rich_tip_flush(void)
{
    int f = ImGui::GetFrameCount();
    ImGuiWindow *prev = GImGui->TooltipPreviousWindow;       /* (any tooltip opened this frame, by any screen: it has the place) */
    bool other = taken_frame == f || (prev && prev->LastFrameActive == f);
    if (tip_frame == f && !other && tip_key >= 0) ImGui::SetTooltip("%s", rich_key_name(tip_key).c_str());
    tip_key = -1;
}
