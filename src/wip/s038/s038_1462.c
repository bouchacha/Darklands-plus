/* place: s038 1462:0000 */
/* orig: size=26be CONST=7ed2+0004 _DATA=4a06+0154 */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* status: 5/7 match (0292 23ec 25ca 01fe 024e; _DATA complete once 02f8's literals are in).
   02f8 (8.4 KB) written in full, 8428/8436 bytes, frame/slots match (hash-ordered local names,
   fillers x1g/x2h/x3l), all ten colour jump tables match. Left (6.00 vs 6.00A, see e4 NOTES):
     - 'A' case: 6.00 placed the `if (392c) {..}` block of the party-search loop BEFORE the
       loop increment (`jmp 097f` / `jmp 08fa`); 6.00A emits it inline (goto version: no help);
     - i*2 kept in DI across calls (`mov di,bx` before _dos_freemem, push di/pop di around the
       inline strcpy/strcat in 'H'); pragma "a" gives the DI CSE but breaks the colour tables;
     - cross-jump direction: 6.00 keeps the 'A' no-free-slot copy of 0553(..)+1293() and 'H'/'S'
       jump back into it (`jmp 09c3`), 6.00A keeps the later copy; 'C' then jumps to `call 1293`;
     - `if (D_a67e <= i) D_a67e = i + 1;` operand order (`mov ax,i; cmp [a67e],ax; jg`); struct
       wrapper and declpos search did not help.
   0000: no separate copy var - `key` itself is used in the tail (6.00A splits it: memory in
   the mouse part, SI from `mov si,[bp-4]` on) + #pragma optimize("a"): registers now right
   (row DI). Left: the original has an unused 2-byte slot at [bp-2] above key ([bp-4]);
   unused/dead locals get no slot in 6.00A, a dead struct (`r.a = f(); return r.a;`) gets
   one but below the scalars (tried 16 names, both positions).
   m0: `int km[2]` for the mouse-part key (km[0], km[1] = the hole) + `key = km[0];` after
   0000(9419) gives enter 4 and the right slots, but then row and key share SI (no DI);
   register/declaration order/comma placement of the copy: no change.
   Character creation main menu (pics\cretscr3.pic): list of saved characters from charactr.tmp.

   charactr.tmp layout: int count; int ids[5] (party slot -> character index, -1 = empty);
   char shield[5][4] (D_a668, "F01"/"F60"/"C00"/"A00" + 0); then count records of 0x22a bytes:
   0x80-byte member record (read to D_9c55 + slot*0x80, i.e. D_9c69[slot] - 0x14) + far tables
   F_2e38_082c[0x14], F_2e38_07be[0x16], F_2e38_003e[0x180] (cf. D_8d2e / D_9085 in 23ec).
   02f8 keys: up/left/right & down = previous/next character (portrait palette from
   F_20a5_33fc[slot], 24 bytes = 8 RGB triples, copied to D_e753..D_e76a); R = leave;
   A = add/remove party member (sub_s029_0e76_392c/3cde/2a32 with 0x4c); C = new character
   (vec_20cb); H = next shield letter (D_9c69[slot][1] 'A'..'O', pics\shieldX.pic);
   S = next shield code (D_e7e2[D_e7e0], D_e7de codes); 1/2/3 = cycle the colours of shield
   parts (D_e890/D_e892/D_e894, palette bytes depend on the shield code); Alt-Q = quit game. */
#include "darklands/card.h"
#include "darklands/game.h"

struct res { int a; int b; };
extern int cur_loc;   /* position matters: compare direction in 23ec */
struct font { char _p0[0xc]; int color; };
/* character record as stored in charactr.tmp (0x80 bytes, cf. party member records party) */
struct chr { int id; char _p2[0x12]; char status; char _p15[0x10]; char name[0x5b]; };
extern struct chr D_8d2e;
extern int D_8d06[];
extern char msg_buf[];
extern char member_shield[][4];      /* shield code per party slot ("F01", ...) */
extern char party[][0x80];
extern struct { char _p[0x1ddf]; char far *s[0x41]; } far F_290e_0000;
int far party_panel_frames();
int far party_panel_rows();
int far gfx_text();
int far _dos_close();
extern char screen_win[];
extern int screen_win_font;
extern int back_win_colour;
extern int back_win_font;
extern volatile int mouse_buttons, mouse_x, mouse_y;
extern char D_9419[];
extern char D_9920;
extern char D_9934;
extern int party_icons_dirty;
extern char cur_palette[];
extern int D_e7a0;
extern int D_e7a2;
extern int D_e7da;
extern int D_e7dc;
extern int D_e88a;
extern int D_e88e;
typedef struct { char _p[16]; } FILE;
extern char D_e753, D_e754, D_e755, D_e756, D_e757, D_e758, D_e759, D_e75a, D_e75b, D_e75c,
    D_e75d, D_e75e, D_e75f, D_e760, D_e761, D_e762, D_e763, D_e764, D_e765, D_e766, D_e767,
    D_e768, D_e769, D_e76a;     /* current portrait palette (24 bytes) */
extern char D_e7e2[][4];        /* shield codes */
extern int D_e7de, D_e7e0;      /* number of shield codes, current one */
extern int D_e890, D_e892, D_e894;  /* colour indices for keys 1/2/3 */
extern int screen_win_colour;
extern unsigned shield_pics[];
extern char combatants[];
extern struct { int v; } party_size;   /* number of party members (struct: compare operand order) */
extern char far member_palette[5][0x18];   /* palette per party slot */
extern char far saints_known[][0x14];
extern char far formulas_known[][0x16];
extern char far inventories[][0x180];
FILE *far fdopen();
int far fread();
int far fclose();
int far fseek();
int far _dos_write();
int memcmp();
char *strcpy();
char *strcat();
int far quit_to_dos();
int far event_here_type();
int far event_remove();
int far equip_weight_vec();
int far create_character_vec();
int far _dos_freemem();
int far _dos_read();
int far menu_bar_run();
int far draw_text_far();
int far gfx_recolor_rect();
int far sub_root_047c_04f8();
int far sub_root_047c_0545();
int far gfx_copy_rect();
int far sub_root_047c_0568();
int far sub_root_047c_056f();
int far set_palette();
int far gfx_draw_pic();
int far key_pressed();
int far get_key();
int far mouse_show();
int far mouse_hide();
int far sub_root_047c_1320();
int far mem_free();
int far pic_draw_pal();
int far pic_load();
int far sub_root_06a1_2c9e();
int far event_type_at();
int far slot_of_char_vec();
int far vec_20c1();
int far sub_s038_1462_0000();
int far sub_s038_1462_01fe();
int far sub_s038_1462_024e();
void far sub_s038_1462_0292();
int far sub_s038_1462_02f8();
void far sub_s038_1462_23ec(struct font *);
void far sub_s038_1462_25ca(struct font *);

#pragma optimize("a", on)
int far sub_s038_1462_0000(void)
{
    int key;
    int row;
    int k;      /* unused: keeps the symbol table (compare direction in 23ec) */
    sub_root_047c_0568();
    D_9920 = 2;
    D_9934 = 2;
    D_e7a2 = back_win_font;
    D_e7a0 = screen_win_font;
    D_e88e = 0;
    vec_20c1();
    sub_s038_1462_024e();
    sub_s038_1462_0292();
    sub_root_047c_056f();
    while (mouse_buttons & 1)
        ;
    do {
        key = 0;
        if ((mouse_buttons & 1) && mouse_x > 0x46 && mouse_x < 0xc8 && mouse_y > 0x2d && mouse_y < 0xa5) {
            while (mouse_buttons & 1)
                ;
            switch (row = (mouse_y - 0x2d) / 15) {
            case 0:
                key = 0x2e43;
                break;
            case 1:
                key = 0x1e41;
                break;
            case 2:
                key = 0x2348;
                break;
            case 3:
                key = 0x1f53;
                break;
            case 4:
                key = 0x231;
                break;
            case 5:
                key = 0x332;
                break;
            case 6:
                key = 0x433;
                break;
            case 7:
                key = 0x11b;
                break;
            }
        } else if ((mouse_buttons & 1) && mouse_x > 0xc5 && mouse_x < 0x138 && mouse_y > 0x14 && mouse_y < 0xb4) {
            while (mouse_buttons & 1)
                ;
            row = (mouse_y - 0x14) / 8;
            if (D_e7dc > row) {
                mouse_hide();
                gfx_recolor_rect(screen_win, 0xc6, (D_e7da << 3) + 0x16, 0x71, 8, 0x12, 0x10);
                D_e7da = row;
                gfx_recolor_rect(screen_win, 0xc6, (row << 3) + 0x16, 0x71, 8, 0x10, 0x12);
                mouse_show();
            }
        }
        menu_bar_run(D_9419, 0);
        if (party_icons_dirty) {
            party_icons_dirty = 0;
            sub_root_047c_0568();
            sub_s038_1462_024e();
            sub_s038_1462_0292();
            sub_root_047c_056f();
            key = 0;
        }
        if (key_pressed())
            key = get_key();
            if (key == 0x11b || key == 0x1352 || key == 0x1372)
            return sub_s038_1462_01fe(0);
    } while (!key || sub_s038_1462_02f8(key) != 0x11b);
    return sub_s038_1462_01fe(1);
}
#pragma optimize("", on)

int far sub_s038_1462_01fe(int p_6)
{
    if (D_e88e) {
        _dos_freemem(D_e88e);
    }
    mem_free(D_e88a, "b");
    back_win_font = D_e7a2;
    screen_win_font = D_e7a0;
    D_9920 = 1;
    D_9934 = 1;
    do {
    } while (mouse_buttons & 1);
    sub_root_047c_1320(6, 0, 0, 0);
    return p_6;
}

int far sub_s038_1462_024e(void)
{
    pic_draw_pal(1, "pics\\cretscr3.pic", cur_palette);
    sub_root_047c_0545();
    D_e88a = pic_load("pics\\buttnCR2.pic");
    sub_root_047c_04f8();
    set_palette(cur_palette);
    back_win_colour = 8;
    back_win_font = 1;
    return screen_win_font = 1;
}

void far sub_s038_1462_0292(void)
{
    int i;
    sub_s038_1462_25ca((struct font *)0x2806);
    sub_s038_1462_23ec((struct font *)0x2806);
    mouse_hide();
    gfx_copy_rect(0x2806, 0, 0, 0x140, 0xc8, screen_win, 0, 0);
    for (i = 0; i < 5; i++) {
        if (party[i][0]) {
            party_panel_frames(i);
            party_panel_rows(i);
        }
    }
    mouse_show();
}

/* character-creation menu commands (key code from 0000); returns 0x11b to leave the menu */
int far sub_s038_1462_02f8(register int key)
{
    int x1g;
    int fhc;
    long x2h;
    FILE *fpm;
    int nrd;
    int il;
    int hn;
    int x3l;
    int idsh[5];
    char nameh[20];
    switch (key) {
    case 0x4800: case 0x4b00: case 0x4d00:
        gfx_recolor_rect(screen_win, 0xc6, (D_e7da << 3) + 0x16, 0x71, hn = 8, 0x12, 0x10);
        if (--D_e7da < 0)
            D_e7da = D_e7dc - 1;
        gfx_recolor_rect(screen_win, 0xc6, hn * D_e7da + 0x16, 0x71, hn, 0x10, 0x12);
        gfx_copy_rect(0x2806, 0x46, 0xa5, 0x23, 0x23, screen_win, 0x46, 0xa5);
        sub_root_06a1_2c9e("charactr.tmp", 2, &fhc);
        fpm = fdopen(fhc, "r+b");
        _dos_read(fhc, (int far *)&il, 2, &nrd);
        _dos_read(fhc, (int far *)idsh, 10, &nrd);
        _dos_read(fhc, (char far *)member_shield, 0x14, &nrd);
        fclose(fpm);
        for (il = 0; il < 4; il++)
            if (idsh[il] == D_e7da)
                break;
        if (il < 4) {
            if (D_e88e)
                _dos_freemem(D_e88e);
            sub_root_047c_0545();
            strcpy(nameh, "pics\\");
            strcat(nameh, D_e7e2[D_e7da]);
            strcat(nameh, "small.pic");
            D_e88e = pic_load(nameh);
            sub_root_047c_04f8();
            D_e753 = member_palette[il][0];
            D_e754 = member_palette[il][1];
            D_e755 = member_palette[il][2];
            D_e756 = member_palette[il][3];
            D_e757 = member_palette[il][4];
            D_e758 = member_palette[il][5];
            D_e759 = member_palette[il][6];
            D_e75a = member_palette[il][7];
            D_e75b = member_palette[il][8];
            D_e75c = member_palette[il][9];
            D_e75d = member_palette[il][10];
            D_e75e = member_palette[il][11];
            D_e75f = member_palette[il][12];
            D_e760 = member_palette[il][13];
            D_e761 = member_palette[il][14];
            D_e762 = member_palette[il][15];
            D_e763 = member_palette[il][16];
            D_e764 = member_palette[il][17];
            D_e765 = member_palette[il][18];
            D_e766 = member_palette[il][19];
            D_e767 = member_palette[il][20];
            D_e768 = member_palette[il][21];
            D_e769 = member_palette[il][22];
            D_e76a = member_palette[il][23];
            set_palette(cur_palette);
            gfx_draw_pic(screen_win, 0xc, 0xa4, D_e88e);
        }
        break;
    case 0x5000:
        gfx_recolor_rect(screen_win, 0xc6, (D_e7da << 3) + 0x16, 0x71, hn = 8, 0x12, 0x10);
        if (++D_e7da >= D_e7dc)
            D_e7da = 0;
        gfx_recolor_rect(screen_win, 0xc6, hn * D_e7da + 0x16, 0x71, hn, 0x10, 0x12);
        gfx_copy_rect(0x2806, 0x46, 0xa5, 0x23, 0x23, screen_win, 0x46, 0xa5);
        sub_root_06a1_2c9e("charactr.tmp", 2, &fhc);
        fpm = fdopen(fhc, "r+b");
        _dos_read(fhc, (int far *)&il, 2, &nrd);
        _dos_read(fhc, (int far *)idsh, 10, &nrd);
        _dos_read(fhc, (char far *)member_shield, 0x14, &nrd);
        fclose(fpm);
        for (il = 0; il < 4; il++)
            if (idsh[il] == D_e7da)
                break;
        if (il < 4) {
            if (D_e88e)
                _dos_freemem(D_e88e);
            sub_root_047c_0545();
            strcpy(nameh, "pics\\");
            strcat(nameh, D_e7e2[D_e7da]);
            strcat(nameh, "small.pic");
            D_e88e = pic_load(nameh);
            sub_root_047c_04f8();
            D_e753 = member_palette[il][0];
            D_e754 = member_palette[il][1];
            D_e755 = member_palette[il][2];
            D_e756 = member_palette[il][3];
            D_e757 = member_palette[il][4];
            D_e758 = member_palette[il][5];
            D_e759 = member_palette[il][6];
            D_e75a = member_palette[il][7];
            D_e75b = member_palette[il][8];
            D_e75c = member_palette[il][9];
            D_e75d = member_palette[il][10];
            D_e75e = member_palette[il][11];
            D_e75f = member_palette[il][12];
            D_e760 = member_palette[il][13];
            D_e761 = member_palette[il][14];
            D_e762 = member_palette[il][15];
            D_e763 = member_palette[il][16];
            D_e764 = member_palette[il][17];
            D_e765 = member_palette[il][18];
            D_e766 = member_palette[il][19];
            D_e767 = member_palette[il][20];
            D_e768 = member_palette[il][21];
            D_e769 = member_palette[il][22];
            D_e76a = member_palette[il][23];
            set_palette(cur_palette);
            gfx_draw_pic(screen_win, 0x46, 0xa5, D_e88e);
        }
        break;
    case 0x1352: case 0x1372:
        return 0x11b;
    case 0x1e41: case 0x1e61:
        if (D_e7dc > 0) {
            mouse_hide();
            gfx_draw_pic(screen_win, 0x43, 0x3b, D_e88a);
            screen_win_colour = 8;
            draw_text_far(screen_win, 0x4a, 0x3f, F_290e_0000.s[0]);
            sub_root_06a1_2c9e("charactr.tmp", 2, &fhc);
            fpm = fdopen(fhc, "r+b");
            _dos_read(fhc, (int far *)&il, 2, &nrd);
            _dos_read(fhc, (int far *)idsh, 10, &nrd);
            _dos_read(fhc, (char far *)member_shield, 0x14, &nrd);
            for (il = 0; il < 4; il++)
                if (idsh[il] == D_8d06[D_e7da]) {
                    if (event_type_at(0x4c, D_8d06[D_e7da], cur_loc)) {
                        event_remove(0x4c, D_8d06[D_e7da], cur_loc, -1, -1);
                        _dos_freemem(shield_pics[il]);
                        shield_pics[il] = 0;
                        idsh[il] = -1;
                        party[il][0] = 0;
                        party_panel_rows(il);
                    }
                    break;
                }
            if (il >= 4) {
                for (il = 0; il < 4; il++)
                    if (idsh[il] == -1)
                        break;
                if (il >= 4) {
                    fclose(fpm);
                    gfx_copy_rect(0x2806, 0x43, 0x3b, 0x82, 0xe, screen_win, 0x43, 0x3b);
                    mouse_show();
                    break;
                }
                event_here_type(0x4c, D_8d06[D_e7da], 0xa8);
                idsh[il] = D_8d06[D_e7da];
                strcpy(member_shield[il], D_e7e2[0]);
                if (D_e88e)
                    _dos_freemem(D_e88e);
                sub_root_047c_0545();
                strcpy(nameh, "pics\\shielda.pic");
                shield_pics[il] = pic_load(nameh);
                strcpy(nameh, "pics\\");
                strcat(nameh, D_e7e2[0]);
                strcat(nameh, "small.pic");
                D_e88e = pic_load(nameh);
                sub_root_047c_04f8();
                D_e753 = 0x39;
                member_palette[il][0] = 0x39;
                D_e754 = 0x36;
                member_palette[il][1] = 0x36;
                D_e755 = 0;
                D_e759 = 0;
                D_e75c = 0;
                D_e75f = 0;
                D_e763 = 0;
                D_e764 = 0;
                D_e766 = 0;
                D_e767 = 0;
                D_e769 = 0;
                D_e76a = 0;
                member_palette[il][2] = 0;
                D_e756 = 0x3d;
                member_palette[il][3] = 0x3d;
                D_e757 = 0x38;
                member_palette[il][4] = 0x38;
                D_e758 = 0x15;
                D_e760 = 0x15;
                member_palette[il][5] = 0x15;
                member_palette[il][6] = 0;
                D_e75a = 0x18;
                member_palette[il][7] = 0x18;
                D_e75b = 0x28;
                member_palette[il][8] = 0x28;
                member_palette[il][9] = 0;
                D_e75d = 0x11;
                D_e768 = 0x11;
                member_palette[il][10] = 0x11;
                D_e75e = 0x25;
                D_e761 = 0x25;
                member_palette[il][11] = 0x25;
                member_palette[il][12] = 0;
                member_palette[il][13] = 0x15;
                member_palette[il][14] = 0x25;
                D_e762 = 0x1c;
                member_palette[il][15] = 0x1c;
                member_palette[il][16] = 0;
                member_palette[il][17] = 0;
                D_e765 = 0x17;
                member_palette[il][18] = 0x17;
                member_palette[il][19] = 0;
                member_palette[il][20] = 0;
                member_palette[il][21] = 0x11;
                member_palette[il][22] = 0;
                member_palette[il][23] = 0;
                set_palette(cur_palette);
                gfx_copy_rect(0x2806, 0x46, 0xa5, 0x23, 0x24, screen_win, 0x46, 0xa5);
                gfx_draw_pic(screen_win, 0x46, 0xa5, D_e88e);
                fseek(fpm, (long)(unsigned)(D_8d06[D_e7da] * 0x22a), 1);
                fread(&combatants[il << 7], 0x80, 1, fpm);
                fseek(fpm, 0L, 1);
                _dos_read(fhc, (char far *)saints_known[il], 0x14, &nrd);
                _dos_read(fhc, (char far *)formulas_known[il], 0x16, &nrd);
                _dos_read(fhc, (char far *)inventories[il], 0x180, &nrd);
                if (!party[il][0])
                    party[il][0] = 1;
                party[il][0x3d] = -1;
                party[il][0x44] = 0;
                party[il][0x0e] = -1;
                party[il][0x46] = 0;
                party[il][0x37] = -1;
                party[il][0x38] = -1;
                party[il][0x3b] = 0;
                party[il][0x3c] = 0;
                party[il][0x39] = 0;
                party[il][0x3a] = 0;
                party[il][0x48] = -1;
                party[il][0x47] = 0;
                party[il][0x6a] = 0;
                if (party_size.v <= il)
                    party_size.v = il + 1;
                equip_weight_vec(il);
                party_panel_frames(il);
                party_panel_rows(il);
            }
            fseek(fpm, 2L, 0);
            _dos_write(fhc, (int far *)idsh, 10, &nrd);
            _dos_write(fhc, (char far *)member_shield, 0x14, &nrd);
            fclose(fpm);
            mouse_show();
            mouse_hide();
            gfx_copy_rect(0x2806, 0x43, 0x3b, 0x82, 0xe, screen_win, 0x43, 0x3b);
            mouse_show();
            sub_s038_1462_23ec((struct font *)screen_win);
        }
        break;
    case 0x2e43: case 0x2e63:
        mouse_hide();
        gfx_draw_pic(screen_win, 0x43, 0x2c, D_e88a);
        screen_win_colour = 8;
        draw_text_far(screen_win, 0x4a, 0x31, F_290e_0000.s[1]);
        create_character_vec();
        if (D_e88a)
            mem_free(D_e88a, "b");
        sub_s038_1462_024e();
        sub_s038_1462_0292();
        mouse_show();
        break;
    case 0x2348: case 0x2368:
        if (D_e7dc > 0) {
            mouse_hide();
            gfx_draw_pic(screen_win, 0x43, 0x4a, D_e88a);
            screen_win_colour = 8;
            draw_text_far(screen_win, 0x4a, 0x4e, F_290e_0000.s[0x40]);
            sub_root_06a1_2c9e("charactr.tmp", 2, &fhc);
            fpm = fdopen(fhc, "r+b");
            _dos_read(fhc, (int far *)&il, 2, &nrd);
            _dos_read(fhc, (int far *)idsh, 10, &nrd);
            _dos_read(fhc, (char far *)member_shield, 0x14, &nrd);
            for (il = 0; il < 5; il++)
                if (idsh[il] == D_8d06[D_e7da])
                    break;
            if (il >= 5)
                fclose(fpm);
            else {
                if (++party[il][1] >= 'P')
                    party[il][1] = 'A';
                _dos_freemem(shield_pics[il]);
                sub_root_047c_0545();
                strcpy(nameh, "pics\\");
                strcat(nameh, "shield");
                nameh[11] = party[il][1];
                nameh[12] = 0;
                strcat(nameh, ".pic");
                shield_pics[il] = pic_load(nameh);
                sub_root_047c_04f8();
                party_panel_rows(il);
                fclose(fpm);
                mouse_show();
                mouse_hide();
            }
            gfx_copy_rect(0x2806, 0x43, 0x4a, 0x82, 0xe, screen_win, 0x43, 0x4a);
            mouse_show();
        }
        break;
    case 0x1f53: case 0x1f73:
        if (D_e7dc > 0) {
            mouse_hide();
            gfx_draw_pic(screen_win, 0x43, 0x59, D_e88a);
            screen_win_colour = 8;
            draw_text_far(screen_win, 0x4a, 0x5d, F_290e_0000.s[0x1a]);
            sub_root_06a1_2c9e("charactr.tmp", 2, &fhc);
            fpm = fdopen(fhc, "r+b");
            _dos_read(fhc, (int far *)&il, 2, &nrd);
            _dos_read(fhc, (int far *)idsh, 10, &nrd);
            _dos_read(fhc, (char far *)member_shield, 0x14, &nrd);
            for (il = 0; il < 5; il++)
                if (idsh[il] == D_8d06[D_e7da])
                    break;
            if (il >= 5)
                fclose(fpm);
            else {
                if (++D_e7e0 >= D_e7de)
                    D_e7e0 = 0;
                _dos_freemem(D_e88e);
                strcpy(member_shield[il], D_e7e2[D_e7e0]);
                sub_root_047c_0545();
                strcpy(nameh, "pics\\");
                strcat(nameh, D_e7e2[D_e7e0]);
                strcat(nameh, "small.pic");
                D_e88e = pic_load(nameh);
                sub_root_047c_04f8();
                gfx_copy_rect(0x2806, 0x46, 0xa5, 0x23, 0x24, screen_win, 0x46, 0xa5);
                gfx_draw_pic(screen_win, 0x46, 0xa5, D_e88e);
                fseek(fpm, -20L, 1);
                _dos_write(fhc, (char far *)member_shield, 0x14, &nrd);
                fclose(fpm);
                mouse_show();
                mouse_hide();
            }
            gfx_copy_rect(0x2806, 0x43, 0x59, 0x82, 0xe, screen_win, 0x43, 0x59);
            mouse_show();
        }
        break;
    case 0x231: case 0x4f31:
        if (D_e7dc > 0) {
            sub_root_06a1_2c9e("charactr.tmp", 2, &fhc);
            fpm = fdopen(fhc, "r+b");
            _dos_read(fhc, (int far *)&il, 2, &nrd);
            _dos_read(fhc, (int far *)idsh, 10, &nrd);
            _dos_read(fhc, (char far *)member_shield, 0x14, &nrd);
            fclose(fpm);
            for (il = 0; il < 5; il++)
                if (idsh[il] == D_e7da)
                    break;
            if (il < 5) {
                if (!memcmp(member_shield[il], "F01", 4)) {
                    if (++D_e890 >= 6)
                        D_e890 = 0;
                    switch (D_e890) {
                    case 0:
                        D_e753 = 0x39;
                        D_e754 = 0x36;
                        D_e755 = 0x0;
                        D_e756 = 0x3d;
                        D_e757 = 0x38;
                        D_e758 = 0x15;
                        break;
                    case 1:
                        D_e754 = 0x1c;
                        D_e755 = 0x1c;
                        D_e756 = 0x1c;
                        D_e753 = 0x1a;
                        D_e757 = 0x1a;
                        D_e758 = 0x1a;
                        break;
                    case 2:
                        D_e753 = 0x0;
                        D_e754 = 0x0;
                        D_e755 = 0x0;
                        D_e756 = 0x0;
                        D_e757 = 0x0;
                        D_e758 = 0x0;
                        break;
                    case 3:
                        D_e754 = 0x9;
                        D_e753 = 0x23;
                        D_e756 = 0x23;
                        D_e757 = 0xc;
                        D_e755 = 0x0;
                        D_e758 = 0x0;
                        break;
                    case 4:
                        D_e753 = 0x23;
                        D_e755 = 0x9;
                        D_e754 = 0x21;
                        D_e756 = 0x21;
                        D_e757 = 0x1a;
                        D_e758 = 0x0;
                        break;
                    case 5:
                        D_e753 = 0x21;
                        D_e754 = 0x15;
                        D_e756 = 0x19;
                        D_e755 = 0x12;
                        D_e757 = 0x12;
                        D_e758 = 0xe;
                        break;
                    }
                    member_palette[il][0] = D_e753;
                    member_palette[il][1] = D_e754;
                    member_palette[il][2] = D_e755;
                    member_palette[il][3] = D_e756;
                    member_palette[il][4] = D_e757;
                    member_palette[il][5] = D_e758;
                } else if (!memcmp(member_shield[il], "F60", 4)) {
                    if (++D_e890 >= 6)
                        D_e890 = 0;
                    switch (D_e890) {
                    case 0:
                        D_e762 = 0x21;
                        D_e765 = 0x28;
                        D_e768 = 0x2d;
                        D_e763 = 0x0;
                        D_e764 = 0x0;
                        D_e766 = 0x0;
                        D_e767 = 0x0;
                        D_e769 = 0x0;
                        D_e76a = 0x0;
                        break;
                    case 1:
                        D_e762 = 0x2f;
                        D_e763 = 0x2a;
                        D_e764 = 0x12;
                        D_e765 = 0x37;
                        D_e766 = 0x32;
                        D_e767 = 0x1a;
                        D_e768 = 0x3e;
                        D_e769 = 0x3c;
                        D_e76a = 0x25;
                        break;
                    case 2:
                        D_e762 = 0x2d;
                        D_e763 = 0x1e;
                        D_e764 = 0x0;
                        D_e765 = 0x35;
                        D_e766 = 0x28;
                        D_e767 = 0x8;
                        D_e768 = 0x3e;
                        D_e769 = 0x33;
                        D_e76a = 0x12;
                        break;
                    case 3:
                        D_e762 = 0x1c;
                        D_e763 = 0xd;
                        D_e765 = 0x25;
                        D_e766 = 0x11;
                        D_e768 = 0x2d;
                        D_e769 = 0x15;
                        D_e764 = 0x0;
                        D_e767 = 0x0;
                        D_e76a = 0x0;
                        break;
                    case 4:
                        D_e762 = 0x0;
                        D_e763 = 0x0;
                        D_e764 = 0x0;
                        D_e765 = 0xa;
                        D_e766 = 0xa;
                        D_e767 = 0xa;
                        D_e768 = 0x12;
                        D_e769 = 0x12;
                        D_e76a = 0x12;
                        break;
                    case 5:
                        D_e762 = 0x15;
                        D_e763 = 0x9;
                        D_e765 = 0x1a;
                        D_e766 = 0xb;
                        D_e768 = 0x21;
                        D_e769 = 0xd;
                        D_e764 = 0x0;
                        D_e767 = 0x0;
                        D_e76a = 0x0;
                        break;
                    }
                    member_palette[il][15] = D_e762;
                    member_palette[il][16] = D_e763;
                    member_palette[il][17] = D_e764;
                    member_palette[il][18] = D_e765;
                    member_palette[il][19] = D_e766;
                    member_palette[il][20] = D_e767;
                    member_palette[il][21] = D_e768;
                    member_palette[il][22] = D_e769;
                    member_palette[il][23] = D_e76a;
                } else if (!memcmp(member_shield[il], "C00", 4)) {
                    if (++D_e890 >= 6)
                        D_e890 = 0;
                    switch (D_e890) {
                    case 0:
                        D_e756 = 0x1c;
                        D_e753 = 0x1c;
                        D_e757 = 0x13;
                        D_e754 = 0x13;
                        D_e758 = 0xa;
                        D_e755 = 0xa;
                        break;
                    case 1:
                        D_e756 = 0x23;
                        D_e757 = 0x23;
                        D_e758 = 0x23;
                        D_e753 = 0x23;
                        D_e754 = 0x23;
                        D_e755 = 0x23;
                        break;
                    case 2:
                        D_e756 = 0x0;
                        D_e757 = 0x0;
                        D_e758 = 0x0;
                        D_e753 = 0x0;
                        D_e754 = 0x0;
                        D_e755 = 0x0;
                        break;
                    case 3:
                        D_e756 = 0x1c;
                        D_e757 = 0x13;
                        D_e758 = 0xa;
                        D_e753 = 0x3e;
                        D_e754 = 0x28;
                        D_e755 = 0x19;
                        break;
                    case 4:
                        D_e756 = 0x23;
                        D_e757 = 0x23;
                        D_e758 = 0x23;
                        D_e753 = 0x3e;
                        D_e754 = 0x28;
                        D_e755 = 0x19;
                        break;
                    case 5:
                        D_e756 = 0x0;
                        D_e757 = 0x0;
                        D_e758 = 0x0;
                        D_e753 = 0x3e;
                        D_e754 = 0x28;
                        D_e755 = 0x19;
                        break;
                    }
                    member_palette[il][0] = D_e753;
                    member_palette[il][1] = D_e754;
                    member_palette[il][2] = D_e755;
                    member_palette[il][3] = D_e756;
                    member_palette[il][4] = D_e757;
                    member_palette[il][5] = D_e758;
                } else if (!memcmp(member_shield[il], "A00", 4)) {
                    if (++D_e890 >= 6)
                        D_e890 = 0;
                    switch (D_e890) {
                    case 0:
                        D_e753 = 0x28;
                        D_e756 = 0x21;
                        D_e757 = 0xc;
                        D_e755 = 0x19;
                        D_e759 = 0x19;
                        D_e75a = 0x7;
                        D_e754 = 0x12;
                        D_e758 = 0x12;
                        D_e75b = 0x12;
                        break;
                    case 1:
                        D_e754 = 0x20;
                        D_e755 = 0x33;
                        D_e757 = 0x1c;
                        D_e758 = 0x2d;
                        D_e753 = 0x0;
                        D_e756 = 0x0;
                        D_e759 = 0x0;
                        D_e75a = 0x18;
                        D_e75b = 0x28;
                        break;
                    case 2:
                        D_e753 = 0x1a;
                        D_e755 = 0x26;
                        D_e756 = 0x15;
                        D_e757 = 0x19;
                        D_e754 = 0x20;
                        D_e758 = 0x20;
                        D_e759 = 0x13;
                        D_e75a = 0x17;
                        D_e75b = 0x1e;
                        break;
                    case 3:
                        D_e753 = 0x2e;
                        D_e754 = 0x25;
                        D_e755 = 0x25;
                        D_e756 = 0x2a;
                        D_e757 = 0x21;
                        D_e758 = 0x21;
                        D_e759 = 0x23;
                        D_e75a = 0x19;
                        D_e75b = 0x19;
                        break;
                    case 4:
                        D_e753 = 0x33;
                        D_e754 = 0x2f;
                        D_e756 = 0x2e;
                        D_e758 = 0x19;
                        D_e755 = 0x25;
                        D_e757 = 0x25;
                        D_e759 = 0x25;
                        D_e75a = 0x1c;
                        D_e75b = 0x15;
                        break;
                    case 5:
                        D_e754 = 0x1e;
                        D_e755 = 0x15;
                        D_e757 = 0x15;
                        D_e758 = 0x12;
                        D_e753 = 0x0;
                        D_e756 = 0x0;
                        D_e759 = 0x0;
                        D_e75a = 0x10;
                        D_e75b = 0xb;
                        break;
                    }
                    member_palette[il][0] = D_e753;
                    member_palette[il][1] = D_e754;
                    member_palette[il][2] = D_e755;
                    member_palette[il][3] = D_e756;
                    member_palette[il][4] = D_e757;
                    member_palette[il][5] = D_e758;
                    member_palette[il][6] = D_e759;
                    member_palette[il][7] = D_e75a;
                    member_palette[il][8] = D_e75b;
                }
                set_palette(cur_palette);
            }
        }
        break;
    case 0x332: case 0x5032:
        if (D_e7dc > 0) {
            sub_root_06a1_2c9e("charactr.tmp", 2, &fhc);
            fpm = fdopen(fhc, "r+b");
            _dos_read(fhc, (int far *)&il, 2, &nrd);
            _dos_read(fhc, (int far *)idsh, 10, &nrd);
            _dos_read(fhc, (char far *)member_shield, 0x14, &nrd);
            fclose(fpm);
            for (il = 0; il < 5; il++)
                if (idsh[il] == D_e7da)
                    break;
            if (il < 5) {
                if (!memcmp(member_shield[il], "F01", 4)) {
                    if (++D_e892 >= 6)
                        D_e892 = 0;
                    switch (D_e892) {
                    case 0:
                        D_e75a = 0x18;
                        D_e75b = 0x28;
                        D_e75d = 0x11;
                        D_e759 = 0x0;
                        D_e75c = 0x0;
                        D_e75f = 0x0;
                        D_e760 = 0x15;
                        D_e75e = 0x25;
                        D_e761 = 0x25;
                        break;
                    case 1:
                        D_e75b = 0x0;
                        D_e75c = 0xc;
                        D_e75a = 0x7;
                        D_e75d = 0x7;
                        D_e759 = 0x6;
                        D_e75e = 0x6;
                        D_e75f = 0x8;
                        D_e760 = 0x8;
                        D_e761 = 0x5;
                        break;
                    case 2:
                        D_e75a = 0x18;
                        D_e759 = 0x0;
                        D_e75b = 0x0;
                        D_e75c = 0xf;
                        D_e75d = 0x12;
                        D_e75e = 0xe;
                        D_e760 = 0x13;
                        D_e75f = 0xc;
                        D_e761 = 0xc;
                        break;
                    case 3:
                        D_e759 = 0x35;
                        D_e75a = 0x35;
                        D_e75b = 0xb;
                        D_e75c = 0x2f;
                        D_e75d = 0x2f;
                        D_e75e = 0x8;
                        D_e75f = 0x39;
                        D_e760 = 0x2e;
                        D_e761 = 0x9;
                        break;
                    case 4:
                        D_e759 = 0x31;
                        D_e75a = 0x1a;
                        D_e75b = 0x11;
                        D_e75d = 0x13;
                        D_e75e = 0x5;
                        D_e75c = 0x25;
                        D_e75f = 0x25;
                        D_e760 = 0x17;
                        D_e761 = 0x2;
                        break;
                    case 5:
                        D_e759 = 0x1c;
                        D_e75a = 0xa;
                        D_e75c = 0x28;
                        D_e75d = 0x10;
                        D_e75e = 0x11;
                        D_e75f = 0x21;
                        D_e75b = 0xb;
                        D_e760 = 0xb;
                        D_e761 = 0xc;
                        break;
                    }
                    member_palette[il][6] = D_e759;
                    member_palette[il][7] = D_e75a;
                    member_palette[il][8] = D_e75b;
                    member_palette[il][9] = D_e75c;
                    member_palette[il][10] = D_e75d;
                    member_palette[il][11] = D_e75e;
                    member_palette[il][12] = D_e75f;
                    member_palette[il][13] = D_e760;
                    member_palette[il][14] = D_e761;
                } else if (!memcmp(member_shield[il], "A00", 4)) {
                    if (++D_e892 >= 6)
                        D_e892 = 0;
                    switch (D_e892) {
                    case 0:
                        D_e75c = 0x23;
                        D_e75f = 0x21;
                        D_e75d = 0xc;
                        D_e760 = 0xc;
                        D_e762 = 0x1a;
                        D_e763 = 0x7;
                        D_e75e = 0x12;
                        D_e761 = 0x12;
                        D_e764 = 0x12;
                        D_e765 = 0x13;
                        D_e768 = 0x13;
                        D_e766 = 0x6;
                        D_e767 = 0x6;
                        D_e769 = 0x6;
                        D_e76a = 0x6;
                        break;
                    case 1:
                        D_e75c = 0x2b;
                        D_e75f = 0x26;
                        D_e760 = 0x1e;
                        D_e762 = 0x25;
                        D_e763 = 0x1a;
                        D_e75d = 0x21;
                        D_e765 = 0x21;
                        D_e768 = 0x21;
                        D_e766 = 0x18;
                        D_e769 = 0x18;
                        D_e75e = 0x9;
                        D_e761 = 0x9;
                        D_e764 = 0x9;
                        D_e767 = 0x9;
                        D_e76a = 0x9;
                        break;
                    case 2:
                        D_e75c = 0x8;
                        D_e75d = 0x1c;
                        D_e75f = 0x5;
                        D_e760 = 0x18;
                        D_e75e = 0x14;
                        D_e761 = 0x14;
                        D_e763 = 0x14;
                        D_e762 = 0x0;
                        D_e765 = 0x0;
                        D_e764 = 0x10;
                        D_e766 = 0x10;
                        D_e767 = 0xe;
                        D_e768 = 0x9;
                        D_e769 = 0x9;
                        D_e76a = 0x9;
                        break;
                    case 3:
                        D_e75d = 0x14;
                        D_e75e = 0x15;
                        D_e761 = 0x1c;
                        D_e763 = 0xd;
                        D_e764 = 0x17;
                        D_e766 = 0xa;
                        D_e760 = 0x11;
                        D_e767 = 0x11;
                        D_e75c = 0x0;
                        D_e75f = 0x0;
                        D_e762 = 0x0;
                        D_e765 = 0x0;
                        D_e768 = 0x0;
                        D_e769 = 0x8;
                        D_e76a = 0xe;
                        break;
                    case 4:
                        D_e75c = 0x10;
                        D_e75e = 0x10;
                        D_e75f = 0xb;
                        D_e760 = 0xb;
                        D_e75d = 0xe;
                        D_e761 = 0xe;
                        D_e762 = 0xc;
                        D_e763 = 0x9;
                        D_e764 = 0x9;
                        D_e765 = 0x9;
                        D_e766 = 0x9;
                        D_e767 = 0x9;
                        D_e768 = 0x0;
                        D_e769 = 0x0;
                        D_e76a = 0x0;
                        break;
                    case 5:
                        D_e75c = 0x2a;
                        D_e75d = 0x15;
                        D_e760 = 0x14;
                        D_e75f = 0x28;
                        D_e762 = 0x28;
                        D_e763 = 0xf;
                        D_e765 = 0x23;
                        D_e768 = 0x23;
                        D_e766 = 0xe;
                        D_e769 = 0xe;
                        D_e75e = 0xb;
                        D_e761 = 0xb;
                        D_e764 = 0xb;
                        D_e767 = 0xb;
                        D_e76a = 0xb;
                        break;
                    }
                    member_palette[il][9] = D_e75c;
                    member_palette[il][10] = D_e75d;
                    member_palette[il][11] = D_e75e;
                    member_palette[il][12] = D_e75f;
                    member_palette[il][13] = D_e760;
                    member_palette[il][14] = D_e761;
                    member_palette[il][15] = D_e762;
                    member_palette[il][16] = D_e763;
                    member_palette[il][17] = D_e764;
                    member_palette[il][18] = D_e765;
                    member_palette[il][19] = D_e766;
                    member_palette[il][20] = D_e767;
                    member_palette[il][21] = D_e768;
                    member_palette[il][22] = D_e769;
                    member_palette[il][23] = D_e76a;
                } else if (!memcmp(member_shield[il], "C00", 4)) {
                    if (++D_e892 >= 6)
                        D_e892 = 0;
                    switch (D_e892) {
                    case 0:
                        D_e762 = 0x18;
                        D_e763 = 0x11;
                        D_e764 = 0x9;
                        D_e765 = 0x15;
                        D_e766 = 0x10;
                        D_e767 = 0x8;
                        D_e768 = 0x13;
                        D_e769 = 0xe;
                        D_e76a = 0x7;
                        break;
                    case 1:
                        D_e762 = 0x12;
                        D_e763 = 0x12;
                        D_e764 = 0x12;
                        D_e765 = 0xd;
                        D_e766 = 0xd;
                        D_e767 = 0xd;
                        D_e768 = 0xa;
                        D_e769 = 0xa;
                        D_e76a = 0xa;
                        break;
                    case 2:
                        D_e763 = 0x1c;
                        D_e766 = 0x15;
                        D_e769 = 0x10;
                        D_e762 = 0x0;
                        D_e764 = 0x0;
                        D_e765 = 0x0;
                        D_e767 = 0x0;
                        D_e768 = 0x0;
                        D_e76a = 0x0;
                        break;
                    case 3:
                        D_e762 = 0x28;
                        D_e763 = 0x25;
                        D_e764 = 0x14;
                        D_e765 = 0x21;
                        D_e766 = 0x20;
                        D_e767 = 0x11;
                        D_e768 = 0x1c;
                        D_e769 = 0x1a;
                        D_e76a = 0xe;
                        break;
                    case 4:
                        D_e762 = 0x21;
                        D_e763 = 0x1e;
                        D_e764 = 0x1e;
                        D_e765 = 0x1e;
                        D_e766 = 0x1a;
                        D_e767 = 0x1a;
                        D_e768 = 0x18;
                        D_e769 = 0x14;
                        D_e76a = 0x14;
                        break;
                    case 5:
                        D_e762 = 0x32;
                        D_e763 = 0x32;
                        D_e764 = 0x25;
                        D_e765 = 0x2a;
                        D_e766 = 0x2a;
                        D_e767 = 0x1c;
                        D_e768 = 0x23;
                        D_e769 = 0x23;
                        D_e76a = 0x15;
                        break;
                    }
                    member_palette[il][15] = D_e762;
                    member_palette[il][16] = D_e763;
                    member_palette[il][17] = D_e764;
                    member_palette[il][18] = D_e765;
                    member_palette[il][19] = D_e766;
                    member_palette[il][20] = D_e767;
                    member_palette[il][21] = D_e768;
                    member_palette[il][22] = D_e769;
                    member_palette[il][23] = D_e76a;
                } else if (!memcmp(member_shield[il], "F60", 4)) {
                    if (++D_e892 >= 6)
                        D_e892 = 0;
                    switch (D_e892) {
                    case 0:
                        D_e756 = 0x10;
                        D_e757 = 0x10;
                        D_e758 = 0x12;
                        D_e759 = 0x14;
                        D_e75a = 0x14;
                        D_e75b = 0x15;
                        D_e75c = 0x18;
                        D_e75d = 0x18;
                        D_e75e = 0x1a;
                        D_e75f = 0x1c;
                        D_e760 = 0x1c;
                        D_e761 = 0x1e;
                        break;
                    case 1:
                        D_e757 = 0x10;
                        D_e75a = 0x14;
                        D_e75d = 0x18;
                        D_e760 = 0x1c;
                        D_e756 = 0x0;
                        D_e758 = 0x0;
                        D_e759 = 0x0;
                        D_e75b = 0x0;
                        D_e75c = 0x0;
                        D_e75e = 0x0;
                        D_e75f = 0x0;
                        D_e761 = 0x0;
                        break;
                    case 2:
                        D_e758 = 0xd;
                        D_e75a = 0x4;
                        D_e75b = 0x11;
                        D_e75d = 0xa;
                        D_e75e = 0x14;
                        D_e756 = 0x0;
                        D_e757 = 0x0;
                        D_e759 = 0x0;
                        D_e75c = 0x0;
                        D_e75f = 0x0;
                        D_e760 = 0x12;
                        D_e761 = 0x19;
                        break;
                    case 3:
                        D_e756 = 0x11;
                        D_e757 = 0xb;
                        D_e75a = 0xe;
                        D_e75c = 0x19;
                        D_e75d = 0x12;
                        D_e75f = 0x1e;
                        D_e759 = 0x15;
                        D_e760 = 0x15;
                        D_e758 = 0x0;
                        D_e75b = 0x0;
                        D_e75e = 0x0;
                        D_e761 = 0x0;
                        break;
                    case 4:
                        D_e756 = 0x15;
                        D_e759 = 0x19;
                        D_e75c = 0x1c;
                        D_e75f = 0x21;
                        D_e757 = 0x0;
                        D_e758 = 0x0;
                        D_e75a = 0x0;
                        D_e75b = 0x0;
                        D_e75d = 0x0;
                        D_e75e = 0x0;
                        D_e760 = 0x0;
                        D_e761 = 0x0;
                        break;
                    case 5:
                        D_e756 = 0x0;
                        D_e757 = 0x0;
                        D_e758 = 0x0;
                        D_e759 = 0x9;
                        D_e75a = 0x9;
                        D_e75b = 0x9;
                        D_e75c = 0xe;
                        D_e75d = 0xe;
                        D_e75e = 0xe;
                        D_e75f = 0x15;
                        D_e760 = 0x15;
                        D_e761 = 0x15;
                        break;
                    }
                    member_palette[il][3] = D_e756;
                    member_palette[il][4] = D_e757;
                    member_palette[il][5] = D_e758;
                    member_palette[il][6] = D_e759;
                    member_palette[il][6] = D_e75a;
                    member_palette[il][8] = D_e75b;
                    member_palette[il][9] = D_e75c;
                    member_palette[il][10] = D_e75d;
                    member_palette[il][11] = D_e75e;
                    member_palette[il][12] = D_e75f;
                    member_palette[il][13] = D_e760;
                    member_palette[il][14] = D_e761;
                }
                set_palette(cur_palette);
            }
        }
        break;
    case 0x433: case 0x5133:
        if (D_e7dc > 0) {
            sub_root_06a1_2c9e("charactr.tmp", 2, &fhc);
            fpm = fdopen(fhc, "r+b");
            _dos_read(fhc, (int far *)&il, 2, &nrd);
            _dos_read(fhc, (int far *)idsh, 10, &nrd);
            _dos_read(fhc, (char far *)member_shield, 0x14, &nrd);
            fclose(fpm);
            for (il = 0; il < 5; il++)
                if (idsh[il] == D_e7da)
                    break;
            if (il < 5) {
                if (!memcmp(member_shield[il], "F01", 4)) {
                    if (++D_e894 >= 6)
                        D_e894 = 0;
                    switch (D_e894) {
                    case 0:
                        D_e762 = 0x1c;
                        D_e765 = 0x17;
                        D_e768 = 0x11;
                        D_e763 = 0x0;
                        D_e764 = 0x0;
                        D_e766 = 0x0;
                        D_e767 = 0x0;
                        D_e769 = 0x0;
                        D_e76a = 0x0;
                        break;
                    case 1:
                        D_e764 = 0x0;
                        D_e765 = 0xc;
                        D_e763 = 0x7;
                        D_e766 = 0x7;
                        D_e762 = 0x6;
                        D_e767 = 0x6;
                        D_e768 = 0x8;
                        D_e769 = 0x8;
                        D_e76a = 0x5;
                        break;
                    case 2:
                        D_e762 = 0x2b;
                        D_e763 = 0x2a;
                        D_e764 = 0x1c;
                        D_e765 = 0x23;
                        D_e766 = 0x23;
                        D_e767 = 0x12;
                        D_e768 = 0x28;
                        D_e769 = 0x25;
                        D_e76a = 0x13;
                        break;
                    case 3:
                        D_e763 = 0x2a;
                        D_e764 = 0x2a;
                        D_e766 = 0x20;
                        D_e767 = 0x21;
                        D_e762 = 0x0;
                        D_e765 = 0x0;
                        D_e768 = 0x0;
                        D_e769 = 0x25;
                        D_e76a = 0x25;
                        break;
                    case 4:
                        D_e762 = 0x15;
                        D_e763 = 0x0;
                        D_e764 = 0xc;
                        D_e765 = 0x13;
                        D_e766 = 0x8;
                        D_e767 = 0xe;
                        D_e768 = 0x11;
                        D_e769 = 0x2;
                        D_e76a = 0x18;
                        break;
                    case 5:
                        D_e762 = 0x26;
                        D_e763 = 0x28;
                        D_e764 = 0x2e;
                        D_e765 = 0x1e;
                        D_e766 = 0x1c;
                        D_e767 = 0x25;
                        D_e768 = 0x2c;
                        D_e769 = 0x2b;
                        D_e76a = 0x33;
                        break;
                    }
                    member_palette[il][15] = D_e762;
                    member_palette[il][16] = D_e763;
                    member_palette[il][17] = D_e764;
                    member_palette[il][18] = D_e765;
                    member_palette[il][19] = D_e766;
                    member_palette[il][20] = D_e767;
                    member_palette[il][21] = D_e768;
                    member_palette[il][22] = D_e769;
                    member_palette[il][23] = D_e76a;
                } else if (!memcmp(member_shield[il], "F60", 4)) {
                    if (++D_e894 >= 6)
                        D_e894 = 0;
                    switch (D_e894) {
                    case 0:
                        D_e753 = 0x19;
                        D_e754 = 0x12;
                        D_e755 = 0x0;
                        break;
                    case 1:
                        D_e754 = 0x18;
                        D_e753 = 0x0;
                        D_e755 = 0x0;
                        break;
                    case 2:
                        D_e753 = 0x0;
                        D_e754 = 0xa;
                        D_e755 = 0x15;
                        break;
                    case 3:
                        D_e753 = 0x1c;
                        D_e754 = 0x0;
                        D_e755 = 0x0;
                        break;
                    case 4:
                        D_e753 = 0xe;
                        D_e754 = 0xe;
                        D_e755 = 0xe;
                        break;
                    case 5:
                        D_e753 = 0x3f;
                        D_e754 = 0x28;
                        D_e755 = 0x19;
                        break;
                    }
                    member_palette[il][0] = D_e753;
                    member_palette[il][1] = D_e754;
                    member_palette[il][2] = D_e755;
                } else if (!memcmp(member_shield[il], "C00", 4)) {
                    if (++D_e894 >= 6)
                        D_e894 = 0;
                    switch (D_e894) {
                    case 0:
                        D_e759 = 0x2a;
                        D_e75a = 0x1a;
                        D_e75b = 0x11;
                        D_e75c = 0x25;
                        D_e75d = 0x18;
                        D_e75e = 0xe;
                        D_e75f = 0x1c;
                        D_e760 = 0x13;
                        D_e761 = 0xa;
                        break;
                    case 1:
                        D_e759 = 0x11;
                        D_e75a = 0x11;
                        D_e75b = 0x11;
                        D_e75c = 0xa;
                        D_e75d = 0xa;
                        D_e75e = 0xa;
                        D_e75f = 0x3;
                        D_e760 = 0x3;
                        D_e761 = 0x3;
                        break;
                    case 2:
                        D_e75a = 0x2a;
                        D_e759 = 0x9;
                        D_e75b = 0x9;
                        D_e75d = 0x25;
                        D_e760 = 0x1a;
                        D_e75c = 0xa;
                        D_e75e = 0xa;
                        D_e75f = 0xa;
                        D_e761 = 0xa;
                        break;
                    case 3:
                        D_e759 = 0x21;
                        D_e75a = 0x21;
                        D_e75b = 0x21;
                        D_e75c = 0x1c;
                        D_e75d = 0x1c;
                        D_e75e = 0x1c;
                        D_e75f = 0x18;
                        D_e760 = 0x18;
                        D_e761 = 0x18;
                        break;
                    case 4:
                        D_e759 = 0x30;
                        D_e75a = 0x2d;
                        D_e75c = 0x28;
                        D_e75d = 0x28;
                        D_e75e = 0x1c;
                        D_e75b = 0x21;
                        D_e75f = 0x21;
                        D_e760 = 0x21;
                        D_e761 = 0x18;
                        break;
                    case 5:
                        D_e759 = 0x2f;
                        D_e75a = 0x2c;
                        D_e75b = 0x18;
                        D_e75c = 0x2b;
                        D_e75e = 0x15;
                        D_e75d = 0x28;
                        D_e75f = 0x28;
                        D_e760 = 0x25;
                        D_e761 = 0x13;
                        break;
                    }
                    member_palette[il][6] = D_e759;
                    member_palette[il][7] = D_e75a;
                    member_palette[il][8] = D_e75b;
                    member_palette[il][9] = D_e75c;
                    member_palette[il][10] = D_e75d;
                    member_palette[il][11] = D_e75e;
                    member_palette[il][12] = D_e75f;
                    member_palette[il][13] = D_e760;
                    member_palette[il][14] = D_e761;
                } else if (!memcmp(member_shield[il], "A00", 4)) {
                    if (++D_e894 >= 6)
                        D_e894 = 0;
                    switch (D_e894) {
                    case 0:
                        D_e75c = 0x23;
                        D_e75f = 0x21;
                        D_e75d = 0xc;
                        D_e760 = 0xc;
                        D_e762 = 0x1a;
                        D_e763 = 0x7;
                        D_e75e = 0x12;
                        D_e761 = 0x12;
                        D_e764 = 0x12;
                        D_e765 = 0x13;
                        D_e768 = 0x13;
                        D_e766 = 0x6;
                        D_e767 = 0x6;
                        D_e769 = 0x6;
                        D_e76a = 0x6;
                        break;
                    case 1:
                        D_e75c = 0x2b;
                        D_e75f = 0x26;
                        D_e760 = 0x1e;
                        D_e762 = 0x25;
                        D_e763 = 0x1a;
                        D_e75d = 0x21;
                        D_e765 = 0x21;
                        D_e768 = 0x21;
                        D_e766 = 0x18;
                        D_e769 = 0x18;
                        D_e75e = 0x9;
                        D_e761 = 0x9;
                        D_e764 = 0x9;
                        D_e767 = 0x9;
                        D_e76a = 0x9;
                        break;
                    case 2:
                        D_e75c = 0x8;
                        D_e75d = 0x1c;
                        D_e75f = 0x5;
                        D_e760 = 0x18;
                        D_e75e = 0x14;
                        D_e761 = 0x14;
                        D_e763 = 0x14;
                        D_e762 = 0x0;
                        D_e765 = 0x0;
                        D_e764 = 0x10;
                        D_e766 = 0x10;
                        D_e767 = 0xe;
                        D_e768 = 0x9;
                        D_e769 = 0x9;
                        D_e76a = 0x9;
                        break;
                    case 3:
                        D_e75d = 0x14;
                        D_e75e = 0x15;
                        D_e761 = 0x1c;
                        D_e763 = 0xd;
                        D_e764 = 0x17;
                        D_e766 = 0xa;
                        D_e760 = 0x11;
                        D_e767 = 0x11;
                        D_e75c = 0x0;
                        D_e75f = 0x0;
                        D_e762 = 0x0;
                        D_e765 = 0x0;
                        D_e768 = 0x0;
                        D_e769 = 0x8;
                        D_e76a = 0xe;
                        break;
                    case 4:
                        D_e75c = 0x10;
                        D_e75e = 0x10;
                        D_e75f = 0xb;
                        D_e760 = 0xb;
                        D_e75d = 0xe;
                        D_e761 = 0xe;
                        D_e762 = 0xc;
                        D_e763 = 0x9;
                        D_e764 = 0x9;
                        D_e765 = 0x9;
                        D_e766 = 0x9;
                        D_e767 = 0x9;
                        D_e768 = 0x0;
                        D_e769 = 0x0;
                        D_e76a = 0x0;
                        break;
                    case 5:
                        D_e75c = 0x2a;
                        D_e75d = 0x15;
                        D_e760 = 0x14;
                        D_e75f = 0x28;
                        D_e762 = 0x28;
                        D_e763 = 0xf;
                        D_e765 = 0x23;
                        D_e768 = 0x23;
                        D_e766 = 0xe;
                        D_e769 = 0xe;
                        D_e75e = 0xb;
                        D_e761 = 0xb;
                        D_e764 = 0xb;
                        D_e767 = 0xb;
                        D_e76a = 0xb;
                        break;
                    }
                    member_palette[il][9] = D_e75c;
                    member_palette[il][10] = D_e75d;
                    member_palette[il][11] = D_e75e;
                    member_palette[il][12] = D_e75f;
                    member_palette[il][13] = D_e760;
                    member_palette[il][14] = D_e761;
                    member_palette[il][15] = D_e762;
                    member_palette[il][16] = D_e763;
                    member_palette[il][17] = D_e764;
                    member_palette[il][18] = D_e765;
                    member_palette[il][19] = D_e766;
                    member_palette[il][20] = D_e767;
                    member_palette[il][21] = D_e768;
                    member_palette[il][22] = D_e769;
                    member_palette[il][23] = D_e76a;
                }
                set_palette(cur_palette);
            }
        }
        break;

    case 0x1000:
        sub_s038_1462_01fe(0);
        quit_to_dos();
        break;
    }
}

/* list of the characters saved in charactr.tmp (right-hand column); '+' marks party members */
void far sub_s038_1462_23ec(struct font *f)
{
    int ids[5];
    int n;
    int nr;
    int h;
    int *slot;
    int i;
    int shown;
    int y;
    int j;
    mouse_hide();
    sub_root_06a1_2c9e("charactr.tmp", 0, &h);
    _dos_read(h, (int far *)&n, 2, &nr);
    _dos_read(h, (int far *)ids, 10, &nr);
    _dos_read(h, (char far *)member_shield, 0x14, &nr);
    shown = 0;
    for (i = 0; i < n; i++) {
        _dos_read(h, (char far *)&D_8d2e, 0x80, &nr);
        _dos_read(h, (char far *)msg_buf, 0x1aa, &nr);
        if ((D_8d2e.id == cur_loc || D_8d2e.status) && D_8d2e.status != 5
            && (!D_8d2e.status || !event_type_at(0x43, slot_of_char_vec(i)))) {
            D_8d06[shown] = i;
            gfx_copy_rect(0x2806, 0xc5, 0x18 + shown * 8, 0x73, 8, screen_win, 0xc5, 0x18 + shown * 8);
            for (j = 0; j < 5; j++) {
                if (shown == ids[j]) {
                    f->color = 0x11;
                    gfx_text(f, 200, 0x18 + shown * 8, "+");
                    f->color = 0x10;
                    gfx_text(f, 199, 0x18 + shown * 8 - 1, "+");
                }
            }
            f->color = 0x11;
            gfx_text(f, 0xd0, 0x18 + shown * 8 - 1, D_8d2e.name);
            f->color = 0x10;
            gfx_text(f, 0xcf, 0x18 + shown * 8 - 2, D_8d2e.name);
            shown++;
        }
    }
    _dos_close(h);
    D_e7dc = n;
    gfx_recolor_rect(f, 0xc6, (D_e7da << 3) + 0x16, 0x71, 8, 0x10, 0x12);
    mouse_show();
}

/* the eight attribute names */
void far sub_s038_1462_25ca(struct font *f)
{
    mouse_hide();
    f->color = 8;
    draw_text_far(f, 0x48, 0x30, F_290e_0000.s[1]);
    draw_text_far(f, 0x48, 0x3f, F_290e_0000.s[0]);
    draw_text_far(f, 0x48, 0x4e, F_290e_0000.s[0x40]);
    draw_text_far(f, 0x48, 0x5d, F_290e_0000.s[0x1a]);
    draw_text_far(f, 0x48, 0x6c, F_290e_0000.s[0x3d]);
    draw_text_far(f, 0x48, 0x7b, F_290e_0000.s[0x3e]);
    draw_text_far(f, 0x48, 0x8a, F_290e_0000.s[0x3f]);
    draw_text_far(f, 0x48, 0x99, F_290e_0000.s[0x3c]);
    mouse_show();
}
