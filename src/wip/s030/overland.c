/* place: s030 1462:0000 */
/* flags: /AM /G2 /Gs /Ox /Oa /Zp1 */
/* Overland travel map (overlay s030): the screen where the party travels
   across the map of medieval Germany.  Note /Oa (assume no aliasing): it is
   what makes MSC address DGROUP through ss: while DS may hold a far segment.
   STATUS (e1b, 2026-10-09): all 22 functions written; 13/22 match (0000 00a8 02c0 1c4e 20d2
     271a 27aa 2842 28f2 29bc 29fa 2ac8 2d44); DATA and CONST match.  [n] = fdskip.py lines.
     02c0 matched after renaming its locals (hash order) and choosing one-line vs two-line
       `case ..: return K;` arms (build/agent/e1/greedy_lines.py) - see e1 NOTES.
     20d2 matched with hash-ordered names + unused gap arrays + the position of
       `extern int D_286e;` (decides `D_286e + row` operand order; moving it changes 0c44/16c6).
     1c76: only the jump table differs (relocation; case 4 written on one line, last).
     2dd6 [1]: `cmp si,dx` (ours `cmp dx,si`).
     2bb0 [1]: matches except __aFchkstk (6.00A name of the stack probe; game: __chkstk).
     1f30 [1]: frame 8 vs 6 (one unused slot; an unused local did not help here).
     25be [17]: speed/x in bx/si (ours si/bx).
     16c6: slots fixed by renaming; `xa` (x+16/x+24 column) lives in cx in the original,
       ours [bp-18]; operand orders of `global + local` sums depend on extern positions.
     0c44: locals renamed to hash order; left: 6.00 reloads col/k after storing them,
       D_e3f8 kept in cx, operand orders.
     1300, 1dd4: shape right; 6.00A shares stack slots between locals with disjoint
       lifetimes; original has unused slots (try unused arrays + hash names, as in 20d2). */

typedef struct { int x, y; } XY;

struct city {                       /* DARKLAND.CTY record (far) */
    char short_name[32];
    char full_name[32];
    int  city_size;                 /* 0x40 */
    int  entry_x, entry_y;          /* 0x42 */
    int  exit_x, exit_y;            /* 0x46: party position after leaving */
};

struct location {                   /* DARKLAND.LOC record, 0x3a bytes */
    int  icon;                      /* 0x00 */
    int  unk02;
    int  x, y;                      /* 0x04 */
    int  unk08, unk0a;
    int  menu;                      /* 0x0c */
    int  unk0e;
    char pad10[4];
    unsigned char flags;            /* 0x14 */
    char pad15[2];
    char unk17;                     /* 0x17 */
};

/* far data in segment 2e38 */
struct far2e38 {
    char pad0[0x10be];
    struct location far *loc[414];  /* 2e38:10be  all locations; 0..91 are cities */
};
extern struct far2e38 far F_2e38_0000;
extern int D_2866, D_2868;          /* party marker screen offset */

extern int D_907e;                  /* current location index, -1 = wilderness */
extern struct city far *D_9785;     /* current city record */
extern int D_e3d0, D_e3d2;          /* party map x, y */
extern int D_e48a;                  /* next card/menu id */

extern int D_00e4;
extern int D_2860;
extern unsigned D_2870;             /* map buffer segment */
extern int D_775a;                  /* errno */
extern int D_8672;
extern int D_9c3d, D_9c42;
extern int D_a891;
extern int D_e3f2;
extern struct _iobuf *D_e3f4;       /* DARKLAND.MAP file */
extern int D_e3f6, D_e3f8;          /* map width, height */
extern unsigned D_e416;
extern int D_e488;

struct _iobuf *fopen(const char *, const char *);
int printf(const char *, ...);
int fcloseall(void);
unsigned _dos_freemem(unsigned);

int far sub_s029_0e76_32ce(int);
void far sub_s029_0e76_2930(int, int);
int far sub_s029_0e76_1326(int);
void far vec_1fd1(void);
int far vec_18f1(int);
int far sub_root_0000_0c70(int);
void far sub_root_047c_12c3(void);
void far sub_root_047c_0568(void);
void far sub_root_047c_056f(void);
void far sub_root_047c_124f(int, int, int);
void far sub_root_05ba_00c2(unsigned, char *);

void far sub_s030_1462_1c4e(struct _iobuf *);
void far sub_s030_1462_1c76(struct _iobuf *);
void far sub_s030_1462_1f30(void);
void far sub_s030_1462_20d2(int, int);
int far sub_s030_1462_02c0(void);
#define BIOS_KBFLAGS ((unsigned char far *)0x00400017L)
int far sub_s030_1462_00a8(int, int);

extern unsigned far F_2e38_1ffc[];

struct party_slot { char present; char pad[127]; };   /* 128-byte records at DGROUP 9c69 */
extern struct party_slot D_9c69[];
extern int D_6eec, D_6eee;          /* mouse x, y */
extern unsigned char D_6ef0;        /* mouse buttons */
extern int D_27f4[];                /* status-line window */
extern int D_2800;                  /* its text colour */
extern int D_9419[];
extern int D_9c46;                  /* party changed: reload icons */
extern int D_e48c[];                /* palette */
extern signed char D_0880, D_0881, D_0882;   /* joystick-ish x, y, button */
extern signed char D_e3fa, D_e3fb;
extern char D_978b[];               /* city description (DARKLAND.DSC record) */
extern int D_d843;                  /* terrain type under the party */

int sprintf(char *, const char *, ...);
struct _iobuf *fdopen(int, const char *);
int fseek(struct _iobuf *, long, int);
int fclose(struct _iobuf *);
int fgetc(struct _iobuf *);
unsigned fread(void *, unsigned, unsigned, struct _iobuf *);
unsigned far sub_root_06a1_2c9e(const char *, unsigned, int *);   /* _dos_open */
unsigned _dos_read(int, void far *, unsigned, unsigned *);

void far vec_1fdb(int);
extern int D_286e;                  /* map view: top row */
int far vec_1fe5(void);
int far vec_1c1b(void);
void far sub_root_047c_058b(int *);
void far sub_root_047c_055a(int *, int, int, char *);
void far sub_root_0000_1588(int);
void far sub_root_0000_1716(int);
void far sub_root_0000_0000(int *, int);
void far sub_root_0617_024a(int, char *, int *);
int far sub_root_047c_073d(void);
int far sub_root_047c_0744(void);
int far sub_s029_0e76_3470(int, int);
int far sub_s029_0e76_36d4(int, int);
int far sub_s029_1367_072a(void);
int far sub_s029_0e76_32fe(int);
void far sub_s029_0e76_2e74(int, int, int, int, int, int, int);
int far sub_s029_0e76_35aa(int, int);
void far sub_s030_1767_0046(long);
void far sub_s030_1462_2dd6(void);
int far sub_s030_1462_0c44(int);

extern int D_00e2;
extern int D_8676, D_290e, D_2910;
extern char D_907b;
void far vec_1ff9(int);
void far free_buffer(unsigned, char *);
void far delay_ticks(long);
void far sub_root_0000_13a0(void);
int far sub_s030_1462_1300(int, int);
void far sub_s030_1462_16c6(int, int, int, int);
int far sub_s030_1462_25be(void);
int far sub_s030_1462_271a(int, int);
int far sub_s030_1462_2ac8(int);
int far sub_s030_1462_2d44(int, int);
void far sub_s030_1462_2bb0(int);
void far sub_s029_0e76_3cde(int, int, int, int, int);
int abs(int);
struct find_t {
    char reserved[21];
    char attrib;
    unsigned wr_time;
    unsigned wr_date;
    long size;
    char name[13];
};
unsigned _dos_findfirst(char *, unsigned, struct find_t *);
void far exit_(int);
int far sub_root_047c_04d5(int, int, int, int, int);
void far sub_root_047c_04f8(void);
void far sub_root_047c_0545(void);
void far sub_root_047c_0952(int *, int, int, int, int, int);
void far sub_root_047c_0d62(int *, int, int, int, int, int);
int far sub_root_047c_051b(int);
void far sub_root_0265_0134(int *, char *, int, int, int, int, int, int, int, int, int, int);
void far sub_root_047c_05a5(int *, int, int, unsigned);
void far sub_root_047c_1293(void);
extern int D_2804;
#define ICONS ((unsigned (far *)[16])F_2e38_1ffc)
extern unsigned far F_2e38_2798[][8];  /* character icons */
extern unsigned D_e3fc;
extern unsigned D_e400[8];
void far sub_root_0617_0264(int, char *);
unsigned far sub_root_0617_045c(char *);
#define G F_2e38_239c
extern int D_866a, D_00e0;
extern int D_286a;                  /* map view: left column */
extern int D_286c;                  /* map view: party row in view */
extern int D_284e[];
extern int D_282a[];
extern signed char far F_290e_38db[][16], far F_290e_391b[][16], far F_290e_395b[][16];
void far sub_root_047c_04ea(int, int);
void far sub_root_047c_04ef(int *, int, int, unsigned);
void far sub_root_047c_0553(int *, int, int, int, int, int *, int, int);
extern int D_a67e;
extern unsigned char far F_2e38_239c[][19];   /* terrain grid around the party */
struct micon { unsigned y, x, icon, frame; int flag; };    /* map icon list, far 2e38:2838 */
extern struct micon far F_2e38_2838[];
extern unsigned char far *D_866c;   /* map rows buffer (0xd2 bytes per row) */
extern long D_2862;
unsigned far alloc_buffer(unsigned long, char *);
void far sub_root_06a1_3108(void far *, void far *, unsigned);
void far sub_s030_1462_1dd4(struct _iobuf *, int, int);
void far sub_s030_1767_00d2(int *);
void far sub_s029_1367_05c8(int);
int far sub_s029_0e76_3878(int, int);
int far sub_s029_0e76_3a20(int, int);
#define D_e48c_b ((unsigned char *)D_e48c)

