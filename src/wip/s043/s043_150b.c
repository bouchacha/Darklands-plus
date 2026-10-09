/* place: s043 150b:000a */
/* orig: size=2d9e CONST=7ee6+0016 _DATA=51f4+0110 */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* status: WIP (e2, 2026-10-09 session 4): 24/28 functions match (fmatch), _DATA matches.
 *   match: 000a 0110 01e0 020e 03b0 049a 04c4 06fe 10fc 1142 13a0 1374 13f2 168c 16e8 1f3e
 *          1f40 2252 2802 2826 28e6 2936 2a04 1744.
 *   04c4, 13f2: contain the original inline _asm recolour block (allowed for these two).
 *   Several functions were compiled without global optimisation (probably the 1992 compiler
 *   ran out of memory): 2252 `#pragma optimize("egl", off)`, 13a0 "eglc" off, 203a "eglc".
 *   0886: 2174/2166. (a) case 51's inner switch reloads `les bx,[9785]` for cases 1/2 (the
 *         original reuses es:bx from the f58/f5a ternary); (b) cross-jump direction: the
 *         original keeps the `mov dx,[9787]; add ax,6eh` tail in case 51's names[0] arm and
 *         case 52 jumps into it, 6.00A keeps case 52's copy (line layout/order tried).
 *         egl/pragma sweep: no gain.
 *   1744: matched (session 4): `*(volatile char *)&D_e96f` in the Shift-hover compare (loads
 *         D_5202 first), a `register struct mouse *m` set after the loop guard of the click
 *         loop (do/while), and `extern int D_e972;` moved before `struct res` (compare
 *         direction `cmp [si+4],ax`).
 *   1d56: the 05a5 call: orig pushes D_a626 before computing `y = who*0x28+1`, and tests
 *         `or ax,ax` after `mov si,ax`; 6.00A hoists the embedded assignment (pragma
 *         combos, volatile, separate statements tried).
 *   203a: eglc off + x as the reused register parameter p8: frame/registers right; left:
 *         ternary/if arm layout (orig falls through to the `yaj` arm, else block of the x
 *         clamp out of line) and the late stores of x1m/y1m embedded in the 0d62 args
 *         (6.00A stores them before the pushes) - both look like 6.00 vs 6.00A.
 *   2d3a: code identical; jump-table words follow once earlier functions have the right length.
 *   X_1: unused extern, its position keeps 06fe's `[bx+si]` operand order.
 * The card/dialog engine.
 *
 * Card text: msgfiles\<card>.msg is loaded into D_a895 (< 6000 bytes) by 020e:
 *   byte n, then n text blocks: byte y, word x, word width, zero-terminated text.
 *   D_e8a5 = n, D_e8a7.y[i] (= D_e90b[i]) = y (clamped 0..200), D_e8a7.x[i] = x (0..320),
 *   D_e8a7.w[i] = width, D_e8a7.ofs[i] = offset of the text in D_a895.  The three int
 *   tables must be ONE struct: the original addresses them relative to one base pointer.
 *   Text may contain $variables: 06fe copies a block to D_9085 replacing each $name by
 *   0886(name); 10fc maps the name to an index in F_290e_21ff[73].
 * Option menu (1142): D_e96f = highlighted line (-1 none), D_e9b0[line] = option number of
 *   that text line (D_ee42 set: line encodes member*4+action -> D_ee6c/D_ee6a, the result
 *   is D_a7ba[D_a779]); Up/Down (0x4800/0x5000, keyboard mode D_ed6c == 1) move the
 *   highlight (1374 rectangles, colour D_ee57 = 9 / D_ee59), Enter/Space choose, 1744
 *   handles the mouse (D_8dde = 1 when a choice was clicked).  168c/16e8 check whether an
 *   able member (status 1) has one of the skills D_ee4b[4] / D_ee43[4] (the script sets
 *   them); they return 1 (enabled) or 2.
 * Text windows: D_017c = card text window, D_0104 = side window, D_0118 = screen;
 *   struct gfx: +0a, +0c colour, +10 font.  2826 waits for Enter/Space/click (returns key).
 * Next card: the script stores it in D_e48a; options run D_ea14[sel]() (or D_eb0a[D_ee6a]
 *   when D_ee6e) and their struct res results are dispatched by the script.
 * 000a/0110 (card name, picture name) show a card: picture pics\<pic>.pic into the
 *   palette D_e48c, D_ee59 = text colour (15 when the picture name starts with 'x',
 *   i.e. dark background), D_ee55 = "picture not yet shown", D_e957 = card name.
 * 03b0 (block, how) shows a text block and waits (2826); 049a (block) shows the option
 *   list block; then the script fills D_e99c[] (option enabled flags) and D_ea14[]
 *   (option handlers returning struct res) and calls 1142 (D_017c, D_0154) which returns
 *   the chosen option.  The script sets D_e48a = next card.
 * 0265_0134 lays text out into D_e96e (n lines, x, width, per-line y[] and height[], plus
 *   per-line option number opt[] and kind[]; struct layout, 0xf6 bytes).
 * $variables (0886): 73 names in F_290e_21ff (PlaceName, He, Money1, LeaderName, ...); see
 *   the comment at 0886 for what each expands to.  Unknown names stay "$name".
 * Mouse (1744): hover highlights lines, Shift+hover shows the chance (1d56 via the option's
 *   D_ea14 handler), kind > 0 lines open a skill-check pop-up box (2252 text, 203a box) with
 *   its own option list (D_ee42).  Global keys (2936): F1..F5 party sheets, F6, Shift-F1..F5
 *   choose the leader.  2a04 lets hours pass with the hourglass (2d3a frames).
 */

extern int D_e972;              /* position decides 1744's compare direction */
struct res { int a; int b; };
struct gfx { char pad[0xa]; int f0a; int color; char pad2[2]; int f10; };
struct finfo { char pad[0x10]; long size; char pad2[4]; };
struct layout {                 /* D_e96e: result of the text layout */
    char n;                     /* lines */
    char pad;
    int w;                      /* e970 */
    int x;                      /* e972 */
    unsigned char y[20];        /* e974 */
    unsigned char h[20];        /* e988 */
    char en[20];                /* e99c */
    char opt[20];               /* e9b0: option number per line */
    int kind[20];               /* e9c4: 1/2 = show a skill-check box (2252), -1 = ... */
    char pad2[0x78];
};                              /* 0xf6 bytes; copies: D_a778 (saved), D_ea64 (pop-up) */

