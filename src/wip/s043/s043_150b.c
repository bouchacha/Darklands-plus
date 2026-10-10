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
#include "darklands/card.h"
#include "darklands/game.h"

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
extern int cur_hour;
extern int cur_day;
extern int cur_month;
extern struct gfx *side_win;
extern int screen_ptr;
extern struct gfx *card_window2;
extern struct gfx *card_window;
extern char D_01bc;
extern int combat_clock;
extern int mouse_x;
extern int mouse_y;
extern char mouse_buttons;
struct mouse { int f0; int click; int x; int y; };
extern struct mouse *D_8ddc;
extern char D_9069;
extern char leader;
extern char msg_buf[];
extern char D_9419[];
extern int party_icons_dirty;
extern char D_9c4c;
struct member {                 /* party member, 0x80 bytes */
    char status;                /* 1 = able */
    char pad1[2];
    char sex;                   /* 0 male, else female */
    char pad2[0x26];
    char name[0x56];            /* +2a */
};
extern struct member party[];
extern int D_a61e, D_a620, D_a622, D_a624, D_a626, D_a628, D_a62a, D_a62e, D_a630, D_a632;
extern int party_size;
extern int D_a776;
extern char msg_file[];           /* the .msg file */
extern char card_name[];           /* current card name */
extern char pic_buf[];
extern char cur_palette[];           /* palette */
extern char msg_blocks;             /* number of text blocks */
struct blocks {                 /* msg_index: the text blocks of the current card */
    int ofs[25];                /* e8a7: offset of the text in msg_file */
    int x[25];                  /* e8d9 */
    unsigned char y[26];        /* e90b */
    int w[25];                  /* e925 */
};
extern struct blocks msg_index;
extern unsigned char D_e90b[];  /* = msg_index.y */
extern int D_e898;
extern char *D_e957;
extern int D_e959;              /* current text block */
extern struct layout D_e96e;
extern char menu_highlight;
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
extern int menu_popup, D_8dde, menu_member, menu_slot, D_ee57;
extern char D_e9b0[];           /* option number per text line (-1 = not an option) */
extern char D_a7ba[];
extern char D_a779;
extern char D_ed6c;
extern char game_difficulty;
extern int card_saints[4];
extern int card_items[4];
int far member_bit(int, int);
int far has_item(int, int);
extern unsigned char D_e988[];
extern char far * far var_names[];   /* $variable names */
struct loc {                    /* map location (far) */
    char pad[4];
    int x, y;                   /* +4, +6 */
    char pad2[0xc];
    unsigned char flags;        /* +14 */
    char pad3[0x11];
    char name[1];               /* +26 */
};
struct city {                   /* city_rec: current city record */
    char pad[0x56];
    int f56;
    int f58, f5a;               /* lord title kind (0 = name strings, 1, 2) */
    char pad2[0x12];
    char names[16][32];         /* +6e: lord name/title, buildings (3 = city square ..) */
};
extern struct loc far * far locations[];
extern char far * far F_290e_2183[];
extern char far * far bell_names[];
extern char far * far F_290e_2323[];
extern char far * far F_290e_233b[];
extern char far * far item_defs[];
extern char far * far F_290e_20e3[];
extern char far * far F_290e_2b5b[];
extern char far * far F_290e_1d43[];
extern char far var_named[][0x1e];
extern struct city far *city_rec;
extern int cur_loc, var_loc, var_loc2, D_ed52, var_chosen1, var_chosen2, var_chosen3, var_chosen4, var_chosen5;
extern int var_number1, var_number2, var_item, var_support0, var_support1, var_support2, var_support3, var_support4, var_support5, var_support6;
extern int party_x, party_y;
extern char city_desc[];
extern struct res (far *opt_fn[])();
extern int D_5202;
extern int X_1;                 /* unused: symbol-table position (06fe operand order) */
extern int days_per_month[12];          /* days per month */
void far free_buffer(unsigned, char *);
extern char *quality_words[];           /* quality words (s043_1462) */
extern char var_money1[], var_money2[], var_money3[], var_money4[], var_money5[];
extern char var_text1[], var_text2[], var_text3[], var_text4[];
char far * far _fstrcpy(char far *, const char far *);
char far * far _fstrcat(char far *, const char far *);
void far money_format(char *, char *);
int far cur_band(void);
int far loc_query(int, int);
int far cur_loc_query(int);
int far nearest_loc_type(int, int, int);
int far nearest_city(int, int);
int far vec_1fb3(int);
int far vec_1fbd(int, int, int, int);
int far vec_20df(void);
int far vec_20fd(void);
int far vec_21a7(void);

char * far itoa();
void far sub_s043_1462_0000(char *, char *);
void far palette_fade(int, int, int, int, int);
int far menu_bar_run();
int far exit_if_requested();
int far flush_keys();
int far quit_to_dos();
int far party_panel_frames();
int far party_panel_rows();
int far datafile_select();
int far draw_text_far();
int far draw_text_wrapped();
long far measure_text_wrapped();
int far itoa_commas();
int far sub_root_044a_0043();
int far gfx_save_rect();
int far gfx_recolor_rect();
int far sub_root_047c_04f8();
int far font_height();
int far sub_root_047c_0545();
int far gfx_copy_rect();
int far sub_root_047c_055f();
int far sub_root_047c_0568();
int far sub_root_047c_056f();
int far set_palette();
int far gfx_draw_pic();
int far key_pressed();
int far get_key();
int far gfx_fill_rect();
int far gfx_line();
int far mouse_show();
int far mouse_hide();
struct mouse * far sub_root_047c_1320();
int far mem_free();
int far file_load_far();
int far pic_draw_pal();
int far catalog_find();
int far clamp_range();
int far hour_band();
int far message_box();
int far party_info_screen_vec();
int far mouse_move();
int far sub_root_047c_059e();
int far _fstrcmp(const char far *, const char far *);
int far member_sheet_vec();
int far vec_1fef();
void far card_load();
void far card_load_near();
void far sub_s043_150b_01e0();
void far msg_load();
void far card_text();
void far card_options();
void far sub_s043_150b_04c4();
void far text_expand();
void far var_expand(char *);
int far var_index();
int far card_menu();
void far highlight_rect();
int far sub_s043_150b_13a0();
void far sub_s043_150b_13f2();
int far saints_usable();
int far items_usable();
int far card_mouse();
void far sub_s043_150b_1d56(int, int);
int far sub_s043_150b_1f3e();
void far sub_s043_150b_1f40();
int far sub_s043_150b_203a();
int far sub_s043_150b_2252();
int far sub_s043_150b_2802();
int far wait_continue();
void far sub_s043_150b_28e6();
void far card_global_keys(unsigned);
void far pass_hours_hourglass(int);
void far hourglass_frame();

