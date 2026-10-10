/* place: s000 0e76:0000 */
/* orig: size=13c8 CONST=7d12+0006 _DATA=0946+04e1 */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* Start-up module: EMS error messages, main(), command line / config.drk / sound driver
   set-up (0114), game start (052a), battle menus (083e, 0f04), commonsp.img (1010),
   memory check (1150).
   status (e9, 2026-10-09): COMPLETE, 8/11 match (0000, main, 0092, 00b8, 0114, 083c, 083e,
   0f04).  Not matching (6.00 vs 6.00A):
   - 052a: with pragma "a" 6.00A also makes the three `F[o + j] = v` fills `rep stosw` and the
     function has the original length; left: the table loop's register/slot assignment
     (orig i in SI, k/v/c in [bp-0a]/[bp-8]/[bp-4]).
   - 1010: 6.00 keeps the address of the image's first word (its size) in SI:DI and reads it
     again for the final _fmemcpy; 6.00A reloads D_c103 (only that, 17 diff lines).
   - 1150 MATCHED (w4, 9/11): 6.00 stores the dead far pointer hi word first; written as
     `fpf = (char far *)(char *)nm; *(unsigned *)&fpf = nm;` 6.00A drops the first offset
     store and keeps the same order.
   083e: `bo + aa + 9` vs `aa + bo + 9` flips with unrelated edits elsewhere in the file
   (symbol-table order) - re-check after any change.
   0114: the `name` variable was a cross-jumped strcat in both arms of the config.drk if.
   _DATA after main's strings: 21 bytes of initialised data at 0bf1 (D_bfb used by 083e),
   then 0114's "rb", "config.drk", "nsound.dlc", ... - literals in source order. */
#include "darklands/card.h"
#include "darklands/game.h"
struct WORDREGS { unsigned ax, bx, cx, dx, si, di, cflag; };
struct BYTEREGS { unsigned char al, ah, bl, bh, cl, ch, dl, dh; };
union REGS { struct WORDREGS x; struct BYTEREGS h; };

extern int D_0942;
extern char D_907a;
extern char D_907d;
extern int msg_file[];
int far printf(const char *, ...);
void far exit_(int);
int far int86(int, union REGS *, union REGS *);
int far sub_root_044a_00a9(int, int, char *);
int far dos_print(char *);
int far sub_root_044a_0100(void);
int far sub_root_047c_07ce(char *, char *);
int far sub_root_047c_08c8(int);
int far sub_root_05f8_0064(void);
int far file_load_far(char *, void far *);
void far video_detect(void);

