// questicons.cpp - a picture for each kind of quest (ui/quests.h QuestCat), drawn with ImGui's shapes
// on a small parchment roundel, in the manner of a woodcut or a guild's sign: a great helm before
// crossed swords for a robber knight, a purse for a reward waiting, the miners' hammer and pick (Schlägel und
// Eisen) for a mine's trouble, a horn for the Wild Hunt, a lowered portcullis for a town that bars
// the party, the pillory where the watch wants it ... A merchant's errand to fetch a thing from a
// cave, tomb or lake is a chest; papers to be got from someone in a town, a sealed letter and a
// dagger.
#include "imgui.h"
#include <math.h>
#include "pen.h"
#include "quests.h"

using pen::C;
using pen::Pen;

const char *QUEST_CAT_NAME[QC_N] = {
    "Quest", "Robber knight", "Reward waiting", "A treasure to fetch", "Papers to get in a town", "Mine",
    "Dragon", "Wild Hunt", "Unwelcome in a town", "Wanted in a town", "Sabbat", "City politics", "City at war"
};

namespace {
const ImU32 INK = C(0.16f, 0.10f, 0.05f), PARCH = C(0.95f, 0.90f, 0.77f), RIM = C(0.47f, 0.31f, 0.16f);
const ImU32 STEEL = C(0.76f, 0.78f, 0.82f), IRON = C(0.42f, 0.43f, 0.47f), GOLD = C(0.93f, 0.72f, 0.20f);
const ImU32 RED = C(0.72f, 0.12f, 0.10f), WOOD = C(0.52f, 0.33f, 0.16f), LEATHER = C(0.58f, 0.36f, 0.18f);

/* a bar from (cx, cy) along (dx, dy): lb units back, lf forward, half as wide as wb at the back and
   wf at the front (wf 0: a point); with an ink outline o units wide */
void bar(const Pen &p, float cx, float cy, float dx, float dy, float lb, float lf, float wb, float wf, ImU32 k, float o = 0)
{
    float l = sqrtf(dx * dx + dy * dy); dx /= l; dy /= l;
    float nx = -dy, ny = dx;
    auto q = [&](float b, float f, float w0, float w1, ImU32 col) {
        p.quad(cx - dx * b + nx * w0, cy - dy * b + ny * w0, cx + dx * f + nx * w1, cy + dy * f + ny * w1,
               cx + dx * f - nx * w1, cy + dy * f - ny * w1, cx - dx * b - nx * w0, cy - dy * b - ny * w0, col);
    };
    if (o > 0) q(lb + o, lf + o, wb + o, wf + o * 0.6f, INK);
    q(lb, lf, wb, wf, k);
}
void coin(const Pen &p, float x, float y, float r, float o)
{
    p.disc(x, y, r + o, INK); p.disc(x, y, r, GOLD); p.ring(x, y, r * 0.62f, C(0.70f, 0.50f, 0.10f), 0.6f);
}
} // namespace