extern int D_000a[];
extern int D_000c[];
extern int D_0010[];
extern int D_00e0;
extern int D_00e2;
extern int D_00e4;
extern struct gfx *D_0104;
extern int D_0118;
extern struct gfx *D_0154;
extern struct gfx *D_017c;
extern char D_01bc;
extern int D_07d4;
extern int D_6eec;
extern int D_6eee;
extern char D_6ef0;
struct mouse { int f0; int click; int x; int y; };
extern struct mouse *D_8ddc;
extern char D_9069;
extern char D_907b;
extern char D_9085[];
extern char D_9419[];
extern int D_9c46;
extern char D_9c4c;
struct member {                 /* party member, 0x80 bytes */
    char status;                /* 1 = able */
    char pad1[2];
    char sex;                   /* 0 male, else female */
    char pad2[0x26];
    char name[0x56];            /* +2a */
};
extern struct member D_9c69[];
extern int D_a61e, D_a620, D_a622, D_a624, D_a626, D_a628, D_a62a, D_a62e, D_a630, D_a632;
extern int D_a67e;
extern int D_a776;
extern char D_a895[];           /* the .msg file */
extern char D_e3d4[];           /* current card name */
extern char D_e3e3[];
extern char D_e48c[];           /* palette */
extern char D_e8a5;             /* number of text blocks */
struct blocks {                 /* D_e8a7: the text blocks of the current card */
    int ofs[25];                /* e8a7: offset of the text in D_a895 */
    int x[25];                  /* e8d9 */
    unsigned char y[26];        /* e90b */
    int w[25];                  /* e925 */
};
extern struct blocks D_e8a7;
extern unsigned char D_e90b[];  /* = D_e8a7.y */
extern int D_e898;
extern char *D_e957;
extern int D_e959;              /* current text block */
extern struct layout D_e96e;
extern char D_e96f;
extern int D_e970;
extern unsigned char D_e974[];
extern struct layout D_ea64, D_eb5a, D_ec50, D_a778;
extern int D_e9c4[];
extern int D_ee0d;
extern unsigned char D_ee10;
extern unsigned char D_ee0f;
extern int D_ee11;
extern int D_ee13;
extern int D_ee2f;
extern int D_ee55;
extern int D_ee59;
extern char D_ee5b[];           /* current picture name */
extern int D_ee72;
extern char D_ee41, D_ee42;
extern int D_ee6e, D_8dde, D_ee6c, D_ee6a, D_ee57;
extern char D_e9b0[];           /* option number per text line (-1 = not an option) */
extern char D_a7ba[];
extern char D_a779;
extern char D_ed6c;
extern char D_906a;
extern int D_ee4b[4];
extern int D_ee43[4];
int far sub_s029_0e76_1260(int, int);
int far sub_s029_0e76_118e(int, int);
extern unsigned char D_e988[];
extern char far * far F_290e_21ff[];   /* $variable names */
struct loc {                    /* map location (far) */
    char pad[4];
    int x, y;                   /* +4, +6 */
    char pad2[0xc];
    unsigned char flags;        /* +14 */
    char pad3[0x11];
    char name[1];               /* +26 */
};
struct city {                   /* D_9785: current city record */
    char pad[0x56];
    int f56;
    int f58, f5a;               /* lord title kind (0 = name strings, 1, 2) */
    char pad2[0x12];
    char names[16][32];         /* +6e: lord name/title, buildings (3 = city square ..) */
};
extern struct loc far * far F_2e38_10be[];
extern char far * far F_290e_2183[];
extern char far * far F_290e_20c3[];
extern char far * far F_290e_2323[];
extern char far * far F_290e_233b[];
extern char far * far F_2e38_0890[];
extern char far * far F_290e_20e3[];
extern char far * far F_290e_2b5b[];
extern char far * far F_290e_1d43[];
extern char far F_2e38_1a96[][0x1e];
extern struct city far *D_9785;
extern int D_907e, D_e89b, D_ed54, D_ed52, D_ee8c, D_ee8e, D_ee90, D_ee92, D_ee94;
extern int D_ed4c, D_ed50, D_ed5c, D_ed5e, D_ed60, D_ed62, D_ed64, D_ed66, D_ed68, D_ed6a;
extern int D_e3d0, D_e3d2;
extern char D_978b[];
extern struct res (far *D_ea14[])();
extern int D_5202;
extern int X_1;                 /* unused: symbol-table position (06fe operand order) */
extern int D_2776[12];          /* days per month */
void far free_buffer(unsigned, char *);
extern char *D_4fce[];           /* quality words (s043_1462) */
extern char D_ee9c[], D_eea2[], D_eea8[], D_eeae[], D_ed46[];
extern char D_ed6d[], D_ed95[], D_edbd[], D_ede5[];
char far * far _fstrcpy(char far *, const char far *);
char far * far _fstrcat(char far *, const char far *);
void far sub_s029_1367_0376(char *, char *);
int far sub_s029_1367_0796(void);
int far sub_s029_0e76_1a8e(int, int);
int far sub_s029_0e76_1a7e(int);
int far vec_1f77(int, int, int);
int far vec_1fa9(int, int);
int far vec_1fb3(int);
int far vec_1fbd(int, int, int, int);
int far vec_20df(void);
int far vec_20fd(void);
int far vec_21a7(void);

char * far itoa();
void far sub_s043_1462_0000(char *, char *);
void far sub_s043_17e5_0008(int, int, int, int, int);
int far sub_root_0000_0000();
int far sub_root_0000_1338();
int far sub_root_0000_1384();
int far sub_root_0000_13a0();
int far sub_root_0000_1588();
int far sub_root_0000_1716();
int far sub_root_0000_25f2();
int far sub_root_0265_0004();
int far sub_root_0265_0134();
long far sub_root_0265_0654();
int far sub_root_0265_09b2();
int far sub_root_044a_0043();
int far sub_root_047c_04d5();
int far sub_root_047c_04dc();
int far sub_root_047c_04f8();
int far sub_root_047c_051b();
int far sub_root_047c_0545();
int far sub_root_047c_0553();
int far sub_root_047c_055f();
int far sub_root_047c_0568();
int far sub_root_047c_056f();
int far sub_root_047c_058b();
int far sub_root_047c_05a5();
int far sub_root_047c_073d();
int far sub_root_047c_0744();
int far sub_root_047c_0952();
int far sub_root_047c_0d62();
int far sub_root_047c_1293();
int far sub_root_047c_12c3();
struct mouse * far sub_root_047c_1320();
int far sub_root_05ba_00c2();
int far sub_root_0617_011e();
int far sub_root_0617_024a();
int far sub_root_0617_0736();
int far sub_s029_1367_0066();
int far sub_s029_1367_0744();
int far sub_s029_1367_0c30();
int far vec_185b();
int far sub_root_047c_1266();
int far sub_root_047c_059e();
int far _fstrcmp(const char far *, const char far *);
int far vec_1fdb();
int far vec_1fef();
void far sub_s043_150b_000a();
void far sub_s043_150b_0110();
void far sub_s043_150b_01e0();
void far sub_s043_150b_020e();
void far sub_s043_150b_03b0();
void far sub_s043_150b_049a();
void far sub_s043_150b_04c4();
void far sub_s043_150b_06fe();
void far sub_s043_150b_0886(char *);
int far sub_s043_150b_10fc();
int far sub_s043_150b_1142();
void far sub_s043_150b_1374();
int far sub_s043_150b_13a0();
void far sub_s043_150b_13f2();
int far sub_s043_150b_168c();
int far sub_s043_150b_16e8();
int far sub_s043_150b_1744();
void far sub_s043_150b_1d56(int, int);
int far sub_s043_150b_1f3e();
void far sub_s043_150b_1f40();
int far sub_s043_150b_203a();
int far sub_s043_150b_2252();
int far sub_s043_150b_2802();
int far sub_s043_150b_2826();
void far sub_s043_150b_28e6();
void far sub_s043_150b_2936(unsigned);
void far sub_s043_150b_2a04(int);
void far sub_s043_150b_2d3a();

int D_5202 = -10;
int D_5204 = 0;

/* show card `name` with picture `pic` (far): logs MEM.LOG, loads pics\<pic>.pic (or clears
 * the screen when pic is empty) and loads msgfiles\<name>.msg (020e). */
void far sub_s043_150b_000a(char *name, char far *pic)
{
    char path[60];

    strcpy(D_ee5b, pic);
    sub_s043_1462_0000(D_e3d4, "a");
    strcpy(path, "pics\\");
    strcat(path, pic);
    strcat(path, ".pic");
    D_ee59 = *pic == 'x' ? 15 : 0;
    if (*pic)
        sub_root_0617_024a(1, path, D_e48c);
    else
        sub_root_047c_0952(D_0118, 0, 0, 0x140, 0xc8, 0xff);
    D_ee55 = 1;
    D_e957 = name;
    sub_root_0000_1588(0x100);
    sub_root_0000_1716(0x100);
    sub_s043_150b_020e(name);
}

/* same as 000a with a near picture name, without the log / D_e957 / sound reset */
void far sub_s043_150b_0110(char *name, char *pic)
{
    char path[60];

    strcpy(D_ee5b, pic);
    strcpy(path, "pics\\");
    strcat(path, pic);
    strcat(path, ".pic");
    if (*pic == 'x') D_ee59 = 15; else D_ee59 = 0;
    if (*pic)
        sub_root_0617_024a(1, path, D_e48c);
    else
        sub_root_047c_0952(D_0118, 0, 0, 0x140, 0xc8, 0xff);
    D_ee55 = 1;
    sub_s043_150b_020e(name);
}

void far sub_s043_150b_01e0(void)
{
    if (D_ee55) {
        D_ee55 = 0;
        sub_root_047c_12c3();
        sub_root_047c_0568();
        sub_root_047c_058b(D_e48c);
        sub_root_047c_056f();
        sub_root_047c_1293();
    }
}

/* load msgfiles\<name>.msg into D_a895 and index its text blocks (see the header comment) */
void far sub_s043_150b_020e(char *name)
{
    char path[60];
    struct finfo st;
    int i, n, k;

    strcpy(path, name);
    strcat(path, ".msg");
    sub_root_0000_25f2("msgfiles");
    sub_root_0617_0736(path, &st);
    if (st.size < 6000) {
        sub_root_0617_011e(path, (char far *)D_a895);
        k = 1;
        n = D_e8a5 = D_a895[0];
        for (i = 0; i < n; i++) {
            D_e90b[i] = D_a895[k];
            k++;
            D_e8a7.x[i] = *(int *)&D_a895[k];
            k += 2;
            D_e8a7.w[i] = *(int *)&D_a895[k];
            k += 2;
            D_e8a7.ofs[i] = k;
            D_e90b[i] = sub_s029_1367_0066(0, 200, D_e90b[i]);
            D_e8a7.x[i] = sub_s029_1367_0066(0, 0x140, D_e8a7.x[i]);
            while (D_a895[k])
                k++;
            k++;
        }
    } else {
        strcpy(D_9085, name);
        strcat(D_9085, "too large - ");
        sub_root_0265_09b2(D_ee2f, path);
        strcat(D_9085, path);
        sub_s029_1367_0c30(D_9085);
    }
}