/* overlay entry: place the party, then run the overland map */
int far sub_s030_1462_0000(void)
{
    int x, y;
    if (D_907e < 92 && D_907e != -1) {
        x = D_e3d0 = D_9785->exit_x;
        y = D_e3d2 = D_9785->exit_y;
    } else if (D_907e != -1) {
        x = D_e3d0 = F_2e38_0000.loc[D_907e]->x;
        y = D_e3d2 = F_2e38_0000.loc[D_907e]->y;
    } else {
        x = D_e3d0;
        y = D_e3d2;
    }
    if (sub_s029_0e76_32ce(0x47)) {
        return D_e48a = 0xf1;
    }
    if (!sub_s029_0e76_32ce(0x63)) {
        vec_1fd1();
        sub_s029_0e76_2930(0x63, 0x3e80);
    }
    return sub_s030_1462_00a8(x, y);
}

/* open DARKLAND.MAP, centre the view on (x, y), run the map loop */
int far sub_s030_1462_00a8(int x, int y)
{
    int row;
    int result;


    sub_root_047c_12c3();
    D_9c3d = vec_18f1(5);
    if (sub_root_0000_0c70(2))
        vec_18f1(0x14);
    else
        vec_18f1(0x13);
    D_e3f2 = sub_s029_0e76_1326(5);
    switch (D_00e4) {
    case 0: case 1: case 2:
        D_2860 = 0xf;
        break;
    case 3:
        D_2860 = 6;
        break;
    case 8: case 9:
        D_2860 = 6;
        break;
    case 10: case 11:
        D_2860 = 0xf;
        break;
    default:
        D_2860 = 0x68;
    }
    D_a891 = 3;
    D_8672 = 0;
    D_e488 = 0;
    if ((D_e3f4 = fopen("darkland.map", "rb")) == 0)
        printf("Error %d: Could not open map file\n", D_775a);
    sub_s030_1462_1c4e(D_e3f4);

    /* vertical: top row and party row within the view */
    row = y / 100 - 1;
    if (row < 0)
        row = 0;
    D_286e = row * 100;
    D_286c = y - D_286e - 16;
    D_2868 = 0x40;
    while (D_286c > 0x53) {
        D_286e += 24;
        D_286c = y - D_286e - 16;
    }
    if (D_e3f8 - D_286e - 0x30 < D_286c) {
        D_2868 = (D_286e - D_e3f8 + D_286c + 0x40) * 4;
        D_286e = D_e3f8 - 0x83;
        D_286c = 0x53;
    }
    if (D_286c < 0)
        D_286c = 0;

    /* horizontal */
    D_2866 = 0x82;
    D_286a = x - 8;
    if (D_286a > D_e3f6 - 0x11) {
        D_2866 = (D_286a - D_e3f6) * 16 + 0x192;
        D_286a = D_e3f6 - 0x11;
    } else if (D_286a <= 0) {
        D_2866 = 0x82 - D_286a;
        D_286a = 0;
    }

    sub_s030_1462_1c76(D_e3f4);
    sub_root_047c_0568();
    sub_s030_1462_1f30();
    sub_root_047c_124f(5, 0, D_9c42);
    sub_s030_1462_20d2(D_286a, D_286c);
    sub_root_047c_056f();
    result = sub_s030_1462_02c0();
    fcloseall();
    sub_root_05ba_00c2(D_2870, "Map");
    if (F_2e38_1ffc[0])
        sub_root_05ba_00c2(F_2e38_1ffc[0], "S");
    if (D_e416)
        _dos_freemem(D_e416);
    D_2870 = 0;
    F_2e38_1ffc[0] = 0;
    D_e416 = 0;
    return result;
}

