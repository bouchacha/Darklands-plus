// padglyphs.cpp - a game controller's buttons drawn on screen (padglyphs.h)
#include "padglyphs.h"
#include <math.h>
#include <string.h>
#include <string>
#include <vector>

int ui_pad_style_force;                        /* (a kind to draw whatever is in use: the settings' pictures, the start's note) */
static int style(void) { if (ui_pad_style_force >= 1 && ui_pad_style_force <= 3) return ui_pad_style_force; int s = plat_prompts(); return s >= 1 && s <= 3 ? s : 1; }
/* the button that does job b now (the player may have given the buttons other jobs: platform_sdl.c) */
extern "C" int plat_pad_bind(int k);
int ui_pad_button_for(int b)
{
    if (b < PB_A || b > PB_LS) return b;
    for (int k = PB_A; k <= PB_LS; k++) if (plat_pad_bind(k) == b) return k;
    return b;
}

/* what the face buttons say, by kind: Xbox's letters, PlayStation's shapes, Nintendo's letters */
static const char *XB_NAME[PB_COUNT] = { "A", "B", "X", "Y", "LB", "RB", "Menu", "View", "LS", "RT", "Left stick", "Right stick", "D-pad" };
static const char *PS_NAME[PB_COUNT] = { "Cross", "Circle", "Square", "Triangle", "L1", "R1", "Options", "Share", "L3", "R2", "Left stick", "Right stick", "D-pad" };
static const char *NS_NAME[PB_COUNT] = { "A", "B", "X", "Y", "L", "R", "+", "-", "L stick", "ZR", "Left stick", "Right stick", "D-pad" };

const char *ui_pad_name(int b)
{
    if (b < 0 || b >= PB_COUNT) return "";
    b = ui_pad_button_for(b);
    int s = style();
    return s == 2 ? PS_NAME[b] : s == 3 ? NS_NAME[b] : XB_NAME[b];
}

static bool face(int b) { return b >= PB_A && b <= PB_Y; }
static bool pill(int b) { return b == PB_LB || b == PB_RB || b == PB_RT || b == PB_LS; }
static const char *pill_text(int b)
{
    int s = style();
    switch (b) {
    case PB_LB: return s == 2 ? "L1" : s == 3 ? "L" : "LB";
    case PB_RB: return s == 2 ? "R1" : s == 3 ? "R" : "RB";
    case PB_RT: return s == 2 ? "R2" : s == 3 ? "ZR" : "RT";
    case PB_LS: return s == 2 ? "L3" : "LS";
    }
    return "";
}

static float width_of(float h, int b);
float ui_pad_glyph_width(float h, int b) { return width_of(h, ui_pad_button_for(b)); }
static float width_of(float h, int b)
{
    if (pill(b)) { ImFont *f = ImGui::GetFont(); return f->CalcTextSizeA(h * 0.58f, FLT_MAX, 0, pill_text(b)).x + h * 0.7f; }
    if (b == PB_START || b == PB_BACK) return style() == 2 ? h * 1.5f : h;
    return h;
}

static void label(ImDrawList *dl, ImVec2 c, float sz, ImU32 col, const char *t)
{
    ImFont *f = ImGui::GetFont();
    ImVec2 ts = f->CalcTextSizeA(sz, FLT_MAX, 0, t);
    dl->AddText(f, sz, ImVec2(roundf(c.x - ts.x * 0.5f), roundf(c.y - ts.y * 0.5f)), col, t);
}

