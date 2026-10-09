/* place: root 0000:0000 */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* status: hand port (e3 rounds 3-4), every function written; 26/38 match (fmatch counts 38
   because 0000:2652 - sub_root_0265_0002 - belongs to this file, see below).
   Matching: 0bc2..0caa, 1080..1408 group, 14f6, 1588, 2286, 24bc.., 2648.  Open:
   - 0000 (480/468), 1438 (6.00 keeps the constant 2 in AL), 23e6 (slot order bw/bh);
   - 14f6 matches only in some symbol-table states (cmp [si+6],ax and the h + y operand
     order flip when lines above change): the dummy D_ffe0 in front of it selects one;
     re-check it after every edit above it;
   - 1716: only the stack slots differ (c at the top instead of the bottom, m/i swapped);
     18fa: only its jump table, i.e. the length of an earlier function;
   - 06c2/07d4/0984/0dd2/01d4 (classified 'asm' in functions.json, but they are plain MSC
     code: inline strlen with loop, enter/leave) written from the asm, lengths within a
     few bytes; 06c2 frame 8 vs 4 (two slots the old compiler kept), push [si] vs our
     reuse of DX; the if/else colour stores need #pragma optimize("a", on);
   - 1a9c (status column) cannot match: 6.00 saves SI/DI around the inline strcpy/strcat
     (known 6.00 vs 6.00A limit); written for documentation.
   2648/2652: 2648 calls 0000:2652 with push cs / call near, and the other callers reach
   2652 as 0000:2652 (lcall 0, 2652h), so the empty function at 2652 is the LAST function
   of this file, not of root 0265 (text.c, which really starts at 0265:0004).  Defined
   here as sub_root_0265_0002, 2648 matches; build/analysis/srcfiles.json still assigns
   it to the 0265 file (fmatch: 'len 2/0').  root 0000 and root 0265 are NOT one file:
   0000 calls the text functions with far calls.
   Game shell: menu bar handling, hot keys (0caa), "Game Paused" box, options toggles (sound
   effects / music / saintly changes / difficulty), "Quit to DOS? y/n", text boxes, party
   panel (1588 frames, 1716 rows, 18fa status letter, 1a9c bars). */

/* screen/window descriptor (D_0154 = off-screen copy, D_017c = visible screen) */
struct win {
    int f00;
    int w;          /* +0x02 */
    int f04;
    int x1;         /* +0x06 last column */
    int y1;         /* +0x08 last row */
    int f0a;
    int f0c;        /* +0x0c text colour */
    int f0e;        /* +0x0e background colour */
    int f10;        /* +0x10 font */
};
struct rect { int x; int y; int w; int h; };
struct res { int a; int b; };

/* menu bar entry (10 bytes), its items (0x14 bytes) and sub-items (0x16 bytes) */
struct subitem {
    char *name;                 /* +0x00 */
    char enabled;               /* +0x02 */
    char key[4];                /* +0x03 shortcut text */
    unsigned char count;        /* +0x07 quantity shown in front (>= 2) */
    char pad08[0x0a];
    void (far *fn)(void);       /* +0x12 */
};
struct item {
    char *name;                 /* +0x00 */
    char enabled;               /* +0x02 */
    char key[0x0b];             /* +0x03 shortcut text, right column */
    void (far *fn)(void);       /* +0x0e */
    struct subitem *sub;        /* +0x12 */
};
struct menu {
    char *name;                 /* +0x00 */
    int x0;                     /* +0x02 */
    int x1;                     /* +0x04 */
    char has_items;             /* +0x06 */
    char pad07;
    struct item *items;         /* +0x08 */
};

extern char far F_2e38_0000[];
extern int D_000c[];
extern int D_000e[];
extern struct win *D_0154;
extern struct win *D_017c;
extern char D_9cb2[][0x80];
extern char D_9cb3[][0x80];
extern char D_9cb8[][0x80];
extern char D_9cb9[][0x80];
extern char D_9cba[][0x80];
extern char D_9cbf[][0x80];
extern char D_9c9f[][0x80];
extern char D_01bc;
extern char *D_01a8;
extern char *D_01aa;
extern char *D_01ac;
extern char *D_01ae;
extern char *D_01b0;
extern char D_a6c1;
extern char D_a6d3[];
extern char D_9c69[][0x80];
extern char D_9c93[][0x80];
extern int D_9c65[][0x40];
extern char D_907b;
extern int D_6eec;
extern int D_a2d7[];
extern int D_a2e5;
extern int D_a626;
extern char D_a3d8[];
extern int D_a696[];
extern int D_a6b0;
extern char D_a6b2;
extern char D_a6b3;
extern char D_a6b4[];
extern int D_a6c2;
extern unsigned char D_a6c4[];
extern int D_a6d1;
extern struct win *D_0168;
extern struct win *D_0190;
extern int D_01e4[];
extern char D_9064[];
extern int D_a2e9;
extern int D_a33b;
extern int D_a33e;
extern int D_a392;
extern int D_a3d6;
extern int D_a3e5;
extern char *D_01a6;
extern char *D_01b2;
extern char *D_01b4;
extern char *D_01b6;
extern char *D_01b8;
extern char *D_01ba;
extern unsigned char D_01bd;
extern unsigned char D_01be;
extern unsigned char D_01bf;
extern int D_0204;
extern int D_0206;
extern char D_07d3;
extern char D_07f4;
extern unsigned char D_6ef0;
extern char D_906a;
extern char D_907a;
extern char D_907d;
extern int D_9409;
extern int D_940b;
extern int D_940d;
extern int D_940f;
extern int D_9411;
extern int D_9413;
extern int D_9415;
extern int D_944b;
extern struct menu D_9419[4];
extern int D_9783;
extern int D_9789;
extern int D_987b;
extern char D_987f;
extern char D_9893;
extern char D_9920;
extern char D_9934;
extern char *D_995a;
extern char *D_996e;
extern char *D_9982;
extern int D_99e7;
extern int D_99e9;
extern int D_9a45;
extern char *D_99ec;
extern char *D_9a02;
extern char *D_9a18;
extern char D_9c26;
extern int D_9c27;
extern int D_9c29;
extern char D_9c2f;
extern char D_9c3c;
extern int D_9c3f;
extern char D_9c41;
extern int D_9c42;
extern int D_9c15[];
extern int D_9c1e;
extern int D_9c20;
extern unsigned char D_9c22[];
extern char D_a394;
extern int D_6eee;
extern int D_9c44;
extern int D_9c48;
extern int D_9c51;
extern int D_9c53;
extern int D_9c3a;
extern char D_9c4c;
extern int D_a2e3;
extern int D_a2e7;
extern int D_a340;
extern int D_a660;
extern int D_a662;
extern int D_a664;
extern int D_a666;
extern int D_a68f;
extern int D_a691;
extern int D_a693;
extern char D_a695;