/* show text block `idx` of the current card in the text window and wait for a key.
 * how: 0/2 = draw the frame on the card picture, other = clear the window first */
void far sub_s043_150b_03b0(register int idx, register int how)
{
    D_07d4 = 0;
    sub_root_0000_1384();
    D_e959 = idx;
    if (how && how != 2) {
        sub_root_047c_12c3();
        sub_root_047c_0952(D_017c, 7, 6, 0xf5, 0xbc, 0xff);
        sub_s043_150b_28e6(D_017c);
    } else {
        sub_root_047c_12c3();
        if (D_ee55) {
            D_ee55 = 0;
            sub_root_047c_0568();
            sub_root_047c_058b(D_e48c);
            sub_s043_150b_28e6(D_017c);
            sub_root_047c_0553(D_0118, 0x43, 6, 0xf5, 0xbc, D_017c, 7, 6);
            sub_root_047c_056f();
        } else {
            sub_s043_150b_28e6(D_017c);
            sub_root_047c_0553(D_0118, 0x43, 6, 0xf5, 0xbc, D_017c, 7, 6);
        }
    }
    sub_root_047c_1293();
    sub_s043_150b_04c4(D_017c, idx, 0, how);
    sub_s043_150b_2826();
    sub_root_0000_1384();
}

/* show text block `idx` (the option list) without waiting; the menu (1142) follows */
void far sub_s043_150b_049a(register int idx)
{
    sub_root_0000_1384();
    D_07d4 = 0;
    D_e959 = idx;
    sub_s043_150b_04c4(D_017c, idx, 1, 2);
}

/* draw text block idx (after $variable expansion, 06fe) into window win.
 * mode 1 = the option list: wait (2826) and, unless D_9069, fade and recolour the screen:
 * the original has an inline _asm block (lodsb/stosb over A000:0000, every pixel 0x20..0x7f
 * becomes colour c), which also turns off the optimiser for this function. */
#pragma optimize("a", on)
void far sub_s043_150b_04c4(struct gfx *win, int idx, int mode, int flag)
{
    int uaj[4];
    int ibf;
    int ubk[3];
    int tbm;
    int cbo;

    D_e898 = mode;
    tbm = sub_root_047c_051b(win->f10);
    sub_s043_150b_06fe(idx, D_9085);
    D_017c->color = D_0154->color = mode == 0 ? flag == 1 ? 0 : D_ee59 : D_ee59;
    sub_root_047c_12c3();
    if (D_ee55 && mode == 1) {
        D_ee55 = 0;
        sub_root_047c_0568();
        sub_root_047c_058b(D_e48c);
        sub_s043_150b_28e6(D_017c);
        sub_root_047c_0553(D_0118, 0x43, 6, 0xf5, 0xbc, D_017c, 7, 6);
        sub_root_0265_0134(D_017c, D_9085, D_e8a7.w[idx] - 10, D_e8a7.x[idx] + 5, D_e90b[idx] + 5, tbm, &D_e96e, 1, 0, 1, 1, 1);
        sub_root_047c_056f();
    } else {
        if (mode == 1) {
            sub_root_047c_058b(D_e48c);
            sub_s043_150b_28e6(D_017c);
            sub_root_047c_0553(D_0118, 0x43, 6, 0xf5, 0xbc, D_017c, 7, 6);
        } else
            sub_root_047c_058b(D_e48c);
        sub_root_0265_0134(D_017c, D_9085, D_e8a7.w[idx] - 10, D_e8a7.x[idx] + 5, D_e90b[idx] + 5, tbm, &D_e96e, 1, 0, 1, 1, 1);
    }
    sub_root_047c_1293();
    if (mode == 1) {
        sub_s043_150b_2826();
        D_e898 = 2;
        if (D_9069 == 0) {
            cbo = 7;
            if (D_e3e3[0] == 'x')
                sub_s043_17e5_0008(7, 100, 0x1f, 0x46, 3);
            else
                sub_s043_17e5_0008(cbo, 100, 0xff, 0x46, 3);
            sub_root_047c_12c3();
            _asm {
                push si
                push di
                push ds
                push es
                mov ax, 0A000h
                mov ds, ax
                mov es, ax
                mov bl, byte ptr cbo
                mov cx, 0FA00h
                sub si, si
                sub di, di
            l1:
                lodsb
                cmp al, 20h
                jb l2
                cmp al, 80h
                jae l2
                mov al, bl
            l2:
                stosb
                loop l1
                pop es
                pop ds
                pop di
                pop si
            }
            for (ibf = 0; ibf < D_e96e.n; ibf++)
                if (D_e3e3[0] == 'x') sub_root_047c_04dc(D_017c, D_e96e.x, D_e96e.y[ibf], D_e96e.w, D_e96e.h[ibf], cbo, 15);
                else sub_root_047c_04dc(D_017c, D_e96e.x, D_e96e.y[ibf], D_e96e.w, D_e96e.h[ibf], cbo, 0);
            sub_root_047c_1293();
        }
    }
}

/* copy text block idx to out, replacing each $name by its value (0886) */
#pragma check_stack(on)
void far sub_s043_150b_06fe(int idx, unsigned char *out)
{
    unsigned char c;
    char invar;
    unsigned o, i;
    unsigned len;
    int vl;
    char var[200];
    char buf[900];

    sub_root_044a_0043(buf, (char far *)(D_a895 + D_e8a7.ofs[idx]));
    out[0] = 0;
    invar = 0;
    i = o = 0;
    len = strlen(buf);
    if (len) {
        do {
            c = buf[i];
            if (c == '$') {
                vl = 0;
                invar = 1;
            } else if (invar == 1 && !(c >= 'a' && c <= 'z') && !(c >= 'A' && c <= 'Z') && !(c >= '0' && c <= '9')) {
                invar = 0;
                var[vl] = 0;
                sub_s043_150b_0886(var);
                out[o] = 0;
                strcat(out, var);
                o += strlen(var);
            }
            if (invar == 1 && c != '$')
                var[vl++] = c;
            else if (invar == 0)
                out[o++] = c;
        } while (++i < len);
    }
    if (invar == 1) {
        var[vl] = 0;
        sub_s043_150b_0886(var);
        out[o] = 0;
        strcat(out, var);
    } else
        out[o] = 0;
}
#pragma check_stack(off)

/* expand $variable `name` in place (name is the buffer the variable name was collected in,
 * 200 bytes).  The index from 10fc selects the value (names: F_290e_21ff):
 *   0 PlaceName   current city name (D_9785) or location name (D_907e >= 0x5c)
 *   1 PlaceDesc   D_978b                       2 Inn .. 38 cityChurch: city building names
 *   3 He/She 4 he/she 19 his/her 20 His/Her 21 him/her 44 himself/herself: by the sex of the
 *     party member D_ee8c (the "chosen one")
 *   5..9 Money1..5 (money records D_ee9c.. formatted by s029 0376), 17/18 Number1/2 (itoa)
 *   10 LeaderName (D_907b), 11..15 ChosenOne..FiveName (D_ee8c..D_ee94), 39..43 NamedOne..Five
 *   (F_2e38_1a96[5][30]), 45..48 Text1..4 (D_ed6d.. 40 bytes each), 49 CurrentBell,
 *   50/51 CityLordName/Title, 52 CityLocation (D_ed52), 54/70 LocName/LocName2 (D_e89b,
 *   D_ed54), 55 ItemName (D_ed5c), 56 NearestCity, 57/58 Direction/Direction2, 59..65
 *   Support..Support6, 66 Government, 67 MonthName, 68 SabbatTime, 69 SabbatLocation,
 *   71 MonasteryLocation, 72 LastCastleLocation.
 *   Unknown names (and 31 docks, 32 poorhouse, 34 whorehouse, 35 warehouse, 53 MapLocation)
 *   are left as "$name". */
