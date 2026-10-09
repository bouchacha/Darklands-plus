/* place: s031 1462:0000 */
/* flags: /AM /G2 /Gs /Ox /Oa /Zp1 */
/* Inventory screen (overlay s031, main file): the "arms" screen where a party member's
   equipment and pack are shown and items are moved, used, dropped or traded.
   STATUS (e1d, 2026-10-09): all 19 functions written, _DATA and CONST match; 16/19 match (+43b2 only its jump table).
     0124 [1 diff line]: `cmp si,di` for `m == who` (ours `cmp di,si`; who is region-cached in
       si there, m is the di register var).  Tried both operand orders, (m = ..) == who,
       who - m == 0, unsigned m/casts, renaming m and who, pragmas a/c/w/z, and a dummy
       extern at every top-level position: no flip.  (Frame fixed in e1d: [bp-8] is MSC's CSE
       of D_867c, not a named k; the ph+iy temp lands at [bp-0ch] once k is gone.)
     433c [1 instruction]: the original keeps a dead `mov word ptr [bp-2],80h` (b = 0x80 before
       the loops); 6.00A deletes it (constant propagation + global DSE, independent of
       /Oe /Og /Ol /Oc).  Everything else matches (no v local: the byte is a hoisted temp).
       Keeping it via volatile/union/struct/&b stops b being cached in bx.  433c is therefore
       5 bytes short and 43b2's jump table (absolute offsets) is off by 5.
     42ac matched in e1d: no `f` local (hoisted CSE temp) and a do-while loop (the for loop gave
       `mov cx,ax` instead of `mov cx,bx` in the i*3 return blocks).
     43b2: code identical; only its jump table differs (433c length).
     Notes: the box table externs are declared a few bytes early (see there) to get the
     original operand order of `x + w`; D_867e must be unsigned char or `1d44(D_867e = 0, 12)`
     pushes an immediate.  Declarations anywhere in the file can flip operand orders in
     other functions (symbol table), so re-run fmatch on the whole file after any edit. */

struct gfx { char pad[12]; int color; char pad2[4]; };     /* 0x12 bytes, D_27f4/D_2806 */

struct box {                        /* screen rectangles, 0x24 bytes */
    char lead[2];                   /* see the note at the box table externs */
    int x, y, w, h;                 /* +00 */
    unsigned char color;            /* +08  frame colour */
    unsigned char flags;            /* +09  1: active, 2: has a list, 0x10: holds items */
    int ix, iy;                     /* +0a  icon position */
    unsigned char pw, ph;           /* +0e */
    char pic;                       /* +10  index into D_8682[] */
    char pad11;
    int nvis;                       /* +12  visible lines */
    unsigned char top[5];           /* +14  first visible line, per member */
    unsigned char sel[5];           /* +19  selected line, per member */
    unsigned char up, down, left, right;    /* +1e  neighbour boxes for the cursor keys */
};                                  /* (field offsets above are from the real record start) */

/* party member record in DGROUP (0x80 bytes) */
struct member {
    char present;               /* +00 */
    char pad01[0x0d];
    char missile_type;          /* +0e  equipped missile weapon (item type, -1 none) */
    char pad0f;
    unsigned char x10;          /* +10 */
    char full_name[0x19];       /* +11 */
    char name[0x0a];            /* +2a */
    unsigned char x34;          /* +34  two nibbles */
    char pad35[2];              /* +35  +35: encumbrance */
    char vital_type;            /* +37  vital armour */
    char limb_type;             /* +38  limb armour */
    char pad39[2];
    char vital_q;               /* +3b */
    char limb_q;                /* +3c */
    char weapon_type;           /* +3d */
    char pad3e[3];
    unsigned char x41;          /* +41  two nibbles */
    char pad42[2];
    char weapon_q;              /* +44 */
    char pad45;
    char missile_q;             /* +46 */
    char shield_q;              /* +47 */
    char shield_type;           /* +48 */
    char attr[7];               /* +49  End Str Agl Per Int Chr DF: current */
    char attr_max[7];           /* +50  maximum */
    char skill[19];             /* +57  wEdg..wMsD, Alch..R&W, Heal..WdWs */
    char nitems;                /* +6a */
    char pad6b[0x15];
};

/* item definition (DARKLAND.LST) */
struct itemdef {
    char name[0x14];
    char short_name[0x0a];
    char type;                  /* +1e */
    char pad1f;
    long flags;                 /* +20 */
    char pad24;
    char weight;                /* +25 */
    char quality;               /* +26 */
};

/* one inventory entry (6 bytes) */
struct item {
    int code;                   /* LST item code (low 12 bits) */
    unsigned char type;         /* LST item type */
    unsigned char qual;         /* quality */
    unsigned char cnt;          /* quantity */
    unsigned char weight;
};

/* character record in far data (0x180 bytes); the items run on past the record */
struct fchar {
    char pad00[0x3e];
    struct item items[53];
    char pad17c[4];
};

#define FP_SEG(fp) (*((unsigned far *)&(fp) + 1))
#define FP_OFF(fp) (*((unsigned far *)&(fp)))

extern int D_a67e;                       /* party size */
extern struct member D_9c69[];
extern char D_e48c[];
extern struct gfx D_27f4, D_2806;
extern int D_9c42;
extern int D_e7a4;
extern int D_906b;
extern int D_9c46;
extern char D_907a, D_907c;
extern char D_a6b4[];
extern char D_9419[];
extern unsigned char D_00e8[];            /* bit masks 0x80 >> n */                       /* number of lines in the list */
extern volatile char D_6ef0;             /* mouse buttons */
extern volatile int D_6eec, D_6eee;               /* mouse x, y */
extern unsigned D_867a, D_8680, D_8678;
extern unsigned D_8682[];                /* brush pictures */
extern unsigned char D_867c;
extern char D_907b;
extern unsigned char D_867d;
extern unsigned char D_867e;
extern char D_9920, D_9934;
extern int D_2816, D_2804, D_e7a2, D_e7a0, D_2800;
extern unsigned char far *D_a889;        /* darkland.lst buffer */
extern unsigned char far *D_a885;        /* location file buffer */
extern char far *D_9785;                 /* city record */

extern char far * far F_2e38_1b2c[];     /* event records (0x30 bytes each), 300 slots */
extern char far * far F_2e38_0890[];     /* item definitions */
extern char far * far F_2e38_1736[];     /* names, 3 per entry of F_2e38_07be */
extern char far * far F_2e38_183e[];     /* names, 8 per byte of F_2e38_082c */
extern char far * far F_290e_1e67[];     /* attribute and skill labels */
extern struct fchar far F_2e38_0000[];
extern struct box far F_290e_3557[];             /* = 290e:3559, the screen's boxes */
/* The box tables (far, 290e:3559 and 290e:3829) are declared a few bytes early, with a
   `lead` in struct box: with the base symbols at the real addresses MSC 6.00A loads `w` before
   `x` in `x + w` (the operand order of two far struct fields depends on the symbol and offset);
   declared this way it matches the original. */
struct box_l2 { char lead[4]; int x, y, w, h; char pad[0x18]; };   /* only the rectangle */
extern struct box_l2 far F_290e_3825[];          /* = 290e:3829, frame boxes (4 bytes early) */
extern unsigned char far F_2e38_07be[][22];
extern unsigned char far F_2e38_082c[][20];

char *strcpy(char *, const char *);
char *strcat(char *, const char *);
char *itoa(int, char *, int);
int sprintf(char *, const char *, ...);
unsigned _dos_write(int, void far *, unsigned, unsigned *);
unsigned _dos_close(int);
unsigned _dos_creat(char *, unsigned, int *);
void far free_buffer(unsigned seg, char *name);

void far F_0000_2652(int);
void far vec_18e7(char *);
void far vec_1905(char *, char *);
unsigned far sub_root_047c_04d5(int, int, int, int, int);
void far sub_root_047c_1293(void);
void far sub_root_047c_12c3(void);
void far sub_root_047c_0568(void);
void far sub_root_047c_0545(void);
void far sub_root_047c_04f8(void);
void far sub_root_047c_056f(void);
void far sub_root_047c_058b(char *);
void far sub_root_047c_124f(int, int, int);
void far sub_root_047c_05a5(struct gfx *, int, int, unsigned);
void far sub_root_047c_0553(struct gfx *, int, int, int, int, struct gfx *, int, int);
void far sub_root_047c_0d62(struct gfx *, int, int, int, int, int);
void far sub_root_0617_024a(int, char *, char *);
void far sub_root_047c_055a(struct gfx *, int, int, char *);
void far sub_root_0265_0004(struct gfx *, int, int, char far *);
int far sub_s029_0e76_013a(int);
int far sub_s029_0e76_3996(int, int);
int far sub_s029_1367_0066(int, int, int);
int far sub_s031_18e7_14b6(int who, int slot);
void far sub_root_047c_04dc(struct gfx *, int, int, int, int, int, int);
void far sub_s031_18e7_0000(int from, int to, int idx, int slot);
void far sub_s031_18e7_007c(int from, int to, int idx, int slot);
void far sub_s031_18e7_10d2(int who, int a, int b);
void far sub_s031_18e7_0474(int who, int idx);
void far sub_s031_18e7_07d4(int who, int idx);
void far sub_s031_18e7_12e8(int who, int idx, int slot);
void far sub_root_0000_13a0(void);
void far sub_s031_18e7_11b2(int who, int idx);
void far vec_2017(int, int);
void far vec_1fef(int, int);
void far vec_1d51(int);
void far sub_root_0000_0000(char *, int);
int far sub_root_047c_073d(void);
int far sub_root_047c_0744(void);
void far vec_200d(int, int, int);
unsigned far sub_root_0617_045c(char *);
void far sub_root_0000_1588(int);
void far sub_root_0000_1716(int);
int far sub_root_0000_0c70(int);
void far sub_s029_0e76_4d24(int, int, int, int);
void far sub_s029_0e76_4b08(int, int, int, int);
void far sub_s029_0e76_4c04(int, int, int, int);
void far sub_s029_0e76_08f2(int, int, int);

int far sub_s031_1462_0124(int who);
int far sub_s031_1462_08f6(int r);
void far sub_s031_1462_0950(int k);
void far sub_s031_1462_09b8(int who);
void far sub_s031_1462_0a82(int who);
void far sub_s031_1462_0b22(int who);
void far sub_s031_1462_10ae(int key, int who);
void far sub_s031_1462_1e42(int k, int who, int y);
void far sub_s031_1462_1d44(unsigned char k, int color);
void far sub_s031_1462_2518(int k, int color);
void far sub_s031_1462_264c(int k, int who);
int far sub_s031_1462_42ac(int who, int n);
int far sub_s031_1462_433c(int who, int n);
void far sub_s031_1462_25de(int k, int who);
void far sub_s031_1462_43b2(int who, int idx);
void far sub_s031_1462_2b14(int k, int who);
void far sub_s031_1462_39a0(int i, int who, struct gfx g);
void far sub_s031_1462_39c6(int i, int who, struct gfx g);

