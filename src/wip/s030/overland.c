/* place: s030 1462:0000 */
/* flags: /AM /G2 /Gs /Ox /Oa /Zp1 */
/* Overland travel map (overlay s030): the screen where the party travels
   across the map of medieval Germany.  Note /Oa (assume no aliasing): it is
   what makes MSC address DGROUP through ss: while DS may hold a far segment.
   STATUS (e1, 2026-10-09): only the first 3 of 22 functions are written.
     0000 matches.  00a8 matches except calls to functions of this file that are
     not written yet (they compile as far calls instead of push cs/call near).
     02c0 (2.4 KB): code is right but the stack locals are laid out differently
     (e.g. original [bp-6]=0x1e counter, [bp-46]/[bp-48] saved 2866/2868, buffer at
     [bp-3a]); needs the locals' first-use order reworked.
     0c44 .. 2dd6 (19 functions, about 10 KB): still to be written from the asm
     (build/agent/gen_engine/s030/s030_1462.c has only stubs for them). */

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
};

/* far data in segment 2e38 */
struct far2e38 {
    char pad0[0x10be];
    struct location far *loc[414];  /* 2e38:10be  all locations; 0..91 are cities */
};
extern struct far2e38 far F_2e38_0000;

extern int D_907e;                  /* current location index, -1 = wilderness */
extern struct city far *D_9785;     /* current city record */
extern int D_e3d0, D_e3d2;          /* party map x, y */
extern int D_e48a;                  /* next card/menu id */

extern int D_00e4;
extern int D_2860;
extern int D_2866, D_2868;          /* party marker screen offset */
extern int D_286a, D_286c, D_286e;  /* map view: left column, party row in view, top row */
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

void far map_read_header(struct _iobuf *);
void far map_load(struct _iobuf *);
void far sub_s030_1462_1f30(void);
void far sub_s030_1462_20d2(int, int);
int far overland_loop(void);
#define BIOS_KBFLAGS ((unsigned char far *)0x00400017L)
int far overland_run(int, int);

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
unsigned _dos_open(const char *, unsigned, int *);
unsigned _dos_read(int, void far *, unsigned, unsigned *);

void far vec_1fdb(int);
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

/* overlay entry: place the party, then run the overland map */
int far overland_main(void)
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
    return overland_run(x, y);
}