/* the overland main loop: returns the next card/menu id */
int far sub_s030_1462_02c0(void)
{
    register int key;
    int fdf;
    struct _iobuf *fpk;
    int delayc;
    long posa;
    int sel;
    unsigned nreadk;
    int colh, rowo;
    char bufk[40];
    int unusede[5];
    int mxe, mye;

    delayc = 30;
    D_907e = -1;
    mxe = D_2866;
    mye = D_2868;
    for (;;) {
        if (D_6ef0 & 1) {
            if (D_6eec > 0x3c) {
                mye = D_6eee - 0x15;
                mxe = D_6eec - 0x41;
            } else {
                /* click on a party portrait */
                mxe = 0;
                sel = D_6eee / 40;
                if (D_9c69[sel].present) {
                    sub_root_05ba_00c2(F_2e38_1ffc[0], "S");
                    F_2e38_1ffc[0] = 0;
                    vec_1fdb(sel);
                    sub_root_047c_0568();
                    sub_s030_1462_1f30();
                    sub_s030_1462_2dd6();
                    sub_root_047c_058b(D_e48c);
                    sub_root_0000_1588(0x100);
                    sub_root_0000_1716(0x100);
                    sub_s030_1462_20d2(D_286a, D_286c);
                    sub_root_047c_056f();
                }
            }
        }
        if ((D_6ef0 & 2) && ((*BIOS_KBFLAGS & 2) || (*BIOS_KBFLAGS & 1))) {
            /* right button with shift: show map coordinates */
            D_2800 = 0xf;
            rowo = (D_2868 + 11) / 4;
            if ((rowo + D_286e + D_286c - 1) % 2)
                colh = (D_2866 + 9) / 16;
            else
                colh = (D_2866 + 17) / 16;
            rowo += D_286e + D_286c;
            colh += D_286a;
            sprintf(bufk, "character X:%02d Y:%02d", colh, rowo);
            sub_root_047c_055a(D_27f4, 0x58, 0x1f, bufk);
        }
        sub_root_0000_0000(D_9419, 0);
        if (D_9c46) {
            D_9c46 = 0;
            sub_root_0617_024a(1, "pics\\mapicons.pic", D_e48c);
            sub_root_047c_0568();
            sub_s030_1462_2dd6();
            sub_root_047c_058b(D_e48c);
            sub_root_0000_1588(0x100);
            sub_root_0000_1716(0x100);
            sub_s030_1462_20d2(D_286a, D_286c);
            sub_root_047c_056f();
        }

        /* direction towards the mouse target, as a keypad scan code */
        if (mxe == 0)
            key = 0x4c35;
        else if (D_2866 > mxe && D_2868 > mye)
            key = 0x4700;
        else if (D_2866 > mxe && D_2868 < mye)
            key = 0x4f00;
        else if (D_2866 < mxe && D_2868 > mye)
            key = 0x4900;
        else if (D_2866 < mxe && D_2868 < mye)
            key = 0x5100;
        else if (D_2866 > mxe)
            key = 0x4b00;
        else if (D_2866 < mxe)
            key = 0x4d00;
        else if (D_2868 > mye)
            key = 0x4800;
        else if (D_2868 < mye)
            key = 0x5000;
        else {
            key = 0x4c35;
            mxe = 0;
        }

        /* keyboard / joystick override the mouse */
        if (sub_root_047c_073d() || D_0880 || D_0881 || D_0882) {
            mxe = 0;
            key = sub_root_047c_0744();
            if (sub_root_047c_073d())
                key = sub_root_047c_0744();
            else if ((D_e3fa < -60 && D_e3fb < -60) || (D_0880 < 0 && D_0881 < 0))
                key = 0x4700;
            else if ((D_e3fa < -60 && D_e3fb > 60) || (D_0880 < 0 && D_0881 > 0))
                key = 0x4f00;
            else if ((D_e3fa > 60 && D_e3fb < -60) || (D_0880 > 0 && D_0881 < 0))
                key = 0x4900;
            else if ((D_e3fa > 60 && D_e3fb > 60) || (D_0880 > 0 && D_0881 > 0))
                key = 0x5100;
            else if (D_e3fa < -60 || D_0880 < 0)
                key = 0x4b00;
            else if (D_e3fa > 60 || D_0880 > 0)
                key = 0x4d00;
            else if (D_e3fb < -60 || D_0881 < 0)
                key = 0x4800;
            else if (D_e3fb > 60 || D_0881 > 0)
                key = 0x5000;
            else if (D_0882)
                key = 0x1c0d;
            D_0880 = D_0881 = D_0882 = 0;
        }

        sel = sub_s030_1462_0c44(key);
        if (sel >= 0) {
            if (sel < 92) {
                /* arrived at a city: load its CTY and DSC records */
                sub_root_06a1_2c9e("darkland.cty", 0, &fdf);
                fpk = fdopen(fdf, "r");
                posa = (long)sel * 0x26e;
                fseek(fpk, posa + 1, 0);
                _dos_read(fdf, D_9785, 0x26e, &nreadk);
                fclose(fpk);
                fpk = fopen("darkland.dsc", "r");
                fgetc(fpk);
                fseek(fpk, (long)(sel * 0x50 + 1), 0);
                fread(D_978b, 0x50, 1, fpk);
                fclose(fpk);
            }
            if (sel == 1000) {
                /* a day passed in the wilderness: random events */
                if (sub_s029_0e76_32ce(0x5f))
                    return sub_root_0000_0c70(2) ? 0x16a : 0x169;
                if (D_907e != -1 && sub_s029_0e76_3470(4, D_907e)) {
                    if (sub_s029_0e76_36d4(4, 1) % 2 == 0)
                        return 0x97;
                    switch (vec_1fe5()) {
                    case 0:
                        if (sub_s029_0e76_36d4(4, 5) == 0)
                            return 0x97;
                        return 0xa1;
                    case 1:
                        return 0x9e;
                    case 2:
                        return 0x9b;
                    case 3:
                        return 0x9c;
                    }
                } else if (!sub_s029_1367_072a() && !sub_root_0000_0c70(3) && sub_s029_0e76_32fe(7)) {
                    sub_s029_0e76_2e74(-2, -1, 0x1c, 5, 1, 7, 1);
                    sel = sub_s029_0e76_36d4(7, 5);
                    switch (sel) {
                    case 2:
                        return 0xa8;
                    case 3:
                        return sub_s029_0e76_35aa(7, 1) ? 0xa9 : 0xa8;
                    case 6: case 9:
                        return sub_s029_0e76_35aa(7, 2) ? 0x41 : 0xa9;
                    case 1:
                        return 0x41;
                    default:
                        if (sel % 3 == 0 && !sub_s029_0e76_35aa(7, 2))
                            return 0xa9;
                        return 0x41;
                    }
                } else if (!sub_s029_1367_072a() && vec_1c1b() > 4 && sub_s029_0e76_35aa(1, 2)) {
                    return 0x126;
                } else {
                    switch (D_d843) {
                    case 1: case 2: case 3:
                        break;
                    case 4:
                        switch (sub_root_0000_0c70(9)) {
                        case 0: case 1:
                            return 0x112;
                        case 2: case 3: case 4:
                            if (D_00e4 < 3 || D_00e4 > 9) return 0x117;
                            break;
                        case 5: case 6: return 0x116;
                        default: return 0x11b;
                        }
                        break;
                    case 5:
                        switch (sub_root_0000_0c70(0x11)) {
                        case 0: case 1:
                            return 0x112;
                        case 2: case 3: case 4:
                            if (D_00e4 < 3 || D_00e4 > 9) return 0x117;
                            break;
                        case 5: case 6:
                            return 0x116;
                        case 7: case 8:
                            return 0xa0;
                        case 9: case 10:
                            return 0x102;
                        case 11: case 12:
                            return 0x103;
                        case 13:
                            return 0x111;
                        default: return 0x122;
                        }
                        break;
                    case 6: case 7:
                        switch (sub_root_0000_0c70(0x11)) {
                        case 0: case 1: return 0x112;
                        case 2: case 3:
                            return 0xa5;
                        case 4: case 5:
                            return 0x107;
                        case 6: case 7: return 0xa0;
                        case 8: case 9: return 0x102;
                        case 10: case 11: return 0x103;
                        case 12: return 0x111;
                        case 13:
                            return 0x118;
                        default: return 0x122;
                        }
                        break;
                    case 8: case 9:
                        switch (sub_root_0000_0c70(0x1a)) {
                        case 0: case 1: return 0xa0;
                        case 2: case 3: return 0x102;
                        case 4: case 5: return 0x103;
                        case 6: case 7: case 8:
                            return 0x106;
                        case 9: case 10: return 0x107;
                        case 11: case 12: case 13: return 0x7f;
                        case 14: case 15: case 16: case 17: return 0x104;
                        case 18: case 19: return 0x105;
                        case 20: case 21: case 22: return 0x108;
                        case 23: case 24:
                            if (D_00e4 < 3 || D_00e4 > 9) return 0x117;
                            break;
                        default:
                            return 0x24;
                        }
                        break;
                    case 10: case 11:
                        switch (sub_root_0000_0c70(0x11)) {
                        case 0: case 1: return 0xa0;
                        case 2: case 3: return 0xa5;
                        case 4: case 5: return 0x102;
                        case 6: case 7: return 0x103;
                        case 8: case 9: case 10: return 0x106;
                        case 11: case 12: return 0x107;
                        case 13: return 0x112;
                        case 14: case 15:
                            if (D_00e4 < 3 || D_00e4 > 9) return 0x117;
                            break;
                        default: return 0x24;
                        }
                        break;
                    case 12: case 13: case 14: case 15:
                        switch (sub_root_0000_0c70(0x10)) {
                        case 0: case 1: return 0xa0;
                        case 2: case 3: return 0xa5;
                        case 4: case 5: return 0x102;
                        case 6: case 7: return 0x103;
                        case 8: return 0x113;
                        case 9: return 0x112;
                        case 10: return 0x122;
                        case 11: case 12:
                            if (D_00e4 < 3 || D_00e4 > 9) return 0x117;
                            break;
                        case 13: return 0x118;
                        default: return 0x24;
                        }
                        break;
                    case 16: case 17:
                        switch (sub_root_0000_0c70(0x10)) {
                        case 0: case 1: return 0xa0;
                        case 2: case 3: return 0xa5;
                        case 4: return 0x111;
                        case 5: return 0x113;
                        case 6: case 7: return 0x112;
                        case 8: case 9: return 0x122;
                        case 10: case 11: case 12:
                            if (D_00e4 < 3 || D_00e4 > 9) return 0x117;
                            break;
                        case 13: return 0x118;
                        default: return 0x24;
                        }
                        break;
                    case 18: case 19: case 20: case 21:
                        switch (sub_root_0000_0c70(0xf)) {
                        case 0: case 1: return 0xa0;
                        case 2: case 3: return 0x102;
                        case 4: case 5: return 0x103;
                        case 6: return 0x111;
                        case 7: case 8: return 0x112;
                        case 9: case 10: case 11:
                            if (D_00e4 < 3 || D_00e4 > 9) return 0x117;
                            break;
                        case 12: return 0x118;
                        default: return 0x24;
                        }
                        break;
                    case 22: case 23:
                        switch (sub_root_0000_0c70(9)) {
                        case 0: case 1: case 2: return 0xf0;
                        case 3: return 0x111;
                        default:
                            if (D_00e4 < 4 || D_00e4 > 8) return 0x117;
                            break;
                        }
                        break;
                    case 24:
                        switch (sub_root_0000_0c70(0x1d)) {
                        case 0: case 1: return 0xa0;
                        case 2: case 3: return 0xa6;
                        case 4: case 5: return 0xa7;
                        case 6: return 0x3d;
                        case 7: return 0xc3;
                        case 8: case 9: return 0x102;
                        case 10: case 11: return 0x103;
                        case 12: case 13: case 14: return 0x106;
                        case 15: case 16: return 0x107;
                        case 17: case 18: return 0x7f;
                        case 19: case 20: case 21: case 22: return 0x104;
                        case 23: case 24: return 0x105;
                        case 25: return 0x163;
                        case 26: return 0x108;
                        default: return 0x24;
                        }
                        break;
                    }
                }
            } else if (sel == 2000) {
                return F_2e38_0000.loc[D_907e]->unk0e;
            } else if (sel != 9999) {
                return F_2e38_0000.loc[D_907e]->menu;
            } else {
                mxe = 0;
            }
        }
        sub_s030_1767_0046((long)delayc);
    }
}

