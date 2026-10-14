/* encounters.c - new encounters written as data.
 *
 * A mod folder can hold ENCOUNTERS/<NAME>.TXT files: each is one new wilderness encounter, a
 * card like the game's own (a picture, text blocks from MSGFILES/<CARD>.TXT, a menu of options
 * with chances of success), run by this interpreter instead of compiled card code. While such a
 * mod is ticked in F12 > Mods, some of the random encounters of the wilderness are new ones,
 * picked as if they were extra entries of the game's own table for the terrain (the format and
 * the language are described in docs/MODDING.md, "New encounters").
 *
 * How it plugs into the game:
 *  - The main loop runs the current card through a far table of card functions (F_2dae_0000,
 *    card numbers 0..0x16d, root 03a1). Entries 0x170.. are free: they get a native function
 *    (runtime/native.c) that runs the encounter whose number it is and returns the next card.
 *  - The overland loop (s030 1462:02c0) returns the card of a random encounter; when the step
 *    that led to it was a random encounter (travel_step_result 1000) and the card is one of the
 *    terrain tables', a new encounter may be returned instead.
 *  - The card engine (s043 150b) does the drawing, the text and the menus exactly as for the
 *    game's cards: card_load, card_options, card_menu, card_text. The option handlers it calls
 *    for "Shift: chance of success" (opt_fn[], item_fn[][], saint_fn[][]) are native functions
 *    too; they evaluate the encounter's chance expressions.
 *  - Game rules (skills, saints, battles, items, time) are the game's own functions, called on
 *    the game's stack (gcall); a load or quit from the menu bar inside them (a longjmp) is
 *    passed on: the encounter stops at once. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <stdarg.h>
#include <dirent.h>
#include "../runtime/cpu.h"
#include "../runtime/hooks.h"
#include "../runtime/game.h"
#include "../runtime/config.h"
#include "../runtime/native.h"
#include "../runtime/hle.h"
#include "datamods.h"
#include "travel.h"
#include "encounters.h"

void ui_notice(const char *);

#define ENC_BASE   0x170                 /* card numbers of the new encounters */
#define ENC_MAX    128                   /* 0x170..0x1ef: unused space after the card table */
#define CARD_TABLE ((u16)(LOADSEG + 0x2dae))
#define NOPT       10

/* engine variables (DGROUP, include/darklands/card.h) */
#define V_NEXT_CARD   0xe48a
#define V_CARD_NAME   0xe3d4
#define V_PIC_BUF     0xe3e3
#define V_PIC_BUF2    0xe95f
#define V_CARD_WIN    0x017c
#define V_CARD_WIN2   0x0154
#define V_OPT_BLOCK   0xee53
#define V_MSG_BUF     0x9085
#define V_OPT_ON      0xe99c
#define V_OPT_FN      0xea14
#define V_OPT_FLAG    0xee76
#define V_CARD_ITEMS  0xee43
#define V_CARD_SAINTS 0xee4b
#define V_ITEM_FN     0xec00
#define V_SAINT_FN    0xecf6
#define V_MENU_MEMBER 0xee6c
#define V_MENU_SLOT   0xee6a
#define V_MENU_POPUP  0xee6e
#define V_CHOSEN1     0xee8c
#define V_MONEY1      0xee9c
#define V_NUMBER1     0xed4c
#define V_NUMBER2     0xed50
#define V_ITEM        0xed5c
#define V_EE70        0xee70
#define V_EE72        0xee72
#define V_BEST_MEMBER 0x991d
#define V_CUR_CARD    0xa772
#define V_PARTY_X     0xe3d0
#define V_PARTY_Y     0xe3d2
#define V_TERRAIN     0xd843
#define V_HOUR        0x00e0
#define V_DAY         0x00e2
#define V_MONTH       0x00e4
#define V_YEAR        0x00e6
#define S_NO_RISK     0x4f02
#define S_SEL_POTION  0x4f0c
#define S_SEL_SAINT   0x4f2a
#define S_ASSURED     0x4f48
#define S_POTION_OK   0x4f7a
#define S_UNKNOWN     0x4f9a
#define S_COMBAT      0x4fc0

static int enc_log_on(void) { static int v = -1; if (v < 0) v = getenv("DL_ENC_LOG") != 0; return v; }
#define LOG(...) do { if (enc_log_on()) fprintf(stderr, "encounters: " __VA_ARGS__); } while (0)

/* ================================================================ the encounters */
typedef struct { char *s; int line; } Src;      /* a value from the file, with its line */

typedef struct {
    int defined, kind;                  /* kind 0 option, 1 potion box, 2 saint box */
    Src only, need, who, chance, ok, fail;
    int npot, pot_code[4]; Src pot[4];
} Opt;

typedef struct {
    char name[40];
    int line, block;                    /* the option-list text block; -1: no menu */
    char pic[16];
    Src chosen, enter;
    int saints[4], items[4];
    Opt opt[NOPT];
} Sec;

typedef struct {
    char id[40], title[80], file[1200], card[16], pic[16];
    int weight, when, music, slot;      /* when: 0 any, 1 day, 2 night */
    unsigned terrain;                   /* bit t: map terrain t (D_d843) */
    Src only;
    Sec *sec; int nsec;
    char vars[24][24]; int nvars;
    int ok;
    long last_day;
    int nblocks;                        /* the card text's blocks (0: not found, e.g. one of the game's own cards) */
    unsigned char nopts[64], optcode[64][NOPT];   /* each block's option lines and their codes (0x15 0x16 0x10 0x06) */
} Enc;

static Enc encs[ENC_MAX];
static int nenc, nerr, loaded_gen = -1;
static int by_slot[ENC_MAX];             /* card slot -> encounter, -1 */
static int force_next = -1;

/* the encounter being run */
static struct {
    int active, enc, card, sec, last_menu, jumped;
    int flag[NOPT];                      /* option overrides: -1 none, 0 hidden, 1 on, 2 greyed */
    long var[24];
    int who, q, saint, item, result, cur_opt;
} rt;
static int aborting;                     /* the game unwinds (load, quit): stop */

/* ================================================================ calling the game */
#define ROOT (-1)
#define VEC  999
static u16 ret_dx;
static int gcall(int sect, u16 seg, u16 off, int n, const int *a)
{
    if (aborting) return 0;
    void (*f)(void) = 0;
    if (sect != VEC && !(f = game_func(sect, seg, off))) { fprintf(stderr, "encounters: no game function s%d %04x:%04x\n", sect, seg, off); return 0; }
    u16 sp0 = SP, ds = DS, es = ES;
    DS = ES = G_DGROUP;
    tail_pending = 0;
    for (int i = n - 1; i >= 0; i--) push16((u16)a[i]);
    push16(HLE_SEG); push16(0xfff0);
    OvlSet o; ovl_save(&o);
    if (sect == VEC) dispatch_far((u16)(LOADSEG + 0x09c0), off);
    else { if (sect >= 0) ovl_enter(sect); f(); }
    ovl_restore(&o);
    if (unw_active) { aborting = 1; return 0; }
    SP = sp0; DS = ds; ES = es;
    ret_dx = DX;
    return (s16)AX;
}
static int G(int sect, u16 seg, u16 off, int n, ...)
{
    int a[12]; va_list ap; va_start(ap, n);
    for (int i = 0; i < n && i < 12; i++) a[i] = va_arg(ap, int);
    va_end(ap);
    return gcall(sect, seg, off, n, a);
}
/* root */
static int g_rand(int n) { return G(ROOT, 0x0000, 0x0c70, 1, n); }
static void g_mouse_show(void) { G(ROOT, 0x047c, 0x1293, 0); }
static void g_mouse_hide(void) { G(ROOT, 0x047c, 0x12c3, 0); }
static void g_panel_rows(void) { G(ROOT, 0x0000, 0x1716, 1, 0x100); }
/* card engine, s043 */
static void g_card_load(void) { G(43, 0x150b, 0x000a, 3, V_CARD_NAME, V_PIC_BUF, G_DGROUP); }
static void g_card_load_near(void) { G(43, 0x150b, 0x0110, 2, V_CARD_NAME, V_PIC_BUF2); }
static void g_card_text(int b) { G(43, 0x150b, 0x03b0, 2, b, 0); }
static void g_card_options(int b) { G(43, 0x150b, 0x049a, 1, b); }
static int g_card_menu(void) { return G(43, 0x150b, 0x1142, 2, g_rw(V_CARD_WIN), g_rw(V_CARD_WIN2)); }
static int g_saints_usable(void) { return G(43, 0x150b, 0x168c, 0); }
static int g_items_usable(void) { return G(43, 0x150b, 0x16e8, 0); }
static void g_music(void) { G(43, 0x1462, 0x09ae, 1, 0x20); }
/* time and numbers, s029 1367 */
static void g_hours(int h) { if (h > 0) G(29, 0x1367, 0x05c8, 1, h); }
static void g_days(int d) { if (d > 0) G(29, 0x1367, 0x0680, 1, d); }
static int g_hours_until(int h) { return G(29, 0x1367, 0x086a, 1, h); }
static int g_daylight(void) { return G(29, 0x1367, 0x072a, 0); }
static void g_chance_text(int lo, int hi) { G(29, 0x1367, 0x0c52, 3, V_MSG_BUF, lo, hi); }
/* rules, s029 0e76 */
static int g_skill(int w, int k) { return G(29, 0x0e76, 0x01a0, 2, w, k); }
static int g_attr(int w, int k) { return G(29, 0x0e76, 0x05ee, 2, w, k); }
static int g_attr_max(int w, int k) { return G(29, 0x0e76, 0x0b64, 2, w, k); }
static void g_add_skill(int w, int k, int d) { G(29, 0x0e76, 0x02ea, 3, w, k, d); }
static void g_add_attr(int w, int k, int d) { G(29, 0x0e76, 0x0a72, 3, w, k, d); }
static void g_add_attr_max(int w, int k, int d) { G(29, 0x0e76, 0x0bb6, 3, w, k, d); }
static void g_set_attr(int w, int k, int v) { G(29, 0x0e76, 0x0988, 3, w, k, v); }
static int g_party_q(u16 off, int k) { return G(29, 0x0e76, off, 1, k); }
static int g_party_get(int what) { return G(29, 0x0e76, 0x1326, 1, what); }
static int g_has_item(int w, int code) { return G(29, 0x0e76, 0x118e, 2, w, code); }
static int g_count_items(int w, int code) { return G(29, 0x0e76, 0x0c76, 2, w, code); }
static int g_saint_lo(int w, int s) { return G(29, 0x0e76, 0x20c6, 2, w, s); }
static int g_saint_hi(int w, int s) { return G(29, 0x0e76, 0x2116, 2, w, s); }
static int g_invoke_saint(int w, int s)
{
    int fake = config_get_int("test.enc_saint", -99);       /* tests: the prayer's outcome without the prayer screen */
    if (fake != -99) { fprintf(stderr, "encounters: saint %d by %d: faked %d\n", s, w, fake); return fake; }
    return G(29, 0x0e76, 0x2180, 2, w, s);
}
static void g_drop_away(void) { G(29, 0x0e76, 0x2434, 0); }
static void g_rejoin_away(void) { G(29, 0x0e76, 0x251a, 0); }
static void g_rep_adj(int loc, int a, int b) { G(29, 0x0e76, 0x19d0, 3, loc, a, b); }
static void g_name_saint(int s) { G(29, 0x0e76, 0x1fb8, 3, 1, s, 0); }
static void g_load_city(int c) { G(29, 0x0e76, 0x4e68, 1, c); }
static int g_event_started(int k) { return G(29, 0x0e76, 0x32ce, 1, k); }
static int g_count_able(void) { return G(29, 0x0e76, 0x007c, 0); }
static int g_battle(const int *a)
{
    int fake = config_get_int("test.enc_battle", -99);      /* tests: the battle's result without fighting it */
    if (fake != -99) { fprintf(stderr, "encounters: battle %#x: %d x %#x (q %d), %d x %#x (q %d): faked result %d\n", a[0], a[4], a[2], a[3], a[7], a[5], a[6], fake); return fake; }
    LOG("battle %#x: %d x %#x (q %d), %d x %#x (q %d), %d x %#x (q %d)\n", a[0], a[4], a[2], a[3], a[7], a[5], a[6], a[10], a[8], a[9]);
    return gcall(29, 0x0e76, 0x2278, 11, a);
}
/* rules through overlay vectors (root 09c0) */
static void g_practice(int w, int k, int m, int p, int l) { G(VEC, 0, 0x1f63, 5, w, k, m, p, l); }
static int g_lore(void) { return G(VEC, 0, 0x1c1b, 0); }
static void g_injure(int w, int a, int b, int m) { G(VEC, 0, 0x2143, 4, w, a, b, m); }
static int g_present(void) { return G(VEC, 0, 0x2161, 0); }
static void g_reward(int k) { G(VEC, 0, 0x1f31, 1, k); }
static int g_nearest_city(void) { return G(VEC, 0, 0x1fa9, 2, (s16)g_rw(V_PARTY_X), (s16)g_rw(V_PARTY_Y)); }
static void g_lose_items(int w) { G(VEC, 0, 0x1eb9, 1, w); }
static int g_take_item(int w, int code) { return G(VEC, 0, 0x2085, 2, w, code); }
static void g_add_item(int w, int code) { G(VEC, 0, 0x2067, 2, w, code); }
static void g_add_item_q(int w, int code, int q) { G(VEC, 0, 0x1f8b, 3, w, code, q); }
static int g_item_quality(int w, int code) { return G(VEC, 0, 0x205d, 2, w, code); }
static void g_keep_items(int w, int p) { G(VEC, 0, 0x2021, 2, w, p); }
static void g_remove_flags(int w, long mask) { G(VEC, 0, 0x202b, 3, w, (int)(mask & 0xffff), (int)((mask >> 16) & 0xffff)); }
static void g_party_delete(int m) { G(VEC, 0, 0x18b5, 1, m); }

