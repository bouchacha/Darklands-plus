/* place: root 0265:0004 */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
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

unsigned far text_width(struct textwin *w, char *s);
void far draw_text(struct textwin *w, int x, int y, char *s);
void far draw_text_wrapped(struct textwin *w, char *text, int width, int x, int y, int spacing,
                           struct hotspots *hs, int init, int stop, int leading, int icon,
                           int color);
long far measure_text_wrapped(struct textwin *w, char *text, int width, int x, int y,
                              int spacing, struct hotspots *hs, int a, int leading, int icon,
                              int b);

void far draw_text_far(register struct textwin *w, int x, int y, char far *s)
{
    char buf[200];
    int hires;

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

void far draw_text(register struct textwin *w, int x, int y, char *s)
{
    int hires;

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

void far draw_text_centered(struct textwin *w, int y, char *s)
{
    draw_text(w, (320 - text_width(w, s)) / 2, y, s);
}

void far draw_text_wrapped_far(struct textwin *w, char far *text, int a, int b, int c, int d,
                               int e, int f, int g, int h, int i, int j)
{
    char buf[900];

    sub_root_044a_0043(buf, text);
    draw_text_wrapped(w, buf, a, b, c, d, (struct hotspots *)e, f, g, h, i, j);
}

void far draw_text_wrapped(struct textwin *w, char *text, int width, int x, int y, int spacing,
                           struct hotspots *hs, int init, int stop, int leading, int icon,
                           int color)
{
    char buf[150];
    int tw;
    int width_used;
    int link;
    int lx;
    int lw;
    int k;
    int j;
    int i;
    char *start;
    char *line;
    char *cur;
    char *p;
    int n;
    int font;
    char had_icon;
    char iconw;
    char link_open;
    char newline;
    char more;
    char trail;

    had_icon = 0;
    link_open = 0;
    link = 0;
    k = 0;
    lx = x;
    lw = width;
    start = line = cur = text;
    font = w->font;
    newline = 1;
    more = 1;
    if (init) {
        hs->count = 0;
        hs->x = x;
        hs->width = width;
    }
    if (icon == 1 && *cur > '@' && *cur < '[') {
        sub_root_047c_05a5(w, x, y, D_a65e[*cur]);
        lx = x + 21;
        newline = 1;
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
                *cur != 0x1d && *cur != 0x0d && *cur != 1 && *cur != 0x13)
                width_used += sub_root_047c_04e3(font, *cur);
            else if (*cur == 0x14)
                y += 2;
            else if (*cur == 1) {
                cur++;
                n++;
                if (D_9c30[k] == 0) {
                    while (*cur != 0x13 && *cur != 0)
                        cur++;
                    line = cur + 1;
                    n = -1;
                }
                k++;
            } else if (*cur == 0x13) {
                cur++;
                n++;
            } else if (*cur == 0x15 || *cur == 0x10 || *cur == 0x16 || *cur == 6) {
                if (stop) {
                    more = 0;
                    break;
                }
                if (color == 1)
                    w->color = 0x380;
                if (hs->enabled[link] == 0) {
                    while (*cur != '\n' && *cur != 0)
                        cur++;
                    if (*cur == 0) {
                        line = cur;
                        cur--;
                    } else
                        line = cur + 1;
                    n = -1;
                } else if (hs->enabled[link] == 1) {
                    hs->y[hs->count] = y;
                    hs->height[hs->count] = 0;
                    hs->index[hs->count] = link;
                    link_open = 1;
                    switch (*cur) {
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
                link++;
            } else if (*cur == 0x1d) {
                memcpy(buf, line, n);
                buf[n] = 0;
                iconw = text_width(w, buf);
                newline = 0;
                had_icon = 1;
            }
            n++;
            cur++;
        }
        if (width_used >= lw) {
            cur--;
            n--;
        }
        if (*cur == '\n') {
            newline = 1;
            if (link_open == 1) {
                hs->height[hs->count - 1] += leading + spacing;
                link_open = 0;
            }
        }
        if (*cur == 0) {
            newline = 1;
            if (link_open == 1) {
                hs->height[hs->count - 1] += leading + spacing;
                link_open = 0;
            }
        }
        while (*cur != ' ' && *cur != 0 && *cur != '\n' && *cur != '-' && cur > start &&
               cur > line) {
            cur--;
            n--;
        }
        trail = 0;
        for (p = cur; *p != '\n' && *p != 0; p++)
            if (*p != ' ') {
                trail = 1;
                break;
            }
        if (trail == 0) {
            newline = 1;
            if (link_open == 1) {
                hs->height[hs->count - 1] += leading + spacing;
                link_open = 0;
            }
            cur = p;
        }
        if (*cur == '-')
            n++;
        if (*cur == 0)
            more = 0;
        if (n > 0) {
            for (j = 0, i = 0; i < n; i++) {
                if (line[i] == 0x14 || line[i] == 0x15 || line[i] == 0x10 || line[i] == 0x16 ||
                    line[i] == 6 || line[i] == 0x1d || line[i] == 1 || line[i] == 0x13 ||
                    line[i] == 0x0d)
                    continue;
                buf[j++] = line[i];
            }
            buf[j] = 0;
            switch (D_01bc) {
            case 0:
                sub_root_047c_055a(w, lx, y, buf);
                break;
            case 1:
                sub_root_047c_055a(w, (lw - (tw = text_width(w, buf))) / 2 + lx, y, buf);
                break;
            case 2:
                sub_root_047c_055a(w, lw - (tw = text_width(w, buf)) + lx, y, buf);
                break;
            }
            if (link_open == 1)
                hs->height[hs->count - 1] += spacing;
            y += leading + spacing;
        } else
            width++;
        cur++;
        line = cur;
        if (had_icon == 1) {
            lx = (unsigned char)iconw + x;
            lw = width - (unsigned char)iconw;
            had_icon = 0;
        }
        if (newline == 1) {
            lx = x;
            lw = width;
        }
    }
}