/* leave the inventory screen: save locs.tmp / events.tmp and free the world data */
int far sub_s031_1462_0000(int who)
{
    int fd, fd2;
    unsigned nw;
    int i;
    int nev;

    if (D_a67e <= who)
        return -1;
    F_0000_2652(0x60);
    vec_18e7("locs.tmp");
    sub_root_047c_1293();
    sub_s031_1462_0124(who);
    _dos_creat("events.tmp", 0, &fd2);
    nev = 0;
    for (i = 0; i < 300; i++)
        if (F_2e38_1b2c[i])
            nev++;
    _dos_write(fd2, (void far *)&nev, 2, &nw);
    for (i = 0; i < 300; i++) {
        if (F_2e38_1b2c[i]) {
            _dos_write(fd2, F_2e38_1b2c[i], 0x30, &nw);
            free_buffer(FP_SEG(F_2e38_1b2c[i]), "e");
            F_2e38_1b2c[i] = 0;
        }
    }
    _dos_close(fd2);
    free_buffer(FP_SEG(D_a889), "S");
    free_buffer(FP_SEG(D_9785), "C");
    free_buffer(FP_SEG(D_a885), "M");
    sub_root_047c_0568();
}

int far sub_s031_1462_0124(int who)
{
    int key, m, i;

    if (D_9c69[who].present == 0 || D_9c69[who].present == 5)
        return -1;
    D_9920 = 2;
    D_9934 = 2;
    D_907c = 2;
    D_2800 = 0xbf;
    D_867c = 0;
    D_867d = who;
    sub_root_0000_1588(0x100);
    sub_root_0000_1716(0x100);
    sub_s031_1462_09b8(who);
    sub_s031_1462_1d44(D_867e = 0, 12);
    while (D_6ef0 & 1)
        ;
    for (;;) {
        key = 0;
        if (D_6ef0 & 1) {
            if (D_6eec > 0x3c) {
                if (D_867e && (F_290e_3557[D_867c].flags & 2)
                        && F_290e_3557[D_867c].ix <= D_6eec && F_290e_3557[D_867c].pw + F_290e_3557[D_867c].ix >= D_6eec
                        && F_290e_3557[D_867c].iy + 9 <= D_6eee && F_290e_3557[D_867c].ph + F_290e_3557[D_867c].iy - 0xe >= D_6eee) {
                    sub_s031_1462_1e42(D_867c, who, D_6eee);
                } else if (D_867e && (F_290e_3557[D_867c].flags & 2)
                        && F_290e_3557[D_867c].ix <= D_6eec && F_290e_3557[D_867c].pw + F_290e_3557[D_867c].ix >= D_6eec
                        && F_290e_3557[D_867c].iy <= D_6eee && F_290e_3557[D_867c].iy + 9 >= D_6eee) {
                    while (D_6ef0 & 1) {
                        D_867c;
                        if (F_290e_3557[D_867c].sel[who] == 0) {
                            if (F_290e_3557[D_867c].top[who] != 0)
                                F_290e_3557[D_867c].top[who]--;
                            sub_s031_1462_25de(D_867c, who);
                        } else {
                            sub_root_047c_12c3();
                            sub_root_047c_04dc(&D_27f4, F_290e_3557[D_867c].ix + 8, F_290e_3557[D_867c].sel[who] * 8 + F_290e_3557[D_867c].iy + 9,
                                               F_290e_3557[D_867c].pw - 0xf, 8, 9, 8);
                            F_290e_3557[D_867c].sel[who]--;
                            sub_root_047c_04dc(&D_27f4, F_290e_3557[D_867c].ix + 8, F_290e_3557[D_867c].sel[who] * 8 + F_290e_3557[D_867c].iy + 9,
                                               F_290e_3557[D_867c].pw - 0xf, 8, 8, 9);
                            sub_root_047c_1293();
                        }
                        vec_1fef(200, 0);
                    }
                } else if (D_867e && (F_290e_3557[D_867c].flags & 2)
                        && F_290e_3557[D_867c].ix <= D_6eec && F_290e_3557[D_867c].pw + F_290e_3557[D_867c].ix >= D_6eec
                        && F_290e_3557[D_867c].ph + F_290e_3557[D_867c].iy - 0xe <= D_6eee && F_290e_3557[D_867c].ph + F_290e_3557[D_867c].iy >= D_6eee) {
                    while (D_6ef0 & 1) {
                        D_867c;
                        if (F_290e_3557[D_867c].sel[who] >= F_290e_3557[D_867c].nvis - 1) {
                            if (F_290e_3557[D_867c].top[who] + F_290e_3557[D_867c].sel[who] >= D_e7a4)
                                break;
                            F_290e_3557[D_867c].top[who]++;
                            sub_s031_1462_25de(D_867c, who);
                        } else {
                            if (F_290e_3557[D_867c].top[who] + F_290e_3557[D_867c].sel[who] >= D_e7a4 - 1)
                                break;
                            sub_root_047c_12c3();
                            sub_root_047c_04dc(&D_27f4, F_290e_3557[D_867c].ix + 8, F_290e_3557[D_867c].sel[who] * 8 + F_290e_3557[D_867c].iy + 9,
                                               F_290e_3557[D_867c].pw - 0xf, 8, 9, 8);
                            F_290e_3557[D_867c].sel[who]++;
                            sub_root_047c_04dc(&D_27f4, F_290e_3557[D_867c].ix + 8, F_290e_3557[D_867c].sel[who] * 8 + F_290e_3557[D_867c].iy + 9,
                                               F_290e_3557[D_867c].pw - 0xf, 8, 8, 9);
                            sub_root_047c_1293();
                        }
                        vec_1fef(200, 0);
                    }
                } else {
                    for (i = 0; i < 18; i++)
                        if (F_290e_3557[i].x <= D_6eec && F_290e_3557[i].w + F_290e_3557[i].x >= D_6eec
                                && F_290e_3557[i].y <= D_6eee && F_290e_3557[i].y + F_290e_3557[i].h >= D_6eee)
                            break;
                    if (i == 18)
                        i = 0;
                    if (D_867e && D_867a) {
                        sub_root_047c_12c3();
                        if (D_867a) {
                            sub_root_047c_05a5(&D_27f4, F_290e_3557[D_867c].ix - 1, F_290e_3557[D_867c].iy, D_867a);
                            free_buffer(D_867a, "b");
                            D_867a = 0;
                        }
                        sub_root_047c_1293();
                    }
                    if (D_867e && (!D_867e || D_867c == i)) {
                        sub_s031_1462_1d44(D_867c, 12);
                        D_867e = 0;
                        sub_root_047c_12c3();
                        sub_root_047c_124f(0, 0, D_9c42);
                        sub_root_047c_1293();
                    } else {
                        sub_s031_1462_1d44(D_867c, F_290e_3557[D_867c].color);
                        sub_s031_1462_264c(i, who);
                        D_867c = i;
                    }
                }
            } else {
                m = D_6eee / 40;
                if (m == who)
                    return sub_s031_1462_08f6(-1);
                if (D_9c69[m].present == 1) {
                    if (D_907a == 1 && D_a6b4[m] == (char)0x9d)
                        vec_1d51(m);
                    else {
                        who = m;
                        D_867d = m;
                        sub_root_0000_1588(0x100);
                        sub_root_0000_1716(0x100);
                        sub_s031_1462_2518(m, 11);
                        D_867e = 0;
                        if (D_867a) {
                            free_buffer(D_867a, "b");
                            D_867a = 0;
                        }
                        sub_s031_1462_0a82(who);
                        sub_s031_1462_1d44(D_867c, 12);
                    }
                }
            }
            while (D_6ef0 & 1)
                ;
        }
        sub_root_0000_0000(D_9419, 0);
        if (D_9c46) {
            D_9c46 = 0;
            sub_root_0000_1588(0x100);
            sub_root_0000_1716(0x100);
            sub_s031_1462_2518(who, 11);
            D_867e = 0;
            if (D_867a) {
                free_buffer(D_867a, "b");
                D_867a = 0;
            }
            sub_s031_1462_0a82(who);
        }
        if (sub_root_047c_073d()) {
            key = sub_root_047c_0744();
            if (key == 0x3b00 || key == 0x3c00 || key == 0x3d00 || key == 0x3e00 || key == 0x3f00) {
                m = (key >> 8) - 0x3b;
                if (m == who)
                    return sub_s031_1462_08f6(-1);
                if (D_9c69[m].present == 1) {
                    if (D_907a == 1 && D_a6b4[m] == (char)0x9d)
                        vec_1d51(m);
                    else {
                        who = m;
                        D_867d = m;
                        sub_root_0000_1588(0x100);
                        sub_root_0000_1716(0x100);
                        sub_s031_1462_2518(m, 11);
                        D_867e = 0;
                        if (D_867a) {
                            free_buffer(D_867a, "b");
                            D_867a = 0;
                        }
                        sub_s031_1462_0a82(m);
                        sub_s031_1462_1d44(D_867c, 12);
                    }
                }
            } else if (key == 0x1c0d && D_867c >= 12 && D_867c <= 16) {
                    m = D_867c - 12;
                    if (D_9c69[m].present == 0)
                        return;
                    if (D_907a == 1 && D_a6b4[m] == (char)0x9d)
                        vec_1d51(m);
                    else if (who != m) {
                        who = m;
                        D_867d = m;
                        sub_root_0000_1588(0x100);
                        sub_root_0000_1716(0x100);
                        sub_s031_1462_2518(m, 11);
                        D_867e = 0;
                        if (D_867a) {
                            free_buffer(D_867a, "b");
                            D_867a = 0;
                        }
                        sub_s031_1462_0a82(m);
                    } else
                        key = 0x11b;
            }
            if (key == 0x11b) {
                sub_s031_1462_1d44(D_867c, F_290e_3557[D_867c].color);
                return sub_s031_1462_08f6(-1);
            }
        }
        if (key)
            sub_s031_1462_10ae(key, who);
    }
}

int far sub_s031_1462_08f6(int r)
{
    D_9920 = 1;
    D_9934 = 1;
    if (D_867a)
        free_buffer(D_867a, "b");
    D_867a = 0;
    sub_root_047c_124f(0, 0, D_9c42);
    if (D_8680)
        free_buffer(D_8680, "h");
    D_2816 = D_e7a2;
    D_2804 = D_e7a0;
    return r;
}

void far sub_s031_1462_0950(int k)
{
    sub_root_047c_12c3();
    if (D_867a) {
        sub_root_047c_05a5(&D_27f4, F_290e_3557[k].ix - 1, F_290e_3557[k].iy, D_867a);
        free_buffer(D_867a, "b");
        D_867a = 0;
    }
    sub_root_047c_124f(0, 0, D_9c42);
    sub_root_047c_1293();
    D_867e = 0;
}