typedef struct _iobuf FILE;
FILE *far fopen(const char *, const char *);
unsigned far fread(void *, unsigned, unsigned, FILE *);
int far fclose(FILE *);
char *strcpy(char *, const char *);
char *strcat(char *, const char *);
unsigned strlen(const char *);
void far *far _fmemcpy(void far *, const void far *, unsigned);
long far time(long *);
unsigned far _dos_allocmem(unsigned, unsigned *);
unsigned far alloc_buffer(unsigned long, char *);
void far sub_root_044a_000a(char *);
void far sub_root_044a_002c(char far *, char *);
void far sub_root_044a_019c(void);
void far sub_root_044a_01f4(void);
void far game_shutdown(void);
void far show_difficulty(int);
int far text_pixel_width(void *, char *);
void far sub_root_03a1_000a(int);
void far sub_root_047c_0066(void);
int far sub_root_047c_04c7(int);
int far gfx_save_rect(int, int, int, int, int);
void far sub_root_047c_04f8(void);
int far font_height(int);
void far sub_root_047c_0545(void);
void far sub_root_047c_0576(int, int);
void far gfx_draw_pic(void *, int, int, int);
void far sub_root_047c_05ca(int);
void far sub_root_047c_0767(void);
void far sub_root_047c_0783(int, int, int, int, int, int, int);
void far sound_play(int);
void far sub_root_047c_0791(void);
void far sub_root_047c_11f6(void);
void far mouse_set_cursor(int, int, int);
int far sub_root_05f8_00a9(int *);
void far sub_root_05f8_00ca(int *, int *);
int far sub_root_05f8_00f3(int, int *);
void far ems_map_page(int, int, int);
int far file_save(char *, void far *, int);
void far swap_init_vec(void);
void far battle_run_vec(int, int, int, int, int, int, int, int, int, int, int, int);
void far vec_17e3(void), choose_party_order_vec(void), save_game_vec(void), load_game_vec(void), party_info_screen_vec(void);
void far vec_1941(void), vec_194b(void), vec_1955(void), vec_195f(void), vec_1969(void);
void far vec_1973(void), vec_197d(void), vec_1987(void), vec_1991(void), vec_199b(void);
void far vec_19a5(void), vec_19af(void), vec_19b9(void), vec_19c3(void), vec_19cd(void);
void far vec_19d7(void), vec_19e1(void), do_surrender_vec(void), vec_19f5(void), vec_19ff(void);
void far vec_1a09(void), vec_1a13(void), vec_1a1d(void), vec_1a27(void), vec_1a31(void);
void far vec_1a3b(void), vec_1a45(void), enemy_info_vec(void), order_mode_leave_vec(void), vec_1a63(void);
void far vec_1a6d(void), vec_1a77(void), vec_1a81(void), vec_1a8b(void), vec_1a95(void);
void far vec_1a9f(void), vec_1aa9(void), vec_1ab3(void), vec_1abd(void), vec_1ac7(void);
void far vec_1ad1(void);
void far sub_root_0000_1080(void), toggle_saint_msgs(void), toggle_music(void);
void far toggle_sound(void), quit_to_dos(void), set_difficulty_basic(void);
void far set_difficulty_standard(void), set_difficulty_expert(void);

/* menus (header D_9419[5], items 0x14 bytes, sub-menu items 0x16 bytes) */
struct mi2 { char *text; char on; char x3[4]; char x7; char x8[10]; void (far *fn)(void); };
struct mi { char *text; char on; char key[3]; char x6[8]; void (far *fn)(void); struct mi2 *sub; };
struct menu { char *title; char x2[4]; char x6; char x7; struct mi *items; };
struct font { char x0[0x10]; int x10; };
extern struct menu D_9419[5];
extern struct mi D_9607[19];
extern struct mi D_97db[7];
extern struct mi D_987d[3];
extern struct mi D_991e[9];
extern struct mi2 D_99ec[4];
extern struct mi2 D_9a47[20];
extern int D_9c15[4];
extern char D_9c22[4];
extern struct font *screen_ptr;
extern struct font *card_window;
extern char *D_11f2;
extern char D_c107[];
extern char D_060c[], D_067c[], D_0690[], D_06a4[], D_06aa[], D_06ba[], D_06cc[], D_06de[];
extern char D_06f0[], D_05b2[], D_0706[], D_0718[], D_0726[], D_07aa[], D_07bc[];
extern char D_06b3[], D_06c5[], D_06d7[], D_06e8[], D_06fe[], D_0737[], D_073e[], D_0744[];
extern char *D_01a6;
extern int D_08ca, D_08cc, swap_allowed;
extern char D_9069, game_difficulty, D_9071, D_9c4c, D_11dd;
extern char party[][0x80];
extern int D_9417;
extern int ems_frame_handle;
extern int cursor_default;
extern int D_a2e5, D_a2e9, D_a33b, D_a33e, D_a392, D_a3d6, D_a3e5;
extern int D_a626, D_a628, D_a660, D_a662, D_a664, D_a666;
extern char real_combat;
extern int D_a6c2;
extern int D_a86e, D_a870, D_a872, D_a874;
extern char D_a876[];
extern int D_7760;
extern char msg_buf[];
extern char D_c0fd;
extern int D_c0fe, D_c100;
extern char move_mode;
extern int load_buf, D_c105;
extern int far F_20a5_415e[];
extern int far F_20a5_4a32[];
extern int far F_20a5_46d6[];
extern int far F_20a5_45be[];
extern int far F_20a5_491a[];
extern int far F_20a5_4046[];
extern int far F_20a5_4b4a[];
void far startup_config(int, char **, char **);
void far sub_s000_0e76_052a(void);
void far sub_s000_0e76_083c(void);
void far sub_s000_0e76_083e(void);
void far battle_menus_enable(void);
void far sub_s000_0e76_1010(void);
void far sub_s000_0e76_1150(void);