static float glyph_of(ImDrawList *dl, ImVec2 c, float h, int b);
float ui_pad_glyph(ImDrawList *dl, ImVec2 c, float h, int b) { return glyph_of(dl, c, h, ui_pad_button_for(b)); }
float ui_pad_button_glyph(ImDrawList *dl, ImVec2 c, float h, int k) { return glyph_of(dl, c, h, k); }   /* (the button itself, whatever its job) */
static float glyph_of(ImDrawList *dl, ImVec2 c, float h, int b)
{
    int s = style();
    float r = h * 0.5f, w = width_of(h, b), th = fmaxf(1.0f, h * 0.09f);
    const ImU32 DARK = IM_COL32(38, 38, 44, 255), RIM = IM_COL32(0, 0, 0, 110), WHITE = IM_COL32(245, 245, 245, 255);
    if (face(b)) {
        if (s == 1) {                                   /* Xbox: the letter on its colour */
            static const ImU32 C[4] = { IM_COL32(88, 170, 60, 255), IM_COL32(214, 52, 54, 255), IM_COL32(44, 118, 206, 255), IM_COL32(236, 186, 42, 255) };
            dl->AddCircleFilled(c, r, C[b]);
            dl->AddCircle(c, r - th * 0.3f, RIM, 0, th * 0.6f);
            label(dl, c, h * 0.72f, b == PB_Y ? IM_COL32(40, 30, 0, 255) : WHITE, XB_NAME[b]);
        } else if (s == 2) {                            /* PlayStation: the shape in its colour */
            dl->AddCircleFilled(c, r, DARK);
            float q = r * 0.48f;
            switch (b) {
            case PB_A: { ImU32 k = IM_COL32(124, 168, 226, 255);
                dl->AddLine(ImVec2(c.x - q, c.y - q), ImVec2(c.x + q, c.y + q), k, th * 1.3f);
                dl->AddLine(ImVec2(c.x - q, c.y + q), ImVec2(c.x + q, c.y - q), k, th * 1.3f); break; }
            case PB_B: dl->AddCircle(c, q * 1.05f, IM_COL32(236, 96, 92, 255), 0, th * 1.3f); break;
            case PB_X: dl->AddRect(ImVec2(c.x - q * 0.9f, c.y - q * 0.9f), ImVec2(c.x + q * 0.9f, c.y + q * 0.9f), IM_COL32(214, 140, 202, 255), 0, 0, th * 1.3f); break;
            case PB_Y: dl->AddTriangle(ImVec2(c.x, c.y - q * 1.05f), ImVec2(c.x + q * 1.05f, c.y + q * 0.75f), ImVec2(c.x - q * 1.05f, c.y + q * 0.75f), IM_COL32(64, 196, 160, 255), th * 1.3f); break;
            }
        } else {                                        /* Nintendo: a white letter on dark */
            dl->AddCircleFilled(c, r, DARK);
            label(dl, c, h * 0.72f, WHITE, NS_NAME[b]);
        }
        return w;
    }
    if (pill(b)) {
        ImVec2 a(c.x - w * 0.5f, c.y - r * 0.82f), e(c.x + w * 0.5f, c.y + r * 0.82f);
        dl->AddRectFilled(a, e, DARK, r * (b == PB_LS ? 0.82f : 0.35f));
        label(dl, c, h * 0.58f, WHITE, pill_text(b));
        return w;
    }
    if (b == PB_START || b == PB_BACK) {
        if (s == 2) {                                   /* PlayStation: a small pill with lines (Options) or a dot ring (Share) */
            ImVec2 a(c.x - w * 0.5f, c.y - r * 0.6f), e(c.x + w * 0.5f, c.y + r * 0.6f);
            dl->AddRectFilled(a, e, DARK, r * 0.6f);
            if (b == PB_START) for (int i = -1; i <= 1; i++) dl->AddLine(ImVec2(c.x - r * 0.35f, c.y + i * r * 0.24f), ImVec2(c.x + r * 0.35f, c.y + i * r * 0.24f), WHITE, th * 0.8f);
            else { dl->AddLine(ImVec2(c.x - r * 0.3f, c.y + r * 0.2f), ImVec2(c.x, c.y - r * 0.25f), WHITE, th * 0.9f); dl->AddLine(ImVec2(c.x, c.y - r * 0.25f), ImVec2(c.x + r * 0.3f, c.y + r * 0.2f), WHITE, th * 0.9f); }
            return w;
        }
        dl->AddCircleFilled(c, r, DARK);
        if (s == 3) { label(dl, c, h * 0.8f, WHITE, b == PB_START ? "+" : "-"); return w; }
        if (b == PB_START) for (int i = -1; i <= 1; i++) dl->AddLine(ImVec2(c.x - r * 0.42f, c.y + i * r * 0.3f), ImVec2(c.x + r * 0.42f, c.y + i * r * 0.3f), WHITE, th);   /* Xbox Menu: three lines */
        else {                                          /* Xbox View: two windows */
            float q = r * 0.3f;
            dl->AddRect(ImVec2(c.x - q * 1.4f, c.y - q * 1.1f), ImVec2(c.x + q * 0.6f, c.y + q * 0.6f), WHITE, 0, 0, th * 0.8f);
            dl->AddRectFilled(ImVec2(c.x - q * 0.6f, c.y - q * 0.4f), ImVec2(c.x + q * 1.4f, c.y + q * 1.2f), WHITE);
        }
        return w;
    }
    if (b == PB_LSTICK || b == PB_RSTICK) {
        dl->AddCircleFilled(c, r, DARK);
        dl->AddCircle(c, r * 0.62f, IM_COL32(150, 150, 160, 255), 0, th);
        label(dl, c, h * 0.5f, WHITE, b == PB_LSTICK ? "L" : "R");
        return w;
    }
    if (b == PB_DPAD) {
        float q = r * 0.95f, t = r * 0.36f;
        dl->AddRectFilled(ImVec2(c.x - t, c.y - q), ImVec2(c.x + t, c.y + q), DARK, t * 0.4f);
        dl->AddRectFilled(ImVec2(c.x - q, c.y - t), ImVec2(c.x + q, c.y + t), DARK, t * 0.4f);
        return w;
    }
    return 0;
}

