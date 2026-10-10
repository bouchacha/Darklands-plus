/* place: s004 1091:000a */
/* orig: size=1808 */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* status (e6, 2026-10-09): COMPLETE (13/13 written), 9/13 match; stamped (--wip).
 * Written from the asm (dec16 drafts were stubs). Turn / mirror a loaded tactical map (imaps.cat)
 * by r = 0..7 (r & 3 quarter turns clockwise, r >= 4 mirrored): 000a drives it, 00ca the cells
 * (0604 permutes the 4 wall nibbles of a cell) and heights, 074e the party start, 0800 passages,
 * 0bb6 objects, 0cca the part lists, 0da4 exits, 0fb6 chest groups, 1126 object extras (chest
 * contents, traps), 1334 raised rectangles, 145a doors; 17e0 turns a facing.
 * Coordinates: x pixels = col * 4 (0..0x500), y pixels = row * 2.
 * Not matching: 0604 (6.00A loads `hi & 0xf` as a byte), 0800 (3: arm order of `neg = x < 0 ? 1 : 0`),
 * 0cca (23), 1126 (85: 6.00 far CSE temps). Operand order of `0x140 - a - b` follows the declaration
 * order of the locals: re-sweep permutations (mkv) after changing declarations elsewhere.
 */

struct res { int a; int b; };

struct pas {                            /* 14 bytes at 0x1f54 */
    int x0, x2;
    unsigned char x4, x5;
    char p6[2];
    unsigned char x8, x9, x0a, x0b;
    char p0c[2];
};
struct obj {                            /* 14 bytes at 0x2186 */
    int x0, x2;
    unsigned char x4, x5;
    int type;                           /* 06 */
    int x8;
    int x0a;                            /* 0a: record index */
    int x0c;
};
struct exit {                           /* 14 bytes at 0x2fb8 */
    int x0, x2;
    unsigned char x4, x5;
    int x6;
    int x8, x0a;
    char x0c;
    char x0d;
};
struct grp {                            /* 0x2c bytes at 0x300e */
    int x0, x2;
    unsigned char x4, x5, x6, x7;
    char p8[0x2c - 8];
};
struct rec10 {                          /* 10 bytes at 0x316e */
    char x0, x1;
    int x2, x4;
    char x6;
    unsigned char x7;
    int x8;
};
struct rec12 {                          /* 12 bytes at 0x3236 */
    unsigned char a[4];
    unsigned char b[4];
    char p8[4];
};
struct rect {                           /* 10 bytes at 0x3328 */
    int x0, x2, x4, x6, x8;
};
struct doora {                          /* 10 bytes at 0x3350 */
    char x0;
    unsigned char x1, x2, x3, x4;
    unsigned char x5;
    int x6, x8;
};
struct doorb {                          /* 10 bytes at 0x3382 */
    char x0;
    unsigned char x1, x2;
    char x3;
    int x4, x6;
    unsigned char x8;
    char x9;
};
struct level {
    long cell[40][40];                  /* 0000 */
    unsigned char hgt[40][40];          /* 1900 */
    char p1f40[0x1f4c - 0x1f40];
    int x1f4c, x1f4e;                   /* 1f4c: party start */
    unsigned char x1f50;                /* 1f50: facing */
    char x1f51;
    char n1f52;                         /* 1f52 */
    char p1f53;
    struct pas pas[(0x2184 - 0x1f54) / 14];  /* 1f54 */
    char nobj;                          /* 2184 */
    char p2185;
    struct obj obj[55];                 /* 2186 */
    char ncomp;                         /* 2488 */
    char p2489;
    int comp[30];                       /* 248a */
    int list[(0x2fb6 - 0x24c6) / 2];    /* 24c6 */
    char nexit;                         /* 2fb6 */
    char p2fb7;
    struct exit ex[6];                  /* 2fb8 */
    char ngrp;                          /* 300c */
    char p300d;
    struct grp grp[8];                  /* 300e */
    struct rec10 r316e[20];             /* 316e */
    struct rec12 r3236[20];             /* 3236 */
    char nrect;                         /* 3326 */
    char p3327;
    struct rect rect[4];                /* 3328 */
    struct doora da[5];                 /* 3350 */
    struct doorb db[5];                 /* 3382 */
};