/* EMS error messages, indexed by error code - 0x80 */
char *D_0ba0[] = {
    "Internal error in EMM.\n",
    "Malfunction in expanded-memory hardware.\n",
    "Expanded Memory Manager busy.\n",
    "Invalid EMS Handle.\n",
    "EMS function not defined.\n",
    "No more EMS handles available.\n",
    "Error in save or restore of mapping context.\n",
    "requested # of pages not in system.\n",
    "requested # of pages not available.\n",
    "Zero pages cannot be allocated.\n",
    "Logical page out of range for handle.\n",
    "Illegal physical page # !(0 <= pg <= 3).\n",
    "Page-mapping hardware state area full.\n",
    "Save of mapping context failed.\n",
    "Restore of mapping context failed.\n",
    "Subfunction parameter not defined.\n",
    "Expanded Memory Manager not present.\n",
};

/* print "<program>: msg" when D_942 is set */
void far startup_error(char *msg)
{
    if (D_0942)
        printf("%s: %s\n", msg, D_0ba0[D_0942 - 0x80]);
}

void far main(int argc, char **argv, char **envp)
{
    video_detect();
    sub_root_047c_08c8(sub_root_047c_07ce("misc.exe", 0));
    D_907a = 0;
    file_load_far("banner.dat", (void far *)msg_file);
    if (!(D_907d = !sub_root_05f8_0064())) {
        printf("Unable to find EMS memory\n");
        exit_(0);
    }
    startup_config(argc, argv, envp);
    sub_root_044a_0100();
    sub_s000_0e76_052a();
}

/* set the cursor position (int 10h function 2) */
void far bios_set_cursor(char row, char col)
{
    union REGS r;

    r.h.ah = 2;
    r.h.bh = 0;
    r.h.dh = row;
    r.h.dl = col;
    int86(0x10, &r, &r);
}

/* start-up options (set by the command line, used by 052a / 1150) */
int D_0bf2 = 0;                 /* 'Q' */
char D_0bf4 = 0;                /* 'O': show the memory report */
int D_0bf6 = -1;
int D_0bf8 = 0;
char D_0bfa[9] = { -1, 0, 0, -1, 0, 0, -1, 0, 0 };

/* print the banner (msg_file: offsets of its lines) */
void far print_banner(void)
{
    register int i;

    bios_set_cursor(12, 0);
    for (i = 12; i; i--)
        dos_print((char *)msg_file + msg_file[0]);
    sub_root_044a_00a9(12, 0, (char *)msg_file + msg_file[11]);
    sub_root_044a_00a9(13, 0, (char *)msg_file + msg_file[12]);
    sub_root_044a_00a9(14, 0, (char *)msg_file + msg_file[13]);
}

