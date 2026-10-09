/* place: s011 1432:0000 */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* status (e6, 2026-10-09): Templar fortress & tombs battlefields. Written: 16d2 (random monster
 * type 0x96..0xab; code right, jump table only relocates once 0000..1794 have their lengths),
 * 1794 (add an entry to a group: MATCH), 1d94 (set value of the n-th type-5 object; 6.00
 * stores the loop pointer's segment every iteration - close with the volatile store, register
 * choice still differs), 1968 (close), 1f5c (save level k as level%d.flr: MATCH once _DATA
 * lines up), 209e ("keep" tile description: code right, string placement needs all earlier
 * literals). Others are still the dec16 stubs.
 * Map objects: 55 x 14 bytes at 0x2186 {type @6, char @0x0a, char @0x0b, value @0x0c};
 * D_e047 (based on D_e045) = 4 saved levels of 0x33fc bytes; D_db35 (based on D_dae3) =
 * 0x2e-byte item records.
 */

struct res { int a; int b; };

struct obj {                            /* map object, 14 bytes, 55 of them at 0x2186 */
    char p0[6];
    int type;                           /* 06: 5, 7, 8 ... */
    char p8[2];
    unsigned char x0a;
    char x0b;
    int x0c;                            /* door id / value */
};

struct t1c { char p0[9]; char a; char p0a[0x17 - 0x0a]; char b; char p18[0x1c - 0x18]; };

struct bfdesc {
    char x00;
    char set0[10];                      /* 01 */
    char t0b;                           /* 0b */
    char p0c[0x34 - 0x0c];
    char set2[10];                      /* 34 */
    char t3e;                           /* 3e */
    char p3f[0x67 - 0x3f];
    char set3[10];                      /* 67 */
    char t71;                           /* 71 */
    char p72[0x9a - 0x72];
    char set4[9];                       /* 9a */
    char a3;
    char a4;
    char ta5[2];                        /* a5 */
    char pa7[0xab - 0xa7];
    char tab;                           /* ab */
    char pac[0xb0 - 0xac];
    char tb0;                           /* b0 */
};
extern _segment D_dadd;
extern struct bfdesc _based(D_dadd) *D_db33;
int far _fstrcpy();

struct level {                          /* battle map (0x33fc bytes) */
    unsigned long cell[40][40];         /* 0000 */
    unsigned char hgt[40][40];          /* 1900 */
    char x1f40;                         /* 1f40: level number */
    char p1f41;
    int x1f42[5];                       /* 1f42 */
    char p1f4c[0x1f52 - 0x1f4c];
    struct t1c tbl[(0x2184 - 0x1f52) / 0x1c]; /* 1f52 */
    char p2182[0x2184 - 0x1f52 - (0x2184 - 0x1f52) / 0x1c * 0x1c];
    char nobj;                          /* 2184 */
    char p2185;
    struct obj obj[55];                 /* 2186 */
    char p2488[0x33fc - 0x2488];
};

struct gent {                           /* 6 bytes */
    int type;
    char b, c, d, e;
};

struct grp2 {
    char p0[0x0b];
    unsigned char n;                    /* 0b */
    char p0c[0x14 - 0x0c];
    struct gent ent[4];                 /* 14 */
};

struct item { char p0[0x25]; char x25; char p26[0x2e - 0x26]; };

extern _segment D_dae3;
extern struct item _based(D_dae3) *D_db35;
extern struct level far F_20a5_0000;
extern _segment D_e045;
extern struct level _based(D_e045) *D_e047;
int far sprintf();
int far sub_root_05ba_0004();
int far sub_root_05d9_019a();
int far sub_s004_0e76_20dc();
int far sub_s004_0e76_20f6();
int far sub_s004_1262_0d0a();
void far sub_s011_1432_0000();
void far sub_s011_1432_029e();
void far sub_s011_1432_0484();
void far sub_s011_1432_05c6();
void far sub_s011_1432_096a();
int far sub_s011_1432_16d2();
void far sub_s011_1432_17fa();
void far sub_s011_1432_1a2a();
void far sub_s011_1432_1bc0();
void far sub_s011_1432_1cb4();
void far sub_s011_1432_1ddc();
int far sub_s011_1432_1fb2();
void far sub_s011_1432_209e();

/* sub_s011_1432_0000: not decompiled: unhandled instruction rep movsw word ptr es:[di], word ptr [si] */
void far sub_s011_1432_0000(void) { }

/* sub_s011_1432_029e: not decompiled: mov es from non-slot */
void far sub_s011_1432_029e(void) { }

/* sub_s011_1432_0484: not decompiled: mov es from non-slot */
void far sub_s011_1432_0484(void) { }

/* sub_s011_1432_05c6: not decompiled: mov es from non-slot */
void far sub_s011_1432_05c6(void) { }

/* sub_s011_1432_096a: not decompiled: mov es from non-slot */
void far sub_s011_1432_096a(void) { }