void far sub_s043_150b_0886(char *name)
{
    int jf;
    int idxl;
    int kg;
    char tmpb[40];

    idxl = sub_s043_150b_10fc(name);
    switch (idxl) {
    case 0:
        if (D_907e < 0x5c)
            _fstrcpy(name, (char far *)D_9785);
        else
            _fstrcpy(name, F_2e38_10be[D_907e]->name);
        break;
    case 54:
        _fstrcpy(name, F_2e38_10be[D_e89b]->name);
        break;
    case 70:
        _fstrcpy(name, F_2e38_10be[D_ed54]->name);
        break;
    case 1:
        strcpy(name, D_978b);
        break;
    case 16:
        kg = sub_s029_0e76_1a8e(D_907e, 1);
        _fstrcpy(name, F_290e_2183[kg]);
        break;
    case 2:
        _fstrcpy(name, D_9785->names[14]);
        break;
    case 52:
        _fstrcpy(name, D_9785->names[D_ed52]);
        break;
    case 22:
        _fstrcpy(name, D_9785->names[3]);
        break;
    case 24:
        _fstrcpy(name, D_9785->names[9]);
        break;
    case 25:
        _fstrcpy(name, D_9785->names[11]);
        break;
    case 27:
        _fstrcpy(name, D_9785->names[8]);
        break;
    case 29:
        _fstrcpy(name, D_9785->names[12]);
        break;
    case 30:
    case 33:
        _fstrcpy(name, D_9785->names[10]);
        break;
    case 36:
        _fstrcpy(name, D_9785->names[13]);
        break;
    case 37:
        _fstrcpy(name, D_9785->names[6]);
        break;
    case 38:
        _fstrcpy(name, D_9785->names[7]);
        break;
    case 26:
        _fstrcpy(name, D_9785->names[15]);
        break;
    case 3:
        if (D_9c69[D_ee8c].sex == 0)
            strcpy(name, "He");
        else
            strcpy(name, "She");
        break;
    case 4:
        if (D_9c69[D_ee8c].sex == 0)
            strcpy(name, "he");
        else
            strcpy(name, "she");
        break;
    case 20:
        if (D_9c69[D_ee8c].sex == 0)
            strcpy(name, "His");
        else
            strcpy(name, "Her");
        break;
    case 19:
        if (D_9c69[D_ee8c].sex == 0)
            strcpy(name, "his");
        else
            strcpy(name, "her");
        break;
    case 21:
        if (D_9c69[D_ee8c].sex == 0)
            strcpy(name, "him");
        else
            strcpy(name, "her");
        break;
    case 44:
        if (D_9c69[D_ee8c].sex == 0)
            strcpy(name, "himself");
        else
            strcpy(name, "herself");
        break;
    case 5:
        sub_s029_1367_0376(D_ee9c, name);
        break;
    case 6:
        sub_s029_1367_0376(D_eea2, name);
        break;
    case 7:
        sub_s029_1367_0376(D_eea8, name);
        break;
    case 8:
        sub_s029_1367_0376(D_eeae, name);
        break;
    case 9:
        sub_s029_1367_0376(D_ed46, name);
        break;
    case 10:
        strcpy(name, D_9c69[D_907b].name);
        break;
    case 11:
        strcpy(name, D_9c69[D_ee8c].name);
        break;
    case 12:
        strcpy(name, D_9c69[D_ee8e].name);
        break;
    case 13:
        strcpy(name, D_9c69[D_ee90].name);
        break;
    case 14:
        strcpy(name, D_9c69[D_ee92].name);
        break;
    case 15:
        strcpy(name, D_9c69[D_ee94].name);
        break;
    case 17:
        itoa(D_ed4c, name, 10);
        break;
    case 18:
        itoa(D_ed50, name, 10);
        break;
    case 39:
    case 40:
    case 41:
    case 42:
    case 43:
        _fstrcpy(name, F_2e38_1a96[idxl - 39]);
        break;
    case 45:
        strcpy(name, D_ed6d);
        break;
    case 46:
        strcpy(name, D_ed95);
        break;
    case 47:
        strcpy(name, D_edbd);
        break;
    case 48:
        strcpy(name, D_ede5);
        break;
    case 49:
        _fstrcpy(name, F_290e_20c3[sub_s029_1367_0796()]);
        break;
    case 51:
        kg = !(F_2e38_10be[D_907e]->flags & 0x80) ? D_9785->f58 : D_9785->f5a;
        switch (kg) {
        case 0:
            if (!(F_2e38_10be[D_907e]->flags & 0x80))
                _fstrcpy(name, D_9785->names[0]);
            else
                _fstrcpy(name, D_9785->names[1]);
            break;
        case 1:
            _fstrcpy(name, F_290e_2323[D_9785->f56 % 6]);
            break;
        case 2:
            _fstrcpy(name, F_290e_233b[D_9785->f56 % 9]);
            break;
        }
        break;
    case 50:
        _fstrcpy(name, D_9785->names[0]);
        break;
    case 55:
        _fstrcpy(name, F_2e38_0890[D_ed5c]);
        break;
    case 56:
        if (D_907e != -1)
            _fstrcpy(name, F_2e38_10be[vec_1fa9(F_2e38_10be[D_e89b]->x, F_2e38_10be[D_e89b]->y)]->name);
        else
            _fstrcpy(name, F_2e38_10be[vec_1fa9(D_e3d0, D_e3d2)]->name);
        break;
    case 57:
        _fstrcpy(name, F_290e_20e3[vec_1fb3(D_e89b)]);
        break;
    case 58:
        kg = vec_1fbd(F_2e38_10be[D_ed54]->x, F_2e38_10be[D_ed54]->y, F_2e38_10be[D_e89b]->x, F_2e38_10be[D_e89b]->y);
        _fstrcpy(name, F_290e_20e3[kg]);
        break;
    case 59:
        _fstrcpy(name, F_290e_2b5b[D_ed5e]);
        break;
    case 60:
        _fstrcpy(name, F_290e_2b5b[D_ed60]);
        break;
    case 61:
        _fstrcpy(name, F_290e_2b5b[D_ed62]);
        break;
    case 62:
        _fstrcpy(name, F_290e_2b5b[D_ed64]);
        break;
    case 63:
        _fstrcpy(name, F_290e_2b5b[D_ed66]);
        break;
    case 64:
        _fstrcpy(name, F_290e_2b5b[D_ed68]);
        break;
    case 65:
        _fstrcpy(name, F_290e_2b5b[D_ed6a]);
        break;
    case 66:
        if (sub_s029_0e76_1a7e(0x18))
            _fstrcpy(name, D_9785->names[4]);
        else
            _fstrcpy(name, D_9785->names[5]);
        break;
    case 23:
        _fstrcpy(name, D_9785->names[4]);
        break;
    case 28:
        _fstrcpy(name, D_9785->names[5]);
        break;
    case 67:
        _fstrcpy(name, F_290e_1d43[D_00e4]);
        break;
    case 68:
        _fstrcpy(name, F_290e_1d43[vec_20df()]);
        itoa(kg = vec_21a7(), tmpb, 10);
        strcat(name, " ");
        strcat(name, tmpb);
        break;
    case 69:
        kg = vec_20fd();
        jf = vec_1fa9(F_2e38_10be[kg]->x, F_2e38_10be[kg]->y);
        _fstrcpy(name, F_290e_20e3[vec_1fbd(F_2e38_10be[jf]->x, F_2e38_10be[jf]->y, F_2e38_10be[kg]->x, F_2e38_10be[kg]->y)]);
        strcat(name, " of ");
        _fstrcat(name, F_2e38_10be[vec_1fa9(F_2e38_10be[kg]->x, F_2e38_10be[kg]->y)]->name);
        break;
    case 71:
        kg = vec_1f77(D_e3d0, D_e3d2, 0x16);
        jf = vec_1fa9(F_2e38_10be[kg]->x, F_2e38_10be[kg]->y);
        _fstrcpy(name, F_290e_20e3[vec_1fbd(F_2e38_10be[jf]->x, F_2e38_10be[jf]->y, F_2e38_10be[kg]->x, F_2e38_10be[kg]->y)]);
        strcat(name, " of ");
        _fstrcat(name, F_2e38_10be[jf]->name);
        break;
    case 72:
        kg = vec_1f77(D_e3d0, D_e3d2, 0x17);
        jf = vec_1fa9(F_2e38_10be[kg]->x, F_2e38_10be[kg]->y);
        _fstrcpy(name, F_290e_20e3[vec_1fbd(F_2e38_10be[jf]->x, F_2e38_10be[jf]->y, F_2e38_10be[kg]->x, F_2e38_10be[kg]->y)]);
        strcat(name, " of ");
        _fstrcat(name, F_2e38_10be[jf]->name);
        break;
    default:
        strcpy(tmpb, "$");
        strcat(tmpb, name);
        strcpy(name, tmpb);
        break;
    }
}

/* index of $variable `name` in the variable-name table F_290e_21ff[73], or -1 */
int far sub_s043_150b_10fc(char *name)
{
    int r;
    int i;

    r = -1;
    for (i = 0; i < 73; i++)
        if (_fstrcmp(name, F_290e_21ff[i]) == 0) {
            r = i;
            break;
        }
    return r;
}

/* the option menu: highlight with the cursor keys (D_e96f = highlighted line of the
 * option text, D_e9b0[line] = option number of that line), Enter/Space chooses; mouse
 * handling is in 1744.  Returns the option index (the script calls D_ea14[index]). */