/* title screen, config.drk (sound driver) and command line options */
void far startup_config(int argc, char **argv, char **envp)
{
    unsigned char i;
    char bad;
    FILE *f;
    char buf[50];
    char nm[13];
    union REGS r;

    r.x.ax = 3;
    int86(0x10, &r, &r);
    sub_root_044a_00a9(0, 0, "\xff\x7f");
    sub_root_044a_00a9(1, 0, (char *)msg_file + msg_file[1]);
    sub_root_044a_00a9(2, 0, (char *)msg_file + msg_file[2]);
    sub_root_044a_00a9(3, 0, (char *)msg_file + msg_file[3]);
    sub_root_044a_00a9(4, 0, (char *)msg_file + msg_file[4]);
    sub_root_044a_00a9(5, 0, (char *)msg_file + msg_file[5]);
    sub_root_044a_00a9(6, 0, (char *)msg_file + msg_file[6]);
    sub_root_044a_00a9(7, 0, (char *)msg_file + msg_file[7]);
    sub_root_044a_00a9(8, 0, (char *)msg_file + msg_file[8]);
    sub_root_044a_00a9(9, 0, (char *)msg_file + msg_file[9]);
    sub_root_044a_00a9(10, 0, (char *)msg_file + msg_file[10]);
    if ((f = fopen("config.drk", "rb")) == 0)
        strcat(D_a876, "nsound.dlc");
    else {
        fread(&D_a86e, 8, 1, f);
        fclose(f);
        switch (D_a86e) {
        case 0: case 1: case 2: case 3:
            strcpy(D_a876, "a");
            break;
        case 4: case 5: case 6:
            strcpy(D_a876, "p");
            break;
        case 7:
            strcpy(D_a876, "r");
            break;
        case 0x7f:
            strcpy(D_a876, "i");
            break;
        default:
            strcpy(D_a876, "n");
            break;
        }
        strcat(D_a876, "sound.dlc");
    }
    for (i = 0; i < argc; i++)
        sub_root_044a_000a(argv[i]);
    bios_set_cursor(0x16, 0);
    bad = 0;
    for (i = 1; i < argc; i++) {
        if (*argv[i] == '-' || *argv[i] == '/')
            argv[i]++;
        else {
            strcpy(buf, "Unknown argument: ");
            strcat(buf, argv[i]);
            strcat(buf, "\r\n$");
            dos_print(buf);
            bad = 1;
        }
    }
    if (bad == 1)
        exit_(0);
    D_c0fe = D_c100 = -1;
    D_c0fd = 0;
    for (i = 1; i < argc; i++) {
        switch (*argv[i]) {
        case 'A':
            nm[0] = *++argv[i];
            nm[1] = 0;
            strcat(nm, "sound.dlc");
            if ((f = fopen(nm, "r")) == 0) {
                argv[i] -= 2;
                strcpy(buf, "Sound driver not found: ");
                strcat(buf, argv[i]);
                strcat(buf, "\r\n$");
                dos_print(buf);
                bad = 1;
            } else {
                sub_root_044a_002c((char far *)D_a876, nm);
                fclose(f);
            }
            break;
        case 'O':
            D_0bf4 = 1;
            break;
        case 'Q':
            D_0bf2 = 1;
            break;
        default:
            strcpy(buf, "Unknown argument: ");
            strcat(buf, --argv[i]);
            strcat(buf, "\r\n$");
            dos_print(buf);
            bad = 1;
            break;
        }
    }
    if (bad == 1)
        exit_(0);
    sub_s000_0e76_1150();
}

