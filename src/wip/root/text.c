/* place: root 0265:0004 */
/* orig: size=0a60 */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* status (e3): 8/10 match. Open: 0134 draw_text_wrapped (locals renamed for 6.00A hash order,
   frame layout now right; left: memcpy count kept in a temp (frame 0xc0 vs 0xbe), loop shapes of
   the trailing-space and copy loops (6.00 left them unrotated: jmp test / out-of-line increment),
   switch tail `hs->count++` not shared, `push ax` for x, `cmp [start],cur` operand order).
   0654 measure_text_wrapped: (m0) `start = line = cur = text;` puts start/font in the original
   slots and line in DX.  Left: the original loads text into AX and stores start first
   (`mov ax,[bp+8]; mov [bp-14h],ax; mov dx,ax; mov si,ax`; ours `mov si,[bp+8]; mov dx,si;
   mov [bp-14h],si`) and newline/cur slots (orig -5/-8, ours -7/-6); a struct for cur/newline
   or `char nl[2]` is much worse, `&text[0]` gives the AX load but the wrong order. */
/* Text output (root segment 0265): draw strings in a text window, word-wrapped text with
   embedded control codes and hyperlink-like hotspots, number formatting.
   (The empty function at 0x2652 in front of this module belongs to segment 0000; it is
   called as 0000:2652.) */

struct textwin {
    int f0;
    int x0;         /* +2 */
    int f4;
    int x1;         /* +6 */
    int f8;
    int fa;
    int color;      /* +0c */
    int fe;
    int font;       /* +10 */
};

struct hotspots {
    char count;
    char pad;
    int width;      /* +2 */
    int x;          /* +4 */
    char y[20];     /* +6 */
    char height[20];/* +1a */
    char enabled[20];/* +2e */
    char index[20]; /* +42 */
    int type[20];   /* +56 */
};

extern char D_9c30[];
extern int D_a65e[];
extern char D_01bc;
void *memcpy(void *, const void *, unsigned);

int far sub_root_047c_053e(int);
void far sub_root_047c_055a(struct textwin *, int, int, char *);
void far sub_root_047c_05a5(struct textwin *, int, int, int);
int far sub_root_047c_04e3(int, char);
void far sub_root_044a_0043(char *, char far *);

unsigned far sub_root_0265_084e(struct textwin *w, char *s);
void far sub_root_0265_006e(struct textwin *w, int x, int y, char *s);
void far sub_root_0265_0134(struct textwin *w, char *text, int width, int x, int y, int spacing,
                           struct hotspots *hs, int init, int stop, int leading, int icon,
                           int color);
long far sub_root_0265_0654(struct textwin *w, char *text, int width, int x, int y,
                              int spacing, struct hotspots *hs, int a, int leading, int icon,
                              int b);

/* draw_text_far */
void far sub_root_0265_0004(struct textwin *w0, int x, int y, char far *s)
{
    char buf[200];
    int hires;
    register struct textwin *w;

    w = w0;
    if ((hires = sub_root_047c_053e(6)) != 0) {
        x <<= 1;
        w->x0 <<= 1;
        w->x1 <<= 1;
        w->x1++;
    }
    sub_root_044a_0043(buf, s);
    sub_root_047c_055a(w, x, y, buf);
    if (hires) {
        w->x0 /= 2;
        w->x1 /= 2;
    }
}

/* draw_text */
void far sub_root_0265_006e(struct textwin *w0, int x, int y, char *s)
{
    int hires;
    register struct textwin *w;

    w = w0;
    if ((hires = sub_root_047c_053e(6)) != 0) {
        x <<= 1;
        w->x0 <<= 1;
        w->x1 <<= 1;
        w->x1++;
    }
    sub_root_047c_055a(w, x, y, s);
    if (hires) {
        w->x0 /= 2;
        w->x1 /= 2;
    }
}

/* draw_text_centered */
void far sub_root_0265_00c4(struct textwin *w, int y, char *s)
{
    sub_root_0265_006e(w, (320 - sub_root_0265_084e(w, s)) / 2, y, s);
}

/* draw_text_wrapped_far */
void far sub_root_0265_00f0(struct textwin *w, char far *text, int a, int b, int c, int d,
                               int e, int f, int g, int h, int i, int j)
{
    char buf[900];

    sub_root_044a_0043(buf, text);
    sub_root_0265_0134(w, buf, a, b, c, d, (struct hotspots *)e, f, g, h, i, j);
}