int far _fstrcpy(char far *, char far *);
int far _fstricmp(char far *, char far *);
void far exit_(int);
int far getch(void);
int far int86(int, void *, void *);
int far rand(void);
int far remove(char *);
char *strcpy(char *, const char *);
char *strcat(char *, const char *);
unsigned strlen(const char *);
void far copy_rect_saved(struct win *, int, int, int, int, struct win *, int, int);
void far sub_root_0265_006e(struct win *, int, int, char *);
void far sub_root_0265_0134(struct win *, char *, int, int, int, int, int, int, int, int, int, int);
long far sub_root_0265_0654(struct win *, char *, int, int, int, int, int, int, int, int, int);
void far sub_root_0265_09b2(int, char *);
int far sub_root_0265_084e(struct win *, char *);
void far sub_root_0410_0302(int);
void far sub_root_044a_0122(void);
void far sub_root_044a_01f4(void);
void far sub_root_047c_00a9(void);
int far sub_root_047c_051b(int);
void far sub_root_047c_04dc(struct win *, int, int, int, int, int, int);
void far sub_root_047c_0553(struct win *, int, int, int, int, struct win *, int, int);
void far sub_root_047c_05a5(struct win *, int, int, int);
int far sub_root_047c_073d(void);
int far sub_root_047c_0744(void);
void far sub_root_047c_078a(int);
void far sub_root_047c_0791(void);
void far sub_root_047c_0952(struct win *, int, int, int, int);
void far sub_root_047c_0d62(struct win *, int, int, int, int, int);
void far sub_root_047c_123f(void);
void far sub_root_047c_124f(int, int, int);
void far sub_root_047c_1293(void);
void far sub_root_047c_12c3(void);
void far sub_root_05f8_0139(int);
void far sub_root_0617_011e(char *, char far *);
void far sub_root_0617_06d8(char *);
void far sub_root_0617_070e(void);
void far vec_183d(void);
void far vec_1847(void);
void far vec_1851(void);
void far vec_185b(void);
void far vec_1865(void);
void far vec_186f(void);

void far sub_root_0000_0000(struct menu *m, int once);
void far sub_root_0000_01d4(struct menu *m);
void far sub_root_0000_06c2(struct menu *m);
void far sub_root_0000_07d4(struct item *items, int x, int y);
void far sub_root_0000_0984(struct subitem *subs, int x, int y);
void far sub_root_0000_0bc2(void);
void far sub_root_0000_0c2c(void);
int far sub_root_0000_0c70(int n);
int far sub_root_0000_0c8c(int n);
void far sub_root_0000_0caa(void);
void far sub_root_0000_0dd2(void);
void far sub_root_0000_1080(void);
void far sub_root_0000_1154(void);
void far sub_root_0000_11f2(void);
void far sub_root_0000_129a(void);
void far sub_root_0000_1338(void);
void far sub_root_0000_1354(void);
void far sub_root_0000_1384(void);
void far sub_root_0000_13a0(void);
void far sub_root_0000_1408(void);
void far sub_root_0000_1418(void);
void far sub_root_0000_1428(void);
void far sub_root_0000_1438(int show);
int far sub_root_0000_14f6(struct win *src, int x, int y, int w, int h, struct win *dst,
                           int dx, int dy);
void far sub_root_0000_1588(int which);
void far sub_root_0000_1716(int which);
void far sub_root_0000_18fa(int i, struct win *w);
void far sub_root_0000_1a9c(int i);
void far sub_root_0000_2286(struct win *w, int x, int y, int cw, int ch, int bg);
void far sub_root_0000_23e6(struct win *w, int y, int save, char *text, struct win *dst,
                            struct rect *r);
void far sub_root_0000_24bc(char *text, int ticks);
int far sub_root_0000_2522(char *question);
void far sub_root_0000_25f0(void);
void far sub_root_0000_25f2(char *name);
void far sub_root_0000_262e(void);
void far sub_root_0000_2646(void);
void far sub_root_0000_2648(void);
void far sub_root_0265_0002(int);

/* menu bar loop: runs until neither the mouse button (D_6ef0 bit 1) nor menu mode (D_9c26)
   is active; once != 0 only restores the screen colours.  While active it draws (06c2) and
   tracks (01d4) the bar; on release it closes the open dropdown (0bc2) and calls the chosen
   item's far handler (+0x0e) or sub-item handler (+0x12). */
void far sub_root_0000_0000(struct menu *m, int once)
{
    int first;
    int c0c;
    int c10;
    int loop;
    int active;
    int bar;
    int item;
    int sub;
    int sel;
    struct item *it;
    struct subitem *si;

    first = 0;
    c0c = D_017c->f0c;
    c10 = D_017c->f10;
    loop = D_017c->f10 = 1;
    if (once == 0) {
        if (sub_root_047c_073d())
            sub_root_0000_0caa();
        if (D_07d3 == 0) {
            D_07d3 = 1;
            first = 1;
        }
        do {
            if (D_9c26 == 1 && (sub_root_047c_073d() || (D_6ef0 & 2)))
                sub_root_0000_0dd2();
            if ((D_6ef0 & 2) || D_9c26 == 1)
                active = 1;
            else
                active = loop = 0;
            if (active == 1) {
                D_a2e3 = active;
                if (D_9c42 != D_a68f && D_907a == 1 && D_9c4c == 1) {
                    sub_root_047c_12c3();
                    sub_root_047c_124f(0, 0, D_a695 == 0 ? D_a693 : D_a691);
                    D_a68f = D_9c42;
                    sub_root_047c_1293();
                }
                sub_root_0000_06c2(m);
                sub_root_0000_01d4(m);
            } else {
                D_a2e3 = active;
                if (active == 0 && D_9409 == 1) {
                    D_9409 = loop = 0;
                    bar = D_a2e7;
                    item = D_0204;
                    sub = D_0206;
                    sel = D_a340;
                    sub_root_0000_0bc2();
                    if (D_944b == 1) {
                        D_944b = 0;
                        it = &m[bar].items[item];
                        if (it->enabled == 1 && D_9413 > item) {
                            D_017c->f0c = c0c;
                            D_017c->f10 = c10;
                            (*it->fn)();
                        }
                    }
                    if (D_9783 == 1) {
                        D_9783 = 0;
                        si = &m[bar].items[sel].sub[sub];
                        if (si->enabled == 1 && D_9415 > sub) {
                            D_017c->f0c = c0c;
                            D_017c->f10 = c10;
                            (*si->fn)();
                        }
                    }
                }
            }
        } while (loop);
    }
    D_017c->f0c = c0c;
    D_017c->f10 = c10;
    if (--first == 0)
        D_07d3 = 0;
}

/* menu tracking for one frame.  Position: the mouse (D_6eec - 0x3c, D_6eee) or, in keyboard
   menu mode, a position derived from the bar/item/sub-item cursor (D_a394 counts down the
   steps of opening a menu from the keyboard).  On the bar (y < 12) it highlights the entry
   under the cursor and opens its dropdown (07d4); inside the open dropdown it highlights
   the item row (D_0204, D_944b) and opens its sub-menu (0984); inside the sub-menu it
   highlights the sub-item row (D_0206, D_9783); elsewhere it removes the highlights */