/* game start: graphics, EMS work segment D_c105, swap stores, menus, tables, battle */
#pragma optimize("a", on)
void far sub_s000_0e76_052a(void)
{
    int seg, k, v, i, j, o, c;

    sub_root_047c_08c8(sub_root_047c_07ce("mgraphic.exe", "fonts.fnt"));
    D_9069 = 0;
    sub_root_047c_11f6();
    D_9c4c = 1;
    sub_root_047c_0767();
    sub_root_047c_05ca(0);
    sub_root_047c_04c7(0);
    if (D_907d == 1 && !sub_root_05f8_00f3(11, &ems_frame_handle)) {
        ems_map_page(0, 0, ems_frame_handle);
        ems_map_page(1, 1, ems_frame_handle);
        ems_map_page(2, 2, ems_frame_handle);
        ems_map_page(3, 3, ems_frame_handle);
        sub_root_05f8_00a9(&seg);
        sub_root_047c_0576(1, seg);
        D_c105 = seg;
    } else
        sub_root_047c_0576(1, D_c105 = sub_root_047c_04c7(1));
    load_buf = 0;
    *(char far **)&D_08ca = (char far *)msg_buf;
    swap_allowed = 0x2b;
    swap_init_vec();
    sub_s000_0e76_1010();
    sub_root_047c_0545();
    gfx_draw_pic(screen_ptr, 3, 1, D_a626);
    D_a6c2 = gfx_save_rect(1, 6, 0x1f, 10, font_height(screen_ptr->x10));
    sub_root_047c_04f8();
    mouse_set_cursor(0, 0, cursor_default);
    sub_root_047c_08c8(D_9417 = sub_root_047c_07ce(D_a876, 0));
    sub_root_047c_0783(0, 0, 0, D_a870, D_a86e, D_a872, D_a874);
    sub_root_047c_0066();
    game_difficulty = 1;
    *(volatile char *)&D_9071 = 0;
    move_mode = real_combat = 0;
    sub_s000_0e76_083e();
    sub_root_044a_019c();
    sound_play(7);
    for (i = 0, k = 0, v = -7; v < 0xd9; k++, v += 16) {
        o = i;
        c = -((k & 1) * 16 + 24);
        for (j = 0; j < 10; j++)
            F_20a5_415e[o + j] = v;
        for (j = 0; j < 10; j++)
            F_20a5_4a32[o + j] = v - 9;
        for (j = 0; j < 10; j++)
            F_20a5_46d6[o + j] = v - 0x19;
        i += 10;
        for (j = 0; j < 10; j++, c += 32) {
            F_20a5_45be[o + j] = c + 16;
            F_20a5_491a[o + j] = c;
            F_20a5_4046[o + j] = c + 32;
            F_20a5_4b4a[o + j] = c;
        }
    }
    if (D_907a == 1) {
        D_11dd = 1;
        if (D_0bf6 != -1)
            battle_run_vec(0, D_0bf6, D_0bf8, D_0bfa[0], D_0bfa[1], D_0bfa[2], D_0bfa[3], D_0bfa[4],
                     D_0bfa[5], D_0bfa[6], D_0bfa[7], D_0bfa[8]);
        else
            battle_run_vec(0, 10, 0, 3, 2, 5, 0, 1, 1, -1, 0, 0);
    } else {
        party[0][0] = party[1][0] = party[2][0] = party[3][0] = party[4][0] = D_11dd = 0;
        for (;;)
            sub_root_03a1_000a(4);
    }
    sub_root_044a_01f4();
    game_shutdown();
    sub_root_047c_0791();
    exit_(0);
}
#pragma optimize("", on)

void far sub_s000_0e76_083c(void)
{
}

/* battle menus: Game / Orders / Attack / Party.  Locals are named for 6.00A's name-hash
   slot order (6.00 used declaration order). */