int far sub_s043_150b_1142(struct gfx *win, struct gfx *win2)
{
    register int key;
    register int sel;

    D_07d4 = 0;
    if (D_9c4c)
        sub_root_047c_1320(6, 0, 0, 0);
    sub_root_0000_1384();
    D_ee41 = D_ee42 = 0;
    D_ee6e = D_8dde = 0;
    D_e96f = -1;
    for (;;) {
        sub_root_0000_0000(D_9419, 0);
        if (D_9c46) {
            D_9c46 = 0;
            sub_root_0000_1588(0x100);
            sub_root_0000_1716(0x100);
            sub_s043_150b_13f2();
        }
        if (D_6ef0 & 1)
            sub_s043_150b_13a0();
        sel = sub_s043_150b_1744(win, win2);
        if (D_8dde == 1)
            break;
        if (!sub_root_047c_073d())
            continue;
        key = sub_root_047c_0744();
        if ((key == 0x1c0d || key == 0x3920) && D_e96f != -1) {
            if (D_ee42) {
                D_ee6c = D_e9b0[D_e96f] / (char)4;
                D_ee6a = D_e9b0[D_e96f] % (char)4;
                sel = D_a7ba[D_a779];
            } else
                sel = D_e9b0[D_e96f];
            if (sel >= 0)
                break;
        }
        if (key == 0x1000)
            sub_root_0000_13a0();
        sub_s043_150b_2936(key);
        if (key == 0x4800 && D_ed6c == 1) {
            sub_s043_150b_1374(win, D_e972, D_e974[key], D_e970, D_e988[key = D_e96f < 0 ? 0 : D_e96f], D_ee57 = 9, D_ee59);
            key--;
            if (D_e96f == 0)
                key = D_e96f = D_e96e.n - 1;
            else
                D_e96f--;
        } else if (key == 0x5000 && D_ed6c == 1) {
            sub_s043_150b_1374(win, D_e972, D_e974[key], D_e970, D_e988[key = D_e96f < 0 ? 0 : D_e96f], D_ee57 = 9, D_ee59);
            key++;
            if (++D_e96f >= D_e96e.n) {
                D_e96f = 0;
                key = 0;
            }
        } else
            goto next;
        sub_s043_150b_1374(win, D_e972, D_e974[key], D_e970, D_e988[key], D_ee59, D_ee57);
        sub_root_047c_12c3();
        sub_root_047c_1266(D_6eec, D_e974[key] + 2);
        sub_root_047c_1293();
    next:
        sub_root_0000_1338();
    }
    sub_root_047c_059e(0);
    return sel;
}

/* draw a (highlight) rectangle with the mouse cursor hidden */
void far sub_s043_150b_1374(struct gfx *win, int x, int y, int w, int h, int c1, int c2)
{
    sub_root_047c_12c3();
    sub_root_047c_04dc(win, x, y, w, h, c1, c2);
    sub_root_047c_1293();
}

/* mouse click in the party portrait column: show that member's sheet (vec_1fdb) */
#pragma optimize("eglc", off)
int far sub_s043_150b_13a0(void)
{
    register int m;

    if (D_6eec < 0x3c) {
        m = D_6eee / 0x28;
        if (D_a67e > m && D_9c69[m].status) {
            vec_1fdb(m);
            sub_root_0000_1588(0x100);
            sub_root_0000_1716(0x100);
            sub_s043_150b_13f2();
            return 1;
        }
    }
    return 0;
}
#pragma optimize("", on)

/* redraw the card after a party sheet / global screen (9c46 or 13a0): reload the picture
 * D_ee5b and the .msg file, then redraw the current text block D_e959 the way 04c4 drew it
 * (D_e898: 1 = option list being shown, 2 = option list after the fade, else plain text).
 * The original contains the same inline _asm recolour loop as 04c4 (unoptimised function). */
#pragma optimize("a", on)
void far sub_s043_150b_13f2(void)
{
    char pathc[60];
    int ih;
    int tn;
    int c;

    strcpy(pathc, "pics\\");
    strcat(pathc, D_ee5b);
    strcat(pathc, ".pic");
    sub_root_0617_024a(1, pathc, D_e48c);
    sub_root_047c_0568();
    sub_root_047c_058b(D_e48c);
    sub_s043_150b_020e(D_e957);
    sub_root_047c_12c3();
    tn = sub_root_047c_051b(D_017c->f10);
    sub_s043_150b_06fe(D_e959, D_9085);
    D_017c->color = D_0154->color = D_ee59;
    if (D_e898 == 1) {
        sub_root_047c_0553(D_0118, 0x43, 6, 0xf5, 0xbc, D_017c, 7, 6);
        sub_s043_150b_28e6(D_017c);
        sub_root_0265_0134(D_017c, D_9085, D_e8a7.w[D_e959] - 10, D_e8a7.x[D_e959] + 5, D_e90b[D_e959] + 5, tn, &D_e96e, 1, 0, 1, 1, 1);
    } else if (D_e898 == 2) {
        sub_root_047c_12c3();
        sub_root_047c_0553(D_0118, 0x43, 6, 0xf5, 0xbc, D_017c, 7, 6);
        sub_s043_150b_28e6(D_017c);
        sub_root_0265_0134(D_017c, D_9085, D_e8a7.w[D_e959] - 10, D_e8a7.x[D_e959] + 5, D_e90b[D_e959] + 5, tn, &D_e96e, 1, 0, 1, 1, 1);
        if (D_9069 == 0) {
            c = 7;
            if (D_e3e3[0] == 'x')
                sub_s043_17e5_0008(7, 100, 0x1f, 0x46, 3);
            else
                sub_s043_17e5_0008(c, 100, 0xff, 0x46, 3);
            sub_root_047c_12c3();
            _asm {
                push si
                push di
                push ds
                push es
                mov ax, 0A000h
                mov ds, ax
                mov es, ax
                mov bl, byte ptr c
                mov cx, 0FA00h
                sub si, si
                sub di, di
            l1:
                lodsb
                cmp al, 20h
                jb l2
                cmp al, 80h
                jae l2
                mov al, bl
            l2:
                stosb
                loop l1
                pop es
                pop ds
                pop di
                pop si
            }
        }
        for (ih = 0; ih < D_e96e.n; ih++)
            if (D_e3e3[0] == 'x') sub_root_047c_04dc(D_017c, D_e96e.x, D_e96e.y[ih], D_e96e.w, D_e96e.h[ih], c, 15);
            else sub_root_047c_04dc(D_017c, D_e96e.x, D_e96e.y[ih], D_e96e.w, D_e96e.h[ih], c, 0);
    } else {
        sub_s043_150b_28e6(D_017c);
        sub_root_047c_0553(D_0118, 0x43, 6, 0xf5, 0xbc, D_017c, 7, 6);
        sub_root_0265_0134(D_017c, D_9085, D_e8a7.w[D_e959] - 10, D_e8a7.x[D_e959] + 5, D_e90b[D_e959] + 5, tn, &D_e96e, 1, 0, 1, 1, 1);
    }
    sub_root_047c_056f();
    sub_root_047c_1293();
    D_8ddc = sub_root_047c_1320(6, 0, 0, 0);
}
#pragma optimize("", on)

/* 1 if the check is disabled (D_906a == 0) or an able party member (status 1) passes one of
 * skills D_ee4b[0..3] (set by the card script) via sub_s029_0e76_1260, else 2 */
int far sub_s043_150b_168c(void)
{
    int i, j;

    if (D_906a == 0)
        return 1;
    for (i = 0; i < D_a67e; i++)
        if (D_9c69[i].status == 1)
            for (j = 0; j < 4; j++)
                if (D_ee4b[j] != -1 && sub_s029_0e76_1260(i, D_ee4b[j]))
                    return 1;
    return 2;
}

/* 1 if the check is disabled (D_906a == 0) or an able party member (status 1) passes one of
 * D_ee43[0..3] via sub_s029_0e76_118e, else 2 */
int far sub_s043_150b_16e8(void)
{
    int i, j;

    if (D_906a == 0)
        return 1;
    for (i = 0; i < D_a67e; i++)
        if (D_9c69[i].status == 1)
            for (j = 0; j < 4; j++)
                if (D_ee43[j] != -1 && sub_s029_0e76_118e(i, D_ee43[j]))
                    return 1;
    return 2;
}

#define INSIDE(x, y, w, h, mx, my) \
    ((x) + 0x3c <= (mx) && (mx) <= (w) + (x) + 0x3c && (y) <= (my) && (my) <= (y) + (h))
#define MX (*(volatile int *)&D_6eec)
#define MY (*(volatile int *)&D_6eee)

