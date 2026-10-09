/* place: s014 1ec9:0000 */
/* orig: size=02ec */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* s014 1ec9: sprite colour remapping and "is the line of fire blocked by an effect cloud".
   status: 4/4 written, 0 match - by design: the original functions contain inline assembly
   (0000/0070/00da: `lodsb / xlat ss:[bx] / stosb / loop` over a far sprite buffer;
   0142: `push word ptr [D_e0a5] ... pop word ptr [D_e0a3]` to save the line endpoints on the
   stack around a call), which the porting rules forbid here. The C below is functionally
   equivalent (for mod builds).

   Colour remap: palette entries 0xf3..0xff of combat sprites are "team colours". A remap
   table (256 bytes, identity outside 0xf3..0xff) maps the colours of the currently applied
   scheme to the new scheme, then every pixel of the sprite buffer is translated. */

struct cbt {                    /* combatant record, 0x80 bytes, D_9c55[13] */
    char pad0[4];
    int x;                      /* +04 */
    int y;                      /* +06 */
    char pad8[0x78];
};

struct efa {                    /* effect cloud, D_a2eb[8], count D_a33d */
    int x;                      /* +00 centre */
    int y;                      /* +02 */
    unsigned char f4;           /* +04 high nibble = size class (0..5) */
    unsigned char kind;         /* +05 */
    char f6;
    char f7;
    int t;                      /* +08 remaining time */
};

int far sub_s014_1d69_000c(int, int, int);
int far sub_s014_1d69_0354(void);

extern struct cbt D_9c55[];
extern struct efa D_a2eb[];
extern char D_a33d;

extern unsigned char D_d438;            /* scheme currently applied to buffer 1 */
extern unsigned char D_d4b0;            /* scheme currently applied to buffer 2 */
extern unsigned char D_1c4a[][3];       /* 3-colour schemes */
extern unsigned char D_1c5c[][4];       /* 4-colour schemes */
extern unsigned char D_c2cc[256];       /* remap table 1 */
extern unsigned char D_c3db[256];       /* remap table 2 */
extern unsigned char D_c500[256];       /* remap table 3 */
extern unsigned char D_c5eb[8];         /* = D_c500 + 0xeb: the 8 remapped entries */
extern unsigned D_d503;                 /* pixel count of buffer 1 */
extern unsigned D_d5b5;                 /* pixel count of buffer 2 */
extern unsigned D_ca2c;                 /* segment of buffer 1 */
extern unsigned D_cea4;                 /* segment of buffer 2 */

/* line-of-sight scratch: segment (D_e0a5,D_e0a3)-(D_e0a9,D_e0a7), box D_e1d5..D_e1db */
extern int D_e0a5, D_e0a3, D_e0a9, D_e0a7;
extern int D_e1d5, D_e1d7, D_e1d9, D_e1db;
extern char D_18b6[];                   /* half width of a cloud of size class n */
extern char D_18bc[];                   /* half height */
extern char *D_21b2[];                  /* per size class: x offsets of the n*n puffs */
extern char *D_21bc[];                  /* y offsets */

/* apply 3-colour scheme `s` to sprite buffer 1 */
void far sub_s014_1ec9_0000(int s)
{
    int i;
    unsigned c;
    unsigned char far *p;

    if (D_d438 != s) {
        for (i = 0xf3; i < 0x100; i++)
            D_c2cc[i] = i;
        for (i = 0; i < 3; i++)
            D_c2cc[D_1c4a[D_d438][i]] = D_1c4a[s][i];
        D_d438 = s;
        p = (unsigned char far *)((unsigned long)D_ca2c << 16 | 4);
        for (c = D_d503; c != 0; c--, p++)
            *p = D_c2cc[*p];
    }
}

/* apply 4-colour scheme `s` to sprite buffer 2 */
void far sub_s014_1ec9_0070(int s)
{
    int i;
    unsigned c;
    unsigned char far *p;

    if (D_d4b0 != s) {
        for (i = 0xf3; i < 0x100; i++)
            D_c3db[i] = i;
        for (i = 0; i < 4; i++)
            D_c3db[D_1c5c[D_d4b0][i]] = D_1c5c[s][i];
        D_d4b0 = s;
        p = (unsigned char far *)((unsigned long)D_cea4 << 16 | 4);
        for (c = D_d5b5; c != 0; c--, p++)
            *p = D_c3db[*p];
    }
}

/* recolour a sprite buffer: palette entries 0xeb..0xf2 -> (n + 10) * 8 + 0..7 */
void far sub_s014_1ec9_00da(int n, unsigned count, unsigned seg)
{
    int b;
    unsigned c;
    unsigned char far *p;

    b = (n + 10) << 3;
    D_c5eb[0] = b;
    D_c5eb[1] = b + 1;
    D_c5eb[2] = b + 2;
    D_c5eb[3] = b + 3;
    D_c5eb[4] = b + 4;
    D_c5eb[5] = b + 5;
    D_c5eb[6] = b + 6;
    D_c5eb[7] = b + 7;
    p = (unsigned char far *)((unsigned long)seg << 16 | 4);
    for (c = count; c != 0; c--, p++)
        *p = D_c500[*p];
}

/* 1 if the line from combatant a to combatant b passes through an effect cloud
   (smoke/fog etc.): first the cloud's bounding box, then every puff of it. */
int far sub_s014_1ec9_0142(int a, int b)
{
    struct cbt *c;
    int k;
    int sx0, sy0, sx1, sy1;
    int i;
    int n;
    int px, py;

    if (D_a33d == 0)
        return 0;
    c = &D_9c55[a];
    D_e0a5 = c->x;
    D_e0a3 = c->y;
    D_e0a9 = D_9c55[b].x;
    D_e0a7 = D_9c55[b].y;
    for (i = 0; i <= D_a33d; i++) {
        n = D_a2eb[i].f4 >> 4;
        D_e1d5 = D_a2eb[i].x - D_18b6[n];
        D_e1d9 = D_a2eb[i].y - D_18bc[n];
        D_e1d7 = D_18b6[n] + D_a2eb[i].x;
        D_e1db = D_18bc[n] + D_a2eb[i].y;
        if (sub_s014_1d69_0354() == 1) {
            sx0 = D_e0a5;
            sy0 = D_e0a3;
            sx1 = D_e0a9;
            sy1 = D_e0a7;
            D_e0a5 = D_a2eb[i].x;
            D_e0a3 = D_a2eb[i].y;
            for (k = 0; k < n * n; k++) {
                D_e0a9 = D_21b2[n][k] + D_a2eb[i].x;
                D_e0a7 = D_21bc[n][k] + D_a2eb[i].y;
                if (sub_s014_1d69_000c(-1, -1, 0) == 0) {
                    /* the puff is visible from the cloud centre: test its box */
                    px = D_e0a5;
                    py = D_e0a3;
                    D_e1d5 = D_e0a9 - 8;
                    D_e1d9 = D_e0a7 - 4;
                    D_e1d7 = D_e0a9 + 8;
                    D_e1db = D_e0a7 + 4;
                    D_e0a5 = sx0;
                    D_e0a3 = sy0;
                    D_e0a9 = sx1;
                    D_e0a7 = sy1;
                    if (sub_s014_1d69_0354() == 1)
                        return 1;
                    D_e0a3 = py;
                    D_e0a5 = px;
                }
            }
        }
        /* (the original also restores here when the box test failed) */
        D_e0a5 = sx0;
        D_e0a3 = sy0;
        D_e0a9 = sx1;
        D_e0a7 = sy1;
    }
    return 0;
}
