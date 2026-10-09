/* place: s013 1432:0000 */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* status (e6, 2026-10-09): 0000 matches; 0338 047a 05bc 06fe 0848 098a 0adc 0c1e (one template:
 * load lc_* level, set door ids, save level0.flr, load .fil/.atv, remap group monsters) differ
 * ONLY in the door loop: 6.00 reloads the level pointer with `les bx,[bp-0eh]` for the
 * `i < lv->nobj` test every iteration (es unknown at the loop head) and then uses `mov ax,es`
 * after the loop; 6.00A keeps es (`mov bx,[bp-0eh]`, later `mov ax,[bp-0ch]`). Tried pragma
 * sweeps, pointer/index forms, volatile pointee, D_c103 direct. 0098 (court, 7 doors + start
 * position bit masks from D_160c) is drafted, not matching (register choice, shift loop rotation).
 * Baphomet's citadel (lcastle): one battlefield per room. Each sub loads a prepared level
 * file (map + objects + groups) into D_c103, fixes up door ids, saves the map as level0.flr
 * and loads the .fil (tile description) and .atv (enemy groups) files; the group monster
 * types are then mapped through sub_s004_0e76_06aa.
 */

struct res { int a; int b; };

struct obj {                            /* map object, 14 bytes, at 0x218c */
    int type;                           /* 7, 8: doors */
    int x2;
    char x4;                            /* index into pts[] */
    char x5;
    int x6;                             /* door id */
    char p8[6];
};

struct pt {                             /* 10 bytes at 0x3386 */
    int x;
    int y;
    char z;
    char p5[5];
};

struct level {                          /* battle map file image (0x33fc bytes) + groups */
    unsigned long cell[40][40];         /* 0000 */
    unsigned char hgt[40][40];          /* 1900 */
    char x1f40;
    char x1f41;
    int x1f42[5];                       /* 1f42 */
    int x1f4c;                          /* 1f4c */
    int x1f4e;                          /* 1f4e */
    char x1f50;                         /* 1f50 */
    char x1f51;
    char p1f52[0x2184 - 0x1f52];
    char nobj;                          /* 2184 */
    char p2185[0x218c - 0x2185];
    struct obj obj[54];                 /* 218c */
    char p2480[0x3386 - 0x2480];
    struct pt pts[6];                   /* 3386 */
    char p33c2[0x33fc - 0x33c2];
    int ngroups;                        /* 33fc */
};

struct group {
    int pos;
    int pos2;
    int x04;
    char x06;
    char x07;
    char type[3];
    char count[3];
    char p0e[0x1e - 0x0e];
};

extern struct level far *D_c103;
unsigned D_160c = 0;
int D_160e = 0;
extern _segment D_dadd;
extern char _based(D_dadd) *D_db33;
extern struct group _based(D_dadd) *D_db2d;
extern int D_db05;
struct mtype { char p00[0x0c]; char x0c; char p0d[0xd6 - 0x0d]; };
extern struct mtype _based(D_dadd) *D_db2b;
int far sub_s004_0e76_06aa();
int far sub_s004_1262_0d0a();
extern char D_9080;
int far sub_root_0000_25f2();
int far sub_root_05d9_019a();
int far sub_s013_1432_0000();
int far sub_s013_1432_0098();
int far sub_s013_1432_0338();
int far sub_s013_1432_047a();
int far sub_s013_1432_05bc();
int far sub_s013_1432_06fe();
int far sub_s013_1432_0848();
int far sub_s013_1432_098a();
int far sub_s013_1432_0adc();
int far sub_s013_1432_0c1e();

int far sub_s013_1432_0000(int p_6, int p_8, int p_a)
{
    sub_root_0000_25f2("lcastle");
    switch (p_6) {
    default:
        return 0;
    case 0x57:
        sub_s013_1432_0098();
        goto L_008e;
    case 0x58:
        sub_s013_1432_0338();
        goto L_008e;
    case 0x59:
        sub_s013_1432_047a();
        goto L_008e;
    case 0x5a:
        sub_s013_1432_05bc();
        goto L_008e;
    case 0x5b:
        if (p_a) {
            sub_s013_1432_06fe();
            goto L_008e;
        }
        sub_s013_1432_0848();
        goto L_008e;
    case 0x5c:
        sub_s013_1432_098a();
        goto L_008e;
    case 0x5d:
        sub_s013_1432_0adc();
        goto L_008e;
    case 0x6e:
        sub_s013_1432_0c1e();
L_008e:
        D_9080 |= 1;
        return 1;
    }
}


