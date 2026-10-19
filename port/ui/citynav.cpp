// citynav.cpp - "Go to" in cities. A city in Darklands is a maze of card menus (main street,
// market, churches, the crafts district, side streets ...); the list here names every place at
// once, and picking one walks the party there through the game's own menus - the very options a
// player would click, one card after the other, at the game's pace. It stops at anything it
// does not know (an encounter on the way, a closed gate), at a key press or click of the
// player, and when it has chosen the place.
//
// Cards are told apart by their names (s043 card_name, DS:e3d4: "$MainS01" ...); the option
// lines by their text in MSGFILES (the card file's text block D_ee53, option k = the k-th
// option mark), so nothing depends on the crisp text or on the screen.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <math.h>
#include <float.h>
#include <string>
#include <vector>
#include <map>
#include "imgui.h"
#include "ui.h"
#include "gamedata.h"
#include "teachersui.h"
extern "C" {
#include "../mods/offices.h"
}
#include "townicons.h"
#include "quests.h"
extern "C" {
int datamods_generation(void);
#include "../runtime/cpu.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "../runtime/config.h"
#include "../runtime/hle.h"
#include "../mods/assist.h"
#include "../mods/cardview.h"
u32 plat_ticks_ms(void);
void plat_click_game(int gx, int gy);
void ui_notice(const char *msg);
}

/* ---------------------------------------------------------------- the card files */
static std::map<std::string, std::vector<std::string>> cards;   /* "$MAINS01" -> text blocks */
static bool cards_loaded;

static void load_cards(void)
{
    cards_loaded = true;
    char path[1300]; dos_host_path_read("MSGFILES", path, sizeof path);
    FILE *f = fopen(path, "rb");
    if (!f) return;
    std::vector<unsigned char> d;
    unsigned char buf[65536]; size_t n;
    while ((n = fread(buf, 1, sizeof buf, f)) > 0) d.insert(d.end(), buf, buf + n);
    fclose(f);
    if (d.size() < 2) return;
    unsigned cnt = d[0] | d[1] << 8;
    for (unsigned i = 0; i < cnt && 2 + (i + 1) * 24 <= d.size(); i++) {
        const unsigned char *e = &d[2 + i * 24];
        char name[13]; memcpy(name, e, 12); name[12] = 0;
        for (char *p = name; *p; p++) { *p = (char)toupper((unsigned char)*p); if (*p == '.') *p = 0; }
        unsigned len = e[16] | e[17] << 8 | e[18] << 16 | (unsigned)e[19] << 24, off = e[20] | e[21] << 8 | e[22] << 16 | (unsigned)e[23] << 24;
        if ((size_t)off + len > d.size() || len < 1) continue;
        const unsigned char *m = &d[off], *end = m + len;
        unsigned nb = *m++;
        std::vector<std::string> blocks;
        for (unsigned b = 0; b < nb && m + 5 <= end; b++) {
            m += 5;
            const unsigned char *z = m; while (z < end && *z) z++;
            blocks.emplace_back((const char *)m, (size_t)(z - m));
            m = z + 1;
        }
        cards[name] = blocks;
    }
}

/* the option texts of a text block, by option number (lower case, "$vars" kept) */
static std::vector<std::string> block_options(const std::string &t)
{
    std::vector<std::string> o;
    for (size_t i = 0; i < t.size(); i++) {
        unsigned char c = (unsigned char)t[i];
        if (c != 0x15 && c != 0x16 && c != 0x10 && c != 0x06) continue;
        size_t e = t.find('\n', i + 1); if (e == std::string::npos) e = t.size();
        std::string s = t.substr(i + 1, e - i - 1);
        size_t sep = s.find('\x1d'); if (sep != std::string::npos) s = s.substr(sep + 1);
        for (char &ch : s) ch = (char)tolower((unsigned char)ch);
        o.push_back(s);
        i = e - 1;
    }
    return o;
}

/* whether block `block` of card `card` (as the card files have it, $variables unexpanded)
   contains `word` (ui/quests.cpp: what a card tells of a Sabbat) */
extern "C" int citynav_card_block_has(const char *card, int block, const char *word)
{
    { static int gen = -1; int datamods_generation(void); if (gen != datamods_generation()) { gen = datamods_generation(); cards.clear(); cards_loaded = false; } }
    if (!cards_loaded) load_cards();
    std::string k(card); for (char &c : k) c = (char)toupper((unsigned char)c);
    auto it = cards.find(k);
    if (it == cards.end() || block < 0 || block >= (int)it->second.size()) return 0;
    return it->second[block].find(word) != std::string::npos;
}

static std::string card_now(void)
{
    char n[13]; g_str(G_DGROUP, 0xe3d4, n, sizeof n);
    for (char *p = n; *p; p++) *p = (char)toupper((unsigned char)*p);
    return n;
}

/* ---------------------------------------------------------------- the city */
enum Hub { H_MAIN, H_MARKET, H_CHURCHES, H_CRAFTS, H_ARMS, H_CIVIL, H_SIDE, H_SQUARE, H_N, H_NONE = -1 };
static const char *HUB_CARD[H_N] = { "$MAINS", "$MARKE", "$CHURC", "$BUSIN", "$MILCR", "$CIVCR", "$SIDES", "$CITYS" };