#pragma optimize("a", on)
void far sub_s000_0e76_083e(void)
{
    int xh, aa, bo, saved, ik, jk;

    D_9419[0].title = "Game";
    D_9419[1].title = "Orders";
    D_9419[2].title = "Attack";
    D_9419[3].title = "Party";
    D_9419[4].title = "";
    D_9419[1].x6 = 2;
    D_9419[2].x6 = 2;
    D_9419[0].x6 = 1;
    D_9419[3].x6 = 1;
    D_9419[1].items = D_9607;
    D_9419[2].items = D_97db;
    D_9419[3].items = D_987d;
    D_9419[0].items = D_991e;
    D_991e[0].text = "\x02Save Game";
    strcpy(D_991e[0].key, "\x0fS");
    D_991e[1].text = "\x02Load Saved Game";
    strcpy(D_991e[1].key, "\x0fL");
    D_991e[2].text = "\x02" "Difficulty";
    strcpy(D_991e[2].key, "\x0f" "D");
    D_991e[2].sub = D_99ec;
    D_991e[3].text = "\x01Show Changes";
    strcpy(D_991e[3].key, "\x0f" "C");
    D_991e[4].text = "\x01Music";
    strcpy(D_991e[4].key, "\x0fM");
    D_991e[5].text = "\x01Sound FX";
    strcpy(D_991e[5].key, "\x0f" "F");
    D_991e[6].text = "\x02Pause";
    strcpy(D_991e[6].key, "\x0fP");
    D_991e[7].text = "\x02Quit to DOS";
    strcpy(D_991e[7].key, "\x0fQ");
    D_991e[8].text = "";
    for (ik = 0; ik <= 8; ik++)
        D_991e[ik].on = 1;
    D_991e[0].fn = save_game_vec;
    D_991e[1].fn = load_game_vec;
    D_991e[2].fn = vec_17e3;
    D_991e[3].fn = toggle_saint_msgs;
    D_991e[4].fn = toggle_music;
    D_991e[5].fn = toggle_sound;
    D_991e[6].fn = sub_root_0000_1080;
    D_991e[7].fn = quit_to_dos;
    D_99ec[0].text = "\x02" "Basic";
    D_99ec[1].text = "\x02Standard";
    D_99ec[2].text = "\x02" "Expert";
    D_99ec[3].text = "";
    for (ik = 0; ik < 3; ik++) {
        D_99ec[ik].on = 1;
        D_99ec[ik].x7 = 0;
    }
    D_99ec[0].fn = set_difficulty_basic;
    D_99ec[1].fn = set_difficulty_standard;
    D_99ec[2].fn = set_difficulty_expert;
    show_difficulty(0);
    D_9607[0].text = "\x02Resume";
    strcpy(D_9607[0].key, "\x13");
    D_9607[1].text = strcat(strcpy(D_c107, "\x02?"), D_11f2);
    strcpy(D_9607[1].key, "\x12");
    D_9607[2].text = "\x02" "Enemy Info";
    strcpy(D_9607[2].key, " E");
    D_9607[3].text = D_067c;
    strcpy(D_9607[3].key, " W");
    D_9607[4].text = D_0690;
    strcpy(D_9607[4].key, " F");
    D_9607[5].text = D_06a4;
    strcpy(D_9607[5].key, " H");
    D_9607[6].text = D_07aa;
    strcpy(D_9607[6].key, " G");
    D_9607[7].text = D_07bc;
    strcpy(D_9607[7].key, " Q");
    D_9607[8].text = D_06aa;
    strcpy(D_9607[8].key, " U");
    D_9607[9].text = D_06ba;
    strcpy(D_9607[9].key, " U");
    D_9607[10].text = D_06cc;
    strcpy(D_9607[10].key, " O");
    D_9607[11].text = D_06de;
    strcpy(D_9607[11].key, " P");
    D_9607[12].text = D_06f0;
    strcpy(D_9607[12].key, " D");
    D_9607[13].text = D_05b2;
    strcpy(D_9607[13].key, " D");
    D_9607[14].text = D_0706;
    strcpy(D_9607[14].key, " S");
    D_9607[15].text = D_0718;
    strcpy(D_9607[15].key, " L");
    D_9607[16].text = D_0726;
    strcpy(D_9607[16].key, " X");
    D_9607[17].text = "\x02" "Cancel Giving Order";
    strcpy(D_9607[17].key, "\x03");
    D_9607[18].text = "";
    D_9607[0].fn = vec_1941;
    D_9607[1].fn = order_mode_leave_vec;
    D_9607[2].fn = enemy_info_vec;
    D_9607[3].fn = vec_1a45;
    D_9607[4].fn = vec_1a3b;
    D_9607[5].fn = vec_1a31;
    D_9607[6].fn = vec_1a27;
    D_9607[7].fn = vec_1a1d;
    D_9607[8].fn = vec_1a13;
    D_9607[9].fn = vec_1a13;
    D_9607[10].fn = vec_1a09;
    D_9607[11].fn = vec_19ff;
    D_9607[12].fn = vec_19f5;
    D_9607[13].fn = vec_19f5;
    D_9607[14].fn = do_surrender_vec;
    D_9607[15].fn = vec_19e1;
    D_9607[16].fn = vec_19d7;
    D_9607[17].fn = vec_19cd;
    D_97db[0].text = "Throw";
    strcpy(D_97db[0].key, "T>");
    D_97db[0].sub = D_9a47;
    D_97db[1].text = "Std Attack ";
    strcpy(D_97db[1].key, "A");
    D_97db[2].text = "Vulnerable";
    strcpy(D_97db[2].key, "V");
    D_97db[3].text = "Berserk";
    strcpy(D_97db[3].key, "B");
    D_97db[4].text = "Parry";
    strcpy(D_97db[4].key, "P");
    D_97db[5].text = "Use Missile";
    strcpy(D_97db[5].key, "M");
    D_97db[6].text = "";
    for (ik = 0; ik <= 6; ik++)
        D_97db[ik].on = 1;
    for (ik = 0; ik < 20; ik++)
        D_9a47[ik].on = 1;
    D_97db[0].fn = vec_17e3;
    D_97db[1].fn = vec_19c3;
    D_97db[2].fn = vec_19b9;
    D_97db[3].fn = vec_19af;
    D_97db[4].fn = vec_19a5;
    D_97db[5].fn = vec_199b;
    D_9a47[0].fn = vec_1991;
    D_9a47[1].fn = vec_1987;
    D_9a47[2].fn = vec_197d;
    D_9a47[3].fn = vec_1973;
    D_9a47[4].fn = vec_1969;
    D_9a47[5].fn = vec_195f;
    D_9a47[6].fn = vec_1955;
    D_9a47[7].fn = vec_194b;
    D_9a47[8].fn = vec_1ad1;
    D_9a47[9].fn = vec_1ac7;
    D_9a47[10].fn = vec_1abd;
    D_9a47[11].fn = vec_1ab3;
    D_9a47[12].fn = vec_1aa9;
    D_9a47[13].fn = vec_1a9f;
    D_9a47[14].fn = vec_1a95;
    D_9a47[15].fn = vec_1a8b;
    D_9a47[16].fn = vec_1a81;
    D_9a47[17].fn = vec_1a77;
    D_9a47[18].fn = vec_1a6d;
    D_9a47[19].fn = vec_1a63;
    D_987d[0].text = "Party Info";
    D_987d[0].key[0] = 10;
    D_987d[0].key[1] = 0;
    D_987d[1].text = "Change Marching Order";
    strcpy(D_987d[1].key, "\x0fO");
    D_987d[2].text = "";
    D_987d[0].on = 1;
    D_987d[1].on = 1;
    D_987d[0].fn = party_info_screen_vec;
    D_987d[1].fn = choose_party_order_vec;
    battle_menus_enable();
    saved = card_window->x10;
    card_window->x10 = 1;
    xh = 15;
    for (ik = 0; ik <= 3; ik++) {
        D_9c15[ik] = xh;
        xh += text_pixel_width(card_window, D_9419[ik].title) + 8;
        jk = bo = aa = 0;
        while (strlen(D_9419[ik].items[jk].text)) {
            int kk;
            if ((kk = text_pixel_width(card_window, D_9419[ik].items[jk].text)) > aa)
                aa = kk;
            if ((kk = text_pixel_width(card_window, D_9419[ik].items[jk].key)) > bo)
                bo = kk;
            jk++;
        }
        D_9c22[ik] = bo + aa + 9;
    }
    card_window->x10 = saved;
}
#pragma optimize("", on)