int far sub_s011_1432_16d2(void)
{
    switch (sub_s004_0e76_20f6(1, 0x16)) {
    case 1:
        return 0x96;
    case 2:
        return 0x97;
    case 3:
        return 0x98;
    case 4:
        return 0x99;
    case 5:
        return 0x9a;
    case 6:
        return 0x9b;
    case 7:
        return 0x9c;
    case 8:
        return 0x9d;
    case 9:
        return 0x9e;
    case 10:
        return 0x9f;
    case 11:
        return 0xa0;
    case 12:
        return 0xa1;
    case 13:
        return 0xa2;
    case 14:
        return 0xa3;
    case 15:
        return 0xa4;
    case 16:
        return 0xa5;
    case 17:
        return 0xa6;
    case 18:
        return 0xa7;
    case 19:
        return 0xa8;
    case 20:
        return 0xa9;
    case 21:
        return 0xaa;
    case 22:
        return 0xab;
    }
}

#pragma optimize("a", on)
void far sub_s011_1432_1794(struct grp2 far *g, int type, char b, char c, char d)
{
    struct gent far *e;

    if (g->n < 4) {
        e = &g->ent[g->n];
        g->n++;
        e->type = type;
        e->b = b;
        e->c = c;
        e->d = d;
        e->e = D_db35[type].x25;
    }
}
#pragma optimize("", on)

/* sub_s011_1432_17fa: not decompiled: mov es from non-slot */
void far sub_s011_1432_17fa(void) { }

#pragma optimize("a", on)
void far sub_s011_1432_1968(struct level far *lv, int idx, int val, char flag)
{
    struct obj far *o;
    struct obj far *p;
    int i;

    if (!flag) {
        lv->tbl[idx].b = lv->tbl[idx].a = val | 0x80;
        return;
    }
    for (i = 0; i < lv->nobj; i++) {
        o = &lv->obj[i];
        if (o->type == 0 && o->x0a == idx)
            break;
    }
    if (i < lv->nobj) {
        o->x0c = val;
        lv->obj[o->x0b].x0c = val;
    }
}
#pragma optimize("", on)

/* sub_s011_1432_1a2a: not decompiled: mov es from non-slot */
void far sub_s011_1432_1a2a(void) { }

/* sub_s011_1432_1bc0: not decompiled: mov es from non-slot */
void far sub_s011_1432_1bc0(void) { }

/* sub_s011_1432_1cb4: not decompiled: read of undefined bx */
void far sub_s011_1432_1cb4(void) { }

void far sub_s011_1432_1d94(struct level far *lv, int n, int val)
{
    struct obj far *o;
    struct obj far *p;
    int i;
    int k;

    for (i = k = 0, p = lv->obj; i < 55; i++, p++) {
        *(struct obj far * volatile *)&o = p;
        if (p->type == 5) {
            if (n == k)
                break;
            k++;
        }
    }
    if (i < 55)
        o->x0c = val;
}
#pragma optimize("", on)

/* sub_s011_1432_1ddc: not decompiled: mov es from non-slot */
void far sub_s011_1432_1ddc(void) { }

void far sub_s011_1432_1f5c(int k)
{
    char name[0x42];

    if (k >= 0 && k < 4) {
        sprintf(name, "level%d.flr", D_e047[k].x1f40);
        sub_s004_1262_0d0a(name, (struct level far *)(D_e047 + k), 0x33fc, 0);
    }
}

int far sub_s011_1432_1fb2(int p_6, int p_8, int p_a, int count)
{
    char name[20];
    struct level far *lv;
    int n;

    sub_s004_0e76_20dc();
    sub_s011_1432_209e();
    n = sub_s004_0e76_20f6(1, 2);
    sprintf(name, "imisctom.%03d", n);
    lv = &F_20a5_0000;
    sub_root_05d9_019a(name, lv);
    F_20a5_0000.x1f40 = 0;
    for (n = 0; n < 55 && F_20a5_0000.obj[n].type == 6; n++)
        ;
    F_20a5_0000.nobj = n;
    F_20a5_0000.x1f42[0] = -2;
    F_20a5_0000.x1f42[1] = -2;
    F_20a5_0000.x1f42[2] = -2;
    F_20a5_0000.x1f42[3] = -2;
    F_20a5_0000.x1f42[4] = -2;
    sub_s011_1432_05c6(0, 0, (*(struct level far * volatile *)&lv));
    for (n = 1; n <= count; n++)
        sub_s011_1432_1a2a((*(struct level far * volatile *)&lv), n, 0x2d, 2, 1, -1, 0, 0, 0, 0);
    sub_s004_1262_0d0a("level0.flr", (*(struct level far * volatile *)&lv), 0x33fc, 0);
    return 1;
}

#pragma optimize("a", on)
void far sub_s011_1432_209e(void)
{
    char keep[5] = "keep";
    char keep1[6] = "keep1";

    D_db33->x00 = 0;
    _fstrcpy((char far *)D_db33->set0, (char far *)keep);
    D_db33->t0b = 2;
    _fstrcpy((char far *)D_db33->set2, (char far *)keep1);
    D_db33->t3e = 1;
    _fstrcpy((char far *)D_db33->set3, (char far *)keep1);
    D_db33->t71 = 1;
    _fstrcpy((char far *)D_db33->set4, (char far *)keep);
    D_db33->ta5[0] = 13;
    D_db33->ta5[1] = 14;
    D_db33->tab = 18;
    D_db33->a3 = 12;
    D_db33->tb0 = 16;
}
#pragma optimize("", on)