enum Dest {
    D_MERCHANT, D_FOREIGN, D_SWORD, D_ARMOR, D_BOW, D_BLACKSMITH, D_SOLDIERS, D_JEWEL, D_TINKER, D_CLOTH,
    D_ALCHEMIST, D_HERBAL, D_PHARMACIST, D_PHYSICIAN,
    D_FUGGER, D_MEDICI, D_HANSE, D_PAWN,
    D_CATHEDRAL, D_CHURCH, D_MONASTERY, D_UNIVERSITY, D_HOSPITAL,
    D_INN, D_SQUARE, D_TOWNHALL, D_PRISON, D_BARRACKS, D_FORTRESS, D_DOCKS, D_SLUM, D_GROVE, D_WALL, D_LEAVE, D_N
};
/* a place: its name in the list, what it is (the card texts, said plainly), its section, the
   building bit / shop quality index of DARKLAND.CTY that it needs (-1 any; -2 the bowyer or the
   gunsmith), and its picture (ui/townicons.h) */
struct DestInfo { const char *label; const char *desc; int group; int building; int shop; int icon; };
static const DestInfo DEST[D_N] = {
    { "Merchant", "Market stalls: everyday goods to buy and sell.", 0, -1, 1, TI_MARKET },
    { "Foreign trader", "Pavilions of traders from distant lands: goods to buy and sell.", 0, -1, -1, TI_FOREIGN },
    { "Swordsmith", "Swordsmith's Lane: weapons, and the swordsmiths' guild.", 1, -1, 2, TI_SWORD },
    { "Armourer", "Armorer's Way: armour and shields, and the armourers' guild.", 1, -1, 3, TI_ARMOUR },
    { "Bowyer", "The Square of Bowyers and Gunsmiths: bows, crossbows and guns, and their guild.", 1, -1, -2, TI_BOW },
    { "Blacksmith", "The Street of the Blacksmiths: weapons, and the blacksmiths' guild.", 1, -1, 0, TI_SMITH },
    { "Soldiers' Road", "Shops and push-carts crowded with soldiers and camp followers, and the street's leaders.", 1, -1, -1, TI_SOLDIERS },
    { "Jeweller", "", 1, -1, 7, TI_FOREIGN },
    { "Artificer", "Tinkers' Square: hammers, maces, flails and polearms, and the artisans' guild.", 1, -1, 6, TI_TOOLS },
    { "Clothmaker", "Clothmakers' Street: shopfronts, and the clothmakers' guild.", 1, -1, 8, TI_CLOTH },
    { "Alchemist", "Astrologists' Lane: formulas to buy or trade, a better philosopher's stone, lessons in Alchemy.", 3, -1, -1, TI_ALCHEMIST },
    { "Herbalist", "", 3, -1, -1, TI_PHARMACY },
    { "Pharmacist", "Stalls with herbs and rare materials: alchemical components.", 0, -1, -1, TI_PHARMACY },
    { "Physician", "Wounds treated for a fee, lessons in Healing, alchemical components bought and sold.", 3, -1, -1, TI_PHYSICIAN },
    { "Fugger bank", "Heavy coin changed into a letter of credit that weighs nothing, and back, for a fee.", 2, -1, -1, TI_FUGGER },
    { "Medici bank", "The Medici representative: letters of credit, as at the Fugger bank.", 2, -1, -1, TI_MEDICI },
    { "Hanseatic League", "The offices of the league of northern merchant towns.", 2, -1, -1, TI_HANSE },
    { "Pawnshop", "The Fuggers' Leihhaus: a wide choice of everyday goods, and money for almost anything you sell.", 0, 9, -1, TI_PAWN },
    { "Cathedral", "Donations for divine favour, a relic from the canon, the patron saint's story, sanctuary.", 3, 4, -1, TI_CATHEDRAL },
    { "Church", "Mass, confession, alms for divine favour, sanctuary.", 3, -1, -1, TI_CHURCH },
    { "Monastery", "The monks' library (Religion, Latin) and prayers for divine favour.", 3, 7, -1, TI_MONASTERY },
    { "University", "New saints and alchemical formulas to learn, rare alchemical materials, a better philosopher's stone. The staff expect Latin and a sharp mind.", 3, 8, -1, TI_UNIVERSITY },
    { "Hospice", "", 3, -1, -1, TI_PHYSICIAN },
    { "Inn", "Meals and a bed, news and rumours, the stables, a store room; stay to study or train.", 4, -1, -1, TI_INN },
    { "Town square", "Notices, gossip and news.", 4, 1, -1, TI_SQUARE },
    { "Town hall", "The city council: an audience with the lord, prisoners in the dungeons.", 4, 1, -1, TI_TOWNHALL },
    { "Prison", "The city's prison.", 4, 1, -1, TI_PRISON },
    { "Barracks", "The city guard's headquarters: people to join the party, weapons training.", 4, 1, -1, TI_BARRACKS },
    { "Fortress", "The lord's fortress: an audience with the lord, prisoners in the dungeons.", 4, 2, -1, TI_FORTRESS },
    { "Docks", "Boats to other towns, for a fee (each leaves on its own day).", 4, 11, -1, TI_DOCKS },
    { "Slums", "A dangerous slum: rumours, and a room for next to nothing.", 4, 6, -1, TI_SLUMS },
    { "Grove", "A quiet corner where the party can wait: an hour, three hours, or until nightfall.", 4, -1, -1, TI_GROVE },
    { "City wall", "An alley to a stretch of the wall: a way out when the gates will not do.", 4, -1, -1, TI_WALL },
    { "City gate", "Leave the city by one of its gates.", 4, -1, -1, TI_GATE },
};
static const char *GROUPS[5] = { "Market", "Crafts", "Banks and trading houses", "Church, learning and healing", "Around town" };
/* the order within the list (the sections' own) */
static const int ORDER[] = {
    D_MERCHANT, D_FOREIGN, D_PHARMACIST, D_PAWN,
    D_SWORD, D_ARMOR, D_BOW, D_BLACKSMITH, D_TINKER, D_CLOTH, D_SOLDIERS, D_JEWEL,
    D_FUGGER, D_MEDICI, D_HANSE,
    D_CATHEDRAL, D_CHURCH, D_MONASTERY, D_UNIVERSITY, D_PHYSICIAN, D_ALCHEMIST, D_HERBAL, D_HOSPITAL,
    D_INN, D_SQUARE, D_TOWNHALL, D_FORTRESS, D_PRISON, D_BARRACKS, D_DOCKS, D_SLUM, D_GROVE, D_WALL, D_LEAVE,
};