void far sub_root_0000_01d4(struct menu *menus)
{
    int j;
    int y;
    int x;
    int row;
    int mx;
    int my;
    int sx;
    int iy;
    int sy;
    struct menu *m;
    struct item *it;

    if (D_9c26 == 0) {
        mx = D_6eec;
        my = D_6eee;
    } else {
        if (D_a394 == 3) {
            D_a394--;
        } else if (D_a394 == 2) {
            D_a394--;
            D_9c3c = 0;
        } else if (D_a394 == 1) {
            D_a394--;
            D_9c41 = 0;
        }
        sx = D_9c41 == -1 ? 0 : D_9c22[D_9c2f];
        mx = sx + D_9c15[D_9c2f] + 0x3c;
        iy = D_9c3c == -1 ? 0 : (D_9c3c << 3) + 0xc;
        sy = D_9c41 == -1 ? 0 : D_9c41 << 3;
        my = iy + sy + 4;
    }
    if (my < 0xc) {
        y = my;
        D_944b = 0;
        x = mx - 0x3c;
        for (j = 0, m = menus; j < D_9411; j++, m++) {
            if (x < m->x0 || m->x1 < x)
                continue;
            if (D_a2e7 != j) {
                sub_root_047c_12c3();
                if (D_a2e7 >= 0 && D_a2e7 < D_9411)
                    sub_root_047c_04dc(D_017c, D_9c1e, 0, D_9c20 - D_9c1e + 1, 0xc, 9, 0);
                if (D_940f == 1) {
                    sub_root_0000_0c2c();
                    D_a340 = -0x63;
                }
                if (D_940d == 1) {
                    copy_rect_saved(D_0154, D_9789, 0xc, D_99e7, (D_9413 << 3) + 3, D_017c,
                                    D_9789, 0xc);
                    D_940d = 0;
                    D_0204 = -1;
                }
                D_a2e7 = j;
                D_9c1e = m->x0;
                D_9c20 = m->x1;
                sub_root_047c_04dc(D_017c, D_9c1e, 0, D_9c20 - D_9c1e + 1, 0xc, 0, 9);
                if (menus[D_a2e7].has_items == 1)
                    sub_root_0000_07d4(menus[D_a2e7].items, menus[D_a2e7].x0, y);
                sub_root_047c_1293();
            } else if (D_0204 >= 0 && D_9413 > D_0204) {
                sub_root_047c_12c3();
                sub_root_047c_04dc(D_017c, D_9789, (D_0204 << 3) + 0xc, D_99e7, 8, 9, 0);
                sub_root_047c_1293();
                D_0204 = -1;
            }
        }
        return;
    }
    y = my;
    if (D_940d == 1 && (x = mx - 0x3c) >= D_9789 && x <= D_987b && my >= 0xc && D_99e9 >= my) {
        D_944b = 1;
        if ((row = (my - 0xc) / 8) != D_0204) {
            sub_root_047c_12c3();
            if (D_0204 >= 0 && D_0204 < D_9413)
                sub_root_047c_04dc(D_017c, D_9789, (D_0204 << 3) + 0xc, D_99e7, 8, 9, 0);
            if (D_940f == 1) {
                sub_root_0000_0c2c();
                D_a340 = -0x63;
            }
            if ((D_0204 = row) >= 0 && D_0204 < D_9413)
                sub_root_047c_04dc(D_017c, D_9789, (D_0204 << 3) + 0xc, D_99e7, 8, 0, 9);
            it = &menus[D_a2e7].items[D_0204];
            if (it->sub != 0 && D_940f == 0 && it->enabled == 1) {
                sub_root_0000_0984(it->sub, D_987b + 1, (D_0204 << 3) + 0xc);
                D_a340 = D_0204;
            }
            sub_root_047c_1293();
            return;
        }
        if (D_0206 != -0x63) {
            sub_root_047c_04dc(D_017c, D_9c29, (D_0206 << 3) + D_9c27 + 1, D_9c3f, 8, 9, 0);
            D_0206 = -0x63;
        }
        return;
    }
    if (D_940f == 1 && (x = mx - 0x3c) >= D_9c29 && x <= D_9c3a && D_9c27 <= my &&
        D_9c44 >= my) {
        D_9783 = 1;
        if ((row = (my - 0xc) / 8) - D_a340 != D_0206) {
            sub_root_047c_12c3();
            if (D_0206 >= 0 && D_0206 < D_9415)
                sub_root_047c_04dc(D_017c, D_9c29, (D_0206 << 3) + D_9c27 + 1, D_9c3f, 8, 9,
                                   0);
            if ((D_0206 = row - D_a340) >= 0 && D_0206 < D_9415)
                sub_root_047c_04dc(D_017c, D_9c29, (D_0206 << 3) + D_9c27 + 1, D_9c3f, 8, 0,
                                   9);
            sub_root_047c_1293();
        }
        return;
    }
    if (D_944b == 1) {
        sub_root_047c_12c3();
        sub_root_047c_04dc(D_017c, D_9789, (D_0204 << 3) + 0xc, D_99e7, 8, 9, 0);
        sub_root_047c_1293();
        D_944b = 0;
        D_0204 = -1;
        return;
    }
    if (D_9783 == 1) {
        sub_root_047c_12c3();
        sub_root_047c_04dc(D_017c, D_9c29, (D_0206 << 3) + D_9c27 + 1, D_9c3f, 8, 9, 0);
        sub_root_047c_1293();
        D_9783 = 0;
        D_0206 = -0x63;
    }
}

/* draw the menu bar: save the bar area to the off-screen page, clear it and print the
   titles left to right (grey when the entry has no items), recording each title's x range */
#pragma optimize("a", on)
void far sub_root_0000_06c2(struct menu *m)
{
    int x;
    int w;

    if (D_9409 == 0) {
        x = 3;
        sub_root_047c_12c3();
        sub_root_047c_0553(D_017c, 0, 0, 0x104, 0xc, D_0154, 0, 0);
        copy_rect_saved(D_017c, 0, 0, 0x104, 0xc, D_0154, 0, 0);
        sub_root_0000_2286(D_017c, 0, 0, 0x104, 0xc, 0x80);
        D_9411 = 0;
        for (; strlen(m->name) != 0; m++) {
            w = sub_root_0265_084e(D_017c, m->name);
            m->x0 = x;
            if (m->has_items == 1)
                D_017c->f0c = 0;
            else
                D_017c->f0c = 7;
            sub_root_0265_006e(D_017c, x + 4, 3, m->name);
            x += w + 8;
            m->x1 = x - 1;
            D_9411++;
        }
        sub_root_047c_1293();
        D_940d = 0;
        D_940f = 0;
        D_9409 = 1;
        D_a2e7 = -1;
    }
}
#pragma optimize("", on)

/* open the dropdown for items at x: size it to the widest name and shortcut column, save
   the area under it, draw the frame and the items (grey when disabled) and highlight the
   item under y */
