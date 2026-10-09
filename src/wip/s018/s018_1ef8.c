/* place: s018 1ef8:0000 */
/* orig: size=04b6 CONST=7ddc+0002 _DATA=20de+0010 */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* status: wip 3/4, all functions written (027e, 02be, 03ae match). 0000: MSC 6.00A puts the
 * "monster" (M-path, stricmp m73/m90/m13) block inline and the spr=6/speed=20 block last; the
 * original has 6/20 falling into the common tail and the M-path out of line after it
 * (if/else order, goto and else-if forms all compile the same).
 * Combat projectile/effect list (overlay s018, child of s014):
 * 0000 add an effect record to D_944d[] (16 records of 0x1a bytes, count D_c180),
 * 027e remove record idx (shift the rest down), 02be walk a missile from its start
 * towards the target until sub_s014_1d69_000c (line of fire test) fails, 03ae count a
 * missile/effect kind in the far table F_20a5_3954[] (6-byte records, count D_c783, max 10). */

/* combatant record, 0x80 bytes, 13 of them at D_9c55 (party 0..4) */
struct cmb {
    char pad0[4];
    int x;          /* +04 */
    int y;          /* +06 */
    int f08;        /* +08 */
    int f0a;        /* +0a */
    char padc[10];
    char kind;      /* +16 animation slot */
    char pad17[3];
    char facing;    /* +1a */
    char pad1b[7];
    char weapon;    /* +22 */
    char pad23[0x36];
    char f59;       /* +59 */
    char pad5a[0x26];
};

/* effect / missile record, 0x1a bytes */
struct fx {
    char pad0[8];
    int x0;         /* +08 start */
    int y0;         /* +0a */
    int x1;         /* +0c target */
    int y1;         /* +0e */
    char tgt;       /* +10 */
    char moving;    /* +11 */
    char facing;    /* +12 */
    char who;       /* +13 */
    char sprite;    /* +14 */
    char f15;       /* +15 */
    char f16;       /* +16 */
    char speed;     /* +17 */
    char f18;       /* +18 */
    char pad19;
};

struct cnt6 { int id; char a; char b; char cnt; char pad; };

extern struct cmb D_9c55[];
extern struct fx D_944d[];
extern char D_c180;
extern int D_e0a3, D_e0a5, D_e0a7, D_e0a9;
extern char D_11ca;
extern char *D_1166[];
extern char D_ca57[];
extern int D_d68d[];
extern unsigned char D_c783;
extern struct cnt6 far F_20a5_3954[];

int stricmp(const char *, const char *);
void far sub_s014_0e76_465e(int);
int far sub_s014_1d69_000c(int, int, int);
void far sub_s018_1ef8_0000(int, int, int, int, int);
void far sub_s018_1ef8_027e(int);
void far sub_s018_1ef8_02be(int, int, int);
void far sub_s018_1ef8_03ae(int, char);