void far sub_s031_1462_09b8(int who)
{
    D_e7a2 = D_2816;
    D_e7a0 = D_2804;
    D_2816 = 1;
    D_2804 = 1;
    sub_s031_1462_2518(who, 11);
    if (D_867a) {
        free_buffer(D_867a, "b");
        D_867a = 0;
    }
    sub_root_0617_024a(1, "pics\\armback.pic", D_e48c);
    sub_root_047c_0545();
    D_8680 = sub_root_0617_045c("pics\\handicon.pic");
    D_8678 = sub_root_0617_045c("pics\\handicn2.pic");
    D_8682[1] = sub_root_0617_045c("pics\\armbrsh1.pic");
    D_8682[2] = sub_root_0617_045c("pics\\armbrsh2.pic");
    D_8682[3] = sub_root_0617_045c("pics\\armbrsh3.pic");
    D_8682[4] = sub_root_0617_045c("pics\\armbrsh4.pic");
    D_8682[5] = sub_root_0617_045c("pics\\armbrsh5.pic");
    sub_root_047c_04f8();
    sub_s031_1462_0a82(who);
}

void far sub_s031_1462_0a82(int who)
{
    int i;

    D_2800 = 0xbf;
    sub_root_0617_024a(1, "pics\\armback.pic", D_e48c);
    sub_root_047c_0568();
    sub_root_047c_12c3();
    sub_root_047c_058b(D_e48c);
    sub_s031_1462_0b22(who);
    sub_root_047c_0553(&D_2806, 0x3c, 0, 0x104, 0xc8, &D_27f4, 0x3c, 0);
    for (i = 0; i < 17; i++)
        sub_s031_1462_39a0(i, who, D_27f4);
    sub_root_047c_124f(0, 0, D_9c42);
    sub_root_047c_1293();
    sub_root_047c_056f();
}

void far sub_s031_1462_0b22(int who)
{
    unsigned pic, wp;
    int n;
    char num[8];
    char name[20];

    sub_root_047c_0545();
    pic = sub_root_0617_045c("pics\\armsback.pic");
    sub_root_047c_04f8();
    if (pic)
        sub_root_047c_05a5(&D_2806, 0x3b, 0xd, pic);
    if (pic)
        free_buffer(pic, "c");
    sub_root_047c_0545();
    switch (D_9c69[who].limb_type) {
    case 0x4c:
        pic = sub_root_0617_045c("pics\\pad-lims.pic");
        break;
    case 0x4d:
        pic = sub_root_0617_045c("pics\\pad-lims.pic");
        break;
    case 0x4e:
        pic = sub_root_0617_045c("pics\\leat-lim.pic");
        break;
    case 0x4f:
        pic = sub_root_0617_045c("pics\\studlim.pic");
        break;
    case 0x50:
        pic = sub_root_0617_045c("pics\\cuirblim.pic");
        break;
    case 0x51:
        pic = sub_root_0617_045c("pics\\scalelim.pic");
        break;
    case 0x52:
        pic = sub_root_0617_045c("pics\\chainlim.pic");
        break;
    case 0x53:
        pic = sub_root_0617_045c("pics\\briglim.pic");
        break;
    case 0x54:
        pic = sub_root_0617_045c("pics\\platelim.pic");
        break;
    }
    sub_root_047c_04f8();
    if (pic)
        sub_root_047c_05a5(&D_2806, 0x3b, 0xd, pic);
    if (pic)
        free_buffer(pic, "c");
    sub_root_047c_0545();
    switch (D_9c69[who].vital_type) {
    case 0x43:
        pic = sub_root_0617_045c("pics\\pad-vit.pic");
        break;
    case 0x44:
        pic = sub_root_0617_045c("pics\\pad-vit.pic");
        break;
    case 0x45:
        pic = sub_root_0617_045c("pics\\leat-vit.pic");
        break;
    case 0x46:
        pic = sub_root_0617_045c("pics\\studvit.pic");
        break;
    case 0x47:
        pic = sub_root_0617_045c("pics\\cuirbvit.pic");
        break;
    case 0x48:
        pic = sub_root_0617_045c("pics\\scalevit.pic");
        break;
    case 0x49:
        pic = sub_root_0617_045c("pics\\chainvit.pic");
        break;
    case 0x4a:
        pic = sub_root_0617_045c("pics\\brigvit.pic");
        break;
    case 0x4b:
        pic = sub_root_0617_045c("pics\\platevit.pic");
        break;
    }
    sub_root_047c_04f8();
    if (pic)
        sub_root_047c_05a5(&D_2806, 0x3b, 0xd, pic);
    if (pic)
        free_buffer(pic, "c");
    if (D_9c69[who].weapon_type != -1) {
        switch (D_9c69[who].weapon_type) {
        case 0:
            n = 2;
            break;
        case 1:
            n = 3;
            break;
        case 2:
            n = 4;
            break;
        case 3:
            n = 5;
            break;
        case 4:
            n = 6;
            break;
        case 5:
            n = 7;
            break;
        case 6:
            n = 8;
            break;
        case 7: n = 0; break;
        case 8:
            n = 9;
            break;
        case 9: n = 1; break;
        case 10:
            n = 11;
            break;
        case 11:
            n = 12;
            break;
        case 12:
            n = 13;
            break;
        case 13:
            n = 14;
            break;
        case 14:
            n = 15;
            break;
        case 15:
            n = 16;
            break;
        case 16:
            n = 17;
            break;
        case 17:
            n = 18;
            break;
        case 18:
            n = 19;
            break;
        case 19:
            n = 20;
            break;
        case 20:
            n = 21;
            break;
        case 21:
            n = 22;
            break;
        case 22:
            n = 24;
            break;
        case 23:
            n = 0;
            break;
        case 24:
            n = 26;
            break;
        case 25:
            n = 27;
            break;
        case 26:
            n = 1;
            break;
        case 27:
            n = 28;
            break;
        case 28:
            n = 29;
            break;
        case 29:
            n = 30;
            break;
        case 30:
            n = 31;
            break;
        case 31:
            n = 32;
            break;
        case 32:
            n = 33;
            break;
        case 33:
            n = 34;
            break;
        default:
            n = -1;
            break;
        }
        if (n != -1) {
            strcpy(name, "pics\\weapon");
            itoa(n, num, 10);
            strcat(name, num);
            strcat(name, ".pic");
            sub_root_047c_0545();
            wp = sub_root_0617_045c(name);
            sub_root_047c_04f8();
            if (wp)
                sub_root_047c_05a5(&D_2806, 0x3b, 0xd, wp);
            if (wp)
                free_buffer(wp, "l");
        }
    }
    if (D_9c69[who].shield_type != -1) {
        strcpy(name, "pics\\");
        if (D_9c69[who].shield_type == 0x5f)
            strcat(name, "smallsh.pic");
        else if (D_9c69[who].shield_type == 0x60)
            strcat(name, "mediumsh.pic");
        else
            strcat(name, "mediumsh.pic");
        sub_root_047c_0545();
        pic = sub_root_0617_045c(name);
        sub_root_047c_04f8();
        if (pic)
            sub_root_047c_05a5(&D_2806, 0x3b, 0xd, pic);
        if (pic)
            free_buffer(pic, "l");
    }
    if (D_9c69[who].missile_type != -1) {
        switch (D_9c69[who].missile_type) {
        case 23:
            n = 23;
            break;
        case 24:
            n = 26;
            break;
        case 25:
            n = 27;
            break;
        case 26:
            n = 1;
            break;
        case 27:
            n = 28;
            break;
        case 28:
            n = 29;
            break;
        case 29:
            n = 30;
            break;
        case 30:
            n = 31;
            break;
        case 31:
            n = 32;
            break;
        case 32:
            n = 33;
            break;
        case 33:
            n = 34;
            break;
        default:
            n = -1;
            break;
        }
        if (n != -1) {
            strcpy(name, "pics\\weapon");
            itoa(n, num, 10);
            strcat(name, num);
            strcat(name, ".pic");
            sub_root_047c_0545();
            wp = sub_root_0617_045c(name);
            sub_root_047c_04f8();
            if (wp)
                sub_root_047c_05a5(&D_2806, 0x3b, 0xd, wp);
            if (wp)
                free_buffer(wp, "l");
        }
    }
}

