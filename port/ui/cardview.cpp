/* cardview.cpp - draws the card screen (mods/cardview.c runs it): the card's picture stays the
 * game's own (with its palette, frame and any replacement art), the text and the options are
 * set here in the chosen typeface, sized to the text block the card file gives for them.
 *
 *  - the narrative in paragraphs, the first letter as an illuminated initial (the original
 *    drew a 21-pixel picture of it);
 *  - the options as rows: the hovered one highlighted, a number for the keys, the chance of
 *    success at the end of the row, options that cannot be chosen greyed;
 *  - an option that calls on a saint or uses an item opens, under it, the list of who can
 *    (the original's pop-up box beside the option), each with its tooltip;
 *  - while the options are up the picture is dimmed (the original faded it);
 *  - everything shrinks a little when a long text would run off the card.
 * The rows' places go back to the engine in game pixels (cardview.row_y0 ...), where the
 * game's own pointer is tested against them. */
#include "padglyphs.h"
#include "imgui.h"
#include <math.h>
#include <string.h>
#include <string>
#include <vector>
#include "../runtime/cpu.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "../runtime/config.h"
#include "../mods/cardview.h"
#include "../mods/nscreen.h"
#include "../mods/assist.h"
#include "ui.h"
#include "ornament.h"
#include "gamedata.h"
#include "money.h"
#include "teachersui.h"
#include "skillicons.h"
#include "richtext.h"
#include "gamepics.h"
#include "quests.h"

extern ImFont *ui_font_text, *ui_font_overlay;
extern ImFont *ui_font_game[UI_GAME_FONTS], *ui_font_game_hi[UI_GAME_FONTS];
extern int ui_game_font;
extern "C" void plat_game_to_win(float gx, float gy, float *wx, float *wy);
extern "C" int cardview_dialog, game_dialog_kind;
extern "C" { void gm_member_figure_code(int m, char *out, int n); void gm_member_palette(int m, unsigned char *mp); const char *gm_skill_name(int k); const char *gm_attr_name(int a); }
const GamePic *ns_figure_head(int cache, const char *code, const unsigned char *mp, ImVec2 *uv0, ImVec2 *uv1);   /* ui/partypanel.cpp */
/* the chips of who speaks (an option where the player picks): where they were drawn, for the click (ui_event) */
static float chip_r[25][4]; static int chip_opt[25], chip_m[25], n_chips;
static int slide_line = -1; static float slide_t;                /* (the option whose members' chances are slid out under the pointer, and how far) */
extern "C" int cardview_chip_click(int wx, int wy)
{
    for (int i = 0; i < n_chips; i++)
        if (wx >= chip_r[i][0] && wx < chip_r[i][2] && wy >= chip_r[i][1] && wy < chip_r[i][3]) {
            assist_pick_speaker(chip_opt[i], chip_m[i]); cardview_choose(chip_opt[i]); n_chips = 0;
            return 1;
        }
    return 0;
}
/* who tries an option: the member's head, as on the character sheet's saints (from the figure beside a battle) */
static bool member_head(ImDrawList *dl, int m, ImVec2 p0, float sz)
{
    char code[8]; gm_member_figure_code(m, code, sizeof code);
    unsigned char mp[24]; gm_member_palette(m, mp);
    ImVec2 u0, u1;
    const GamePic *g = ns_figure_head(60 + m, code, mp, &u0, &u1);
    if (!g) return false;
    dl->AddImage((ImTextureID)(intptr_t)g->tex, p0, ImVec2(p0.x + sz, p0.y + sz), u0, u1);
    return true;
}
extern "C" int worldmap_docks_inset(float x0, float y0, float x1, float y1, int here, const int *dest, const int *opt, const int *state, const char *const *fare, int n, int hover_opt);
extern "C" u32 plat_ticks_ms(void);
extern "C" const char *card_hint(const char *card, int opt);     /* ui/cardhints.cpp */
static const char *greyed_hint(const CvPiece &p)                  /* why this greyed option cannot be chosen, if known */
{
    if (!assist_card_help || !p.is_opt || p.state != 2) return nullptr;
    char nm[16]; g_str(G_DGROUP, GV_card_name, nm, sizeof nm);
    return card_hint(nm, p.opt);
}
float ui_face_k(ImFont *f);                                   /* overlay.cpp */
void ui_card_tip(ImVec2 pos, ImVec2 pivot, int item, int saint);
void ui_card_formula_tip(ImVec2 pos, ImVec2 pivot, int formula);
extern "C" {
#include "../mods/gamemodel.h"
extern "C" int ui_is_open(void);
}
void ui_item_icon_q(ImDrawList *dl, ImVec2 c, float s, int id, int q);
void ui_item_icon_tier(ImDrawList *dl, ImVec2 c, float s, int id, int tier);
/* the quality of member m's best thing id (-1 none): a potion's picture shows its grade by it */
static int best_quality(int m, int id)
{
    int q = -1; GmItem it;
    for (int i = 0; i < gm_item_count(m); i++) if (gm_item(m, i, &it) && it.id == id && it.qual > q) q = it.qual;
    return q;
}

/* a card that sells formulae ($Alche01: "the following formulae can be purchased"): its four
 * card_items are the potions, $Text1..4 the formulae's names, $Money1..4 their prices; the formula
 * of slot s (gamedata order, potion 95 + f / 3), or -1 */
static int cv_formula_of_slot(int s)
{
    if (s < 0 || s > 3) return -1;
    char nm[16]; g_str(G_DGROUP, GV_card_name, nm, sizeof nm);
    if (strncasecmp(nm, "$Alche01", 8)) return -1;
    char t[48]; g_str(G_DGROUP, (u16)(GV_var_text1 + 40 * s), t, sizeof t);
    std::string want = game_text(t, strlen(t));
    int pot = (s16)g_rw((u16)(GV_card_items + 2 * s));
    for (int f = 0; f < (int)gamedata.formula_names.size(); f++)
        if (gamedata.formula_game_names[f] == want && 95 + f / 3 == pot) return f;
    return -1;
}
static std::string cv_formula_price(int s)
{
    int fl = (s16)g_rw((u16)(GV_var_money1 + 6 * s)), gr = (s16)g_rw((u16)(GV_var_money1 + 6 * s + 2)), pf = (s16)g_rw((u16)(GV_var_money1 + 6 * s + 4));
    return money_fgp(fl, gr, pf);
}

static ImVec2 g2w(float x, float y) { float a, b; plat_game_to_win(x, y, &a, &b); return ImVec2(a, b); }

namespace {                       /* (file-local: ui/quests.cpp has a struct Line of its own, and two
                                     classes of one name in two files break each other's vectors) */
struct Line { std::string t; float x, y; };
struct Laid {
    std::vector<Line> lines;
    float y0, y1;            /* the row (options), window pixels */
    float text_end_x, last_y;
};
}

/* words of t, wrapped to width w (the first `indent_lines` lines narrower by `indent`) */
static void wrap(ImFont *f, float size, const std::string &t, float w, float indent, int indent_lines,
                 std::vector<std::string> &out, float cont = 0)
{
    out.clear();
    std::string line, word;
    auto width = [&](const std::string &s) { return f->CalcTextSizeA(size, FLT_MAX, 0, s.c_str()).x; };
    auto avail = [&]() { return (int)out.size() < indent_lines ? w - indent : w - (out.empty() ? 0 : cont); };
    auto flush_word = [&]() {
        if (word.empty()) return;
        std::string cand = line.empty() ? word : line + " " + word;
        if (!line.empty() && width(cand) > avail()) { out.push_back(line); line = word; }
        else line = cand;
        word.clear();
    };
    for (size_t i = 0; i <= t.size(); i++) {
        char ch = i < t.size() ? t[i] : 0;
        if (ch == ' ' || ch == 0 || ch == '\n') {
            flush_word();
            if (ch == '\n') { out.push_back(line); line.clear(); }
        } else word += ch;
    }
    if (!line.empty()) out.push_back(line);
}

static std::string gtext(const char *s) { return game_text(s, strlen(s)); }

/* the mark of the key that chooses an option: a red numeral, as a manuscript's rubric. c: the
   mark's middle; size: the card's letters; f: the card's face */
