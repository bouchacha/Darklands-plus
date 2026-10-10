/* place: s033 15d1:000a */
/* orig: size=04a2 CONST=7e7c+0006 _DATA=335c+0092 */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* Alchemy formula viewer (overlay s033, segment 15d1): loads DARKLAND.ALC, counts the
   party's ingredients and shows one formula page with its ingredient list. */
/* STATUS (m0, 2026-10-09): FILE MATCH.  sub_s033_15d1_0106 (alc_load): the 0x2e-byte clear
   is not memset but a fill loop `for (i = 0; i < 0x17; i++) D_e7a6[i] = 0;` under
   `#pragma optimize("a", on)` (6.00A turns it into the same `rep stosw`; with the memset
   intrinsic 6.00A compared `cmp [a67e],ax` with the known-zero AX), plus the session-5 inner
   loop `j = 0; if (D_9cd3[i].nitems > 0) for (; j < ..; j++)` for the DS=2e38 placement.
   Functions carry their original sub_ names; the descriptive names are in the comments. */
#include "darklands/card.h"
#include "darklands/game.h"

struct find_t {
    char reserved[21];
    char attrib;
    unsigned wr_time;
    unsigned wr_date;
    long size;
    char name[13];
};

struct gfx { char pad[12]; int color; };

struct ingred { char count; char pad; int item; };
struct formula {
    char name[0x50];
    int mystic;                 /* +50 "Mystical Number" */
    int x52;
    struct ingred ingr[5];      /* +54 */
};                              /* 0x68 bytes */

struct item { unsigned id; char x2[2]; unsigned char qty; char x5; };
struct invent { struct item it[64]; };          /* 0x180 bytes per character */
struct chr { char nitems; char pad[0x7f]; };    /* DGROUP 9cd3, 0x80 bytes per character */

extern volatile char mouse_buttons;
extern int back_win_font, screen_win_font, D_e7a2, D_e7a0, party_icons_dirty;
extern int party_size;
extern char cur_palette[];
extern struct gfx back_win, screen_win;
extern unsigned char far *D_9c2b;
extern int D_e7a6[];
extern struct chr D_9cd3[];

extern char far *far virtue_names[];
extern struct formula far *far F_2e38_35e4[];
extern char far *far item_defs[];
extern struct invent far inventories[];

int printf(const char *, ...);
int sprintf(char *, const char *, ...);
unsigned _dos_findfirst(const char *, unsigned, struct find_t *);
void far exit_(int);
void *memset(void *, int, unsigned);

void far menu_bar_run(int, int);
void far pic_draw_pal(int, char *, char *);
void far file_load_far(char *, unsigned char far *);
unsigned far mem_alloc(long, char *);
void far mem_free(unsigned, char *);
void far sub_root_047c_0568(void);
void far sub_root_047c_056f(void);
void far set_palette(char *);
int far key_pressed(void);
int far get_key(void);
void far party_panel_frames(int);
void far party_panel_rows(int);
void far quit_to_dos(void);
void far mouse_hide(void);
void far mouse_show(void);
void far gfx_text(struct gfx *, int, int, char *);
void far gfx_copy_rect(struct gfx *, int, int, int, int, struct gfx *, int, int);
void far draw_text_wrapped(struct gfx *, char *, int, int, int, int, int, int, int, int, int, int);

int far sub_s033_15d1_00e6(int r);
void far sub_s033_15d1_0106(int a, int idx);
void far sub_s033_15d1_0260(int a, int idx);
void far sub_s033_15d1_0320(int key);
void far sub_s033_15d1_033a(int idx, struct gfx *g);
void far sub_s033_15d1_03ce(int idx, struct gfx *g);
void far sub_s033_15d1_04aa(int idx, struct gfx *g);

/* alc_view (alc_view): formula viewer main loop; Esc (0x11b) leaves */
void far alc_view(int a, int idx)
{
    int key;
    D_e7a2 = back_win_font;
    D_e7a0 = screen_win_font;
    sub_s033_15d1_0106(a, idx);
    sub_s033_15d1_0260(a, idx);
    while (mouse_buttons & 1)
        ;
    for (;;) {
        key = 0;
        if (mouse_buttons & 1) {
            key = 0x11b;
            while (mouse_buttons & 1)
                ;
        }
        menu_bar_run(0x9419, 0);
        if (party_icons_dirty) {
            party_icons_dirty = 0;
            pic_draw_pal(1, "pics\\alcscrn0.pic", cur_palette);
            sub_root_047c_0568();
            set_palette(cur_palette);
            party_panel_frames(0x100);
            party_panel_rows(0x100);
            sub_s033_15d1_0260(a, idx);
            sub_root_047c_056f();
            key = 0;
        }
        if (key_pressed())
            key = get_key();
        if (key == 0x11b)
            break;
        if (key)
            sub_s033_15d1_0320(key);
    }
    sub_s033_15d1_00e6(-1);
}