/* open DARKLAND.MAP, centre the view on (x, y), run the map loop */
int far overland_run(int x, int y)
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
    map_read_header(D_e3f4);

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

    map_load(D_e3f4);
    sub_root_047c_0568();
    sub_s030_1462_1f30();
    sub_root_047c_124f(5, 0, D_9c42);
    sub_s030_1462_20d2(D_286a, D_286c);
    sub_root_047c_056f();
    result = overland_loop();
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
int far overland_loop(void)
{
    register int key;
    int fd;
    struct _iobuf *fp;
    int delay;
    long pos;
    int sel;
    unsigned nread;
    int col, row;
    char buf[40];
    int unused[5];
    int mx, my;

    delay = 30;
    D_907e = -1;
    mx = D_2866;
    my = D_2868;
    for (;;) {
        if (D_6ef0 & 1) {
            if (D_6eec > 0x3c) {
                my = D_6eee - 0x15;
                mx = D_6eec - 0x41;
            } else {
                /* click on a party portrait */
                mx = 0;
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
            row = (D_2868 + 11) / 4;
            if ((row + D_286e + D_286c - 1) % 2)
                col = (D_2866 + 9) / 16;
            else
                col = (D_2866 + 17) / 16;
            row += D_286e + D_286c;
            col += D_286a;
            sprintf(buf, "character X:%02d Y:%02d", col, row);
            sub_root_047c_055a(D_27f4, 0x58, 0x1f, buf);
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
        if (mx == 0)
            key = 0x4c35;
        else if (D_2866 > mx && D_2868 > my)
            key = 0x4700;
        else if (D_2866 > mx && D_2868 < my)
            key = 0x4f00;
        else if (D_2866 < mx && D_2868 > my)
            key = 0x4900;
        else if (D_2866 < mx && D_2868 < my)
            key = 0x5100;
        else if (D_2866 > mx)
            key = 0x4b00;
        else if (D_2866 < mx)
            key = 0x4d00;
        else if (D_2868 > my)
            key = 0x4800;
        else if (D_2868 < my)
            key = 0x5000;
        else {
            key = 0x4c35;
            mx = 0;
        }

        /* keyboard / joystick override the mouse */
        if (sub_root_047c_073d() || D_0880 || D_0881 || D_0882) {
            mx = 0;
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
                _dos_open("darkland.cty", 0, &fd);
                fp = fdopen(fd, "r");
                pos = (long)sel * 0x26e;
                fseek(fp, pos + 1, 0);
                _dos_read(fd, D_9785, 0x26e, &nread);
                fclose(fp);
                fp = fopen("darkland.dsc", "r");
                fgetc(fp);
                fseek(fp, (long)(sel * 0x50 + 1), 0);
                fread(D_978b, 0x50, 1, fp);
                fclose(fp);
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
                    case 1:
                        return 0x41;
                    case 2:
                        return 0xa8;
                    case 3:
                        return sub_s029_0e76_35aa(7, 1) ? 0xa9 : 0xa8;
                    case 6: case 9:
                        return sub_s029_0e76_35aa(7, 2) ? 0x41 : 0xa9;
                    default:
                        if (sel % 3 == 0 && !sub_s029_0e76_35aa(7, 2))
                            return 0xa9;
                        return 0x41;
                    }
                } else if (!sub_s029_1367_072a() && vec_1c1b() > 4 && sub_s029_0e76_35aa(1, 2)) {
                    return 0x126;
                } else {
                    switch (D_d843) {
                    case 4:
                        switch (sub_root_0000_0c70(9)) {
                        case 0: case 1: return 0x112;
                        case 2: case 3: case 4:
                            if (D_00e4 < 3 || D_00e4 > 9) return 0x117;
                            break;
                        case 5: case 6: return 0x116;
                        default: return 0x11b;
                        }
                        break;
                    case 5:
                        switch (sub_root_0000_0c70(0x11)) {
                        case 0: case 1: return 0x112;
                        case 2: case 3: case 4:
                            if (D_00e4 < 3 || D_00e4 > 9) return 0x117;
                            break;
                        case 5: case 6: return 0x116;
                        case 7: case 8: return 0xa0;
                        case 9: case 10: return 0x102;
                        case 11: case 12: return 0x103;
                        case 13: return 0x111;
                        default: return 0x122;
                        }
                        break;
                    case 6: case 7:
                        switch (sub_root_0000_0c70(0x11)) {
                        case 0: case 1: return 0x112;
                        case 2: case 3: return 0xa5;
                        case 4: case 5: return 0x107;
                        case 6: case 7: return 0xa0;
                        case 8: case 9: return 0x102;
                        case 10: case 11: return 0x103;
                        case 12: return 0x111;
                        case 13: return 0x118;
                        default: return 0x122;
                        }
                        break;
                    case 8: case 9:
                        switch (sub_root_0000_0c70(0x1a)) {
                        case 0: case 1: return 0xa0;
                        case 2: case 3: return 0x102;
                        case 4: case 5: return 0x103;
                        case 6: case 7: case 8: return 0x106;
                        case 9: case 10: return 0x107;
                        case 11: case 12: case 13: return 0x7f;
                        case 14: case 15: case 16: case 17: return 0x104;
                        case 18: case 19: return 0x105;
                        case 20: case 21: case 22: return 0x108;
                        case 23: case 24:
                            if (D_00e4 < 3 || D_00e4 > 9) return 0x117;
                            break;
                        default: return 0x24;
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
                mx = 0;
            }
        }
        sub_s030_1767_0046((long)delay);
    }
}