/* what an option of a hub card leads to: a place (D_*) or another hub (100 + H_*) */
struct Rule { int hub; const char *key; int to; };
static const Rule RULES[] = {
    { H_MAIN, "$citysquare", 100 + H_SQUARE }, { H_MAIN, "$fortress", D_FORTRESS }, { H_MAIN, "$marketplace", 100 + H_MARKET },
    { H_MAIN, "spires", 100 + H_CHURCHES }, { H_MAIN, "craft", 100 + H_CRAFTS }, { H_MAIN, "$inn", D_INN }, { H_MAIN, "docks", D_DOCKS },
    { H_MAIN, "side street", 100 + H_SIDE }, { H_MAIN, "grove", D_GROVE }, { H_MAIN, "gate", D_LEAVE },
    { H_MARKET, "everyday", D_MERCHANT }, { H_MARKET, "foreign", D_FOREIGN }, { H_MARKET, "pharmacist", D_PHARMACIST },
    { H_MARKET, "fugger bank", D_FUGGER }, { H_MARKET, "medici", D_MEDICI }, { H_MARKET, "hanseatic", D_HANSE },
    { H_MARKET, "leihhaus", D_PAWN }, { H_MARKET, "main street", 100 + H_MAIN }, { H_MARKET, "side street", 100 + H_SIDE },
    { H_CHURCHES, "$cathedral", D_CATHEDRAL }, { H_CHURCHES, "$citychurch", D_CHURCH }, { H_CHURCHES, "$hospital", D_HOSPITAL },
    { H_CHURCHES, "$monastery", D_MONASTERY }, { H_CHURCHES, "$university", D_UNIVERSITY }, { H_CHURCHES, "$citysquare", 100 + H_SQUARE },
    { H_CHURCHES, "$fortress", D_FORTRESS }, { H_CHURCHES, "main street", 100 + H_MAIN }, { H_CHURCHES, "side street", 100 + H_SIDE },
    { H_CRAFTS, "arms and armor", 100 + H_ARMS }, { H_CRAFTS, "less violent", 100 + H_CIVIL }, { H_CRAFTS, "$inn", D_INN },
    { H_CRAFTS, "physician", D_PHYSICIAN }, { H_CRAFTS, "grove", D_GROVE }, { H_CRAFTS, "$slum", D_SLUM },
    { H_CRAFTS, "main street", 100 + H_MAIN }, { H_CRAFTS, "side street", 100 + H_SIDE }, { H_CRAFTS, "city wall", D_WALL },
    { H_CRAFTS, "gate", D_LEAVE },
    { H_ARMS, "soldier", D_SOLDIERS }, { H_ARMS, "blacksmith", D_BLACKSMITH }, { H_ARMS, "swordsmith", D_SWORD },
    { H_ARMS, "armorer", D_ARMOR }, { H_ARMS, "bowyer", D_BOW }, { H_ARMS, "other crafts", 100 + H_CIVIL },
    { H_ARMS, "side street", 100 + H_SIDE }, { H_ARMS, "main street", 100 + H_MAIN },
    { H_CIVIL, "physician", D_PHYSICIAN }, { H_CIVIL, "astrologist", D_ALCHEMIST }, { H_CIVIL, "herbalist", D_HERBAL },
    { H_CIVIL, "jeweler", D_JEWEL }, { H_CIVIL, "tinker", D_TINKER }, { H_CIVIL, "clothmaker", D_CLOTH },
    { H_CIVIL, "-making", 100 + H_ARMS }, { H_CIVIL, "side street", 100 + H_SIDE }, { H_CIVIL, "main street", 100 + H_MAIN },
    { H_SIDE, "main street", 100 + H_MAIN }, { H_SIDE, "$inn", D_INN }, { H_SIDE, "$cityslums", D_SLUM },
    { H_SIDE, "grove", D_GROVE }, { H_SIDE, "city wall", D_WALL },
    { H_SQUARE, "$councilhall", D_TOWNHALL }, { H_SQUARE, "prison", D_PRISON }, { H_SQUARE, "$citybarracks", D_BARRACKS },
    { H_SQUARE, "$university", D_UNIVERSITY }, { H_SQUARE, "spires", 100 + H_CHURCHES }, { H_SQUARE, "$marketplace", 100 + H_MARKET },
    { H_SQUARE, "main street", 100 + H_MAIN }, { H_SQUARE, "side street", 100 + H_SIDE },
};
/* a hub that is itself a place to go to (the square: its notices and gossip) */
static const int HUB_PLACE[H_N] = { -1, -1, -1, -1, -1, -1, -1, D_SQUARE };

static int hub_of(const std::string &card)
{
    for (int h = 0; h < H_N; h++) if (!strncmp(card.c_str(), HUB_CARD[h], 6)) return h;
    return H_NONE;
}
static int rule_for(int hub, const std::string &text)
{
    for (const Rule &r : RULES) if (r.hub == hub && text.find(r.key) != std::string::npos) return r.to;
    return -1;
}