int D_5202 = -10;
int D_5204 = 0;

/* show card `name` with picture `pic` (far): logs MEM.LOG, loads pics\<pic>.pic (or clears
 * the screen when pic is empty) and loads msgfiles\<name>.msg (020e). */
void far card_load(char *name, char far *pic)
{
    char path[60];

    strcpy(D_ee5b, pic);
    sub_s043_1462_0000(card_name, "a");
    strcpy(path, "pics\\");
    strcat(path, pic);
    strcat(path, ".pic");
    D_ee59 = *pic == 'x' ? 15 : 0;
    if (*pic)
        pic_draw_pal(1, path, cur_palette);
    else
        gfx_fill_rect(screen_ptr, 0, 0, 0x140, 0xc8, 0xff);
    D_ee55 = 1;
    D_e957 = name;
    party_panel_frames(0x100);
    party_panel_rows(0x100);
    msg_load(name);
}

/* same as 000a with a near picture name, without the log / D_e957 / sound reset */
void far card_load_near(char *name, char *pic)
{
    char path[60];

    strcpy(D_ee5b, pic);
    strcpy(path, "pics\\");
    strcat(path, pic);
    strcat(path, ".pic");
    if (*pic == 'x') D_ee59 = 15; else D_ee59 = 0;
    if (*pic)
        pic_draw_pal(1, path, cur_palette);
    else
        gfx_fill_rect(screen_ptr, 0, 0, 0x140, 0xc8, 0xff);
    D_ee55 = 1;
    msg_load(name);
}

void far sub_s043_150b_01e0(void)
{
    if (D_ee55) {
        D_ee55 = 0;
        mouse_hide();
        sub_root_047c_0568();
        set_palette(cur_palette);
        sub_root_047c_056f();
        mouse_show();
    }
}

/* load msgfiles\<name>.msg into msg_file and index its text blocks (see the header comment) */
void far msg_load(char *name)
{
    char path[60];
    struct finfo st;
    int i, n, k;

    strcpy(path, name);
    strcat(path, ".msg");
    datafile_select("msgfiles");
    catalog_find(path, &st);
    if (st.size < 6000) {
        file_load_far(path, (char far *)msg_file);
        k = 1;
        n = msg_blocks = msg_file[0];
        for (i = 0; i < n; i++) {
            D_e90b[i] = msg_file[k];
            k++;
            msg_index.x[i] = *(int *)&msg_file[k];
            k += 2;
            msg_index.w[i] = *(int *)&msg_file[k];
            k += 2;
            msg_index.ofs[i] = k;
            D_e90b[i] = clamp_range(0, 200, D_e90b[i]);
            msg_index.x[i] = clamp_range(0, 0x140, msg_index.x[i]);
            while (msg_file[k])
                k++;
            k++;
        }
    } else {
        strcpy(msg_buf, name);
        strcat(msg_buf, "too large - ");
        itoa_commas(D_ee2f, path);
        strcat(msg_buf, path);
        message_box(msg_buf);
    }
}

/* show text block `idx` of the current card in the text window and wait for a key.
 * how: 0/2 = draw the frame on the card picture, other = clear the window first */
void far card_text(register int idx, register int how)
{
    combat_clock = 0;
    flush_keys();
    D_e959 = idx;
    if (how && how != 2) {
        mouse_hide();
        gfx_fill_rect(card_window, 7, 6, 0xf5, 0xbc, 0xff);
        sub_s043_150b_28e6(card_window);
    } else {
        mouse_hide();
        if (D_ee55) {
            D_ee55 = 0;
            sub_root_047c_0568();
            set_palette(cur_palette);
            sub_s043_150b_28e6(card_window);
            gfx_copy_rect(screen_ptr, 0x43, 6, 0xf5, 0xbc, card_window, 7, 6);
            sub_root_047c_056f();
        } else {
            sub_s043_150b_28e6(card_window);
            gfx_copy_rect(screen_ptr, 0x43, 6, 0xf5, 0xbc, card_window, 7, 6);
        }
    }
    mouse_show();
    sub_s043_150b_04c4(card_window, idx, 0, how);
    wait_continue();
    flush_keys();
}

/* show text block `idx` (the option list) without waiting; the menu (1142) follows */
void far card_options(register int idx)
{
    flush_keys();
    combat_clock = 0;
    D_e959 = idx;
    sub_s043_150b_04c4(card_window, idx, 1, 2);
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
    tbm = font_height(win->f10);
    text_expand(idx, msg_buf);
    card_window->color = card_window2->color = mode == 0 ? flag == 1 ? 0 : D_ee59 : D_ee59;
    mouse_hide();
    if (D_ee55 && mode == 1) {
        D_ee55 = 0;
        sub_root_047c_0568();
        set_palette(cur_palette);
        sub_s043_150b_28e6(card_window);
        gfx_copy_rect(screen_ptr, 0x43, 6, 0xf5, 0xbc, card_window, 7, 6);
        draw_text_wrapped(card_window, msg_buf, msg_index.w[idx] - 10, msg_index.x[idx] + 5, D_e90b[idx] + 5, tbm, &D_e96e, 1, 0, 1, 1, 1);
        sub_root_047c_056f();
    } else {
        if (mode == 1) {
            set_palette(cur_palette);
            sub_s043_150b_28e6(card_window);
            gfx_copy_rect(screen_ptr, 0x43, 6, 0xf5, 0xbc, card_window, 7, 6);
        } else
            set_palette(cur_palette);
        draw_text_wrapped(card_window, msg_buf, msg_index.w[idx] - 10, msg_index.x[idx] + 5, D_e90b[idx] + 5, tbm, &D_e96e, 1, 0, 1, 1, 1);
    }
    mouse_show();
    if (mode == 1) {
        wait_continue();
        D_e898 = 2;
        if (D_9069 == 0) {
            cbo = 7;
            if (pic_buf[0] == 'x')
                palette_fade(7, 100, 0x1f, 0x46, 3);
            else
                palette_fade(cbo, 100, 0xff, 0x46, 3);
            mouse_hide();
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
                if (pic_buf[0] == 'x') gfx_recolor_rect(card_window, D_e96e.x, D_e96e.y[ibf], D_e96e.w, D_e96e.h[ibf], cbo, 15);
                else gfx_recolor_rect(card_window, D_e96e.x, D_e96e.y[ibf], D_e96e.w, D_e96e.h[ibf], cbo, 0);
            mouse_show();
        }
    }
}

