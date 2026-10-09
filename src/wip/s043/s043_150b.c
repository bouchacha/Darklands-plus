/* place: s043 150b:000a */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* status: WIP (e2, 2026-10-09): 15/28 functions match (fmatch).
 *   match: 000a 0110 01e0 020e 03b0 049a 06fe 1374 168c 16e8 1f3e 1f40 2802 2826 28e6.
 *   2d3a: code identical to the original; only its jump-table words differ because the
 *         function sits at a different offset while earlier functions are stubs.
 *   04c4: BLOCKED - the original contains an inline _asm block (screen recolour loop), which
 *         also disables optimisation for the function; not reproducible without _asm.
 *   10fc: returns through DX (`mov dx,[bp-2]; mov ax,dx`) and passes `name` as far (push ds):
 *         return type/prototype not found yet; CONST slot 7ef8 vs 7dea (far-extern order).
 *   1142: written; the line-index temp is computed before the argument pushes in the
 *         original (6.00A schedules it after) - unsolved.
 *   13a0: compare direction `cmp [a67e],si` (decl-position effect, try declpos.py).
 *   203a: generated code; frame 0x10 vs 0xc (more locals in memory), needs a rewrite.
 *   stubs (not written): 0886 ($variable expansion, 2166 bytes: big switch on 10fc's index,
 *   reads party/city data, he/she/his/her/himself strings), 13f2 (redraw card + text after
 *   a party-sheet visit), 1744 (mouse handling of the option menu, calls 1d56/203a/2252),
 *   1d56 ("The chance of success is N%" box), 2252 (skill/attribute success-chance
 *   calculation for an option), 2936 (global keys: party sheets etc.), 2a04 (combat/
 *   enemy-stats screen).
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
 * 0265_0134 lays text out into D_e96e (n lines, x, width, per-line y[] and height[]).
 */

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
};

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
extern int D_8ddc;
extern char D_9069;
extern char D_907b;
extern char D_9085[];
extern char D_9419[];
extern int D_9c46;
extern char D_9c4c;
struct member { char status; char pad[0x7f]; };
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
extern int D_e972;
extern unsigned char D_e974[];
extern char D_ea64[];
extern int D_ee0d;
extern char D_ee0f;
extern char D_ee10;
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

int far itoa();
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
int far sub_root_047c_1320();
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
int far _fstrcmp();
int far vec_1f77();
int far vec_1fa9();
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
void far sub_s043_150b_0886();
int far sub_s043_150b_10fc();
int far sub_s043_150b_1142();
void far sub_s043_150b_1374();
int far sub_s043_150b_13a0();
void far sub_s043_150b_13f2();
int far sub_s043_150b_168c();
int far sub_s043_150b_16e8();
void far sub_s043_150b_1744();
void far sub_s043_150b_1d56();
int far sub_s043_150b_1f3e();
void far sub_s043_150b_1f40();
int far sub_s043_150b_203a();
void far sub_s043_150b_2252();
int far sub_s043_150b_2802();
int far sub_s043_150b_2826();
void far sub_s043_150b_28e6();
void far sub_s043_150b_2936();
void far sub_s043_150b_2a04();
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
 * mode 1 = the option list: wait (2826) and, unless D_9069, fade and recolour the screen.
 * NOT MATCHABLE under the rules: the original has an inline _asm block (lodsb/stosb loop
 * over A000:0000, recolouring every pixel 0x20..0x7f to colour c) which also turns off the
 * optimiser for the whole function. */