/* draw_text_wrapped */
void far sub_root_0265_0134(struct textwin *w, char *text, int width, int x, int y, int spacing,
                           struct hotspots *hs, int init, int stop, int leading, int icon,
                           int color)
{
    char bufb[150];
    int twd;
    int width_usedo;
    char link_open;
    int linka;
    int lxk;
    int lwk;
    int kh;
    int jc;
    int ib;
    unsigned char *starti;
    unsigned char *linej;
    unsigned char *curj;
    unsigned char *pf;
    int ng;
    int fonti;
    char had_iconl;
    char iconwl;
    char newlinef;
    char moreg;
    char trailm;

    had_iconl = 0;
    link_open = 0;
    linka = 0;
    kh = 0;
    lxk = x;
    lwk = width;
    curj = linej = starti = text;
    fonti = w->font;
    newlinef = 1;
    moreg = 1;
    if (init) {
        hs->count = 0;
        hs->x = x;
        hs->width = width;
    }
    if (icon == 1 && *curj > '@' && *curj < '[') {
        sub_root_047c_05a5(w, x, y, D_a65e[*curj]);
        lxk = x + 21;
        newlinef = 1;
        y += 21 - spacing;
        lwk = width - 21;
        curj++;
        linej++;
    }
    while (moreg) {
        ng = 0;
        width_usedo = 0;
        while (width_usedo < lwk && *curj != 0 && *curj != '\n') {
            if (*curj != 0x14 && *curj != 0x15 && *curj != 0x10 && *curj != 0x16 && *curj != 6 &&
                *curj != 0x1d && *curj != 0x0d && *curj != 1 && *curj != 0x13)
                width_usedo += sub_root_047c_04e3(fonti, *curj);
            else if (*curj == 0x14)
                y += 2;
            else if (*curj == 1) {
                curj++;
                ng++;
                if (D_9c30[kh] == 0) {
                    while (*curj != 0x13 && *curj != 0)
                        curj++;
                    linej = curj + 1;
                    ng = -1;
                }
                kh++;
            } else if (*curj == 0x13) {
                curj++;
                ng++;
            } else if (*curj == 0x15 || *curj == 0x10 || *curj == 0x16 || *curj == 6) {
                if (stop) {
                    moreg = 0;
                    break;
                }
                if (color == 1)
                    w->color = 0x380;
                if (hs->enabled[linka] != 0) {
                    if (hs->enabled[linka] == 1) {
                        hs->y[hs->count] = y;
                        hs->height[hs->count] = 0;
                        hs->index[hs->count] = linka;
                        link_open = 1;
                        switch (*curj) {
                        default:
                            hs->type[hs->count] = 0;
                            break;
                        case 0x10:
                            hs->type[hs->count] = 1;
                            break;
                        case 0x16:
                            hs->type[hs->count] = 2;
                            break;
                        case 6:
                            hs->type[hs->count] = -1;
                            break;
                        }
                        hs->count++;
                    }
                } else {
                    while (*curj != '\n' && *curj != 0)
                        curj++;
                    if (*curj == 0) {
                        linej = curj;
                        curj--;
                    } else
                        linej = curj + 1;
                    ng = -1;
                }
                linka++;
            } else if (*curj == 0x1d) {
                memcpy(bufb, linej, ng);
                bufb[ng] = 0;
                iconwl = sub_root_0265_084e(w, bufb);
                newlinef = 0;
                had_iconl = 1;
            }
            ng++;
            curj++;
        }
        if (width_usedo >= lwk) {
            curj--;
            ng--;
        }
        if (*curj == '\n') {
            newlinef = 1;
            if (link_open == 1) {
                hs->height[hs->count - 1] += leading + spacing;
                link_open = 0;
            }
        }
        if (*curj == 0) {
            newlinef = 1;
            if (link_open == 1) {
                hs->height[hs->count - 1] += leading + spacing;
                link_open = 0;
            }
        }
        while (*curj != ' ' && *curj != 0 && *curj != '\n' && *curj != '-' && starti < curj &&
               curj > linej) {
            curj--;
            ng--;
        }
        trailm = 0;
        pf = curj;
        while (*pf != '\n' && *pf != 0) {
            if (*pf == ' ') {
                pf++;
                continue;
            }
            trailm = 1;
            break;
        }
        if (trailm == 0) {
            newlinef = 1;
            if (link_open == 1) {
                hs->height[hs->count - 1] += leading + spacing;
                link_open = 0;
            }
            curj = pf;
        }
        if (*curj == '-')
            ng++;
        if (*curj == 0)
            moreg = 0;
        if (ng > 0) {
            for (jc = 0, ib = 0; ib < ng; ib++) {
                if (linej[ib] == 0x14 || linej[ib] == 0x15 || linej[ib] == 0x10 || linej[ib] == 0x16 ||
                    linej[ib] == 6 || linej[ib] == 0x1d || linej[ib] == 1 || linej[ib] == 0x13 ||
                    linej[ib] == 0x0d)
                    continue;
                bufb[jc++] = linej[ib];
            }
            bufb[jc] = 0;
            switch (D_01bc) {
            case 0:
                sub_root_047c_055a(w, lxk, y, bufb);
                break;
            case 1:
                sub_root_047c_055a(w, (lwk - (twd = sub_root_0265_084e(w, bufb))) / 2 + lxk, y, bufb);
                break;
            case 2:
                sub_root_047c_055a(w, lwk - (twd = sub_root_0265_084e(w, bufb)) + lxk, y, bufb);
                break;
            }
            if (link_open == 1)
                hs->height[hs->count - 1] += spacing;
            y += leading + spacing;
        } else
            width++;
        curj++;
        linej = curj;
        if (had_iconl == 1) {
            lxk = (unsigned char)iconwl + x;
            lwk = width - (unsigned char)iconwl;
            had_iconl = 0;
        }
        if (newlinef == 1) {
            lxk = x;
            lwk = width;
        }
    }
}