/* copy text block idx to out, replacing each $name by its value (0886) */
#pragma check_stack(on)
void far text_expand(int idx, unsigned char *out)
{
    unsigned char c;
    char invar;
    unsigned o, i;
    unsigned len;
    int vl;
    char var[200];
    char buf[900];

    sub_root_044a_0043(buf, (char far *)(msg_file + msg_index.ofs[idx]));
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
                var_expand(var);
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
        var_expand(var);
        out[o] = 0;
        strcat(out, var);
    } else
        out[o] = 0;
}
#pragma check_stack(off)

/* expand $variable `name` in place (name is the buffer the variable name was collected in,
 * 200 bytes).  The index from 10fc selects the value (names: var_names):
 *   0 PlaceName   current city name (city_rec) or location name (cur_loc >= 0x5c)
 *   1 PlaceDesc   city_desc                       2 Inn .. 38 cityChurch: city building names
 *   3 He/She 4 he/she 19 his/her 20 His/Her 21 him/her 44 himself/herself: by the sex of the
 *     party member var_chosen1 (the "chosen one")
 *   5..9 Money1..5 (money records var_money1.. formatted by s029 0376), 17/18 Number1/2 (itoa)
 *   10 LeaderName (leader), 11..15 ChosenOne..FiveName (var_chosen1..var_chosen5), 39..43 NamedOne..Five
 *   (var_named[5][30]), 45..48 Text1..4 (var_text1.. 40 bytes each), 49 CurrentBell,
 *   50/51 CityLordName/Title, 52 CityLocation (D_ed52), 54/70 LocName/LocName2 (var_loc,
 *   var_loc2), 55 ItemName (var_item), 56 NearestCity, 57/58 Direction/Direction2, 59..65
 *   Support..Support6, 66 Government, 67 MonthName, 68 SabbatTime, 69 SabbatLocation,
 *   71 MonasteryLocation, 72 LastCastleLocation.
 *   Unknown names (and 31 docks, 32 poorhouse, 34 whorehouse, 35 warehouse, 53 MapLocation)
 *   are left as "$name". */
void far var_expand(char *name)
{
    int jf;
    int idxl;
    int kg;
    char tmpb[40];

    idxl = var_index(name);
    switch (idxl) {
    case 0:
        if (cur_loc < 0x5c)
            _fstrcpy(name, (char far *)city_rec);
        else
            _fstrcpy(name, locations[cur_loc]->name);
        break;
    case 54:
        _fstrcpy(name, locations[var_loc]->name);
        break;
    case 70:
        _fstrcpy(name, locations[var_loc2]->name);
        break;
    case 1:
        strcpy(name, city_desc);
        break;
    case 16:
        kg = loc_query(cur_loc, 1);
        _fstrcpy(name, F_290e_2183[kg]);
        break;
    case 2:
        _fstrcpy(name, city_rec->names[14]);
        break;
    case 52:
        _fstrcpy(name, city_rec->names[D_ed52]);
        break;
    case 22:
        _fstrcpy(name, city_rec->names[3]);
        break;
    case 24:
        _fstrcpy(name, city_rec->names[9]);
        break;
    case 25:
        _fstrcpy(name, city_rec->names[11]);
        break;
    case 27:
        _fstrcpy(name, city_rec->names[8]);
        break;
    case 29:
        _fstrcpy(name, city_rec->names[12]);
        break;
    case 30:
    case 33:
        _fstrcpy(name, city_rec->names[10]);
        break;
    case 36:
        _fstrcpy(name, city_rec->names[13]);
        break;
    case 37:
        _fstrcpy(name, city_rec->names[6]);
        break;
    case 38:
        _fstrcpy(name, city_rec->names[7]);
        break;
    case 26:
        _fstrcpy(name, city_rec->names[15]);
        break;
    case 3:
        if (party[var_chosen1].sex == 0)
            strcpy(name, "He");
        else
            strcpy(name, "She");
        break;
    case 4:
        if (party[var_chosen1].sex == 0)
            strcpy(name, "he");
        else
            strcpy(name, "she");
        break;
    case 20:
        if (party[var_chosen1].sex == 0)
            strcpy(name, "His");
        else
            strcpy(name, "Her");
        break;
    case 19:
        if (party[var_chosen1].sex == 0)
            strcpy(name, "his");
        else
            strcpy(name, "her");
        break;
    case 21:
        if (party[var_chosen1].sex == 0)
            strcpy(name, "him");
        else
            strcpy(name, "her");
        break;
    case 44:
        if (party[var_chosen1].sex == 0)
            strcpy(name, "himself");
        else
            strcpy(name, "herself");
        break;
    case 5:
        money_format(var_money1, name);
        break;
    case 6:
        money_format(var_money2, name);
        break;
    case 7:
        money_format(var_money3, name);
        break;
    case 8:
        money_format(var_money4, name);
        break;
    case 9:
        money_format(var_money5, name);
        break;
    case 10:
        strcpy(name, party[leader].name);
        break;
    case 11:
        strcpy(name, party[var_chosen1].name);
        break;
    case 12:
        strcpy(name, party[var_chosen2].name);
        break;
    case 13:
        strcpy(name, party[var_chosen3].name);
        break;
    case 14:
        strcpy(name, party[var_chosen4].name);
        break;
    case 15:
        strcpy(name, party[var_chosen5].name);
        break;
    case 17:
        itoa(var_number1, name, 10);
        break;
    case 18:
        itoa(var_number2, name, 10);
        break;
    case 39:
    case 40:
    case 41:
    case 42:
    case 43:
        _fstrcpy(name, var_named[idxl - 39]);
        break;
    case 45:
        strcpy(name, var_text1);
        break;
    case 46:
        strcpy(name, var_text2);
        break;
    case 47:
        strcpy(name, var_text3);
        break;
    case 48:
        strcpy(name, var_text4);
        break;
    case 49:
        _fstrcpy(name, bell_names[cur_band()]);
        break;
    case 51:
        kg = !(locations[cur_loc]->flags & 0x80) ? city_rec->f58 : city_rec->f5a;
        switch (kg) {
        case 0:
            if (!(locations[cur_loc]->flags & 0x80))
                _fstrcpy(name, city_rec->names[0]);
            else
                _fstrcpy(name, city_rec->names[1]);
            break;
        case 1:
            _fstrcpy(name, F_290e_2323[city_rec->f56 % 6]);
            break;
        case 2:
            _fstrcpy(name, F_290e_233b[city_rec->f56 % 9]);
            break;
        }
        break;
    case 50:
        _fstrcpy(name, city_rec->names[0]);
        break;
    case 55:
        _fstrcpy(name, item_defs[var_item]);
        break;
    case 56:
        if (cur_loc != -1)
            _fstrcpy(name, locations[nearest_city(locations[var_loc]->x, locations[var_loc]->y)]->name);
        else
            _fstrcpy(name, locations[nearest_city(party_x, party_y)]->name);
        break;
    case 57:
        _fstrcpy(name, F_290e_20e3[vec_1fb3(var_loc)]);
        break;
    case 58:
        kg = vec_1fbd(locations[var_loc2]->x, locations[var_loc2]->y, locations[var_loc]->x, locations[var_loc]->y);
        _fstrcpy(name, F_290e_20e3[kg]);
        break;
    case 59:
        _fstrcpy(name, F_290e_2b5b[var_support0]);
        break;
    case 60:
        _fstrcpy(name, F_290e_2b5b[var_support1]);
        break;
    case 61:
        _fstrcpy(name, F_290e_2b5b[var_support2]);
        break;
    case 62:
        _fstrcpy(name, F_290e_2b5b[var_support3]);
        break;
    case 63:
        _fstrcpy(name, F_290e_2b5b[var_support4]);
        break;
    case 64:
        _fstrcpy(name, F_290e_2b5b[var_support5]);
        break;
    case 65:
        _fstrcpy(name, F_290e_2b5b[var_support6]);
        break;
    case 66:
        if (cur_loc_query(0x18))
            _fstrcpy(name, city_rec->names[4]);
        else
            _fstrcpy(name, city_rec->names[5]);
        break;
    case 23:
        _fstrcpy(name, city_rec->names[4]);
        break;
    case 28:
        _fstrcpy(name, city_rec->names[5]);
        break;
    case 67:
        _fstrcpy(name, F_290e_1d43[cur_month]);
        break;
    case 68:
        _fstrcpy(name, F_290e_1d43[vec_20df()]);
        itoa(kg = vec_21a7(), tmpb, 10);
        strcat(name, " ");
        strcat(name, tmpb);
        break;
    case 69:
        kg = vec_20fd();
        jf = nearest_city(locations[kg]->x, locations[kg]->y);
        _fstrcpy(name, F_290e_20e3[vec_1fbd(locations[jf]->x, locations[jf]->y, locations[kg]->x, locations[kg]->y)]);
        strcat(name, " of ");
        _fstrcat(name, locations[nearest_city(locations[kg]->x, locations[kg]->y)]->name);
        break;
    case 71:
        kg = nearest_loc_type(party_x, party_y, 0x16);
        jf = nearest_city(locations[kg]->x, locations[kg]->y);
        _fstrcpy(name, F_290e_20e3[vec_1fbd(locations[jf]->x, locations[jf]->y, locations[kg]->x, locations[kg]->y)]);
        strcat(name, " of ");
        _fstrcat(name, locations[jf]->name);
        break;
    case 72:
        kg = nearest_loc_type(party_x, party_y, 0x17);
        jf = nearest_city(locations[kg]->x, locations[kg]->y);
        _fstrcpy(name, F_290e_20e3[vec_1fbd(locations[jf]->x, locations[jf]->y, locations[kg]->x, locations[kg]->y)]);
        strcat(name, " of ");
        _fstrcat(name, locations[jf]->name);
        break;
    default:
        strcpy(tmpb, "$");
        strcat(tmpb, name);
        strcpy(name, tmpb);
        break;
    }
}