/* ---------------------------------------------------------------- memory helpers */
static void dset(u16 off, const char *s, int max)
{
    int i = 0;
    for (; s[i] && i < max - 1; i++) g_wb((u16)(off + i), (u8)s[i]);
    g_wb((u16)(off + i), 0);
}
/* a picture name as the game writes them: lower case (card_load gives pictures whose name
 * starts with 'x', the dark ones, white text) */
static void dset_pic(u16 off, const char *s)
{
    char b[16]; int i = 0;
    for (; s[i] && i < 12; i++) b[i] = (char)tolower((unsigned char)s[i]);
    b[i] = 0;
    dset(off, b, 13);
}
static void dcopy(u16 dst, u16 src) { int i = 0; u8 c; do { c = g_rb((u16)(src + i)); g_wb((u16)(dst + i), c); i++; } while (c && i < 200); }
static long money_get(u16 m) { return (s16)g_rw(m) * 240L + (s16)g_rw((u16)(m + 2)) * 12L + (s16)g_rw((u16)(m + 4)); }
static void money_put(u16 m, long pf)
{
    if (pf < 0) pf = 0;
    g_ww(m, (u16)(pf / 240)); pf %= 240;
    g_ww((u16)(m + 2), (u16)(pf / 12)); g_ww((u16)(m + 4), (u16)(pf % 12));
}
static int member_status(int m) { return m >= 0 && m < 5 ? g_rb(G_MEMBER(m)) : 0; }
static int leader(void) { return (s8)g_rb(G_LEADER); }
static void exit_to_map(void) { g_ww(G_CUR_LOC, 0xffff); g_ww(V_NEXT_CARD, 0x0c); }
static void script_error_at(const char *file)     /* a script error while it runs: the encounter ends, the player is never stuck in it */
{
    fprintf(stderr, "encounters: %s: an error while running: back to the map\n", file);
    ui_notice("A new encounter stopped: its script has an error (see darklands_port.log).");
    exit_to_map();
}
#define script_error(e) script_error_at((e)->file)
static long game_day(void) { return (long)(s16)g_rw(V_YEAR) * 365 + (s16)g_rw(V_MONTH) * 31 + (s16)g_rw(V_DAY); }

/* ================================================================ the language */
typedef struct {
    const char *s; int p, line;
    Enc *e;
    int dry;                             /* parse and check only: no effects */
    int err;
    int tt; long num; char id[48];       /* the current token */
} P;
enum { TEND, TNUM, TID, TOP };

static void perr(P *p, const char *fmt, ...)
{
    if (p->err) return;
    p->err = 1;
    char m[300]; va_list ap; va_start(ap, fmt); vsnprintf(m, sizeof m, fmt, ap); va_end(ap);
    fprintf(stderr, "encounters: %s, line %d: %s (near \"%.30s\")\n", p->e ? p->e->file : "?", p->line, m, p->s + (p->p > 0 ? p->p - 1 : 0));
}

static void nx(P *p)
{
    const char *s = p->s;
    while (s[p->p] == ' ' || s[p->p] == '\t' || s[p->p] == '\r' || s[p->p] == '\n') p->p++;
    char c = s[p->p];
    if (!c) { p->tt = TEND; p->id[0] = 0; return; }
    if (isdigit((unsigned char)c)) {
        char *end; p->num = strtol(s + p->p, &end, 0); p->p = (int)(end - s); p->tt = TNUM;
        if (p->num > 1000000000L || p->num < 0) p->num = 1000000000L;      /* (VMAX) */
        snprintf(p->id, sizeof p->id, "%ld", p->num);
        return;
    }
    if (isalpha((unsigned char)c) || c == '_' || c == '$') {
        int n = 0;
        while ((isalnum((unsigned char)s[p->p]) || s[p->p] == '_' || s[p->p] == '$' || s[p->p] == '.') && n < 47) p->id[n++] = s[p->p++];
        p->id[n] = 0; p->tt = TID;
        return;
    }
    p->tt = TOP;
    if (s[p->p + 1] == '=' && strchr("=!<>", c)) { p->id[0] = c; p->id[1] = '='; p->id[2] = 0; p->p += 2; return; }
    p->id[0] = c; p->id[1] = 0; p->p++;
}
static int is_op(P *p, const char *o) { return p->tt == TOP && !strcmp(p->id, o); }
static int is_word(P *p, const char *w) { return p->tt == TID && !strcasecmp(p->id, w); }
static int accept_op(P *p, const char *o) { if (is_op(p, o)) { nx(p); return 1; } return 0; }
static void expect_op(P *p, const char *o) { if (!accept_op(p, o)) perr(p, "'%s' expected", o); }

/* ---- names */
static const char *SKILLS[19] = { "edged", "impact", "flails", "polearms", "thrown", "bows", "missile", "alchemy", "religion",
    "virtue", "speakcommon", "latin", "readwrite", "healing", "artifice", "stealth", "streetwise", "riding", "woodwise" };
static const char *SKILLS2[19] = { "edg", "imp", "fll", "pol", "thr", "bow", "msl", "alch", "relig", "virt", "spkc", "spkl",
    "rw", "heal", "artif", "stlth", "strwise", "ride", "wdwise" };
static const char *ATTRS[7] = { "endurance", "strength", "agility", "perception", "intelligence", "charisma", "divinefavor" };
static const char *ATTRS2[7] = { "end", "str", "agl", "per", "int", "cha", "df" };
static const struct { const char *n; int v; } CONSTS[] = {
    /* enemies (DARKLAND.ENM) */
    { "Sergeant", 0 }, { "Archer", 2 }, { "Guard", 3 }, { "Bandit", 7 }, { "Follower", 0x0d }, { "Mercenary", 0x0f },
    { "BrigandSergeant", 0x12 }, { "RobberCaptain", 0x16 }, { "Alchemist", 0x18 }, { "Friar", 0x20 }, { "Knight", 0x21 },
    { "Captain", 0x25 }, { "Witch", 0x27 }, { "Wolf", 0x2a }, { "Boar", 0x2b }, { "Gargoyle", 0x2f }, { "Spider", 0x39 },
    { "Bear", 0x3a }, { "Tatzelwurm", 0x3b }, { "Schrat", 0x3e }, { "Huntsman", 1 }, { "Hussite", 5 }, { "Trooper", 6 },
    { "Brigand", 8 }, { "Madman", 9 }, { "Villager", 0x0b }, { "Thug", 0x0c }, { "Rebel", 0x0e }, { "Lackey", 0x10 },
    { "Henchman", 0x11 }, { "Soldier", 0x15 }, { "Raubritter", 0x23 }, { "Monk", 0x1d }, { "BadFriar", 0x1f }, { "Cultist", 0x29 },
    { "Dwarf", 0x2c }, { "Skeleton", 0x2d }, { "Gnome", 0x2e }, { "Kobold", 0x30 }, { "KoboldKing", 0x19 }, { "Undead", 0x32 },
    { "WildHunter", 0x3c }, { "Zealot", 0x49 }, { "BanditSergeant", 0x4e },
    /* battlefields */
    { "Wilds", 0x2f }, { "Surrounded", 0x30 }, { "Clearing", 0x2e }, { "Cave", 0x68 },   /* (a cave holds one enemy only) */
    /* battle results */
    { "Won", 0 }, { "Fled", 2 }, { "Beaten", 3 }, { "Surrendered", 4 },
    { "true", 1 }, { "false", 0 },
};

/* ---- run-time values */
static long eval(P *p);

static int var_index(Enc *e, const char *n) { for (int i = 0; i < e->nvars; i++) if (!strcasecmp(e->vars[i], n)) return i; return -1; }
static int name_index(const char **t, int n, const char *s) { for (int i = 0; i < n; i++) if (!strcasecmp(t[i], s)) return i; return -1; }
static int resolve_who(int w) { return w == -1 ? leader() : w; }