/* measure_text_wrapped_far */
void far sub_root_0265_0614(struct textwin *w, char far *text, int a, int b, int c, int d,
                                  int e, int f, int g, int h, int i)
{
    char buf[900];

    sub_root_044a_0043(buf, text);
    sub_root_0265_0654(w, buf, a, b, c, d, (struct hotspots *)e, f, g, h, i);
}

/* lay out wrapped text without drawing it: returns (y_end - 1) << 16 | last line width */
/* measure_text_wrapped */
long far sub_root_0265_0654(struct textwin *w, char *text, int width, int x, int y,
                              int spacing, struct hotspots *hs, int a, int b, int leading,
                              int icon)
{
    char buf[150];
    int width_used;
    int link;
    int lw;
    char *start;
    char *line;
    char *cur;
    int n;
    int font;
    char had_icon;
    char iconw;
    char newline;
    char more;

    had_icon = 0;
    link = 0;
    lw = width;
    start = line = cur = text;
    font = w->font;
    newline = 1;
    more = 1;
    if (icon == 1) {
        y += 21 - spacing;
        lw = width - 21;
        cur++;
        line++;
    }
    while (more) {
        n = 0;
        width_used = 0;
        while (width_used < lw && *cur != 0 && *cur != '\n') {
            if (*cur != 0x14 && *cur != 0x15 && *cur != 0x10 && *cur != 0x16 && *cur != 6 &&
                *cur != 0x1d && *cur != 0x0d)
                width_used += sub_root_047c_04e3(font, *cur);
            else if (*cur == 0x14)
                y += 2;
            else if (*cur == 0x15 || *cur == 0x10 || *cur == 0x16 || *cur == 6) {
                if (hs->enabled[link] == 0) {
                    while (*cur != '\n' && *cur != 0)
                        cur++;
                    line = cur + 1;
                    n = -1;
                }
                link++;
            } else if (*cur == 0x1d) {
                memcpy(buf, line, n);
                buf[n] = 0;
                iconw = sub_root_0265_084e(w, buf);
                newline = 0;
                had_icon = 1;
            }
            n++;
            cur++;
        }
        if (*cur == '\n')
            newline = 1;
        if (width_used >= lw) {
            cur--;
            n--;
        }
        while (*cur != ' ' && *cur != 0 && *cur != '\n' && *cur != '-' && start < cur) {
            cur--;
            n--;
        }
        if (*cur == '-')
            n++;
        if (*cur == 0)
            more = 0;
        if (n > 0)
            y += leading + spacing;
        else
            width++;
        cur++;
        line = cur;
        if (had_icon == 1) {
            lw = width - (unsigned char)iconw;
            had_icon = 0;
        }
        if (newline == 1)
            lw = width;
    }
    return ((long)(y - 1) << 16) + width_used;
}

/* width in pixels of a string in the window's font */
/* text_width */
unsigned far sub_root_0265_084e(struct textwin *w, char *s)
{
    int font;
    unsigned n;
    char *p;

    font = w->font;
    for (n = 0, p = s; *p; )
        n += sub_root_047c_04e3(font, *p++);
    return n;
}

/* long to decimal text with a thousands comma ("12,345"); at most 6 digits */
/* ltoa_commas */
void far sub_root_0265_0882(long v, char *buf)
{
    char d[6];
    char *p;
    char started;
    char i;

    p = buf;
    if (v < 0) {
        v = -v;
        *p++ = '-';
    }
    d[0] = v % 10;
    d[1] = (v /= 10) % 10;
    d[2] = (v /= 10) % 10;
    d[3] = (v /= 10) % 10;
    d[4] = (v /= 10) % 10;
    d[5] = (v /= 10) % 10;
    *(volatile char *)&started = 0;
    i = 5;
    while (d[i] == 0 && --i > 0)
        ;
    do {
        if (i == 2 && started == 1)
            *p++ = ',';
        *p++ = d[i] + '0';
        started = 1;
    } while (--i >= 0);
    *p = 0;
}

/* int to decimal text with a thousands comma */
/* itoa_commas */
void far sub_root_0265_09b2(int v, char *buf)
{
    char d[6];
    char *p0;
    char *p;
    char started;
    char i;

    if (v < 0) {
        v = -v;
        *buf++ = '-';
    }
    d[0] = v % 10;
    d[1] = (v /= 10) % 10;
    d[2] = (v /= 10) % 10;
    d[3] = (v /= 10) % 10;
    d[4] = (v /= 10) % 10;
    d[5] = (v /= 10) % 10;
    started = 0;
    i = 5;
    p0 = buf;
    p = p0;
    while (d[i] == 0 && --i > 0)
        ;
    do {
        if (i == 2 && started == 1)
            *p++ = ',';
        *p++ = d[i] + '0';
        started = 1;
    } while (--i >= 0);
    *p = 0;
}