/* index of $variable `name` in the variable-name table var_names[73], or -1 */
int far var_index(char *name)
{
    int r;
    int i;

    r = -1;
    for (i = 0; i < 73; i++)
        if (_fstrcmp(name, var_names[i]) == 0) {
            r = i;
            break;
        }
    return r;
}

/* the option menu: highlight with the cursor keys (menu_highlight = highlighted line of the
 * option text, D_e9b0[line] = option number of that line), Enter/Space chooses; mouse
 * handling is in 1744.  Returns the option index (the script calls opt_fn[index]). */
int far card_menu(struct gfx *win, struct gfx *win2)
{
    register int key;
    register int sel;

    combat_clock = 0;
    if (D_9c4c)
        sub_root_047c_1320(6, 0, 0, 0);
    flush_keys();
    D_ee41 = D_ee42 = 0;
    menu_popup = D_8dde = 0;
    menu_highlight = -1;
    for (;;) {
        menu_bar_run(D_9419, 0);
        if (party_icons_dirty) {
            party_icons_dirty = 0;
            party_panel_frames(0x100);
            party_panel_rows(0x100);
            sub_s043_150b_13f2();
        }
        if (mouse_buttons & 1)
            sub_s043_150b_13a0();
        sel = card_mouse(win, win2);
        if (D_8dde == 1)
            break;
        if (!key_pressed())
            continue;
        key = get_key();
        if ((key == 0x1c0d || key == 0x3920) && menu_highlight != -1) {
            if (D_ee42) {
                menu_member = D_e9b0[menu_highlight] / (char)4;
                menu_slot = D_e9b0[menu_highlight] % (char)4;
                sel = D_a7ba[D_a779];
            } else
                sel = D_e9b0[menu_highlight];
            if (sel >= 0)
                break;
        }
        if (key == 0x1000)
            quit_to_dos();
        card_global_keys(key);
        if (key == 0x4800 && D_ed6c == 1) {
            highlight_rect(win, D_e972, D_e974[key], D_e970, D_e988[key = menu_highlight < 0 ? 0 : menu_highlight], D_ee57 = 9, D_ee59);
            key--;
            if (menu_highlight == 0)
                key = menu_highlight = D_e96e.n - 1;
            else
                menu_highlight--;
        } else if (key == 0x5000 && D_ed6c == 1) {
            highlight_rect(win, D_e972, D_e974[key], D_e970, D_e988[key = menu_highlight < 0 ? 0 : menu_highlight], D_ee57 = 9, D_ee59);
            key++;
            if (++menu_highlight >= D_e96e.n) {
                menu_highlight = 0;
                key = 0;
            }
        } else
            goto next;
        highlight_rect(win, D_e972, D_e974[key], D_e970, D_e988[key], D_ee59, D_ee57);
        mouse_hide();
        mouse_move(mouse_x, D_e974[key] + 2);
        mouse_show();
    next:
        exit_if_requested();
    }
    sub_root_047c_059e(0);
    return sel;
}

/* draw a (highlight) rectangle with the mouse cursor hidden */
void far highlight_rect(struct gfx *win, int x, int y, int w, int h, int c1, int c2)
{
    mouse_hide();
    gfx_recolor_rect(win, x, y, w, h, c1, c2);
    mouse_show();
}