/* party queries that also name a member (best_member): which 0 value, 1 member */
static long party_query(P *p, u16 fn, int k, int member)
{
    if (p->dry) return 0;
    g_wb(V_BEST_MEMBER, (u8)leader());           /* (left as it was when no one has any of it) */
    int v = g_party_q(fn, k);
    int m = (s8)g_rb(V_BEST_MEMBER);
    if (member_status(m) != 1) m = leader();
    return member ? m : v;
}
static long party_query2(P *p, u16 fn, int a, int b, int member)
{
    if (p->dry) return 0;
    g_wb(V_BEST_MEMBER, (u8)leader());
    int v = G(29, 0x0e76, fn, 2, a, b);
    int m = (s8)g_rb(V_BEST_MEMBER);
    if (member_status(m) != 1) m = leader();
    return member ? m : v;
}

static int value_of(P *p, const char *n, long *v)
{
    int i;
    if (p->e && (i = var_index(p->e, n)) >= 0) { *v = rt.var[i]; return 1; }
    if ((i = name_index(SKILLS, 19, n)) >= 0 || (i = name_index(SKILLS2, 19, n)) >= 0) { *v = i; return 1; }
    if ((i = name_index(ATTRS, 7, n)) >= 0 || (i = name_index(ATTRS2, 7, n)) >= 0) { *v = i; return 1; }
    for (i = 0; i < (int)(sizeof CONSTS / sizeof CONSTS[0]); i++) if (!strcasecmp(CONSTS[i].n, n)) { *v = CONSTS[i].v; return 1; }
    if (!strcasecmp(n, "who")) { *v = rt.who; return 1; }
    if (!strcasecmp(n, "party")) { *v = -2; return 1; }
    if (!strcasecmp(n, "leader")) { *v = p->dry ? 0 : leader(); return 1; }
    if (!strcasecmp(n, "chosen")) { *v = p->dry ? 0 : (s16)g_rw(V_CHOSEN1); return 1; }
    if (!strcasecmp(n, "q") || !strcasecmp(n, "quality")) { *v = rt.q; return 1; }
    if (!strcasecmp(n, "saint")) { *v = rt.saint; return 1; }
    if (!strcasecmp(n, "item")) { *v = rt.item; return 1; }
    if (!strcasecmp(n, "result")) { *v = rt.result; return 1; }
    if (!strcasecmp(n, "number1")) { *v = p->dry ? 0 : (s16)g_rw(V_NUMBER1); return 1; }
    if (!strcasecmp(n, "money1")) { *v = p->dry ? 0 : money_get(V_MONEY1); return 1; }
    if (p->dry) {                        /* zero-argument functions: checked, not run */
        static const char *Z[] = { "lore", "fame", "mounted", "daylight", "night", "hour", "month", "day", "year", "money", "size",
            "women", "avgagility", "minagility", "slowest", "slowspeed", "spotter", "spotvalue", "terrain", "difficulty", "members", 0 };
        for (int k = 0; Z[k]; k++) if (!strcasecmp(n, Z[k])) { *v = 0; return 1; }
        return 0;
    }
    if (!strcasecmp(n, "lore")) { *v = g_lore(); return 1; }
    if (!strcasecmp(n, "fame")) { *v = g_party_get(4); return 1; }
    if (!strcasecmp(n, "mounted")) { *v = g_party_get(5) != 0; return 1; }
    if (!strcasecmp(n, "daylight")) { *v = g_daylight() != 0; return 1; }
    if (!strcasecmp(n, "night")) { *v = g_daylight() == 0; return 1; }
    if (!strcasecmp(n, "hour")) { *v = (s16)g_rw(V_HOUR); return 1; }
    if (!strcasecmp(n, "month")) { *v = (s16)g_rw(V_MONTH) + 1; return 1; }          /* 1 January .. 12 */
    if (!strcasecmp(n, "day")) { *v = (s16)g_rw(V_DAY) + 1; return 1; }
    if (!strcasecmp(n, "year")) { *v = (s16)g_rw(V_YEAR); return 1; }
    if (!strcasecmp(n, "money")) { *v = money_get(G_MONEY); return 1; }               /* the purse in pfennigs */
    if (!strcasecmp(n, "size")) { *v = g_present(); return 1; }                       /* members present */
    if (!strcasecmp(n, "members")) { *v = (s16)g_rw(G_PARTY_SIZE); return 1; }
    if (!strcasecmp(n, "women")) { *v = g_count_able(); return 1; }
    if (!strcasecmp(n, "avgagility")) { *v = G(29, 0x0e76, 0x060e, 0); return 1; }
    if (!strcasecmp(n, "minagility")) { *v = G(29, 0x0e76, 0x0656, 0); return 1; }
    if (!strcasecmp(n, "slowspeed")) { *v = G(29, 0x0e76, 0x088e, 0); return 1; }
    if (!strcasecmp(n, "slowest")) { g_wb(V_BEST_MEMBER, (u8)leader()); G(29, 0x0e76, 0x088e, 0); *v = (s8)g_rb(V_BEST_MEMBER); if (member_status((int)*v) != 1) *v = leader(); return 1; }
    if (!strcasecmp(n, "spotvalue")) { *v = G(29, 0x0e76, 0x184a, 2, 3, 0x12); return 1; }
    if (!strcasecmp(n, "spotter")) { g_wb(V_BEST_MEMBER, (u8)leader()); G(29, 0x0e76, 0x184a, 2, 3, 0x12); *v = (s8)g_rb(V_BEST_MEMBER); if (member_status((int)*v) != 1) *v = leader(); return 1; }
    if (!strcasecmp(n, "terrain")) { *v = (s16)g_rw(V_TERRAIN); return 1; }
    if (!strcasecmp(n, "difficulty")) { *v = g_rb(G_DIFFICULTY) & 3; return 1; }
    return 0;
}

static int nargs_ok(P *p, const char *f, int n, int lo, int hi)
{
    if (n < lo || n > hi) { perr(p, "%s takes %d argument%s", f, lo, lo == 1 ? "" : "s"); return 0; }
    return 1;
}

static long call_fn(P *p, const char *f, const long *a, int n)
{
    static const struct { const char *n; u16 fn; int member; } PQ[] = {
        { "maxskill", 0x14a4, 0 }, { "minskill", 0x1396, 0 }, { "avgskill", 0x1600, 0 },
        { "maxattr", 0x16fe, 0 }, { "minattr", 0x164a, 0 }, { "avgattr", 0x1800, 0 },
        { "bestskill", 0x14a4, 1 }, { "worstskill", 0x1396, 1 }, { "bestattr", 0x16fe, 1 }, { "worstattr", 0x164a, 1 } };
    for (int i = 0; i < (int)(sizeof PQ / sizeof PQ[0]); i++)
        if (!strcasecmp(f, PQ[i].n)) { if (!nargs_ok(p, f, n, 1, 1)) return 0; return party_query(p, PQ[i].fn, (int)a[0], PQ[i].member); }
    static const struct { const char *n; u16 fn; int member; } PQ2[] = {
        { "maxskill2", 0x153a, 0 }, { "bestskill2", 0x153a, 1 }, { "maxskillsum", 0x15a2, 0 }, { "bestskillsum", 0x15a2, 1 },
        { "maxattrsum", 0x179c, 0 }, { "bestattrsum", 0x179c, 1 }, { "maxattrskill", 0x184a, 0 }, { "bestattrskill", 0x184a, 1 } };
    for (int i = 0; i < (int)(sizeof PQ2 / sizeof PQ2[0]); i++)
        if (!strcasecmp(f, PQ2[i].n)) { if (!nargs_ok(p, f, n, 2, 2)) return 0; return party_query2(p, PQ2[i].fn, (int)a[0], (int)a[1], PQ2[i].member); }
    if (!strcasecmp(f, "skill") || !strcasecmp(f, "attr") || !strcasecmp(f, "attrmax")) {
        if (!nargs_ok(p, f, n, 2, 2) || p->dry) return 0;
        int w = resolve_who((int)a[0]);
        if (w < 0 || w > 4) return 0;
        return f[0] == 's' ? g_skill(w, (int)a[1]) : f[4] ? g_attr_max(w, (int)a[1]) : g_attr(w, (int)a[1]);
    }
    if (!strcasecmp(f, "random")) { if (!nargs_ok(p, f, n, 1, 1) || p->dry || a[0] <= 0) return 0; return g_rand((int)(a[0] > 32767 ? 32767 : a[0])); }
    if (!strcasecmp(f, "roll")) { if (!nargs_ok(p, f, n, 1, 1) || p->dry) return 0; return g_rand(100) <= a[0]; }   /* a check as the game rolls it */
    if (!strcasecmp(f, "clamp")) { if (!nargs_ok(p, f, n, 3, 3)) return 0; return a[2] < a[0] ? a[0] : a[2] > a[1] ? a[1] : a[2]; }
    if (!strcasecmp(f, "min")) { if (!nargs_ok(p, f, n, 2, 2)) return 0; return a[0] < a[1] ? a[0] : a[1]; }
    if (!strcasecmp(f, "max")) { if (!nargs_ok(p, f, n, 2, 2)) return 0; return a[0] > a[1] ? a[0] : a[1]; }
    if (!strcasecmp(f, "abs")) { if (!nargs_ok(p, f, n, 1, 1)) return 0; return a[0] < 0 ? -a[0] : a[0]; }
    if (!strcasecmp(f, "has") || !strcasecmp(f, "count") || !strcasecmp(f, "quality")) {
        if (!nargs_ok(p, f, n, 2, 2) || p->dry) return 0;
        int w = (int)a[0], code = (int)a[1];
        if (f[0] == 'q') return g_item_quality(resolve_who(w), code);
        if (f[0] == 'c') return g_count_items(w == -1 ? leader() : w, code);
        if (w == -2) { for (int m = 0; m < 5; m++) if (member_status(m) == 1 && g_has_item(m, code) == 1) return 1; return 0; }
        return g_has_item(resolve_who(w), code) == 1;
    }
    if (!strcasecmp(f, "able")) { if (!nargs_ok(p, f, n, 1, 1) || p->dry) return 0; return member_status(resolve_who((int)a[0])) == 1; }
    if (!strcasecmp(f, "event")) { if (!nargs_ok(p, f, n, 1, 1) || p->dry) return 0; return g_event_started((int)a[0]) != 0; }
    perr(p, "unknown function %s", f);
    return 0;
}

