/* repwatch.cpp - a town's opinion of the party (its reputation, location +0x12) watched while a
 * card or the map is up: each change is remembered with what was going on when it came (the card,
 * the option just chosen on it, the date), flashed beside "Reputation" in the right margin
 * (ui/margins.cpp) and listed in its tooltip, and written to the log. A load (or a new game)
 * changes many towns at once: taken as the new state, not as news. */
#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
#include "imgui.h"
#include "gamedata.h"
#include "repwatch.h"
extern "C" {
#include "../runtime/cpu.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
u32 plat_ticks_ms(void);
int saves_loading_cover(void);
extern char cardview_last_choice[160]; extern unsigned cardview_last_choice_t;
}
std::string game_text(const char *s, size_t n);

#define NTOWN 0x5c
static int last[NTOWN], have, seed = -99999;
static std::vector<RepChange> hist;

extern "C" void repwatch_tick(void)
{
    int gm = game_mode();
    if (gm != GM_CARD && gm != GM_MAP) return;
    int sd = (s16)g_rw(G_PARTY_SEED);
    int now[NTOWN];
    for (int i = 0; i < NTOWN; i++) { u32 r = g_location(i); now[i] = r ? (s16)rw((u16)(r >> 16), (u16)((u16)r + 0x12)) : 0; }
    int changed = 0;
    for (int i = 0; i < NTOWN; i++) if (now[i] != last[i]) changed++;
    char card[16]; g_str(G_DGROUP, GV_card_name, card, sizeof card);
    /* the save/load menu ($Selec..) and a load under way: towns read from the save, not news */
    if (saves_loading_cover() || !strncmp(card, "$Selec", 6) || !strncmp(card, "$SELEC", 6)) { memcpy(last, now, sizeof last); have = 1; seed = sd; return; }
    if (!have || sd != seed || changed > 3) { memcpy(last, now, sizeof last); have = 1; if (sd != seed) hist.clear(); seed = sd; return; }
    if (!changed) return;
    for (int i = 0; i < NTOWN; i++) {
        if (now[i] == last[i]) continue;
        RepChange c; c.loc = i; c.delta = now[i] - last[i]; c.value = now[i]; c.t = plat_ticks_ms();
        c.day = (s16)g_rw(G_CUR_DAY); c.month = (s16)g_rw(G_CUR_MONTH); c.year = (s16)g_rw(G_CUR_YEAR);
        c.card = card;
        c.choice = plat_ticks_ms() - cardview_last_choice_t < 30000 && cardview_last_choice[0] ? game_text(cardview_last_choice, strlen(cardview_last_choice)) : "";
        if (gm == GM_MAP && c.choice.empty()) c.choice = "";
        hist.push_back(c);
        if (hist.size() > 200) hist.erase(hist.begin());
        std::string town = i < (int)gamedata.cities.size() ? gamedata.cities[i].name : std::to_string(i);
        fprintf(stderr, "reputation: %s %+d (now %d) at card %s%s%s\n", town.c_str(), c.delta, c.value, card, c.choice.empty() ? "" : " after: ", c.choice.c_str());
    }
    memcpy(last, now, sizeof last);
}

const RepChange *repwatch_last(int loc)
{
    for (int k = (int)hist.size() - 1; k >= 0; k--) if (hist[k].loc == loc) return &hist[k];
    return nullptr;
}
void repwatch_list(int loc, std::vector<RepChange> &out)
{
    out.clear();
    for (const RepChange &c : hist) if (c.loc == loc) out.push_back(c);
}