void far sub_s031_1462_10ae(register int key, int who)
{
    int idxk, mmg, codeg;
    unsigned char ccm;

    switch (key) {
    case 0x11b:                                 /* Esc */
        if (D_867e) {
            sub_s031_1462_0950(D_867c);
            sub_s031_1462_1d44(D_867c, 12);
        }
        break;
    case 0x1c0d:                                /* Enter */
        if (D_867e) {
            sub_s031_1462_0950(D_867c);
            sub_s031_1462_1d44(D_867c, 12);
        } else
            sub_s031_1462_264c(D_867c, who);
        break;
    case 0x4b00:                                /* Left */
        if (D_867e)
            sub_s031_1462_0950(D_867c);
        sub_s031_1462_1d44(D_867c, F_290e_3557[D_867c].color);
        sub_s031_1462_1d44(D_867c = F_290e_3557[D_867c].left, 12);
        break;
    case 0x4d00:                                /* Right */
        if (D_867e)
            sub_s031_1462_0950(D_867c);
        sub_s031_1462_1d44(D_867c, F_290e_3557[D_867c].color);
        sub_s031_1462_1d44(D_867c = F_290e_3557[D_867c].right, 12);
        break;
    case 0x4800:                                /* Up */
        if ((F_290e_3557[D_867c].flags & 2) && D_867e) {
            if (F_290e_3557[D_867c].sel[who] == 0) {
                if (F_290e_3557[D_867c].top[who] == 0)
                    return;
                F_290e_3557[D_867c].top[who]--;
                sub_s031_1462_25de(D_867c, who);
            } else {
                sub_root_047c_12c3();
                sub_root_047c_04dc(&D_27f4, F_290e_3557[D_867c].ix + 5, F_290e_3557[D_867c].sel[who] * 8 + F_290e_3557[D_867c].iy + 9, F_290e_3557[D_867c].pw - 0xf, 8, 9, 8);
                F_290e_3557[D_867c].sel[who]--;
                sub_root_047c_04dc(&D_27f4, F_290e_3557[D_867c].ix + 5, F_290e_3557[D_867c].sel[who] * 8 + F_290e_3557[D_867c].iy + 9, F_290e_3557[D_867c].pw - 0xf, 8, 8, 9);
                sub_root_047c_1293();
            }
        } else {
            if (D_867e)
                sub_s031_1462_0950(D_867c);
            sub_s031_1462_1d44(D_867c, F_290e_3557[D_867c].color);
            sub_s031_1462_1d44(D_867c = F_290e_3557[D_867c].up, 12);
        }
        break;
    case 0x5000:                                /* Down */
        if ((F_290e_3557[D_867c].flags & 2) && D_867e) {
            F_290e_3557[D_867c].sel[who]++;
            if (F_290e_3557[D_867c].sel[who] >= F_290e_3557[D_867c].nvis) {
                ccm = F_290e_3557[D_867c].sel[who];
                F_290e_3557[D_867c].sel[who]--;
                if (F_290e_3557[D_867c].top[who] + ccm >= D_e7a4)
                    return;
                F_290e_3557[D_867c].top[who]++;
                sub_s031_1462_25de(D_867c, who);
            } else {
                if (F_290e_3557[D_867c].top[who] + F_290e_3557[D_867c].sel[who] >= D_e7a4) {
                    F_290e_3557[D_867c].sel[who]--;
                    return;
                }
                sub_root_047c_12c3();
                sub_root_047c_04dc(&D_27f4, F_290e_3557[D_867c].ix + 5, F_290e_3557[D_867c].sel[who] * 8 + F_290e_3557[D_867c].iy + 1, F_290e_3557[D_867c].pw - 0xf, 8, 9, 8);
                sub_root_047c_04dc(&D_27f4, F_290e_3557[D_867c].ix + 5, F_290e_3557[D_867c].sel[who] * 8 + F_290e_3557[D_867c].iy + 9, F_290e_3557[D_867c].pw - 0xf, 8, 8, 9);
                sub_root_047c_1293();
            }
        } else {
            if (D_867e)
                sub_s031_1462_0950(D_867c);
            sub_s031_1462_1d44(D_867c, F_290e_3557[D_867c].color);
            sub_s031_1462_1d44(D_867c = F_290e_3557[D_867c].down, 12);
        }
        break;
    case 0x4900:                                /* PgUp */
        if ((F_290e_3557[D_867c].flags & 2) && D_867e) {
            if (F_290e_3557[D_867c].top[who] > 14)
                F_290e_3557[D_867c].top[who] -= 14;
            else
                F_290e_3557[D_867c].top[who] = 0;
            sub_s031_1462_25de(D_867c, who);
        }
        break;
    case 0x5100:                                /* PgDn */
        if ((F_290e_3557[D_867c].flags & 2) && D_867e) {
            if (F_290e_3557[D_867c].top[who] + 14 >= D_e7a4)
                return;
            F_290e_3557[D_867c].top[who] += 14;
            if (F_290e_3557[D_867c].top[who] + F_290e_3557[D_867c].sel[who] >= D_e7a4) {
                if (D_e7a4 >= 14)
                    F_290e_3557[D_867c].top[who] = D_e7a4 - 14;
                else
                    F_290e_3557[D_867c].top[who] = 0;
                F_290e_3557[D_867c].sel[who] = D_e7a4 >= 14 ? 13 : D_e7a4 - 1;
            }
            sub_s031_1462_25de(D_867c, who);
        }
        break;
    case 0x4f00:                                /* End */
        if ((F_290e_3557[D_867c].flags & 2) && D_867e) {
            sub_root_047c_12c3();
            F_290e_3557[D_867c].top[who] = D_e7a4 >= 14 ? D_e7a4 - 14 : 0;
            F_290e_3557[D_867c].sel[who] = D_e7a4 >= 14 ? 13 : D_e7a4 - 1;
            sub_s031_1462_25de(D_867c, who);
            sub_root_047c_1293();
        }
        break;
    case 0x4700:                                /* Home */
        if ((F_290e_3557[D_867c].flags & 2) && D_867e) {
            sub_root_047c_12c3();
            F_290e_3557[D_867c].top[who] = 0;
            F_290e_3557[D_867c].sel[who] = 0;
            sub_s031_1462_25de(D_867c, who);
            sub_root_047c_1293();
        }
        break;
    case 0x5e00: case 0x5f00: case 0x6000: case 0x6100: case 0x6200:   /* Ctrl-F1..F5: new leader */
        key = (char)(key >> 8) - 0x5e;
        if (key >= D_a67e || D_9c69[key].present != 1)
            return;
        D_907b = key;
        sub_root_0000_1716(0x100);
    case 0x231: case 0x332: case 0x433: case 0x534: case 0x635:       /* 1..5: give to member */
        if ((F_290e_3557[D_867c].flags & 0x10) && D_867e) {
            if (D_9c69[(key & 7) - 1].present == 0)
                return;
            if (D_9c69[(key & 7) - 1].present == 5)
                return;
            if ((idxk = F_290e_3557[D_867c].top[who] + F_290e_3557[D_867c].sel[who]) >= D_9c69[who].nitems)
                return;
            if ((*(unsigned char far *)0x00400017L & 2 || *(unsigned char far *)0x00400017L & 1))
                sub_s031_18e7_007c(who, (key & 0xf) - 1, idxk, D_867c);
            else
                sub_s031_18e7_0000(who, (key & 0xf) - 1, idxk, D_867c);
            sub_root_047c_12c3();
            if (D_867a) {
                sub_root_047c_05a5(&D_2806, F_290e_3557[D_867c].ix - 1, F_290e_3557[D_867c].iy, D_867a);
                free_buffer(D_867a, "b");
                D_867a = 0;
            }
            sub_s031_1462_0b22(who);
            sub_root_047c_0553(&D_2806, 0x3c, 0xd, 0x8a, 0xaa, &D_27f4, 0x3c, 0xd);
            sub_root_047c_0545();
            D_867a = sub_root_047c_04d5(0, F_290e_3557[D_867c].ix - 1, F_290e_3557[D_867c].iy, F_290e_3557[D_867c].pw + 3, F_290e_3557[D_867c].ph + 0x11);
            sub_root_047c_04f8();
            sub_s031_1462_25de(9, who);
            sub_s031_1462_39a0(6, who, D_27f4);
            sub_root_047c_1293();
        } else {
            mmg = (key & 0xf) + 0xb;
            if (D_9c69[mmg].present == 1)
                return;
            if (D_907a == 1 && D_a6b4[mmg] == (char)0x9d) {
                vec_1d51(mmg);
                return;
            }
            D_867d = mmg;
            sub_root_0000_1588(0x100);
            sub_root_0000_1716(0x100);
            sub_s031_1462_2518(mmg, 11);
            D_867e = 0;
            if (D_867a) {
                free_buffer(D_867a, "b");
                D_867a = 0;
            }
            sub_s031_1462_0a82(who);
            sub_s031_1462_1d44(D_867c, 12);
        }
        break;
    case 0x2044: case 0x2064:                   /* d: drop */
        if ((F_290e_3557[D_867c].flags & 0x10) && D_867e) {
            if ((idxk = F_290e_3557[D_867c].top[who] + F_290e_3557[D_867c].sel[who]) >= D_9c69[who].nitems)
                return;
            if ((*(unsigned char far *)0x00400017L & 2 || *(unsigned char far *)0x00400017L & 1))
                sub_s031_18e7_07d4(who, idxk);
            else
                sub_s031_18e7_0474(who, idxk);
            sub_root_047c_12c3();
            if (D_867a)
                sub_root_047c_05a5(&D_2806, F_290e_3557[D_867c].ix - 1, F_290e_3557[D_867c].iy, D_867a);
            sub_s031_1462_0b22(who);
            sub_root_047c_0553(&D_2806, 0x3c, 0xd, 0x8a, 0xaa, &D_27f4, 0x3c, 0xd);
            sub_s031_1462_25de(9, who);
            sub_s031_1462_39a0(6, who, D_27f4);
            sub_root_047c_1293();
        }
        break;
    case 0x1e41: case 0x1e61:                   /* a */
        if ((F_290e_3557[D_867c].flags & 0x10) && D_867e) {
            idxk = F_290e_3557[D_867c].top[who] + F_290e_3557[D_867c].sel[who];
            sub_s031_18e7_11b2(who, idxk);
            sub_root_047c_12c3();
            if (D_867a)
                sub_root_047c_05a5(&D_2806, F_290e_3557[D_867c].ix - 1, F_290e_3557[D_867c].iy, D_867a);
            sub_s031_1462_0b22(who);
            sub_root_047c_0553(&D_2806, 0x3c, 0xd, 0x8a, 0xaa, &D_27f4, 0x3c, 0xd);
            if (D_867c == 9) {
                if (D_867a)
                    free_buffer(D_867a, "b");
                sub_root_047c_0545();
                D_867a = sub_root_047c_04d5(0, F_290e_3557[9].ix - 1, F_290e_3557[9].iy, F_290e_3557[9].pw + 3, F_290e_3557[9].ph + 0x11);
                sub_root_047c_04f8();
                sub_s031_1462_25de(D_867c, who);
            } else
                sub_s031_1462_264c(D_867c, who);
            sub_root_047c_12c3();
            sub_root_047c_0553(&D_2806, F_290e_3557[6].x, F_290e_3557[6].y, F_290e_3557[6].w, F_290e_3557[6].h, &D_27f4, F_290e_3557[6].x, F_290e_3557[6].y);
            sub_s031_1462_39a0(6, who, D_27f4);
            sub_root_047c_1293();
        }
        break;
    case 0x1655: case 0x1675:                   /* u */
        if ((F_290e_3557[D_867c].flags & 0x10) && D_867e) {
            idxk = F_290e_3557[D_867c].top[who] + F_290e_3557[D_867c].sel[who];
            sub_s031_18e7_12e8(who, idxk, D_867c);
            sub_root_047c_12c3();
            if (D_867a)
                sub_root_047c_05a5(&D_2806, F_290e_3557[D_867c].ix - 1, F_290e_3557[D_867c].iy, D_867a);
            sub_s031_1462_0b22(who);
            sub_root_047c_0553(&D_2806, 0x3c, 0xd, 0x8a, 0xaa, &D_27f4, 0x3c, 0xd);
            if (D_867c == 9) {
                if (D_867a)
                    free_buffer(D_867a, "b");
                sub_root_047c_0545();
                D_867a = sub_root_047c_04d5(0, F_290e_3557[9].ix - 1, F_290e_3557[9].iy, F_290e_3557[9].pw + 3, F_290e_3557[9].ph + 0x11);
                sub_root_047c_04f8();
                sub_s031_1462_25de(D_867c, who);
            } else
                sub_s031_1462_264c(D_867c, who);
            sub_root_047c_12c3();
            sub_root_047c_0553(&D_2806, F_290e_3557[6].x, F_290e_3557[6].y, F_290e_3557[6].w, F_290e_3557[6].h, &D_27f4, F_290e_3557[6].x, F_290e_3557[6].y);
            sub_s031_1462_39a0(6, who, D_27f4);
            sub_root_047c_1293();
        }
        break;
    case 0x1950: case 0x1970:                   /* p */
        if (D_867c == 9 && D_867e) {
            idxk = F_290e_3557[D_867c].top[who] + F_290e_3557[D_867c].sel[who];
            codeg = F_2e38_0000[who].items[idxk].code & 0xfff;
            if (!(((struct itemdef far *)F_2e38_0890[codeg])->flags & 0x800L))
                return;
            if (((struct itemdef far *)F_2e38_0890[codeg])->flags & 0x1000000L)
                return;
            sub_s031_1462_43b2(who, idxk);
            sub_s031_18e7_0474(who, idxk);
            if (D_9c69[who].nitems < D_e7a4) {
                if (F_290e_3557[9].top[who] != 0)
                    F_290e_3557[9].top[who]--;
                else if (F_290e_3557[9].sel[who] != 0)
                    F_290e_3557[9].sel[who]--;
                D_e7a4 = D_9c69[who].nitems;
            }
            sub_s031_1462_25de(9, who);
            sub_s031_1462_39a0(6, who, D_27f4);
        }
        break;
    case 0x1000:                                /* Alt-Q */
        sub_s031_1462_08f6(-1);
        sub_root_0000_13a0();
        break;
    default:
        if (D_867e)
            sub_s031_1462_0950(D_867c);
        break;
    }
}