/* sub_s033_15d1_00e6 (alc_leave): restore the saved state and free the formula file */
int far sub_s033_15d1_00e6(int r)
{
    back_win_font = D_e7a2;
    screen_win_font = D_e7a0;
    mem_free((unsigned)((unsigned long)D_9c2b >> 16), "alc");
    return r;
}

/* sub_s033_15d1_0106 (alc_load): read DARKLAND.ALC, index its formulas and count the party's ingredients */
#pragma optimize("a", on)
void far sub_s033_15d1_0106(int a, int idx)
{
    char msg[] = "alchemy file\n";
    struct find_t ff;
    unsigned char far *p;
    int i, j, n;

    pic_draw_pal(1, "pics\\alcscrn0.pic", cur_palette);
    if (_dos_findfirst("darkland.alc", 0, &ff)) {
        printf(msg);
        exit_(1);
    } else {
        p = (unsigned char far *)((unsigned long)mem_alloc(ff.size, "alc mem") << 16);
        D_9c2b = p;
        file_load_far("darkland.alc", p);
        n = *p++;
        for (i = 0; i < n; i++) {
            F_2e38_35e4[i] = (struct formula far *)p;
            p += 0x68;
        }
    }
    for (i = 0; i < 0x17; i++)
        D_e7a6[i] = 0;
    for (i = 0; i < party_size; i++) {
        j = 0;
        if (D_9cd3[i].nitems > 0) {
            for (; j < D_9cd3[i].nitems; j++) {
            if (inventories[i].it[j].id >= 0x48 && inventories[i].it[j].id <= 0x5f)
                D_e7a6[inventories[i].it[j].id - 0x48] += inventories[i].it[j].qty;
            }
        }
    }
    back_win.color = 8;
    set_palette(cur_palette);
}

/* sub_s033_15d1_0260 (alc_draw): draw the page for formula idx */
#pragma optimize("", on)
void far sub_s033_15d1_0260(int a, int idx)
{
    char buf[120];
    mouse_hide();
    back_win.color = 0xe;
    sprintf(buf, "%Fs", virtue_names[idx]);
    gfx_text(&back_win, 100, 9, buf);
    sprintf(buf, "Mystical Number - %d", F_2e38_35e4[idx]->mystic);
    gfx_text(&back_win, 100, 0x12, buf);
    sub_s033_15d1_033a(idx, &back_win);
    sub_s033_15d1_03ce(idx, &back_win);
    sub_s033_15d1_04aa(idx, &back_win);
    gfx_copy_rect(&back_win, 0x3c, 0, 0x104, 200, &screen_win, 0x3c, 0);
    mouse_show();
}

/* sub_s033_15d1_0320 (alc_key): key handler */
void far sub_s033_15d1_0320(int key)
{
    switch (key) {
    case 0x1000:
        sub_s033_15d1_00e6(-1);
        quit_to_dos();
    }
}

void far sub_s033_15d1_033a(int idx, struct gfx *g)
{
    char buf[120];
    g->color = 8;
    sprintf(buf, "%Fs", virtue_names[idx]);
    draw_text_wrapped(g, buf, 0x5f, 0x49, 0x3a, 8, 0, 0, 0, 1, 0, 0);
    sprintf(buf, "%Fs", F_2e38_35e4[idx]->name);
    draw_text_wrapped(g, buf, 0x5f, 0x49, 0x52, 8, 0, 0, 0, 1, 0, 0);
}

/* sub_s033_15d1_03ce (alc_draw_ingredients): list the five ingredients, grey when the party has too few */
void far sub_s033_15d1_03ce(int idx, struct gfx *g)
{
    char buf[40];
    int k, y;
    g->color = 8;
    gfx_text(g, 0xcb, 0x39, "Ingredients:");
    y = 0x41;
    for (k = 0; k < 5; k++) {
        if (F_2e38_35e4[idx]->ingr[k].count) {
            sprintf(buf, "%d %Fs (%d)", F_2e38_35e4[idx]->ingr[k].count,
                    item_defs[F_2e38_35e4[idx]->ingr[k].item],
                    D_e7a6[F_2e38_35e4[idx]->ingr[k].item - 0x48]);
            g->color = D_e7a6[F_2e38_35e4[idx]->ingr[k].item - 0x48] < F_2e38_35e4[idx]->ingr[k].count ? 7 : 8;
            gfx_text(g, 0xcd, y, buf);
        }
        y += 8;
    }
}

void far sub_s033_15d1_04aa(int idx, struct gfx *g)
{
}