/* steps from each hub to a place, through the hubs (side streets cost more: more encounters) */
static int dist_to(int dest, int from, unsigned avoid)
{
    int d[H_N]; for (int h = 0; h < H_N; h++) d[h] = HUB_PLACE[h] == dest ? 0 : 1000;
    for (const Rule &r : RULES) if (r.to == dest && !(avoid >> r.hub & 1)) d[r.hub] = 1;
    for (int it = 0; it < H_N; it++)
        for (const Rule &r : RULES)
            if (r.to >= 100 && !(avoid >> r.hub & 1)) {
                int c = d[r.to - 100] + (r.to - 100 == H_SIDE ? 3 : 1);
                if (c < d[r.hub]) d[r.hub] = c;
            }
    return d[from];
}

/* ---------------------------------------------------------------- walking */
static int goal = -1, steps_taken;
static unsigned visited;                                   /* hubs passed on this walk: not again */
static int conts, leaves;                                  /* texts read on, places left before the first hub */
static std::string last_key; static int same_count;
static u32 phase_t, last_act;
static std::string phase_key;

static int logon = -1;
#define LOG(...) do { if (logon < 0) logon = getenv("DL_CITYNAV_LOG") != 0; if (logon) fprintf(stderr, "citynav: " __VA_ARGS__); } while (0)

static void stop(const char *why)
{
    LOG("stop: %s\n", why ? why : "");
    if (goal >= 0 && why) { char m[160]; snprintf(m, sizeof m, "Go to %s: %s", DEST[goal].label, why); ui_notice(m); }
    goal = -1;
}

extern "C" void citynav_user_input(void) { if (goal >= 0) stop("stopped."); }
extern "C" int citynav_busy(void) { return goal >= 0; }
extern "C" { extern int hdtext_fast_fades; }

static bool in_city(void)
{
    int cur = (s16)g_rw(G_CUR_LOC);
    return cur >= 0 && cur < 0x5c && game_mode() == GM_CARD;
}
/* before the walls: the city's approach, its gates and its walls ($CityE, $CityG00..04, $CityW),
   where no street leads anywhere yet (the groves, $CityG05/06, are inside) */
static bool outside_walls(void)
{
    std::string c = card_now();
    if (c.compare(0, 6, "$CITYE") == 0 || c.compare(0, 6, "$CITYW") == 0) return true;
    return c.compare(0, 6, "$CITYG") == 0 && c.size() >= 8 && c[6] == '0' && c[7] >= '0' && c[7] <= '4';
}
extern "C" int citynav_available(void) { return in_city() && !outside_walls(); }

static bool leave_option(const std::string &t)
{
    if (t.find("main street") == std::string::npos) return false;
    static const char *verbs[] = { "leave", "return", "go out", "move away", "stroll away", "back down", "way to", "exit" };
    for (const char *v : verbs) if (t.find(v) != std::string::npos) return true;
    return false;
}

static int choose_leave(const std::vector<std::string> &line)
{
    for (size_t i = 0; i < line.size(); i++) if (leave_option(line[i])) return (int)i;
    for (size_t i = 0; i < line.size(); i++) if (line[i].find("return to") != std::string::npos && line[i].find("side") == std::string::npos) return (int)i;
    for (size_t i = 0; i < line.size(); i++)
        if ((line[i].find("leave") != std::string::npos || line[i].find("depart") != std::string::npos) && line[i].find("side") == std::string::npos
            && line[i].find("alley") == std::string::npos) return (int)i;
    for (size_t i = 0; i < line.size(); i++)                 /* the only way out may be a side street (the slums) or a step back */
        if (line[i].find("leave") != std::string::npos || line[i].find("fall back") != std::string::npos || line[i].find("reconsider") != std::string::npos) return (int)i;
    return -1;
}

static void click_line(int i)
{
    int o = (s8)g_rb((u16)(GV_card_layout_opt + i));
    LOG("choose line %d (option %d)\n", i, o);
    if (cardview_active() && o >= 0) cardview_choose(o);   /* the cards drawn by Darklands+: by the option's number (a click by
                                                               position could land on the next row: Alchemist led to the clothmakers) */
    else {
        int x0 = 60 + (s16)g_rw(GV_card_layout_x), y0 = g_rb((u16)(GV_card_layout_y + i)), h = g_rb((u16)(GV_card_layout_h + i));
        plat_click_game(x0 + 10, y0 + (h > 8 ? 4 : h / 2));
    }
    last_act = plat_ticks_ms(); steps_taken++;
}