void far sub_s004_1091_000a(struct level far *, int, char);
void far sub_s004_1091_00ca(struct level far *, int);
long far sub_s004_1091_0604(long, int);
void far sub_s004_1091_074e(struct level far *, int);
void far sub_s004_1091_0800(struct level far *, int);
void far sub_s004_1091_0bb6(struct level far *, int);
void far sub_s004_1091_0cca(struct level far *, int);
void far sub_s004_1091_0da4(struct level far *, int, char);
void far sub_s004_1091_0fb6(struct level far *, int);
void far sub_s004_1091_1126(struct level far *, int);
void far sub_s004_1091_1334(struct level far *, int);
void far sub_s004_1091_145a(struct level far *, int);
unsigned char far sub_s004_1091_17e0(unsigned char, int);

/* turn / mirror the whole level */
#pragma optimize("a", on)
void far sub_s004_1091_000a(struct level far *lv, int r, char c)
{
    r %= 8;
    if (r < 0)
        r += (7 - r) & ~7;
    lv->x1f50 = sub_s004_1091_17e0(lv->x1f50, r);
    sub_s004_1091_00ca(lv, r);
    sub_s004_1091_074e(lv, r);
    sub_s004_1091_0800(lv, r);
    sub_s004_1091_0bb6(lv, r);
    sub_s004_1091_0cca(lv, r);
    sub_s004_1091_0da4(lv, r, c);
    sub_s004_1091_0fb6(lv, r);
    sub_s004_1091_1126(lv, r);
    sub_s004_1091_1334(lv, r);
    sub_s004_1091_145a(lv, r);
}
#pragma optimize("", on)

/* cells and heights */
#pragma optimize("a", on)
void far sub_s004_1091_00ca(struct level far *lv, int r)
{
    int i, j;
    long t;
    unsigned char h;

    for (i = 0; i < 20; i++) {
        for (j = 0; j < 20; j++) {
            switch (r) {
            case 1:
            case 5:
                t = sub_s004_1091_0604(lv->cell[i][j], r % 4);
                lv->cell[i][j] = sub_s004_1091_0604(lv->cell[j][39 - i], r % 4);
                lv->cell[j][39 - i] = sub_s004_1091_0604(lv->cell[39 - i][39 - j], r % 4);
                lv->cell[39 - i][39 - j] = sub_s004_1091_0604(lv->cell[39 - j][i], r % 4);
                lv->cell[39 - j][i] = t;
                h = lv->hgt[i][j];
                lv->hgt[i][j] = lv->hgt[j][39 - i];
                lv->hgt[j][39 - i] = lv->hgt[39 - i][39 - j];
                lv->hgt[39 - i][39 - j] = lv->hgt[39 - j][i];
                lv->hgt[39 - j][i] = h;
                break;
            case 2:
            case 6:
                t = sub_s004_1091_0604(lv->cell[i][j], r % 4);
                lv->cell[i][j] = sub_s004_1091_0604(lv->cell[39 - i][39 - j], r % 4);
                lv->cell[39 - i][39 - j] = t;
                t = sub_s004_1091_0604(lv->cell[39 - j][i], r % 4);
                lv->cell[39 - j][i] = sub_s004_1091_0604(lv->cell[j][39 - i], r % 4);
                lv->cell[j][39 - i] = t;
                h = lv->hgt[i][j];
                lv->hgt[i][j] = lv->hgt[39 - i][39 - j];
                lv->hgt[39 - i][39 - j] = h;
                h = lv->hgt[39 - j][i];
                lv->hgt[39 - j][i] = lv->hgt[j][39 - i];
                lv->hgt[j][39 - i] = h;
                break;
            case 3:
            case 7:
                t = sub_s004_1091_0604(lv->cell[i][j], r % 4);
                lv->cell[i][j] = sub_s004_1091_0604(lv->cell[39 - j][i], r % 4);
                lv->cell[39 - j][i] = sub_s004_1091_0604(lv->cell[39 - i][39 - j], r % 4);
                lv->cell[39 - i][39 - j] = sub_s004_1091_0604(lv->cell[j][39 - i], r % 4);
                lv->cell[j][39 - i] = t;
                h = lv->hgt[i][j];
                lv->hgt[i][j] = lv->hgt[39 - j][i];
                lv->hgt[39 - j][i] = lv->hgt[39 - i][39 - j];
                lv->hgt[39 - i][39 - j] = lv->hgt[j][39 - i];
                lv->hgt[j][39 - i] = h;
                break;
            }
        }
    }
    for (i = 0; i < 20; i++) {
        for (j = 0; j < 20; j++) {
            if (r >= 4) {
                t = sub_s004_1091_0604(lv->cell[i][j], 4);
                lv->cell[i][j] = sub_s004_1091_0604(lv->cell[39 - i][j], 4);
                lv->cell[39 - i][j] = t;
                t = sub_s004_1091_0604(lv->cell[i][39 - j], 4);
                lv->cell[i][39 - j] = sub_s004_1091_0604(lv->cell[39 - i][39 - j], 4);
                lv->cell[39 - i][39 - j] = t;
                h = lv->hgt[i][j];
                lv->hgt[i][j] = lv->hgt[39 - i][j];
                lv->hgt[39 - i][j] = h;
                h = lv->hgt[i][39 - j];
                lv->hgt[i][39 - j] = lv->hgt[39 - i][39 - j];
                lv->hgt[39 - i][39 - j] = h;
            }
        }
    }
}
#pragma optimize("", on)