void far sub_s031_1462_1d44(unsigned char k, int color)
{
    sub_root_047c_12c3();
    if (color == 0xff) {
        color = k - 12;
        sub_root_0000_1588(color);
        sub_root_0000_1716(color);
        sub_s031_1462_2518(D_867d, 11);
    } else {
        sub_root_047c_0d62(&D_27f4, F_290e_3557[k].x, F_290e_3557[k].y,
                           F_290e_3557[k].x + F_290e_3557[k].w, F_290e_3557[k].y, color);
        sub_root_047c_0d62(&D_27f4, F_290e_3557[k].x + F_290e_3557[k].w, F_290e_3557[k].y,
                           F_290e_3557[k].x + F_290e_3557[k].w, F_290e_3557[k].y + F_290e_3557[k].h, color);
        sub_root_047c_0d62(&D_27f4, F_290e_3557[k].x + F_290e_3557[k].w, F_290e_3557[k].y + F_290e_3557[k].h,
                           F_290e_3557[k].x, F_290e_3557[k].y + F_290e_3557[k].h, color);
        sub_root_047c_0d62(&D_27f4, F_290e_3557[k].x, F_290e_3557[k].y + F_290e_3557[k].h,
                           F_290e_3557[k].x, F_290e_3557[k].y, color);
    }
    sub_root_047c_1293();
}

void far sub_s031_1462_1e42(int k, int who, int y)
{
    int shift, i;

    if (*(unsigned char far *)0x00400017L & 1 || *(unsigned char far *)0x00400017L & 2)
        shift = 1;
    else
        shift = 0;
    if ((y - F_290e_3557[k].iy - 9) / 8 + F_290e_3557[k].top[who] >= D_e7a4)
        return;
    sub_root_047c_12c3();
    sub_root_047c_04dc(&D_27f4, F_290e_3557[k].ix + 8, F_290e_3557[k].iy + F_290e_3557[k].sel[who] * 8 + 9, F_290e_3557[k].pw - 0xf, 8, 9, 8);
    F_290e_3557[D_867c].sel[who] = (y - F_290e_3557[k].iy - 9) / 8;
    sub_root_047c_04dc(&D_27f4, F_290e_3557[k].ix + 8, F_290e_3557[k].iy + F_290e_3557[k].sel[who] * 8 + 9, F_290e_3557[k].pw - 0xf, 8, 8, 9);
    if (k == 9)
        sub_root_047c_124f(3, 3, D_8678);
    sub_root_047c_1293();
    while (D_6ef0 & 1)
        ;
    if (D_6eec > 0x3c) {
        if (F_290e_3557[k].ix <= D_6eec && F_290e_3557[k].pw + F_290e_3557[k].ix >= D_6eec
                && F_290e_3557[k].iy <= D_6eee && F_290e_3557[k].ph + F_290e_3557[k].iy >= D_6eee) {
            if (k == 9) {
                shift = (D_6eee - F_290e_3557[k].iy - k) / 8;
                if (shift < 0)
                    shift = 0;
                if (D_9c69[who].nitems > shift) {
                    sub_s031_18e7_10d2(who, F_290e_3557[k].top[who] + (y - F_290e_3557[k].iy - k) / 8, F_290e_3557[k].top[who] + shift);
                    sub_s031_1462_25de(k, who);
                }
            } else if (k == 10 || k == 11) {
                if (k == 10 && (y - F_290e_3557[k].iy - 9) / 8 + F_290e_3557[k].top[who] < D_e7a4)
                    vec_2017(who, sub_s031_1462_42ac(who, (y - F_290e_3557[k].iy - 9) / 8 + F_290e_3557[k].top[who]));
                else if (k == 11 && (y - F_290e_3557[k].iy - 9) / 8 + F_290e_3557[k].top[who] < D_e7a4)
                    vec_200d(who, sub_s031_1462_433c(who, (y - F_290e_3557[k].iy - 9) / 8 + F_290e_3557[k].top[who]), 0);
                sub_s031_1462_0a82(who);
                sub_s031_1462_264c(k, who);
            }
        } else {
            for (i = 0; i <= 18; i++)
                if (F_290e_3557[i].x <= D_6eec && F_290e_3557[i].w + F_290e_3557[i].x >= D_6eec
                        && F_290e_3557[i].y <= D_6eee && F_290e_3557[i].y + F_290e_3557[i].h >= D_6eee)
                    break;
            if (i == 6) {
                sub_s031_18e7_11b2(who, (y - F_290e_3557[k].iy - 9) / 8 + F_290e_3557[k].top[who]);
                sub_root_047c_12c3();
                if (D_867a)
                    sub_root_047c_05a5(&D_2806, F_290e_3557[k].ix - 1, F_290e_3557[k].iy, D_867a);
                sub_s031_1462_0b22(who);
                sub_root_047c_1293();
                sub_root_047c_12c3();
                sub_root_047c_0553(&D_2806, 0x3c, 0xd, 0x8a, 0xaa, &D_27f4, 0x3c, 0xd);
                sub_root_047c_1293();
                sub_root_047c_12c3();
                if (D_867c == 9) {
                    if (D_867a)
                        free_buffer(D_867a, "b");
                    sub_root_047c_0545();
                    D_867a = sub_root_047c_04d5(0, F_290e_3557[k].ix - 1, F_290e_3557[k].iy, F_290e_3557[k].pw + 3, F_290e_3557[k].ph + 0x11);
                    sub_root_047c_04f8();
                    sub_s031_1462_25de(D_867c, who);
                } else
                    sub_s031_1462_264c(D_867c, who);
                sub_root_047c_1293();
                sub_root_047c_12c3();
                sub_root_047c_0553(&D_2806, F_290e_3557[6].x, F_290e_3557[6].y, F_290e_3557[6].w, F_290e_3557[6].h, &D_27f4, F_290e_3557[6].x, F_290e_3557[6].y);
                sub_s031_1462_39a0(6, who, D_27f4);
                sub_root_047c_1293();
            }
        }
    } else if (D_9c69[D_6eee / 40].present != 0 && D_9c69[D_6eee / 40].present != 5 && (F_290e_3557[k].flags & 0x10)) {
        if (shift)
            sub_s031_18e7_007c(who, D_6eee / 40, (y - F_290e_3557[k].iy - 9) / 8 + F_290e_3557[k].top[who], k);
        else
            sub_s031_18e7_0000(who, D_6eee / 40, (y - F_290e_3557[k].iy - 9) / 8 + F_290e_3557[k].top[who], k);
        if (k == 9) {
            sub_root_047c_12c3();
            if (D_867a) {
                sub_root_047c_05a5(&D_2806, F_290e_3557[k].ix - 1, F_290e_3557[k].iy, D_867a);
                free_buffer(D_867a, "b");
                D_867a = 0;
            }
            sub_s031_1462_0b22(who);
            sub_root_047c_0553(&D_2806, 0x3c, 0xd, 0x8a, 0xaa, &D_27f4, 0x3c, 0xd);
            sub_root_047c_0545();
            D_867a = sub_root_047c_04d5(0, F_290e_3557[k].ix - 1, F_290e_3557[k].iy, F_290e_3557[k].pw + 3, F_290e_3557[k].ph + 0x11);
            sub_root_047c_04f8();
            sub_s031_1462_25de(9, who);
            sub_s031_1462_39a0(6, who, D_27f4);
            sub_root_047c_1293();
        } else if (k == 6) {
            sub_root_047c_12c3();
            if (D_867a) {
                sub_root_047c_05a5(&D_2806, F_290e_3557[k].ix - 1, F_290e_3557[k].iy, D_867a);
                free_buffer(D_867a, "b");
                D_867a = 0;
            }
            sub_s031_1462_0b22(who);
            sub_root_047c_0553(&D_2806, 0x3c, 0xd, 0x8a, 0xaa, &D_27f4, 0x3c, 0xd);
            sub_s031_1462_39a0(9, who, D_27f4);
            sub_s031_1462_264c(6, who);
            sub_root_047c_1293();
        }
    }
    if (F_290e_3557[k].flags & 2) {
        sub_root_047c_12c3();
        sub_root_047c_124f(3, 3, D_8680);
        sub_root_047c_1293();
    }
}

void far sub_s031_1462_2518(int k, int color)
{
    sub_root_047c_12c3();
    sub_root_047c_0d62(&D_27f4, F_290e_3825[k].x, F_290e_3825[k].y,
                       F_290e_3825[k].x + F_290e_3825[k].w, F_290e_3825[k].y, color);
    sub_root_047c_0d62(&D_27f4, F_290e_3825[k].x + F_290e_3825[k].w, F_290e_3825[k].y,
                       F_290e_3825[k].x + F_290e_3825[k].w, F_290e_3825[k].y + F_290e_3825[k].h, color);
    sub_root_047c_0d62(&D_27f4, F_290e_3825[k].x + F_290e_3825[k].w, F_290e_3825[k].y + F_290e_3825[k].h,
                       F_290e_3825[k].x, F_290e_3825[k].y + F_290e_3825[k].h, color);
    sub_root_047c_0d62(&D_27f4, F_290e_3825[k].x, F_290e_3825[k].y + F_290e_3825[k].h,
                       F_290e_3825[k].x, F_290e_3825[k].y, color);
    sub_root_047c_1293();
}

void far sub_s031_1462_25de(int k, int who)
{
    sub_root_047c_12c3();
    sub_root_047c_0545();
    D_8682[0] = sub_root_0617_045c("pics\\armbrs13.pic");
    sub_root_047c_04f8();
    sub_root_047c_05a5(&D_27f4, F_290e_3557[k].ix, F_290e_3557[k].iy, D_8682[0]);
    if (D_8682[0])
        free_buffer(D_8682[0], "x");
    sub_s031_1462_2b14(k, who);
    sub_root_047c_1293();
}

