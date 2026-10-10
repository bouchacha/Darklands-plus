/* game.h - native access to the game's state in emulated memory (for mods and the new UI).
 * Addresses are link-time segment:offset values from the decompilation
 * (include/darklands/card.h, game.h); see docs/ENGINE.md for what they mean. */
#ifndef DL_GAME_H
#define DL_GAME_H
#include "cpu.h"

#define G_DGROUP     ((u16)(LOADSEG + 0x321a))      /* the game's DS */
#define G_FAR2E38    ((u16)(LOADSEG + 0x2e38))      /* far data: inventories, item tables */

/* party member records: 5 x 128 bytes at DS:9c69 */
#define G_PARTY        0x9c69
#define G_MEMBER(m)    ((u16)(G_PARTY + (m) * 0x80))
#define G_PARTY_SIZE   0x9c44                         /* not verified: see party_size */
#define G_M_FULLNAME   17        /* char[25] */
#define G_M_SHORTNAME  42        /* char[?] */
#define G_M_ATTR_MAX   0x49      /* END STR AGL PER INT CHR */
#define G_M_ATTR_CUR   0x50
#define G_M_SKILLS     0x57      /* 19 skills */
#define G_M_NITEMS     0x6a      /* number of inventory records */

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
#endif