void far sub_root_0000_07d4(struct item *items, int x, int y)
{
    int j;
    int wname;
    int wkey;
    int w;
    struct item *it;
    int yy;

    D_9413 = 0;
    wname = 0;
    wkey = 0;
    j = 0;
    for (it = items; strlen(it->name) != 0; it++) {
        if ((w = sub_root_0265_084e(D_017c, it->name)) > wname)
            wname = w;
        if ((w = sub_root_0265_084e(D_017c, it->key)) > wkey)
            wkey = w;
        D_9413++;
    }
    D_9789 = x;
    D_987b = x + wkey + wname + 9;
    D_9a45 = D_9789 + wname + 6;
    D_99e7 = D_987b - D_9789 + 1;
    D_99e9 = (D_9413 << 3) + 0xe;
    copy_rect_saved(D_017c, D_9789, 0xc, D_99e7, (D_9413 << 3) + 3, D_0154, D_9789, 0xc);
    sub_root_0000_2286(D_017c, D_9789, 0xb, D_99e7, (D_9413 << 3) + 4, 0x80);
    for (j = 0, it = items, yy = 0xe; j < D_9413; j++, it++, yy += 8) {
        if (it->enabled == 1)
            D_017c->f0c = 0;
        else
            D_017c->f0c = 7;
        sub_root_0265_006e(D_017c, x + 3, yy, it->name);
        sub_root_0265_006e(D_017c, D_9a45, yy, it->key);
    }
    if ((D_0204 = (y - 0xe) / 8) >= 0 && D_0204 < D_9413)
        sub_root_047c_04dc(D_017c, D_9789, (D_0204 << 3) + 0xc, D_99e7, 8, 0, 9);
    D_940d = 1;
}

/* open the sub-menu for subs at (x, y): like 07d4 with a third, centred column for item
   counts of 2 and more */
void far sub_root_0000_0984(struct subitem *subs, int x, int y)
{
    char num[16];
    int wkey;
    int wnum;
    int wname;
    int j;
    unsigned char *pc;
    int w;
    struct subitem *s;
    int yy;

    D_9415 = 0;
    wnum = 0;
    wname = 0;
    wkey = 0;
    j = 0;
    for (s = subs; strlen(s->name) != 0; s++) {
        if (s->count >= 2) {
            sub_root_0265_09b2(s->count, num);
            if ((w = sub_root_0265_084e(D_017c, num)) > wnum)
                wnum = w;
        }
        if ((w = sub_root_0265_084e(D_017c, s->name)) > wname)
            wname = w;
        if ((w = sub_root_0265_084e(D_017c, s->key)) > wkey)
            wkey = w;
        D_9415++;
    }
    D_9c51 = wnum ? wnum + 3 : 0;
    D_9c29 = x - 1;
    D_9c3a = D_9c29 + wkey + D_9c51 + wname + 9;
    D_9c53 = D_9c29 + D_9c51 + 3;
    D_9c48 = D_9c29 + D_9c51 + wname + 6;
    D_9c3f = D_9c3a - D_9c29 + 1;
    D_9c27 = y - 1;
    D_9c44 = D_9c27 + (D_9415 << 3) - 1;
    copy_rect_saved(D_017c, D_9c29, D_9c27, D_9c3f, (D_9415 << 3) + 4, D_0154, D_9c29 + 1,
                    D_9c27 + 1);
    sub_root_0000_2286(D_017c, D_9c29, D_9c27, D_9c3f, (D_9415 << 3) + 4, 0x80);
    for (j = 0, s = subs, yy = 0; j < D_9415; j++, s++, yy += 8) {
        D_017c->f0c = s->enabled == 1 ? 0 : 7;
        if (s->count > 1) {
            sub_root_0265_09b2(s->count, num);
            w = sub_root_0265_084e(D_017c, num);
            sub_root_0265_006e(D_017c, (D_9c51 - w) / 2 + D_9c29 + 3, D_9c27 + yy + 3, num);
        }
        sub_root_0265_006e(D_017c, D_9c53, D_9c27 + yy + 3, s->name);
        sub_root_0265_006e(D_017c, D_9c48, D_9c27 + yy + 3, s->key);
    }
    D_0206 = -0x63;
    D_940f = 1;
}

/* close the open dropdown: restore the item highlight and the menu bar from the off-screen
   copy D_0154 */
void far sub_root_0000_0bc2(void)
{
    sub_root_047c_12c3();
    sub_root_0000_0c2c();
    if (D_940d == 1) {
        copy_rect_saved(D_0154, D_9789, 0xc, D_99e7, (D_9413 << 3) + 3, D_017c, D_9789, 0xc);
        D_0204 = -1;
    }
    copy_rect_saved(D_0154, 0, 0, 0x104, 0xc, D_017c, 0, 0);
    D_a2e7 = -1;
    sub_root_047c_1293();
}

/* close the open sub-menu */
void far sub_root_0000_0c2c(void)
{
    if (D_940f == 1) {
        copy_rect_saved(D_0154, D_9c29 + 1, D_9c27 + 1, D_9c3f, (D_9415 << 3) + 4, D_017c,
                        D_9c29, D_9c27);
        D_0206 = -0x63;
        D_940f = 0;
    }
}

/* random number 0 .. n-1 (n at least 2) */
int far sub_root_0000_0c70(int n)
{
    int v_si;
    v_si = n;
    if (v_si <= 1) {
        v_si = 2;
    }
    return rand() % v_si;
}

/* random number 1 .. n (n at least 2) */
int far sub_root_0000_0c8c(int n)
{
    int v_si;
    v_si = n;
    if (v_si <= 1) {
        v_si = 2;
    }
    return rand() % v_si + 1;
}

/* hot keys (BIOS scan codes): Alt-Q quit, Alt-O/Alt-F/Alt-L/Alt-J/Alt-? optional handlers
   (vec_183d..185b, enabled by D_9893/D_9920/D_9934/D_987f), Alt-P pause, Alt-D cycle
   difficulty, Alt-F sound effects, Alt-C saintly changes, Alt-M music, F10 menu mode */
void far sub_root_0000_0caa(void)
{
    switch (sub_root_047c_073d()) {
    case 0x1000:
        sub_root_047c_0744();
        sub_root_0000_13a0();
        return;
    case 0x1800:
        sub_root_047c_0744();
        if (D_9893 != 1) {
            return;
        }
        vec_183d();
        return;
    case 0x1900:
        sub_root_047c_0744();
        sub_root_0000_1080();
        return;
    case 0x1f00:
        sub_root_047c_0744();
        if (D_9920 != 1) {
            return;
        }
        vec_1847();
        return;
    case 0x2000:
        sub_root_047c_0744();
        if (!D_906a) {
            sub_root_0000_1418();
            return;
        }
        if (D_906a == 1) {
            sub_root_0000_1428();
            return;
        }
        if (D_906a != 2) {
            return;
        }
        sub_root_0000_1408();
        return;
    case 0x2100:
        sub_root_047c_0744();
        sub_root_0000_129a();
        return;
    case 0x2600:
        sub_root_047c_0744();
        if (D_9934 != 1) {
            return;
        }
        vec_1851();
        return;
    case 0x2e00:
        sub_root_047c_0744();
        sub_root_0000_1154();
        return;
    case 0x3200:
        sub_root_047c_0744();
        sub_root_0000_11f2();
        return;
    case 0x4000:
        sub_root_047c_0744();
        if (D_987f != 1) {
            return;
        }
        vec_185b();
        return;
    case 0x4400:
        sub_root_047c_0744();
        D_9c26 = 1;
        D_9c2f = 0;
        D_9c3c = -1;
        D_9c41 = -1;
    default:
        return;
    }
}

/* keyboard menu navigation (menu mode D_9c26): Esc/F10 or a mouse click leave it, Enter
   selects, Alt-Q quits, the cursor keys move between bar entries (D_9c2f, skipping entries
   whose has_items is 2), items (D_9c3c, -1 = none, only enabled ones) and sub-items
   (D_9c41, -1 = none) of the bar D_9419 */