/* permute the wall nibbles 0, 2, 4, 6 of a cell for turn q (q >= 4: mirrored) */
long far sub_s004_1091_0604(long c, int q)
{
    long n4, n0, n6, n2;

    n4 = ((volatile int *)&c)[1] & 0xf;
    n0 = c & 0xf;
    n6 = (c & 0x0f000000L) >> 24;
    n2 = (c >> 8) & 0xf;
    c &= 0xf0f0f0f0L;
    switch (q) {
    case 0:
        c |= (n6 << 16 | n2) << 8 | n4 << 16 | n0;
        break;
    case 1:
        c |= (n0 << 16 | n6) << 8 | n2 << 16 | n4;
        break;
    case 2:
        c |= (n4 << 16 | n0) << 8 | n6 << 16 | n2;
        break;
    case 3:
        c |= (n2 << 16 | n4) << 8 | n0 << 16 | n6;
        break;
    case 4:
        c |= (n6 << 16 | n0) << 8 | n4 << 16 | n2;
        break;
    case 5:
        c |= (n0 << 16 | n4) << 8 | n2 << 16 | n6;
        break;
    case 6:
        c |= (n4 << 16 | n2) << 8 | n6 << 16 | n0;
        break;
    case 7:
        c |= (n2 << 16 | n6) << 8 | n0 << 16 | n4;
        break;
    }
    return c;
}

/* party start position */
void far sub_s004_1091_074e(struct level far *lv, int r)
{
    int x, y;

    x = lv->x1f4c / 4;
    y = lv->x1f4e / 2;
    switch (r) {
    case 1:
    case 5:
        lv->x1f4c = (0x140 - y) << 2;
        lv->x1f4e = x << 1;
        break;
    case 2:
    case 6:
        lv->x1f4c = (0x140 - x) << 2;
        lv->x1f4e = (0x140 - y) << 1;
        break;
    case 3:
    case 7:
        lv->x1f4c = y << 2;
        lv->x1f4e = (0x140 - x) << 1;
        break;
    }
    if (r >= 4)
        lv->x1f4c = (0x140 - lv->x1f4c / 4) << 2;
}