void far sub_s031_1462_264c(int k, int who)
{
    if (!(F_290e_3557[k].flags & 1)) {
        if (k == 0 && D_867c == k) {
            D_907b = who;
            sub_root_0000_1716(0x100);
            sub_root_047c_12c3();
            sub_root_047c_0553(&D_2806, F_290e_3557[k].x, F_290e_3557[k].y, F_290e_3557[k].w, F_290e_3557[k].h,
                               &D_27f4, F_290e_3557[k].x, F_290e_3557[k].y);
            sub_root_047c_1293();
            sub_root_047c_12c3();
            sub_s031_1462_39a0(k, who, D_27f4);
            sub_root_047c_1293();
        }
        sub_s031_1462_1d44(k, 12);
        return;
    }
    sub_root_047c_12c3();
    if (D_867a) {
        sub_root_047c_05a5(&D_27f4, F_290e_3557[k].ix - 1, F_290e_3557[k].iy, D_867a);
        free_buffer(D_867a, "b");
        D_867a = 0;
    }
    sub_root_047c_0545();
    D_867a = sub_root_047c_04d5(0, F_290e_3557[k].ix - 1, F_290e_3557[k].iy,
                                F_290e_3557[k].pw + 3, F_290e_3557[k].ph + 0x11);
    sub_root_047c_04f8();
    if (F_290e_3557[k].flags & 2) {
        sub_root_047c_0545();
        D_8682[8] = sub_root_0617_045c("pics\\armbrsh8.pic");
        D_8682[9] = sub_root_0617_045c("pics\\armbrsh9.pic");
        D_8682[10] = sub_root_0617_045c("pics\\armbrs10.pic");
        D_8682[12] = sub_root_0617_045c("pics\\armbrs12.pic");
        sub_root_047c_04f8();
        sub_root_047c_05a5(&D_2806, F_290e_3557[k].ix - 1, F_290e_3557[k].iy, D_867a);
        sub_root_047c_05a5(&D_2806, F_290e_3557[k].ix, F_290e_3557[k].iy + 0x71, D_8682[8]);
        sub_root_047c_0553(&D_2806, F_290e_3557[k].ix, F_290e_3557[k].iy, F_290e_3557[k].pw, F_290e_3557[k].ph,
                           &D_27f4, F_290e_3557[k].ix, F_290e_3557[k].iy);
        sub_root_047c_05a5(&D_2806, F_290e_3557[k].ix - 1, F_290e_3557[k].iy, D_867a);
        sub_root_047c_05a5(&D_2806, F_290e_3557[k].ix, F_290e_3557[k].iy + 0x67, D_8682[9]);
        sub_root_047c_0553(&D_2806, F_290e_3557[k].ix, F_290e_3557[k].iy, F_290e_3557[k].pw, F_290e_3557[k].ph,
                           &D_27f4, F_290e_3557[k].ix, F_290e_3557[k].iy);
        sub_root_047c_05a5(&D_2806, F_290e_3557[k].ix - 1, F_290e_3557[k].iy, D_867a);
        sub_root_047c_05a5(&D_2806, F_290e_3557[k].ix, F_290e_3557[k].iy + 0x4e, D_8682[10]);
        sub_root_047c_0553(&D_2806, F_290e_3557[k].ix, F_290e_3557[k].iy, F_290e_3557[k].pw, F_290e_3557[k].ph,
                           &D_27f4, F_290e_3557[k].ix, F_290e_3557[k].iy);
        sub_root_047c_05a5(&D_2806, F_290e_3557[k].ix - 1, F_290e_3557[k].iy, D_867a);
        sub_root_047c_05a5(&D_2806, F_290e_3557[k].ix, F_290e_3557[k].iy + 0x11, D_8682[12]);
        sub_root_047c_0553(&D_2806, F_290e_3557[k].ix, F_290e_3557[k].iy, F_290e_3557[k].pw, F_290e_3557[k].ph,
                           &D_27f4, F_290e_3557[k].ix, F_290e_3557[k].iy);
        if (D_8682[8])
            free_buffer(D_8682[8], "x");
        D_8682[8] = 0;
        sub_root_047c_0545();
        D_8682[0] = sub_root_0617_045c("pics\\armbrs13.pic");
        sub_root_047c_04f8();
        vec_1905("full up", "a");
        sub_root_047c_05a5(&D_2806, F_290e_3557[k].ix - 1, F_290e_3557[k].iy, D_867a);
        sub_root_047c_05a5(&D_2806, F_290e_3557[k].ix, F_290e_3557[k].iy, D_8682[0]);
        sub_root_047c_0553(&D_2806, F_290e_3557[k].ix, F_290e_3557[k].iy, F_290e_3557[k].pw, F_290e_3557[k].ph,
                           &D_27f4, F_290e_3557[k].ix, F_290e_3557[k].iy);
        sub_root_047c_05a5(&D_2806, F_290e_3557[k].ix - 1, F_290e_3557[k].iy, D_867a);
        if (D_8682[0])
            free_buffer(D_8682[0], "x");
        D_8682[0] = 0;
    } else {
        sub_root_047c_05a5(&D_27f4, F_290e_3557[k].ix, F_290e_3557[k].iy, D_8682[F_290e_3557[k].pic]);
    }
    sub_s031_1462_2b14(k, who);
    D_867e = 1;
    if (F_290e_3557[k].flags & 2)
        sub_root_047c_124f(5, 0, D_8680);
    sub_root_047c_1293();
}

void far sub_s031_1462_2b14(int k, int who)
{
    register int j, n;
    int cntl, topm, bitc;
    char buff[40];

    D_2800 = 0xbf;
    switch (k) {
    case 3:
        sub_root_0265_0004(&D_27f4, F_290e_3557[k].ix + 0x2b, F_290e_3557[k].iy + 0x5, F_290e_1e67[0]);
        sub_root_0265_0004(&D_27f4, F_290e_3557[k].ix + 0x2b, F_290e_3557[k].iy + 0xd, F_290e_1e67[1]);
        sub_root_0265_0004(&D_27f4, F_290e_3557[k].ix + 0x2b, F_290e_3557[k].iy + 0x15, F_290e_1e67[2]);
        sub_root_0265_0004(&D_27f4, F_290e_3557[k].ix + 0x2b, F_290e_3557[k].iy + 0x1d, F_290e_1e67[3]);
        sub_root_0265_0004(&D_27f4, F_290e_3557[k].ix + 0x2b, F_290e_3557[k].iy + 0x25, F_290e_1e67[4]);
        sub_root_0265_0004(&D_27f4, F_290e_3557[k].ix + 0x2b, F_290e_3557[k].iy + 0x2d, F_290e_1e67[5]);
        sub_root_0265_0004(&D_27f4, F_290e_3557[k].ix + 0x2b, F_290e_3557[k].iy + 0x35, F_290e_1e67[6]);
        for (j = 0; j < 7; j++) {
            sprintf(buff, "%02d/%02d", D_9c69[who].attr[j], D_9c69[who].attr_max[j]);
            sub_root_047c_055a(&D_27f4, F_290e_3557[k].ix + 8, j * 8 + F_290e_3557[k].iy + 5, buff);
        }
        break;
    case 4:
        sub_root_0265_0004(&D_27f4, F_290e_3557[k].ix + 0x16, F_290e_3557[k].iy + 0x5, F_290e_1e67[7]);
        sub_root_0265_0004(&D_27f4, F_290e_3557[k].ix + 0x16, F_290e_3557[k].iy + 0xd, F_290e_1e67[8]);
        sub_root_0265_0004(&D_27f4, F_290e_3557[k].ix + 0x16, F_290e_3557[k].iy + 0x15, F_290e_1e67[9]);
        sub_root_0265_0004(&D_27f4, F_290e_3557[k].ix + 0x16, F_290e_3557[k].iy + 0x1d, F_290e_1e67[10]);
        sub_root_0265_0004(&D_27f4, F_290e_3557[k].ix + 0x16, F_290e_3557[k].iy + 0x25, F_290e_1e67[11]);
        sub_root_0265_0004(&D_27f4, F_290e_3557[k].ix + 0x16, F_290e_3557[k].iy + 0x2d, F_290e_1e67[12]);
        sub_root_0265_0004(&D_27f4, F_290e_3557[k].ix + 0x16, F_290e_3557[k].iy + 0x35, F_290e_1e67[13]);
        for (j = 0; j < 7; j++) {
            sprintf(buff, "%02d", D_9c69[who].skill[j]);
            sub_root_047c_055a(&D_27f4, F_290e_3557[k].ix + 8, j * 8 + F_290e_3557[k].iy + 5, buff);
        }
        break;
    case 5:
        sub_root_0265_0004(&D_27f4, F_290e_3557[k].ix + 0x16, F_290e_3557[k].iy + 0x5, F_290e_1e67[14]);
        sub_root_0265_0004(&D_27f4, F_290e_3557[k].ix + 0x16, F_290e_3557[k].iy + 0xd, F_290e_1e67[15]);
        sub_root_0265_0004(&D_27f4, F_290e_3557[k].ix + 0x16, F_290e_3557[k].iy + 0x15, F_290e_1e67[16]);
        sub_root_0265_0004(&D_27f4, F_290e_3557[k].ix + 0x16, F_290e_3557[k].iy + 0x1d, F_290e_1e67[17]);
        sub_root_0265_0004(&D_27f4, F_290e_3557[k].ix + 0x16, F_290e_3557[k].iy + 0x25, F_290e_1e67[18]);
        sub_root_0265_0004(&D_27f4, F_290e_3557[k].ix + 0x16, F_290e_3557[k].iy + 0x2d, F_290e_1e67[19]);
        for (j = 0; j < 6; j++) {
            sprintf(buff, "%02d", D_9c69[who].skill[7 + j]);
            sub_root_047c_055a(&D_27f4, F_290e_3557[k].ix + 8, j * 8 + F_290e_3557[k].iy + 5, buff);
        }
        break;
    case 6:
        if (D_9c69[who].weapon_type != -1)
            sub_root_0265_0004(&D_27f4, F_290e_3557[k].ix + 4, 0x58, F_2e38_0890[sub_s031_18e7_14b6(who, 0)]);
        else
            sub_root_047c_055a(&D_27f4, F_290e_3557[k].ix + 5, 0x58, "No Weapon");
        if (D_9c69[who].vital_type != -1)
            sub_root_0265_0004(&D_27f4, F_290e_3557[k].ix + 4, 0x62, F_2e38_0890[sub_s031_18e7_14b6(who, 1)]);
        else
            sub_root_047c_055a(&D_27f4, F_290e_3557[k].ix + 5, 0x62, "No Vitals Armor");
        if (D_9c69[who].limb_type != -1)
            sub_root_0265_0004(&D_27f4, F_290e_3557[k].ix + 4, 0x6c, F_2e38_0890[sub_s031_18e7_14b6(who, 2)]);
        else
            sub_root_047c_055a(&D_27f4, F_290e_3557[k].ix + 5, 0x6c, "No Limb Armor");
        if (D_9c69[who].shield_type != -1)
            sub_root_0265_0004(&D_27f4, F_290e_3557[k].ix + 4, 0x76, F_2e38_0890[sub_s031_18e7_14b6(who, 3)]);
        else
            sub_root_047c_055a(&D_27f4, F_290e_3557[k].ix + 5, 0x76, "No Shield");
        if (D_9c69[who].missile_type != -1)
            sub_root_0265_0004(&D_27f4, F_290e_3557[k].ix + 4, 0x80, F_2e38_0890[sub_s031_18e7_14b6(who, 4)]);
        else
            sub_root_047c_055a(&D_27f4, F_290e_3557[k].ix + 5, 0x80, "No Missile Weapon");
        if (sub_s029_0e76_3996(0x49, who) || sub_s029_0e76_3996(0x48, who) || sub_s029_0e76_3996(0x4a, who))
            sub_root_047c_055a(&D_27f4, F_290e_3557[k].ix + 5, 0x8a, "Enhc/Bless");
        break;
    case 7:
        sub_root_0265_0004(&D_27f4, F_290e_3557[k].ix + 0x20, F_290e_3557[k].iy + 0x5, F_290e_1e67[20]);
        sub_root_0265_0004(&D_27f4, F_290e_3557[k].ix + 0x20, F_290e_3557[k].iy + 0xd, F_290e_1e67[21]);
        sub_root_0265_0004(&D_27f4, F_290e_3557[k].ix + 0x20, F_290e_3557[k].iy + 0x15, F_290e_1e67[22]);
        sub_root_0265_0004(&D_27f4, F_290e_3557[k].ix + 0x20, F_290e_3557[k].iy + 0x1d, F_290e_1e67[23]);
        sub_root_0265_0004(&D_27f4, F_290e_3557[k].ix + 0x20, F_290e_3557[k].iy + 0x25, F_290e_1e67[24]);
        sub_root_0265_0004(&D_27f4, F_290e_3557[k].ix + 0x20, F_290e_3557[k].iy + 0x2d, F_290e_1e67[25]);
        for (j = 0; j < 6; j++) {
            sprintf(buff, "%02d", D_9c69[who].skill[13 + j]);
            sub_root_047c_055a(&D_27f4, F_290e_3557[k].ix + 8, j * 8 + F_290e_3557[k].iy + 5, buff);
        }
        break;
    case 9:
        if (D_9c69[who].nitems <= 0)
            break;
        {
            D_2800 = 8;
            if (F_290e_3557[k].top[who] >= D_9c69[who].nitems)
                F_290e_3557[k].top[who] = sub_s029_1367_0066(0, 0x63, D_9c69[who].nitems - 10);
            if (F_290e_3557[k].sel[who] >= D_9c69[who].nitems)
                F_290e_3557[k].sel[who] = D_9c69[who].nitems - 1;
            for (j = 0; j < F_290e_3557[k].nvis; j++) {
                if (F_290e_3557[k].top[who] + j < D_9c69[who].nitems) {
                    sprintf(buff, "%Fs %02dq (%d)",
                            F_2e38_0890[F_2e38_0000[who].items[F_290e_3557[k].top[who] + j].code & 0xfff],
                            F_2e38_0000[who].items[F_290e_3557[k].top[who] + j].qual,
                            F_2e38_0000[who].items[F_290e_3557[k].top[who] + j].cnt);
                    sub_root_047c_055a(&D_27f4, F_290e_3557[k].ix + 9, j * 8 + F_290e_3557[k].iy + 9, buff);
                }
            }
            sub_root_047c_04dc(&D_27f4, F_290e_3557[k].ix + 8, F_290e_3557[k].sel[who] * 8 + F_290e_3557[k].iy + 9, F_290e_3557[k].pw - 0xf, 8, 8, 9);
            D_e7a4 = D_9c69[who].nitems;
            D_2800 = 0xbf;
        }
        break;
    case 10:
        D_2800 = 8;
        cntl = n = j = 0;
        for (; j < 22 && n < F_290e_3557[k].nvis; j++) {
            if ((F_2e38_07be[who][j] & 1) && F_290e_3557[k].top[who] <= cntl++ && F_290e_3557[k].nvis > n)
                sub_root_0265_0004(&D_27f4, F_290e_3557[k].ix + 9,
                                   n++ * 8 + F_290e_3557[k].iy + 9, F_2e38_1736[j * 3]);
            if ((F_2e38_07be[who][j] & 2) && F_290e_3557[k].top[who] <= cntl++ && F_290e_3557[k].nvis > n)
                sub_root_0265_0004(&D_27f4, F_290e_3557[k].ix + 9,
                                   n++ * 8 + F_290e_3557[k].iy + 9, F_2e38_1736[j * 3 + 1]);
            if ((F_2e38_07be[who][j] & 4) && F_290e_3557[k].top[who] <= cntl++ && F_290e_3557[k].nvis > n)
                sub_root_0265_0004(&D_27f4, F_290e_3557[k].ix + 9,
                                   n++ * 8 + F_290e_3557[k].iy + 9, F_2e38_1736[j * 3 + 2]);
        }
        sub_root_047c_04dc(&D_27f4, F_290e_3557[k].ix + 8, F_290e_3557[k].sel[who] * 8 + F_290e_3557[k].iy + 9, F_290e_3557[k].pw - 0xf, 8, 8, 9);
        D_e7a4 = 0;
        for (j = 0; j < 22; j++) {
            if (F_2e38_07be[who][j] & 1)
                D_e7a4++;
            if (F_2e38_07be[who][j] & 2)
                D_e7a4++;
            if (F_2e38_07be[who][j] & 4)
                D_e7a4++;
        }
        D_2800 = 0xbf;
        break;
    case 11:
        D_2800 = 8;
        topm = F_290e_3557[k].top[who];
        j = n = 0;
        for (; j < 17 && n - F_290e_3557[k].top[who] < F_290e_3557[k].nvis; j++) {
            for (bitc = 0; bitc < 8; bitc++) {
                if (F_2e38_082c[who][j] & D_00e8[bitc % 8]) {
                    if (F_290e_3557[k].top[who] <= n && n - F_290e_3557[k].top[who] < F_290e_3557[k].nvis)
                        sub_root_0265_0004(&D_27f4, F_290e_3557[k].ix + 9, (n - topm) * 8 + F_290e_3557[k].iy + 9, F_2e38_183e[j * 8 + bitc]);
                    n++;
                }
            }
        }
        D_e7a4 = 0;
        for (j = 0; j < 17; j++)
            for (bitc = 0; bitc < 8; bitc++)
                if (F_2e38_082c[who][j] & D_00e8[bitc % 8])
                    D_e7a4++;
        sub_root_047c_04dc(&D_27f4, F_290e_3557[k].ix + 8, F_290e_3557[k].sel[who] * 8 + F_290e_3557[k].iy + 9, F_290e_3557[k].pw - 0xf, 8, 8, 9);
        D_2800 = 0xbf;
        break;
    }
}