int far sub_s030_1462_0c44(register int key)
{
    int oel, kf, cold, rowk, oad, occ, oxo, oyo;

    rowk = (D_2868 + 11) / 4;
    cold = ((rowk + D_286e + D_286c - 1) % 2 ? D_2866 + 16 : D_2866 + 24) / 16;
    D_e3d0 = D_286a + cold - 1;
    D_e3d2 = rowk + D_286e + D_286c - 1;
    oxo = D_2866;
    oyo = D_2868;
    switch (key) {
    case 0x4b00:
        D_2866--;
        if (D_2866 < 11)
            D_2866 = 11;
        D_8676 = 7;
        break;
    case 0x4d00:
        D_2866++;
        if (D_2866 > 0xf7)
            D_2866 = 0xf7;
        D_8676 = 3;
        break;
    case 0x4800:
        if (D_290e++) {
            D_2868--;
            if (D_2868 < 13)
                D_2868 = 13;
        }
        D_8676 = 1;
        break;
    case 0x5000:
        if (D_290e++) {
            D_2868++;
            if (D_2868 > 0xb1)
                D_2868 = 0xb1;
        }
        D_8676 = 5;
        break;
    case 0x4700:
        D_2866--;
        if (D_2866 < 11)
            D_2866 = 11;
        if (D_290e++) {
            D_2868--;
            if (D_2868 < 13)
                D_2868 = 13;
        }
        D_8676 = 8;
        break;
    case 0x4900:
        D_2866++;
        if (D_2866 > 0xf7)
            D_2866 = 0xf7;
        if (D_290e++) {
            D_2868--;
            if (D_2868 < 13)
                D_2868 = 13;
        }
        D_8676 = 2;
        break;
    case 0x5100:
        D_2866++;
        if (D_2866 > 0xf7)
            D_2866 = 0xf7;
        if (D_290e++) {
            D_2868++;
            if (D_2868 > 0xb1)
                D_2868 = 0xb1;
        }
        D_8676 = 4;
        break;
    case 0x4f00:
        D_2866--;
        if (D_2866 < 11)
            D_2866 = 11;
        if (D_290e++) {
            D_2868++;
            if (D_2868 > 0xb1)
                D_2868 = 0xb1;
        }
        D_8676 = 6;
        break;
    case 0x4c35:
        D_8676 = 0;
        break;
    case 0x3b00: case 0x3c00: case 0x3d00: case 0x3e00: case 0x3f00:
        kf = (key >> 8) - 0x3b;
        vec_1fdb(kf);
        sub_root_0617_024a(1, "pics\\mapicons.pic", D_e48c);
        sub_root_047c_0568();
    redraw:
        sub_s030_1462_2dd6();
        sub_root_047c_058b(D_e48c);
        sub_root_0000_1588(0x100);
        sub_root_0000_1716(0x100);
        sub_s030_1462_20d2(D_286a, D_286c);
        sub_root_047c_056f();
        break;
    case 0x5e00: case 0x5f00: case 0x6000: case 0x6100: case 0x6200:
        kf = (key >> 8) - 0x5e;
        if (kf < D_a67e && D_9c69[kf].present == 1) {
            D_907b = kf;
            sub_root_0000_1716(0x100);
        }
        break;
    case 0x2e43: case 0x2e63: case 0x4200:
        free_buffer(F_2e38_1ffc[0], "S");
        F_2e38_1ffc[0] = 0;
        vec_1ff9(sub_s030_1462_25be());
        if (sub_s029_0e76_32ce(0x5f))
            return 1000;
        sub_root_047c_0568();
        sub_s030_1462_1f30();
        goto redraw;
    case 0x1e41: case 0x1e61: case 0x4100:
        D_2910 = 0;
        D_a891 = 1;
        goto test;
        do {
            if (D_6ef0 & 3)
                break;
            delay_ticks(75L);
            kf = sub_s030_1462_1300(D_2866, D_2868);
            D_a891 = 3;
            if (kf != -1)
                return kf;
            sub_s030_1462_16c6(D_2866, D_2868, D_2910++, 9);
            if (D_2910 >= 8)
                D_2910 = 0;
            delay_ticks(75L);
        test:;
        } while (!sub_root_047c_073d());
        D_a891 = 3;
        break;
    case 0x1000:
        free_buffer(D_2870, "M");
        free_buffer(F_2e38_1ffc[0], "S");
        if (D_e416)
            _dos_freemem(D_e416);
        D_2870 = 0;
        F_2e38_1ffc[0] = 0;
        D_e416 = 0;
        sub_root_0000_13a0();
        break;
    }
    if (D_290e > 1)
        D_290e = 0;
    if (D_8676 != 0) {
        kf = sub_s030_1462_1300(D_2866, D_2868);
        if (kf >= 0) {
            D_2866 = oxo;
            D_2868 = oyo;
            D_8676 = 0;
            return kf;
        }
    }
    occ = D_286c;
    oad = D_286a;
    oel = D_286e;
    if (D_2868 < 20 || D_2868 > 170) {
        oyo = D_2868 - 0x5c;
        if (D_2868 < 20)
            D_286c -= 23;
        else
            D_286c += 23;
        if (D_e3f8 - D_286e - 0x30 <= D_286c) {
            D_286e = D_e3f8 - 0x83;
            D_286c = D_e3f8 - D_286e - 0x30;
            D_2868 += (occ - D_286c) * 5;
        } else if (D_286c > 0x53) {
            if (D_286e + 0xc4 >= D_e3f8)
                D_286e = D_e3f8 - 0x83;
            else
                D_286e += 0x41;
            if (oel != D_286e) {
                sub_s030_1462_1dd4(D_e3f4, D_286e, oel);
                D_286c += oel - D_286e;
                D_2868 += D_2868 < 20 ? 92 : -92;
            }
            if (D_286c > 0x53)
                D_286c = 0x53;
        } else if (D_286c <= 1) {
            if (D_286e <= 0x41)
                D_286e = 0;
            else
                D_286e -= 0x41;
            if (oel != D_286e) {
                sub_s030_1462_1dd4(D_e3f4, D_286e, oel);
                D_286c += oel - D_286e;
                D_2868 += D_2868 < 20 ? 92 : -92;
            }
            if (D_286c <= 1) {
                D_286c = 1;
                D_2868 += (occ - 1) * 4;
            }
        } else {
            D_2868 += D_2868 < 20 ? 92 : -92;
        }
    }
    if (D_2866 < 20 || D_2866 > 0xe2) {
        oxo = D_2866 - 0x80;
        if (D_2866 < 20)
            D_286a -= 8;
        else
            D_286a += 8;
        if (D_e3f6 - 0x11 < D_286a) {
            D_286a = D_e3f6 - 0x11;
            D_2866 += (oad - D_286a) * 16;
        } else if (D_286a <= 0) {
            D_286a = 0;
            D_2866 += oad * 16;
        } else if (D_2866 < 20)
            D_2866 += 0x80;
        else
            D_2866 -= 0x80;
    }
    if (occ != D_286c || D_286a != oad)
        sub_s030_1462_20d2(D_286a, D_286c);
    sub_s030_1462_16c6(D_2866, D_2868, D_2910, D_8676);
    if (key)
        D_2910++;
    if (D_2910 >= 8)
        D_2910 = 0;
    return -1;
}

int D_290e = 0;
int D_2910 = 0;

int far sub_s030_1462_1300(int x, int y)
{
    int row, col, t, tt, r, i, best, found, dx, dy, d, m, w;
    unsigned char c;

    if (sub_s029_0e76_32ce(0x5d)) {
        sub_s029_1367_05c8(1);
        return D_907e = sub_s030_1462_271a(D_e3d0, D_e3d2);
    }
    row = (y + 11) / 4;
    col = ((row + D_286e + D_286c - 1) % 2 ? x + 16 : x + 24) / 16;
    c = G[row][col];
    t = c >= 50 ? 28 : c;
    if ((t == 1 || t == 2) && !sub_s029_0e76_32ce(0x4b) && D_d843 != 1 && D_d843 != 2)
        return 9999;
    D_d843 = t;
    D_8672 += sub_s030_1462_2ac8(t);
    if (D_8672 >= 60) {
        sub_s029_1367_05c8(1);
        sub_root_047c_05a5(D_27f4, 289, 0, D_e400[D_00e0 / 3]);
        D_8672 -= 60;
        m = D_2860;
        sub_s030_1462_2dd6();
        if (m != D_2860)
            sub_s030_1462_20d2(D_286a, D_286c);
    }
    tt = t;
    if (t >= 25 && t != 27 && t != 25) {
        r = t == 29 ? 4 : 1;
        for (i = 0; i < 414; i++) {
            if (D_286a - r + col - 1 <= F_2e38_0000.loc[i]->x
                    && D_286a + col + r + 1 > F_2e38_0000.loc[i]->x
                    && row - r + D_286e + D_286c - 1 <= F_2e38_0000.loc[i]->y
                    && row + D_286e + D_286c + r + 1 > F_2e38_0000.loc[i]->y
                    && F_2e38_0000.loc[i]->menu != 0x62)
                return D_907e = i;
        }
    }
    best = 100;
    found = -1;
    for (i = 0; i < 414; i++) {
        if ((-1 - F_2e38_0000.loc[i]->unk08) / 2 + D_286a + col > F_2e38_0000.loc[i]->x)
            continue;
        if ((F_2e38_0000.loc[i]->unk08 + 1) / 2 + D_286a + col <= F_2e38_0000.loc[i]->x)
            continue;
        if (row - F_2e38_0000.loc[i]->unk08 + D_286e + D_286c - 1 > F_2e38_0000.loc[i]->y)
            continue;
        if (F_2e38_0000.loc[i]->unk08 + row + D_286e + D_286c + 1 <= F_2e38_0000.loc[i]->y)
            continue;
        dx = F_2e38_0000.loc[i]->x - col - D_286a;
        if (dx < 0)
            dx = -dx;
        dy = F_2e38_0000.loc[i]->y - D_286e - D_286c - row;
        if (dy < 0)
            dy = -dy;
        d = dy <= dx ? dy / 2 + dx : dx / 2 + dy;
        if (d < best) {
            if (found != i && (m = sub_s029_0e76_3a20(0x2b, i)) != 0) {
                sub_s029_0e76_3cde(0x2b, -1, i, -1, -1);
                sub_s030_1462_2bb0(m);
            }
            found = i;
            best = d;
        }
    }
    if (found != -1 && F_2e38_0000.loc[found]->icon == 15)
        vec_18f1(0x18);
    t = tt;
    if (t == 1 || t == 2)
        return -1;
    if (D_e488 < 10 && sub_root_0000_0c70(500) <= 9)
        D_e488++;
    w = sub_s030_1462_2d44(t, found);
    if ((D_e488 + 1) * w < sub_root_0000_0c70(1000))
        return -1;
    D_e3d0 = D_286a + col - 1;
    D_e3d2 = row + D_286e + D_286c - 1;
    D_907e = found;
    if (found != -1 && F_2e38_0000.loc[found]->unk0e != 0x62 && sub_root_0000_0c70(100) <= 60)
        return 2000;
    return 1000;
}