/* mouse handling of the option menu: highlight the option line under the mouse (D_6eec,
 * D_6eee; D_ed6c = 1 while it is over a line), show the skill-check box of lines with
 * D_e96e.kind > 0 (2252 + 203a, D_ee41 = 1), switch into that box's own option list when
 * the mouse enters it (D_ee42 = 1, the menu layout is saved in D_a778), and with Shift held
 * show the success chance of the highlighted option (its D_ea14 handler, 1d56).  A click
 * on a line sets D_8dde = 1 and returns the option number (with D_ee42: D_a7ba[D_a779]). */
int far sub_s043_150b_1744(struct gfx *win, struct gfx *win2)
{
    int i2;
    int u;
    struct res r;
    int i;
    int flag;
    int show;
    int t;
    register int j;
    register struct mouse *m;

    D_ed6c = 0;
    i = 0;
    if (i < D_e96e.n)
        do {
            if (INSIDE(D_e972, D_e974[i], D_e970, D_e988[i], MX, MY)) {
                D_ed6c = 1;
                break;
            }
        } while (++i < D_e96e.n);
    if (i < D_e96e.n && D_e96f != i) {
        sub_root_047c_12c3();
        if (D_ee42 == 0 && D_ee41 == 1) {
            sub_root_047c_05a5(win, D_ee0d, D_ee0f, D_a776);
            if (D_e3e3[0] == 'x')
                sub_root_047c_04dc(D_017c, D_ee0d, D_ee0f, D_ee11, D_ee10, 5, D_ee59);
            else
                sub_root_047c_04dc(D_017c, D_ee0d, D_ee0f, D_ee11, D_ee10, 5, 0);
            if (D_a776)
                free_buffer(D_a776, "b");
            D_a776 = 0;
            D_ee41 = 0;
            sub_s043_150b_2802(2);
        }
        if (D_e96f != -1) {
            sub_root_047c_04dc(win, D_e972, D_e974[D_e96f], D_e970, D_e988[D_e96f], D_ee57 = 9, D_ee59);
            if (D_e9c4[D_e96f] == -1) {
                sub_root_0000_1588(0x100);
                sub_root_0000_1716(0x100);
            }
        }
        sub_root_047c_04dc(win, D_e972, D_e974[D_e96f], D_e970, D_e988[D_e96f = i], D_ee59, D_ee57 = 9);
        if (D_e9c4[D_e96f] == -1)
            sub_s043_150b_1f3e();
        if (D_e9c4[D_e96f] > 0 && D_ee41 == 0) {
            switch (D_e9c4[D_e96f]) {
            case 1:
                D_ea64 = D_eb5a;
                break;
            case 2:
                D_ea64 = D_ec50;
                break;
            }
            sub_s043_150b_203a(win, win2, sub_s043_150b_2252(win, D_e96f, D_e9c4[D_e96f]));
            D_ee41 = 1;
            sub_s043_150b_2802(1);
        }
        sub_root_047c_1293();
    }
    if (D_ed6c == 0 && (D_e96f != -1 || D_ee42 == 1)) {
        flag = 0;
        if (D_ee41 == 1 && D_ee42 == 0 && INSIDE(D_ee0d, D_ee0f, D_ee11, D_ee10, MX, MY))
            flag = 1;
        if (flag == 1) {
            D_a778 = D_e96e;
            D_e96e = D_ea64;
            D_e96f = -1;
            D_ee42 = 1;
        } else {
            flag = 0;
            if (D_ee42 == 1 && !INSIDE(D_ee0d, D_ee0f, D_ee11, D_ee10, MX, MY)) {
                D_ee42 = 0;
                D_e96e = D_a778;
                for (i = 0; i < D_e96e.n; i++)
                    if (INSIDE(D_e972, D_e974[i], D_e970, D_e988[i], MX, MY))
                        break;
                if (D_e96f == i)
                    flag = 1;
            }
            if (D_ee41 == 1 && D_ee42 == 0 && flag == 0) {
                sub_root_047c_12c3();
                sub_root_047c_05a5(win, D_ee0d, D_ee0f, D_a776);
                if (D_a776)
                    free_buffer(D_a776, "b");
                if (D_e3e3[0] == 'x')
                    sub_root_047c_04dc(D_017c, D_ee0d, D_ee0f, D_ee11, D_ee10, 5, D_ee59);
                else
                    sub_root_047c_04dc(D_017c, D_ee0d, D_ee0f, D_ee11, D_ee10, 5, 0);
                D_a776 = 0;
                sub_root_047c_1293();
                sub_s043_150b_2802(2);
                D_ee41 = 0;
            }
            if (D_e96f != -1) {
                sub_root_047c_12c3();
                sub_root_047c_04dc(win, D_e972, D_e974[D_e96f], D_e970, D_e988[D_e96f], D_ee57 = 9, D_ee59);
                if (D_e9c4[D_e96f] == -1) {
                    sub_root_0000_1588(0x100);
                    sub_root_0000_1716(0x100);
                }
                sub_root_047c_1293();
                D_e96f = -1;
            }
        }
    }
    if ((*(unsigned char far *)0x00400017L & 2 || *(unsigned char far *)0x00400017L & 1) && D_ed6c == 1 && D_e96f != -1)
        show = 1;
    else
        show = 0;
    if (show && *(volatile char *)&D_e96f != (t = D_5202)) {
        if (t != -10 && D_906a != 2) {
            sub_root_0000_1588(0x100);
            sub_root_0000_1716(0x100);
        }
        D_ee6c = D_e9b0[D_e96f] / (char)4;
        D_ee6a = D_e9b0[D_e96f] % (char)4;
        r = (*D_ea14[D_e9b0[D_e96f]])();
        D_5202 = D_e96f;
        sub_s043_150b_1d56(r.a, r.b);
    }
    if (D_5202 != -10 && !show) {
        D_5202 = -10;
        if (D_906a != 2) {
            sub_root_0000_1588(0x100);
            sub_root_0000_1716(0x100);
        }
    }
    if ((D_8ddc = sub_root_047c_1320(6, 0, 0, 0))->click > 0) {
        D_ed6c = 0;
        j = 0;
        if (j < D_e96e.n) {
            m = D_8ddc;
            do {
                if ((int)(D_e972 + 0x3c) <= m->x && m->x <= D_e970 + D_e972 + 0x3c && D_e974[j] <= m->y && m->y <= D_e974[j] + D_e988[j]) {
                    D_ed6c = 1;
                    break;
                }
            } while (++j < D_e96e.n);
        }
        if (D_ed6c == 1) {
            if (D_ee42 == 0 && D_ee41 == 0) {
                D_8dde = 1;
                return D_e9b0[D_e96f];
            }
            if (D_ee42 == 1) {
                D_ee6a = D_e9b0[D_e96f] % (char)4;
                D_ee6c = D_e9b0[D_e96f] / (char)4;
                D_ee6e = 1;
                D_8dde = 1;
                return D_a7ba[D_a779];
            }
        }
    }
}
#pragma optimize("", on)

/* show "The chance of success is N%" (D_906a == 0), a quality word (D_4fce[pct / 20], or
 * "hopeless" for 0) or "ERROR!!!" (pct == -2) in the side window next to party member `who`
 * (-2 = the whole column).  Nothing when D_906a == 2 (chances hidden). */
#pragma optimize("a", on)
void far sub_s043_150b_1d56(int pct, int who)
{
    int h;
    int y;
    char num[20];
    register int v;

    if (D_906a == 2)
        return;
    D_0104->color = 15;
    D_0104->f0a = 0;
    if (*(volatile int *)&who == -2) {
        for (v = 0; v < 0xc6; v += 0xb)
            sub_root_047c_05a5(D_0118, 3, v, D_a628);
        sub_root_047c_0553(D_0118, 3, 1, 0x36, 0xc6, D_0104, 3, 1);
        h = 0xc6;
        y = 1;
    } else {
        h = 0x26;
        sub_root_047c_05a5(D_0104, 3, y = who * 0x28 + 1, D_a626);
    }
    if (pct >= 0) {
        strcpy(D_9085, "The chance of success is ");
        if (D_906a == 0) {
            itoa(sub_s029_1367_0066(0, 100, pct), num, 10);
            strcat(D_9085, num);
            strcat(D_9085, "%");
        } else if (sub_s029_1367_0066(0, 100, pct))
            strcat(D_9085, D_4fce[sub_s029_1367_0066(0, 100, pct) / 20]);
        else
            strcat(D_9085, "hopeless");
    } else if (pct == -2)
        strcpy(D_9085, "ERROR!!!");
    if ((v = (h - (int)(sub_root_0265_0654(D_0104, D_9085, 0x32, 5, 0, sub_root_047c_051b(D_0104->f10), 0, 0, 0, 0, 0) >> 16)) / 2) < 0)
        v = 0;
    D_01bc = 1;
    sub_root_0265_0134(D_0104, D_9085, 0x32, 5, v + y, sub_root_047c_051b(D_0104->f10), 0, 0, 0, 0, 0, 0);
    D_01bc = 0;
}
#pragma optimize("", on)