void far sub_s031_1462_39a0(int i, int who, struct gfx g)
{
    sub_s031_1462_39c6(i, who, g);
}

void far sub_s031_1462_39c6(int i, int who, struct gfx g)
{
    int j, y, t;
    char buf[40];

    switch (i) {
    case 0:
        sub_root_047c_0553(&D_2806, F_290e_3557[0].x, F_290e_3557[0].y, F_290e_3557[0].w, F_290e_3557[0].h, &D_27f4, F_290e_3557[0].x, F_290e_3557[0].y);
        sub_root_047c_055a(&g, F_290e_3557[i].x + 5, 3, D_9c69[who].full_name);
        if (D_907b == who)
            sub_root_047c_055a(&g, F_290e_3557[i].x + 0x82, F_290e_3557[i].y + 3, "Leader");
        else
            sub_root_047c_055a(&g, F_290e_3557[i].x + 0x82, F_290e_3557[i].y + 3, "Not Leader");
        break;
    case 1:
        sub_root_047c_055a(&g, F_290e_3557[i].x + 5, F_290e_3557[i].y + 3, "Age");
        sprintf(buf, "%02d", *(int *)((char *)&D_9c69[who] - 2));
        sub_root_047c_055a(&g, F_290e_3557[i].x + 0x19, F_290e_3557[i].y + 3, buf);
        if (D_9c69[who].pad01[2] == 0)
            sub_root_047c_055a(&g, F_290e_3557[i].x + 0x28, F_290e_3557[i].y + 3, "(M)");
        else
            sub_root_047c_055a(&g, F_290e_3557[i].x + 0x28, F_290e_3557[i].y + 3, "(F)");
        break;
    case 2:
        sub_root_047c_0553(&D_2806, F_290e_3557[2].x, F_290e_3557[2].y, F_290e_3557[2].w, F_290e_3557[2].h, &D_27f4, F_290e_3557[2].x, F_290e_3557[2].y);
        switch (sub_s029_0e76_013a(who)) {
        case 0:
            sub_root_047c_055a(&g, F_290e_3557[i].x + 0xc, F_290e_3557[i].y + 4, "Light Load");
            break;
        case 1:
            sub_root_047c_055a(&g, F_290e_3557[i].x + 0xc, F_290e_3557[i].y + 4, "Normal Load");
            break;
        case 2:
            sub_root_047c_055a(&g, F_290e_3557[i].x + 0xc, F_290e_3557[i].y + 4, "Heavy Load");
            break;
        case 3:
            sub_root_047c_055a(&g, F_290e_3557[i].x + 0xc, F_290e_3557[i].y + 4, "Overloaded");
            break;
        }
        sprintf(buf, "%02d/%02d", D_9c69[who].pad35[0], D_9c69[who].attr[1] + D_9c69[who].attr[0]);
        sub_root_047c_055a(&g, F_290e_3557[2].x + 0x48, F_290e_3557[2].y + 4, buf);
        break;
    case 3:
        sub_root_047c_055a(&g, 0xed, 0x12, "End");
        sub_root_047c_055a(&g, 0xed, 0x1a, "Str");
        sub_root_047c_055a(&g, 0xed, 0x22, "Agl");
        sub_root_047c_055a(&g, 0xed, 0x2a, "Per");
        sub_root_047c_055a(&g, 0xed, 0x32, "Int");
        sub_root_047c_055a(&g, 0xed, 0x3a, "Chr");
        sub_root_047c_055a(&g, 0xed, 0x42, "DF");
        for (j = 0, y = 0x12; y < 0x4a; j++, y += 8) {
            sprintf(buf, "%02d/%02d", D_9c69[who].attr[j], D_9c69[who].attr_max[j]);
            sub_root_047c_055a(&g, 0xca, y, buf);
        }
        break;
    case 4:
        sub_root_047c_055a(&g, 0x11d, 0x12, "wEdg");
        sub_root_047c_055a(&g, 0x11d, 0x1a, "wImp");
        sub_root_047c_055a(&g, 0x11d, 0x22, "wFll");
        sub_root_047c_055a(&g, 0x11d, 0x2a, "wPol");
        sub_root_047c_055a(&g, 0x11d, 0x32, "wThr");
        sub_root_047c_055a(&g, 0x11d, 0x3a, "wBow");
        sub_root_047c_055a(&g, 0x11d, 0x42, "wMsD");
        for (j = 0, y = 0x12; y < 0x4a; j++, y += 8) {
            sprintf(buf, "%02d", D_9c69[who].skill[j]);
            sub_root_047c_055a(&g, 0x10c, y, buf);
        }
        break;
    case 5:
        sub_root_047c_055a(&g, 0x11d, 0x55, "Alch");
        sub_root_047c_055a(&g, 0x11d, 0x5d, "Relg");
        sub_root_047c_055a(&g, 0x11d, 0x65, "Virt");
        sub_root_047c_055a(&g, 0x11d, 0x6d, "SpkC");
        sub_root_047c_055a(&g, 0x11d, 0x75, "SpkL");
        sub_root_047c_055a(&g, 0x11d, 0x7d, "R&W");
        for (j = 0, y = 0x55; y < 0x85; j++, y += 8) {
            sprintf(buf, "%02d", D_9c69[who].skill[7 + j]);
            sub_root_047c_055a(&g, 0x10c, y, buf);
        }
        break;
    case 6:
        sub_root_047c_0553(&D_2806, F_290e_3557[6].x, F_290e_3557[6].y, F_290e_3557[6].w, F_290e_3557[6].h, &D_27f4, F_290e_3557[6].x, F_290e_3557[6].y);
        if (D_9c69[who].weapon_type != -1 && (t = sub_s031_18e7_14b6(who, 0)) != -1)
            sub_root_0265_0004(&g, F_290e_3557[i].x + 4, 0x58, F_2e38_0890[t] + 0x14);
        else
            sub_root_047c_055a(&g, F_290e_3557[i].x + 4, 0x58, "No Weapon");
        if (D_9c69[who].vital_type != -1 && (t = sub_s031_18e7_14b6(who, 1)) != -1)
            sub_root_0265_0004(&g, F_290e_3557[i].x + 4, 0x62, F_2e38_0890[t] + 0x14);
        else
            sub_root_047c_055a(&g, F_290e_3557[i].x + 4, 0x62, "No V:Armor");
        if (D_9c69[who].limb_type != -1 && (t = sub_s031_18e7_14b6(who, 2)) != -1)
            sub_root_0265_0004(&g, F_290e_3557[i].x + 4, 0x6c, F_2e38_0890[t] + 0x14);
        else
            sub_root_047c_055a(&g, F_290e_3557[i].x + 4, 0x6c, "No L:Armor");
        if (D_9c69[who].shield_type != -1 && (t = sub_s031_18e7_14b6(who, 3)) != -1)
            sub_root_0265_0004(&g, F_290e_3557[i].x + 4, 0x76, F_2e38_0890[t] + 0x14);
        else
            sub_root_047c_055a(&g, F_290e_3557[i].x + 4, 0x76, "No Shield");
        if (D_9c69[who].missile_type != -1 && (t = sub_s031_18e7_14b6(who, 4)) != -1)
            sub_root_0265_0004(&g, F_290e_3557[i].x + 4, 0x80, F_2e38_0890[t] + 0x14);
        else
            sub_root_047c_055a(&g, F_290e_3557[i].x + 4, 0x80, "No Msl Wpn");
        sub_s031_1462_39a0(2, who, g);
        break;
    case 7:
        sub_root_047c_055a(&g, 0x11d, 0x91, "Heal");
        sub_root_047c_055a(&g, 0x11d, 0x99, "Artf");
        sub_root_047c_055a(&g, 0x11d, 0xa1, "Stlh");
        sub_root_047c_055a(&g, 0x11d, 0xa9, "StrW");
        sub_root_047c_055a(&g, 0x11d, 0xb1, "Ride");
        sub_root_047c_055a(&g, 0x11d, 0xb9, "WdWs");
        for (j = 0, y = 0x91; y < 0xc1; j++, y += 8) {
            sprintf(buf, "%02d", D_9c69[who].skill[13 + j]);
            sub_root_047c_055a(&g, F_290e_3557[i].x + 3, y, buf);
        }
        break;
    case 17:
        sub_root_047c_055a(&g, 0xed, 0x12, "End");
        sub_root_047c_055a(&g, 0xed, 0x1a, "Str");
        sub_root_047c_055a(&g, 0xed, 0x22, "Agl");
        sub_root_047c_055a(&g, 0xed, 0x2a, "Per");
        sub_root_047c_055a(&g, 0xed, 0x32, "Int");
        sub_root_047c_055a(&g, 0xed, 0x3a, "Chr");
        sub_root_047c_055a(&g, 0xed, 0x42, "DF");
        for (j = 0, y = 0x20; y < 0x58; j++, y += 8) {
            sprintf(buf, "%02d/%02d", D_9c69[who].attr[j], D_9c69[who].attr_max[j]);
            sub_root_047c_055a(&g, 0xca, y, buf);
        }
        break;
    }
}