void far sub_s030_1462_16c6(int x, int y, int frame, int dir)
{
    register int cc, r;
    register int xa;
    int tl, dyd, oddk, pyj, maskh, fxg, fyg, coli, row, pxa, cg;

    sub_root_047c_0952(D_282a, 0, pyj = 0, 0x37, 0x32, D_2860);
    fyg = (y + 11) % 4;
    row = (y + 11) / 4;
    xa = (row + D_286e + D_286c - 1) % 2 ? x + 16 : x + 24;
    coli = xa / 16;
    fxg = xa % 16;
    tl = G[row][coli] > 50 ? 28 : G[row][coli];
    D_d843 = tl;
    switch (tl) {
    case 16: case 17: case 18: case 19:
        dyd = F_290e_391b[fyg][fxg];
        break;
    case 20: case 21: case 22: case 23:
        dyd = F_290e_38db[fyg][fxg];
        break;
    case 3: case 4: case 25:
        dyd = F_290e_395b[fyg][fxg];
        break;
    default:
        dyd = 0;
        break;
    }
    for (r = row - 6; r <= row + 3; r++) {
        oddk = (D_286e + D_286c + r - 1) % 2;
        if (oddk)
            pxa = 8;
        else
            pxa = 0;
        for (cc = coli - 1; cc <= coli + 2; cc++) {
            maskh = 0;
            cg = G[r][cc];
            if (cg == 1 || cg == 2 || cg == 3 || cg == 27 || cg == 25) {
                if (G[r - 1][cc - 1 + oddk] == 1 || G[r - 1][cc - 1 + oddk] == 2 || G[r - 1][cc - 1 + oddk] == 3 || G[r - 1][cc - 1 + oddk] == 25 || G[r - 1][cc - 1 + oddk] == 27)
                    maskh |= 1;
                if (G[r - 1][cc + oddk] == 1 || G[r - 1][cc + oddk] == 2 || G[r - 1][cc + oddk] == 3 || G[r - 1][cc + oddk] == 25 || G[r - 1][cc + oddk] == 27)
                    maskh |= 2;
                if (G[r + 1][cc - 1 + oddk] == 1 || G[r + 1][cc - 1 + oddk] == 2 || G[r + 1][cc - 1 + oddk] == 3 || G[r + 1][cc - 1 + oddk] == 25 || G[r + 1][cc - 1 + oddk] == 27)
                    maskh |= 4;
                if (G[r + 1][cc + oddk] == 1 || G[r + 1][cc + oddk] == 2 || G[r + 1][cc + oddk] == 3 || G[r + 1][cc + oddk] == 25 || G[r + 1][cc + oddk] == 27)
                    maskh |= 8;
                sub_root_047c_04ef(D_282a, pxa, pyj, ICONS[cg - 1][maskh]);
            } else if (cg == 4 || cg == 5 || cg == 6 || cg == 7) {
                if (G[r - 1][cc - 1 + oddk] == 4 || G[r - 1][cc - 1 + oddk] == 5 || G[r - 1][cc - 1 + oddk] == 6 || G[r - 1][cc - 1 + oddk] == 7)
                    maskh |= 1;
                if (G[r - 1][cc + oddk] == 4 || G[r - 1][cc + oddk] == 5 || G[r - 1][cc + oddk] == 6 || G[r - 1][cc + oddk] == 7)
                    maskh |= 2;
                if (G[r + 1][cc - 1 + oddk] == 4 || G[r + 1][cc - 1 + oddk] == 5 || G[r + 1][cc - 1 + oddk] == 6 || G[r + 1][cc - 1 + oddk] == 7)
                    maskh |= 4;
                if (G[r + 1][cc + oddk] == 4 || G[r + 1][cc + oddk] == 5 || G[r + 1][cc + oddk] == 6 || G[r + 1][cc + oddk] == 7)
                    maskh |= 8;
                sub_root_047c_04ef(D_282a, pxa, pyj, ICONS[cg - 1][maskh]);
            } else if (cg == 24) {
                if (G[r - 1][cc - 1 + oddk] == 24 || G[r - 1][cc - 1 + oddk] >= 25 || G[r - 1][cc - 1 + oddk] == 27)
                    maskh |= 1;
                if (G[r - 1][cc + oddk] == 24 || G[r - 1][cc + oddk] >= 25 || G[r - 1][cc + oddk] == 27)
                    maskh |= 2;
                if (G[r + 1][cc - 1 + oddk] == 24 || G[r + 1][cc - 1 + oddk] >= 25 || G[r + 1][cc - 1 + oddk] == 27)
                    maskh |= 4;
                if (G[r + 1][cc + oddk] == 24 || G[r + 1][cc + oddk] >= 25 || G[r + 1][cc + oddk] == 27)
                    maskh |= 8;
                sub_root_047c_04ef(D_282a, pxa, pyj, ICONS[cg - 1][maskh]);
            } else if (cg >= 50) {
                sub_root_047c_04ef(D_282a, pxa, pyj, F_2e38_1ffc[cg + 382]);
            } else if (cg != 0) {
                if (G[r - 1][cc - 1 + oddk] == cg)
                    maskh |= 1;
                if (G[r - 1][cc + oddk] == cg)
                    maskh |= 2;
                if (G[r + 1][cc - 1 + oddk] == cg)
                    maskh |= 4;
                if (G[r + 1][cc + oddk] == cg)
                    maskh |= 8;
                sub_root_047c_04ef(D_282a, pxa, pyj, ICONS[cg - 1][maskh]);
            }
            pxa += 16;
        }
        if ((row == r && tl <= 9) || (row - r == 1 && fyg == 0 && tl >= 20)
                || (row == r && tl >= 15 && fyg != 0) || (row - r == 1 && tl >= 9 && tl <= 19))
            sub_root_047c_04ef(D_282a, (row + D_286e + D_286c - 1) % 2 * 8 + fxg + 10, fyg - dyd + 18,
                               F_2e38_2798[dir][frame]);
        pyj += 4;
    }
    sub_root_047c_12c3();
    sub_root_047c_0553(D_282a, ((row + D_286e + D_286c - 1) % 2 + 1) * 8 + fxg, fyg - dyd + 14, 17, 21,
                       D_27f4, x + 52, y - dyd + 5);
    sub_root_047c_1293();
}

void far sub_s030_1462_1c4e(struct _iobuf *f)
{
    fread(&D_e3f6, 4, 1, f);
    sub_s030_1767_00d2(&D_e3f6);
    sub_s030_1767_00d2(&D_e3f8);
}