static void card_key_mark(ImDrawList *dl, ImFont *f, ImVec2 cpos, float size, const char *n, bool dark)
{
    char rn[6]; snprintf(rn, sizeof rn, "%s.", n);
    float rs = size * 1.0f; ImVec2 rt = f->CalcTextSizeA(rs, FLT_MAX, 0, rn);
    ImU32 red = dark ? IM_COL32(232, 118, 92, 255) : IM_COL32(158, 34, 20, 255);
    dl->AddText(f, rs, ImVec2(cpos.x - rt.x * 0.5f, cpos.y - rt.y * 0.52f), red, rn);
}

extern "C" void cardview_draw(void)
{
    n_chips = 0;                                                   /* (drawn anew below, or none) */
    CardView &c = cardview;
    if (!c.phase || (cardview_dialog && !game_dialog_kind)) return;
    if (ns_screen != NS_CARD || ns_hidden()) return;           /* a member's sheet ..., the menu bar or what it opened (mods/nscreen.h) */
    ImFont *f = ui_font_game[ui_game_font] ? ui_font_game[ui_game_font] : ui_font_text;
    if (!f) return;
    ImVec2 top = g2w(0, 0), bot = g2w(0, 200);
    float k = (bot.y - top.y) / 200.0f;                         /* window pixels per game row */
    if (k * 6 * 1.18f > 44 && ui_font_game_hi[ui_game_font]) f = ui_font_game_hi[ui_game_font];
    ImFont *fs = ui_font_overlay ? ui_font_overlay : f;
    ImDrawList *dl = ImGui::GetBackgroundDrawList();
    if (orn_on()) orn_braid_frame_knotted(dl, g2w(60, 0), g2w(320, 200), 7.0f * k);   /* the card's border: the plaited copper band, over the game's */
    const bool dark = c.dark;                                    /* light letters on a dark picture */
    const bool menu = c.phase == 2;
    const ImU32 ink = dark ? IM_COL32(246, 234, 208, 255) : IM_COL32(44, 26, 10, 255);
    const ImU32 ink_dim = dark ? IM_COL32(246, 234, 208, 115) : IM_COL32(44, 26, 10, 110);
    const ImU32 ink_soft = dark ? IM_COL32(220, 200, 165, 220) : IM_COL32(92, 60, 30, 230);
    float fk = ui_face_k(f);

    /* the card window: the dimmed picture under an option list */
    ImVec2 cw0 = g2w(67, 6), cw1 = g2w(312, 194);
    if (menu || (c.how && c.how != 2)) dl->AddRectFilled(cw0, cw1, IM_COL32(8, 5, 2, menu ? 120 : 60));

    /* the chances (mods/assist.c asks the game's option handlers) */
    const CardChances &cc = card_chances;
    bool chances = assist_chances && cc.active && cc.n == c.nlines && plat_ticks_ms() - cc.polled < 400;
    int diff = g_rb(G_DIFFICULTY) & 0x7f;
    int cmode = assist_chances == 2 ? 0 : diff;
    int members = (s16)g_rw(G_PARTY_SIZE);
    auto chance_shown = [&](int line) { return menu && chances && cmode < 2 && line >= 0 && line < 20 && cc.line[line].show; };

    /* lay the text out at a size that fits (the block's box, down to the frame) */
    float X0 = g2w((float)c.x, 0).x, X1 = g2w((float)(c.x + c.w), 0).x;
    float bottom = g2w(0, 191).y;
    float Y0 = g2w(0, (float)c.y).y;
    {   /* a box anchored at the picture's top left or top right: as far from the corner across as
           down (the picture is (67, 6) to (312, 194); the box reaches 3.5 rows past its text) */
        ImVec2 p0 = g2w(67, 6), p1 = g2w(312, 194);
        float m = 3.0f * k, padb = 3.5f * k, w = X1 - X0;
        if (c.anchor == 0) { X0 = p0.x + m + padb; X1 = X0 + w; Y0 = p0.y + m + padb; }
        else if (c.anchor == 1) { X1 = p1.x - m - padb; X0 = X1 - w; Y0 = p0.y + m + padb; }
    }
    /* the options that bring a teacher to the inn (mods/teachers.c): a lectern at the row's end */
    std::vector<TchSource> teach(c.npieces);
    std::vector<char> teaches(c.npieces, 0);
    if (ui_teachers_on()) {
        char cn[16]; g_str(G_DGROUP, GV_card_name, cn, sizeof cn);
        for (int j = 0; j < c.npieces; j++) if (c.piece[j].is_opt && c.piece[j].state) teaches[j] = (char)teach_card_option(cn, c.piece[j].opt, &teach[j]);
    }
    /* the town gate's "simply walk out" ($Selec00): when the guards will stop the party and attack (or
       may: the town on alert), the crossed blades of a fight, and why under the pointer (ui/quests.cpp) */
    int gate_piece = -1; bool gate_certain = false; std::string gate_why;
    {
        char cn[16]; g_str(G_DGROUP, GV_card_name, cn, sizeof cn);
        if (menu && !strcasecmp(cn, "$SELEC00") && (gate_why = town_gate_risk((s16)g_rw(G_CUR_LOC), &gate_certain)).size())
            for (int j = 0; j < c.npieces; j++)
                if (c.piece[j].is_opt && c.piece[j].state == 1 && c.piece[j].type == 0 && strstr(c.piece[j].text, "walk out")) gate_piece = j;
    }
    auto fights = [&](int j) { const CvPiece &p = c.piece[j]; return p.is_opt && (p.type == -1 || j == gate_piece); };
    /* a better Philosopher's Stone ($Alche00 option 0, $Unive00 option 2): what it is (each town's is
       always the same, town_stone_offer), the option's words dim when it is no better than the party's */
    int stone_piece = -1, stone_offer = 0, stone_uni = 0;
    if (menu && assist_card_help) {
        char cn[16]; g_str(G_DGROUP, GV_card_name, cn, sizeof cn);
        stone_uni = !strcasecmp(cn, "$UNIVE00") ? 1 : !strcasecmp(cn, "$ALCHE00") ? 0 : -1;
        if (stone_uni >= 0)
            for (int j = 0; j < c.npieces; j++)
                if (c.piece[j].is_opt && c.piece[j].state && c.piece[j].opt == (stone_uni ? 2 : 0)) { stone_piece = j; stone_offer = town_stone_offer((s16)g_rw(G_CUR_LOC), stone_uni); }
    }
    const int stone_have = (s16)g_rw(GV_philosophers_stone);
    auto no_gain = [&](int j) { return j == stone_piece && stone_offer > 0 && stone_offer <= stone_have; };
    /* an option that leads to one of the party's tasks here (a reward waiting, a thing to get from an
       office): the task's picture at the row's end, the task under the pointer (ui/citynav.cpp) */
    std::vector<int> task_of(c.npieces, -1);
    std::vector<CardTask> tasks;
    if (menu) {
        tasks = citynav_option_tasks();
        for (int j = 0; j < c.npieces; j++)
            if (c.piece[j].is_opt && c.piece[j].state == 1)
                for (size_t t = 0; t < tasks.size(); t++) if (tasks[t].opt == c.piece[j].opt) { task_of[j] = (int)t; break; }
    }
    float scale = 1.0f;
    std::vector<Laid> laid(c.npieces);
    float size = 0, L = 0, gap = 0, pad = 0, cap = 0, total = 0;
    std::vector<std::string> ws;
    std::vector<std::string> utf(c.npieces);
    for (int j = 0; j < c.npieces; j++) utf[j] = money_words(gamedata_spell_text(gtext(c.piece[j].text)));   /* (names the game cut short, in full; money with its coins) */
    /* a narrow column beside a picture: the first letter in the line (an initial two lines high
       would leave a word or two a line beside it) */
    const int dropcap = c.dropcap && c.w < 110 ? 0 : c.dropcap;
    if (c.dropcap && !dropcap && c.npieces > 0) { char t[2] = { (char)c.dropcap, 0 }; utf[0] = t + utf[0]; }
    for (int tries = 0; tries < 10; tries++) {
        size = 7.1f * k * fk * scale;
        L = 9.0f * k * scale;
        gap = 3.0f * k * scale;
        pad = 1.2f * size;                                       /* an option's number */
        cap = dropcap ? 2 * L - 1.0f * k : 0;
        float y = Y0;
        for (int j = 0; j < c.npieces; j++) {
            const CvPiece &p = c.piece[j];
            Laid &la = laid[j];
            la.lines.clear();
            float x = X0, w = X1 - X0;
            float ind = 0; int indl = 0;
            if (j == 0 && cap > 0) { ind = cap + 0.35f * size; indl = 2; }
            if (p.is_opt) { x += pad; w -= pad; }
            std::string body = utf[j];
            float hang = 0;                                      /* the lines of an option under its words, after the "..." */
            if (p.is_opt && p.lead[0]) { std::string ld = gtext(p.lead); hang = f->CalcTextSizeA(size, FLT_MAX, 0, ld.c_str()).x; body = ld + body; }
            float reserve = (fights(j) ? 1.6f * size : 0) + (teaches[j] ? 1.6f * size : 0) + (task_of[j] >= 0 ? 1.6f * size : 0);   /* room for the crossed blades, the lectern, a task's picture */
            wrap(f, size, body, w - reserve, ind, indl, ws, hang);
            if (p.is_opt && j > 0 && !c.piece[j - 1].is_opt) y += gap * 0.6f;
            la.y0 = y - 0.12f * L;
            for (size_t i = 0; i < ws.size(); i++) {
                float lx = x + ((int)i < indl ? ind : i > 0 ? hang : 0);
                la.lines.push_back({ ws[i], lx, y });
                la.text_end_x = lx + f->CalcTextSizeA(size, FLT_MAX, 0, ws[i].c_str()).x;
                la.last_y = y;
                y += L;
            }
            if (ws.empty()) y += L;
            if (j == 0 && cap > 0 && (int)ws.size() < 2) y = fmaxf(y, Y0 + cap + 0.2f * L);
            la.y1 = y - 0.12f * L + 0.5f * k;
            /* the members' saints or items under the option */
            if (p.is_opt && c.popup_piece == j) y += (float)c.nentries * L * 0.92f + (c.nentries ? 0.4f * L : L);
            if (!p.is_opt) y += gap;
        }
        total = y - Y0;
        if (y <= bottom || scale < 0.74f) break;
        scale *= 0.94f;
    }
    float shift = 0;                                            /* still too long: start higher */
    if (Y0 + total > bottom) { shift = fminf(Y0 + total - bottom, Y0 - g2w(0, 9).y); if (shift < 0) shift = 0; }
    float yend = Y0 + total - shift;

    /* the box behind the text */
    float bx0 = X0 - 3.5f * k, bx1 = X1 + 3.5f * k, by0 = Y0 - shift - 3.5f * k, by1 = yend + 1.0f * k;
    {
        ImU32 bg = dark ? IM_COL32(14, 9, 4, menu ? 175 : 150) : IM_COL32(240, 226, 192, menu ? 222 : 196);
        ImU32 edge = dark ? IM_COL32(170, 125, 65, 120) : IM_COL32(120, 80, 36, 110);
        dl->AddRectFilled(ImVec2(bx0, by0), ImVec2(bx1, by1), bg, 5.0f * k * 0.5f);
        dl->AddRect(ImVec2(bx0, by0), ImVec2(bx1, by1), edge, 5.0f * k * 0.5f, 0, fmaxf(1.0f, 0.35f * k));
        if (orn_on()) orn_gilt_frame(dl, ImVec2(bx0, by0), ImVec2(bx1, by1), 3.2f * k);
    }
    {   /* the box in game pixels, for the engine's pointer tests */
        ImVec2 a = g2w(0, 0), b = g2w(320, 0);
        float kx = (b.x - a.x) / 320.0f;
        c.box_x0 = (bx0 - a.x) / kx; c.box_x1 = (bx1 - a.x) / kx;
    }
    auto wy2g = [&](float wy) { return (wy - top.y) / k; };

    /* the illuminated initial */
    if (dropcap) {
        ImVec2 p0(X0, Y0 - shift - 0.05f * L), p1(X0 + cap, Y0 - shift - 0.05f * L + cap);
        dl->AddRectFilled(p0, p1, IM_COL32(112, 24, 16, 255), 1.5f * k);
        float s = cap * 0.16f;                                  /* a diaper of small diamonds */
        for (float yy = p0.y + s; yy < p1.y - s * 0.5f; yy += 2 * s)
            for (float xx = p0.x + s + (fmodf((yy - p0.y) / (2 * s), 2.0f) > 0.5f ? s : 0); xx < p1.x - s * 0.5f; xx += 2 * s) {
                ImVec2 q[4] = { ImVec2(xx, yy - s * 0.45f), ImVec2(xx + s * 0.45f, yy), ImVec2(xx, yy + s * 0.45f), ImVec2(xx - s * 0.45f, yy) };
                dl->AddConvexPolyFilled(q, 4, IM_COL32(150, 44, 30, 255));
            }
        dl->AddRect(p0, p1, IM_COL32(214, 172, 92, 255), 1.5f * k, 0, fmaxf(1.0f, 0.55f * k));
        char t[2] = { (char)c.dropcap, 0 };
        float cs = cap * 1.25f;                                 /* the letter's ink centred in the square */
        const ImFontGlyph *g = f->FindGlyph((ImWchar)c.dropcap);
        ImVec2 at(p0.x, p0.y);
        if (g) {
            float sc = cs / f->FontSize, gw = (g->X1 - g->X0) * sc, gh = (g->Y1 - g->Y0) * sc;
            if (gh > cap * 0.74f) { cs *= cap * 0.74f / gh; sc = cs / f->FontSize; gw = (g->X1 - g->X0) * sc; gh = (g->Y1 - g->Y0) * sc; }
            at = ImVec2(p0.x + (cap - gw) * 0.5f - g->X0 * sc, p0.y + (cap - gh) * 0.5f - g->Y0 * sc);
        }
        dl->AddText(f, cs, ImVec2(at.x + 0.5f * k, at.y + 0.5f * k), IM_COL32(40, 6, 4, 200), t);
        dl->AddText(f, cs, at, IM_COL32(250, 236, 200, 255), t);
    }

    int tip_item = -1, tip_saint = -1, tip_formula = -1; ImVec2 tip_at;
    int tip_head = -1; ImVec2 tip_head0, tip_head1;                              /* (the pointer on a member's head in an option's list: the name) */
    int chance_tip = -1; ImVec2 chance_tip_a0, chance_tip_a1;                   /* (the pointer on an option's chance: who tries it, and what it rests on) */
    /* each member's chip, their head and their chance: a click on one chooses the option with that member
       speaking. x: where they begin; t: how far slid out (0..1, the hover's); returns where they end */
    auto chips = [&](float x, float cy, float ms, int line, int opt, bool formula, float t) -> float {
        int leader = (s8)g_rb(G_LEADER), best = cc.line[line].lead >= 0 ? cc.line[line].lead : leader;
        float hsz = ms * 1.25f, bh = hsz * 1.06f, x0 = x;
        for (int m = 0; m < 5 && m < members; m++) {
            int pm = cc.line[line].per[m];
            if (pm < 0) continue;
            char t2[16];
            if (cmode == 0) snprintf(t2, sizeof t2, "%d%%", pm);
            else { int q = pm / 20; if (q > 6) q = 6; u16 pp = g_rw((u16)(GV_quality_words + 2 * q)); if (pm <= 0) snprintf(t2, sizeof t2, "hopeless"); else g_str(G_DGROUP, pp, t2, sizeof t2); }
            ImVec2 tt = fs->CalcTextSizeA(ms, FLT_MAX, 0, t2);
            float w = hsz * 1.05f + tt.x + 0.45f * ms, xs = x0 + (x - x0) * t;   /* (sliding out from the box's edge) */
            ImVec2 q0(xs, cy - bh * 0.5f), q1(xs + w, cy + bh * 0.5f);
            int a = (int)(255 * t);
            bool over = t > 0.9f && ImGui::IsMouseHoveringRect(q0, q1, false);
            ImU32 fg = cmode == 1 ? IM_COL32(245, 228, 185, a) : pm >= 70 ? IM_COL32(150, 230, 120, a) : pm >= 40 ? IM_COL32(250, 210, 95, a) : IM_COL32(250, 125, 100, a);
            dl->AddRectFilled(q0, q1, over ? IM_COL32(60, 40, 20, 240 * a / 255) : IM_COL32(28, 18, 10, 228 * a / 255), bh * 0.3f);
            dl->AddRect(q0, q1, over ? IM_COL32(240, 200, 120, a) : m == best ? IM_COL32(210, 160, 80, 230 * a / 255) : IM_COL32(150, 110, 60, 190 * a / 255), bh * 0.3f, 0, fmaxf(1.0f, (m == best ? 0.45f : 0.3f) * k));
            if (t > 0.5f && !member_head(dl, m, ImVec2(q0.x + 0.25f * ms, cy - hsz * 0.5f), hsz)) {
                char nm[24]; g_str(G_DGROUP, (u16)(G_MEMBER(m) + G_M_SHORTNAME), nm, sizeof nm); nm[1] = 0;
                dl->AddText(fs, ms, ImVec2(q0.x + 0.5f * ms, cy - ms * 0.5f), IM_COL32(215, 195, 160, a), game_text(nm, 1).c_str());
            }
            dl->AddText(fs, ms, ImVec2(q0.x + 0.25f * ms + hsz * 0.95f, cy - tt.y * 0.52f), fg, t2);
            if (over) {
                char nm[24]; g_str(G_DGROUP, (u16)(G_MEMBER(m) + G_M_SHORTNAME), nm, sizeof nm);
                std::string who = game_text(nm, strlen(nm));
                ui_tip_beside(q0, q1);
                ImGui::BeginTooltip(); ImGui::PushTextWrapPos(ImGui::GetFontSize() * 22);
                ImGui::TextUnformatted((who + " speaks for the party" + (m == leader ? " (the leader)" : "") + (formula ? ". A formula traded is " + who + "'s to know." : ".")).c_str());
                ImGui::TextDisabled("Click to choose this with %s speaking.", who.c_str());
                ImGui::PopTextWrapPos(); ImGui::EndTooltip();
            }
            if (t > 0.9f && n_chips < 25) { chip_r[n_chips][0] = q0.x; chip_r[n_chips][1] = q0.y; chip_r[n_chips][2] = q1.x; chip_r[n_chips][3] = q1.y; chip_opt[n_chips] = opt; chip_m[n_chips] = m; n_chips++; }
            x += w + 0.25f * ms;
        }
        return x0 + (x - x0) * t;
    };
    auto chips_width = [&](float ms, int line) {
        float w = 0;
        for (int m = 0; m < 5 && m < members; m++) { int pm = cc.line[line].per[m]; if (pm < 0) continue; char t2[16]; snprintf(t2, sizeof t2, cmode == 0 ? "%d%%" : "hopeless", pm);
            w += ms * 1.25f * 1.05f + fs->CalcTextSizeA(ms, FLT_MAX, 0, t2).x + 0.45f * ms + 0.25f * ms; }
        return w;
    };
    struct { bool on; float x, cy, ms, e; int line, opt; ImVec2 r0, r1; } slide_out = {};   /* (drawn over the options after them) */
    for (int j = 0; j < c.npieces; j++) {
        const CvPiece &p = c.piece[j];
        Laid &la = laid[j];
        float y0 = la.y0 - shift, y1 = la.y1 - shift;
        bool hov = menu && p.is_opt && p.state == 1 && (c.hover == j || (c.key_line >= 0 && p.line == c.key_line));
        if (hov) {
            ImU32 hl = dark ? IM_COL32(255, 214, 140, 46) : IM_COL32(150, 92, 30, 52);
            dl->AddRectFilled(ImVec2(X0 - 1.5f * k, y0), ImVec2(X1 + 1.5f * k, y1), hl, 1.5f * k);
            dl->AddRectFilled(ImVec2(X0 - 1.5f * k, y0), ImVec2(X0 - 0.6f * k, y1), dark ? IM_COL32(240, 190, 100, 220) : IM_COL32(150, 60, 20, 220));
        }
        ImU32 col = !p.is_opt ? ink : p.state == 1 && !no_gain(j) ? ink : ink_dim;
        if (p.is_opt && menu && p.state == 1 && p.line >= 0 && p.line < 10) {          /* the key that chooses it (0: the tenth) */
            char n[4]; snprintf(n, sizeof n, "%d", (p.line + 1) % 10);
            ImVec2 cpos(X0 + pad * 0.42f, la.lines.empty() ? y0 + 0.5f * L : la.lines[0].y - shift + size * 0.52f);
            card_key_mark(dl, f, cpos, size, n, dark);
        } else if (menu && (greyed_hint(p) || (assist_card_help && p.is_opt && p.state == 2 && (p.type == 1 || p.type == 2)))) {
            /* greyed, and why is known (the card's own reason, or the saints or items nobody has): a "?" where the number would be */
            ImVec2 cpos(X0 + pad * 0.42f, la.lines.empty() ? y0 + 0.5f * L : la.lines[0].y - shift + size * 0.52f);
            float rs = size * 1.0f; ImVec2 rt = f->CalcTextSizeA(rs, FLT_MAX, 0, "?");   /* (as the numerals, dim) */
            dl->AddText(f, rs, ImVec2(cpos.x - rt.x * 0.5f, cpos.y - rt.y * 0.52f), ink_dim, "?");
        }
        if (fights(j) && p.state == 1) {                         /* a fight: two crossed blades (faint: a fight that may come) */
            ImVec2 cpos(X1 - 0.9f * size, (la.lines.empty() ? y0 : la.lines[0].y - shift) + size * 0.5f);
            float r = size * 0.42f, th = fmaxf(1.2f, 0.34f * k);
            int ba = j == gate_piece && !gate_certain ? 120 : 230;
            ImU32 bc = dark ? IM_COL32(230, 120, 100, ba) : IM_COL32(140, 30, 20, ba);
            for (int s = -1; s <= 1; s += 2) {                   /* (each a sword: blade, crossguard, pommel - two bare lines read as an X, "not possible") */
                ImVec2 tip(cpos.x - s * r, cpos.y - r), hilt(cpos.x + s * r, cpos.y + r);
                ImVec2 g(tip.x + 0.68f * (hilt.x - tip.x), tip.y + 0.68f * (hilt.y - tip.y));
                float px = 0.7071f, py = -0.7071f * s, cg = r * 0.42f;       /* across the blade */
                dl->AddLine(tip, g, bc, th);
                dl->AddLine(ImVec2(g.x - px * cg, g.y - py * cg), ImVec2(g.x + px * cg, g.y + py * cg), bc, th * 1.15f);
                dl->AddLine(g, hilt, bc, th * 1.5f);
                dl->AddCircleFilled(hilt, th * 1.1f, bc, 8);
            }
        }
        if (teaches[j]) {                                          /* brings a teacher: a lectern (left of the blades, if both) */
            ImVec2 cpos(X1 - 0.9f * size - (fights(j) && p.state == 1 ? 1.6f * size : 0), (la.lines.empty() ? y0 : la.lines[0].y - shift) + size * 0.5f);
            int a = p.state == 1 ? 235 : 110;
            ImU32 ti = dark ? IM_COL32(236, 210, 160, a) : IM_COL32(80, 50, 24, a), tp = dark ? IM_COL32(238, 230, 208, a) : IM_COL32(252, 246, 228, a);
            ui_draw_teach_mark(dl, cpos, size * 1.05f, ti, tp);
        }
        if (task_of[j] >= 0) {                                     /* leads to a task: its picture (left of the blades and the lectern) */
            float off = (fights(j) && p.state == 1 ? 1.6f * size : 0) + (teaches[j] ? 1.6f * size : 0);
            ImVec2 cpos(X1 - 0.9f * size - off, (la.lines.empty() ? y0 : la.lines[0].y - shift) + size * 0.5f);
            ui_draw_quest_icon(dl, cpos, size * 1.25f, tasks[task_of[j]].cat);
        }
        for (const Line &l : la.lines) dl->AddText(f, size, ImVec2(l.x, l.y - shift), col, l.t.c_str());
        if (p.is_opt && (p.type == 1 || p.type == 2) && p.state == 1) {               /* opens a list: a small arrow */
            float ax = la.text_end_x + 0.45f * size, ay = la.last_y - shift + size * 0.55f, r = size * 0.22f;
            bool open = c.popup_piece == j;
            ImU32 ac = ink_soft;
            if (open) dl->AddTriangleFilled(ImVec2(ax - r, ay - r * 0.5f), ImVec2(ax + r, ay - r * 0.5f), ImVec2(ax, ay + r * 0.7f), ac);
            else dl->AddTriangleFilled(ImVec2(ax - r * 0.5f, ay - r), ImVec2(ax - r * 0.5f, ay + r), ImVec2(ax + r * 0.7f, ay), ac);
        }
        if (p.is_opt) { c.row_y0[j] = wy2g(y0); c.row_y1[j] = wy2g(y1); }
        else c.row_y0[j] = c.row_y1[j] = -1;
        /* the chance of success (the alchemist's formulae too, where what comes of it is the speaker's: the
           leader's chance, and the arrow for each member's, as everywhere) */
        if (p.is_opt && chance_shown(p.line)) {
            int pct = cc.line[p.line].pct, who = cc.line[p.line].who;
            char main[40], sub[24] = "";
            if (cmode == 0) snprintf(main, sizeof main, "%d%%", pct);
            else {
                int q = pct / 20; if (q > 6) q = 6;
                u16 pp = g_rw((u16)(GV_quality_words + 2 * q));
                if (pct <= 0) snprintf(main, sizeof main, "hopeless");
                else g_str(G_DGROUP, pp, main, sizeof main);
            }
            /* who tries it (one head; "all" when each member tries - never a row of heads: one chance, one
               look, on every option), and what it rests on (the skills and attributes as on the character
               sheet, the town's opinion, fame) */
            u32 reads = cc.line[p.line].reads;
            int heads[5], nh = 0, icons[30], ni = 0;
            if (who >= 0 && who < 5) heads[nh++] = who;
            for (int q = 0; q < 19; q++) if (reads & CC_SKILL(q)) icons[ni++] = q;
            for (int q = 0; q < 7; q++) if (reads & CC_ATTR(q)) icons[ni++] = 100 + q;
            if (reads & CC_REP) icons[ni++] = 200;
            if (reads & CC_FAME) icons[ni++] = 201;
            if (who >= 0 && who < 5 && members > 1) { char nm[24]; g_str(G_DGROUP, (u16)(G_MEMBER(who) + G_M_SHORTNAME), nm, sizeof nm); snprintf(sub, sizeof sub, "%s", game_text(nm, strlen(nm)).c_str()); }
            else if (who == -2 && members > 1) snprintf(sub, sizeof sub, "all");
            float ms = size * 0.82f, ss = size * 0.66f;
            float hsz = ms * 1.4f, isz = ms * 0.95f, gap = 0.28f * ms;          /* (a head large enough to tell who; a shield is 1.44 sizes tall) */
            if (nh) sub[0] = 0;                                     /* (the heads say who: no name) */
            ImVec2 mt = fs->CalcTextSizeA(ms, FLT_MAX, 0, main), st = sub[0] ? fs->CalcTextSizeA(ss, FLT_MAX, 0, sub) : ImVec2(0, 0);
            const float shw = isz * 1.25f, step = shw + 0.24f * ms;           /* (a shield is 1.25 sizes wide: apart by a little more) */
            /* a chance that is the speaker's: a chevron at the box's end; pointing at it slides out every member's
               chance under the box, to pick who speaks (the one who speaks unless picked: ringed) */
            bool slide = cc.line[p.line].dep && p.state == 1 && menu && !ui_is_open();
            const float chw = slide ? 1.15f * ms : 0;
            float bw = mt.x + (sub[0] ? st.x + 0.3f * ms : 0) + 0.6f * ms + (nh ? gap + nh * hsz * 0.92f : 0) + (ni ? gap + ni * step - 0.24f * ms : 0) + chw, bh = fmaxf(ms * 1.02f, fmaxf(nh ? hsz * 1.06f : 0, ni ? isz * 1.56f : 0));
            /* beside the box, level with the option's first line: a tab on the card's edge */
            float cy = (la.lines.empty() ? y0 : la.lines[0].y - shift) + size * 0.52f;
            float bxl = X1 + 4.5f * k, bxr = bxl + bw;
            ImU32 fg = cmode == 1 ? IM_COL32(245, 228, 185, 255)
                     : pct >= 70 ? IM_COL32(150, 230, 120, 255) : pct >= 40 ? IM_COL32(250, 210, 95, 255) : IM_COL32(250, 125, 100, 255);
            ImVec2 q0(bxl, cy - bh * 0.5f), q1(bxr, cy + bh * 0.5f);
            dl->AddRectFilled(q0, q1, IM_COL32(28, 18, 10, 228), bh * 0.3f);
            dl->AddRect(q0, q1, hov ? IM_COL32(230, 185, 100, 240) : IM_COL32(170, 125, 65, 200), bh * 0.3f, 0, fmaxf(1.0f, 0.3f * k));
            dl->AddText(fs, ms, ImVec2(q0.x + 0.3f * ms, cy - mt.y * 0.52f), fg, main);
            if (sub[0]) dl->AddText(fs, ss, ImVec2(q0.x + 0.3f * ms + mt.x + 0.3f * ms, cy - st.y * 0.5f), IM_COL32(215, 195, 160, 240), sub);
            float hx = q0.x + 0.3f * ms + mt.x + gap;
            for (int h = 0; h < nh; h++, hx += hsz * 0.92f)
                if (!member_head(dl, heads[h], ImVec2(hx, cy - hsz * 0.5f), hsz)) {   /* (no figure: the initial) */
                    char nm[24]; g_str(G_DGROUP, (u16)(G_MEMBER(heads[h]) + G_M_SHORTNAME), nm, sizeof nm); nm[1] = 0;
                    dl->AddText(fs, ss, ImVec2(hx + hsz * 0.25f, cy - ss * 0.5f), IM_COL32(215, 195, 160, 240), game_text(nm, 1).c_str());
                }
            if (nh) hx += gap * 0.5f;
            for (int q = 0; q < ni; q++, hx += step) {
                ImVec2 ic(hx + shw * 0.5f, cy - isz * 0.03f);
                if (icons[q] < 100) ui_skill_icon(dl, ic, isz, icons[q]);
                else if (icons[q] < 200) ui_attr_icon(dl, ic, isz, icons[q] - 100);
                else ui_standing_icon(dl, ic, isz, icons[q] - 200);
            }
            bool on_box = menu && !ui_is_open() && ImGui::IsMouseHoveringRect(q0, q1, false);
            float row_h = 0;
            if (slide) {                                            /* (in a row under the box, on a dark band, over the options below) */
                ImVec2 c0(bxr - chw, q0.y), c1 = q1;                /* the chevron's part of the box */
                bool on_chev = ImGui::IsMouseHoveringRect(c0, c1, false);
                float sms = ms * 0.9f, sw = chips_width(sms, p.line), room = ImGui::GetIO().DisplaySize.x - bxl - 6 * k;
                if (sw > room && sw > 0) { sms *= fmaxf(0.6f, room / sw); sw = chips_width(sms, p.line); }
                row_h = sms * 1.25f * 1.06f + 0.5f * ms;
                ImVec2 s0(bxl, q0.y), s1(fmaxf(bxr, bxl + sw + 0.3f * ms), q1.y + row_h);
                bool keep = slide_line == j && ImGui::IsMouseHoveringRect(s0, s1, false);
                if (on_chev || keep) { if (slide_line != j) { slide_line = j; slide_t = 0; } }
                else if (slide_line == j) slide_line = -1;
                bool open = slide_line == j;
                dl->AddLine(ImVec2(c0.x, q0.y + bh * 0.2f), ImVec2(c0.x, q1.y - bh * 0.2f), IM_COL32(150, 110, 60, 160), fmaxf(1.0f, 0.2f * k));
                float ccx = c0.x + chw * 0.5f, r = 0.3f * ms;
                ImU32 cc2 = open || on_chev ? IM_COL32(250, 220, 140, 255) : IM_COL32(215, 175, 100, 230);
                ImVec2 v[3];                                        /* > pointing on; once open, down to the row */
                if (open) { v[0] = ImVec2(ccx - r, cy - r * 0.45f); v[1] = ImVec2(ccx, cy + r * 0.55f); v[2] = ImVec2(ccx + r, cy - r * 0.45f); }
                else { v[0] = ImVec2(ccx - r * 0.45f, cy - r); v[1] = ImVec2(ccx + r * 0.55f, cy); v[2] = ImVec2(ccx - r * 0.45f, cy + r); }
                dl->AddPolyline(v, 3, cc2, 0, fmaxf(1.5f, 0.16f * ms));
                if (open) {
                    slide_t = fminf(1.0f, slide_t + ImGui::GetIO().DeltaTime / 0.14f); float e = 1 - (1 - slide_t) * (1 - slide_t);
                    slide_out.on = true; slide_out.x = bxl + 0.15f * ms; slide_out.cy = q1.y + row_h * 0.5f + 0.1f * ms; slide_out.ms = sms; slide_out.e = e;
                    slide_out.line = p.line; slide_out.opt = p.opt; slide_out.r0 = ImVec2(bxl, q1.y + 0.1f * ms); slide_out.r1 = ImVec2(bxl + sw + 0.3f * ms, q1.y + row_h + 0.15f * ms);
                } else row_h = 0;
            }
            if (on_box) { chance_tip = j; chance_tip_a0 = q0; chance_tip_a1 = ImVec2(q1.x, q1.y + row_h); }   /* (the box, and the members' row under it when open) */
        }
        /* the list under it */
        if (p.is_opt && c.popup_piece == j) {
            float ey = y1 + 0.15f * L;
            float ex = X0 + pad + 0.8f * size;
            if (!c.nentries) {
                const char *none = p.type == 2 ? "Nobody in the party knows a saint who can help here." : "Nobody in the party carries anything that helps here.";
                dl->AddText(fs, size * 0.86f, ImVec2(ex, ey), ink_dim, none);
            }
            /* a list of saints to learn (a library, a teacher), not to call on: some member is offered one it
               does not know; then those it knows already are dimmed */
            bool saint_learning = false;
            if (c.popup_kind == 2) for (int e = 0; e < c.nentries; e++) { const CvEntry &q = c.entry[e]; if (q.id >= 0 && q.member >= 0 && q.member < 5 && !gm_knows_saint(q.member, q.id)) saint_learning = true; }
            float namew = 0, hs = L * 0.92f * 0.88f;                 /* (who: the member's head, before their first entry) */
            for (int e = 0; e < c.nentries; e++) if (c.entry[e].who[0]) namew = hs / 1.2f;
            for (int e = 0; e < c.nentries; e++) {
                const CvEntry &en = c.entry[e];
                float h = L * 0.92f;
                bool eh = c.hover_entry == e && en.on;
                if (eh) dl->AddRectFilled(ImVec2(ex - 0.4f * size, ey - 0.1f * h), ImVec2(X1 + 1.0f * k, ey + 0.95f * h), dark ? IM_COL32(255, 214, 140, 50) : IM_COL32(150, 92, 30, 55), 1.2f * k);
                if (en.who[0]) {
                    ImVec2 h0(ex, ey + 0.02f * h), h1(ex + hs / 1.2f, ey + 0.02f * h + hs);
                    ui_member_head(dl, en.member, h0, hs);
                    if (ImGui::IsMouseHoveringRect(h0, h1, false)) { tip_head = en.member; tip_head0 = h0; tip_head1 = h1; }
                }
                ImU32 ec = !en.on || en.code == 0x83 ? ink_dim : en.code == 0x87 ? (dark ? IM_COL32(250, 210, 110, 255) : IM_COL32(150, 90, 0, 255)) : ink;
                std::string et = gtext(en.text);
                if (c.popup_kind == 2 && en.id >= 0 && en.id < (int)gamedata.saints.size() && !gamedata.saints[en.id].name.empty()) et = gamedata.saints[en.id].name;   /* (the saint's whole name, as the sheet writes it: not the card's "S.Christina") */
                int ef = c.popup_kind == 2 ? -1 : cv_formula_of_slot(en.slot);
                float isz = c.popup_kind == 2 ? 0 : size * 1.05f;     /* (an item's picture before it: a potion's with its grade) */
                if (ef >= 0) {                                       /* (a formula on sale: its own name, and whether this member knows it) */
                    bool knows = gm_knows_formula(en.member, ef) != 0;
                    et = gamedata.formula_names[ef] + "  (" + cv_formula_price(en.slot) + ")";
                    if (knows) ec = ink_dim;
                    ui_item_icon_tier(dl, ImVec2(ex + namew + 0.9f * size + isz * 0.5f, ey + 0.46f * h), isz, 95 + ef / 3, ef % 3 + 1);
                    float tx = ex + namew + 0.9f * size + isz * 1.15f;
                    dl->AddText(f, size * 0.95f, ImVec2(tx, ey), ec, et.c_str());
                    if (knows) dl->AddText(fs, size * 0.78f, ImVec2(tx + f->CalcTextSizeA(size * 0.95f, FLT_MAX, 0, et.c_str()).x + 0.6f * size, ey + 0.12f * h), ink_soft, "knows it already");
                } else {
                    float tx = ex + namew + 0.9f * size;
                    bool knows_saint = c.popup_kind == 2 && en.id >= 0 && en.member >= 0 && en.member < 5 && saint_learning && gm_knows_saint(en.member, en.id);
                    if (knows_saint) ec = ink_dim;                   /* (a saint to learn that the member knows already: dim, as a known formula) */
                    if (isz > 0 && en.id >= 0) { ui_item_icon_q(dl, ImVec2(tx + isz * 0.5f, ey + 0.46f * h), isz, en.id, best_quality(en.member, en.id)); tx += isz * 1.15f; }
                    dl->AddText(f, size * 0.95f, ImVec2(tx, ey), ec, et.c_str());
                    /* why a line is there that cannot be chosen, or can without the thing (the game's colour codes) */
                    const char *why = en.code == 0x84 ? "knows the formula but carries none" :
                                      en.code == 0x87 ? (c.popup_kind == 2 ? "at the easiest difficulty, the leader need not know the saint" : "at the easiest difficulty, the leader needs none") : nullptr;
                    if (knows_saint) why = "knows this saint already";
                    if (why) dl->AddText(fs, size * 0.78f, ImVec2(tx + f->CalcTextSizeA(size * 0.95f, FLT_MAX, 0, et.c_str()).x + 0.6f * size, ey + 0.12f * h), ink_soft, why);
                    /* a saint to call on: the Virtue it asks of the one who prays, at the row's end - Virtue's shield and
                       the number, greyed when this member's is below it (more Divine Favor does not help then), as on the
                       Saints page and the prayer screen */
                    if (c.popup_kind == 2 && !saint_learning && en.vmin > 0) {
                        char vb[16]; snprintf(vb, sizeof vb, "%d", en.vmin);
                        float vs = size * 0.82f, ish = size * 0.8f;
                        float vw = UI_SHIELD_W * ish + ish * 0.2f + fs->CalcTextSizeA(vs, FLT_MAX, 0, vb).x;
                        bool low = en.virtue < en.vmin;
                        ImVec2 m0(X1 - 0.5f * size - vw, ey), m1(X1 - 0.5f * size, ey + h);
                        int v0 = dl->VtxBuffer.Size;
                        ui_shield_value(dl, ImVec2(m0.x, ey + 0.46f * h), ish, UI_KEY_SKILL + SKILL_VIRTUE, vb, fs, vs, ink_soft);
                        if (low) ui_dim_since(dl, v0, 0.38f);
                        if (ImGui::IsMouseHoveringRect(m0, m1, false) && !ui_is_open()) {
                            ui_tip_beside(m0, m1);
                            ImGui::BeginTooltip(); ImGui::PushTextWrapPos(ImGui::GetFontSize() * 22);
                            ui_rich_textf("This saint asks a {Virtue} of %d of the one who prays.", en.vmin);
                            if (low) { ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImGuiCol_TextDisabled));
                                       ui_rich_textf("%s: too low to call on this saint, however much {Divine Favor} is spent.", rich_member(en.member, std::to_string(en.virtue)).c_str());
                                       ImGui::PopStyleColor(); }
                            ImGui::PopTextWrapPos(); ImGui::EndTooltip();
                        }
                    }
                }
                c.entry_y0[e] = wy2g(ey - 0.1f * h); c.entry_y1[e] = wy2g(ey + 0.95f * h);
                if (c.hover_entry == e) { tip_at = ImVec2(X1 + 2 * k, ey); if (c.popup_kind == 2) tip_saint = en.id; else if (ef >= 0) tip_formula = ef; else tip_item = en.id; }
                ey += h;
            }
        }
    }
    /* go on: a quiet sign at the end of a text that waits */
    if (!menu) {
        float t = (plat_ticks_ms() % 1600) / 1600.0f, a = 0.45f + 0.35f * sinf(t * 6.2831853f);
        float r = size * 0.28f, ax = X1 - r, ay = yend - shift * 0 - 0.62f * L;
        ImU32 ac = dark ? IM_COL32(246, 220, 160, (int)(255 * a)) : IM_COL32(110, 50, 15, (int)(255 * a));
        dl->AddTriangleFilled(ImVec2(ax - r, ay - r * 0.55f), ImVec2(ax + r, ay - r * 0.55f), ImVec2(ax, ay + r * 0.75f), ac);
        if (ui_pad_prompts()) ui_pad_glyph(dl, ImVec2(ax - r * 4.2f, ay + r * 0.1f), r * 3.0f, PB_A);   /* (a controller: A goes on) */
    }
    /* a tooltip beside the box, level with the option under the pointer (narrower to fit there, down
       to 14 letters); else under the option, above it, at its left - as every tooltip, clear of the
       option and the pointer and inside the window (ui/tipplace.cpp). It returns the width to wrap at */
    auto tip_place = [&](float em) -> float {
        float fs = ImGui::GetFontSize(), pad = ImGui::GetStyle().WindowPadding.x * 2 + 0.5f * fs;
        float W = ImGui::GetIO().DisplaySize.x, x = fmaxf(bx1 + 4.5f * k, ui_pointer_area().z + 0.3f * fs), w = em * fs;   /* (past the pointer too) */
        if (x + w + pad > W - 4 && W - 4 - x - pad >= 14 * fs) w = W - 4 - x - pad;
        w = fminf(w, W - 8 - pad);
        ImGui::SetNextWindowSizeConstraints(ImVec2(0, 0), ImVec2(w + pad, FLT_MAX));
        ui_tip_beside_in(ImVec2(X0, laid[c.hover].y0 - shift), ImVec2(bx1 + 4.5f * k - fs * 0.3f, laid[c.hover].y1 - shift), "rbal");
        return w;
    };
    const bool tipmenu = menu && !ui_is_open();                  /* (no card tooltips over the Darklands+ window, the world map, Go to ...) */
    if (ui_is_open()) tip_formula = tip_item = tip_saint = -1;
    if (slide_out.on) {                                         /* every member's chance, slid out under the pointer's option */
        dl->AddRectFilled(slide_out.r0, ImVec2(slide_out.r0.x + (slide_out.r1.x - slide_out.r0.x) * slide_out.e, slide_out.r1.y), IM_COL32(14, 9, 5, (int)(200 * slide_out.e)), 4 * k);
        chips(slide_out.x, slide_out.cy, slide_out.ms, slide_out.line, slide_out.opt, cc.line[slide_out.line].pick != 0, slide_out.e);
    }
    if (chance_tip >= 0 && !ui_is_open()) {
        const CvPiece &p = c.piece[chance_tip];
        int who = cc.line[p.line].who; u32 reads = cc.line[p.line].reads;
        std::string w;
        if (who >= 0 && who < 5) { char nm[24]; g_str(G_DGROUP, (u16)(G_MEMBER(who) + G_M_SHORTNAME), nm, sizeof nm); w = game_text(nm, strlen(nm)) + " tries it"; }
        else if (who == -2) w = "Each member tries it";
        /* what it rests on: each its shield and its name (the skills, the attributes, the town's opinion, fame) */
        struct Rest { int kind, k; std::string name; };
        std::vector<Rest> rest;
        for (int q = 0; q < 19; q++) if (reads & CC_SKILL(q)) rest.push_back({ 1, q, game_text(gm_skill_name(q), 40) });
        for (int q = 0; q < 7; q++) if (reads & CC_ATTR(q)) rest.push_back({ 0, q, game_text(gm_attr_name(q), 40) });
        if (reads & CC_REP) rest.push_back({ 2, 0, "the town's opinion of the party (its reputation here)" });
        if (reads & CC_FAME) rest.push_back({ 2, 1, "the party's fame" });
        if (!w.empty() || !rest.empty()) {
            ui_tip_beside(chance_tip_a0, chance_tip_a1);
            ImGui::BeginTooltip();
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 22);
            if (!w.empty()) ImGui::TextUnformatted((w + ".").c_str());
            if (!rest.empty()) {
                ImGui::TextUnformatted("It rests on:");
                float lh = ImGui::GetTextLineHeight(), is = lh * 1.2f;
                for (const Rest &x : rest) {
                    ImVec2 a = ImGui::GetCursorScreenPos();
                    ImGui::Dummy(ImVec2(is, lh)); ImGui::SameLine(0, lh * 0.3f);
                    ImVec2 c(a.x + is * 0.5f, a.y + lh * 0.5f);
                    if (x.kind == 1) ui_skill_icon(ImGui::GetWindowDrawList(), c, is * 0.9f, x.k);
                    else if (x.kind == 0) ui_attr_icon(ImGui::GetWindowDrawList(), c, is * 0.9f, x.k);
                    else ui_standing_icon(ImGui::GetWindowDrawList(), c, is * 0.9f, x.k);
                    ImGui::TextUnformatted(x.name.c_str());
                }
            }
            if (cc.line[p.line].pick) ImGui::TextDisabled("The leader speaks for the party, and what is traded is the speaker's to know, unless you pick another (the arrow at the end shows each member's chance).");
            else if (cc.line[p.line].dep) ImGui::TextDisabled(assist_best_leader ? "Whoever speaks for the party tries it: the best at it, unless you pick another (the arrow at the end shows each member's chance)."
                                                                           : "Whoever speaks for the party tries it: the leader, unless you pick another (the arrow at the end shows each member's chance).");
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }
    }
    else if (tip_head >= 0 && !ui_is_open()) { ui_tip_beside(tip_head0, tip_head1); ImGui::SetTooltip("%s", ui_member_name(tip_head).c_str()); }
    else if (tip_formula >= 0) ui_card_formula_tip(tip_at, ImVec2(0, 0), tip_formula);
    else if (tip_item >= 0 || tip_saint >= 0) ui_card_tip(tip_at, ImVec2(0, 0), tip_item, tip_saint);
    else if (tipmenu && c.hover >= 0 && c.hover < c.npieces && teaches[c.hover]) {
        /* a teacher: what, how well, for how much, and what it takes (and why it is greyed, if known) */
        float ww = tip_place(23);
        ImGui::BeginTooltip();
        ImGui::TextDisabled("Brings a teacher to the inn");
        ImGui::PushTextWrapPos(ww);
        ui_rich_text(ui_teacher_text(teach[c.hover]).c_str());
        std::string hired = ui_teachers_hired_line((s16)g_rw(GV_cur_loc));
        if (!hired.empty()) ui_rich_textf_dim("Hired here now: %s.", hired.c_str());
        if (const char *why = greyed_hint(c.piece[c.hover])) { ImGui::Separator(); ImGui::TextDisabled("Not possible now"); ui_rich_text(why); }
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
    else if (tipmenu && c.hover >= 0 && c.hover == stone_piece && stone_offer > 0 && c.piece[c.hover].state == 1) {
        float ww = tip_place(21);                                  /* the stone on offer, against the party's */
        ImGui::BeginTooltip();
        ImGui::TextDisabled("%s", no_gain(c.hover) ? "Nothing to gain here" : "A better Philosopher's Stone");
        ImGui::PushTextWrapPos(ww);
        char b[400];
        if (!no_gain(c.hover) && stone_have > 0) snprintf(b, sizeof b, "The stone %s offers is of quality %d; the party's is %d. A batch's chance of success would rise by %d.",
                                                          stone_uni ? "the university" : "this alchemist", stone_offer, stone_have, stone_offer - stone_have);
        else if (!no_gain(c.hover)) snprintf(b, sizeof b, "The stone %s offers is of quality %d; the party has none. It would add %d to a batch's chance of success.",
                                             stone_uni ? "the university" : "this alchemist", stone_offer, stone_offer);
        else if (stone_uni) snprintf(b, sizeof b, "The university's best stone is of quality %d, no better than the party's %d. Asking gains nothing, and if it goes wrong it costs "
                                                  "two hours, a point of the party's reputation here and the question for a week.", stone_offer, stone_have);
        else snprintf(b, sizeof b, "This alchemist's best stone is of quality %d, no better than the party's %d. Asking gains nothing, and if the alchemist "
                                   "will not talk the party is turned away for 60 hours.", stone_offer, stone_have);
        ImGui::TextUnformatted(b);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
    else if (tipmenu && c.hover >= 0 && c.hover == gate_piece) {
        float ww = tip_place(21);
        ImGui::BeginTooltip();
        ImGui::TextDisabled("%s", gate_certain ? "A fight" : "Perhaps a fight");
        ImGui::PushTextWrapPos(ww);
        ImGui::TextUnformatted(gate_why.c_str());
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
    else if (tipmenu && c.hover >= 0 && c.hover < c.npieces && task_of[c.hover] >= 0) {
        float ww = tip_place(22);                                  /* the task this way leads to */
        int n = 0; for (const CardTask &t : tasks) n += t.opt == c.piece[c.hover].opt;
        ImGui::BeginTooltip();
        ImGui::TextDisabled("%s", n > 1 ? "Your tasks" : "Your task");
        ImGui::PushTextWrapPos(ww);
        for (const CardTask &t : tasks)
            if (t.opt == c.piece[c.hover].opt) { ImGui::TextUnformatted(t.title.c_str()); ImGui::TextDisabled("%s", t.text.c_str()); }
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
    else if (tipmenu && c.hover >= 0 && c.hover < c.npieces && c.piece[c.hover].type != 1 && c.piece[c.hover].type != 2 && greyed_hint(c.piece[c.hover])) {
        float ww = tip_place(21);                                  /* beside the option (the pointer may be the game's only) */
        ImGui::BeginTooltip();
        ImGui::TextDisabled("Not possible now");
        ImGui::PushTextWrapPos(ww);
        ui_rich_text(greyed_hint(c.piece[c.hover]));        /* (an amount with its coin: ui/richtext.h) */
        if (no_gain(c.hover)) ImGui::TextDisabled("Nor would it gain anything: the stone sold here is of quality %d, the party's %d.", stone_offer, stone_have);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
    else if (tipmenu && assist_card_help && c.hover >= 0 && c.hover < c.npieces && c.popup_piece != c.hover
             && (c.piece[c.hover].type == 1 || c.piece[c.hover].type == 2)) {
        /* which saints or items this option takes, and who in the party knows or carries them
           (even when nobody does: the option is then greyed) */
        const CvPiece &p = c.piece[c.hover];
        int npar = (s16)g_rw(G_PARTY_SIZE);
        auto alive = [](int m) { int st = g_rb((u16)(G_MEMBER(m) + G_M_STATUS)); return st != 0 && st != 3; };
        std::vector<std::pair<std::string, std::string>> rows;
        bool formulas = false;
        for (int j = 0; j < 4; j++) {
            int id = (s16)g_rw((u16)((p.type == 2 ? 0xee4b : 0xee43) + 2 * j));
            std::string what, who;
            if (p.type == 2) {
                if (id < 0 || id >= (int)gamedata.saints.size()) continue;
                what = gamedata.saints[id].name;
                for (int m = 0; m < npar && m < 5; m++)
                    if (alive(m) && (rb(G_FAR2E38, (u16)(0x082c + m * 0x14 + (id >> 3))) & (0x80 >> (id & 7)))) { if (!who.empty()) who += " "; who += rich_member(m); }
            } else if (cv_formula_of_slot(j) >= 0) {             /* a formula on sale: who knows it already */
                int fo = cv_formula_of_slot(j);
                what = gamedata.formula_names[fo] + " (" + cv_formula_price(j) + ")";
                for (int m = 0; m < npar && m < 5; m++)
                    if (alive(m) && gm_knows_formula(m, fo)) { if (!who.empty()) who += " "; who += rich_member(m); }
                rows.push_back({ what, who.empty() ? "nobody knows it yet" : "known to " + who });
                formulas = true;
                continue;
            } else {
                if (id <= 0 || id >= (int)gamedata.items.size()) continue;
                what = game_text(gamedata.items[id].name.c_str(), gamedata.items[id].name.size());
                for (int m = 0; m < npar && m < 5; m++) {
                    if (!alive(m)) continue;
                    int n = 0, k = g_rb((u16)(G_MEMBER(m) + G_M_NITEMS));
                    for (int i = 0; i < k && i < 64; i++) if ((rw(G_FAR2E38, G_ITEM(m, i)) & 0xfff) == id) n += rb(G_FAR2E38, (u16)(G_ITEM(m, i) + 4));
                    if (n) { if (!who.empty()) who += "  "; who += rich_member(m, n > 1 ? std::to_string(n) : ""); }
                }
            }
            rows.push_back({ what, who.empty() ? (p.type == 2 ? "nobody knows this saint" : "nobody carries one") : who });
        }
        if (!rows.empty()) {
            float ww = tip_place(30);                              /* beside the option (the pointer may be the game's only) */
            ImGui::BeginTooltip();
            ImGui::TextDisabled("%s", p.type == 2 ? "Saints who help here" : formulas ? "Formulae for sale" : "Items that help here");
            for (auto &r : rows) { ImGui::TextUnformatted(r.first.c_str()); ImGui::SameLine(); ui_rich_textf_dim("  %s", r.second.c_str()); }
            if (const char *why = greyed_hint(p)) {                 /* greyed for a reason of the card's own */
                ImGui::Separator();
                ImGui::PushTextWrapPos(fminf(ww, ImGui::GetFontSize() * 21));
                ImGui::TextDisabled("Not possible now");
                ui_rich_text(why);
                ImGui::PopTextWrapPos();
            }
            if (formulas) ImGui::TextDisabled("(click to choose who learns one; point at a formula there for its details)");
            else if (p.state == 1) ImGui::TextDisabled("(click to choose who)");
            ImGui::EndTooltip();
        }
    }
    /* the docks' boats (assist.docks_map): a small map of where they go, under the list */
    if (menu && config_get_int("assist.docks_map", 1)) {
        char cn[16]; g_str(G_DGROUP, GV_card_name, cn, sizeof cn);
        int here = (s16)g_rw(G_CUR_LOC);
        if (!strcasecmp(cn, "$DOCKS00") && here >= 0 && here < (int)gamedata.cities.size()) {
            int dest[5], opt[5], st[5], n = 0, hov = -1;
            std::string fares[5]; const char *farep[5] = {};
            for (int j = 0; j < c.npieces && n < 5; j++) {
                const CvPiece &p = c.piece[j];
                if (!p.is_opt || p.opt < 0 || p.opt > 4 || (p.state != 1 && p.state != 2)) continue;
                int d = gamedata.cities[here].docks[p.opt];
                if (d < 0 || d >= (int)gamedata.cities.size() || d == here) continue;
                {   /* the fare, as the option says it ("..., for 3 groschen.") */
                    size_t f0 = utf[j].rfind("for ");
                    fares[n] = f0 == std::string::npos ? std::string() : utf[j].substr(f0 + 4);
                    while (!fares[n].empty() && (fares[n].back() == '.' || fares[n].back() == ' ')) fares[n].pop_back();
                    farep[n] = fares[n].c_str();
                }
                dest[n] = d; opt[n] = p.opt; st[n] = p.state; n++;
                if (c.hover == j || (c.key_line >= 0 && p.line == c.key_line)) hov = p.opt;
            }
            ImVec2 p1 = g2w(312, 194);
            float y0 = by1 + 4 * k, y1 = p1.y - 4 * k, x0 = bx0, x1 = fminf(p1.x - 4 * k, bx0 + (y1 - y0) * 1.9f);
            if (n > 0 && y1 - y0 > 40 * k) {
                int o = worldmap_docks_inset(x0, y0, x1, y1, here, dest, opt, st, farep, n, hov);
                if (o >= 0) cardview_choose(o);
            }
        }
    }
}