extern "C" void citynav_tick(void)
{
    hdtext_fast_fades = goal >= 0;                         /* the cards' fades at once while walking */
    if (goal < 0) return;
    { static int gen = -1; int datamods_generation(void); if (gen != datamods_generation()) { gen = datamods_generation(); cards.clear(); cards_loaded = false; } }
    if (!cards_loaded) load_cards();
    u32 now = plat_ticks_ms();
    if (now - last_act < 250) return;                      /* let the game take the last click */
    if (!in_city()) { stop(game_mode() == GM_CARD ? "left the city." : "interrupted."); return; }
    if (steps_taken > 20) { stop("too many steps; stopped."); return; }
    std::string card = card_now();
    int blk = (s16)g_rw(GV_options_block);
    char kb[48]; snprintf(kb, sizeof kb, "%s/%d/%d", card.c_str(), blk, assist_in_card_menu());
    if (phase_key != kb) { phase_key = kb; phase_t = now; }
    int hub = hub_of(card);
    if (!assist_in_card_menu()) {                        /* the card's text: read on (hubs, and the place the walk starts from) */
        if (now - phase_t < 300) return;
        if (hub == H_NONE) {
            if (visited || conts >= 16) { stop(steps_taken ? "something happened on the way." : "leave this place first."); return; }   /* (the Quickstart's inn card has six pages; a slow game lets clicks pass unread) */
            conts++;
        }
        LOG("%s: read on\n", kb);
        plat_click_game(304, 12); last_act = now;
        return;
    }
    if (now - phase_t < 120) return;                       /* the option list fading in (at once while walking) */
    if (kb == last_key) { if (++same_count > 3) { stop("the way is blocked."); return; } } else { last_key = kb; same_count = 0; }
    auto it = cards.find(card);
    if (it == cards.end() || blk < 0 || blk >= (int)it->second.size()) { stop("unknown card."); return; }
    std::vector<std::string> opts = block_options(it->second[blk]);
    int n = (s8)g_rb(GV_card_layout);
    if (n <= 0 || n > 20) return;
    if (g_rb(GV_card_box_shown)) return;                              /* a skill-check box is up */
    std::vector<std::string> line(n);
    for (int i = 0; i < n; i++) { int o = (s8)g_rb((u16)(GV_card_layout_opt + i)); line[i] = o >= 0 && o < (int)opts.size() ? opts[o] : ""; }
    if (logon) { LOG("%s hub %d visited %x:\n", kb, hub, visited); for (int i = 0; i < n; i++) LOG("  %d: %s -> %d\n", i, line[i].c_str(), hub >= 0 ? rule_for(hub, line[i]) : -1); }
    if (hub == H_NONE) {                                   /* a shop or other place of the city: out the way the player would leave */
        if (visited || leaves >= 3) { stop("something happened on the way."); return; }
        int pick = choose_leave(line);
        if (pick < 0) { stop(leaves ? "something happened on the way." : "leave this place first."); return; }
        leaves++; click_line(pick);
        return;
    }
    if (HUB_PLACE[hub] == goal) { LOG("arrived\n"); goal = -1; return; }   /* the square itself */
    for (int i = 0; i < n; i++) if (rule_for(hub, line[i]) == goal) { click_line(i); goal = -1; return; }   /* there */
    visited |= 1u << hub;
    int best = -1, bd = 1000;
    for (int i = 0; i < n; i++) {
        int to = rule_for(hub, line[i]);
        if (to < 100 || (visited >> (to - 100) & 1)) continue;
        int d = dist_to(goal, to - 100, visited) + (to - 100 == H_SIDE ? 3 : 1);
        if (d < bd) { bd = d; best = i; }
    }
    if (best < 0 || bd >= 1000) { stop("not found in this city."); return; }
    click_line(best);
}

extern "C" void citynav_go(int dest)
{
    if (dest < 0 || dest >= D_N) return;
    assist_watch_cards();
    goal = dest; steps_taken = 0; visited = 0; conts = leaves = 0; same_count = 0; last_key.clear(); phase_key.clear(); last_act = 0;
    char m[120]; snprintf(m, sizeof m, "Going to: %s. Press any key or click to stop.", DEST[dest].label); ui_notice(m);
}

/* ---------------------------------------------------------------- the list */
/* Whether the city's cards offer a place, as the card scripts decide (s045..s050):
 * - a building bit of DARKLAND.CTY or a shop quality;
 * - the Fugger, Medici and Hanse offices: bytes +0x15..+0x17 of the location record;
 * - Soldier's Road: byte +0x14 of the location record is 1 or 2 (rarely so);
 * - the physician and Astrologists' Lane: in small cities only some games have them
 *   (from the city's seed, the game's seed and, for the physician, the year);
 * - the hospital and Jewelers' Avenue are never offered (their options stay switched off),
 *   nor the herbalists and barber-surgeons ($CivCr01 and $Barbe00 are unused cards);
 * - Clothmakers' Street is offered, but its card ($Cloth00) has nothing on it but "leave" (its
 *   other options stay switched off): not listed, a walk there would be for nothing. */
static bool never(int d) { return d == D_HOSPITAL || d == D_JEWEL || d == D_HERBAL || d == D_CLOTH; }   /* (the herbalists' road is only on $CivCr01, a card the game never loads) */
static bool present_at(int d, int cur)
{
    if (never(d)) return false;
    if (cur < 0 || cur >= (int)gamedata.cities.size()) return true;
    const City &c = gamedata.cities[cur];
    if (DEST[d].building >= 0 && !(c.buildings >> DEST[d].building & 1)) return false;
    if (DEST[d].shop >= 0 && !c.shopq[DEST[d].shop]) return false;
    if (DEST[d].shop == -2 && !c.shopq[4] && !c.shopq[5]) return false;
    u32 lr = g_location(cur);
    if (lr) {
        u16 ls = (u16)(lr >> 16), lo = (u16)lr;
        if (d == D_SOLDIERS) { int v = (s8)rb(ls, (u16)(lo + 0x14)); if (v != 1 && v != 2) return false; }
        if (d == D_FUGGER && !rb(ls, (u16)(lo + 0x15))) return false;
        if (d == D_MEDICI && !rb(ls, (u16)(lo + 0x16))) return false;
        if (d == D_HANSE && !rb(ls, (u16)(lo + 0x17))) return false;
    }
    s16 q = (s16)(c.seed + (s16)g_rw(G_PARTY_SEED));             /* loc_query 0x21 (16-bit arithmetic, as the game) */
    if (d == D_PHYSICIAN && c.size <= 3 && !((s16)(q + (s16)g_rw(G_CUR_YEAR)) % 3)) return false;
    if (d == D_ALCHEMIST && ((c.size <= 3 && !(q % 2)) || (c.size == 4 && !(q % 3)))) return false;
    return true;
}
static bool present(int d) { return present_at(d, (s16)g_rw(G_CUR_LOC)); }

/* what town loc offers that is worth knowing (ui/townicons.h TI_UNIVERSITY .. TI_BANK), for the
   margin: as the city's cards decide (the location record is read: while a card or the map is up) */