static long primary(P *p)
{
    if (p->tt == TNUM) { long v = p->num; nx(p); return v; }
    if (accept_op(p, "(")) { long v = eval(p); expect_op(p, ")"); return v; }
    if (p->tt == TID) {
        char n[48]; snprintf(n, sizeof n, "%s", p->id);
        nx(p);
        if (accept_op(p, "(")) {
            long a[6]; int k = 0;
            if (!is_op(p, ")")) do { long v = eval(p); if (k < 6) a[k++] = v; } while (accept_op(p, ",") && !p->err);
            expect_op(p, ")");
            return p->err ? 0 : call_fn(p, n, a, k);
        }
        long v;
        if (value_of(p, n, &v)) return v;
        perr(p, "unknown name %s", n);
        return 0;
    }
    perr(p, "a value expected");
    return 0;
}
static long unary(P *p)
{
    if (accept_op(p, "-")) return -unary(p);                 /* (within +-VMAX: no overflow) */
    if (accept_op(p, "!") || (is_word(p, "not") && (nx(p), 1))) return !unary(p);
    return primary(p);
}
/* values stay within +-1e9 (no overflow, whatever a script computes; long is 32 bits on Windows) */
#define VMAX 1000000000L
static long vclamp(long long x) { return x > VMAX ? VMAX : x < -VMAX ? -VMAX : (long)x; }
static long term(P *p)
{
    long v = unary(p);
    for (;;) {
        if (accept_op(p, "*")) v = vclamp((long long)v * unary(p));
        else if (accept_op(p, "/")) { long d = unary(p); v = d ? v / d : 0; }
        else if (accept_op(p, "%")) { long d = unary(p); v = d ? v % d : 0; }
        else return v;
    }
}
static long sum(P *p)
{
    long v = term(p);
    for (;;) {
        if (accept_op(p, "+")) v = vclamp((long long)v + term(p));
        else if (accept_op(p, "-")) v = vclamp((long long)v - term(p));
        else return v;
    }
}
static long cmp(P *p)
{
    long v = sum(p);
    if (accept_op(p, "==")) return v == sum(p);
    if (accept_op(p, "!=")) return v != sum(p);
    if (accept_op(p, "<=")) return v <= sum(p);
    if (accept_op(p, ">=")) return v >= sum(p);
    if (accept_op(p, "<")) return v < sum(p);
    if (accept_op(p, ">")) return v > sum(p);
    return v;
}
static long and_expr(P *p) { long v = cmp(p); while (is_word(p, "and")) { nx(p); long w = cmp(p); v = v && w; } return v; }
static long eval(P *p) { long v = and_expr(p); while (is_word(p, "or")) { nx(p); long w = and_expr(p); v = v || w; } return v; }

static void p_init(P *p, Enc *e, const Src *src, int dry)
{
    memset(p, 0, sizeof *p);
    p->s = src && src->s ? src->s : ""; p->line = src ? src->line : 0; p->e = e; p->dry = dry;
    nx(p);
}
/* an expression from the file (0 if empty or wrong) */
static long eval_src(Enc *e, const Src *src, int dry, int *err)
{
    if (!src->s) return 0;
    P p; p_init(&p, e, src, dry);
    long v = eval(&p);
    if (!p.err && p.tt != TEND) perr(&p, "unexpected \"%s\"", p.id);
    if (err) *err = p.err;
    return p.err ? 0 : v;
}

/* ---- actions */
static int find_sec(Enc *e, const char *n) { for (int i = 0; i < e->nsec; i++) if (!strcasecmp(e->sec[i].name, n)) return i; return -1; }

static int args(P *p, long *a, int lo, int hi, const char *what)
{
    int n = 0;
    if (p->tt != TEND && !is_op(p, ";") && !is_op(p, "}")) {
        do { long v = eval(p); if (n < 12) a[n] = v; n++; } while (accept_op(p, ",") && !p->err);
    }
    if (!p->err && (n < lo || n > hi)) perr(p, "%s takes %d to %d values", what, lo, hi);
    return n;
}

static int exec_list(P *p);
static int exec_stmt(P *p);

/* a branch of an if: { ... } or one statement; run or (skip) only checked */
static int branch(P *p, int run)
{
    int dry = p->dry, r = 0;
    if (!run) p->dry = 1;
    if (accept_op(p, "{")) { r = exec_list(p); expect_op(p, "}"); }
    else r = exec_stmt(p);
    p->dry = dry;
    return run ? r : 0;
}

static int jump(P *p, const char *n)
{
    int i = find_sec(p->e, n);
    if (i < 0) { perr(p, "no section [%s]", n); return 0; }
    if (p->dry) return 0;
    rt.sec = i; rt.jumped = 1;
    return 1;
}


/* values the game takes as indices: out of range they would read or write past its tables.
 * In the check (dry) only plain numbers are known; the rest is tested when it runs. */
static int who_ok(long w, int party_ok) { return (w >= 0 && w < 5) || w == -1 || (party_ok && w == -2); }
static int arg_range(P *p, const char *w, const long *a, int n)
{
    if (p->dry) return 1;
    const char *bad = 0;
    if (!strcasecmp(w, "practice")) { if (!who_ok(a[0], 1)) bad = "who"; else if (a[1] < 0 || a[1] > 18) bad = "a skill (0 to 18)"; }
    else if (!strcasecmp(w, "injure") || !strcasecmp(w, "loseitems") || !strcasecmp(w, "keepitems") || !strcasecmp(w, "removeitems")) { if (!who_ok(a[0], 1)) bad = "who"; }
    else if (!strcasecmp(w, "additem")) { if (!who_ok(a[0], 0)) bad = "who (one member)"; else if (a[1] < 0 || a[1] > 199) bad = "an item (0 to 199)"; else if (n > 2 && (a[2] < 1 || a[2] > 99)) bad = "a quality (1 to 99)"; }
    else if (!strcasecmp(w, "takeitem")) { if (!who_ok(a[0], 0)) bad = "who (one member)"; else if (a[1] < 0 || a[1] > 199) bad = "an item (0 to 199)"; }
    else if (!strcasecmp(w, "kill") || !strcasecmp(w, "chosen") || !strcasecmp(w, "chosen2")) { if (!who_ok(a[0], 0)) bad = "who (one member)"; }
    else if (!strcasecmp(w, "itemname")) { if (a[0] < 0 || a[0] > 199) bad = "an item (0 to 199)"; }
    else if (!strcasecmp(w, "saintname")) { if (a[0] < 0 || a[0] > 135) bad = "a saint (0 to 135)"; }
    else if (!strcasecmp(w, "fame")) { if (a[0] < 1 || a[0] > 5) bad = "1 to 5"; }
    else if (!strcasecmp(w, "text")) { if (a[0] < 0 || a[0] > 99) bad = "a text block"; }
    else if (!strcasecmp(w, "hours") || !strcasecmp(w, "days")) { if (a[0] > 9999) bad = "at most 9999"; }
    else if (!strcasecmp(w, "until")) { if (a[0] < 0 || a[0] > 23) bad = "an hour (0 to 23)"; }
    if (!bad) return 1;
    perr(p, "%s: %s expected", w, bad);
    return 0;
}