/* passages */
#pragma optimize("a", on)
void far sub_s004_1091_0800(struct level far *lv, int r)
{
    struct pas far *p;
    int i;
    int x, w, h, y;
    int a16, a18, a0a, a0c;
    char neg, h1, h2;
    char b2, b11, b12, b13, b1, b5, b6, b7;

    for (i = 0; i < lv->n1f52; i++) {
        p = &lv->pas[i];
        neg = p->x0 >= 0 ? 0 : 1;
        h1 = p->x8 != 0;
        h2 = p->x0a != 0;
        if (neg)
            p->x0 = -p->x0;
        x = p->x0 / 4;
        y = p->x2 / 2;
        w = p->x4 >> 2;
        h = p->x5 >> 1;
        a16 = a18 = 0;
        b11 = b2 = b12 = b13 = 0;
        if (h1) {
            a16 = p->x8 & 0x3f;
            a18 = p->x9 & 0x3f;
            b11 = p->x8 >> 7;
            b2 = (p->x8 & 0x40) >> 6;
            b12 = p->x9 >> 7;
            b13 = (p->x9 & 0x40) >> 6;
        }
        a0a = a0c = 0;
        b5 = b1 = b6 = b7 = 0;
        if (h2) {
            a0a = p->x0a & 0x3f;
            a0c = p->x0b & 0x3f;
            b5 = p->x0a >> 7;
            b1 = (p->x0a & 0x40) >> 6;
            b6 = p->x0b >> 7;
            b7 = (p->x0b & 0x40) >> 6;
        }
        switch (r) {
        case 1:
        case 5:
            p->x0 = (0x140 - h - y) << 2;
            p->x2 = x << 1;
            p->x4 = h << 2;
            p->x5 = w << 1;
            if (h1) {
                p->x8 = (b13 << 1 | b11) << 6 | 0x27 - a18;
                p->x9 = (b2 << 1 | b12) << 6 | a16;
            }
            if (h2) {
                p->x0a = (b7 << 1 | b5) << 6 | 0x27 - a0c;
                p->x0b = (b1 << 1 | b6) << 6 | a0a;
            }
            break;
        case 2:
        case 6:
            p->x0 = (0x140 - w - x) << 2;
            p->x2 = (0x140 - y - h) << 1;
            if (h1) {
                p->x8 = (b12 << 1 | b13) << 6 | 0x27 - a16;
                p->x9 = (b11 << 1 | b2) << 6 | 0x27 - a18;
            }
            if (h2) {
                p->x0a = (b6 << 1 | b7) << 6 | 0x27 - a0a;
                p->x0b = (b5 << 1 | b1) << 6 | 0x27 - a0c;
            }
            break;
        case 3:
        case 7:
            p->x0 = y << 2;
            p->x2 = (0x140 - w - x) << 1;
            p->x4 = h << 2;
            p->x5 = w << 1;
            if (h1) {
                p->x8 = (b2 << 1 | b12) << 6 | a18;
                p->x9 = (b13 << 1 | b11) << 6 | 0x27 - a16;
            }
            if (h2) {
                p->x0a = (b1 << 1 | b6) << 6 | a0c;
                p->x0b = (b7 << 1 | b5) << 6 | 0x27 - a0a;
            }
            break;
        }
        if (r >= 4) {
            p->x0 = (0x140 - (unsigned char)(p->x4 >> 2) - p->x0 / 4) << 2;
            if (h1) {
                b2 = (p->x8 & 0x40) >> 6;
                p->x8 = p->x9 & 0x40 | 0x27 - (p->x8 & 0x3f) | p->x8 & 0x80;
                p->x9 = p->x9 & 0xbf | b2 << 6;
            }
            if (h2) {
                b1 = (p->x0a & 0x40) >> 6;
                p->x0a = p->x0b & 0x40 | 0x27 - (p->x0a & 0x3f) | p->x0a & 0x80;
                p->x0b = p->x0b & 0xbf | b1 << 6;
            }
        }
        if (neg)
            p->x0 = -p->x0;
    }
}
#pragma optimize("", on)