#pragma optimize("a", on)
int far sub_s013_1432_0098(void)
{
    struct level far *lv;
    int far *cnt;
    int pos[7][2];
    int doors[8];
    int *dp;
    int *pp;
    int bit;
    int m;
    int m2;
    int x;
    int i;

    lv = D_c103;
    cnt = &lv->ngroups;
    doors[0] = 0x159;
    doors[1] = 0x15c;
    doors[2] = 0x15e;
    doors[3] = 0x15b;
    doors[4] = 0x162;
    doors[5] = 0x165;
    doors[6] = 0x166;
    pos[0][0] = 2;
    pos[1][0] = 7;
    pos[2][0] = 12;
    pos[3][0] = 17;
    pos[4][0] = 22;
    pos[5][0] = 27;
    pos[6][0] = 32;
    pos[6][1] = pos[5][1] = pos[4][1] = pos[3][1] = pos[2][1] = pos[1][1] = pos[0][1] = 5;
    doors[7] = 0;
    x = 0;
    sub_root_05d9_019a("lc_court", lv);
    m = *(volatile unsigned *)&D_160c & 0xff;
    if (m) {
        *cnt = 0;
        while ((m >>= 1) != 0)
            x++;
        lv->x1f4c = (pos[x][0] << 5) + 0x10;
        lv->x1f4e = (pos[x][1] << 4) + 8;
        lv->x1f50 = 6;
    }
    if ((m2 = D_160c >> 8) != 0) {
        for (bit = 1, pp = pos[0]; bit < 0x200; bit <<= 1, pp += 2)
            if (m2 & bit)
                lv->cell[pp[0]][3] = lv->cell[pp[0]][3] & ~0x50000L | 0xa0000L;
        m2 = 1;
    }
    lv->x1f42[0] = 0x158;
    lv->x1f42[1] = 12;
    lv->x1f42[2] = 12;
    lv->x1f42[3] = 12;
    lv->x1f42[4] = 12;
    for (i = 0, dp = doors, pp = pos[0]; i < lv->nobj; i++, pp += 2) {
        if (lv->obj[i].type == 7 || lv->obj[i].type == 8) {
            lv->obj[i].x6 = *dp++;
            x = (unsigned char)lv->obj[i].x4;
            lv->pts[x].x = (pp[0] << 5) + 0x10;
            lv->pts[x].y = (pp[1] << 4) + 8;
            lv->pts[x].z = 4;
        }
    }
    sub_s004_1262_0d0a("level0.flr", lv, 0x33fc, 0);
    sub_root_05d9_019a("lc_court.fil", (char far *)D_db33);
    if (m2)
        D_db33[0x47] = 9;
    sub_root_05d9_019a("lc_court.atv", (struct group far *)D_db2d);
    i = 0;
    D_db05 = *cnt;
    for (i = 0; i < *cnt; i++) {
        D_db2d[i].type[0] = sub_s004_0e76_06aa(0, D_db2d[i].type[0], 1);
        D_db2d[i].count[0] = 2;
    }
    return 0;
}

int far sub_s013_1432_0338(void)
{
    struct level far *lv;
    int far *cnt;
    int doors[1];
    int k;
    int i;

    lv = D_c103;
    cnt = &lv->ngroups;
    doors[0] = 0x161;
    sub_root_05d9_019a("lc_g1", lv);
    lv->x1f42[0] = 0x158;
    lv->x1f42[1] = 0x16d;
    lv->x1f42[2] = 0x16d;
    lv->x1f42[3] = 0x16d;
    lv->x1f42[4] = 0x16d;
    k = 0;
    for (i = 0; i < lv->nobj; i++) {
        if (lv->obj[i].type == 7 || lv->obj[i].type == 8)
            lv->obj[i].x6 = doors[k++];
    }
    sub_s004_1262_0d0a("level0.flr", lv, 0x33fc, 0);
    sub_root_05d9_019a("lc_g1.fil", (char far *)D_db33);
    sub_root_05d9_019a("lc_g1.atv", (struct group far *)D_db2d);
    D_db05 = *cnt;
    for (i = 0; i < *cnt; i++) {
        D_db2d[i].type[0] = sub_s004_0e76_06aa(0, D_db2d[i].type[0], 1);
        D_db2d[i].count[0] = 4;
    }
    return 0;
}
#pragma optimize("", on)