void far sub_s030_1462_1c76(struct _iobuf *f)
{
    int k, n;

    D_2870 = alloc_buffer(30000L, "map");
    D_866c = (unsigned char far *)((unsigned long)D_2870 << 16);
    sub_s030_1462_1dd4(f, D_286e, -1);
    for (k = 92, n = 0; k < 414; k++) {
        if (F_2e38_0000.loc[k]->menu == 0x62 || F_2e38_0000.loc[k]->icon == 15)
            continue;
        F_2e38_2838[n].x = F_2e38_0000.loc[k]->x;
        F_2e38_2838[n].y = F_2e38_0000.loc[k]->y;
        switch (F_2e38_0000.loc[k]->icon) {
        default:
            F_2e38_2838[n].icon = 0x1c;
            break;
        case 3:
            F_2e38_2838[n].icon = 0x33;
            break;
        case 5: case 15: case 19: case 24:
            F_2e38_2838[n].icon = 0x38;
            break;
        case 6:
            F_2e38_2838[n].icon = 0x37;
            break;
        case 8: case 10:
            F_2e38_2838[n].icon = 0x1d;
            F_2e38_2838[n].frame = k % 16;
            break;
        case 9:
            F_2e38_2838[n].icon = 0x3e;
            break;
        case 13:
            F_2e38_2838[n].icon = 0x39;
            break;
        case 16:
            F_2e38_2838[n].icon = 0x36;
            break;
        case 17:
            F_2e38_2838[n].icon = 0x3b;
            break;
        case 18:
            F_2e38_2838[n].icon = 0x35;
            break;
        case 20:
            F_2e38_2838[n].icon = 0x3c;
            break;
        case 21:
            F_2e38_2838[n].icon = 0x3d;
            break;
        case 22:
            F_2e38_2838[n].icon = 0x3f;
            break;
        case 23:
            F_2e38_2838[n].icon = 0x40;
            break;
        case 26:
            F_2e38_2838[n].icon = 0x3a;
            break;
        case 4: F_2e38_2838[n].icon = 0x39; break;
        }
        F_2e38_2838[n].flag = 0;
        n++;
    }
    F_2e38_2838[n].flag = -1;
}

void far sub_s030_1462_1dd4(struct _iobuf *f, int top, int old)
{
    unsigned off;
    unsigned first, rows;
    unsigned r;
    long pos;
    unsigned x, w;
    unsigned d;

    if (old == top)
        return;
    if (old == -1) {
        first = 0;
        rows = 0x83;
        off = top << 2;
    } else if (old < top) {
        d = top - old;
        sub_root_06a1_3108(D_866c, D_866c + d * 0xd2, (0x83 - d) * 0xd2);
        first = d;
        rows = 0x83 - d;
        off = (d + top) << 2;
    } else {
        old -= top;
        sub_root_06a1_3108(D_866c + old * 0xd2, D_866c, old * 0xd2);
        first = 0;
        rows = old;
        off = top << 2;
    }
    fseek(f, (long)(off + 4), 0);
    fread(&pos, 4, 1, f);
    fseek(f, pos, 0);
    for (r = first; r < first + rows; r++) {
        x = 0;
        w = 0;
        while (w < D_e3f6) {
            D_866c[r * 0xd2 + x] = fgetc(f);
            w += D_866c[r * 0xd2 + x] >> 5;
            x++;
            if ((long)x > D_2862)
                D_2862 = x;
        }
    }
}

void far sub_s030_1462_1f30(void)
{
    int r, c, k;

    sub_root_047c_0545();
    sub_root_0617_024a(1, "pics\\mapicons.pic", D_e48c);
    for (r = 1; r < 16; r++)
        for (c = 0; c < 16; c++)
            ICONS[r - 1][c] = sub_root_047c_04d5(1, c * 16, r * 12, 16, 12);
    sub_root_0617_0264(1, "pics\\mapicon2.pic");
    for (k = 0; k < 16; k++)
        for (c = 0; c < 16; c++)
            ICONS[r - 1 + k][c] = sub_root_047c_04d5(1, c * 16, k * 12, 16, 12);
    sub_root_0617_0264(1, "pics\\charicon.pic");
    for (r = 0; r < 10; r++)
        for (c = 0; c < 8; c++)
            F_2e38_2798[r][c] = sub_root_047c_04d5(1, c * 14 + 1, r * 14 + 1, 13, 13);
    D_e3fc = sub_root_0617_045c("pics\\mapheada.pic");
    D_e400[0] = sub_root_0617_045c("pics\\mapbrs00.pic");
    D_e400[1] = sub_root_0617_045c("pics\\mapbrs03.pic");
    D_e400[2] = sub_root_0617_045c("pics\\mapbrs06.pic");
    D_e400[3] = sub_root_0617_045c("pics\\mapbrs09.pic");
    D_e400[4] = sub_root_0617_045c("pics\\mapbrs12.pic");
    D_e400[5] = sub_root_0617_045c("pics\\mapbrs15.pic");
    D_e400[6] = sub_root_0617_045c("pics\\mapbrs18.pic");
    D_e400[7] = sub_root_0617_045c("pics\\mapbrs21.pic");
    sub_root_047c_04f8();
    sub_root_047c_0568();
    sub_s030_1462_2dd6();
    sub_root_047c_058b(D_e48c);
    sub_root_0000_1588(0x100);
    sub_root_0000_1716(0x100);
}

void far sub_s030_1462_20d2(int col, int row)
{
    register int x, y;
    int pyg, kh, outl, maskf, te, oddc, tope, pxo;
    unsigned char cntl, valc;
    int gam[6], gbb[1];

    if (col == 0)
        col = 1;
    if (row == 0)
        row = 1;
    pyg = 0;
    D_866a = 0;
    sub_root_047c_04ea(1, D_2860);
    for (y = 0; y < 50; y++) {
        x = 0;
        kh = 0;
        outl = 0;
        while (col + 18 > kh) {
            te = D_866c[(row + y - 1) * 0xd2 + x];
            valc = te & 0x1f;
            cntl = te >> 5;
            goto test;
            do {
                if (outl >= 19)
                    break;
                if (kh++ >= col - 1) {
                    G[y][outl] = valc;
                    outl++;
                }
            test:;
            } while (cntl--);
            x++;
        }
    }
    for (y = 0, tope = row + D_286e - 1; F_2e38_2838[y].flag != -1; y++) {
        if (F_2e38_2838[y].y >= tope && F_2e38_2838[y].x >= col - 1 && F_2e38_2838[y].x <= col + 18
                && F_2e38_2838[y].y <= tope + 50)
            G[F_2e38_2838[y].y - tope][F_2e38_2838[y].x - col + 1] = F_2e38_2838[y].icon;
    }
    for (y = 1; y <= 48; y++) {
        oddc = (row + D_286e + y - 1) % 2;
        if (oddc)
            pxo = 8;
        else
            pxo = 0;
        for (x = 1; x <= 17; x++) {
            maskf = 0;
            te = G[y][x];
            if (te == 1 || te == 2 || te == 3 || te == 27 || te == 25) {
                if (G[y - 1][x - 1 + oddc] == 1 || G[y - 1][x - 1 + oddc] == 2 || G[y - 1][x - 1 + oddc] == 3 || G[y - 1][x - 1 + oddc] == 25 || G[y - 1][x - 1 + oddc] == 27)
                    maskf |= 1;
                if (G[y - 1][x + oddc] == 1 || G[y - 1][x + oddc] == 2 || G[y - 1][x + oddc] == 3 || G[y - 1][x + oddc] == 25 || G[y - 1][x + oddc] == 27)
                    maskf |= 2;
                if (G[y + 1][x - 1 + oddc] == 1 || G[y + 1][x - 1 + oddc] == 2 || G[y + 1][x - 1 + oddc] == 3 || G[y + 1][x - 1 + oddc] == 25 || G[y + 1][x - 1 + oddc] == 27)
                    maskf |= 4;
                if (G[y + 1][x + oddc] == 1 || G[y + 1][x + oddc] == 2 || G[y + 1][x + oddc] == 3 || G[y + 1][x + oddc] == 25 || G[y + 1][x + oddc] == 27)
                    maskf |= 8;
                sub_root_047c_04ef(D_284e, pxo, pyg, ICONS[te - 1][maskf]);
            } else if (te == 4 || te == 5 || te == 6 || te == 7) {
                if (G[y - 1][x - 1 + oddc] == 4 || G[y - 1][x - 1 + oddc] == 5 || G[y - 1][x - 1 + oddc] == 6 || G[y - 1][x - 1 + oddc] == 7)
                    maskf |= 1;
                if (G[y - 1][x + oddc] == 4 || G[y - 1][x + oddc] == 5 || G[y - 1][x + oddc] == 6 || G[y - 1][x + oddc] == 7)
                    maskf |= 2;
                if (G[y + 1][x - 1 + oddc] == 4 || G[y + 1][x - 1 + oddc] == 5 || G[y + 1][x - 1 + oddc] == 6 || G[y + 1][x - 1 + oddc] == 7)
                    maskf |= 4;
                if (G[y + 1][x + oddc] == 4 || G[y + 1][x + oddc] == 5 || G[y + 1][x + oddc] == 6 || G[y + 1][x + oddc] == 7)
                    maskf |= 8;
                sub_root_047c_04ef(D_284e, pxo, pyg, ICONS[te - 1][maskf]);
            } else if (te == 24) {
                if (G[y - 1][x - 1 + oddc] == 24 || G[y - 1][x - 1 + oddc] >= 25 || G[y - 1][x - 1 + oddc] == 27)
                    maskf |= 1;
                if (G[y - 1][x + oddc] == 24 || G[y - 1][x + oddc] >= 25 || G[y - 1][x + oddc] == 27)
                    maskf |= 2;
                if (G[y + 1][x - 1 + oddc] == 24 || G[y + 1][x - 1 + oddc] >= 25 || G[y + 1][x - 1 + oddc] == 27)
                    maskf |= 4;
                if (G[y + 1][x + oddc] == 24 || G[y + 1][x + oddc] >= 25 || G[y + 1][x + oddc] == 27)
                    maskf |= 8;
                sub_root_047c_04ef(D_284e, pxo, pyg, ICONS[te - 1][maskf]);
            } else if (te >= 50) {
                sub_root_047c_04ef(D_284e, pxo, pyg, F_2e38_1ffc[te + 382]);
            } else if (te != 0) {
                if (G[y - 1][x - 1 + oddc] == te)
                    maskf |= 1;
                if (G[y - 1][x + oddc] == te)
                    maskf |= 2;
                if (G[y + 1][x - 1 + oddc] == te)
                    maskf |= 4;
                if (G[y + 1][x + oddc] == te)
                    maskf |= 8;
                sub_root_047c_04ef(D_284e, pxo, pyg, ICONS[te - 1][maskf]);
            }
            pxo += 16;
        }
        pyg += 4;
    }
    sub_root_047c_12c3();
    sub_root_047c_05a5(D_27f4, 60, 0, D_e3fc);
    sub_root_047c_05a5(D_27f4, 289, 0, D_e400[D_00e0 / 3]);
    sub_root_047c_0553(D_284e, 8, 8, 260, 184, D_27f4, 60, 16);
    sub_root_047c_1293();
}