void far sub_root_0000_0dd2(void)
{
    int b;
    struct item *items;
    int n;
    int j;
    struct subitem *subs;

    if (D_6ef0 & 2) {
        if (sub_root_047c_073d())
            sub_root_047c_0744();
        D_9c26 = 0;
        D_944b = 0;
        D_9783 = 0;
        return;
    }
    switch (sub_root_047c_0744()) {
    case 0x011b:
    case 0x4400:
        D_9c26 = 0;
        D_944b = 0;
        D_9783 = 0;
        break;
    case 0x1000:
        sub_root_0000_13a0();
        break;
    case 0x1c0d:
        D_9c26 = 0;
        break;
    case 0x4800:
        if (D_9c41 != -1) {
            if (D_9c41 > 0)
                D_9c41--;
        } else if (D_9c3c != -1) {
            while (D_9c3c != -1) {
                D_9c3c--;
                if (D_9419[D_9c2f].items[D_9c3c].enabled == 1)
                    break;
            }
        }
        break;
    case 0x4b00:
        if (D_9c41 != -1) {
            D_9c41 = -1;
            return;
        }
        if (D_9c2f > 0) {
            D_9c3c = -1;
            D_9c41 = -1;
            while (D_9c2f != 0) {
                D_9c2f--;
                if (D_9419[D_9c2f].has_items == 1)
                    return;
            }
        }
        break;
    case 0x4d00:
        b = D_9c2f;
        if (D_9419[b].items[D_9c3c].sub != 0) {
            D_9c41 = 0;
            return;
        }
        if (D_9c2f < 3) {
            for (j = b; j < 3; j++)
                if (D_9419[j + 1].has_items != 2)
                    break;
            if (j < 3 && D_9419[j + 1].has_items == 1) {
                D_9c2f = j + 1;
                D_9c3c = -1;
                D_9c41 = -1;
                return;
            }
        }
        break;
    case 0x5000:
        if (D_9c41 != -1) {
            subs = D_9419[D_9c2f].items[D_9c3c].sub;
            if (strlen(subs[D_9c41 + 1].name) != 0) {
                D_9c41++;
                return;
            }
            break;
        }
        n = D_9c3c;
        items = D_9419[D_9c2f].items;
        if (strlen(items[n + 1].name) != 0) {
            while (strlen(items[n + 1].name) != 0 && items[n + 1].enabled == 2)
                n++;
            if (strlen(items[n + 1].name) != 0 && items[n + 1].enabled == 1)
                D_9c3c = n + 1;
        }
        break;
    }
}

/* "Game Paused / Press a key to resume" until a key is pressed */
#pragma optimize("a", on)
void far sub_root_0000_1080(void)
{
    struct rect r2;
    struct rect r1;
    int first;

    first = 0;
    if (!D_07d3) {
        D_07d3 = 1;
        first = 1;
    }
    D_017c->f0c = 1;
    D_017c->f0e = 0xff;
    sub_root_047c_078a(2);
    sub_root_047c_12c3();
    sub_root_0000_23e6(D_017c, 0x32, 1, "Game Paused", D_0154, &r1);
    sub_root_0000_23e6(D_017c, -1, 1, "Press a key to resume", D_0154, &r2);
    sub_root_047c_1293();
    getch();
    sub_root_047c_12c3();
    copy_rect_saved(D_0154, r1.x, r1.y, r1.w, r1.h, D_017c, r1.x, r1.y);
    copy_rect_saved(D_0154, r2.x, r2.y, r2.w, r2.h, D_017c, r2.x, r2.y);
    sub_root_047c_1293();
    sub_root_047c_078a(3);
    if (--first == 0)
        D_07d3 = 0;
}
#pragma optimize("", on)

/* toggle "Show saintly changes" (D_01bd), tick its menu entry, report "... ON/OFF" */
void far sub_root_0000_1154(void)
{
    char buf[80];

    strcpy(buf, D_01ba);
    if ((D_01bd ^= 1) == 1) {
        strcat(buf, "ON");
        *D_995a = 1;
    } else {
        strcat(buf, "OFF");
        *D_995a = 2;
    }
    sub_root_0000_24bc(buf, 0x3c);
}

/* toggle music (D_01bf; off also stops it through 047c_078a(1)) */
void far sub_root_0000_11f2(void)
{
    char buf[80];

    strcpy(buf, D_01b8);
    if ((D_01bf ^= 1) == 1) {
        strcat(buf, "ON");
        *D_996e = 1;
    } else {
        strcat(buf, "OFF");
        *D_996e = 2;
        sub_root_047c_078a(1);
    }
    sub_root_0000_24bc(buf, 0x3c);
}

/* toggle sound effects (D_01be) */
void far sub_root_0000_129a(void)
{
    char buf[80];

    strcpy(buf, D_01b6);
    if ((D_01be ^= 1) == 1) {
        strcat(buf, "ON");
        *D_9982 = 1;
    } else {
        strcat(buf, "OFF");
        *D_9982 = 2;
    }
    sub_root_0000_24bc(buf, 0x3c);
}

/* exit to DOS when D_07f4 is set */
void far sub_root_0000_1338(void)
{
    if (D_07f4) {
        sub_root_0000_1354();
        sub_root_044a_0122();
        exit_(0);
    }
}

/* shut down the sound/timer and return to text mode 3 */
void far sub_root_0000_1354(void)
{
    union { struct { char al; char ah; } h; int x[7]; } r;

    sub_root_044a_0122();
    if (D_9c4c == 1) {
        sub_root_047c_123f();
    }
    sub_root_047c_00a9();
    r.h.ah = 0;
    r.h.al = 3;
    int86(0x10, &r, &r);
}

/* flush the key buffer */
void far sub_root_0000_1384(void)
{
    if (sub_root_047c_073d()) {
        do {
            sub_root_047c_0744();
        } while (sub_root_047c_073d());
    }
}

/* "Quit to DOS? y/n": on yes shut everything down, delete the temporary save "dk.tmp"
   and exit */
void far sub_root_0000_13a0(void)
{
    char *z[2];

    if (sub_root_0000_2522(D_01b2) == 1) {
        sub_root_044a_01f4();
        sub_root_047c_0791();
        vec_186f();
        sub_root_0000_1354();
        if (D_907d == 1) {
            sub_root_05f8_0139(D_940b);
        }
        vec_1865();
        *(char * volatile *)&z[0] = 0;
        sub_root_0617_011e(D_01a6, (char far *)z[0]);
        remove(D_01a6);
        exit_(0);
    }
}

/* difficulty setters (Alt-D cycles 1 -> 2 -> 0 -> 1) */
void far sub_root_0000_1408(void)
{
    D_906a = 0;
    sub_root_0000_1438(1);
}

void far sub_root_0000_1418(void)
{
    D_906a = 1;
    sub_root_0000_1438(1);
}

void far sub_root_0000_1428(void)
{
    D_906a = 2;
    sub_root_0000_1438(1);
}

/* (6.00 keeps the 2 in AL for the second and third store - known 6.00A limit)
   tick the difficulty menu entry (D_99ec Basic, D_9a02 Standard, D_9a18 Expert) and, when
   show == 1, report "Difficulty is now set to ..." */