#pragma optimize("a", on)
int far sub_s013_1432_047a(void)
{
    struct level far *lv;
    int far *cnt;
    int doors[1];
    int k;
    int i;

    lv = D_c103;
    cnt = &lv->ngroups;
    doors[0] = 0x15a;
    sub_root_05d9_019a("lc_g2", lv);
    lv->x1f42[0] = 0x158;
    lv->x1f42[1] = 0x16d;
    lv->x1f42[2] = 0x16d;
    lv->x1f42[3] = 0x16d;
    lv->x1f42[4] = 0x16d;
    k = 0;
    for (i = 0; i < lv->nobj; i++) {
        if (lv->obj[i].type == 7 || lv->obj[i].type == 8)
            lv->obj[i].x6 = doors[k++];
    }
    sub_s004_1262_0d0a("level0.flr", lv, 0x33fc, 0);
    sub_root_05d9_019a("lc_g2.fil", (char far *)D_db33);
    sub_root_05d9_019a("lc_g2.atv", (struct group far *)D_db2d);
    D_db05 = *cnt;
    for (i = 0; i < *cnt; i++) {
        D_db2d[i].type[0] = sub_s004_0e76_06aa(0, D_db2d[i].type[0], 1);
        D_db2d[i].count[0] = 2;
    }
    return 0;
}
#pragma optimize("", on)

#pragma optimize("a", on)
int far sub_s013_1432_05bc(void)
{
    struct level far *lv;
    int far *cnt;
    int doors[1];
    int k;
    int i;

    lv = D_c103;
    cnt = &lv->ngroups;
    doors[0] = 0x15d;
    sub_root_05d9_019a("lc_g3", lv);
    lv->x1f42[0] = 0x158;
    lv->x1f42[1] = 0x16d;
    lv->x1f42[2] = 0x16d;
    lv->x1f42[3] = 0x16d;
    lv->x1f42[4] = 0x16d;
    k = 0;
    for (i = 0; i < lv->nobj; i++) {
        if (lv->obj[i].type == 7 || lv->obj[i].type == 8)
            lv->obj[i].x6 = doors[k++];
    }
    sub_s004_1262_0d0a("level0.flr", lv, 0x33fc, 0);
    sub_root_05d9_019a("lc_g3.fil", (char far *)D_db33);
    sub_root_05d9_019a("lc_g3.atv", (struct group far *)D_db2d);
    D_db05 = *cnt;
    for (i = 0; i < *cnt; i++) {
        D_db2d[i].type[0] = sub_s004_0e76_06aa(0, D_db2d[i].type[0], 2);
        D_db2d[i].count[0] = 1;
    }
    return 0;
}
#pragma optimize("", on)

#pragma optimize("a", on)
int far sub_s013_1432_06fe(void)
{
    struct level far *lv;
    int far *cnt;
    int doors[1];
    int k;
    int i;

    lv = D_c103;
    cnt = &lv->ngroups;
    doors[0] = 0x160;
    sub_root_05d9_019a("lc_g4s", lv);
    lv->x1f42[0] = 0x158;
    lv->x1f42[1] = 0x16d;
    lv->x1f42[2] = 0x16d;
    lv->x1f42[3] = 0x16d;
    lv->x1f42[4] = 0x16d;
    k = 0;
    for (i = 0; i < lv->nobj; i++) {
        if (lv->obj[i].type == 7 || lv->obj[i].type == 8)
            lv->obj[i].x6 = doors[k++];
    }
    sub_s004_1262_0d0a("level0.flr", lv, 0x33fc, 0);
    sub_root_05d9_019a("lc_g4s.fil", (char far *)D_db33);
    sub_root_05d9_019a("lc_g4s.atv", (struct group far *)D_db2d);
    *cnt -= 2;
    D_db05 = *cnt;
    for (i = 0; i < *cnt; i++) {
        D_db2d[i].type[0] = sub_s004_0e76_06aa(0, D_db2d[i].type[0], 2);
        D_db2d[i].count[0] = 3;
    }
    return 0;
}
#pragma optimize("", on)

#pragma optimize("a", on)
int far sub_s013_1432_0848(void)
{
    struct level far *lv;
    int far *cnt;
    int doors[1];
    int k;
    int i;

    lv = D_c103;
    cnt = &lv->ngroups;
    doors[0] = 0x160;
    sub_root_05d9_019a("lc_g4ns", lv);
    lv->x1f42[0] = 0x158;
    lv->x1f42[1] = 0x16d;
    lv->x1f42[2] = 0x16d;
    lv->x1f42[3] = 0x16d;
    lv->x1f42[4] = 0x16d;
    k = 0;
    for (i = 0; i < lv->nobj; i++) {
        if (lv->obj[i].type == 7 || lv->obj[i].type == 8)
            lv->obj[i].x6 = doors[k++];
    }
    sub_s004_1262_0d0a("level0.flr", lv, 0x33fc, 0);
    sub_root_05d9_019a("lc_g4ns.fil", (char far *)D_db33);
    sub_root_05d9_019a("lc_g4ns.atv", (struct group far *)D_db2d);
    D_db05 = *cnt;
    for (i = 0; i < *cnt; i++) {
        D_db2d[i].type[0] = sub_s004_0e76_06aa(0, D_db2d[i].type[0], 1);
        D_db2d[i].count[0] = 1;
    }
    return 0;
}
#pragma optimize("", on)