/* Orders/Attack menu entries enabled at the start of a battle */
void far battle_menus_enable(void)
{
    int i;

    D_9419[2].x6 = 2;
    for (i = 0; i <= 5; i++)
        D_97db[i].on = 2;
    D_9607[3].on = 1;
    D_9607[5].on = 1;
    strcpy(D_06b3, D_073e);
    D_9607[0].on = 2;
    D_9607[1].on = 2;
    D_9607[2].on = 2;
    D_9607[4].on = 2;
    D_9607[8].on = 2;
    D_9607[9].on = 2;
    if (move_mode == 0)
        strcpy(D_06c5, D_073e);
    else
        strcpy(D_06c5, D_0744);
    strcpy(D_06d7, D_073e);
    strcpy(D_06e8, D_073e);
    strcpy(D_06fe, D_073e);
    D_9607[13].text = D_060c;
    D_9607[10].on = 2;
    D_9607[11].on = 2;
    D_9607[12].on = 2;
    D_9607[13].on = 2;
    D_9607[15].on = 2;
    D_9607[16].on = 2;
    if (move_mode == 0)
        strcpy(D_0737, D_073e);
    else
        strcpy(D_0737, D_0744);
    D_9607[17].on = 2;
}

/* load commonsp.img into EMS and set up the common sprite pointers.
   6.00 keeps the address of the first word (the image size) in SI:DI and reads it again
   for the final copy; 6.00A reloads load_buf (not solved). */