void far sub_root_0000_1438(int show)
{
    char buf[80];

    strcpy(buf, D_01b4);
    switch (D_906a & 0x7f) {
    case 0:
        *D_99ec = 1;
        *D_9a02 = 2;
        *D_9a18 = 2;
        strcat(buf, "Basic");
        break;
    case 1:
        *D_9a02 = 1;
        *D_99ec = 2;
        *D_9a18 = 2;
        strcat(buf, "Standard");
        break;
    case 2:
        *D_99ec = 2;
        *D_9a02 = 2;
        *D_9a18 = 1;
        strcat(buf, "Expert");
        break;
    }
    if (show == 1)
        sub_root_0000_24bc(buf, 0x3c);
}

/* unused extern: symbol-table knob for the operand order in 14f6 */
extern int D_ffe0;

/* clip the rectangle (x, y, w, h) of src against dst (dst->x1 +6, dst->y1 +8; negative
   source coordinates move the target origin to 0) and copy it (047c_0553); 0 when nothing
   is left */
int far sub_root_0000_14f6(struct win *src, int x, int y, int w, int h, struct win *dst,
                           int dx, int dy)
{
    if (x < 0) {
        w += x;
        x = 0;
        dx = 0;
        if (w <= 0)
            return 0;
    }
    if (x + w > dst->x1) {
        w = dst->x1 - x + 1;
        if (w <= 0)
            return 0;
    }
    if (y < 0) {
        h += y;
        y = 0;
        dy = 0;
        if (h <= 0)
            return 0;
    }
    if (y + h > dst->y1) {
        h = dst->y1 - y + 1;
        if (h <= 0)
            return 0;
    }
    sub_root_047c_0553(src, x, y, w, h, dst, dx, dy);
    return 1;
}

/* redraw the five party-member slot frames (rows of 0x28 pixels on the pages D_0168 and
   D_0190) whose bits are set in D_01e4[which] (0x100: all five); D_9064[i] picks the
   normal (0) or highlighted (1) frame pieces */
void far sub_root_0000_1588(int which)
{
    unsigned bits;
    int y;
    int i;

    if (which == 0x100)
        bits = 0x1f;
    else
        bits = D_01e4[which];
    i = 0;
    y = 0;
    do {
        sub_root_047c_12c3();
        if ((bits & 1) == 1) {
            if (D_9064[i] == 0) {
                sub_root_047c_05a5(D_0168, 0, y, D_a2e9);
                sub_root_047c_05a5(D_0168, 0x39, y, D_a2e9);
                sub_root_047c_05a5(D_0168, 3, y, D_a3d6);
                sub_root_047c_05a5(D_0168, 3, y + 0x27, D_a3e5);
                sub_root_047c_05a5(D_0190, 0, y, D_a2e9);
                sub_root_047c_05a5(D_0190, 0x39, y, D_a2e9);
                sub_root_047c_05a5(D_0190, 3, y, D_a3d6);
                sub_root_047c_05a5(D_0190, 3, y + 0x27, D_a3e5);
            } else if (D_9064[i] == 1) {
                sub_root_047c_05a5(D_0168, 0, y, D_a33b);
                sub_root_047c_05a5(D_0168, 0x39, y, D_a33b);
                sub_root_047c_05a5(D_0168, 3, y, D_a33e);
                sub_root_047c_05a5(D_0168, 3, y + 0x27, D_a392);
                sub_root_047c_05a5(D_0190, 0, y, D_a33b);
                sub_root_047c_05a5(D_0190, 0x39, y, D_a33b);
                sub_root_047c_05a5(D_0190, 3, y, D_a33e);
                sub_root_047c_05a5(D_0190, 3, y + 0x27, D_a392);
            }
        }
        sub_root_047c_1293();
        bits >>= 1;
        i++;
    } while ((y += 0x28) < 200);
}

/* redraw the party panel rows selected by D_01e4[which] (0x100: all five): portrait frame,
   name (grey when the member is out of action or someone else is acting, yellow for the
   current leader D_907b), the status letter (1a9c/18fa) and a cross over a dead member's
   portrait; then copy the row to the screen page D_0190 */
#pragma optimize("a", on)
void far sub_root_0000_1716(int which)
{
    char c;
    char name[40];
    int save;
    int *pic;
    int i;
    char *m;
    int y;
    int bits;
    int k;

    if (which == 0x100)
        bits = 0x1f;
    else
        bits = D_01e4[which];
    save = D_0168->f0c;
    for (i = 0, y = 1, m = D_9c69[0], pic = D_a2d7; pic < D_a2d7 + 5;
         bits >>= 1, y += 0x28, m += 0x80, i++, pic++) {
        if ((bits & 1) == 1) {
            sub_root_047c_05a5(D_0168, 3, y, D_a626);
            if ((c = m[0]) != 0) {
                if (c == 3)
                    sub_root_047c_05a5(D_0168, 6, y + 8, D_a2e5);
                else
                    sub_root_047c_05a5(D_0168, 6, y + 8, *pic);
                strcpy(name, m + 0x2a);
                if (D_a6b3 == 1 && D_a6b2 != i || D_a6b4[i] == (char)0x9d ||
                    (c = m[0]) == 3 || c == 5)
                    D_0168->f0c = 7;
                else if (D_907b == i)
                    D_0168->f0c = 0x0e;
                else
                    D_0168->f0c = 0x0f;
                sub_root_0265_006e(D_0168, 4, y + 1, name);
                sub_root_0000_1a9c(i);
                sub_root_0000_18fa(i, D_0168);
            }
            if (m[0] == 3) {
                D_0168->w = 3;
                D_0168->f04 = y;
                D_0168->x1 = 0x35;
                D_0168->y1 = 0x26;
                for (k = 5; k < 0x69; k += 10)
                    sub_root_047c_0d62(D_0168, k, 0, 0, k, 7);
                D_0168->w = 0;
                D_0168->f04 = 0;
                D_0168->x1 = 0x13f;
                D_0168->y1 = 0xc7;
            }
            if (D_6eec < 0x2d)
                sub_root_047c_12c3();
            sub_root_0000_14f6(D_0168, 3, y, 0x36, 0x26, D_0190, 3, y);
            sub_root_047c_1293();
        }
    }
    D_0168->f0c = save;
}
#pragma optimize("", on)

/* status letter of member i in its panel row: '-' idle, 'W'/'F' (walk/fight), 'B', 'V', 'A',
   'P', 'R', 'M', 'T' for action modes, 'U', 'P', 'D', 'O', 'X' for combat orders (mode
   0x400), 'b'/'e' for the special states in D_a3d8 */