void far sub_s018_1ef8_0000(int who, int tgt, int type, int d, int e)
{
    struct fx *p;
    int n, speed, spr;
    int k;

    if (D_c180 == 0x11)
        sub_s018_1ef8_027e(0);
    n = D_c180;
    p = &D_944d[n];
    if (tgt != -1) {
        p->x1 = D_9c55[tgt].x;
        p->y1 = D_9c55[tgt].y;
    } else if (who >= 0) {
        p->x1 = D_9c55[who].f08;
        p->y1 = D_9c55[who].f0a;
    } else {
        p->x1 = D_e0a9;
        p->y1 = D_e0a7;
    }
    p->f18 = D_11ca;
    if (type == 0) {
        switch (who == -1 || who == -2 ? e : D_9c55[who].weapon) {
        case 0x17:
            spr = 5;
            speed = 15;
            break;
        case 0x18:
        case 0x19:
            spr = 4;
            speed = 15;
            break;
        case 0x1a:
            spr = 3;
            speed = 15;
            break;
        case 0x1b:
        case 0x1c:
        case 0x1d:
        case 0x1e:
        case 0x1f:
            spr = 0;
            speed = 30;
            sub_s014_0e76_465e(15);
            break;
        case 0x20:
        case 0x21:
            spr = 1;
            speed = 120;
            sub_s014_0e76_465e(11);
            break;
        }
    } else {
        k = D_9c55[who].kind;
        if (D_1166[k][0] == 'E' || D_1166[k][0] == 'e' || stricmp(D_1166[k], "m02") == 0 ||
            (D_1166[k][0] != 'M' && D_1166[k][0] != 'm')) {
            spr = 6;
            speed = 20;
        } else {
            speed = 20;
            if (stricmp(D_1166[k], "m73") == 0)
                spr = 8;
            else if (stricmp(D_1166[k], "m90") == 0 || stricmp(D_1166[k], "m13") == 0)
                spr = 9;
            else
                spr = 7;
        }
    }
    p->sprite = spr;
    p->speed = speed;
    if (who >= 0) {
        p->x0 = D_9c55[who].x;
        p->y0 = D_9c55[who].y;
        p->facing = D_9c55[who].facing;
        p->who = who;
        p->tgt = tgt;
        if (type >= 6) {
            p->moving = 0;
            p->f15 = d;
            p->f16 = e;
        } else if (D_ca57[who] != 1 && D_ca57[who] != 2) {
            p->moving = 1;
            sub_s018_1ef8_02be(n, who, tgt);
        } else {
            if (who < 5)
                D_d68d[who] |= 0x10;
            p->moving = 0;
        }
    } else {
        p->x0 = D_e0a5;
        p->y0 = D_e0a3;
        p->facing = d;
        p->who = -1;
        p->tgt = tgt;
        if (who != -1)
            p->moving = 1;
        else
            p->moving = 0;
    }
    D_c180++;
}

void far sub_s018_1ef8_027e(int idx)
{
    int i;

    if (D_c180) {
        for (i = idx; i < 16; i++)
            D_944d[i] = D_944d[i + 1];
        D_c180--;
    }
}

void far sub_s018_1ef8_02be(int n, int who, int tgt)
{
    int x0, x, y, dx, dy, save, shift, lim, i;

    D_e0a5 = D_944d[n].x0;
    D_e0a3 = D_944d[n].y0;
    x = D_944d[n].x1;
    x0 = D_e0a5;
    y = D_944d[n].y1;
    dx = x - x0;
    dy = y - D_e0a3;
    save = D_9c55[who].f59;
    D_9c55[who].f59 = 0x6e;
    lim = 6;
    i = shift = 0;
    do {
        D_e0a9 = x;
        D_e0a7 = y;
        if (sub_s014_1d69_000c(who, tgt, 0x80) == 0) {
            dx >>= shift;
            x += dx;
            dy >>= shift;
            y += dy;
            if (lim == 16)
                break;
        } else {
            if (lim == 6) {
                lim = 16;
                shift = 1;
                i = 0;
            }
            dx >>= shift;
            x -= dx;
            dy >>= shift;
            y -= dy;
        }
    } while (++i <= lim);
    D_9c55[who].f59 = save;
    D_944d[n].x1 = x;
    D_944d[n].y1 = y;
}

void far sub_s018_1ef8_03ae(int kind, char sub)
{
    struct cnt6 r;
    int i;

    r.b = sub;
    r.pad = r.cnt = 1;
    switch (kind) {
    case 0x17:
        r.id = 0x17;
        r.a = 0x17;
        break;
    case 0x18:
        r.id = 0x18;
        r.a = 0x18;
        r.pad = 3;
        break;
    case 0x19:
        r.id = 0x19;
        r.a = 0x19;
        break;
    case 0x1a:
        r.id = 1;
        r.a = 0x1a;
        break;
    case 0x40:
        r.id = 0x21;
        r.a = 0x40;
        break;
    case 0x41:
        r.id = 0x22;
        r.a = 0x41;
        break;
    }
    for (i = 0; i < D_c783; i++)
        if (F_20a5_3954[i].id == r.id && F_20a5_3954[i].a == r.a && F_20a5_3954[i].b == r.b) {
            F_20a5_3954[i].cnt++;
            break;
        }
    if (D_c783 <= i && D_c783 < 10) {
        F_20a5_3954[D_c783] = r;
        F_20a5_3954[D_c783].cnt = 1;
        D_c783++;
    }
}