/* empty in the game; its message was never shown (the literal is still in _DATA) */
int far sub_s043_150b_1f3e(void)
{
    char *msg;

    msg = "Enemy stats will be displayed here";
}

/* show text D_9085 in the side window D_0104 next to party member `who` (-2 = whole
 * column) and wait for a key or click */
#pragma optimize("a", on)
void far sub_s043_150b_1f40(register int v_di, int p_8)
{
    register int v_si;

    D_0104->color = p_8;
    D_0104->f0a = 0;
    sub_root_047c_12c3();
    if (v_di == -2) {
        v_si = 0;
        do {
            sub_root_047c_05a5(D_0118, 3, v_si, D_a628);
            v_si += 0xb;
        } while (v_si < 0xc6);
        sub_root_047c_0553(D_0118, 3, 1, 0x36, 0xc6, D_0104, 3, 1);
        v_si = 0x51;
    } else {
        v_si = v_di * 0x28;
        v_si++;
        sub_root_047c_05a5(D_0104, 3, v_si, D_a626);
    }
    D_01bc = 1;
    sub_root_0265_0134(D_0104, D_9085, 0x37, 3, v_si + 4, sub_root_047c_051b(D_0104->f10), 0, 0, 0, 0, 0, 0);
    D_01bc = 0;
    sub_root_047c_1293();
    if (!sub_root_047c_073d()) {
        do {
            if (D_6ef0 & 1) {
                break;
            }
            sub_root_0000_0000(D_9419, 0);
        } while (!sub_root_047c_073d());
    }
    do {
    } while (D_6ef0 & 1);
    if (sub_root_047c_073d()) {
        sub_root_047c_0744();
    }
}
#pragma optimize("", on)

/* pop-up text box (D_9085, laid out into D_ea64) of width w next to the highlighted option
 * line; saves the screen under it in D_a776 (04d5) and remembers the box in D_ee0d (x),
 * D_ee0f (y), D_ee11 (w), D_ee10 (h) for 1744's hit test. */
#pragma optimize("eglc", off)
int far sub_s043_150b_203a(struct gfx *win, register int x, register int w)
{
    int yg;
    int hi;
    int x0j;
    int fntk;
    int yaj;
    int oldf;
    int x1m;
    int y1m;

    oldf = win->f10;
    hi = (int)(sub_root_0265_0654(win, D_9085, w, 0, 0, fntk = sub_root_047c_051b(win->f10 = 1), &D_ea64, 1, 0, 0, 0) >> 16) + 6;
    x = D_e972;
    yg = (yaj = D_e974[D_e96f]) + hi <= 0xc1 ? yaj : 0xc1 - hi;
    x += D_e970;
    x0j = x;
    if ((x += w) > 0xf5)
        x = 0xf5 - w;
    else
        x = x0j;
    if (D_a776)
        free_buffer(D_a776, "b");
    sub_root_047c_12c3();
    if (D_e3e3[0] == 'x')
        sub_root_047c_04dc(D_017c, x, yg, w, hi, D_ee59, 5);
    else
        sub_root_047c_04dc(D_017c, x, yg, w, hi, 0, 5);
    sub_root_047c_1293();
    sub_root_047c_12c3();
    sub_root_047c_0545();
    D_a776 = sub_root_047c_04d5(0, x + 0x3c, yg, w, hi);
    sub_root_047c_04f8();
    sub_root_047c_1293();
    sub_root_047c_12c3();
    if (D_e3e3[0] == 'x')
        sub_root_047c_04dc(D_017c, x, yg, w, hi, 5, D_ee59);
    else
        sub_root_047c_04dc(D_017c, x, yg, w, hi, 5, 0);
    sub_root_047c_1293();
    sub_root_047c_12c3();
    if (D_ee72 >= 0) {
        sub_root_047c_0952(win, x, yg, w, hi, 0xff);
        sub_root_047c_0d62(win, x, yg, x1m = w + x - 1, yg, 1);
        sub_root_047c_0d62(win, x1m, yg, x1m, y1m = hi + yg - 1, 1);
        sub_root_047c_0d62(win, x1m, y1m, x, y1m, 1);
        sub_root_047c_0d62(win, x, y1m, x, yg, 1);
    }
    win->color = D_ee59;
    sub_root_0265_0134(win, D_9085, w - 3, x + 3, yg + 3, fntk, &D_ea64, 1, 0, 0, 0, 0);
    D_ee0d = x;
    D_ee0f = yg;
    D_ee11 = w;
    D_ee10 = hi;
    sub_root_047c_1293();
    return win->f10 = oldf;
}
#pragma optimize("", on)

extern char D_ea92[][4];
extern char far * far F_2e38_3bc4[];
int far sub_root_0265_084e();
int far sub_s029_0e76_0c76(int, int);

/* skill-check pop-up text (kind 1: skills D_ee43[4] via s029 118e/0c76, kind 2: D_ee4b[4]
 * via s029 1260): for each able member a line "\x81<name>\xff" and one line per skill:
 * "\x15 " + colour code + skill name (+ " (chance)" for kind 1), or "\x15" when the member
 * cannot act; D_ea92[member][skill] = 2 (leader's own/automatic), 1 (possible), 0 (no).
 * The text goes into D_9085; returns the box width (widest line + 6) for 203a.
 * Compiled without global opts (`egl` off: no loop opts, parameters in memory, only i in a
 * register); local names chosen for the 6.00A name-hash slot order (r s maxw any j line num
 * name sk w top-down). */
#pragma optimize("egl", off)
int far sub_s043_150b_2252(struct gfx *win, int unused, int kind)
{
    register int i;
    int rn;
    char sn[50];
    int maxwe;
    int anyk;
    int jj;
    char linem[160];
    char numf[20];
    char namef[40];
    int skj[4];
    int wb;

    maxwe = -1;
    D_9085[0] = 0;
    if (kind == 1) {
        for (i = 0; i < 4; i++)
            skj[i] = D_ee43[i];
    } else if (kind == 2) {
        for (i = 0; i < 4; i++)
            skj[i] = D_ee4b[i];
    }
    for (i = 0; i < 20; i++)
        D_ea92[0][i] = 0;
    for (i = 0; i < D_a67e; i++) {
        if (D_9c69[i].status == 1) {
            strcpy(namef, "\x81");
            strcat(namef, D_9c69[i].name);
            jj = strlen(namef);
            namef[jj] = 0xff;
            namef[jj + 1] = 0;
            strcat(namef, "\n");
            wb = sub_root_0265_084e(win, D_9c69[i].name);
            if (wb > maxwe)
                maxwe = wb;
        }
        linem[0] = 0;
        anyk = 0;
        for (jj = 0; jj < 4; jj++) {
            if (D_9c69[i].status == 0 || D_9c69[i].status == 5) {
                strcat(linem, "\x15\n");
            } else if (kind == 2 && skj[jj] != -1 &&
                       ((rn = sub_s029_0e76_1260(i, skj[jj])) != 0 ||
                        (D_906a == 0 && D_907b == i) || D_5204 == 1)) {
                anyk = 1;
                strcpy(sn, "\x15 ");
                if (D_906a == 0 && D_907b == i && rn == 0 && D_5204 == 0) {
                    strcat(sn, "\x87");
                    D_ea92[i][jj] = 2;
                } else {
                    if (D_e3e3[0] == 'x')
                        strcat(sn, "\x8f");
                    else
                        strcat(sn, "\x80");
                    D_ea92[i][jj] = 1;
                }
                _fstrcat(sn, F_2e38_3bc4[skj[jj]]);
                strcat(sn, "\n");
                wb = sub_root_0265_084e(win, sn);
                if (wb > maxwe)
                    maxwe = wb;
                strcat(linem, sn);
            } else if (kind == 1 && skj[jj] != -1 &&
                       ((rn = sub_s029_0e76_118e(i, skj[jj])) != 0 ||
                        (D_906a == 0 && D_907b == i) || D_5204 == 1)) {
                anyk = 1;
                strcpy(sn, "\x15 ");
                if (D_906a == 0 && D_907b == i && rn == 0 && D_5204 == 0) {
                    strcat(sn, "\x87");
                    D_ea92[i][jj] = 2;
                } else if (D_906a != 2 && rn == 2 && D_5204 == 0) {
                    strcat(sn, "\x84");
                    D_ea92[i][jj] = 2;
                } else if (rn == 1 || D_5204 != 0) {
                    if (D_e3e3[0] == 'x')
                        strcat(sn, "\x8f");
                    else
                        strcat(sn, "\x80");
                    D_ea92[i][jj] = 1;
                } else {
                    strcat(sn, "\x83");
                    D_ea92[i][jj] = 0;
                }
                _fstrcat(sn, F_2e38_0890[skj[jj]]);
                if (rn == 1) {
                    strcat(sn, " (");
                    sub_root_0265_09b2(rn = sub_s029_0e76_0c76(i, skj[jj]), numf);
                    strcat(sn, numf);
                    strcat(sn, ")");
                }
                strcat(sn, "\n");
                wb = sub_root_0265_084e(win, sn);
                if (wb > maxwe)
                    maxwe = wb;
                strcat(linem, sn);
            } else {
                strcat(linem, "\x15\n");
            }
        }
        if (anyk == 1) {
            strcat(D_9085, namef);
            strcat(D_9085, linem);
            strcat(D_9085, "\x14");
        } else {
            strcat(D_9085, linem);
        }
    }
    if (D_9085[0])
        D_9085[strlen(D_9085) - 2] = 0;
    return maxwe + 6;
}
#pragma optimize("", on)