static int exec_stmt(P *p)
{
    if (p->tt != TID) { perr(p, "an action expected"); return 1; }
    char w[48]; snprintf(w, sizeof w, "%s", p->id);
    nx(p);
    long a[12]; int n;
    int dry = p->dry;
    if (!strcasecmp(w, "if")) {
        long c = eval(p);
        if (p->err) return 1;
        if (is_word(p, "goto")) {
            nx(p); char t1[48]; snprintf(t1, sizeof t1, "%s", p->id); nx(p);
            int r = 0;
            if (c) r = jump(p, t1); else if (dry) jump(p, t1);
            if (is_word(p, "else")) {
                nx(p); if (is_word(p, "goto")) nx(p);
                char t2[48]; snprintf(t2, sizeof t2, "%s", p->id); nx(p);
                if (!c) r = jump(p, t2); else if (dry) jump(p, t2);
            }
            return r;
        }
        if (!is_word(p, "then")) { perr(p, "'then' or 'goto' expected"); return 1; }
        nx(p);
        int r = branch(p, c && !dry);
        if (is_word(p, "else")) { nx(p); int r2 = branch(p, !c && !dry); if (!c) r = r2; }
        return dry ? 0 : r;
    }
    if (!strcasecmp(w, "goto")) { char t[48]; snprintf(t, sizeof t, "%s", p->id); nx(p); return jump(p, t); }
    if (!strcasecmp(w, "exit")) { if (!dry) { exit_to_map(); return 1; } return 0; }
    if (!strcasecmp(w, "tocity")) {             /* the journey ends at the nearest city's gate */
        if (!dry) { int c = g_nearest_city(); g_ww(G_CUR_LOC, (u16)c); g_load_city(c); g_ww(V_NEXT_CARD, 0x0c); return 1; }
        return 0;
    }
    if (!strcasecmp(w, "set")) {
        char v[48]; snprintf(v, sizeof v, "%s", p->id); nx(p);
        expect_op(p, "=");
        long x = eval(p);
        int i = var_index(p->e, v);
        if (i < 0) { perr(p, "unknown variable %s", v); return 1; }
        if (!dry && !p->err) rt.var[i] = x;
        return 0;
    }
    if (!strcasecmp(w, "picture")) {
        char pic[48]; snprintf(pic, sizeof pic, "%s", p->id); nx(p);
        if (strlen(pic) > 8) perr(p, "picture names have at most 8 letters");
        if (!dry && !p->err) { dset_pic(V_PIC_BUF2, pic); g_card_load_near(); }
        return 0;
    }
    if (!strcasecmp(w, "music")) { if (!dry) g_music(); return 0; }
    if (!strcasecmp(w, "rejoin")) { if (!dry) g_rejoin_away(); return 0; }
    if (!strcasecmp(w, "dropaway")) { if (!dry) g_drop_away(); return 0; }
    if (!strcasecmp(w, "disable") || !strcasecmp(w, "enable") || !strcasecmp(w, "hide")) {
        n = args(p, a, 1, 1, w);
        if (!p->err && (a[0] < 0 || a[0] >= NOPT)) perr(p, "options are numbered 0 to 9");
        if (!dry && !p->err) rt.flag[a[0]] = w[0] == 'd' ? 2 : w[0] == 'e' ? 1 : 0;
        return 0;
    }
    if (!strcasecmp(w, "battle")) {             /* battle FIELD, ENEMY, QUALITY, COUNT [, ENEMY, QUALITY, COUNT [, ...]] */
        n = args(p, a, 4, 10, w);
        if (!p->err && (n - 1) % 3) perr(p, "battle: a battlefield, then enemy, quality, count for each group");
        if (dry || p->err) return 0;
        int b[11] = { (int)a[0], -1, -1, 0, 0, -1, 0, 0, -1, 0, 0 };
        int ng = 0, total = 0;                  /* a battle holds 8 foes (13 combatants, 5 of them the party) */
        for (int g = 0; g < 3 && 1 + 3 * g < n; g++) {
            long t = a[1 + 3 * g], q = a[2 + 3 * g], c = a[3 + 3 * g];
            if (t < 0 || t > 81) { fprintf(stderr, "encounters: %s: battle: no enemy %ld (0 to 81), group left out\n", p->e->file, t); continue; }
            if (c < 1) c = 1;
            if (c > (a[0] == 0x68 ? 1 : 8) - total) c = (a[0] == 0x68 ? 1 : 8) - total;     /* (a cave holds one foe) */
            if (c < 1) continue;
            b[2 + 3 * ng] = (int)t;
            b[3 + 3 * ng] = (int)(q < 0 ? 0 : q > 9 ? 9 : q);       /* the game's own cards: 0 to 6 */
            b[4 + 3 * ng] = (int)c;
            ng++; total += (int)c;
        }
        if (!ng) { rt.result = 0; LOG("battle: no foes\n"); return 0; }
        rt.result = g_battle(b);
        if (rt.result == 1) rt.result = 2;
        LOG("battle %d: result %d\n", (int)a[0], rt.result);
        return 0;
    }
    if (!strcasecmp(w, "skill") || !strcasecmp(w, "attr") || !strcasecmp(w, "attrmax") || !strcasecmp(w, "setattr")) {
        /* skill WHO, K, AMOUNT: with WHO party the amount is worked out for each member (who = that member) */
        long who = eval(p); expect_op(p, ","); long k = eval(p); expect_op(p, ",");
        if (p->err) return 1;
        if (!dry && (!who_ok(who, 1) || k < 0 || k > (w[0] == 's' && w[1] == 'k' ? 18 : 6))) { perr(p, "%s: who, then a %s", w, w[0] == 's' && w[1] == 'k' ? "skill (0 to 18)" : "attribute (0 to 6)"); return 1; }
        void (*f)(int, int, int) = w[0] == 's' && w[1] == 'k' ? g_add_skill : w[0] == 's' ? g_set_attr : w[4] ? g_add_attr_max : g_add_attr;
        P at = *p;
        int saved = rt.who;
        if (!dry && who == -2) {
            for (int m = 0; m < 5; m++) if (member_status(m) == 1) { *p = at; rt.who = m; long v = eval(p); if (!p->err) f(m, (int)k, (int)v); }
        }
        *p = at; rt.who = who == -2 ? saved : resolve_who((int)who);
        { int d0 = p->dry; if (who == -2) p->dry = 1; long v = eval(p); p->dry = d0;
          if (!dry && !p->err && who != -2) { int m = resolve_who((int)who); if (m >= 0 && m < 5) f(m, (int)k, (int)v); } }
        rt.who = saved;
        return 0;
    }
    /* the rest: a keyword and its values */
    static const struct { const char *w; int lo, hi; } K[] = {
        { "text", 1, 1 }, { "hours", 1, 1 }, { "days", 1, 1 }, { "until", 1, 1 }, { "practice", 2, 5 },
        { "injure", 2, 4 }, { "pay", 1, 1 }, { "gain", 1, 1 },
        { "money1", 1, 1 }, { "number1", 1, 1 }, { "number2", 1, 1 }, { "fame", 1, 1 }, { "reputation", 2, 2 },
        { "additem", 2, 3 }, { "takeitem", 2, 2 }, { "loseitems", 1, 1 }, { "keepitems", 2, 2 }, { "removeitems", 2, 2 },
        { "kill", 1, 1 }, { "chosen", 1, 1 }, { "chosen2", 1, 1 }, { "itemname", 1, 1 }, { "saintname", 1, 1 }, { "actor", 1, 1 } };
    int k = -1;
    for (int i = 0; i < (int)(sizeof K / sizeof K[0]); i++) if (!strcasecmp(w, K[i].w)) { k = i; break; }
    if (k < 0) { perr(p, "unknown action %s", w); return 1; }
    n = args(p, a, K[k].lo, K[k].hi, w);
    if (!p->err && !arg_range(p, w, a, n)) return 1;
    if (!p->err && !strcasecmp(w, "text") && p->e && p->e->nblocks && (a[0] < 0 || a[0] >= p->e->nblocks)) { perr(p, "text %ld: %s has blocks 0 to %d", a[0], p->e->card, p->e->nblocks - 1); return 1; }
    if (dry || p->err) return 0;
    if (!strcasecmp(w, "text")) g_card_text((int)a[0]);
    else if (!strcasecmp(w, "hours")) g_hours((int)a[0]);
    else if (!strcasecmp(w, "days")) g_days((int)a[0]);
    else if (!strcasecmp(w, "until")) g_hours(g_hours_until((int)a[0]));
    else if (!strcasecmp(w, "practice")) g_practice((int)a[0], (int)a[1], n > 2 ? (int)a[2] : 1, n > 3 ? (int)a[3] : 10, n > 4 ? (int)a[4] : 0);
    else if (!strcasecmp(w, "injure")) g_injure((int)a[0], (int)a[1], n > 2 ? (int)a[2] : (int)a[1], n > 3 ? (int)a[3] : 10);
    else if (!strcasecmp(w, "pay")) money_put(G_MONEY, money_get(G_MONEY) - a[0]);
    else if (!strcasecmp(w, "gain")) money_put(G_MONEY, money_get(G_MONEY) + a[0]);
    else if (!strcasecmp(w, "money1")) money_put(V_MONEY1, a[0]);
    else if (!strcasecmp(w, "number1")) g_ww(V_NUMBER1, (u16)a[0]);
    else if (!strcasecmp(w, "number2")) g_ww(V_NUMBER2, (u16)a[0]);
    else if (!strcasecmp(w, "fame")) g_reward((int)a[0]);
    else if (!strcasecmp(w, "reputation")) g_rep_adj(g_nearest_city(), (int)a[0], (int)a[1]);
    else if (!strcasecmp(w, "additem")) { int m = resolve_who((int)a[0]); if (n > 2) g_add_item_q(m, (int)a[1], (int)a[2]); else g_add_item(m, (int)a[1]); g_ww(V_ITEM, (u16)a[1]); }
    else if (!strcasecmp(w, "takeitem")) g_take_item(resolve_who((int)a[0]), (int)a[1]);
    else if (!strcasecmp(w, "loseitems")) g_lose_items((int)a[0] == -1 ? leader() : (int)a[0]);
    else if (!strcasecmp(w, "keepitems")) g_keep_items((int)a[0] == -1 ? leader() : (int)a[0], (int)a[1]);
    else if (!strcasecmp(w, "removeitems")) g_remove_flags((int)a[0] == -1 ? leader() : (int)a[0], a[1]);
    else if (!strcasecmp(w, "kill")) {          /* a member is lost (taken, dead): as the game's own cards do it */
        int m = resolve_who((int)a[0]);
        int able = 0; for (int i = 0; i < 5; i++) if (member_status(i) == 1 && i != m) able++;
        if (m >= 0 && m < 5 && member_status(m) && able) { g_wb(G_MEMBER(m), 0); g_party_delete(m); g_panel_rows(); }
    }
    else if (!strcasecmp(w, "chosen")) g_ww(V_CHOSEN1, (u16)resolve_who((int)a[0]));
    else if (!strcasecmp(w, "chosen2")) g_ww((u16)(V_CHOSEN1 + 2), (u16)resolve_who((int)a[0]));
    else if (!strcasecmp(w, "itemname")) g_ww(V_ITEM, (u16)a[0]);
    else if (!strcasecmp(w, "saintname")) g_name_saint((int)a[0]);
    else if (!strcasecmp(w, "actor")) rt.who = (int)a[0];
    return 0;
}

/* statements separated by ';' up to the end or a '}'; 1 if one of them left (goto, exit) */
static int exec_list(P *p)
{
    while (!p->err) {
        if (p->tt == TEND || is_op(p, "}")) return 0;
        if (accept_op(p, ";")) continue;
        int r = exec_stmt(p);
        if (aborting) return 1;
        if (r && !p->dry) return 1;
        if (!p->err && p->tt != TEND && !is_op(p, ";") && !is_op(p, "}")) perr(p, "';' expected");
    }
    return 1;
}
static int run_src(Enc *e, const Src *src, int dry)
{
    if (!src->s) return 0;
    P p; p_init(&p, e, src, dry);
    int r = exec_list(&p);
    if (!p.err && !r && p.tt != TEND) perr(&p, "unexpected \"%s\"", p.id);     /* (after a goto or exit the rest is not run) */
    return p.err ? (dry ? -1 : 2) : r;          /* 2: an error while running */
}

/* ================================================================ loading the files */
static char *sdup(const char *s) { size_t n = strlen(s); char *d = malloc(n + 1); memcpy(d, s, n + 1); return d; }
static void src_set(Src *d, const char *v, int line) { free(d->s); d->s = sdup(v); d->line = line; }
static void src_add(Src *d, const char *v)
{
    size_t a = strlen(d->s), b = strlen(v);
    char *n = malloc(a + b + 3); memcpy(n, d->s, a); n[a] = ';'; n[a + 1] = ' '; memcpy(n + a + 2, v, b + 1);
    free(d->s); d->s = n;
}
static void free_enc(Enc *e)
{
    for (int i = 0; i < e->nsec; i++) {
        Sec *s = &e->sec[i];
        free(s->chosen.s); free(s->enter.s);
        for (int k = 0; k < NOPT; k++) {
            Opt *o = &s->opt[k];
            free(o->only.s); free(o->need.s); free(o->who.s); free(o->chance.s); free(o->ok.s); free(o->fail.s);
            for (int j = 0; j < o->npot; j++) free(o->pot[j].s);
        }
    }
    free(e->sec); free(e->only.s);
    memset(e, 0, sizeof *e);
}

static unsigned terrain_bits(const char *v, int *bad)
{
    static const struct { const char *n; unsigned bits; } T[] = {
        { "marsh", 1u << 4 | 1u << 5 }, { "woods", 1u << 6 | 1u << 7 }, { "farmland", 1u << 8 | 1u << 9 },
        { "plains", 1u << 10 | 1u << 11 }, { "forest", 0xfu << 12 }, { "pinewood", 1u << 16 | 1u << 17 },
        { "hills", 1u << 18 | 1u << 19 }, { "mountains", 0xfu << 20 }, { "road", 1u << 24 },
        { "wild", 0x01ff00f0u & ~(1u << 8 | 1u << 9 | 1u << 24) }, { "any", 0x01fffff0u } };
    unsigned m = 0;
    char buf[300]; snprintf(buf, sizeof buf, "%s", v);
    for (char *t = strtok(buf, " ,\t"); t; t = strtok(0, " ,\t")) {
        int found = 0;
        for (int i = 0; i < (int)(sizeof T / sizeof T[0]); i++) if (!strcasecmp(t, T[i].n)) { m |= T[i].bits; found = 1; }
        if (!found && isdigit((unsigned char)*t)) { int k = atoi(t); if (k >= 0 && k < 32) { m |= 1u << k; found = 1; } }
        if (!found) *bad = 1;
    }
    return m;
}