void far sub_s043_150b_04c4(struct gfx *win, int idx, int mode, int flag)
{
    int i;
    int t;
    int c;
    unsigned char far *v;
    unsigned n;

    D_e898 = mode;
    t = sub_root_047c_051b(win->f10);
    sub_s043_150b_06fe(idx, D_9085);
    D_0154->color = D_017c->color = mode == 0 && flag == 1 ? 0 : D_ee59;
    sub_root_047c_12c3();
    if (D_ee55 && mode == 1) {
        D_ee55 = 0;
        sub_root_047c_0568();
        sub_root_047c_058b(D_e48c);
        sub_s043_150b_28e6(D_017c);
        sub_root_047c_0553(D_0118, 0x43, 6, 0xf5, 0xbc, D_017c, 7, 6);
        sub_root_0265_0134(D_017c, D_9085, D_e8a7.w[idx] - 10, D_e8a7.x[idx] + 5, D_e90b[idx] + 5, t, &D_e96e, 1, 0, 1, 1, 1);
        sub_root_047c_056f();
    } else {
        if (mode == 1) {
            sub_root_047c_058b(D_e48c);
            sub_s043_150b_28e6(D_017c);
            sub_root_047c_0553(D_0118, 0x43, 6, 0xf5, 0xbc, D_017c, 7, 6);
        } else
            sub_root_047c_058b(D_e48c);
        sub_root_0265_0134(D_017c, D_9085, D_e8a7.w[idx] - 10, D_e8a7.x[idx] + 5, D_e90b[idx] + 5, t, &D_e96e, 1, 0, 1, 1, 1);
    }
    sub_root_047c_1293();
    if (mode == 1) {
        sub_s043_150b_2826();
        D_e898 = 2;
        if (D_9069 == 0) {
            c = 7;
            if (D_e3e3[0] == 'x')
                sub_s043_17e5_0008(7, 100, 0x1f, 0x46, 3);
            else
                sub_s043_17e5_0008(c, 100, 0xff, 0x46, 3);
            sub_root_047c_12c3();
            v = (unsigned char far *)0xa0000000L;
            for (n = 0xfa00; n; n--, v++)
                if (*v >= 0x20 && *v < 0x80)
                    *v = c;
            for (i = 0; i < D_e96e.n; i++)
                sub_root_047c_04dc(D_017c, D_e96e.x, D_e96e.y[i], D_e96e.w, D_e96e.h[i], c, D_e3e3[0] == 'x' ? 15 : 0);
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

/* sub_s043_150b_0886: not decompiled: es:[reg] unknown es=farslot(32486, 11832) ptr=None */
void far sub_s043_150b_0886(void) { }

/* index of $variable `name` in the variable-name table F_290e_21ff[73], or -1 */
int far sub_s043_150b_10fc(char *name)
{
    int r;
    int i;

    r = -1;
    for (i = 0; i < 73; i++)
        if (_fstrcmp(name, F_290e_21ff[i]) == 0)
            return i;
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
            key = D_e96f < 0 ? 0 : D_e96f;
            sub_s043_150b_1374(win, D_e972, D_e974[key], D_e970, D_e988[key], D_ee57 = 9, D_ee59);
            key--;
            if (D_e96f == 0)
                key = D_e96f = D_e96e.n - 1;
            else
                D_e96f--;
        } else if (key == 0x5000 && D_ed6c == 1) {
            key = D_e96f < 0 ? 0 : D_e96f;
            sub_s043_150b_1374(win, D_e972, D_e974[key], D_e970, D_e988[key], D_ee57 = 9, D_ee59);
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

/* sub_s043_150b_13f2: not decompiled: unhandled instruction movsw word ptr es:[di], word ptr [si] */
void far sub_s043_150b_13f2(void) { }

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

/* sub_s043_150b_1744: not decompiled: memory via unknown si */
void far sub_s043_150b_1744(void) { }

/* sub_s043_150b_1d56: not decompiled: unhandled instruction repne scasb al, byte ptr es:[di] */
void far sub_s043_150b_1d56(void) { }

int far sub_s043_150b_1f3e(void)
{
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

/* pop-up text box (D_9085) of width w next to the highlighted option line */
int far sub_s043_150b_203a(struct gfx *p_6, int p_8, register int v_di)
{
    int l_10;
    int l_e;
    int l_c;
    int l_a;
    int l_8;
    int l_6;
    int l_4;
    int l_2;
    register int v_si;
    l_c = p_6->f10;
    p_6->f10 = 1;
    l_8 = sub_root_047c_051b(1);
    l_4 = (int)(sub_root_0265_0654(p_6, D_9085, v_di, 0, 0, l_8, D_ea64, 1, 0, 0, 0) >> 16) + 6;
    v_si = D_e972;
    l_a = (unsigned char)D_e974[D_e96f];
    if ((unsigned char)D_e974[D_e96f] + l_4 <= 0xc1) {
        l_2 = l_a;
    } else {
        l_2 = 0xc1 - l_4;
    }
    v_si += D_e970;
    l_6 = v_si;
    v_si += v_di;
    if (v_si <= 0xf5) {
        goto L_20e8;
    }
    v_si = 0xf5;
    v_si -= v_di;
    for (;;) {
        if (D_a776) {
            sub_root_05ba_00c2(D_a776, 0x52d5);
        }
        sub_root_047c_12c3();
        if (D_e3e3[0] != 0x78) {
            break;
        }
        sub_root_047c_04dc(D_017c, v_si, l_2, v_di, l_4, D_ee59, 5);
        goto L_2106;
L_20e8:
        v_si = l_6;
    }
    sub_root_047c_04dc(D_017c, v_si, l_2, v_di, l_4, 0, 5);
L_2106:
    sub_root_047c_1293();
    sub_root_047c_12c3();
    sub_root_047c_0545();
    D_a776 = sub_root_047c_04d5(0, v_si + 0x3c, l_2, v_di, l_4);
    sub_root_047c_04f8();
    sub_root_047c_1293();
    sub_root_047c_12c3();
    if (D_e3e3[0] == 0x78) {
        sub_root_047c_04dc(D_017c, v_si, l_2, v_di, l_4, 5, D_ee59);
    } else {
        sub_root_047c_04dc(D_017c, v_si, l_2, v_di, l_4, 5, 0);
    }
    sub_root_047c_1293();
    sub_root_047c_12c3();
    if (D_ee72 >= 0) {
        sub_root_047c_0952(p_6, v_si, l_2, v_di, l_4, 0xff);
        l_e = v_si + v_di - 1;
        sub_root_047c_0d62(p_6, v_si, l_2, v_si + v_di - 1, l_2, 1);
        l_10 = l_2 + l_4 - 1;
        sub_root_047c_0d62(p_6, l_e, l_2, l_e, l_2 + l_4 - 1, 1);
        sub_root_047c_0d62(p_6, l_e, l_10, v_si, l_10, 1);
        sub_root_047c_0d62(p_6, v_si, l_10, v_si, l_2, 1);
    }
    p_6->color = D_ee59;
    sub_root_0265_0134(p_6, D_9085, v_di - 3, v_si + 3, l_2 + 3, l_8, D_ea64, 1, 0, 0, 0, 0);
    D_ee0d = v_si;
    D_ee0f = l_2;
    D_ee11 = v_di;
    D_ee10 = l_4;
    sub_root_047c_1293();
    return p_6->f10 = l_c;
}

/* sub_s043_150b_2252: not decompiled: base+index addressing */
void far sub_s043_150b_2252(void) { }

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

/* sub_s043_150b_2936: not decompiled: cannot print hi8 */
void far sub_s043_150b_2936(void) { }

/* sub_s043_150b_2a04: not decompiled: far struct array with element size 4 / access 2 */
void far sub_s043_150b_2a04(void) { }

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