/* objects */
#pragma optimize("a", on)
void far sub_s004_1091_0bb6(struct level far *lv, int r)
{
    struct obj far *o;
    int i;
    int x, y, w, h;

    for (i = 0; i < lv->nobj; i++) {
        o = &lv->obj[i];
        x = o->x0 / 4;
        y = o->x2 / 2;
        w = o->x4 >> 2;
        h = o->x5 >> 1;
        switch (r) {
        case 1:
        case 5:
            o->x0 = (0x140 - h - y) << 2;
            o->x2 = x << 1;
            o->x4 = h << 2;
            o->x5 = w << 1;
            break;
        case 2:
        case 6:
            o->x0 = (0x140 - w - x) << 2;
            o->x2 = (0x140 - h - y) << 1;
            break;
        case 3:
        case 7:
            o->x0 = y << 2;
            o->x2 = (0x140 - w - x) << 1;
            o->x4 = h << 2;
            o->x5 = w << 1;
            break;
        }
        if (r >= 4)
            o->x0 = (0x140 - (unsigned char)(o->x4 >> 2) - o->x0 / 4) << 2;
    }
}
#pragma optimize("", on)
#pragma optimize("", on)

/* part lists: (col << 8) + row, -1 terminated */
void far sub_s004_1091_0cca(struct level far *lv, int r)
{
    int k;
    int far *q;
    unsigned w;
    unsigned x, y;

    for (k = 0; k < lv->ncomp; k++) {
        q = lv->list + lv->comp[k] / 2;
        while ((w = *q) != 0xffff) {
            x = w >> 8;
            y = w & 0xff;
            switch (r) {
            case 1:
            case 5:
                *q = ((0x27 - y) << 8) + x;
                break;
            case 2:
            case 6:
                *q = 0x2727 - (x << 8) - y;
                break;
            case 3:
            case 7:
                *q = (y << 8) - x + 0x27;
                break;
            }
            if (r >= 4)
                *q = (*q & 0xff) | (0x27 - ((unsigned)*q >> 8)) << 8;
            q++;
        }
    }
}

/* exits (f: also the far side position and facing) */
#pragma optimize("a", on)
void far sub_s004_1091_0da4(struct level far *lv, int r, char f)
{
    struct exit far *e;
    int i;
    int x, w, h, y;

    for (i = 0; i < lv->nexit; i++) {
        e = &lv->ex[i];
        x = e->x0 / 4;
        y = e->x2 / 2;
        w = e->x4 >> 2;
        h = e->x5 >> 1;
        switch (r) {
        case 1:
        case 5:
            e->x0 = (0x140 - h - y) << 2;
            e->x2 = x << 1;
            e->x4 = h << 2;
            e->x5 = w << 1;
            break;
        case 2:
        case 6:
            e->x0 = (0x140 - w - x) << 2;
            e->x2 = (0x140 - h - y) << 1;
            break;
        case 3:
        case 7:
            e->x0 = y << 2;
            e->x2 = (0x140 - w - x) << 1;
            e->x4 = h << 2;
            e->x5 = w << 1;
            break;
        }
        if (f) {
            x = e->x8 / 4;
            y = e->x0a / 2;
            e->x0c = sub_s004_1091_17e0(e->x0c, r % 4);
            switch (r) {
            case 1:
            case 5:
                e->x8 = (0x140 - y) << 2;
                e->x0a = x << 1;
                break;
            case 2:
            case 6:
                e->x8 = (0x140 - x) << 2;
                e->x0a = (0x140 - y) << 1;
                break;
            case 3:
            case 7:
                e->x8 = y << 2;
                e->x0a = (0x140 - x) << 1;
                break;
            }
        }
        if (r >= 4) {
            e->x0 = (0x140 - (unsigned char)(e->x4 >> 2) - e->x0 / 4) << 2;
            if (f) {
                e->x8 = (0x140 - e->x8 / 4) << 2;
                e->x0c = sub_s004_1091_17e0(e->x0c, 4);
            }
        }
    }
}
#pragma optimize("", on)