bool citynav_town_offers(int loc, int icon)
{
    if (loc < 0 || loc >= (int)gamedata.cities.size()) return false;
    switch (icon) {
    case TI_UNIVERSITY: return present_at(D_UNIVERSITY, loc);
    case TI_CATHEDRAL: return present_at(D_CATHEDRAL, loc);
    case TI_ALCHEMIST: return present_at(D_ALCHEMIST, loc);
    case TI_PHYSICIAN: return present_at(D_PHYSICIAN, loc);
    case TI_DOCKS: return present_at(D_DOCKS, loc);
    case TI_BANK: return present_at(D_FUGGER, loc) || present_at(D_MEDICI, loc);
    }
    return false;
}

/* the party's tasks in this town, by the place they lead to: a reward waiting with its giver (the
   giver's house: a bank's, the Hanse's, the cathedral, the council), a thing kept in an office above
   the marketplace (reached at night) */
struct DestTask { int cat; std::string text; };
static std::map<int, std::vector<DestTask>> dest_tasks(int town)
{
    std::map<int, std::vector<DestTask>> m;
    std::vector<QuestRow> rows; quest_rows(rows, QSORT_DEADLINE);
    for (const QuestRow &r : rows) {
        if (r.loc != town) continue;
        int d = -1;
        switch (r.giver) {     /* (where the giver's card looks for the party's return, ev24_at_owner: $Medic00 6, $Hanse00 7, $Fugge00 8,
                                  $Cathe01 3, $Counc00 10; a merchant's, a foreign trader's, a pharmacist's at the marketplace itself) */
        case 6: d = D_MEDICI; break; case 7: d = D_HANSE; break; case 8: d = D_FUGGER; break; case 3: d = D_CATHEDRAL; break; case 10: d = D_TOWNHALL; break;
        }
        if (r.office >= 0) { static const int OD[4] = { D_MERCHANT, D_MEDICI, D_FUGGER, D_HANSE }; d = OD[r.office & 3]; }
        if (d >= 0) m[d].push_back({ r.cat, r.text });
    }
    return m;
}

extern "C" const char *journal_town_fact(int loc, const char *key);   /* ui/journal.cpp */
extern ImFont *ui_font_bold;
static const char *MON3[12] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

/* text cut to fit `w` pixels at font size `fs`, with "..." when cut */
static std::string fit(const std::string &t, float fs, float w)
{
    ImFont *f = ImGui::GetFont();
    if (f->CalcTextSizeA(fs, FLT_MAX, 0, t.c_str()).x <= w) return t;
    std::string r = t;
    while (!r.empty() && f->CalcTextSizeA(fs, FLT_MAX, 0, (r + "...").c_str()).x > w) r.pop_back();
    while (!r.empty() && r.back() == ' ') r.pop_back();
    return r + "...";
}

/* a place's tooltip: its name, what it is, then what is known of it now, a heading to each part */
struct TipPart { std::string head, text; };
static void place_tooltip(int d, const std::vector<TipPart> &parts)
{
    ImGuiIO &io = ImGui::GetIO();
    float w = fminf(io.DisplaySize.x * 0.36f, UIS(560));
    ImGui::SetNextWindowSizeConstraints(ImVec2(0, 0), ImVec2(w + UIS(24), FLT_MAX));
    ImGui::BeginTooltip();
    ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + w);
    if (ui_font_bold) ImGui::PushFont(ui_font_bold);
    ImGui::TextUnformatted(DEST[d].label);
    if (ui_font_bold) ImGui::PopFont();
    ImGui::TextUnformatted(DEST[d].desc);
    for (const TipPart &x : parts) {
        ImGui::Spacing();
        if (!x.head.empty()) ImGui::TextColored(ImVec4(0.55f, 0.24f, 0.13f, 1), "%s", x.head.c_str());
        ImGui::TextUnformatted(x.text.c_str());
    }
    ImGui::PopTextWrapPos();
    ImGui::EndTooltip();
}

/* the "Go to" window; returns false when it closes.
 * Each place is a tile: its picture, its name, and under it the one thing worth knowing now (a
 * task giver's chance of a task or until when none, a reward waiting; a shop's wares; what the
 * party learned of the physician or the alchemist). At the right: a lectern where a teacher is to
 * be had, a task's picture where one of the party's tasks leads. The rest is under the pointer.
 * The places the city lacks are named in one line at the foot. */