/* n-th set flag (3 per entry) in who's 22 flag bytes at 2e38:07be */
int far sub_s031_1462_42ac(int who, int n)
{
    int i;

    i = 0;
    do {
        if ((F_2e38_07be[who][i] & 1) && n-- <= 0)
            return i * 3;
        if ((F_2e38_07be[who][i] & 2) && n-- <= 0)
            return i * 3 + 1;
        if ((F_2e38_07be[who][i] & 4) && n-- <= 0)
            return i * 3 + 2;
    } while (++i < 22);
}

/* n-th set bit in who's 17 bit bytes at 2e38:082c */
int far sub_s031_1462_433c(int who, int n)
{
    int i, b;

    b = 0x80;               /* dead store; the original keeps it (mov [bp-2],80h), 6.00A drops it */
    for (i = 0; i < 17; i++) {
        b = 0;
        for (; b < 8; b++)
            if ((F_2e38_082c[who][i] & (0x80 >> b)) && n-- <= 0)
                return i * 8 + b;
    }
}

void far sub_s031_1462_43b2(int who, int idx)
{
    int hi, lo, t, c;

    c = F_2e38_0000[who].items[idx].code & 0xfff;
    switch (c) {
    case 0x69:
        hi = D_9c69[who].x34 >> 4;
        lo = D_9c69[who].x34 & 0xf;
        if (F_2e38_0000[who].items[idx].qual <= 0x19)
            t = 3;
        else if (F_2e38_0000[who].items[idx].qual <= 0x23)
            t = 4;
        else
            t = 5;
        sub_s029_0e76_4d24(who, c, t, 0x18);
        D_9c69[who].x34 = ((t + hi) << 4) + lo;
        break;
    case 0x6a:
        hi = D_9c69[who].x34 >> 4;
        lo = D_9c69[who].x34 & 0xf;
        if (F_2e38_0000[who].items[idx].qual <= 0x19)
            t = 3;
        else if (F_2e38_0000[who].items[idx].qual <= 0x23)
            t = 4;
        else
            t = 5;
        sub_s029_0e76_4d24(who, idx, t, 0x18);
        D_9c69[who].x34 = (hi << 4) + t + lo;
        break;
    case 0x6b:
        if (F_2e38_0000[who].items[idx].qual <= 0x19)
            t = 0xe;
        else if (F_2e38_0000[who].items[idx].qual <= 0x23)
            t = 0x16;
        else
            t = 0x1e;
        sub_s029_0e76_4d24(who, idx, t, 0x18);
        D_9c69[who].x34 = D_9c69[who].x34 % 16 + (D_9c69[who].x34 & 0xf0) + t;
        break;
    case 0x6c:
        if (F_2e38_0000[who].items[idx].qual <= 0x19)
            t = 0xe;
        else if (F_2e38_0000[who].items[idx].qual <= 0x23)
            t = 0x16;
        else if (F_2e38_0000[who].items[idx].qual <= 0x2d)
            t = 0x1e;
        else
            t = 5;
        sub_s029_0e76_4b08(who, 5, t, 0x18);
        sub_s029_0e76_4b08(who, 4, t, 0x18);
        sub_s029_0e76_4b08(who, 6, t, 0x18);
        break;
    case 0x6d:
        hi = D_9c69[who].x41 >> 4;
        lo = D_9c69[who].x41 & 0xf;
        if (F_2e38_0000[who].items[idx].qual <= 0x19)
            t = 1;
        else if (F_2e38_0000[who].items[idx].qual <= 0x23)
            t = 2;
        else if (F_2e38_0000[who].items[idx].qual <= 0x2d)
            t = 3;
        else
            t = 5;
        D_9c69[who].x41 = (hi << 4) + t + lo;
        break;
    case 0x6e:
        D_906b++;
        break;
    case 0x6f:
        if (F_2e38_0000[who].items[idx].qual <= 0x19)
            t = sub_root_0000_0c70(6) + 9;
        else if (F_2e38_0000[who].items[idx].qual <= 0x23)
            t = sub_root_0000_0c70(7) + 0xf;
        else if (F_2e38_0000[who].items[idx].qual <= 0x2d)
            t = sub_root_0000_0c70(8) + 0x16;
        else
            t = 5;
        sub_s029_0e76_4c04(who, 3, t, 0x18);
        break;
    case 0x70:
        if (F_2e38_0000[who].items[idx].qual <= 0x19)
            t = sub_root_0000_0c70(6) + 6;
        else if (F_2e38_0000[who].items[idx].qual <= 0x23)
            t = sub_root_0000_0c70(6) + 8;
        else if (F_2e38_0000[who].items[idx].qual <= 0x2d)
            t = sub_root_0000_0c70(6) + 0xa;
        else
            t = 5;
        sub_s029_0e76_4c04(who, 0, t, 0x18);
        break;
    case 0x71:
        if (F_2e38_0000[who].items[idx].qual <= 0x19)
            t = sub_root_0000_0c70(6) + 6;
        else if (F_2e38_0000[who].items[idx].qual <= 0x23)
            t = sub_root_0000_0c70(6) + 8;
        else if (F_2e38_0000[who].items[idx].qual <= 0x2d)
            t = sub_root_0000_0c70(6) + 0xa;
        else
            t = 5;
        sub_s029_0e76_4c04(who, 1, t, 0x18);
        break;
    case 0x72:
        if (F_2e38_0000[who].items[idx].qual <= 0x19)
            t = sub_root_0000_0c70(6) + 6;
        else if (F_2e38_0000[who].items[idx].qual <= 0x23)
            t = sub_root_0000_0c70(8) + 8;
        else if (F_2e38_0000[who].items[idx].qual <= 0x2d)
            t = sub_root_0000_0c70(8) + 0xa;
        else
            t = 5;
        sub_s029_0e76_4c04(who, 2, t, 0x18);
        break;
    case 0x73:
        if (F_2e38_0000[who].items[idx].qual <= 0x19)
            t = sub_root_0000_0c70(6) + 9;
        else if (F_2e38_0000[who].items[idx].qual <= 0x23)
            t = sub_root_0000_0c70(7) + 0xf;
        else if (F_2e38_0000[who].items[idx].qual <= 0x2d)
            t = sub_root_0000_0c70(8) + 0x16;
        else
            t = 5;
        sub_s029_0e76_08f2(who, 1, F_2e38_0000[who].items[idx].qual * 2);
        sub_s029_0e76_4c04(who, 0, t, 0x22);
        break;
    case 0x74:
        hi = D_9c69[who].x41 >> 4;
        lo = D_9c69[who].x41 & 0xf;
        if (F_2e38_0000[who].items[idx].qual <= 0x19)
            t = 0x32;
        else if (F_2e38_0000[who].items[idx].qual <= 0x23)
            t = 0x46;
        else
            t = 0x5a;
        D_9c69[who].x41 = ((hi + 1) << 4) + lo;
        D_9c69[who].x10 = t;
        break;
    }
    sub_root_0000_1588(who);
    sub_root_0000_1716(who);
}