void far sub_root_0000_18fa(int i, struct win *w)
{
    char s[2];
    int st;
    int t;

    if (D_907a == 1 && D_9c69[i][0] != 5) {
        if (D_a3d8[i] == 0) {
            if (D_a695 == 1 && D_a6b3 == 1 && D_a6b2 == i && (t = D_a6b0) != -1)
                st = t;
            else
                st = D_9c65[i][0];
            if (st != 0x400) {
                switch (st) {
                case 0:
                    s[0] = '-';
                    break;
                case 1:
                    s[0] = (D_a696[i] & 0x1000) ? 'F' : 'W';
                    break;
                case 6:
                    s[0] = 'B';
                    break;
                case 0x0a:
                    s[0] = 'V';
                    break;
                case 0x12:
                    s[0] = 'A';
                    break;
                case 0x22: s[0] = 'P'; break;
                case 0x40:
                    s[0] = 'R';
                    break;
                case 0x80:
                    s[0] = 'M';
                    break;
                case 0x100:
                    s[0] = 'T';
                    break;
                }
            } else {
                if (D_a695 == 1 && D_a6b3 == 1 && D_a6b2 == i && (t = D_a6d1) != -1)
                    st = t;
                else
                    st = D_a6c4[i];
                switch (st) {
                case 1:
                case 2:
                    s[0] = 'U';
                    break;
                case 3:
                case 6:
                    s[0] = 'P';
                    break;
                case 4:
                case 7:
                    s[0] = 'D';
                    break;
                case 5:
                    s[0] = 'O';
                    break;
                case 8:
                    s[0] = 'X';
                    break;
                }
            }
        } else {
            switch (D_a3d8[i]) {
            case 1:
                s[0] = 'b';
                break;
            case 2:
                s[0] = 'e';
                break;
            }
        }
        s[1] = 0;
        sub_root_047c_05a5(w, 6, i * 0x28 + 0x1f, D_a6c2);
        sub_root_0265_006e(w, 6, i * 0x28 + 0x1f, s);
    }
}

/* status column of party row i: for an unconscious member (state 5) "???? ???? ????";
   otherwise either a status icon text (D_01a8..D_01b0 by the flag bits of D_a696[i]) or
   three vertical bars for endurance, strength and divine favour (current * 16 / maximum,
   "+" above a bar when the current value exceeds the maximum) with the current values
   printed beside them; while the member carries someone (D_9c65 mode bits 0x1be, slot
   +0x36) the carried member's bars are drawn as well */
void far sub_root_0000_1a9c(int i)
{
    long ext;
    int top;
    char *msg;
    int dy;
    int bot;
    char buf[40];
    int j;
    int bars;
    char c;
    int q;
    int len;

    bars = 0;
    if (D_9c69[i][0] == 5) {
        strcpy(buf, "???? ???? ????");
        ext = sub_root_0265_0654(D_0168, buf, 0x1c, 0x16, i * 0x28 + 9,
                                 sub_root_047c_051b(D_0168->f10), 0, 0, 0, 0, 0);
        if ((dy = (i * 0x28 + 0x25 - (int)(ext >> 16)) / 2) < -1)
            dy = -1;
        D_01bc = 1;
        D_0168->f0c = 7;
        sub_root_0265_0134(D_0168, buf, 0x1c, 0x16, i * 0x28 + dy + 9,
                           sub_root_047c_051b(D_0168->f10), 0, 0, 0, 0, 0, 0);
        D_01bc = 0;
        return;
    }
    top = i * 0x28 + 0xc;
    bot = i * 0x28 + 0x1b;
    c = D_9cb2[i][0];
    q = (c << 4) / D_9cb9[i][0];
    if ((len = 16 - q) == 16 && c != 0)
        len = 15;
    if ((D_a696[i] & 8) == 8 && D_a6c1 == 0) {
        if ((D_a696[i] & 2) == 2)
            msg = D_01a8;
        else if ((D_a696[i] & 1) == 1)
            msg = D_01aa;
        else if ((D_a696[i] & 4) == 4)
            msg = D_01ac;
        else if ((D_a696[i] & 0x20) == 0x20)
            msg = D_01ae;
        else if ((D_a696[i] & 0x40) == 0x40)
            msg = D_01b0;
        else
            goto plain;
        ext = sub_root_0265_0654(D_0168, msg, 0x24, 0x13, i * 0x28 + 9,
                                 sub_root_047c_051b(D_0168->f10), 0, 0, 0, 0, 0);
        if ((dy = (i * 0x28 + 0x1f - (int)(ext >> 16)) / 2) < -1)
            dy = -1;
        D_01bc = 1;
        sub_root_0265_0134(D_0168, msg, 0x24, 0x13, i * 0x28 + dy + 9,
                           sub_root_047c_051b(D_0168->f10), 0, 0, 0, 0, 0, 0);
        D_01bc = 0;
    } else {
plain:
        bars = 1;
        if (len < 0)
            len = 0;
        if (D_9cb2[i][0] > D_9cb9[i][0])
            sub_root_0265_006e(D_0168, 0x14, top - 4, "+");
        if (len != 0) {
            sub_root_047c_0d62(D_0168, 0x15, top, 0x15, top + len - 1, 0x0f);
            sub_root_047c_0d62(D_0168, 0x16, top, 0x16, top + len - 1, 0x0f);
        }
        if (len != 16) {
            sub_root_047c_0d62(D_0168, 0x15, top + len, 0x15, bot, 9);
            sub_root_047c_0d62(D_0168, 0x16, top + len, 0x16, bot, 9);
        }
    }
    sub_root_0265_09b2(D_9cb2[i][0], buf);
    strcat(buf, " ");
    D_0168->x1 = 0x38;
    sub_root_0265_006e(D_0168, 0x12, i * 0x28 + 0x1f, buf);

    c = D_9cb3[i][0];
    q = (c << 4) / D_9cba[i][0];
    if ((len = 16 - q) == 16 && c != 0)
        len = 15;
    if (bars == 1) {
        if (len < 0)
            len = 0;
        if (D_9cb3[i][0] > D_9cba[i][0])
            sub_root_0265_006e(D_0168, 0x22, top - 4, "+");
        if (len != 0) {
            sub_root_047c_0d62(D_0168, 0x23, top, 0x23, top + len - 1, 0x0f);
            sub_root_047c_0d62(D_0168, 0x24, top, 0x24, top + len - 1, 0x0f);
        }
        if (len != 16) {
            sub_root_047c_0d62(D_0168, 0x23, top + len, 0x23, bot, 2);
            sub_root_047c_0d62(D_0168, 0x24, top + len, 0x24, bot, 2);
        }
    }
    sub_root_0265_09b2(D_9cb3[i][0], buf);
    strcat(buf, " ");
    sub_root_0265_006e(D_0168, 0x20, i * 0x28 + 0x1f, buf);

    c = D_9cb8[i][0];
    q = (c << 4) / D_9cbf[i][0];
    if ((len = 16 - q) == 16 && c != 0)
        len = 15;
    if (bars == 1) {
        if (len < 0)
            len = 0;
        if (D_9cbf[i][0] < D_9cb8[i][0])
            sub_root_0265_006e(D_0168, 0x2f, top - 4, "+");
        if (len != 0) {
            sub_root_047c_0d62(D_0168, 0x30, top, 0x30, top + len - 1, 0x0f);
            sub_root_047c_0d62(D_0168, 0x31, top, 0x31, top + len - 1, 0x0f);
        }
        if (len != 16) {
            sub_root_047c_0d62(D_0168, 0x30, top + len, 0x30, bot, 0x0e);
            sub_root_047c_0d62(D_0168, 0x31, top + len, 0x31, bot, 0x0e);
        }
    }
    sub_root_0265_09b2(D_9cb8[i][0], buf);
    strcat(buf, " ");
    sub_root_0265_006e(D_0168, 0x2d, i * 0x28 + 0x1f, buf);

    if ((D_9c65[i][0] & 0x1be) && D_a6d3[i] == 0 && (j = D_9c9f[i][0]) != -1) {
        c = D_9cb2[j][0];
        q = (c << 4) / D_9cb9[j][0];
        if ((len = 16 - q) == 16 && c != 0)
            len = 15;
        if (len < 0)
            len = 0;
        if (bars == 1) {
            if (D_9cb2[j][0] > D_9cb9[j][0])
                sub_root_0265_006e(D_0168, 0x18, top - 4, "+");
            if (len != 0)
                sub_root_047c_0d62(D_0168, 0x19, top, 0x19, top + len - 1, 0x0f);
            if (len != 16)
                sub_root_047c_0d62(D_0168, 0x19, top + len, 0x19, bot, 9);
        }
        c = D_9cb3[j][0];
        q = (c << 4) / D_9cba[j][0];
        if ((len = 16 - q) == 16 && c != 0)
            len = 15;
        if (len < 0)
            len = 0;
        if (bars == 1) {
            if (D_9cb3[j][0] > D_9cba[j][0])
                sub_root_0265_006e(D_0168, 0x26, top - 4, "+");
            if (len != 0)
                sub_root_047c_0d62(D_0168, 0x27, top, 0x27, top + len - 1, 0x0f);
            if (len != 16)
                sub_root_047c_0d62(D_0168, 0x27, top + len, 0x27, bot, 2);
        }
        c = D_9cb8[j][0];
        q = (c << 4) / D_9cbf[j][0];
        if ((len = 16 - q) == 16 && c != 0)
            len = 15;
        if (len < 0)
            len = 0;
        if (bars == 1) {
            if (D_9cbf[j][0] < D_9cb8[j][0])
                sub_root_0265_006e(D_0168, 0x33, top - 4, "+");
            if (len != 0)
                sub_root_047c_0d62(D_0168, 0x34, top, 0x34, top + len - 1, 0x0f);
            if (len != 16)
                sub_root_047c_0d62(D_0168, 0x34, top + len, 0x34, bot, 0x0e);
        }
    }
    D_0168->x1 = 0x13f;
}