static void ints(const char *v, int *out, int max)
{
    for (int i = 0; i < max; i++) out[i] = -1;
    char buf[300]; snprintf(buf, sizeof buf, "%s", v);
    int n = 0;
    for (char *t = strtok(buf, " ,\t"); t && n < max; t = strtok(0, " ,\t")) out[n++] = (int)strtol(t, 0, 0);
}

static int is_opt_key(const char *w)
{
    static const char *K[] = { "only", "need", "who", "chance", "potion", "ok", "then", "success", "fail", "failure", 0 };
    for (int i = 0; K[i]; i++) if (!strcasecmp(w, K[i])) return 1;
    return 0;
}
static int is_sec_key(const char *w)
{
    static const char *K[] = { "block", "picture", "chosen", "saints", "items", "do", "option", 0 };
    for (int i = 0; K[i]; i++) if (!strcasecmp(w, K[i])) return 1;
    return 0;
}

static void ferr(Enc *e, int line, const char *fmt, ...)
{
    char m[300]; va_list ap; va_start(ap, fmt); vsnprintf(m, sizeof m, fmt, ap); va_end(ap);
    fprintf(stderr, "encounters: %s, line %d: %s\n", e->file, line, m);
    e->ok = 0;
}

static int load_file(Enc *e, const char *path, const char *id)
{
    memset(e, 0, sizeof *e);
    snprintf(e->file, sizeof e->file, "%s", path);
    snprintf(e->id, sizeof e->id, "%s", id);
    snprintf(e->title, sizeof e->title, "%s", id);
    e->weight = 1; e->terrain = 0x01fffff0u; e->ok = 1; e->last_day = -1000;
    FILE *f = fopen(path, "rb");
    if (!f) return 0;
    char line[2000];
    int ln = 0;
    Sec *s = 0; Opt *o = 0; Src *last = 0;
    while (fgets(line, sizeof line, f)) {
        ln++;
        char *h = strchr(line, '#'); if (h) *h = 0;
        size_t L = strlen(line);
        while (L && (line[L - 1] == '\n' || line[L - 1] == '\r' || line[L - 1] == ' ' || line[L - 1] == '\t')) line[--L] = 0;
        if (!L) continue;
        int indented = line[0] == ' ' || line[0] == '\t';
        char *t = line; while (*t == ' ' || *t == '\t') t++;
        if (*t == '[') {
            char *end = strchr(t, ']');
            if (!end) { ferr(e, ln, "']' missing"); continue; }
            *end = 0;
            e->sec = realloc(e->sec, (size_t)(e->nsec + 1) * sizeof(Sec));
            s = &e->sec[e->nsec++]; memset(s, 0, sizeof *s);
            snprintf(s->name, sizeof s->name, "%s", t + 1);
            s->line = ln; s->block = -1;
            for (int i = 0; i < 4; i++) s->saints[i] = s->items[i] = -1;
            o = 0; last = 0;
            continue;
        }
        char w[48]; int n = 0;
        while (t[n] && !isspace((unsigned char)t[n]) && n < 47) { w[n] = t[n]; n++; }
        w[n] = 0;
        char *v = t + n; while (*v == ' ' || *v == '\t') v++;
        if (!s) {                                       /* the header */
            if (!strcasecmp(w, "name")) snprintf(e->title, sizeof e->title, "%s", v);
            else if (!strcasecmp(w, "card")) { if (strlen(v) > 8 || !*v) ferr(e, ln, "card: a MSGFILES name of at most 8 letters"); snprintf(e->card, sizeof e->card, "%s", v); }
            else if (!strcasecmp(w, "picture")) { if (strlen(v) > 8 || !*v) ferr(e, ln, "picture: at most 8 letters"); snprintf(e->pic, sizeof e->pic, "%s", v); }
            else if (!strcasecmp(w, "terrain")) { int bad = 0; e->terrain = terrain_bits(v, &bad); if (bad) ferr(e, ln, "terrain: marsh woods farmland plains forest pinewood hills mountains road wild any, or numbers"); }
            else if (!strcasecmp(w, "when")) { e->when = !strcasecmp(v, "day") ? 1 : !strcasecmp(v, "night") ? 2 : 0; if (e->when == 0 && strcasecmp(v, "any")) ferr(e, ln, "when: day, night or any"); }
            else if (!strcasecmp(w, "weight")) e->weight = atoi(v);
            else if (!strcasecmp(w, "only")) src_set(&e->only, v, ln);
            else if (!strcasecmp(w, "music")) e->music = !strcasecmp(v, "yes") || !strcasecmp(v, "ambush");
            else if (!strcasecmp(w, "vars")) {
                char buf[300]; snprintf(buf, sizeof buf, "%s", v);
                for (char *q = strtok(buf, " ,\t"); q; q = strtok(0, " ,\t")) if (e->nvars < 24) snprintf(e->vars[e->nvars++], 24, "%s", q);
            }
            else ferr(e, ln, "unknown setting %s", w);
            continue;
        }
        if (!strcasecmp(w, "option")) {
            char kind[32] = ""; int k = -1;
            sscanf(v, "%d %31s", &k, kind);
            if (k < 0 || k >= NOPT) { ferr(e, ln, "option: a number 0 to 9"); o = 0; continue; }
            o = &s->opt[k];
            if (o->defined) ferr(e, ln, "option %d twice", k);
            o->defined = 1;
            o->kind = !strcasecmp(kind, "potion") ? 1 : !strcasecmp(kind, "saint") ? 2 : 0;
            if (*kind && !o->kind) ferr(e, ln, "option %d: potion or saint", k);
            last = 0;
            continue;
        }
        if (o && is_opt_key(w)) {
            Src *d = 0;
            if (!strcasecmp(w, "only")) d = &o->only;
            else if (!strcasecmp(w, "need")) d = &o->need;
            else if (!strcasecmp(w, "who")) d = &o->who;
            else if (!strcasecmp(w, "chance")) d = &o->chance;
            else if (!strcasecmp(w, "ok") || !strcasecmp(w, "then") || !strcasecmp(w, "success")) d = &o->ok;
            else if (!strcasecmp(w, "fail") || !strcasecmp(w, "failure")) d = &o->fail;
            else if (!strcasecmp(w, "potion")) {
                if (o->npot >= 4) { ferr(e, ln, "at most 4 potions"); continue; }
                char *c = strchr(v, ',');
                if (!c) { ferr(e, ln, "potion CODE, CHANCE"); continue; }
                *c = 0;
                o->pot_code[o->npot] = (int)strtol(v, 0, 0);
                d = &o->pot[o->npot++];
                v = c + 1; while (*v == ' ') v++;
            }
            if (d->s) ferr(e, ln, "%s twice", w);
            src_set(d, v, ln);
            last = (d == &o->only || d == &o->need || d == &o->who || d == &o->chance) ? 0 : d;
            continue;
        }
        if (!indented && is_sec_key(w)) {
            o = 0; last = 0;
            if (!strcasecmp(w, "block")) s->block = atoi(v);
            else if (!strcasecmp(w, "picture")) { if (strlen(v) > 8) ferr(e, ln, "picture: at most 8 letters"); snprintf(s->pic, sizeof s->pic, "%s", v); }
            else if (!strcasecmp(w, "chosen")) src_set(&s->chosen, v, ln);
            else if (!strcasecmp(w, "saints")) ints(v, s->saints, 4);
            else if (!strcasecmp(w, "items")) ints(v, s->items, 4);
            else if (!strcasecmp(w, "do")) { if (s->enter.s) ferr(e, ln, "do twice"); src_set(&s->enter, v, ln); last = &s->enter; }
            continue;
        }
        if (indented && last) { src_add(last, t); continue; }
        ferr(e, ln, "unknown key %s", w);
    }
    fclose(f);
    if (!e->card[0]) ferr(e, 1, "no card (the MSGFILES text) given");
    if (!e->pic[0]) ferr(e, 1, "no picture given");
    if (!e->nsec) ferr(e, 1, "no [section]");
    return 1;
}

/* the shape of the encounter's card text (MSGFILES/<card>.TXT of a mod): its blocks and, in each,
 * the option lines in order with their codes, so the script can be checked against them */
static void card_shape(Enc *e)
{
    e->nblocks = 0;
    char dirs[16][1200];
    int nd = datamods_dirs("MSGFILES", dirs, 16);
    char want[32]; snprintf(want, sizeof want, "%s.TXT", e->card);
    FILE *f = 0;
    for (int d = 0; d < nd && !f; d++) {
        DIR *dp = opendir(dirs[d]);
        if (!dp) continue;
        struct dirent *de;
        while ((de = readdir(dp))) if (!strcasecmp(de->d_name, want)) { char p[2400]; snprintf(p, sizeof p, "%s/%s", dirs[d], de->d_name); f = fopen(p, "rb"); break; }
        closedir(dp);
    }
    if (!f) return;
    char line[2000];
    int b = -1;
    while (fgets(line, sizeof line, f)) {
        if (line[0] == '#') continue;
        if (!strncmp(line, "=== card", 8)) { b++; if (b < 64) e->nopts[b] = 0; continue; }
        if (b < 0 || b >= 64) continue;
        /* the first character code of the line, past any \14 */
        const char *t = line; int c = -1;
        for (;;) {
            if (t[0] == '\\' && isxdigit((unsigned char)t[1]) && isxdigit((unsigned char)t[2])) {
                char h[3] = { t[1], t[2], 0 }; int v = (int)strtol(h, 0, 16);
                if (v == 0x14) { t += 3; continue; }
                c = v;
            } else c = (unsigned char)t[0];
            break;
        }
        if ((c == 0x15 || c == 0x16 || c == 0x10 || c == 0x06) && e->nopts[b] < NOPT) e->optcode[b][e->nopts[b]++] = (unsigned char)c;
    }
    fclose(f);
    e->nblocks = b + 1;
}