int far sub_s030_1462_25be(void)
{
    int res, row, col, speed, i, j;

    res = 0;
    row = (D_2868 + 11) / 4;
    col = ((row + D_286e + D_286c - 1) % 2 ? D_2866 + 16 : D_2866 + 24) / 16;
    speed = D_a67e * 3;
    for (i = 0; i < 10; i++) {
        for (j = 0; j < 10; j++) {
            if (F_2e38_239c[row + j][col + i] == 0x1d) {
                i = i > 5 ? i - 5 : abs(i - 5);
                j = j > 5 ? j - 5 : abs(j - 5);
                if (j > i)
                    j = i;
                res = j;
                i = 99;
                break;
            }
        }
    }
    if (res)
        speed += abs(res - 6);
    if (F_2e38_239c[row][col] == 5 || F_2e38_239c[row][col] == 4 || F_2e38_239c[row][col] == 15 || F_2e38_239c[row][col] == 19)
        speed--;
    else if (F_2e38_239c[row][col] == 16 || F_2e38_239c[row][col] == 20)
        speed -= 2;
    else if (F_2e38_239c[row][col] == 17 || F_2e38_239c[row][col] == 21)
        speed -= 3;
    else if (F_2e38_239c[row][col] == 22)
        speed -= 5;
    else if (F_2e38_239c[row][col] == 23)
        speed -= 6;
    if (F_2e38_239c[row][col] == 24)
        speed++;
    if (F_2e38_239c[row][col] == 8 || F_2e38_239c[row][col] == 9)
        speed += 2;
    return speed;
}

int far sub_s030_1462_271a(int x, int y)
{
    int best, min, i, dx, dy, d;

    best = -1;
    min = 9999;
    for (i = 0; i < 92; i++) {
        if (F_2e38_0000.loc[i]->x == x && F_2e38_0000.loc[i]->y == y && F_2e38_0000.loc[i]->icon == 0)
            continue;
        dx = abs(F_2e38_0000.loc[i]->x - x);
        dy = abs(F_2e38_0000.loc[i]->y - y) / 3;
        d = dx >= dy ? dy / 2 + dx : dx / 2 + dy;
        if (d < min) {
            min = d;
            best = i;
        }
    }
    return best;
}

int far sub_s030_1462_27aa(int x, int y)
{
    int best, min, i, dx, dy, d;

    best = -1;
    min = 9999;
    for (i = 0; i < 92; i++) {
        if (F_2e38_0000.loc[i]->x == x && F_2e38_0000.loc[i]->y == y && F_2e38_0000.loc[i]->unk17
                && F_2e38_0000.loc[i]->icon == 0)
            continue;
        dx = abs(F_2e38_0000.loc[i]->x - x);
        dy = abs(F_2e38_0000.loc[i]->y - y) / 3;
        d = dx >= dy ? dy / 2 + dx : dx / 2 + dy;
        if (d < min) {
            min = d;
            best = i;
        }
    }
    return best;
}

int far sub_s030_1462_2842(int x, int y, int icon)
{
    int best, min, i, dx, dy, d;

    best = -1;
    min = 9999;
    for (i = 0; i < 414; i++) {
        if (F_2e38_0000.loc[i]->icon != icon)
            continue;
        if (F_2e38_0000.loc[i]->x == x && F_2e38_0000.loc[i]->y == y)
            continue;
        if (F_2e38_0000.loc[i]->flags & 4)
            continue;
        dx = abs(F_2e38_0000.loc[i]->x - x);
        dy = abs(F_2e38_0000.loc[i]->y - y) / 3;
        d = dx >= dy ? dy / 2 + dx : dx / 2 + dy;
        if (d < min) {
            min = d;
            best = i;
        }
    }
    if (min == 9999 && D_907e != -1)
        return D_907e;
    return best;
}

int far sub_s030_1462_28f2(int x, int y, int icon)
{
    int best, r, max, i, dx, dy, d;

    best = -1;
    r = sub_root_0000_0c70(100);
    max = 0;
    for (i = 0; i < 414; i++) {
        if (F_2e38_0000.loc[i]->icon != icon)
            continue;
        if (F_2e38_0000.loc[i]->x == x && F_2e38_0000.loc[i]->y == y)
            continue;
        dx = abs(F_2e38_0000.loc[i]->x - x);
        dy = abs(F_2e38_0000.loc[i]->y - y) / 3;
        d = dx >= dy ? dy / 2 + dx : dx / 2 + dy;
        if (d > max && d < r + 100) {
            max = d;
            best = i;
        }
    }
    if (best != -1)
        return best;
    return sub_s030_1462_2842(x, y, icon);
}

int far sub_s030_1462_29fa(int, int, int, int);
int far sub_s030_1462_29bc(int k)
{
    return sub_s030_1462_29fa(F_2e38_0000.loc[D_907e]->x, F_2e38_0000.loc[D_907e]->y,
                              F_2e38_0000.loc[k]->x, F_2e38_0000.loc[k]->y);
}

int far sub_s030_1462_29fa(int x1, int y1, int x2, int y2)
{
    int dx, dy;

    dx = abs(x2 - x1);
    dy = abs(y2 - y1) / 3;
    if (x2 <= x1) {
        if (y2 <= y1) {
            if (dx / 2 >= dy)
                return 6;
            if (dy / 2 >= dx)
                return 0;
            return 7;
        } else {
            if (dx / 2 >= dy)
                return 6;
            if (dy / 2 >= dx)
                return 4;
            return 5;
        }
    } else {
        if (y2 <= y1) {
            if (dx / 2 >= dy)
                return 2;
            if (dy / 2 >= dx)
                return 0;
            return 1;
        } else {
            if (dx / 2 >= dy)
                return 2;
            if (dy / 2 >= dx)
                return 4;
            return 3;
        }
    }
}

int far sub_s030_1462_2ac8(int p_6)
{
    int v_di;
    int v_si;
    v_di = 9;
    switch (p_6) {
    case 1:
    case 2:
    case 0x10:
        v_di = 0x28;
        break;
    case 3:
    case 0x13:
        v_di = 0x1e;
        break;
    case 4:
    case 5:
        v_di = 0x32;
        break;
    case 6:
        v_di = 0x11;
        break;
    case 7:
        v_di = 0xd;
        break;
    case 9:
        v_di = 0xb;
        break;
    case 0xa:
        v_di = 0xa;
        break;
    case 0xb:
        v_di = 0xc;
        break;
    case 0xc:
        v_di = 0xe;
        break;
    case 0xd:
        v_di = 0x10;
        break;
    case 0xe:
        v_di = 0x12;
        break;
    case 0xf:
        v_di = 0x1b;
        break;
    case 0x11:
        v_di = 0x3c;
        break;
    case 0x12:
        v_di = 0x14;
        break;
    case 0x14:
        v_di = 0x2d;
        break;
    case 0x15:
        v_di = 0x43;
        break;
    case 0x16:
        v_di = 0x62;
        break;
    case 0x17:
        v_di = 0xc5;
        break;
    case 0x18:
        v_di = 6;
    }
    if (v_di >= 0x5a0) {
        v_si = (unsigned)v_di / 0x5a0;
        v_di += v_si * -0x5a0;
        do {
            sub_s029_1367_05c8(1);
            v_si--;
        } while (v_si);
    }
    return v_di;
}

