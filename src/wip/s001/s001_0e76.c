/* place: s001 0e76:0000 */
/* orig: size=09d2 CONST=7d18+000a _DATA=0e30+007f */
/* PLAY-TEST (2026-10-09): in a --functional build Quickstart gives every member three "Hand Axe 00q (0)" instead of the default equipment: the item records written here are wrong somewhere. */
/* e8 (2026-10-09): every item-record store was compared with build/asm/s001/s001_0e76.asm (full aligned diff,
 * build/agent/e1/fdiff.py): same offsets (inventory 2e38:003e, 64 x 6-byte records per member, stride 0x180;
 * id word +0, f2 +2, f3 +3, f4 +4, weight +5; counts at member +0x6a = 3/3/8/3), same values, same CONST
 * slots (7d18 0890, 7d1a 082c, 7d1c 07be, 7d1e 003e, 7d20 20a5:33fc). The only differences were register
 * choice (dl/cl) and 6.00A re-reading F_2e38_0890[43]->weight through a cached pointer, which made the
 * function 16 bytes LONGER than the original (0x9e2 > 0x9d2 + 14-byte tail) and forced a relayout of the
 * s001 overlay. Now `[1][1].f5 = [0][1].f5` (same value: the weight of item 43 stored just before): the
 * function is 2512 bytes, fits the original range, no relayout. Needs a re-play-test before it leaves EXCLUDE. */
/* e9 (2026-10-09) re-verification: all 421 memory stores of the compiled function were compared
 * in order with the original (build/agent/e9/stores.py: destination incl. segment slot, and the
 * stored value with register contents tracked back to their immediate/memory source): identical
 * destinations and values - item ids (5, 0x2b, 0x33, ...), quality bytes (3, 0x4a, 0x52, 0x50,
 * 0x0f, ...), weights (itemdef[id]->weight via 2e38:0890, e.g. [1][1].f5 = weight of item 0x2b
 * like the original's AL), counts at member +0x6a, record layout (6-byte records, stride 0x180
 * per member at 2e38:003e) and the darkland.lst pointer table.  The full instruction diff is
 * only register choice (CL vs AL, 3 hunks) and the 2-byte shorter code; fmatch: data_ok, CONST
 * at 7d18 (5 segment slots in the original order).  No wrong store found: the "Hand Axe 00q"
 * play-test result must come from outside this function (e.g. link-time resolution of the
 * CONST segment slots / far data shift in the functional build, or another draft that
 * overwrites or reads the inventory) - nothing changed here. */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* QA2 (2026-10-09): re-play-tested: Quickstart equipment is correct with only this draft and in the full --functional build. The 'Hand Axe 00q' result came from src/wip/s025 overwriting this module's CONST slots 7d1c-7d20 (wrong CONST stamp there); removed from src/wip/EXCLUDE. */
/* Overlay s001: set up the default (quick-start) party: four members with names,
   attributes, skills and starting equipment, and load the item definitions from
   darkland.lst when they are not loaded yet (D_11dd == 1). */
#include "darklands/card.h"
#include "darklands/game.h"
struct find_t {
    char reserved[21];
    char attrib;
    unsigned wr_time;
    unsigned wr_date;
    long size;
    char name[13];
};
struct item { unsigned id; unsigned char f2; unsigned char f3; unsigned char f4; unsigned char f5; };
#define FP_SEG(fp) (*((unsigned _far *)&(fp) + 1))
#define MK_FP(seg, off) ((unsigned char far *)(((unsigned long)(seg) << 16) | (off)))
char *strcpy(char *, const char *);
int _dos_findfirst(char *, unsigned, struct find_t *);
void far file_load_far(char *, void far *);
unsigned far alloc_buffer(unsigned long, char *);
void far free_buffer(unsigned, char *);
extern char D_11dd;
extern unsigned char far *lst_buf;     /* item definition file buffer */
extern int party_size;                    /* number of party members */
extern char leader;
extern char party[][0x80];  /* party member records */
struct itemdef { char name[0x25]; unsigned char weight; };
extern struct itemdef far * far item_defs[];   /* item definitions (46 bytes each) */
extern unsigned char far saints_known[][0x14];
extern unsigned char far formulas_known[][0x16];
extern struct item far inventories[][64];       /* inventories */
extern unsigned char far member_palette[];
#define VB(x) (*(volatile unsigned char *)&(x))
#define VW(x) (*(volatile int *)&(x))
#define VFB(x) (*(volatile unsigned char far *)&(x))
#define VFW(x) (*(volatile int far *)&(x))