/* everything in the file parsed and checked once, without running it */
static void check(Enc *e)
{
    int bad = 0;
    card_shape(e);
    if (e->only.s) { int er; eval_src(e, &e->only, 1, &er); bad |= er; }
    for (int i = 0; i < e->nsec; i++) {
        Sec *s = &e->sec[i];
        int er;
        if (s->chosen.s) { eval_src(e, &s->chosen, 1, &er); bad |= er; }
        if (run_src(e, &s->enter, 1) < 0) bad = 1;
        int any = 0;
        for (int k = 0; k < NOPT; k++) {
            Opt *o = &s->opt[k];
            if (!o->defined) continue;
            any = 1;
            if (s->block < 0) { ferr(e, s->line, "[%s] has options but no block", s->name); bad = 1; }
            else if (e->nblocks && s->block < 64) {
                int nl = s->block < e->nblocks ? e->nopts[s->block] : 0;
                if (k >= nl) { ferr(e, s->line, "[%s] option %d: block %d of %s has %d option line%s", s->name, k, s->block, e->card, nl, nl == 1 ? "" : "s"); bad = 1; }
                else {
                    int c = e->optcode[s->block][k];
                    if ((c == 0x16) != (o->kind == 2)) { ferr(e, s->line, "[%s] option %d: %s", s->name, k, c == 0x16 ? "its line (\\16) opens the saints: write option N saint" : "a saint option needs a \\16 line"); bad = 1; }
                    if ((c == 0x10) != (o->kind == 1)) { ferr(e, s->line, "[%s] option %d: %s", s->name, k, c == 0x10 ? "its line (\\10) opens the potions: write option N potion" : "a potion option needs a \\10 line"); bad = 1; }
                }
            }
            Src *ex[] = { &o->only, &o->need, &o->who };
            for (int j = 0; j < 3; j++) if (ex[j]->s) { eval_src(e, ex[j], 1, &er); bad |= er; }
            if (o->chance.s && strcasecmp(o->chance.s, "none") && strcasecmp(o->chance.s, "assured") && strcasecmp(o->chance.s, "unknown") && strcasecmp(o->chance.s, "combat"))
                { eval_src(e, &o->chance, 1, &er); bad |= er; }
            for (int j = 0; j < o->npot; j++) { eval_src(e, &o->pot[j], 1, &er); bad |= er; }
            if (run_src(e, &o->ok, 1) < 0 || run_src(e, &o->fail, 1) < 0) bad = 1;
            if (o->kind == 2 && s->saints[0] < 0) { ferr(e, s->line, "[%s] option %d is a saint option, but there are no saints", s->name, k); bad = 1; }
            if (o->kind == 1 && s->items[0] < 0) { ferr(e, s->line, "[%s] option %d is a potion option, but there are no items", s->name, k); bad = 1; }
        }
        if (s->block >= 0 && !any) { ferr(e, s->line, "[%s] has a block but no options", s->name); bad = 1; }
        for (int j = 0; j < 4; j++) {
            if (s->saints[j] < -1 || s->saints[j] > 135) { ferr(e, s->line, "[%s] saints: numbers 0 to 135 (DARKLAND.LST), or -1", s->name); bad = 1; break; }
            if (s->items[j] < -1 || s->items[j] > 199) { ferr(e, s->line, "[%s] items: numbers 0 to 199 (DARKLAND.LST), or -1", s->name); bad = 1; break; }
        }
        if (s->block >= 0 && e->nblocks && s->block >= e->nblocks) { ferr(e, s->line, "[%s] block %d: %s has blocks 0 to %d", s->name, s->block, e->card, e->nblocks - 1); bad = 1; }
    }
    if (bad) e->ok = 0;
}

static unsigned slot_hash(const char *id) { unsigned h = 2166136261u; for (; *id; id++) h = (h ^ (unsigned)tolower((unsigned char)*id)) * 16777619u; return h; }
static int cmp_enc(const void *a, const void *b) { return strcasecmp(((const Enc *)a)->id, ((const Enc *)b)->id); }

static void reload(void)
{
    int gen = datamods_generation();
    if (gen == loaded_gen) return;
    loaded_gen = gen;
    for (int i = 0; i < nenc; i++) free_enc(&encs[i]);
    nenc = 0; nerr = 0;
    char dirs[16][1200];
    int nd = datamods_dirs("ENCOUNTERS", dirs, 16);
    for (int d = 0; d < nd; d++) {
        DIR *dp = opendir(dirs[d]);
        if (!dp) continue;
        struct dirent *de;
        while ((de = readdir(dp)) && nenc < ENC_MAX) {
            size_t L = strlen(de->d_name);
            if (L < 5 || strcasecmp(de->d_name + L - 4, ".txt")) continue;
            char id[40]; snprintf(id, sizeof id, "%.*s", (int)(L - 4 < 39 ? L - 4 : 39), de->d_name);
            int dup = 0; for (int i = 0; i < nenc; i++) if (!strcasecmp(encs[i].id, id)) dup = 1;   /* a later mod's file wins */
            if (dup) continue;
            char path[2400]; snprintf(path, sizeof path, "%s/%s", dirs[d], de->d_name);
            Enc *e = &encs[nenc];
            if (!load_file(e, path, id)) continue;
            if (e->ok) check(e);
            if (!e->ok) nerr++;
            nenc++;
        }
        closedir(dp);
    }
    qsort(encs, (size_t)nenc, sizeof(Enc), cmp_enc);
    /* card numbers by name, so a save made during one still finds it with other mods in use */
    for (int i = 0; i < ENC_MAX; i++) by_slot[i] = -1;
    for (int i = 0; i < nenc; i++) {
        int sl = (int)(slot_hash(encs[i].id) % ENC_MAX);
        while (by_slot[sl] >= 0) sl = (sl + 1) % ENC_MAX;
        by_slot[sl] = i; encs[i].slot = sl;
    }
    int good = 0; for (int i = 0; i < nenc; i++) good += encs[i].ok;
    if (nenc) fprintf(stderr, "encounters: %d new encounters (%d with errors)\n", good, nerr);
    if (nerr) { char m[200]; snprintf(m, sizeof m, "New encounters: %d file%s with errors left out (see darklands_port.log).", nerr, nerr == 1 ? "" : "s"); ui_notice(m); }
}

/* ================================================================ running a card */
static Sec *cur_sec(void) { return rt.active ? &encs[rt.enc].sec[rt.sec] : 0; }

enum { CH_NUM, CH_NONE, CH_ASSURED, CH_UNKNOWN, CH_COMBAT };
static int chance_mode(const Opt *o)
{
    if (!o->chance.s || !strcasecmp(o->chance.s, "none")) return CH_NONE;
    if (!strcasecmp(o->chance.s, "assured")) return CH_ASSURED;
    if (!strcasecmp(o->chance.s, "unknown")) return CH_UNKNOWN;
    if (!strcasecmp(o->chance.s, "combat")) return CH_COMBAT;
    return CH_NUM;
}
static int opt_who(Enc *e, const Opt *o)
{
    if (o->who.s) return (int)eval_src(e, &o->who, 0, 0);
    return chance_mode(o) == CH_NUM ? leader() : -2;
}
static int pot_index(const Opt *o, int code) { for (int j = 0; j < o->npot; j++) if (o->pot_code[j] == code) return j; return -1; }
static Opt *potion_opt(Sec *s) { for (int k = 0; k < NOPT; k++) if (s->opt[k].defined && s->opt[k].kind == 1) return &s->opt[k]; return 0; }

/* the engine asks an option for its chance (Shift over the line, or the chances the interface shows) */
static void opt_handler(int k)
{
    int a = -1, b = -2, who = rt.who;
    Sec *s = cur_sec();
    Opt *o = s && k < NOPT ? &s->opt[k] : 0;
    if (!o || !o->defined) { dcopy(V_MSG_BUF, S_UNKNOWN); }
    else if (o->kind == 1) dcopy(V_MSG_BUF, S_SEL_POTION);
    else if (o->kind == 2) dcopy(V_MSG_BUF, S_SEL_SAINT);
    else {
        Enc *e = &encs[rt.enc];
        b = opt_who(e, o); rt.who = b;
        switch (chance_mode(o)) {
        case CH_NONE: dcopy(V_MSG_BUF, S_NO_RISK); break;
        case CH_ASSURED: dcopy(V_MSG_BUF, S_ASSURED); break;
        case CH_UNKNOWN: dcopy(V_MSG_BUF, S_UNKNOWN); break;
        case CH_COMBAT: dcopy(V_MSG_BUF, S_COMBAT); break;
        default: a = (int)eval_src(e, &o->chance, 0, 0); if (a < 0) a = 0; if (a > 100) a = 100;
        }
    }
    rt.who = who;
    AX = (u16)a; DX = (u16)b;
}
static void item_handler(int ms)
{
    int m = ms / 4, sl = ms % 4, a = -1, who = rt.who, q = rt.q;
    Sec *s = cur_sec();
    Opt *o = s ? potion_opt(s) : 0;
    int code = s ? s->items[sl] : -1, j = o && code >= 0 ? pot_index(o, code) : -1;
    if (j < 0) dcopy(V_MSG_BUF, S_POTION_OK);
    else {
        rt.who = m; rt.q = g_item_quality(m, code);
        a = (int)eval_src(&encs[rt.enc], &o->pot[j], 0, 0); if (a < 0) a = 0; if (a > 100) a = 100;
    }
    rt.who = who; rt.q = q;
    AX = (u16)a; DX = (u16)m;
}
static void saint_handler(int ms)
{
    int m = ms / 4, sl = ms % 4;
    Sec *s = cur_sec();
    int saint = s ? s->saints[sl] : -1;
    g_wb(V_MSG_BUF, 0);
    if (saint >= 0) g_chance_text(g_saint_lo(m, saint), g_saint_hi(m, saint));
    AX = 0xffff; DX = (u16)m;
}

static int opt_flag(Enc *e, Sec *s, int k)
{
    Opt *o = &s->opt[k];
    if (!o->defined) return 0;
    if (rt.flag[k] >= 0) return rt.flag[k];
    if (o->only.s && !eval_src(e, &o->only, 0, 0)) return 0;
    int f = 1;
    if (o->need.s && !eval_src(e, &o->need, 0, 0)) f = 2;
    if (o->kind == 1 && g_items_usable() != 1) f = 2;
    if (o->kind == 2 && g_saints_usable() != 1) f = 2;
    return f;
}

static void show_sec(Enc *e, Sec *s, int first)
{
    if (s->pic[0]) { dset_pic(V_PIC_BUF2, s->pic); g_card_load_near(); }
    else if (!first) { dcopy(V_PIC_BUF2, V_PIC_BUF); g_card_load_near(); }
    (void)e;
}