/* framed box at x,y with a cw x ch interior: dark (0x86) outer and light (0x82) inner
   bevel lines, corner pieces D_a660..D_a666 */
void far sub_root_0000_2286(struct win *w, int x, int y, int cw, int ch, int bg)
{
    int x1;
    int y1;
    int x2;
    int y2;

    sub_root_047c_0952(w, x, y, cw, ch, bg);
    x1 = x + cw - 1;
    y1 = y + ch - 1;
    sub_root_047c_0d62(w, x, y, x1, y, 0x86);
    sub_root_047c_0d62(w, x1, y, x1, y1, 0x86);
    sub_root_047c_0d62(w, x1, y1, x, y1, 0x86);
    sub_root_047c_0d62(w, x, y1, x, y, 0x86);
    x2 = x1 - 1;
    y2 = y1 - 1;
    sub_root_047c_0d62(w, x + 1, y + 1, x2, y + 1, 0x82);
    sub_root_047c_0d62(w, x2, y + 1, x2, y2, 0x82);
    sub_root_047c_0d62(w, x2, y2, x + 1, y2, 0x82);
    sub_root_047c_0d62(w, x + 1, y2, x + 1, y + 1, 0x82);
    sub_root_047c_05a5(w, x, y, D_a660);
    sub_root_047c_05a5(w, x, y1 - 3, D_a662);
    sub_root_047c_05a5(w, x1 - 3, y, D_a664);
    sub_root_047c_05a5(w, x1 - 3, y1 - 3, D_a666);
}

/* text box: centre text horizontally (320 wide) at text row y (-1: vertically centred),
   frame it (2286) and print it; with save != 0 first save the background into dst and
   return the saved rectangle in *r */
void far sub_root_0000_23e6(struct win *w, int y, int save, char *text, struct win *dst,
                            struct rect *r)
{
    int ty;
    int tw;
    int bw;
    int bh;
    int bx;
    int by;
    int tx;

    if (y != -1)
        ty = y;
    else
        ty = -(sub_root_047c_051b(w->f10) / 2 - 100);
    tw = sub_root_0265_084e(w, text);
    bw = tw + 10;
    bh = sub_root_047c_051b(w->f10) + 7;
    tx = (0x140 - tw - w->w) / 2;
    bx = tx - 5;
    by = ty - 4;
    if (save) {
        r->y = by;
        r->w = bw;
        r->h = bh;
        r->x = bx;
        copy_rect_saved(w, bx, by, bw, bh, dst, bx, by);
    }
    sub_root_0000_2286(w, bx, by, bw, bh, w->f0e);
    sub_root_0265_006e(w, tx, ty, text);
}

/* message box for `ticks` timer ticks (0410_0302), then restore the screen */
#pragma optimize("a", on)
void far sub_root_0000_24bc(char *text, int ticks)
{
    struct rect r;

    D_017c->f0c = 1;
    D_017c->f0e = 0xff;
    sub_root_047c_12c3();
    sub_root_0000_23e6(D_017c, -1, 1, text, D_0154, &r);
    sub_root_0410_0302(ticks);
    copy_rect_saved(D_0154, r.x, r.y, r.w, r.h, D_017c, r.x, r.y);
    sub_root_047c_1293();
}
#pragma optimize("", on)

/* "<question> y/n" box: waits for Y or N (either case), returns 1 for yes */
#pragma optimize("a", on)
int far sub_root_0000_2522(char *question)
{
    char buf[120];
    struct rect r;
    char c;
    int yes;

    D_017c->f0c = 1;
    D_017c->f0e = 0xff;
    strcpy(buf, question);
    strcat(buf, " y/n");
    sub_root_047c_12c3();
    sub_root_0000_23e6(D_017c, -1, 1, buf, D_0154, &r);
    for (;;) {
        c = getch();
        if (c >= 0x60)
            c -= 0x20;
        if (c == 'Y') {
            yes = 1;
            break;
        }
        if (c == 'N') {
            yes = 0;
            break;
        }
    }
    copy_rect_saved(D_0154, r.x, r.y, r.w, r.h, D_017c, r.x, r.y);
    sub_root_047c_1293();
    return yes;
}
#pragma optimize("", on)

void far sub_root_0000_25f0(void)
{
}

/* select data file `name` (kept in F_2e38_0000) unless it is already the current one */
void far sub_root_0000_25f2(char *name)
{
    if (_fstricmp(name, F_2e38_0000)) {
        sub_root_0617_070e();
        sub_root_0617_06d8(name);
        _fstrcpy(F_2e38_0000, name);
    }
}

/* forget the current data file (F_2e38_0000 = "") */
void far sub_root_0000_262e(void)
{
    sub_root_0617_070e();
    _fstrcpy(F_2e38_0000, "");
}

void far sub_root_0000_2646(void)
{
}

/* calls the empty function at 0000:2652 (below) */
void far sub_root_0000_2648(void)
{
    sub_root_0265_0002(0);
}

/* 0000:2652: empty function called by 2648 */
void far sub_root_0265_0002(int a)
{
}