/* chest groups */
#pragma optimize("a", on)
void far sub_s004_1091_0fb6(struct level far *lv, int r)
{
    struct grp far *g;
    int i;
    int x, y, w, h, c6, c7;

    for (i = 0; i < lv->ngrp; i++) {
        g = &lv->grp[i];
        x = g->x0 / 4;
        y = g->x2 / 2;
        w = g->x4 >> 2;
        h = g->x5 >> 1;
        c6 = g->x6;
        c7 = g->x7;
        switch (r) {
        case 1:
        case 5:
            g->x0 = (0x140 - h - y) << 2;
            g->x2 = x << 1;
            g->x4 = h << 2;
            g->x5 = w << 1;
            g->x6 = 0x27 - c7;
            g->x7 = c6;
            break;
        case 2:
        case 6:
            g->x0 = (0x140 - w - x) << 2;
            g->x2 = (0x140 - h - y) << 1;
            g->x6 = 0x27 - c6;
            g->x7 = 0x27 - c7;
            break;
        case 3:
        case 7:
            g->x0 = y << 2;
            g->x2 = (0x140 - w - x) << 1;
            g->x4 = h << 2;
            g->x5 = w << 1;
            g->x6 = c7;
            g->x7 = 0x27 - c6;
            break;
        }
        if (r >= 4) {
            g->x0 = (0x140 - (unsigned char)(g->x4 >> 2) - g->x0 / 4) << 2;
            g->x6 = 0x27 - g->x6;
        }
    }
}
#pragma optimize("", on)

/* object records: type 1 (12-byte cell lists), type 4 (10-byte positioned records) */
void far sub_s004_1091_1126(struct level far *lv, int r)
{
    int k, j, id;
    int c, x, y;
    struct rec12 far *p;
    struct rec10 far *q;

    for (k = 0; k < lv->nobj; k++) {
        id = lv->obj[k].x0a;
        switch (lv->obj[k].type) {
        case 1:
            p = &lv->r3236[id];
            for (j = 0; j < 4; j++) {
                if (!p->a[j])
                    continue;
                c = p->a[j];
                y = p->b[j];
                switch (r) {
                case 1:
                case 5:
                    p->a[j] = 0x27 - y;
                    p->b[j] = c;
                    break;
                case 2:
                case 6:
                    p->a[j] = 0x27 - c;
                    p->b[j] = 0x27 - y;
                    break;
                case 3:
                case 7:
                    p->a[j] = y;
                    p->b[j] = 0x27 - c;
                    break;
                }
                if (r >= 4)
                    p->a[j] = 0x27 - p->a[j];
            }
            break;
        case 4:
            q = &lv->r316e[id];
            q->x7 = sub_s004_1091_17e0(q->x7, r);
            x = q->x2 / 4;
            y = q->x4 / 2;
            switch (r) {
            case 1:
            case 5:
                q->x2 = (0x140 - y) << 2;
                q->x4 = x << 1;
                break;
            case 2:
            case 6:
                q->x2 = (0x140 - x) << 2;
                q->x4 = (0x140 - y) << 1;
                break;
            case 3:
            case 7:
                q->x2 = y << 2;
                q->x4 = (0x140 - x) << 1;
                break;
            }
            if (r >= 4)
                q->x2 = (0x140 - q->x2 / 4) << 2;
            break;
        }
    }
}

/* raised rectangles */
#pragma optimize("a", on)
void far sub_s004_1091_1334(struct level far *lv, int r)
{
    struct rect far *p;
    int i;
    int w, h, x, y;

    for (i = 0; i < lv->nrect; i++) {
        p = &lv->rect[i];
        x = p->x0 / 4;
        y = p->x2 / 2;
        w = p->x4 / 4;
        h = p->x6 / 2;
        switch (r) {
        case 1:
        case 5:
            p->x0 = (0x140 - h - y) << 2;
            p->x2 = x << 1;
            p->x4 = h << 2;
            p->x6 = w << 1;
            break;
        case 2:
        case 6:
            p->x0 = (0x140 - x - w) << 2;
            p->x2 = (0x140 - h - y) << 1;
            break;
        case 3:
        case 7:
            p->x0 = y << 2;
            p->x2 = (0x140 - x - w) << 1;
            p->x4 = h << 2;
            p->x6 = w << 1;
            break;
        }
        if (r >= 4)
            p->x0 = (0x140 - p->x4 / 4 - p->x0 / 4) << 2;
    }
}
#pragma optimize("", on)