/* one round of the menu of section s: the option chosen, its actions run */
static void menu_round(Enc *e, Sec *s)
{
    for (int i = 0; i < 4; i++) { g_ww((u16)(V_CARD_ITEMS + 2 * i), (u16)s->items[i]); g_ww((u16)(V_CARD_SAINTS + 2 * i), (u16)s->saints[i]); }
    for (int m = 0; m < 5; m++) for (int i = 0; i < 4; i++) {
        u32 fi = native_far_ptr(item_handler, m * 4 + i), fs = native_far_ptr(saint_handler, m * 4 + i);
        u16 o = (u16)((m * 4 + i) * 4);
        g_ww((u16)(V_ITEM_FN + o), (u16)fi); g_ww((u16)(V_ITEM_FN + o + 2), (u16)(fi >> 16));
        g_ww((u16)(V_SAINT_FN + o), (u16)fs); g_ww((u16)(V_SAINT_FN + o + 2), (u16)(fs >> 16));
    }
    for (int k = 0; k < NOPT; k++) {
        int f = opt_flag(e, s, k);
        g_ww((u16)(V_OPT_FLAG + 2 * k), (u16)f);
        g_wb((u16)(V_OPT_ON + k), (u8)f);
        u32 fp = native_far_ptr(opt_handler, k);
        g_ww((u16)(V_OPT_FN + 4 * k), (u16)fp); g_ww((u16)(V_OPT_FN + 4 * k + 2), (u16)(fp >> 16));
    }
    if (aborting) return;
    if (enc_log_on()) { char f[NOPT + 1]; for (int k = 0; k < NOPT; k++) f[k] = (char)('0' + g_rb((u16)(V_OPT_ON + k))); f[NOPT] = 0; LOG("menu [%s] block %d, options %s\n", s->name, s->block, f); }
    g_ww(V_EE70, 0); g_ww(V_EE72, 7);
    g_ww(V_OPT_BLOCK, (u16)s->block);
    g_mouse_show();
    g_card_options(s->block);
    int k = g_card_menu();
    g_mouse_hide();
    if (aborting) return;
    int popup = (s16)g_rw(V_MENU_POPUP), m = (s16)g_rw(V_MENU_MEMBER), sl = (s16)g_rw(V_MENU_SLOT);
    if (k < 0 || k >= NOPT || !s->opt[k].defined) { LOG("menu: option %d has no actions\n", k); return; }
    Opt *o = &s->opt[k];
    int ok = 1;
    rt.cur_opt = k;
    if (o->kind == 2) {                          /* a saint */
        if (!popup || m < 0 || m > 4 || sl < 0 || sl > 3 || s->saints[sl] < 0) return;
        rt.who = m; rt.saint = s->saints[sl];
        g_ww(V_CHOSEN1, (u16)m);
        g_name_saint(rt.saint);
        int r = g_invoke_saint(m, rt.saint);
        LOG("saint %d by %d: %d\n", rt.saint, m, r);
        if (aborting || r < 0) return;           /* the prayer was called off */
        ok = r > 0;
    } else if (o->kind == 1) {                   /* a potion */
        if (!popup || m < 0 || m > 4 || sl < 0 || sl > 3 || s->items[sl] < 0) return;
        rt.who = m; rt.item = s->items[sl];
        rt.q = g_item_quality(m, rt.item);
        g_ww(V_CHOSEN1, (u16)m); g_ww(V_ITEM, (u16)rt.item);
        int j = pot_index(o, rt.item);
        int c = j < 0 ? 100 : (int)eval_src(e, &o->pot[j], 0, 0);
        g_take_item(m, rt.item);
        ok = g_rand(100) <= c;
        LOG("potion %d (q %d) by %d: chance %d -> %d\n", rt.item, rt.q, m, c, ok);
    } else {
        rt.who = opt_who(e, o);
        if (chance_mode(o) == CH_NUM) {
            int c = (int)eval_src(e, &o->chance, 0, 0);
            ok = g_rand(100) <= c;
            LOG("option %d: chance %d -> %d\n", k, c, ok);
        }
    }
    if (aborting) return;
    if (run_src(e, ok ? &o->ok : &o->fail, 0) == 2 && !aborting && !rt.jumped && (s16)g_rw(V_NEXT_CARD) == rt.card) script_error(e);
}

static int run_encounter(int card)
{
    int sl = card - ENC_BASE;
    reload();
    int i = sl >= 0 && sl < ENC_MAX ? by_slot[sl] : -1;
    if (i < 0 || !encs[i].ok) { fprintf(stderr, "encounters: card %#x is no encounter in use: back to the map\n", card); exit_to_map(); return 0x0c; }
    Enc *e = &encs[i];
    memset(&rt, 0, sizeof rt);
    for (int k = 0; k < NOPT; k++) rt.flag[k] = -1;
    rt.active = 1; rt.enc = i; rt.card = card; rt.sec = 0; rt.last_menu = -1;
    rt.who = leader(); rt.result = -1;
    LOG("start %s (card %#x)\n", e->id, card);
    e->last_day = game_day();
    g_ww(V_NEXT_CARD, (u16)card);
    g_ww(V_CHOSEN1, (u16)leader());
    dset(V_CARD_NAME, e->card, 13);
    dset_pic(V_PIC_BUF, e->pic);
    if (e->music) g_music();
    int first = 1, entered = 1, hops = 0;
    while (!aborting && (s16)g_rw(V_NEXT_CARD) == card) {
        if (entered && ++hops > 200) {          /* sections that only lead to one another, never to a menu */
            fprintf(stderr, "encounters: %s: the sections go round in a circle without a menu\n", e->file);
            script_error(e); break;
        }
        Sec *s = &e->sec[rt.sec];
        if (entered) {
            entered = 0;
            if (first) { if (s->pic[0]) dset_pic(V_PIC_BUF, s->pic); g_card_load(); }
            else if (s->block >= 0 || s->pic[0]) show_sec(e, s, 0);
            first = 0;
            if (s->chosen.s) g_ww(V_CHOSEN1, (u16)resolve_who((int)eval_src(e, &s->chosen, 0, 0)));
            rt.jumped = 0;
            int r = run_src(e, &s->enter, 0);
            if (aborting || (s16)g_rw(V_NEXT_CARD) != card) break;
            if (rt.jumped) { entered = 1; continue; }
            if (r == 2) { script_error(e); break; }   /* an error in the actions: leave */
            if (r) { exit_to_map(); break; }
            if (s->block < 0) {                  /* actions only: back to the last menu, or the map */
                if (rt.last_menu < 0) { exit_to_map(); break; }
                rt.sec = rt.last_menu; entered = 1;
                continue;
            }
            rt.last_menu = rt.sec;
        }
        rt.jumped = 0;
        hops = 0;
        menu_round(e, s);
        if (rt.jumped) entered = 1;
    }
    rt.active = 0;
    if (aborting) return 0;
    int next = (s16)g_rw(V_NEXT_CARD);
    LOG("end %s -> card %#x\n", e->id, next);
    return next;
}

static void card_entry(int arg)
{
    (void)arg;
    aborting = 0;
    int r = run_encounter((s16)g_rw(V_CUR_CARD));
    if (aborting) return;                        /* native_call passes the unwinding on */
    AX = (u16)r; DX = 0;
}

static void install_cards(void)
{
    u32 fp = native_far_ptr(card_entry, 0);
    for (int i = 0; i < ENC_MAX; i++) {
        u16 o = (u16)((ENC_BASE + i) * 4);
        ww(CARD_TABLE, o, (u16)fp); ww(CARD_TABLE, (u16)(o + 2), (u16)(fp >> 16));
    }
}

/* ================================================================ choosing one */
static const int REGION_CARDS[] = { 0x112, 0x117, 0x116, 0x11b, 0xa0, 0x102, 0x103, 0x111, 0x122, 0xa5, 0x107, 0x118, 0x106,
    0x7f, 0x104, 0x105, 0x108, 0x24, 0xf0, 0xa6, 0xa7, 0x3d, 0xc3, 0x163, 0x113 };
/* entries of the game's table for a terrain (s030 1462:02c0): a new encounter of weight w
 * counts as w more entries */
static int table_size(int t)
{
    switch (t) {
    case 4: case 22: case 23: return 9;
    case 5: case 6: case 7: case 10: case 11: return 17;
    case 8: case 9: return 26;
    case 12: case 13: case 14: case 15: case 16: case 17: return 16;
    case 18: case 19: case 20: case 21: return 15;
    case 24: return 29;
    default: return 0;
    }
}

int encounters_offered(void)
{
    if (config_get_int("original", 0) || !config_get_int("encounters.new", 1)) return 0;
    reload();
    for (int i = 0; i < nenc; i++) if (encs[i].ok) return 1;
    return 0;
}

static int eligible(Enc *e, int t, long today)
{
    if (!e->ok || e->weight <= 0) return 0;
    if (t < 0 || t > 31 || !(e->terrain >> t & 1)) return 0;
    if (today - e->last_day < 4) return 0;           /* not the same one again within a few days */
    if (e->when) { int d = g_daylight() != 0; if ((e->when == 1) != d) return 0; }
    if (e->only.s) { memset(&rt, 0, sizeof rt); rt.who = leader(); if (!eval_src(e, &e->only, 0, 0)) return 0; }
    return 1;
}

static void overland_hook(void)                 /* s030 1462:02c0 overland_loop(): the next card */
{
    travel_step_result = -1;
    hook_call_original();
    if (unw_active) return;
    int r = (s16)AX, t = (s16)g_rw(V_TERRAIN);
    int region = 0;
    for (int i = 0; i < (int)(sizeof REGION_CARDS / sizeof REGION_CARDS[0]); i++) if (REGION_CARDS[i] == r) region = 1;
    if (travel_step_result != 1000 || !region) return;
    if (!encounters_offered()) return;
    install_cards();
    aborting = 0;
    long today = game_day();
    int pick = -1;
    if (force_next >= 0 && force_next < nenc && encs[force_next].ok) { pick = force_next; force_next = -1; }
    else {
        int share = config_get_int("encounters.share", 100);
        if (share <= 0) return;
        int T = table_size(t);
        if (!T) return;
        long w[ENC_MAX], W = 0;
        for (int i = 0; i < nenc; i++) { w[i] = eligible(&encs[i], t, today) ? (long)encs[i].weight * share : 0; W += w[i]; }
        if (aborting || !W) return;
        long roll = rand() % (T * 100L + W);
        LOG("terrain %d: table %d, new weight %ld/100, roll %ld\n", t, T, W, roll);
        if (roll >= W) return;
        for (int i = 0; i < nenc; i++) { if (roll < w[i]) { pick = i; break; } roll -= w[i]; }
    }
    if (pick < 0) return;
    LOG("%s instead of card %#x\n", encs[pick].id, r);
    AX = (u16)(ENC_BASE + encs[pick].slot);
}

/* ================================================================ interface */
int encounters_count(void) { reload(); int n = 0; for (int i = 0; i < nenc; i++) n += encs[i].ok; return n; }
int encounters_errors(void) { reload(); return nerr; }
const char *encounters_title(int i) { return i >= 0 && i < nenc ? encs[i].title : ""; }
int encounters_card_active(void) { return rt.active ? rt.enc + 1 : 0; }
void encounters_force(int i)
{
    reload();
    if (i < 0 || i >= nenc) { fprintf(stderr, "encounters: no encounter %d (%d loaded)\n", i, nenc); return; }
    force_next = i;
    fprintf(stderr, "encounters: the next wilderness encounter is %s\n", encs[i].id);
}
int encounters_force_pending(void) { return force_next >= 0; }

void encounters_init(void)
{
    hook_install(30, 0x1462, 0x02c0, overland_hook);
    install_cards();                             /* also for a save made during a new encounter */
}