/* mouse click in the party portrait column: show that member's sheet (member_sheet_vec) */
#pragma optimize("eglc", off)
int far sub_s043_150b_13a0(void)
{
    register int m;

    if (mouse_x < 0x3c) {
        m = mouse_y / 0x28;
        if (party_size > m && party[m].status) {
            member_sheet_vec(m);
            party_panel_frames(0x100);
            party_panel_rows(0x100);
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
    pic_draw_pal(1, pathc, cur_palette);
    sub_root_047c_0568();
    set_palette(cur_palette);
    msg_load(D_e957);
    mouse_hide();
    tn = font_height(card_window->f10);
    text_expand(D_e959, msg_buf);
    card_window->color = card_window2->color = D_ee59;
    if (D_e898 == 1) {
        gfx_copy_rect(screen_ptr, 0x43, 6, 0xf5, 0xbc, card_window, 7, 6);
        sub_s043_150b_28e6(card_window);
        draw_text_wrapped(card_window, msg_buf, msg_index.w[D_e959] - 10, msg_index.x[D_e959] + 5, D_e90b[D_e959] + 5, tn, &D_e96e, 1, 0, 1, 1, 1);
    } else if (D_e898 == 2) {
        mouse_hide();
        gfx_copy_rect(screen_ptr, 0x43, 6, 0xf5, 0xbc, card_window, 7, 6);
        sub_s043_150b_28e6(card_window);
        draw_text_wrapped(card_window, msg_buf, msg_index.w[D_e959] - 10, msg_index.x[D_e959] + 5, D_e90b[D_e959] + 5, tn, &D_e96e, 1, 0, 1, 1, 1);
        if (D_9069 == 0) {
            c = 7;
            if (pic_buf[0] == 'x')
                palette_fade(7, 100, 0x1f, 0x46, 3);
            else
                palette_fade(c, 100, 0xff, 0x46, 3);
            mouse_hide();
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
            if (pic_buf[0] == 'x') gfx_recolor_rect(card_window, D_e96e.x, D_e96e.y[ih], D_e96e.w, D_e96e.h[ih], c, 15);
            else gfx_recolor_rect(card_window, D_e96e.x, D_e96e.y[ih], D_e96e.w, D_e96e.h[ih], c, 0);
    } else {
        sub_s043_150b_28e6(card_window);
        gfx_copy_rect(screen_ptr, 0x43, 6, 0xf5, 0xbc, card_window, 7, 6);
        draw_text_wrapped(card_window, msg_buf, msg_index.w[D_e959] - 10, msg_index.x[D_e959] + 5, D_e90b[D_e959] + 5, tn, &D_e96e, 1, 0, 1, 1, 1);
    }
    sub_root_047c_056f();
    mouse_show();
    D_8ddc = sub_root_047c_1320(6, 0, 0, 0);
}
#pragma optimize("", on)

/* 1 if the check is disabled (game_difficulty == 0) or an able party member (status 1) passes one of
 * skills card_saints[0..3] (set by the card script) via member_bit, else 2 */
int far saints_usable(void)
{
    int i, j;

    if (game_difficulty == 0)
        return 1;
    for (i = 0; i < party_size; i++)
        if (party[i].status == 1)
            for (j = 0; j < 4; j++)
                if (card_saints[j] != -1 && member_bit(i, card_saints[j]))
                    return 1;
    return 2;
}

/* 1 if the check is disabled (game_difficulty == 0) or an able party member (status 1) passes one of
 * card_items[0..3] via has_item, else 2 */
int far items_usable(void)
{
    int i, j;

    if (game_difficulty == 0)
        return 1;
    for (i = 0; i < party_size; i++)
        if (party[i].status == 1)
            for (j = 0; j < 4; j++)
                if (card_items[j] != -1 && has_item(i, card_items[j]))
                    return 1;
    return 2;
}

#define INSIDE(x, y, w, h, mx, my) \
    ((x) + 0x3c <= (mx) && (mx) <= (w) + (x) + 0x3c && (y) <= (my) && (my) <= (y) + (h))
#define MX (*(volatile int *)&mouse_x)
#define MY (*(volatile int *)&mouse_y)

/* mouse handling of the option menu: highlight the option line under the mouse (mouse_x,
 * mouse_y; D_ed6c = 1 while it is over a line), show the skill-check box of lines with
 * D_e96e.kind > 0 (2252 + 203a, D_ee41 = 1), switch into that box's own option list when
 * the mouse enters it (D_ee42 = 1, the menu layout is saved in D_a778), and with Shift held
 * show the success chance of the highlighted option (its opt_fn handler, 1d56).  A click
 * on a line sets D_8dde = 1 and returns the option number (with D_ee42: D_a7ba[D_a779]). */
int far card_mouse(struct gfx *win, struct gfx *win2)
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
    if (i < D_e96e.n && menu_highlight != i) {
        mouse_hide();
        if (D_ee42 == 0 && D_ee41 == 1) {
            gfx_draw_pic(win, D_ee0d, D_ee0f, D_a776);
            if (pic_buf[0] == 'x')
                gfx_recolor_rect(card_window, D_ee0d, D_ee0f, D_ee11, D_ee10, 5, D_ee59);
            else
                gfx_recolor_rect(card_window, D_ee0d, D_ee0f, D_ee11, D_ee10, 5, 0);
            if (D_a776)
                free_buffer(D_a776, "b");
            D_a776 = 0;
            D_ee41 = 0;
            sub_s043_150b_2802(2);
        }
        if (menu_highlight != -1) {
            gfx_recolor_rect(win, D_e972, D_e974[menu_highlight], D_e970, D_e988[menu_highlight], D_ee57 = 9, D_ee59);
            if (D_e9c4[menu_highlight] == -1) {
                party_panel_frames(0x100);
                party_panel_rows(0x100);
            }
        }
        gfx_recolor_rect(win, D_e972, D_e974[menu_highlight], D_e970, D_e988[menu_highlight = i], D_ee59, D_ee57 = 9);
        if (D_e9c4[menu_highlight] == -1)
            sub_s043_150b_1f3e();
        if (D_e9c4[menu_highlight] > 0 && D_ee41 == 0) {
            switch (D_e9c4[menu_highlight]) {
            case 1:
                D_ea64 = D_eb5a;
                break;
            case 2:
                D_ea64 = D_ec50;
                break;
            }
            sub_s043_150b_203a(win, win2, sub_s043_150b_2252(win, menu_highlight, D_e9c4[menu_highlight]));
            D_ee41 = 1;
            sub_s043_150b_2802(1);
        }
        mouse_show();
    }
    if (D_ed6c == 0 && (menu_highlight != -1 || D_ee42 == 1)) {
        flag = 0;
        if (D_ee41 == 1 && D_ee42 == 0 && INSIDE(D_ee0d, D_ee0f, D_ee11, D_ee10, MX, MY))
            flag = 1;
        if (flag == 1) {
            D_a778 = D_e96e;
            D_e96e = D_ea64;
            menu_highlight = -1;
            D_ee42 = 1;
        } else {
            flag = 0;
            if (D_ee42 == 1 && !INSIDE(D_ee0d, D_ee0f, D_ee11, D_ee10, MX, MY)) {
                D_ee42 = 0;
                D_e96e = D_a778;
                for (i = 0; i < D_e96e.n; i++)
                    if (INSIDE(D_e972, D_e974[i], D_e970, D_e988[i], MX, MY))
                        break;
                if (menu_highlight == i)
                    flag = 1;
            }
            if (D_ee41 == 1 && D_ee42 == 0 && flag == 0) {
                mouse_hide();
                gfx_draw_pic(win, D_ee0d, D_ee0f, D_a776);
                if (D_a776)
                    free_buffer(D_a776, "b");
                if (pic_buf[0] == 'x')
                    gfx_recolor_rect(card_window, D_ee0d, D_ee0f, D_ee11, D_ee10, 5, D_ee59);
                else
                    gfx_recolor_rect(card_window, D_ee0d, D_ee0f, D_ee11, D_ee10, 5, 0);
                D_a776 = 0;
                mouse_show();
                sub_s043_150b_2802(2);
                D_ee41 = 0;
            }
            if (menu_highlight != -1) {
                mouse_hide();
                gfx_recolor_rect(win, D_e972, D_e974[menu_highlight], D_e970, D_e988[menu_highlight], D_ee57 = 9, D_ee59);
                if (D_e9c4[menu_highlight] == -1) {
                    party_panel_frames(0x100);
                    party_panel_rows(0x100);
                }
                mouse_show();
                menu_highlight = -1;
            }
        }
    }
    if ((*(unsigned char far *)0x00400017L & 2 || *(unsigned char far *)0x00400017L & 1) && D_ed6c == 1 && menu_highlight != -1)
        show = 1;
    else
        show = 0;
    if (show && *(volatile char *)&menu_highlight != (t = D_5202)) {
        if (t != -10 && game_difficulty != 2) {
            party_panel_frames(0x100);
            party_panel_rows(0x100);
        }
        menu_member = D_e9b0[menu_highlight] / (char)4;
        menu_slot = D_e9b0[menu_highlight] % (char)4;
        r = (*opt_fn[D_e9b0[menu_highlight]])();
        D_5202 = menu_highlight;
        sub_s043_150b_1d56(r.a, r.b);
    }
    if (D_5202 != -10 && !show) {
        D_5202 = -10;
        if (game_difficulty != 2) {
            party_panel_frames(0x100);
            party_panel_rows(0x100);
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
                return D_e9b0[menu_highlight];
            }
            if (D_ee42 == 1) {
                menu_slot = D_e9b0[menu_highlight] % (char)4;
                menu_member = D_e9b0[menu_highlight] / (char)4;
                menu_popup = 1;
                D_8dde = 1;
                return D_a7ba[D_a779];
            }
        }
    }
}
#pragma optimize("", on)

/* show "The chance of success is N%" (game_difficulty == 0), a quality word (quality_words[pct / 20], or
 * "hopeless" for 0) or "ERROR!!!" (pct == -2) in the side window next to party member `who`
 * (-2 = the whole column).  Nothing when game_difficulty == 2 (chances hidden). */
#pragma optimize("a", on)
void far sub_s043_150b_1d56(int pct, int who)
{
    int h;
    int y;
    char num[20];
    register int v;

    if (game_difficulty == 2)
        return;
    side_win->color = 15;
    side_win->f0a = 0;
    if (*(volatile int *)&who == -2) {
        for (v = 0; v < 0xc6; v += 0xb)
            gfx_draw_pic(screen_ptr, 3, v, D_a628);
        gfx_copy_rect(screen_ptr, 3, 1, 0x36, 0xc6, side_win, 3, 1);
        h = 0xc6;
        y = 1;
    } else {
        h = 0x26;
        gfx_draw_pic(side_win, 3, y = who * 0x28 + 1, D_a626);
    }
    if (pct >= 0) {
        strcpy(msg_buf, "The chance of success is ");
        if (game_difficulty == 0) {
            itoa(clamp_range(0, 100, pct), num, 10);
            strcat(msg_buf, num);
            strcat(msg_buf, "%");
        } else if (clamp_range(0, 100, pct))
            strcat(msg_buf, quality_words[clamp_range(0, 100, pct) / 20]);
        else
            strcat(msg_buf, "hopeless");
    } else if (pct == -2)
        strcpy(msg_buf, "ERROR!!!");
    if ((v = (h - (int)(measure_text_wrapped(side_win, msg_buf, 0x32, 5, 0, font_height(side_win->f10), 0, 0, 0, 0, 0) >> 16)) / 2) < 0)
        v = 0;
    D_01bc = 1;
    draw_text_wrapped(side_win, msg_buf, 0x32, 5, v + y, font_height(side_win->f10), 0, 0, 0, 0, 0, 0);
    D_01bc = 0;
}
#pragma optimize("", on)

/* empty in the game; its message was never shown (the literal is still in _DATA) */
int far sub_s043_150b_1f3e(void)
{
    char *msg;

    msg = "Enemy stats will be displayed here";
}

/* show text msg_buf in the side window side_win next to party member `who` (-2 = whole
 * column) and wait for a key or click */
#pragma optimize("a", on)
void far sub_s043_150b_1f40(register int v_di, int p_8)
{
    register int v_si;

    side_win->color = p_8;
    side_win->f0a = 0;
    mouse_hide();
    if (v_di == -2) {
        v_si = 0;
        do {
            gfx_draw_pic(screen_ptr, 3, v_si, D_a628);
            v_si += 0xb;
        } while (v_si < 0xc6);
        gfx_copy_rect(screen_ptr, 3, 1, 0x36, 0xc6, side_win, 3, 1);
        v_si = 0x51;
    } else {
        v_si = v_di * 0x28;
        v_si++;
        gfx_draw_pic(side_win, 3, v_si, D_a626);
    }
    D_01bc = 1;
    draw_text_wrapped(side_win, msg_buf, 0x37, 3, v_si + 4, font_height(side_win->f10), 0, 0, 0, 0, 0, 0);
    D_01bc = 0;
    mouse_show();
    if (!key_pressed()) {
        do {
            if (mouse_buttons & 1) {
                break;
            }
            menu_bar_run(D_9419, 0);
        } while (!key_pressed());
    }
    do {
    } while (mouse_buttons & 1);
    if (key_pressed()) {
        get_key();
    }
}
#pragma optimize("", on)

/* pop-up text box (msg_buf, laid out into D_ea64) of width w next to the highlighted option
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
    hi = (int)(measure_text_wrapped(win, msg_buf, w, 0, 0, fntk = font_height(win->f10 = 1), &D_ea64, 1, 0, 0, 0) >> 16) + 6;
    x = D_e972;
    yg = (yaj = D_e974[menu_highlight]) + hi <= 0xc1 ? yaj : 0xc1 - hi;
    x += D_e970;
    x0j = x;
    if ((x += w) > 0xf5)
        x = 0xf5 - w;
    else
        x = x0j;
    if (D_a776)
        free_buffer(D_a776, "b");
    mouse_hide();
    if (pic_buf[0] == 'x')
        gfx_recolor_rect(card_window, x, yg, w, hi, D_ee59, 5);
    else
        gfx_recolor_rect(card_window, x, yg, w, hi, 0, 5);
    mouse_show();
    mouse_hide();
    sub_root_047c_0545();
    D_a776 = gfx_save_rect(0, x + 0x3c, yg, w, hi);
    sub_root_047c_04f8();
    mouse_show();
    mouse_hide();
    if (pic_buf[0] == 'x')
        gfx_recolor_rect(card_window, x, yg, w, hi, 5, D_ee59);
    else
        gfx_recolor_rect(card_window, x, yg, w, hi, 5, 0);
    mouse_show();
    mouse_hide();
    if (D_ee72 >= 0) {
        gfx_fill_rect(win, x, yg, w, hi, 0xff);
        gfx_line(win, x, yg, x1m = w + x - 1, yg, 1);
        gfx_line(win, x1m, yg, x1m, y1m = hi + yg - 1, 1);
        gfx_line(win, x1m, y1m, x, y1m, 1);
        gfx_line(win, x, y1m, x, yg, 1);
    }
    win->color = D_ee59;
    draw_text_wrapped(win, msg_buf, w - 3, x + 3, yg + 3, fntk, &D_ea64, 1, 0, 0, 0, 0);
    D_ee0d = x;
    D_ee0f = yg;
    D_ee11 = w;
    D_ee10 = hi;
    mouse_show();
    return win->f10 = oldf;
}
#pragma optimize("", on)

extern char D_ea92[][4];
extern char far * far F_2e38_3bc4[];
int far text_pixel_width();
int far count_items(int, int);

/* skill-check pop-up text (kind 1: skills card_items[4] via s029 118e/0c76, kind 2: card_saints[4]
 * via s029 1260): for each able member a line "\x81<name>\xff" and one line per skill:
 * "\x15 " + colour code + skill name (+ " (chance)" for kind 1), or "\x15" when the member
 * cannot act; D_ea92[member][skill] = 2 (leader's own/automatic), 1 (possible), 0 (no).
 * The text goes into msg_buf; returns the box width (widest line + 6) for 203a.
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
    msg_buf[0] = 0;
    if (kind == 1) {
        for (i = 0; i < 4; i++)
            skj[i] = card_items[i];
    } else if (kind == 2) {
        for (i = 0; i < 4; i++)
            skj[i] = card_saints[i];
    }
    for (i = 0; i < 20; i++)
        D_ea92[0][i] = 0;
    for (i = 0; i < party_size; i++) {
        if (party[i].status == 1) {
            strcpy(namef, "\x81");
            strcat(namef, party[i].name);
            jj = strlen(namef);
            namef[jj] = 0xff;
            namef[jj + 1] = 0;
            strcat(namef, "\n");
            wb = text_pixel_width(win, party[i].name);
            if (wb > maxwe)
                maxwe = wb;
        }
        linem[0] = 0;
        anyk = 0;
        for (jj = 0; jj < 4; jj++) {
            if (party[i].status == 0 || party[i].status == 5) {
                strcat(linem, "\x15\n");
            } else if (kind == 2 && skj[jj] != -1 &&
                       ((rn = member_bit(i, skj[jj])) != 0 ||
                        (game_difficulty == 0 && leader == i) || D_5204 == 1)) {
                anyk = 1;
                strcpy(sn, "\x15 ");
                if (game_difficulty == 0 && leader == i && rn == 0 && D_5204 == 0) {
                    strcat(sn, "\x87");
                    D_ea92[i][jj] = 2;
                } else {
                    if (pic_buf[0] == 'x')
                        strcat(sn, "\x8f");
                    else
                        strcat(sn, "\x80");
                    D_ea92[i][jj] = 1;
                }
                _fstrcat(sn, F_2e38_3bc4[skj[jj]]);
                strcat(sn, "\n");
                wb = text_pixel_width(win, sn);
                if (wb > maxwe)
                    maxwe = wb;
                strcat(linem, sn);
            } else if (kind == 1 && skj[jj] != -1 &&
                       ((rn = has_item(i, skj[jj])) != 0 ||
                        (game_difficulty == 0 && leader == i) || D_5204 == 1)) {
                anyk = 1;
                strcpy(sn, "\x15 ");
                if (game_difficulty == 0 && leader == i && rn == 0 && D_5204 == 0) {
                    strcat(sn, "\x87");
                    D_ea92[i][jj] = 2;
                } else if (game_difficulty != 2 && rn == 2 && D_5204 == 0) {
                    strcat(sn, "\x84");
                    D_ea92[i][jj] = 2;
                } else if (rn == 1 || D_5204 != 0) {
                    if (pic_buf[0] == 'x')
                        strcat(sn, "\x8f");
                    else
                        strcat(sn, "\x80");
                    D_ea92[i][jj] = 1;
                } else {
                    strcat(sn, "\x83");
                    D_ea92[i][jj] = 0;
                }
                _fstrcat(sn, item_defs[skj[jj]]);
                if (rn == 1) {
                    strcat(sn, " (");
                    itoa_commas(rn = count_items(i, skj[jj]), numf);
                    strcat(sn, numf);
                    strcat(sn, ")");
                }
                strcat(sn, "\n");
                wb = text_pixel_width(win, sn);
                if (wb > maxwe)
                    maxwe = wb;
                strcat(linem, sn);
            } else {
                strcat(linem, "\x15\n");
            }
        }
        if (anyk == 1) {
            strcat(msg_buf, namef);
            strcat(msg_buf, linem);
            strcat(msg_buf, "\x14");
        } else {
            strcat(msg_buf, linem);
        }
    }
    if (msg_buf[0])
        msg_buf[strlen(msg_buf) - 2] = 0;
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

int far wait_continue(void)
{
    int v_si;
    D_8ddc = sub_root_047c_1320(6, 0, 0, 0);
    flush_keys();
    for (;;) {
        menu_bar_run(D_9419, 0);
        if (party_icons_dirty) {
            party_icons_dirty = 0;
            party_panel_frames(0x100);
            party_panel_rows(0x100);
            sub_s043_150b_13f2();
        }
        if (mouse_buttons & 1 && !sub_s043_150b_13a0()) {
            break;
        }
        if (key_pressed()) {
            v_si = get_key();
            if (v_si == 0x1c0d || v_si == 0x3920 || v_si == 0x1000) {
                break;
            }
            card_global_keys(v_si);
        }
        exit_if_requested();
    }
    switch (v_si) {
    case 0x1000:
        quit_to_dos();
        break;
    case 0x3920:
        v_si = 0x1c0d;
    }
    do {
    } while (mouse_buttons & 1 && D_9c4c);
    return v_si;
}

void far sub_s043_150b_28e6(int p_6)
{
    int v_si;
    v_si = p_6;
    gfx_draw_pic(v_si, 7, 0, D_a61e);
    gfx_draw_pic(v_si, 0xfc, 0, D_a624);
    gfx_draw_pic(v_si, 0, 0, D_a622);
    gfx_draw_pic(v_si, 7, 0xc2, D_a620);
}

/* global keys while a card waits for input: F1..F5 show party member 0..4's sheet
 * (member_sheet_vec), F6 = party_info_screen_vec (then the card is redrawn, 13f2), Shift-F1..F5 (0x5e00..) make
 * that able member the leader (leader). */
void far card_global_keys(register unsigned key)
{
    switch (key) {
    case 0x3b00:
    case 0x3c00:
    case 0x3d00:
    case 0x3e00:
    case 0x3f00:
        key = (key >> 8) - 0x3b;
        if (key < party_size && party[key].status) {
            member_sheet_vec(key);
            party_panel_frames(0x100);
            party_panel_rows(0x100);
            sub_s043_150b_13f2();
        }
        break;
    case 0x4000:
        party_info_screen_vec();
        party_icons_dirty = 0;
        party_panel_frames(0x100);
        party_panel_rows(0x100);
        sub_s043_150b_13f2();
        break;
    case 0x5e00:
    case 0x5f00:
    case 0x6000:
    case 0x6100:
    case 0x6200:
        key = (key >> 8) - 0x5e;
        if (key < party_size && party[key].status == 1) {
            leader = key;
            party_panel_rows(0x100);
        }
        break;
    }
}

/* let `hours` pass with the hourglass animation (D_a62a frame, 2d3a): every 3 hours one
 * step (the bell name bell_names[s029 0744(frame * 3)], day and month are shown), a day
 * every 8 steps (days_per_month = days per month).  A key or click skips the animation. */
void far pass_hours_hourglass(int hours)
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
    while (mouse_buttons & 1)
        ;
    hj = cur_hour / 3;
    daye = cur_day;
    monthk = cur_month;
    card_window->color = 14;
    mouse_hide();
    gfx_recolor_rect(card_window, 0x3c, 0x32, 0x78, 0x66, 0, 5);
    sub_root_047c_0545();
    savee = gfx_save_rect(0, 0x78, 0x32, 0x78, 0x66);
    sub_root_047c_04f8();
    gfx_draw_pic(card_window, 0x3c, 0x32, D_a62a);
    for (il = 0; il < hours; il++) {
        if (hj >= 8) {
            if (++daye > days_per_month[monthk]) {
                daye -= days_per_month[monthk];
                if (++monthk > 11)
                    monthk = 0;
            }
            hj -= 8;
        }
        framee = hj % 8;
        if (!key_pressed() && !(mouse_buttons & 1)) {
            hourglass_frame(framee);
            gfx_fill_rect(card_window, 0x48, 0x82, 0x28, 8, 0);
            gfx_fill_rect(card_window, 0x84, 0x82, 0xc, 8, 0);
            gfx_fill_rect(card_window, 0xa0, 0x82, 0xc, 8, 0);
            draw_text_far(card_window, 0x48, 0x82, bell_names[hour_band(framee * 3)]);
            draw_text_far(card_window, 0x84, 0x82, (char far *)itoa(daye, numf, 10));
            draw_text_far(card_window, 0xa0, 0x82, (char far *)itoa(monthk + 1, numf, 10));
            vec_1fef(400, 0);
        }
        hj++;
    }
    hourglass_frame(hj = hj % 8);
    gfx_fill_rect(card_window, 0x48, 0x82, 0x28, 8, 0);
    gfx_fill_rect(card_window, 0x84, 0x82, 0xc, 8, 0);
    gfx_fill_rect(card_window, 0xa0, 0x82, 0xc, 8, 0);
    draw_text_far(card_window, 0x48, 0x82, bell_names[hour_band(hj * 3)]);
    draw_text_far(card_window, 0x84, 0x82, (char far *)itoa(daye, numf, 10));
    draw_text_far(card_window, 0xa0, 0x82, (char far *)itoa(monthk + 1, numf, 10));
    if (!key_pressed() && !(mouse_buttons & 1))
        vec_1fef(1000, 0);
    if (key_pressed())
        get_key();
    gfx_draw_pic(card_window, 0x3c, 0x32, savee);
    gfx_recolor_rect(card_window, 0x3c, 0x32, 0x78, 0x66, 5, 0);
    mouse_show();
    if (savee)
        free_buffer(savee, "b");
    card_window->color = 8;
}

/* draw frame k of the hourglass (timer pictures D_a62e/D_a630/D_a632, mirrored) */
void far hourglass_frame(int k)
{
    switch (k) {
    case 0:
        sub_root_047c_055f(card_window, 0x6c, 0x53, 0x18, -0x14, D_a62e);
        break;
    case 1:
        sub_root_047c_055f(card_window, 0x6c, 0x53, -0x18, -0x14, D_a630);
        break;
    case 2:
        sub_root_047c_055f(card_window, 0x6c, 0x53, -0x18, 0x14, D_a632);
        break;
    case 3:
        sub_root_047c_055f(card_window, 0x6c, 0x53, -0x18, 0x14, D_a630);
        break;
    case 4:
        sub_root_047c_055f(card_window, 0x6c, 0x53, 0x18, 0x14, D_a62e);
        break;
    case 5:
        sub_root_047c_055f(card_window, 0x6c, 0x53, 0x18, 0x14, D_a630);
        break;
    case 6:
        sub_root_047c_055f(card_window, 0x6c, 0x53, 0x18, 0x14, D_a632);
        break;
    case 7:
        sub_root_047c_055f(card_window, 0x6c, 0x53, 0x18, -0x14, D_a630);
        break;
    }
}