#pragma optimize("a", on)
int far sub_s013_1432_098a(void)
{
    struct level far *lv;
    int far *cnt;
    int doors[1];
    int k;
    int i;

    lv = D_c103;
    cnt = &lv->ngroups;
    doors[0] = 0x164;
    sub_root_05d9_019a("lc_g5", lv);
    lv->x1f42[0] = 0x158;
    lv->x1f42[1] = 0x16d;
    lv->x1f42[2] = 0x16d;
    lv->x1f42[3] = 0x16d;
    lv->x1f42[4] = 0x16d;
    k = 0;
    for (i = 0; i < lv->nobj; i++) {
        if (lv->obj[i].type == 7 || lv->obj[i].type == 8)
            lv->obj[i].x6 = doors[k++];
    }
    sub_s004_1262_0d0a("level0.flr", lv, 0x33fc, 0);
    sub_root_05d9_019a("lc_g5.fil", (char far *)D_db33);
    D_db33[0x44] = 8;
    D_db33[0x77] = 6;
    sub_root_05d9_019a("lc_g5.atv", (struct group far *)D_db2d);
    D_db05 = *cnt;
    for (i = 0; i < *cnt; i++) {
        D_db2d[i].type[0] = sub_s004_0e76_06aa(0, D_db2d[i].type[0], 1);
        D_db2d[i].count[0] = 1;
    }
    return 0;
}
#pragma optimize("", on)

#pragma optimize("a", on)
int far sub_s013_1432_0adc(void)
{
    struct level far *lv;
    int far *cnt;
    int doors[1];
    int k;
    int i;

    lv = D_c103;
    cnt = &lv->ngroups;
    doors[0] = 0x15f;
    sub_root_05d9_019a("lc_g7a", lv);
    lv->x1f42[0] = 0x158;
    lv->x1f42[1] = 0x16d;
    lv->x1f42[2] = 0x16d;
    lv->x1f42[3] = 0x16d;
    lv->x1f42[4] = 0x16d;
    k = 0;
    for (i = 0; i < lv->nobj; i++) {
        if (lv->obj[i].type == 7 || lv->obj[i].type == 8)
            lv->obj[i].x6 = doors[k++];
    }
    sub_s004_1262_0d0a("level0.flr", lv, 0x33fc, 0);
    sub_root_05d9_019a("lc_g7a.fil", (char far *)D_db33);
    sub_root_05d9_019a("lc_g7a.atv", (struct group far *)D_db2d);
    D_db05 = *cnt;
    for (i = 0; i < *cnt; i++) {
        D_db2d[i].type[0] = sub_s004_0e76_06aa(0, D_db2d[i].type[0], 1);
        D_db2d[i].count[0] = 1;
    }
    return 0;
}
#pragma optimize("", on)

#pragma optimize("a", on)
int far sub_s013_1432_0c1e(void)
{
    struct level far *lv;
    int far *cnt;
    int doors[1];
    int k;
    int i;

    lv = D_c103;
    cnt = &lv->ngroups;
    sub_root_05d9_019a("lc_g7b", lv);
    doors[0] = lv->x1f42[0] = 0x167;
    lv->x1f42[1] = 0x16d;
    lv->x1f42[2] = 0x16d;
    lv->x1f42[3] = 0x16d;
    lv->x1f42[4] = 0x16d;
    k = 0;
    for (i = 0; i < lv->nobj; i++) {
        if (lv->obj[i].type == 7 || lv->obj[i].type == 8)
            lv->obj[i].x6 = doors[k++];
    }
    sub_s004_1262_0d0a("level0.flr", lv, 0x33fc, 0);
    sub_root_05d9_019a("lc_g7b.fil", (char far *)D_db33);
    sub_root_05d9_019a("lc_g7b.atv", (struct group far *)D_db2d);
    D_db05 = *cnt;
    for (i = 0; i < *cnt; i++) {
        D_db2d[i].type[0] = sub_s004_0e76_06aa(0, D_db2d[i].type[0], 1);
        D_db2d[i].count[0] = 1;
        D_db2b[D_db2d[i].type[0]].x0c = 1;
    }
    return 0;
}
#pragma optimize("", on)
