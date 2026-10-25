/* game.h - native access to the game's state in emulated memory (for mods and the new UI).
 * Addresses are link-time segment:offset values from the decompilation
 * (include/darklands/card.h, game.h); see docs/ENGINE.md for what they mean. */
#ifndef DL_GAME_H
#define DL_GAME_H
#include "cpu.h"
#ifdef __cplusplus
extern "C" {
#endif

#define G_DGROUP     ((u16)(LOADSEG + 0x321a))      /* the game's DS */
#define G_FAR2E38    ((u16)(LOADSEG + 0x2e38))      /* far data: inventories, item tables */
#ifndef G_FAR20A5
#define G_FAR20A5    ((u16)(LOADSEG + 0x20a5))      /* far data: weapon tables, colours */
#endif

/* party member records: 5 x 128 bytes at DS:9c69 */
#define G_PARTY        0x9c69
#define G_MEMBER(m)    ((u16)(G_PARTY + (m) * 0x80))
#define G_PARTY_SIZE   0xa67e    /* int: number of party members (card.h party_size) */
#define G_LEADER       0x907b    /* char: active member */
#define G_CUR_LOC      0x907e    /* int: current location (< 0x5c city), -1 wilderness */
#define G_DIFFICULTY   0x906a    /* char: 0 basic, 1 standard, 2 expert (low bits) */
#define G_PARTY_SEED   0x9c4a    /* int: fixed per game (shop stock seed) */
#define G_M_FULLNAME   17        /* char[25] */
#define G_M_SHORTNAME  42        /* char[?] */
#define G_M_STATUS     0x00      /* 1 = able */
#define G_M_ATTR_CUR   0x49      /* END STR AGL PER INT CHR DF: current */
#define G_M_ATTR_MAX   0x50      /* their maxima */
#define G_M_LOAD       0x35      /* weight of the equipped items (capacity END + STR) */
#define G_M_SKILLS     0x57      /* 19 skills */
#define G_M_NITEMS     0x6a      /* number of inventory records */
#define G_M_MISSILE    0x0e      /* char: the missile weapon's type (-1 none) */
#define G_M_ARMOR_BODY 0x37      /* char: the body armor's type (-1 none) */
#define G_M_ARMOR_LEGS 0x38      /* char: the leg armor's type (-1 none) */
#define G_M_MELEE      0x3d      /* char: the melee weapon's type (-1 none) */
#define G_M_SHIELD     0x48      /* char: the shield's type (-1 none) */

/* a combatant's record in a battle: 13 x 128 bytes at DS:9c55 (0..4 the party, 5..12 the foes),
   the fields named here as the code reads them; the rest stay numbers where they are used */
#define CB_REC(i)      ((u16)(0x9c55 + (i) * 0x80))
#define CBF(i, f)      ((u16)(CB_REC(i) + (f)))
#define CB_X           0x04      /* int: map x (pixels) */
#define CB_Y           0x06      /* int: map y */
#define CB_TO_X        0x08      /* int: where it walks, x */
#define CB_TO_Y        0x0a      /* int: where it walks, y */
#define CB_ORDER       0x10      /* int: the order (1 walk, 0x80 shoot, 0x12 Std Attack ...) */
#define CB_STATUS      0x14      /* char: 1 standing, 2 unconscious, 3 dead */
#define CB_MELEE_RATING 0x20     /* byte: both ways hand to hand */
#define CB_AIM         0x21      /* byte: for a shot or a throw */
#define CB_MISSILE     0x22      /* char: the missile weapon's type (-1 none) */
#define CB_NAME        0x25      /* char[]: a party member's name */
#define CB_TARGET      0x4a      /* char: the combatant it is ordered against (-1 none) */
#define CB_BODY_ARMOR  0x4b      /* byte: the body armor's type (0xff none) */
#define CB_MELEE       0x51      /* char: the melee weapon's type, as the member record's G_M_MELEE (-1 none) */
#define CB_END         0x5d      /* char: Endurance now */
#define CB_STR         0x5e      /* char: Strength now */
#define CB_END_MAX     0x64      /* char: Endurance at most */
#define CB_STR_MAX     0x65      /* char: Strength at most */
#define CB_NITEMS      0x7e      /* char: number of item records */

/* purse: florins, groschen, pfennigs (DS:906b) */
#define G_MONEY        0x906b

/* inventories: 2e38:003e, [member][64] records of 6 bytes, 0x180 per member */
#define G_INV          0x003e
#define G_ITEM(m, i)   ((u16)(G_INV + (m) * 0x180 + (i) * 6))
/* item definitions: far pointers at 2e38:0890 */
#define G_ITEMDEFS     0x0890

static inline u8 g_rb(u16 off) { return rb(G_DGROUP, off); }
static inline u16 g_rw(u16 off) { return rw(G_DGROUP, off); }
static inline void g_wb(u16 off, u8 v) { wb(G_DGROUP, off, v); }
static inline void g_ww(u16 off, u16 v) { ww(G_DGROUP, off, v); }

/* copy a 0-terminated string from emulated memory */
static inline void g_str(u16 seg, u16 off, char *out, int n)
{
    int i = 0;
    for (; i < n - 1; i++) { char c = (char)rb(seg, (u16)(off + i)); if (!c) break; out[i] = c; }
    out[i] = 0;
}

/* item definition (name, weight ...) of item id: returns seg:off of the record, 0 if none */
static inline u32 g_itemdef(int id)
{
    u16 o = rw(G_FAR2E38, (u16)(G_ITEMDEFS + id * 4)), s = rw(G_FAR2E38, (u16)(G_ITEMDEFS + id * 4 + 2));
    if (!s) return 0;
    return ((u32)s << 16) | o;
}

/* what the game is doing now (from the resident overlays) */
enum { GM_OTHER, GM_CARD, GM_MAP, GM_SCREEN, GM_BATTLE };
int game_mode(void);

/* evaluate game code without side effects: snapshot, call, restore */
void *eval_snapshot(void);
void eval_restore(void *snap);
void eval_free(void *snap);
u32 game_call_ptr(u16 seg, u16 off, int nargs, const u16 *args, int fill);   /* DX:AX; no restore */
u32 game_call_link(int sect, u16 seg, u16 off, int nargs, const u16 *args);   /* by link address; no restore */
extern int game_call_timed_out;   /* the last of these calls waited (for a key ...) and was abandoned */
#define G_CUR_HOUR  0x00e0
#define G_CUR_DAY   0x00e2       /* 1-based (1..days of the month, date_add_hours) */
#define G_CUR_MONTH 0x00e4       /* 0-11 */
#define G_CUR_YEAR  0x00e6

/* location records (DARKLAND.LOC, 0x19e of them; cities first, index = city index):
 * far pointers at far data :10be. Record: +0 icon/type (0 city, 1 castle, 3 monastery, 6 mine,
 * 8 village ...), +4 x, +6 y (map tile), +0x26 name[20] */
#define G_NLOC 0x19e
static inline u32 g_location(int i)
{
    u16 seg = g_rw(0x7e40); if (!seg) seg = G_FAR2E38;
    if (i < 0 || i >= G_NLOC) return 0;
    u16 o = rw(seg, (u16)(0x10be + i * 4)), s = rw(seg, (u16)(0x10be + i * 4 + 2));
    return s ? ((u32)s << 16) | o : 0;
}

/* the events (the game's timed facts: tasks and their rewards, blessings, potions, teachers,
 * Sabbats ...): 300 far pointers at far data :1b2c (2e38:1b2c) to 0x30-byte records, 0 = free.
 * Record: +0 type0, +0x0a the start and +0x12 the end (hour, day, month, year), +0x1a owner or
 * member, +0x1c location, +0x1e value, +0x22 kind, +0x28 tag, +0x2a aux, +0x2c, +0x2e item */
#define G_NEVENTS 300
#define G_EVENTS  0x1b2c
#define G_EV_TYPE0 0x00
#define G_EV_START 0x0a
#define G_EV_END   0x12
#define G_EV_OWNER 0x1a
#define G_EV_LOC   0x1c
#define G_EV_VALUE 0x1e
#define G_EV_KIND  0x22
#define G_EV_TAG   0x28
#define G_EV_AUX   0x2a
#define G_EV_AUX2  0x2c
#define G_EV_ITEM  0x2e
static inline u32 g_event(int i)                    /* seg:off of event i, 0 if the slot is free */
{
    if (i < 0 || i >= G_NEVENTS) return 0;
    u16 o = rw(G_FAR2E38, (u16)(G_EVENTS + i * 4)), s = rw(G_FAR2E38, (u16)(G_EVENTS + i * 4 + 2));
    return s ? ((u32)s << 16) | o : 0;
}
static inline int g_ev(u32 e, u16 field) { return (s16)rw((u16)(e >> 16), (u16)((u16)e + field)); }        /* a word of event record e */
static inline void g_ev_set(u32 e, u16 field, int v) { ww((u16)(e >> 16), (u16)((u16)e + field), (u16)v); }

/* direct calls from hooks (registers restored, memory effects kept) */
void (*game_func(int sect, u16 seg, u16 off))(void);           /* sect -1 = root */
u32 game_call_native(void (*f)(void), int nargs, const u16 *args);
u32 game_call_live(void (*f)(void), int nargs, const u16 *args);     /* the same, the game running normally inside */

/* call a far game function (link address sect seg:off) with cdecl word args; returns AX */
int game_call_far(int sect, u16 seg, u16 off, int nargs, const u16 *args);
#ifdef __cplusplus
}
#endif
#endif