void ui_draw_quest_icon(ImDrawList *dl, ImVec2 c, float s, int cat)
{
    Pen p{ dl, c, s / 32.0f };
    p.mw = 1.0f;
    const float o = fmaxf(0.8f, 1.0f / p.u);                                 /* an outline: at least a pixel */
    p.disc(0, 0, 15.4f, RIM); p.disc(0, 0, 14.0f, PARCH);
    switch (cat) {
    case QC_ROBBER: {                                                       /* a great helm before two crossed swords */
        for (int sd = -1; sd <= 1; sd += 2) {
            float hx = -6.6f * sd, hy = 6.6f, dx = (float)sd * 0.7071f, dy = -0.7071f;
            bar(p, hx, hy, dx, dy, 0, 21.5f, 1.3f, 0.2f, STEEL, o);          // the blade
            bar(p, hx, hy, -dx, -dy, 0, 3.0f, 0.85f, 0.85f, WOOD, o);         // the grip
            bar(p, hx, hy, 1, (float)sd, 3.3f, 3.3f, 0.8f, 0.8f, GOLD, o);    // the cross-guard
            p.disc(hx - dx * 3.6f, hy - dy * 3.6f, 1.3f + o, INK); p.disc(hx - dx * 3.6f, hy - dy * 3.6f, 1.3f, GOLD);
        }
        p.rect(-5.8f - o, -6.0f - o, 5.8f + o, 8.0f + o, INK, 2.0f);
        p.rect(-5.8f, -6.0f, 5.8f, 8.0f, STEEL, 1.6f);
        p.rect(2.0f, -6.0f, 5.8f, 8.0f, C(0.55f, 0.57f, 0.62f), 1.6f);       // its shaded side
        p.rect(-0.8f, -6.0f, 0.8f, 8.0f, C(0.90f, 0.91f, 0.94f));           // the brow and nose strip
        p.rect(-5.0f, -1.6f, -1.6f, 0.2f, INK); p.rect(1.6f, -1.6f, 5.0f, 0.2f, INK);   // the eye slits
        for (int i = 0; i < 2; i++) for (int j = 0; j < 2; j++) p.disc(2.4f + i * 1.7f, 3.4f + j * 1.8f, 0.55f, INK);
        break; }
    case QC_REWARD: {                                                       /* florins stacked high, waiting to be paid out (not a purse: earning at the inn has it) */
        for (int st = 0; st < 2; st++) {
            float x = st ? 4.2f : -4.6f, base = st ? 10.0f : 11.0f; int n = st ? 4 : 6;
            for (int k = 0; k < n; k++) {
                float y = base - k * 2.6f;
                p.ellipse(x, y, 6.4f + o, 2.2f + o, INK); p.ellipse(x, y, 6.4f, 2.2f, k == n - 1 ? GOLD : C(0.80f, 0.58f, 0.14f));
            }
            p.ellipse(x, base - (n - 1) * 2.6f, 3.6f, 1.1f, C(0.98f, 0.86f, 0.40f));   // the top coin's face
        }
        break; }
    case QC_RETRIEVE: {                                                     /* an iron-bound chest */
        ImU32 oak = C(0.58f, 0.38f, 0.18f), dark = C(0.42f, 0.26f, 0.12f);
        p.rect(-11.0f - o, -7.5f - o, 11.0f + o, 9.5f + o, INK, 3.0f);
        p.rect(-11.0f, -7.5f, 11.0f, -0.5f, C(0.66f, 0.44f, 0.22f), 3.0f);  // the lid
        p.rect(-11.0f, -1.0f, 11.0f, 9.5f, oak, 0.6f);                      // the body
        p.line(-11.0f, -0.8f, 11.0f, -0.8f, INK, 0.9f);
        for (int k = 0; k < 2; k++) p.line(-10.0f, 3.0f + k * 3.2f, 10.0f, 3.0f + k * 3.2f, dark, 0.5f);   // the boards
        p.line(-10.0f, -4.4f, 10.0f, -4.4f, dark, 0.5f);
        for (int sd = -1; sd <= 1; sd += 2) p.rect(sd * 7.2f - 1.3f, -7.3f, sd * 7.2f + 1.3f, 9.4f, IRON);   // the iron bands
        p.rect(-2.2f, -3.2f, 2.2f, 2.4f, GOLD, 0.5f);                        // the lock
        p.disc(0, -1.2f, 0.8f, INK); p.tri(-0.6f, -1.0f, 0.6f, -1.0f, 0, 1.2f, INK);
        break; }
    case QC_PAPERS: {                                                       /* a sealed letter, and a dagger */
        p.rect(-11.0f - o, -6.5f - o, 5.5f + o, 7.5f + o, INK);
        p.rect(-11.0f, -6.5f, 5.5f, 7.5f, C(0.98f, 0.95f, 0.86f));
        p.line(-11.0f, -6.5f, -2.75f, 1.6f, C(0.55f, 0.45f, 0.30f), 0.6f); p.line(5.5f, -6.5f, -2.75f, 1.6f, C(0.55f, 0.45f, 0.30f), 0.6f);
        p.disc(-2.75f, 2.0f, 2.8f + o * 0.6f, INK); p.disc(-2.75f, 2.0f, 2.8f, RED); p.disc(-2.75f, 2.0f, 1.3f, C(0.55f, 0.06f, 0.05f));
        bar(p, 7.8f, 3.0f, 0, -1, 0, 14.0f, 1.3f, 0.1f, STEEL, o);          // the blade, point up
        bar(p, 7.8f, 3.0f, 0, 1, 0, 5.4f, 0.9f, 0.9f, C(0.22f, 0.16f, 0.12f), o);   // the grip
        bar(p, 7.8f, 3.0f, 1, 0, 3.4f, 3.4f, 0.75f, 0.75f, GOLD, o);         // the cross-guard
        p.disc(7.8f, 9.4f, 1.3f + o, INK); p.disc(7.8f, 9.4f, 1.3f, GOLD);
        break; }
    case QC_MINE: {                                                         /* Schlägel und Eisen */
        bar(p, 9.5f, 9.5f, -1, -1, 0, 22.0f, 0.95f, 0.95f, WOOD, o);        // the iron's helve
        bar(p, -5.6f, -5.6f, -1, 1, 3.0f, 6.8f, 2.0f, 0.0f, IRON, o);        // the iron: its point down to the left
        bar(p, -9.5f, 9.5f, 1, -1, 0, 22.0f, 0.95f, 0.95f, WOOD, o);        // the hammer's helve
        bar(p, 5.8f, -5.8f, 1, 1, 4.6f, 4.6f, 2.3f, 2.3f, IRON, o);          // the hammer
        p.line(5.8f - 3.0f, -5.8f - 3.0f + 1.2f, 5.8f + 3.0f - 1.2f, -5.8f + 3.0f, C(0.62f, 0.63f, 0.68f), 0.7f);
        break; }
    case QC_DRAGON: {                                                       /* a dragon's head */
        ImU32 g = C(0.30f, 0.56f, 0.26f), bone = C(0.92f, 0.88f, 0.74f);
        p.tri(3.0f, -7.8f, 6.4f, -7.4f, 9.6f, -13.4f, INK); p.tri(3.8f, -7.8f, 5.8f, -7.6f, 9.0f, -12.2f, bone);
        p.tri(6.0f, -6.8f, 8.6f, -5.6f, 12.8f, -9.6f, INK); p.tri(6.6f, -6.6f, 8.2f, -5.8f, 11.8f, -9.0f, bone);
        p.tri(10.0f, -0.6f, 10.0f, 2.8f, 13.4f, 0.8f, INK); p.tri(9.6f, 3.6f, 9.4f, 7.2f, 12.8f, 6.0f, INK);
        p.poly({ -12.0f, -0.6f, -3.0f, 0.2f, -10.6f, 3.2f }, C(0.60f, 0.08f, 0.08f));       // the open mouth
        p.poly_line({ -12.2f, -3.2f, -8.0f, -5.8f, -3.0f, -6.2f, 0.0f, -8.4f, 4.2f, -8.8f, 8.2f, -6.2f, 10.6f, -1.0f, 10.2f, 4.0f, 9.2f, 12.4f,
                      0.8f, 12.4f, 1.4f, 6.2f, -2.0f, 3.8f, -10.2f, 4.4f, -10.8f, 2.9f, -3.6f, 1.2f, -2.8f, 0.0f, -11.8f, -0.9f }, INK, 2.0f);
        p.poly({ -12.2f, -3.2f, -8.0f, -5.8f, -3.0f, -6.2f, 0.0f, -8.4f, 4.2f, -8.8f, 8.2f, -6.2f, 10.6f, -1.0f, 10.2f, 4.0f, 9.2f, 12.4f,
                 0.8f, 12.4f, 1.4f, 6.2f, -2.0f, 3.8f, -10.2f, 4.4f, -10.8f, 2.9f, -3.6f, 1.2f, -2.8f, 0.0f, -11.8f, -0.9f }, g);
        p.poly({ 0.8f, 12.4f, 3.4f, 12.4f, 3.6f, 7.0f, 1.4f, 6.2f }, C(0.80f, 0.78f, 0.40f));   // the throat's scales
        p.tri(-10.2f, -0.8f, -9.0f, -0.7f, -9.7f, 0.5f, bone); p.tri(-7.4f, -0.5f, -6.2f, -0.4f, -6.9f, 0.7f, bone);
        p.disc(1.4f, -4.6f, 1.6f, C(0.98f, 0.82f, 0.20f)); p.line(1.4f, -5.8f, 1.4f, -3.4f, INK, 0.7f);
        p.disc(-10.4f, -3.0f, 0.55f, INK);
        break; }
    case QC_HUNT: {                                                         /* a hunting horn */
        float x0 = -11.0f, y0 = -4.0f, x1 = -2.5f, y1 = 12.5f, x2 = 9.6f, y2 = -5.0f;
        auto at = [&](float t, float *x, float *y) { float a = 1 - t; *x = a * a * x0 + 2 * a * t * x1 + t * t * x2; *y = a * a * y0 + 2 * a * t * y1 + t * t * y2; };
        auto rad = [](float t) { return 1.3f + powf(t, 1.6f) * 3.6f; };
        for (int pass = 0; pass < 2; pass++)
            for (int k = 0; k <= 40; k++) { float t = k / 40.0f, x, y; at(t, &x, &y); p.disc(x, y, rad(t) + (pass ? 0 : o), pass ? C(0.93f, 0.87f, 0.70f) : INK); }
        for (int k = 0; k <= 40; k++) { float t = k / 40.0f, x, y; at(t, &x, &y); p.disc(x + 0.25f, y + rad(t) * 0.45f, rad(t) * 0.35f, C(0.80f, 0.70f, 0.50f)); }   // its shade
        for (float t : { 0.30f, 0.66f, 0.96f }) {
            float x, y, xa, ya; at(t, &x, &y); at(t - 0.02f, &xa, &ya);
            float dx = x - xa, dy = y - ya, l = sqrtf(dx * dx + dy * dy); dx /= l; dy /= l;
            float r = rad(t) + 0.2f; p.line(x - dy * r, y + dx * r, x + dy * r, y - dx * r, GOLD, 1.5f);
        }
        p.ellipse(x2, y2, rad(1) * 0.95f, rad(1) * 0.42f, C(0.25f, 0.17f, 0.10f), atan2f(y2 - y1, x2 - x1) + 1.5708f);
        p.rect(x0 - 1.4f, y0 - 0.9f, x0 + 0.2f, y0 + 0.9f, GOLD);            // the mouthpiece
        break; }
    case QC_UNWELCOME: {                                                    /* a gate with its portcullis down */
        ImU32 st = C(0.72f, 0.67f, 0.60f);
        p.rect(-11.0f - o, -3.0f - o, 11.0f + o, 10.0f + o, INK);
        for (int i = 0; i < 4; i++) p.rect(-11.0f + i * 6.4f - o, -7.0f - o, -8.0f + i * 6.4f + o, -2.0f, INK);
        p.rect(-11.0f, -3.0f, 11.0f, 10.0f, st);
        for (int i = 0; i < 4; i++) p.rect(-11.0f + i * 6.4f, -7.0f, -8.0f + i * 6.4f, -2.5f, st);
        p.line(-11.0f, 2.0f, -5.0f, 2.0f, C(0.50f, 0.45f, 0.40f), 0.5f); p.line(5.0f, 5.5f, 11.0f, 5.5f, C(0.50f, 0.45f, 0.40f), 0.5f);
        p.rect(-4.6f, 1.5f, 4.6f, 10.0f, C(0.14f, 0.10f, 0.07f)); p.disc(0, 1.5f, 4.6f, C(0.14f, 0.10f, 0.07f));
        ImU32 gr = C(0.62f, 0.58f, 0.52f);
        for (int i = -1; i <= 1; i++) p.line(i * 2.4f, -2.6f, i * 2.4f, 10.0f, gr, 1.0f);
        for (int j = 0; j < 3; j++) p.line(-4.4f, 1.6f + j * 2.9f, 4.4f, 1.6f + j * 2.9f, gr, 0.9f);
        break; }
    case QC_WANTED: {                                                       /* the pillory */
        bar(p, 0, 12.5f, 0, -1, 0, 19.0f, 1.3f, 1.3f, WOOD, o);              // the post
        p.rect(-11.5f - o, -7.0f - o, 11.5f + o, -0.5f + o, INK, 0.8f);
        p.rect(-11.5f, -7.0f, 11.5f, -0.5f, C(0.62f, 0.42f, 0.22f), 0.6f);   // the boards
        p.line(-11.5f, -3.75f, 11.5f, -3.75f, C(0.35f, 0.22f, 0.10f), 0.6f);
        p.disc(0, -3.75f, 2.4f, INK); p.disc(-7.0f, -3.75f, 1.5f, INK); p.disc(7.0f, -3.75f, 1.5f, INK);   // for the head and the hands
        p.rect(-11.0f, 11.0f, 11.0f, 12.8f, C(0.45f, 0.30f, 0.15f));          // the platform
        p.rect(-2.6f, 8.8f, 2.6f, 11.0f, C(0.52f, 0.34f, 0.16f));
        break; }
    case QC_SABBAT: {                                                       /* a crescent moon, and a broom */
        p.disc(-2.0f, -3.0f, 8.6f, INK); p.disc(-2.0f, -3.0f, 8.0f, GOLD);
        p.disc(1.8f, -5.6f, 7.4f, PARCH);
        p.disc(8.6f, -0.4f, 0.8f, GOLD); p.disc(5.4f, -10.6f, 0.7f, GOLD);
        bar(p, 11.0f, -8.0f, -1.0f, 0.95f, 0, 17.0f, 0.75f, 0.75f, WOOD, o);    // the handle
        p.poly({ -5.0f, 6.6f, -3.2f, 8.4f, -7.6f, 14.0f, -12.4f, 9.4f }, INK);
        p.poly({ -5.0f, 7.2f, -3.8f, 8.4f, -7.8f, 13.0f, -11.4f, 9.4f }, C(0.82f, 0.66f, 0.30f));
        for (int k = 0; k < 3; k++) p.line(-4.6f - k * 0.6f, 8.0f - k * 0.6f + 0.6f, -10.0f + k * 1.6f, 9.8f + k * 1.4f, C(0.60f, 0.45f, 0.18f), 0.5f);
        p.line(-5.6f, 6.4f, -3.0f, 9.0f, GOLD, 1.0f);
        break; }
    case QC_POLITICS: {                                                     /* a swallow-tailed banner on its pole, a handbill nailed below */
        bar(p, -7.0f, 12.5f, 0, -1, 0, 24.0f, 0.9f, 0.9f, WOOD, o);          // the pole
        p.disc(-7.0f, -11.8f, 1.4f + o, INK); p.disc(-7.0f, -11.8f, 1.4f, GOLD);
        p.poly({ -6.2f, -10.0f, 10.0f, -10.0f, 5.6f, -5.4f, 10.0f, -0.8f, -6.2f, -0.8f }, INK);
        p.poly({ -6.2f, -9.2f, 8.4f, -9.2f, 4.4f, -5.4f, 8.4f, -1.6f, -6.2f, -1.6f }, RED);
        p.rect(-1.0f - o, 2.4f - o, 9.0f + o, 11.0f + o, INK);
        p.rect(-1.0f, 2.4f, 9.0f, 11.0f, C(0.98f, 0.95f, 0.86f));
        for (int k = 0; k < 3; k++) p.line(1.0f, 4.8f + k * 2.2f, 7.0f, 4.8f + k * 2.2f, C(0.55f, 0.45f, 0.30f), 0.6f);
        p.disc(4.0f, 2.8f, 0.7f, IRON);
        break; }
    case QC_WAR: {                                                          /* two pikes crossed behind a round shield */
        for (int sd = -1; sd <= 1; sd += 2) {
            float hx = 8.5f * sd, hy = 11.0f, dx = -(float)sd * 0.62f, dy = -0.78f;
            bar(p, hx, hy, dx, dy, 0, 26.0f, 0.8f, 0.8f, WOOD, o);           // the shaft
            float tx = hx + dx * 26.0f, ty = hy + dy * 26.0f;
            bar(p, tx, ty, dx, dy, 0, 4.2f, 1.4f, 0.1f, STEEL, o);           // the head
        }
        p.disc(0, 2.0f, 8.4f + o, INK); p.disc(0, 2.0f, 8.4f, RED);
        p.ring(0, 2.0f, 7.0f, GOLD, 0.9f);
        p.disc(0, 2.0f, 2.4f + o, INK); p.disc(0, 2.0f, 2.4f, STEEL);       // the boss
        break; }
    default:                                                                /* a scroll */
        p.rect(-7.0f, -9.0f, 7.0f, 9.0f, C(0.98f, 0.95f, 0.86f)); p.rect_line(-7.0f, -9.0f, 7.0f, 9.0f, INK, 0.8f);
        for (int k = 0; k < 4; k++) p.line(-4.5f, -5.0f + k * 3.4f, 4.5f, -5.0f + k * 3.4f, C(0.55f, 0.45f, 0.30f), 0.6f);
    }
    p.ring(0, 0, 15.4f, INK, 0.7f);
}