int far sub_s043_150b_2802(int p_6)
{
    if (p_6 == 1) {
        D_ee13 = D_e970;
        return D_e970 = D_ee0d - D_e972;
    }
    return D_e970 = D_ee13;
}

int far sub_s043_150b_2826(void)
{
    int v_si;
    D_8ddc = sub_root_047c_1320(6, 0, 0, 0);
    sub_root_0000_1384();
    for (;;) {
        sub_root_0000_0000(D_9419, 0);
        if (D_9c46) {
            D_9c46 = 0;
            sub_root_0000_1588(0x100);
            sub_root_0000_1716(0x100);
            sub_s043_150b_13f2();
        }
        if (D_6ef0 & 1 && !sub_s043_150b_13a0()) {
            break;
        }
        if (sub_root_047c_073d()) {
            v_si = sub_root_047c_0744();
            if (v_si == 0x1c0d || v_si == 0x3920 || v_si == 0x1000) {
                break;
            }
            sub_s043_150b_2936(v_si);
        }
        sub_root_0000_1338();
    }
    switch (v_si) {
    case 0x1000:
        sub_root_0000_13a0();
        break;
    case 0x3920:
        v_si = 0x1c0d;
    }
    do {
    } while (D_6ef0 & 1 && D_9c4c);
    return v_si;
}

void far sub_s043_150b_28e6(int p_6)
{
    int v_si;
    v_si = p_6;
    sub_root_047c_05a5(v_si, 7, 0, D_a61e);
    sub_root_047c_05a5(v_si, 0xfc, 0, D_a624);
    sub_root_047c_05a5(v_si, 0, 0, D_a622);
    sub_root_047c_05a5(v_si, 7, 0xc2, D_a620);
}

/* global keys while a card waits for input: F1..F5 show party member 0..4's sheet
 * (vec_1fdb), F6 = vec_185b (then the card is redrawn, 13f2), Shift-F1..F5 (0x5e00..) make
 * that able member the leader (D_907b). */
void far sub_s043_150b_2936(register unsigned key)
{
    switch (key) {
    case 0x3b00:
    case 0x3c00:
    case 0x3d00:
    case 0x3e00:
    case 0x3f00:
        key = (key >> 8) - 0x3b;
        if (key < D_a67e && D_9c69[key].status) {
            vec_1fdb(key);
            sub_root_0000_1588(0x100);
            sub_root_0000_1716(0x100);
            sub_s043_150b_13f2();
        }
        break;
    case 0x4000:
        vec_185b();
        D_9c46 = 0;
        sub_root_0000_1588(0x100);
        sub_root_0000_1716(0x100);
        sub_s043_150b_13f2();
        break;
    case 0x5e00:
    case 0x5f00:
    case 0x6000:
    case 0x6100:
    case 0x6200:
        key = (key >> 8) - 0x5e;
        if (key < D_a67e && D_9c69[key].status == 1) {
            D_907b = key;
            sub_root_0000_1716(0x100);
        }
        break;
    }
}

/* let `hours` pass with the hourglass animation (D_a62a frame, 2d3a): every 3 hours one
 * step (the bell name F_290e_20c3[s029 0744(frame * 3)], day and month are shown), a day
 * every 8 steps (D_2776 = days per month).  A key or click skips the animation. */
void far sub_s043_150b_2a04(int hours)
{
    int framee;
    int monthk;
    int hj;
    int daye;
    int savee;
    int il;
    char numf[12];

    if ((hours /= 3) == 0)
        return;
    while (D_6ef0 & 1)
        ;
    hj = D_00e0 / 3;
    daye = D_00e2;
    monthk = D_00e4;
    D_017c->color = 14;
    sub_root_047c_12c3();
    sub_root_047c_04dc(D_017c, 0x3c, 0x32, 0x78, 0x66, 0, 5);
    sub_root_047c_0545();
    savee = sub_root_047c_04d5(0, 0x78, 0x32, 0x78, 0x66);
    sub_root_047c_04f8();
    sub_root_047c_05a5(D_017c, 0x3c, 0x32, D_a62a);
    for (il = 0; il < hours; il++) {
        if (hj >= 8) {
            if (++daye > D_2776[monthk]) {
                daye -= D_2776[monthk];
                if (++monthk > 11)
                    monthk = 0;
            }
            hj -= 8;
        }
        framee = hj % 8;
        if (!sub_root_047c_073d() && !(D_6ef0 & 1)) {
            sub_s043_150b_2d3a(framee);
            sub_root_047c_0952(D_017c, 0x48, 0x82, 0x28, 8, 0);
            sub_root_047c_0952(D_017c, 0x84, 0x82, 0xc, 8, 0);
            sub_root_047c_0952(D_017c, 0xa0, 0x82, 0xc, 8, 0);
            sub_root_0265_0004(D_017c, 0x48, 0x82, F_290e_20c3[sub_s029_1367_0744(framee * 3)]);
            sub_root_0265_0004(D_017c, 0x84, 0x82, (char far *)itoa(daye, numf, 10));
            sub_root_0265_0004(D_017c, 0xa0, 0x82, (char far *)itoa(monthk + 1, numf, 10));
            vec_1fef(400, 0);
        }
        hj++;
    }
    sub_s043_150b_2d3a(hj = hj % 8);
    sub_root_047c_0952(D_017c, 0x48, 0x82, 0x28, 8, 0);
    sub_root_047c_0952(D_017c, 0x84, 0x82, 0xc, 8, 0);
    sub_root_047c_0952(D_017c, 0xa0, 0x82, 0xc, 8, 0);
    sub_root_0265_0004(D_017c, 0x48, 0x82, F_290e_20c3[sub_s029_1367_0744(hj * 3)]);
    sub_root_0265_0004(D_017c, 0x84, 0x82, (char far *)itoa(daye, numf, 10));
    sub_root_0265_0004(D_017c, 0xa0, 0x82, (char far *)itoa(monthk + 1, numf, 10));
    if (!sub_root_047c_073d() && !(D_6ef0 & 1))
        vec_1fef(1000, 0);
    if (sub_root_047c_073d())
        sub_root_047c_0744();
    sub_root_047c_05a5(D_017c, 0x3c, 0x32, savee);
    sub_root_047c_04dc(D_017c, 0x3c, 0x32, 0x78, 0x66, 5, 0);
    sub_root_047c_1293();
    if (savee)
        free_buffer(savee, "b");
    D_017c->color = 8;
}

/* draw frame k of the hourglass (timer pictures D_a62e/D_a630/D_a632, mirrored) */
void far sub_s043_150b_2d3a(int k)
{
    switch (k) {
    case 0:
        sub_root_047c_055f(D_017c, 0x6c, 0x53, 0x18, -0x14, D_a62e);
        break;
    case 1:
        sub_root_047c_055f(D_017c, 0x6c, 0x53, -0x18, -0x14, D_a630);
        break;
    case 2:
        sub_root_047c_055f(D_017c, 0x6c, 0x53, -0x18, 0x14, D_a632);
        break;
    case 3:
        sub_root_047c_055f(D_017c, 0x6c, 0x53, -0x18, 0x14, D_a630);
        break;
    case 4:
        sub_root_047c_055f(D_017c, 0x6c, 0x53, 0x18, 0x14, D_a62e);
        break;
    case 5:
        sub_root_047c_055f(D_017c, 0x6c, 0x53, 0x18, 0x14, D_a630);
        break;
    case 6:
        sub_root_047c_055f(D_017c, 0x6c, 0x53, 0x18, 0x14, D_a632);
        break;
    case 7:
        sub_root_047c_055f(D_017c, 0x6c, 0x53, 0x18, -0x14, D_a630);
        break;
    }
}