#pragma check_stack(on)
void far sub_s030_1462_2bb0(int msg)
{
    char *fmt;
    struct find_t ft;
    char buf[400];
    struct _iobuf *f;
    unsigned seg;

    fmt = "Could not open %s file\n";
    while (D_6ef0 & 1)
        ;
    if (_dos_findfirst("darkland.msg", 0, &ft)) {
        printf(fmt, "map message description");
        exit_(1);
    } else {
        f = fopen("darkland.msg", "r");
        fgetc(f);
        fseek(f, (long)(msg * 400 + 1), 0);
        fread(buf, 400, 1, f);
        fclose(f);
    }
    sub_root_047c_12c3();
    sub_root_047c_0545();
    seg = sub_root_047c_04d5(0, 100, 50, 180, 115);
    sub_root_047c_04f8();
    sub_root_047c_0952(D_27f4, 100, 50, 180, 115, 7);
    sub_root_047c_0d62(D_27f4, 100, 50, 279, 50, 8);
    sub_root_047c_0d62(D_27f4, 279, 50, 279, 164, 8);
    sub_root_047c_0d62(D_27f4, 279, 164, 100, 164, 8);
    sub_root_047c_0d62(D_27f4, 100, 164, 100, 50, 8);
    D_2800 = 0;
    sub_root_0265_0134(D_27f4, buf, 170, 105, 55, sub_root_047c_051b(D_2804), 0, 0, 0, 1, 0, 0);
    fmt = (char *)seg;
    while (!sub_root_047c_073d() && !(D_6ef0 & 1) && !(D_6ef0 & 2))
        ;
    sub_root_047c_05a5(D_27f4, 100, 50, seg);
    sub_root_047c_1293();
    _dos_freemem(seg);
}
#pragma check_stack(off)

int far sub_s030_1462_2d44(int type, int loc)
{
    int r;

    r = 1;
    switch (type) {
    case 4: case 5: case 16: case 17: case 20: case 21:
        r = 3;
        break;
    case 6: case 7: case 14: case 15: case 18: case 19: case 22: case 23:
        r = 2;
        break;
    }
    if (loc >= 0 && loc <= 0x19e) {
        if (sub_s029_0e76_3878(0x2a, loc))
            r += sub_s029_0e76_3a20(0x2a, loc);
        else
            r += F_2e38_0000.loc[loc]->unk0a;
    }
    return r;
}

#pragma optimize("a", off)
void far sub_s030_1462_2dd6(void)
{
    int changed, old, v;

    changed = 0;
    old = D_2860;
    switch (D_00e4) {
    case 0: case 1: case 2: case 11:
        if (D_e48c_b[0xde] != 0x2e)
            changed = 1;
        D_e48c_b[0xeb] = 0x30;
        D_e48c_b[0xdf] = 0x34;
        D_e48c_b[0xe8] = 0x34;
        D_e48c_b[0xef] = 0x34;
        D_e48c_b[0xf0] = 0x2A;
        D_e48c_b[0xf1] = 0x28;
        D_e48c_b[0xf2] = 0x31;
        D_e48c_b[0xde] = 0x2E;
        D_e48c_b[0xed] = 0x2E;
        D_e48c_b[0x10b] = 0x2E;
        D_e48c_b[0xee] = 0x2C;
        D_e48c_b[0x10c] = 0x2C;
        D_e48c_b[0xea] = 0x32;
        D_e48c_b[0x10d] = 0x32;
        D_e48c_b[0xe9] = 0x3C;
        D_e48c_b[0x142] = 0x3C;
        D_e48c_b[0x144] = 0x3C;
        D_e48c_b[0x14a] = 0x3C;
        D_e48c_b[0xec] = 0x38;
        D_e48c_b[0x111] = 0x38;
        D_e48c_b[0x13b] = 0x38;
        D_e48c_b[0x145] = 0x38;
        D_e48c_b[0x14b] = 0x38;
        D_e48c_b[0x1f2] = 0x38;
        D_e48c_b[0x1f3] = 0x38;
        D_e48c_b[0x216] = 0x38;
        D_e48c_b[0x112] = 0x33;
        D_e48c_b[0x13c] = 0x33;
        D_e48c_b[0x217] = 0x33;
        D_e48c_b[0x141] = 0x3B;
        D_e48c_b[0x22b] = 0x3B;
        D_e48c_b[0x146] = 0x3A;
        D_e48c_b[0x14c] = 0x3A;
        D_e48c_b[0x22c] = 0x3A;
        D_e48c_b[0x113] = 0x3F;
        D_e48c_b[0x13d] = 0x3F;
        D_e48c_b[0x143] = 0x3F;
        D_e48c_b[0x1f4] = 0x3F;
        D_e48c_b[0x218] = 0x3F;
        D_e48c_b[0x22d] = 0x3F;
        D_e48c_b[0x2be] = 0x25;
        D_e48c_b[0x2bf] = 0x2F;
        D_e48c_b[0xe0] = 0x37;
        D_e48c_b[0xe7] = 0x37;
        D_e48c_b[0x2c0] = 0x37;
        v = 15;
        goto done;
    case 3:
        if (D_e48c_b[0x135] != 0x3f)
            changed = 1;
        D_e48c_b[0x135] = 0x3F;
        D_e48c_b[0x136] = 0x23;
        D_e48c_b[0x137] = 9;
        D_e48c_b[0x144] = 0x38;
        D_e48c_b[0x145] = 0x15;
        D_e48c_b[0x146] = 0;
        break;
    case 8:
        v = 0x68;
        if (D_00e2 > 2)
            D_e48c_b[0x135] = 0x3f;
        D_e48c_b[0x136] = 0x23;
        D_e48c_b[0x137] = 9;
        if (D_00e2 > 4)
            D_e48c_b[0x144] = 0x38;
        D_e48c_b[0x145] = 0x15;
        D_e48c_b[0x146] = 0;
        if (D_00e2 > 6)
            v = 6;
        goto done;
    case 9:
        if (D_e48c_b[0x135] != 0x3f)
            changed = 1;
        D_e48c_b[0x135] = 0x3F;
        D_e48c_b[0x136] = 0x23;
        D_e48c_b[0x137] = 9;
        D_e48c_b[0x144] = 0x38;
        D_e48c_b[0x145] = 0x15;
        D_e48c_b[0x146] = 0;
        v = 6;
        goto done;
    case 10:
        v = 6;
        D_e48c_b[0xde] = 0x2E;
        D_e48c_b[0xe0] = 0x37;
        D_e48c_b[0xe7] = 0x37;
        D_e48c_b[0xdf] = 0x34;
        D_e48c_b[0xe8] = 0x34;
        D_e48c_b[0xe9] = 0x3C;
        D_e48c_b[0xea] = 0x32;
        D_e48c_b[0xeb] = 0x30;
        D_e48c_b[0xec] = 0x38;
        if (D_00e2 > 2) {
            if (D_e48c_b[0xed] != 0x2e)
                changed = 1;
            D_e48c_b[0xef] = 0x34;
            D_e48c_b[0xf0] = 0x2A;
            D_e48c_b[0xf1] = 0x28;
            D_e48c_b[0xf2] = 0x31;
            D_e48c_b[0xed] = 0x2E;
            D_e48c_b[0x10b] = 0x2E;
            D_e48c_b[0xee] = 0x2C;
            D_e48c_b[0x10c] = 0x2C;
            D_e48c_b[0x10d] = 0x32;
            D_e48c_b[0x111] = 0x38;
            D_e48c_b[0x112] = 0x33;
            D_e48c_b[0x113] = 0x3F;
        }
        if (D_00e2 > 3) {
            if (D_e48c_b[0x13b] != 0x38)
                changed = 1;
            D_e48c_b[0x142] = 0x3C;
            D_e48c_b[0x144] = 0x3C;
            D_e48c_b[0x14a] = 0x3C;
            D_e48c_b[0x13b] = 0x38;
            D_e48c_b[0x145] = 0x38;
            D_e48c_b[0x14b] = 0x38;
            D_e48c_b[0x1f2] = 0x38;
            D_e48c_b[0x1f3] = 0x38;
            D_e48c_b[0x216] = 0x38;
            D_e48c_b[0x13c] = 0x33;
            D_e48c_b[0x217] = 0x33;
            D_e48c_b[0x141] = 0x3B;
            D_e48c_b[0x22b] = 0x3B;
            D_e48c_b[0x146] = 0x3A;
            D_e48c_b[0x14c] = 0x3A;
            D_e48c_b[0x22c] = 0x3A;
            D_e48c_b[0x13d] = 0x3F;
            D_e48c_b[0x143] = 0x3F;
            D_e48c_b[0x1f4] = 0x3F;
            D_e48c_b[0x218] = 0x3F;
            D_e48c_b[0x22d] = 0x3F;
            D_e48c_b[0x2be] = 0x25;
            D_e48c_b[0x2bf] = 0x2F;
            D_e48c_b[0x2c0] = 0x37;
        }
        if (D_00e2 > 4)
            v = 15;
        goto done;
    }
    v = 0x68;
done:
    D_2860 = v;
    if (old != v || changed)
        sub_root_047c_058b(D_e48c);
}
#pragma optimize("", on)