extern "C" int citynav_window(void)
{
    ImGuiIO &io = ImGui::GetIO();
    ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(0, 0), io.DisplaySize, IM_COL32(0, 0, 0, 90));
    float w = fminf(io.DisplaySize.x * 0.94f, UIS(1240)), maxh = io.DisplaySize.y * 0.94f;
    ImGui::SetNextWindowSizeConstraints(ImVec2(w, 0), ImVec2(w, maxh));
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    bool keep = true;
    int cur = (s16)g_rw(G_CUR_LOC);
    bool city = cur >= 0 && cur < (int)gamedata.cities.size();
    std::string title = city ? "Go to: " + gamedata.cities[cur].name : std::string("Go to");
    title += "###citynav";
    int picked = -1;
    float scale = fminf(1.0f, fmaxf(0.7f, io.DisplaySize.y / 900.0f));
    static bool tall;                                        /* (the list did not fit last frame: a scroll bar) */
    if (ImGui::Begin(title.c_str(), &keep, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings
                     | (tall ? 0 : ImGuiWindowFlags_NoScrollbar))) {
        tall = ImGui::GetWindowHeight() >= maxh - 2;
        ImGui::SetWindowFontScale(scale);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8 * scale, 6 * scale));
        ImGui::PushTextWrapPos(0);
        ImGui::TextDisabled("Pick a place and the party walks there through the streets. Press any key or click to stop them.");
        { int hour = (s16)g_rw(GV_cur_hour);    /* (the game's night: is_daylight, 5:00 to 18:59) */
          if (hour >= 19 || hour < 5) ImGui::TextColored(ImVec4(0.45f, 0.18f, 0.10f, 1), "Night (%d:00): shops and offices are closed, and the watch is out.", hour); }
        ImGui::PopTextWrapPos();
        static std::map<int, std::vector<DestTask>> tasks; static int tasks_frame = -100;
        static TchSource teach[12]; static int nteach;            /* the teachers the town offers (ui/teachersui.h) */
        if (ImGui::GetFrameCount() - tasks_frame > 30) {
            tasks = dest_tasks(cur); tasks_frame = ImGui::GetFrameCount();   /* (read twice a second) */
            nteach = ui_teachers_on() ? ui_town_teachers(cur, teach, 12) : 0;
        }
        bool work_on = config_get_int("assist.goto_work", 1) && !config_get_int("original", 0);
        float lh = ImGui::GetFontSize(), sfs = lh * 0.8f, th = floorf(lh + sfs + lh * 0.55f), rnd = 6 * scale;
        float gap = ImGui::GetStyle().ItemSpacing.x, avail = ImGui::GetContentRegionAvail().x;
        float need = ImGui::CalcTextSize("Hanseatic League").x + th * (0.8f + 0.62f + 0.56f) + th * 0.5f;   /* (the longest name, the picture, both marks) */
        int cols = (int)((avail + gap) / (need + gap));
        cols = cols < 3 ? 3 : cols > 6 ? 6 : cols;
        float bw = floorf((avail - (cols - 1) * gap) / cols);
        bool any_teach = false, any_task = false;
        std::string absent;
        for (int g = 0; g < 5; g++) {
            int col = 0;
            for (int d : ORDER) {
                if (DEST[d].group != g || never(d)) continue;
                if (!present(d)) { absent += (absent.empty() ? "" : ", ") + std::string(DEST[d].label); continue; }
                if (!col) ImGui::SeparatorText(GROUPS[g]);
                else if (col % cols) ImGui::SameLine();
                col++;
                ImGui::PushID(d);
                ImVec2 p = ImGui::GetCursorScreenPos();
                if (ImGui::InvisibleButton("b", ImVec2(bw, th))) picked = d;
                bool hov = ImGui::IsItemHovered();
                ImDrawList *dl = ImGui::GetWindowDrawList();
                dl->AddRectFilled(p, ImVec2(p.x + bw, p.y + th), hov ? IM_COL32(140, 61, 33, 255) : IM_COL32(224, 204, 166, 255), rnd);
                if (!hov) dl->AddRect(p, ImVec2(p.x + bw, p.y + th), IM_COL32(178, 148, 106, 255), rnd);
                std::vector<TipPart> tip;
                std::string sub; ImU32 subcol = IM_COL32(104, 82, 58, 255);
                /* a task giver: whether it has a task now, or until when none (mods/offices.h) */
                {
                    int giver = d == D_MERCHANT ? 0 : d == D_FOREIGN ? 4 : d == D_PHARMACIST ? 5 : d == D_MEDICI ? 6 : d == D_HANSE ? 7 : d == D_FUGGER ? 8
                              : d == D_CATHEDRAL ? 3 : d == D_TOWNHALL || d == D_FORTRESS ? 10 : -1;
                    if (d == D_FORTRESS && city && (gamedata.cities[cur].buildings & 2)) giver = -1;   /* (a town with a council: the lord gives none at the fortress) */
                    OfficeStatus os;
                    if (giver >= 0 && work_on && office_status(cur, giver, d == D_TOWNHALL, &os) && os.state != OFS_NONE) {
                        char b[48];
                        switch (os.state) {
                        case OFS_ASK: snprintf(b, sizeof b, "%d%% chance of a task", os.chance); subcol = IM_COL32(38, 100, 34, 255); break;
                        case OFS_SHUT: snprintf(b, sizeof b, "No tasks until %d %s", os.day, MON3[(os.month % 12 + 12) % 12]); break;
                        case OFS_ON_TASK: snprintf(b, sizeof b, "On a task for them"); subcol = IM_COL32(40, 70, 140, 255); break;
                        case OFS_REWARD: snprintf(b, sizeof b, "Reward waiting"); subcol = IM_COL32(150, 96, 0, 255); break;
                        default: snprintf(b, sizeof b, "Reputation too low"); subcol = IM_COL32(150, 40, 25, 255); break;
                        }
                        sub = b;
                        std::string n = os.note;
                        if (os.state == OFS_SHUT) { char u[64]; snprintf(u, sizeof u, " Until %d %s, %d:00.", os.day, MON3[(os.month % 12 + 12) % 12], os.hour); n += u; }
                        tip.push_back({ "Tasks", n });
                    } else if (giver >= 0 && os.note[0] && work_on) tip.push_back({ "Tasks", os.note });
                }
                /* a shop: the quality of its wares */
                if (city && DEST[d].shop != -1) {
                    const City &c = gamedata.cities[cur];
                    int r = DEST[d].shop >= 0 ? c.shopq[DEST[d].shop] : c.shopq[5] ? c.shopq[5] : c.shopq[4];
                    if (r > 0) {
                        int q = shop_wares_q(r);
                        char b[96]; snprintf(b, sizeof b, "Wares %dq", q);
                        if (sub.empty()) { sub = b; if (q >= 30) subcol = IM_COL32(38, 100, 34, 255); }
                        snprintf(b, sizeof b, "Wares of %dq (most shops sell about 25q).", q);
                        tip.insert(tip.begin(), { "", b });
                    }
                }
                /* what the party learned of the physician or the alchemist here */
                if (d == D_PHYSICIAN || d == D_ALCHEMIST) {
                    const char *fact = journal_town_fact(cur, d == D_PHYSICIAN ? "physician" : "alchemist");
                    if (fact) {
                        std::string f = fact, s;
                        if (d == D_PHYSICIAN) { s = f; size_t cl = s.find(':'); if (cl != std::string::npos) s.erase(cl); }
                        else { size_t q = f.rfind(" to "); s = q != std::string::npos ? "Stone up to" + f.substr(q + 3) : "No better stone than yours"; }
                        if (!s.empty()) s[0] = (char)toupper((unsigned char)s[0]);
                        if (sub.empty()) sub = s;
                        tip.push_back({ "What you learned", std::string(d == D_PHYSICIAN ? "The physician: " : "The alchemist: ") + f + "." });
                    }
                }
                /* the party's tasks that lead here; a teacher to be had here */
                auto tk = tasks.find(d);
                bool task = tk != tasks.end() && !tk->second.empty();
                if (task) { any_task = true; for (const DestTask &x : tk->second) tip.push_back({ "Your task", x.text }); }
                int src = d == D_SWORD ? TCH_SWORDSMITHS : d == D_ARMOR ? TCH_ARMORERS : d == D_BOW ? TCH_BOWYERS : d == D_BLACKSMITH ? TCH_BLACKSMITHS
                        : d == D_TINKER ? TCH_ARTIFICERS : d == D_ALCHEMIST ? TCH_ALCHEMIST : d == D_PHYSICIAN ? TCH_PHYSICIAN
                        : d == D_MONASTERY ? TCH_MONASTERY : d == D_UNIVERSITY ? TCH_UNIVERSITY : -1;
                int ti = -1;
                for (int t = 0; src >= 0 && t < nteach; t++) if (teach[t].src == src) ti = t;
                if (ti >= 0) { any_teach = true; tip.push_back({ "Teacher", ui_teacher_text(teach[ti]) }); }
                /* the picture, the badges at the right, the name and the line under it */
                float is = th * 0.8f, pad = (th - is) * 0.5f;
                ui_draw_town_icon(dl, ImVec2(p.x + pad + is * 0.5f, p.y + th * 0.5f), is, DEST[d].icon);
                float tx = p.x + pad * 2 + is + 2 * scale, rx = p.x + bw - pad;
                if (task) {
                    float qs = th * 0.62f;
                    ui_draw_quest_icon(dl, ImVec2(rx - qs * 0.5f, p.y + th * 0.5f), qs, tk->second[0].cat);
                    rx -= qs + 3 * scale;
                }
                if (ti >= 0) {
                    float ls = th * 0.56f; int a = teach[ti].off ? 110 : 255;
                    ui_draw_teach_mark(dl, ImVec2(rx - ls * 0.5f, p.y + th * 0.5f), ls,
                                       hov ? IM_COL32(252, 242, 220, a) : IM_COL32(32, 62, 120, a), hov ? IM_COL32(140, 61, 33, a) : IM_COL32(252, 246, 228, a));
                    rx -= ls + 3 * scale;
                }
                float room = rx - tx - 2 * scale;
                std::string lab = fit(DEST[d].label, lh, room);
                ImU32 fg = hov ? IM_COL32(252, 242, 220, 255) : IM_COL32(51, 31, 15, 255);
                if (sub.empty()) dl->AddText(ImVec2(tx, p.y + (th - lh) * 0.5f), fg, lab.c_str());
                else {
                    float y0 = p.y + (th - lh - sfs) * 0.5f;
                    dl->AddText(ImVec2(tx, y0), fg, lab.c_str());
                    std::string s2 = fit(sub, sfs, room);
                    dl->AddText(ImGui::GetFont(), sfs, ImVec2(tx, y0 + lh), hov ? IM_COL32(240, 214, 180, 255) : subcol, s2.c_str());
                }
                if (hov) place_tooltip(d, tip);
                ImGui::PopID();
            }
        }
        /* the foot: what the marks mean (those shown), and what this city lacks */
        if (any_teach || any_task || !absent.empty()) { ImGui::Spacing(); ImGui::Separator(); }
        if (any_teach || any_task) {
            ImDrawList *dl = ImGui::GetWindowDrawList();
            float ms = lh * 1.05f;
            auto mark = [&](int kind, const char *text) {
                ImVec2 a = ImGui::GetCursorScreenPos();
                ImGui::Dummy(ImVec2(ms, ms));
                if (kind) ui_draw_quest_icon(dl, ImVec2(a.x + ms * 0.5f, a.y + ms * 0.5f), ms, 0);
                else ui_draw_teach_mark(dl, ImVec2(a.x + ms * 0.5f, a.y + ms * 0.5f), ms * 0.9f, IM_COL32(32, 62, 120, 255), IM_COL32(252, 246, 228, 255));
                ImGui::SameLine(0, 4 * scale);
                ImGui::AlignTextToFramePadding();
                ImGui::TextDisabled("%s", text);
            };
            bool first = true;
            if (any_teach) { mark(0, "A teacher to hire (faint: not now)"); first = false; }
            if (any_task) { if (!first) ImGui::SameLine(0, 24 * scale); mark(1, "One of your tasks leads here"); }
        }
        if (!absent.empty()) {
            ImGui::PushTextWrapPos(0);
            ImGui::TextDisabled("Not in %s: %s.", city ? gamedata.cities[cur].name.c_str() : "this city", absent.c_str());
            ImGui::PopTextWrapPos();
        }
        ImGui::PopStyleVar();
    }
    ImGui::End();
    if (picked >= 0) { citynav_go(picked); return 0; }
    return keep;
}