#pragma optimize("a", on)
#pragma optimize("eglc", off)
void far sub_s000_0e76_1010(void)
{
    unsigned far *pb;
    unsigned sege;
    char far *dste;
    int uak, ubl;

    file_load_far("commonsp.img", pb = *(unsigned far **)&load_buf);
    sege = cursor_default = alloc_buffer((unsigned long)*pb++, "CnnmSp");
    ((unsigned *)&dste)[1] = sege;
    ((unsigned *)&dste)[0] = 0;
    cursor_default += *pb++;
    D_a3d6 = *pb++ + sege;
    D_a3e5 = *pb++ + sege;
    D_a2e9 = *pb++ + sege;
    D_a33b = *pb++ + sege;
    D_a33e = *pb++ + sege;
    D_a392 = *pb++ + sege;
    D_a660 = *pb++ + sege;
    D_a662 = *pb++ + sege;
    D_a664 = *pb++ + sege;
    D_a666 = *pb++ + sege;
    D_a626 = *pb++ + sege;
    D_a628 = *pb++ + sege;
    D_a2e5 = *pb++ + sege;
    _fmemcpy(dste, pb, **(unsigned far **)&load_buf);
}
#pragma optimize("", on)

struct mcb { char type; unsigned owner; unsigned size; };

/* memory check: DOS arena walk, free memory vs. what the game needs.
   Locals named for 6.00A's name-hash slot order; uaa/ubc fill the original's unused slots.
   Remaining diff: 6.00 stores the near->far converted fp as `mov [hi],ds; mov [lo],ax`,
   6.00A low word first (pfpc = &fpf only keeps the dead store alive). */
#pragma optimize("e", off)
void far sub_s000_0e76_1150(void)
{
    unsigned sega;
    int pagesa, fpagesl;
    long availf;
    char *msgm;
    struct mcb far *mh;
    long haveb;
    int uaa[2];
    long needl;
    char far **pfpc;
    int ubc;
    int nm;
    char far *fpf;
    long t0i;

    pfpc = &fpf;
    availf = 0;
    nm = 0;
    msgm = (char *)msg_file;
    fpf = (char far *)(char *)nm;
    *(unsigned *)&fpf = nm;     /* w4: re-store of the offset: 6.00A then drops the first offset store and keeps the 6.00 order (segment first) */
    file_save(D_01a6, fpf, 0x42);
    sub_root_05f8_00ca(&pagesa, &fpagesl);
    if (pagesa < 11) {
        printf(msgm + msg_file[19]);
        exit_(0);
    }
    *((unsigned *)&mh + 1) = D_7760 - 1;
    *(unsigned *)&mh = 0;
    needl = (unsigned long)mh->size << 4;
    if (mh->type != 'Z')
        while (nm < 300) {
            if (mh->owner)
                availf += (unsigned long)mh->size << 4;
            *((unsigned *)&mh + 1) += mh->size + 1;
            nm++;
            if (mh->type == 'Z')
                break;
        }
    if (mh->owner)
        availf += (unsigned long)mh->size << 4;
    _dos_allocmem(30000, &sega);
    haveb = ((unsigned long)sega << 4) + availf;
    needl += 0x4b6b0L;
    switch (D_a876[0]) {
    case 'A': case 'a':
        needl += 0x2c90;
        break;
    case 'I': case 'i':
        needl += 0x2fc0;
        break;
    case 'N': case 'n':
        needl += 0x3b0;
        break;
    case 'P': case 'p':
        needl += 0x290c;
        break;
    case 'R': case 'r':
        needl += 0x1c60;
        break;
    }
    bios_set_cursor(14, 0);
    if (haveb - needl < 0 || D_0bf4 == 1) {
        printf(msgm + msg_file[15], needl, needl / 1024);
        printf(msgm + msg_file[16], haveb, haveb / 1024);
        printf(msgm + msg_file[17], needl - haveb, (needl - haveb) / 1024);
    }
    if (haveb - needl < 0) {
        if (D_0bf4 == 1)
            printf(msgm + msg_file[18]);
        else
            exit_(0);
    }
    t0i = time(0);
    while (time(0) - t0i <= 3)
        ;
}
#pragma optimize("", on)