void ui_pad_inline(int b)
{
    float h = ImGui::GetTextLineHeight() * 1.05f, w = ui_pad_glyph_width(h, b);
    ImVec2 p = ImGui::GetCursorScreenPos();
    ImGui::Dummy(ImVec2(w, ImGui::GetTextLineHeight()));
    ui_pad_glyph(ImGui::GetWindowDrawList(), ImVec2(p.x + w * 0.5f, p.y + ImGui::GetTextLineHeight() * 0.5f), h, b);
}

static int token(const char *s, size_t n)
{
    static const char *T[PB_COUNT] = { "A", "B", "X", "Y", "LB", "RB", "START", "BACK", "LS", "RT", "LSTICK", "RSTICK", "DPAD" };
    for (int i = 0; i < PB_COUNT; i++) if (strlen(T[i]) == n && !strncmp(s, T[i], n)) return i;
    return -1;
}

void ui_pad_text(const char *text, bool disabled)
{
    /* the pieces: words (with the space after them) and buttons */
    struct Piece { std::string t; int b; };
    std::vector<Piece> ps;
    for (const char *p = text; *p; ) {
        if (*p == '{') {
            const char *e = strchr(p, '}');
            int b = e ? token(p + 1, (size_t)(e - p - 1)) : -1;
            if (b >= 0) { ps.push_back({ "", b }); p = e + 1; continue; }
        }
        const char *q = p;
        while (*q && *q != ' ' && *q != '{') q++;
        while (*q == ' ') q++;
        if (q == p) q++;
        ps.push_back({ std::string(p, q), -1 });
        p = q;
    }
    float wrap = ImGui::GetContentRegionAvail().x, x = 0;
    float h = ImGui::GetTextLineHeight() * 1.05f;
    if (disabled) ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
    bool first = true;
    for (const Piece &pc : ps) {
        float w = pc.b >= 0 ? ui_pad_glyph_width(h, pc.b) + ImGui::GetFontSize() * 0.15f : ImGui::CalcTextSize(pc.t.c_str()).x;
        if (!first) { if (x + w > wrap && x > 0) x = 0; else ImGui::SameLine(0, 0); }
        if (pc.b >= 0) {
            ImVec2 p = ImGui::GetCursorScreenPos();
            ImGui::Dummy(ImVec2(w, ImGui::GetTextLineHeight()));
            ui_pad_glyph(ImGui::GetWindowDrawList(), ImVec2(p.x + (w - ImGui::GetFontSize() * 0.15f) * 0.5f, p.y + ImGui::GetTextLineHeight() * 0.5f), h, pc.b);
        } else ImGui::TextUnformatted(pc.t.c_str());
        x += w; first = false;
    }
    if (disabled) ImGui::PopStyleColor();
}