#pragma optimize("a", on)
#pragma optimize("l", off)
void far party_default(void)
{
    struct find_t ft;
    unsigned cnt;
    unsigned k;
    unsigned char far *p;
    register int si, di;
    int a, x;      /* unused: the original compiler gave its register variables stack slots */

    if (D_11dd == 1) {
        _dos_findfirst("darkland.lst", 0, &ft);
        lst_buf = p = MK_FP(alloc_buffer(ft.size, "Items"), 0);
        file_load_far("darkland.lst", p);
        cnt = *p;
        p++;
        p += 2;
        for (k = 0; k < cnt; k++) {
            item_defs[k] = (struct itemdef far *)p;
            p += 0x2e;
        }
    }
    VW(party_size) = 0x4;
    party[0][0] = 0x1;
    party[1][0] = 0x1;
    party[2][0] = 0x1;
    party[3][0] = 0x1;
    leader = 0;
    party[4][0] = 0;
    strcpy(&party[0][17], "Gretchen Wilburg");
    strcpy(&party[0][42], "Gretch");
    party[0][6] = 0;
    party[0][5] = 0;
    VB(party[0][82]) = 0x1d;
    VB(party[0][83]) = 0x12;
    VB(party[0][84]) = 0x1f;
    VB(party[0][85]) = 0x20;
    VB(party[0][86]) = 0x63;
    VB(party[0][87]) = 0x23;
    VB(party[0][88]) = 0x14;
    VB(party[0][90]) = 0x10;
    party[0][80] = 0x1e;
    party[0][81] = 0x1e;
    party[0][92] = 0x1e;
    party[0][89] = 0x6;
    party[0][94] = 0x6;
    VB(party[0][97]) = 0x28;
    VB(party[0][100]) = 0x3;
    VB(party[0][101]) = 0x2;
    VB(party[0][102]) = 0x15;
    VB(party[0][103]) = 0xb;
    party[0][93] = 0xd;
    party[0][95] = 0xd;
    party[0][104] = 0xd;
    party[0][91] = 0x4;
    party[0][98] = 0x4;
    party[0][99] = 0x4;
    party[0][105] = 0x4;
    party[0][96] = 0x19;
    party[0][59] = 0x19;
    party[0][60] = 0x19;
    strcpy(&party[1][17], "Gunther Langer");
    strcpy(&party[1][42], "Gunther");
    party[1][6] = 0;
    party[1][5] = 0;
    VB(party[1][80]) = 0x27;
    VB(party[1][81]) = 0x28;
    VB(party[1][83]) = 0x18;
    VB(party[1][84]) = 0x10;
    VB(party[1][85]) = 0xd;
    VB(party[1][86]) = 0x63;
    VB(party[1][88]) = 0x2f;
    VB(party[1][90]) = 0x20;
    VB(party[1][91]) = 0x14;
    VB(party[1][92]) = 0xf;
    VB(party[1][93]) = 0x1f;
    VB(party[1][95]) = 0x9;
    party[1][94] = 0x2;
    party[1][98] = 0x2;
    VB(party[1][99]) = 0x5;
    VB(party[1][100]) = 0xc;
    VB(party[1][101]) = 0x7;
    party[1][82] = 0x1a;
    party[1][102] = 0x1a;
    party[1][96] = 0x13;
    party[1][103] = 0x13;
    party[1][89] = 0xb;
    party[1][97] = 0xb;
    party[1][104] = 0xb;
    party[1][87] = 0x1d;
    party[1][105] = 0x1d;
    party[1][59] = 0x19;
    party[1][60] = 0x19;
    strcpy(&party[2][17], "Hans Muller");
    strcpy(&party[2][42], "Hans");
    party[2][6] = 0;
    party[2][5] = 0;
    VB(party[2][81]) = 0x19;
    VB(party[2][82]) = 0x1c;
    VB(party[2][84]) = 0x2b;
    VB(party[2][85]) = 0x10;
    VB(party[2][86]) = 0x63;
    VB(party[2][87]) = 0xb;
    VB(party[2][90]) = 0x6;
    party[2][89] = 0x3;
    party[2][92] = 0x3;
    party[2][83] = 0x14;
    party[2][93] = 0x14;
    VB(party[2][94]) = 0x30;
    VB(party[2][95]) = 0xa;
    party[2][88] = 0x13;
    party[2][91] = 0x13;
    party[2][96] = 0x13;
    party[2][80] = 0x1b;
    party[2][97] = 0x1b;
    VB(party[2][98]) = 0x17;
    VB(party[2][99]) = 0x20;
    VB(party[2][101]) = 0x21;
    party[2][100] = 0x8;
    party[2][102] = 0x8;
    VB(party[2][103]) = 0xe;
    VB(party[2][104]) = 0x4;
    VB(party[2][105]) = 0x7;
    party[2][59] = 0x19;
    party[2][60] = 0x19;
    strcpy(&party[3][17], "Ebhard of Achdorf");
    strcpy(&party[3][42], "Ebhard");
    party[3][6] = 0;
    party[3][5] = 0;
    VB(party[3][80]) = 0x1a;
    VB(party[3][82]) = 0x18;
    VB(party[3][83]) = 0x1c;
    VB(party[3][84]) = 0x1f;
    VB(party[3][85]) = 0x1d;
    VB(party[3][86]) = 0x63;
    VB(party[3][87]) = 0xa;
    VB(party[3][88]) = 0x14;
    VB(party[3][89]) = 0x22;
    VB(party[3][90]) = 0x9;
    VB(party[3][92]) = 0x4;
    VB(party[3][93]) = 0x1b;
    VB(party[3][96]) = 0x34;
    VB(party[3][97]) = 0x13;
    party[3][98] = 0x17;
    party[3][99] = 0x17;
    party[3][91] = 0x8;
    party[3][94] = 0x8;
    party[3][101] = 0x8;
    VB(party[3][102]) = 0xc;
    VB(party[3][103]) = 0x7;
    VB(party[3][104]) = 0x5;
    VB(party[3][105]) = 0x3;
    party[3][81] = 0x19;
    party[3][59] = 0x19;
    party[3][60] = 0x19;
    VW(party[0][-2]) = 0x23;
    *(int *)&party[1][-2] = 0x28;
    *(int *)&party[2][-2] = 0x28;
    VW(party[3][-2]) = 0x2d;
    VB(party[0][53]) = 0x39;
    party[3][95] = 0x28;
    party[3][100] = 0x28;
    party[1][53] = 0x28;
    VB(party[2][53]) = 0x24;
    VB(party[3][53]) = 0x3a;
    for (si = 0; si < 5; si++) {
        for (di = 0; di < 0x14; di++)
            saints_known[si][di] = 0;
        for (di = 0; di < 0x16; di++)
            formulas_known[si][di] = 0;
    }
    VFB(saints_known[3][0]) = 0x10;
    formulas_known[2][7] = 0x1;
    saints_known[3][2] = 0x1;
    formulas_known[2][0] = 0x2;
    formulas_known[2][9] = 0x2;
    formulas_known[2][20] = 0x2;
    saints_known[3][5] = 0x2;
    VFB(saints_known[3][6]) = 0x20;
    VFB(saints_known[3][14]) = 0x40;
    for (si = 0; si < 13; si++) {
        party[si][1] = si + 'A';
        for (di = 0; di < 7; di++)
            party[si][0x49 + di] = party[si][0x50 + di];
    }
    VB(party[0][3]) = 0x1;
    VB(party[0][56]) = 0x52;
    VFW(inventories[0][0].id) = 0x5;
    VFB(inventories[0][0].f2) = 0x3;
    inventories[0][0].f5 = item_defs[5]->weight;
    inventories[0][1].f5 = item_defs[43]->weight;
    VFW(inventories[0][2].id) = 0x33;
    VFB(inventories[0][2].f2) = 0x52;
    inventories[0][2].f5 = item_defs[51]->weight;
    VB(party[1][61]) = 0xf;
    party[0][55] = 0x4a;
    party[1][55] = 0x4a;
    VB(party[1][56]) = 0x50;
    VFW(inventories[1][0].id) = 0xf;
    VFB(inventories[1][0].f2) = 0xf;
    inventories[1][0].f5 = item_defs[15]->weight;
    inventories[0][1].id = 0x2b;
    inventories[1][1].id = 0x2b;
    inventories[0][1].f2 = 0x4a;
    inventories[1][1].f2 = 0x4a;
    inventories[1][1].f5 = inventories[0][1].f5;   /* = weight of item 43, stored above */
    VFW(inventories[1][2].id) = 0x31;
    VFB(inventories[1][2].f2) = 0x50;
    inventories[1][2].f5 = item_defs[49]->weight;
    party[0][61] = 0x3;
    party[0][106] = 0x3;
    party[1][106] = 0x3;
    VB(party[2][14]) = 0x1f;
    party[0][14] = 0xff;
    party[0][72] = 0xff;
    party[1][14] = 0xff;
    party[1][72] = 0xff;
    party[2][61] = 0xff;
    party[2][72] = 0xff;
    party[0][68] = 0x19;
    party[0][70] = 0x19;
    party[0][71] = 0x19;
    party[1][68] = 0x19;
    party[2][68] = 0x19;
    party[2][70] = 0x19;
    party[2][71] = 0x19;
    VB(party[2][55]) = 0x49;
    VB(party[2][56]) = 0x4e;
    VFW(inventories[2][0].id) = 0x1e;
    VFB(inventories[2][0].f2) = 0x1f;
    inventories[2][0].f5 = item_defs[30]->weight;
    VFW(inventories[2][1].id) = 0x5f;
    VFB(inventories[2][1].f2) = 0x84;
    VFB(inventories[2][1].f3) = 0x14;
    inventories[2][1].f5 = item_defs[95]->weight;
    VFW(inventories[2][2].id) = 0x68;
    VFB(inventories[2][2].f2) = 0x8d;
    VFB(inventories[2][2].f3) = 0x2d;
    inventories[2][2].f5 = item_defs[104]->weight;
    VFW(inventories[2][3].id) = 0x66;
    VFB(inventories[2][3].f2) = 0x8b;
    inventories[2][3].f5 = item_defs[102]->weight;
    VFW(inventories[2][4].id) = 0x73;
    VFB(inventories[2][4].f2) = 0x98;
    VFB(inventories[2][4].f3) = 0x23;
    inventories[2][4].f5 = item_defs[115]->weight;
    VFW(inventories[2][5].id) = 0x2f;
    VFB(inventories[2][5].f2) = 0x4e;
    inventories[2][5].f5 = item_defs[47]->weight;
    VFW(inventories[2][6].id) = 0x2a;
    VFB(inventories[2][6].f2) = 0x49;
    inventories[0][0].f4 = 0x1;
    inventories[0][1].f4 = 0x1;
    inventories[0][2].f4 = 0x1;
    inventories[1][0].f4 = 0x1;
    inventories[1][1].f4 = 0x1;
    inventories[1][2].f4 = 0x1;
    inventories[2][0].f4 = 0x1;
    inventories[2][1].f4 = 0x1;
    inventories[2][2].f4 = 0x1;
    inventories[2][3].f4 = 0x1;
    inventories[2][4].f4 = 0x1;
    inventories[2][5].f4 = 0x1;
    inventories[2][6].f4 = 0x1;
    inventories[2][6].f5 = item_defs[42]->weight;
    VFW(inventories[2][7].id) = 0x22;
    VFB(inventories[2][7].f2) = 0x41;
    inventories[0][0].f3 = 0x19;
    inventories[0][1].f3 = 0x19;
    inventories[0][2].f3 = 0x19;
    inventories[1][0].f3 = 0x19;
    inventories[1][1].f3 = 0x19;
    inventories[1][2].f3 = 0x19;
    inventories[2][0].f3 = 0x19;
    inventories[2][3].f3 = 0x19;
    inventories[2][5].f3 = 0x19;
    inventories[2][6].f3 = 0x19;
    inventories[2][7].f3 = 0x19;
    VFB(inventories[2][7].f4) = 0x1e;
    inventories[2][7].f5 = item_defs[65]->weight;
    VB(party[2][106]) = 0x8;
    VB(party[3][61]) = 0x11;
    VB(party[3][14]) = 0xff;
    party[3][68] = 0x19;
    party[3][70] = 0x19;
    VB(party[3][72]) = 0xff;
    party[3][71] = 0x19;
    party[3][55] = 0x4a;
    VB(party[3][56]) = 0x52;
    VFW(inventories[3][0].id) = 0x11;
    VFB(inventories[3][0].f2) = 0x11;
    inventories[3][0].f5 = item_defs[17]->weight;
    VFW(inventories[3][1].id) = 0x2b;
    inventories[3][1].f2 = 0x4a;
    inventories[3][1].f5 = item_defs[43]->weight;
    VFW(inventories[3][2].id) = 0x33;
    VFB(inventories[3][2].f2) = 0x52;
    inventories[3][0].f3 = 0x19;
    inventories[3][1].f3 = 0x19;
    inventories[3][2].f3 = 0x19;
    inventories[3][0].f4 = 0x1;
    inventories[3][1].f4 = 0x1;
    inventories[3][2].f4 = 0x1;
    inventories[3][2].f5 = item_defs[51]->weight;
    VB(party[3][106]) = 0x3;
    if (D_11dd == 1)
        free_buffer(FP_SEG(lst_buf), "Items");
    VFB(member_palette[2]) = 0x19;
    member_palette[9] = 0x9;
    member_palette[10] = 0x9;
    member_palette[11] = 0x9;
    VFB(member_palette[15]) = 0x22;
    VFB(member_palette[21]) = 0x2e;
    VFB(member_palette[24]) = 0x39;
    VFB(member_palette[25]) = 0x36;
    VFB(member_palette[27]) = 0x3d;
    VFB(member_palette[28]) = 0x38;
    member_palette[1] = 0x28;
    member_palette[18] = 0x28;
    member_palette[32] = 0x28;
    member_palette[3] = 0x15;
    member_palette[4] = 0x15;
    member_palette[5] = 0x15;
    member_palette[29] = 0x15;
    member_palette[37] = 0x15;
    VFB(member_palette[42]) = 0x17;
    member_palette[0] = 0x3f;
    member_palette[72] = 0x3f;
    VFB(member_palette[73]) = 0x2a;
    VFB(member_palette[74]) = 0x1d;
    VFB(member_palette[78]) = 0x2b;
    VFB(member_palette[79]) = 0x1b;
    member_palette[35] = 0x25;
    member_palette[38] = 0x25;
    member_palette[81] = 0x25;
    member_palette[6] = 0xe;
    member_palette[7] = 0xe;
    member_palette[8] = 0xe;
    member_palette[83] = 0xe;
    member_palette[34] = 0x11;
    member_palette[45] = 0x11;
    member_palette[80] = 0x11;
    member_palette[88] = 0x11;
    VFB(member_palette[89]) = 0x9;
    VFB(member_palette[90]) = 0x16;
    VFB(member_palette[91]) = 0x10;
    member_palette[86] = 0x8;
    member_palette[92] = 0x8;
    member_palette[75] = 0x13;
    member_palette[85] = 0x13;
    member_palette[93] = 0x13;
    member_palette[76] = 0xf;
    member_palette[94] = 0xf;
    member_palette[77] = 0x7;
    member_palette[95] = 0x7;
    VFB(member_palette[49]) = 0x20;
    VFB(member_palette[50]) = 0x34;
    member_palette[39] = 0x1c;
    member_palette[84] = 0x1c;
    member_palette[52] = 0x1c;
    VFB(member_palette[53]) = 0x2e;
    member_palette[31] = 0x18;
    member_palette[82] = 0x18;
    member_palette[87] = 0x18;
    member_palette[55] = 0x18;
    VFB(member_palette[56]) = 0x28;
    VFB(member_palette[58]) = 0x14;
    VFB(member_palette[59]) = 0x22;
    VFB(member_palette[62]) = 0x1d;
    VFB(member_palette[64]) = 0xd;
    VFB(member_palette[65]) = 0x17;
    VFB(member_palette[67]) = 0xa;
    member_palette[61] = 0x11;
    member_palette[68] = 0x11;
    member_palette[69] = 0x9;
    member_palette[70] = 0x9;
    member_palette[71] = 0x9;
    member_palette[12] = 0;
    member_palette[13] = 0;
    member_palette[14] = 0;
    member_palette[16] = 0;
    member_palette[17] = 0;
    member_palette[19] = 0;
    member_palette[20] = 0;
    member_palette[22] = 0;
    member_palette[23] = 0;
    member_palette[26] = 0;
    member_palette[30] = 0;
    member_palette[33] = 0;
    member_palette[36] = 0;
    member_palette[40] = 0;
    member_palette[41] = 0;
    member_palette[43] = 0;
    member_palette[44] = 0;
    member_palette[46] = 0;
    member_palette[47] = 0;
    member_palette[48] = 0;
    member_palette[51] = 0;
    member_palette[54] = 0;
    member_palette[57] = 0;
    member_palette[60] = 0;
    member_palette[63] = 0;
    member_palette[66] = 0;
    member_palette[96] = 0;
    member_palette[97] = 0;
    member_palette[98] = 0;
    member_palette[99] = 0;
    member_palette[100] = 0;
    member_palette[101] = 0;
    member_palette[102] = 0;
    member_palette[103] = 0;
    member_palette[104] = 0;
    member_palette[105] = 0;
    member_palette[106] = 0;
    member_palette[107] = 0;
    member_palette[108] = 0;
    member_palette[109] = 0;
    member_palette[110] = 0;
    member_palette[111] = 0;
    member_palette[112] = 0;
    member_palette[113] = 0;
    member_palette[114] = 0;
    member_palette[115] = 0;
    member_palette[116] = 0;
    member_palette[117] = 0;
    member_palette[118] = 0;
    member_palette[119] = 0;
}