/* doors */
#pragma optimize("a", on)
void far sub_s004_1091_145a(struct level far *lv, int r)
{
    int k;
    struct doora far *a;
    struct doorb far *b;
    char h1, h2;
    int x, y, a8, aa, ac, ae;

    for (k = 0; k < 5; k++) {
        a = &lv->da[k];
        h1 = a->x1 != 0;
        h2 = a->x3 != 0;
        a->x5 = sub_s004_1091_17e0(a->x5, r);
        x = a->x6 / 4;
        y = a->x8 / 2;
        aa = a8 = 0;
        if (h1) {
            aa = a->x1;
            a8 = a->x2;
        }
        ac = ae = 0;
        if (h2) {
            ac = a->x3;
            ae = a->x4;
        }
        switch (r) {
        case 1:
        case 5:
            a->x6 = (0x140 - y) << 2;
            a->x8 = x << 1;
            if (h1) {
                a->x1 = 0x27 - a8;
                a->x2 = aa;
            }
            if (h2) {
                a->x3 = 0x27 - ae;
                a->x4 = ac;
            }
            break;
        case 2:
        case 6:
            a->x6 = (0x140 - x) << 2;
            a->x8 = (0x140 - y) << 1;
            if (h1) {
                a->x1 = 0x27 - aa;
                a->x2 = 0x27 - a8;
            }
            if (h2) {
                a->x3 = 0x27 - ac;
                a->x4 = 0x27 - ae;
            }
            break;
        case 3:
        case 7:
            a->x6 = y << 2;
            a->x8 = (0x140 - x) << 1;
            if (h1) {
                a->x1 = a8;
                a->x2 = 0x27 - aa;
            }
            if (h2) {
                a->x3 = ae;
                a->x4 = 0x27 - ac;
            }
            break;
        }
        if (r >= 4) {
            a->x6 = (0x140 - a->x6 / 4) << 2;
            if (h1)
                a->x1 = 0x27 - (aa = a->x1 >> 2);
            if (h2)
                a->x3 = 0x27 - (a->x3 >> 2);
        }
        b = &lv->db[k];
        h1 = b->x1 != 0;
        b->x8 = sub_s004_1091_17e0(b->x8, r);
        x = b->x4 / 4;
        y = b->x6 / 2;
        if (h1) {
            aa = b->x1;
            a8 = b->x2;
        }
        switch (r) {
        case 1:
        case 5:
            b->x4 = (0x140 - y) << 2;
            b->x6 = x << 1;
            if (h1) {
                b->x1 = 0x27 - a8;
                b->x2 = aa;
            }
            break;
        case 2:
        case 6:
            b->x4 = (0x140 - x) << 2;
            b->x6 = (0x140 - y) << 1;
            if (h1) {
                b->x1 = 0x27 - aa;
                b->x2 = 0x27 - a8;
            }
            break;
        case 3:
        case 7:
            b->x4 = y << 2;
            b->x6 = (0x140 - x) << 1;
            if (h1) {
                b->x1 = a8;
                b->x2 = 0x27 - aa;
            }
            break;
        }
        if (r >= 4) {
            b->x4 = (0x140 - b->x4 / 4) << 2;
            if (h1)
                b->x1 = 0x27 - (b->x1 >> 2);
        }
    }
}
#pragma optimize("", on)

/* turn a facing (0..7) by r */
unsigned char far sub_s004_1091_17e0(unsigned char f, int r)
{
    f += (r & 3) << 1;
    f &= 7;
    if (r >= 4)
        f = (8 - f) % 8;
    return f;
}
