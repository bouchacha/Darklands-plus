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
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* Overlay s001: set up the default (quick-start) party: four members with names,
   attributes, skills and starting equipment, and load the item definitions from
   darkland.lst when they are not loaded yet (D_11dd == 1). */
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
void far sub_root_0617_011e(char *, void far *);
unsigned far alloc_buffer(unsigned long, char *);
void far free_buffer(unsigned, char *);
extern char D_11dd;
extern unsigned char far *D_a889;     /* item definition file buffer */
extern int D_a67e;                    /* number of party members */
extern char D_907b;
extern char D_9c69[][0x80];  /* party member records */
struct itemdef { char name[0x25]; unsigned char weight; };
extern struct itemdef far * far F_2e38_0890[];   /* item definitions (46 bytes each) */
extern unsigned char far F_2e38_082c[][0x14];
extern unsigned char far F_2e38_07be[][0x16];
extern struct item far F_2e38_003e[][64];       /* inventories */
extern unsigned char far F_20a5_33fc[];
#define VB(x) (*(volatile unsigned char *)&(x))
#define VW(x) (*(volatile int *)&(x))
#define VFB(x) (*(volatile unsigned char far *)&(x))
#define VFW(x) (*(volatile int far *)&(x))

#pragma optimize("a", on)
#pragma optimize("l", off)
void far sub_s001_0e76_0000(void)
{
    struct find_t ft;
    unsigned cnt;
    unsigned k;
    unsigned char far *p;
    register int si, di;
    int a, x;      /* unused: the original compiler gave its register variables stack slots */

    if (D_11dd == 1) {
        _dos_findfirst("darkland.lst", 0, &ft);
        D_a889 = p = MK_FP(alloc_buffer(ft.size, "Items"), 0);
        sub_root_0617_011e("darkland.lst", p);
        cnt = *p;
        p++;
        p += 2;
        for (k = 0; k < cnt; k++) {
            F_2e38_0890[k] = (struct itemdef far *)p;
            p += 0x2e;
        }
    }
    VW(D_a67e) = 0x4;
    D_9c69[0][0] = 0x1;
    D_9c69[1][0] = 0x1;
    D_9c69[2][0] = 0x1;
    D_9c69[3][0] = 0x1;
    D_907b = 0;
    D_9c69[4][0] = 0;
    strcpy(&D_9c69[0][17], "Gretchen Wilburg");
    strcpy(&D_9c69[0][42], "Gretch");
    D_9c69[0][6] = 0;
    D_9c69[0][5] = 0;
    VB(D_9c69[0][82]) = 0x1d;
    VB(D_9c69[0][83]) = 0x12;
    VB(D_9c69[0][84]) = 0x1f;
    VB(D_9c69[0][85]) = 0x20;
    VB(D_9c69[0][86]) = 0x63;
    VB(D_9c69[0][87]) = 0x23;
    VB(D_9c69[0][88]) = 0x14;
    VB(D_9c69[0][90]) = 0x10;
    D_9c69[0][80] = 0x1e;
    D_9c69[0][81] = 0x1e;
    D_9c69[0][92] = 0x1e;
    D_9c69[0][89] = 0x6;
    D_9c69[0][94] = 0x6;
    VB(D_9c69[0][97]) = 0x28;
    VB(D_9c69[0][100]) = 0x3;
    VB(D_9c69[0][101]) = 0x2;
    VB(D_9c69[0][102]) = 0x15;
    VB(D_9c69[0][103]) = 0xb;
    D_9c69[0][93] = 0xd;
    D_9c69[0][95] = 0xd;
    D_9c69[0][104] = 0xd;
    D_9c69[0][91] = 0x4;
    D_9c69[0][98] = 0x4;
    D_9c69[0][99] = 0x4;
    D_9c69[0][105] = 0x4;
    D_9c69[0][96] = 0x19;
    D_9c69[0][59] = 0x19;
    D_9c69[0][60] = 0x19;
    strcpy(&D_9c69[1][17], "Gunther Langer");
    strcpy(&D_9c69[1][42], "Gunther");
    D_9c69[1][6] = 0;
    D_9c69[1][5] = 0;
    VB(D_9c69[1][80]) = 0x27;
    VB(D_9c69[1][81]) = 0x28;
    VB(D_9c69[1][83]) = 0x18;
    VB(D_9c69[1][84]) = 0x10;
    VB(D_9c69[1][85]) = 0xd;
    VB(D_9c69[1][86]) = 0x63;
    VB(D_9c69[1][88]) = 0x2f;
    VB(D_9c69[1][90]) = 0x20;
    VB(D_9c69[1][91]) = 0x14;
    VB(D_9c69[1][92]) = 0xf;
    VB(D_9c69[1][93]) = 0x1f;
    VB(D_9c69[1][95]) = 0x9;
    D_9c69[1][94] = 0x2;
    D_9c69[1][98] = 0x2;
    VB(D_9c69[1][99]) = 0x5;
    VB(D_9c69[1][100]) = 0xc;
    VB(D_9c69[1][101]) = 0x7;
    D_9c69[1][82] = 0x1a;
    D_9c69[1][102] = 0x1a;
    D_9c69[1][96] = 0x13;
    D_9c69[1][103] = 0x13;
    D_9c69[1][89] = 0xb;
    D_9c69[1][97] = 0xb;
    D_9c69[1][104] = 0xb;
    D_9c69[1][87] = 0x1d;
    D_9c69[1][105] = 0x1d;
    D_9c69[1][59] = 0x19;
    D_9c69[1][60] = 0x19;
    strcpy(&D_9c69[2][17], "Hans Muller");
    strcpy(&D_9c69[2][42], "Hans");
    D_9c69[2][6] = 0;
    D_9c69[2][5] = 0;
    VB(D_9c69[2][81]) = 0x19;
    VB(D_9c69[2][82]) = 0x1c;
    VB(D_9c69[2][84]) = 0x2b;
    VB(D_9c69[2][85]) = 0x10;
    VB(D_9c69[2][86]) = 0x63;
    VB(D_9c69[2][87]) = 0xb;
    VB(D_9c69[2][90]) = 0x6;
    D_9c69[2][89] = 0x3;
    D_9c69[2][92] = 0x3;
    D_9c69[2][83] = 0x14;
    D_9c69[2][93] = 0x14;
    VB(D_9c69[2][94]) = 0x30;
    VB(D_9c69[2][95]) = 0xa;
    D_9c69[2][88] = 0x13;
    D_9c69[2][91] = 0x13;
    D_9c69[2][96] = 0x13;
    D_9c69[2][80] = 0x1b;
    D_9c69[2][97] = 0x1b;
    VB(D_9c69[2][98]) = 0x17;
    VB(D_9c69[2][99]) = 0x20;
    VB(D_9c69[2][101]) = 0x21;
    D_9c69[2][100] = 0x8;
    D_9c69[2][102] = 0x8;
    VB(D_9c69[2][103]) = 0xe;
    VB(D_9c69[2][104]) = 0x4;
    VB(D_9c69[2][105]) = 0x7;
    D_9c69[2][59] = 0x19;
    D_9c69[2][60] = 0x19;
    strcpy(&D_9c69[3][17], "Ebhard of Achdorf");
    strcpy(&D_9c69[3][42], "Ebhard");
    D_9c69[3][6] = 0;
    D_9c69[3][5] = 0;
    VB(D_9c69[3][80]) = 0x1a;
    VB(D_9c69[3][82]) = 0x18;
    VB(D_9c69[3][83]) = 0x1c;
    VB(D_9c69[3][84]) = 0x1f;
    VB(D_9c69[3][85]) = 0x1d;
    VB(D_9c69[3][86]) = 0x63;
    VB(D_9c69[3][87]) = 0xa;
    VB(D_9c69[3][88]) = 0x14;
    VB(D_9c69[3][89]) = 0x22;
    VB(D_9c69[3][90]) = 0x9;
    VB(D_9c69[3][92]) = 0x4;
    VB(D_9c69[3][93]) = 0x1b;
    VB(D_9c69[3][96]) = 0x34;
    VB(D_9c69[3][97]) = 0x13;
    D_9c69[3][98] = 0x17;
    D_9c69[3][99] = 0x17;
    D_9c69[3][91] = 0x8;
    D_9c69[3][94] = 0x8;
    D_9c69[3][101] = 0x8;
    VB(D_9c69[3][102]) = 0xc;
    VB(D_9c69[3][103]) = 0x7;
    VB(D_9c69[3][104]) = 0x5;
    VB(D_9c69[3][105]) = 0x3;
    D_9c69[3][81] = 0x19;
    D_9c69[3][59] = 0x19;
    D_9c69[3][60] = 0x19;
    VW(D_9c69[0][-2]) = 0x23;
    *(int *)&D_9c69[1][-2] = 0x28;
    *(int *)&D_9c69[2][-2] = 0x28;
    VW(D_9c69[3][-2]) = 0x2d;
    VB(D_9c69[0][53]) = 0x39;
    D_9c69[3][95] = 0x28;
    D_9c69[3][100] = 0x28;
    D_9c69[1][53] = 0x28;
    VB(D_9c69[2][53]) = 0x24;
    VB(D_9c69[3][53]) = 0x3a;
    for (si = 0; si < 5; si++) {
        for (di = 0; di < 0x14; di++)
            F_2e38_082c[si][di] = 0;
        for (di = 0; di < 0x16; di++)
            F_2e38_07be[si][di] = 0;
    }
    VFB(F_2e38_082c[3][0]) = 0x10;
    F_2e38_07be[2][7] = 0x1;
    F_2e38_082c[3][2] = 0x1;
    F_2e38_07be[2][0] = 0x2;
    F_2e38_07be[2][9] = 0x2;
    F_2e38_07be[2][20] = 0x2;
    F_2e38_082c[3][5] = 0x2;
    VFB(F_2e38_082c[3][6]) = 0x20;
    VFB(F_2e38_082c[3][14]) = 0x40;
    for (si = 0; si < 13; si++) {
        D_9c69[si][1] = si + 'A';
        for (di = 0; di < 7; di++)
            D_9c69[si][0x49 + di] = D_9c69[si][0x50 + di];
    }
    VB(D_9c69[0][3]) = 0x1;
    VB(D_9c69[0][56]) = 0x52;
    VFW(F_2e38_003e[0][0].id) = 0x5;
    VFB(F_2e38_003e[0][0].f2) = 0x3;
    F_2e38_003e[0][0].f5 = F_2e38_0890[5]->weight;
    F_2e38_003e[0][1].f5 = F_2e38_0890[43]->weight;
    VFW(F_2e38_003e[0][2].id) = 0x33;
    VFB(F_2e38_003e[0][2].f2) = 0x52;
    F_2e38_003e[0][2].f5 = F_2e38_0890[51]->weight;
    VB(D_9c69[1][61]) = 0xf;
    D_9c69[0][55] = 0x4a;
    D_9c69[1][55] = 0x4a;
    VB(D_9c69[1][56]) = 0x50;
    VFW(F_2e38_003e[1][0].id) = 0xf;
    VFB(F_2e38_003e[1][0].f2) = 0xf;
    F_2e38_003e[1][0].f5 = F_2e38_0890[15]->weight;
    F_2e38_003e[0][1].id = 0x2b;
    F_2e38_003e[1][1].id = 0x2b;
    F_2e38_003e[0][1].f2 = 0x4a;
    F_2e38_003e[1][1].f2 = 0x4a;
    F_2e38_003e[1][1].f5 = F_2e38_003e[0][1].f5;   /* = weight of item 43, stored above */
    VFW(F_2e38_003e[1][2].id) = 0x31;
    VFB(F_2e38_003e[1][2].f2) = 0x50;
    F_2e38_003e[1][2].f5 = F_2e38_0890[49]->weight;
    D_9c69[0][61] = 0x3;
    D_9c69[0][106] = 0x3;
    D_9c69[1][106] = 0x3;
    VB(D_9c69[2][14]) = 0x1f;
    D_9c69[0][14] = 0xff;
    D_9c69[0][72] = 0xff;
    D_9c69[1][14] = 0xff;
    D_9c69[1][72] = 0xff;
    D_9c69[2][61] = 0xff;
    D_9c69[2][72] = 0xff;
    D_9c69[0][68] = 0x19;
    D_9c69[0][70] = 0x19;
    D_9c69[0][71] = 0x19;
    D_9c69[1][68] = 0x19;
    D_9c69[2][68] = 0x19;
    D_9c69[2][70] = 0x19;
    D_9c69[2][71] = 0x19;
    VB(D_9c69[2][55]) = 0x49;
    VB(D_9c69[2][56]) = 0x4e;
    VFW(F_2e38_003e[2][0].id) = 0x1e;
    VFB(F_2e38_003e[2][0].f2) = 0x1f;
    F_2e38_003e[2][0].f5 = F_2e38_0890[30]->weight;
    VFW(F_2e38_003e[2][1].id) = 0x5f;
    VFB(F_2e38_003e[2][1].f2) = 0x84;
    VFB(F_2e38_003e[2][1].f3) = 0x14;
    F_2e38_003e[2][1].f5 = F_2e38_0890[95]->weight;
    VFW(F_2e38_003e[2][2].id) = 0x68;
    VFB(F_2e38_003e[2][2].f2) = 0x8d;
    VFB(F_2e38_003e[2][2].f3) = 0x2d;
    F_2e38_003e[2][2].f5 = F_2e38_0890[104]->weight;
    VFW(F_2e38_003e[2][3].id) = 0x66;
    VFB(F_2e38_003e[2][3].f2) = 0x8b;
    F_2e38_003e[2][3].f5 = F_2e38_0890[102]->weight;
    VFW(F_2e38_003e[2][4].id) = 0x73;
    VFB(F_2e38_003e[2][4].f2) = 0x98;
    VFB(F_2e38_003e[2][4].f3) = 0x23;
    F_2e38_003e[2][4].f5 = F_2e38_0890[115]->weight;
    VFW(F_2e38_003e[2][5].id) = 0x2f;
    VFB(F_2e38_003e[2][5].f2) = 0x4e;
    F_2e38_003e[2][5].f5 = F_2e38_0890[47]->weight;
    VFW(F_2e38_003e[2][6].id) = 0x2a;
    VFB(F_2e38_003e[2][6].f2) = 0x49;
    F_2e38_003e[0][0].f4 = 0x1;
    F_2e38_003e[0][1].f4 = 0x1;
    F_2e38_003e[0][2].f4 = 0x1;
    F_2e38_003e[1][0].f4 = 0x1;
    F_2e38_003e[1][1].f4 = 0x1;
    F_2e38_003e[1][2].f4 = 0x1;
    F_2e38_003e[2][0].f4 = 0x1;
    F_2e38_003e[2][1].f4 = 0x1;
    F_2e38_003e[2][2].f4 = 0x1;
    F_2e38_003e[2][3].f4 = 0x1;
    F_2e38_003e[2][4].f4 = 0x1;
    F_2e38_003e[2][5].f4 = 0x1;
    F_2e38_003e[2][6].f4 = 0x1;
    F_2e38_003e[2][6].f5 = F_2e38_0890[42]->weight;
    VFW(F_2e38_003e[2][7].id) = 0x22;
    VFB(F_2e38_003e[2][7].f2) = 0x41;
    F_2e38_003e[0][0].f3 = 0x19;
    F_2e38_003e[0][1].f3 = 0x19;
    F_2e38_003e[0][2].f3 = 0x19;
    F_2e38_003e[1][0].f3 = 0x19;
    F_2e38_003e[1][1].f3 = 0x19;
    F_2e38_003e[1][2].f3 = 0x19;
    F_2e38_003e[2][0].f3 = 0x19;
    F_2e38_003e[2][3].f3 = 0x19;
    F_2e38_003e[2][5].f3 = 0x19;
    F_2e38_003e[2][6].f3 = 0x19;
    F_2e38_003e[2][7].f3 = 0x19;
    VFB(F_2e38_003e[2][7].f4) = 0x1e;
    F_2e38_003e[2][7].f5 = F_2e38_0890[65]->weight;
    VB(D_9c69[2][106]) = 0x8;
    VB(D_9c69[3][61]) = 0x11;
    VB(D_9c69[3][14]) = 0xff;
    D_9c69[3][68] = 0x19;
    D_9c69[3][70] = 0x19;
    VB(D_9c69[3][72]) = 0xff;
    D_9c69[3][71] = 0x19;
    D_9c69[3][55] = 0x4a;
    VB(D_9c69[3][56]) = 0x52;
    VFW(F_2e38_003e[3][0].id) = 0x11;
    VFB(F_2e38_003e[3][0].f2) = 0x11;
    F_2e38_003e[3][0].f5 = F_2e38_0890[17]->weight;
    VFW(F_2e38_003e[3][1].id) = 0x2b;
    F_2e38_003e[3][1].f2 = 0x4a;
    F_2e38_003e[3][1].f5 = F_2e38_0890[43]->weight;
    VFW(F_2e38_003e[3][2].id) = 0x33;
    VFB(F_2e38_003e[3][2].f2) = 0x52;
    F_2e38_003e[3][0].f3 = 0x19;
    F_2e38_003e[3][1].f3 = 0x19;
    F_2e38_003e[3][2].f3 = 0x19;
    F_2e38_003e[3][0].f4 = 0x1;
    F_2e38_003e[3][1].f4 = 0x1;
    F_2e38_003e[3][2].f4 = 0x1;
    F_2e38_003e[3][2].f5 = F_2e38_0890[51]->weight;
    VB(D_9c69[3][106]) = 0x3;
    if (D_11dd == 1)
        free_buffer(FP_SEG(D_a889), "Items");
    VFB(F_20a5_33fc[2]) = 0x19;
    F_20a5_33fc[9] = 0x9;
    F_20a5_33fc[10] = 0x9;
    F_20a5_33fc[11] = 0x9;
    VFB(F_20a5_33fc[15]) = 0x22;
    VFB(F_20a5_33fc[21]) = 0x2e;
    VFB(F_20a5_33fc[24]) = 0x39;
    VFB(F_20a5_33fc[25]) = 0x36;
    VFB(F_20a5_33fc[27]) = 0x3d;
    VFB(F_20a5_33fc[28]) = 0x38;
    F_20a5_33fc[1] = 0x28;
    F_20a5_33fc[18] = 0x28;
    F_20a5_33fc[32] = 0x28;
    F_20a5_33fc[3] = 0x15;
    F_20a5_33fc[4] = 0x15;
    F_20a5_33fc[5] = 0x15;
    F_20a5_33fc[29] = 0x15;
    F_20a5_33fc[37] = 0x15;
    VFB(F_20a5_33fc[42]) = 0x17;
    F_20a5_33fc[0] = 0x3f;
    F_20a5_33fc[72] = 0x3f;
    VFB(F_20a5_33fc[73]) = 0x2a;
    VFB(F_20a5_33fc[74]) = 0x1d;
    VFB(F_20a5_33fc[78]) = 0x2b;
    VFB(F_20a5_33fc[79]) = 0x1b;
    F_20a5_33fc[35] = 0x25;
    F_20a5_33fc[38] = 0x25;
    F_20a5_33fc[81] = 0x25;
    F_20a5_33fc[6] = 0xe;
    F_20a5_33fc[7] = 0xe;
    F_20a5_33fc[8] = 0xe;
    F_20a5_33fc[83] = 0xe;
    F_20a5_33fc[34] = 0x11;
    F_20a5_33fc[45] = 0x11;
    F_20a5_33fc[80] = 0x11;
    F_20a5_33fc[88] = 0x11;
    VFB(F_20a5_33fc[89]) = 0x9;
    VFB(F_20a5_33fc[90]) = 0x16;
    VFB(F_20a5_33fc[91]) = 0x10;
    F_20a5_33fc[86] = 0x8;
    F_20a5_33fc[92] = 0x8;
    F_20a5_33fc[75] = 0x13;
    F_20a5_33fc[85] = 0x13;
    F_20a5_33fc[93] = 0x13;
    F_20a5_33fc[76] = 0xf;
    F_20a5_33fc[94] = 0xf;
    F_20a5_33fc[77] = 0x7;
    F_20a5_33fc[95] = 0x7;
    VFB(F_20a5_33fc[49]) = 0x20;
    VFB(F_20a5_33fc[50]) = 0x34;
    F_20a5_33fc[39] = 0x1c;
    F_20a5_33fc[84] = 0x1c;
    F_20a5_33fc[52] = 0x1c;
    VFB(F_20a5_33fc[53]) = 0x2e;
    F_20a5_33fc[31] = 0x18;
    F_20a5_33fc[82] = 0x18;
    F_20a5_33fc[87] = 0x18;
    F_20a5_33fc[55] = 0x18;
    VFB(F_20a5_33fc[56]) = 0x28;
    VFB(F_20a5_33fc[58]) = 0x14;
    VFB(F_20a5_33fc[59]) = 0x22;
    VFB(F_20a5_33fc[62]) = 0x1d;
    VFB(F_20a5_33fc[64]) = 0xd;
    VFB(F_20a5_33fc[65]) = 0x17;
    VFB(F_20a5_33fc[67]) = 0xa;
    F_20a5_33fc[61] = 0x11;
    F_20a5_33fc[68] = 0x11;
    F_20a5_33fc[69] = 0x9;
    F_20a5_33fc[70] = 0x9;
    F_20a5_33fc[71] = 0x9;
    F_20a5_33fc[12] = 0;
    F_20a5_33fc[13] = 0;
    F_20a5_33fc[14] = 0;
    F_20a5_33fc[16] = 0;
    F_20a5_33fc[17] = 0;
    F_20a5_33fc[19] = 0;
    F_20a5_33fc[20] = 0;
    F_20a5_33fc[22] = 0;
    F_20a5_33fc[23] = 0;
    F_20a5_33fc[26] = 0;
    F_20a5_33fc[30] = 0;
    F_20a5_33fc[33] = 0;
    F_20a5_33fc[36] = 0;
    F_20a5_33fc[40] = 0;
    F_20a5_33fc[41] = 0;
    F_20a5_33fc[43] = 0;
    F_20a5_33fc[44] = 0;
    F_20a5_33fc[46] = 0;
    F_20a5_33fc[47] = 0;
    F_20a5_33fc[48] = 0;
    F_20a5_33fc[51] = 0;
    F_20a5_33fc[54] = 0;
    F_20a5_33fc[57] = 0;
    F_20a5_33fc[60] = 0;
    F_20a5_33fc[63] = 0;
    F_20a5_33fc[66] = 0;
    F_20a5_33fc[96] = 0;
    F_20a5_33fc[97] = 0;
    F_20a5_33fc[98] = 0;
    F_20a5_33fc[99] = 0;
    F_20a5_33fc[100] = 0;
    F_20a5_33fc[101] = 0;
    F_20a5_33fc[102] = 0;
    F_20a5_33fc[103] = 0;
    F_20a5_33fc[104] = 0;
    F_20a5_33fc[105] = 0;
    F_20a5_33fc[106] = 0;
    F_20a5_33fc[107] = 0;
    F_20a5_33fc[108] = 0;
    F_20a5_33fc[109] = 0;
    F_20a5_33fc[110] = 0;
    F_20a5_33fc[111] = 0;
    F_20a5_33fc[112] = 0;
    F_20a5_33fc[113] = 0;
    F_20a5_33fc[114] = 0;
    F_20a5_33fc[115] = 0;
    F_20a5_33fc[116] = 0;
    F_20a5_33fc[117] = 0;
    F_20a5_33fc[118] = 0;
    F_20a5_33fc[119] = 0;
}
