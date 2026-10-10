/* place: s000 0e76:0000 */
/* orig: size=13c8 _DATA=0946+035b */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* Start-up module: EMS error messages, main(), command line / config.drk / sound driver
   set-up (0114), game start (052a).
   status (e9, 2026-10-09): INCOMPLETE.  Written: 0000, main (0022), 0092, 00b8, 083c;
   all five match (main is compiled with `mov sp,bp` after each call by 6.00A as well).  Still stubs: 0114 (args,
   config.drk, sound driver), 052a (graphics/EMS/heap start-up), 083e, 0f04, 1010, 1150.
   _DATA after main's strings: 21 bytes of initialised data at 0bf1 (D_bfb used by 083e),
   then 0114's "rb", "config.drk", "nsound.dlc", int table {'a','p','r','i','n'} at 0c1f
   (sound driver letters, D_a876 = table[config]), "sound.dlc", "Unknown argument: ",
   "\r\n$", ... - the stubs must get their literals in this order. */
struct WORDREGS { unsigned ax, bx, cx, dx, si, di, cflag; };
struct BYTEREGS { unsigned char al, ah, bl, bh, cl, ch, dl, dh; };
union REGS { struct WORDREGS x; struct BYTEREGS h; };

extern int D_0942;
extern char D_907a;
extern char D_907d;
extern int D_a895[];
int far printf(const char *, ...);
void far exit_(int);
int far int86(int, union REGS *, union REGS *);
int far sub_root_044a_00a9(int, int, char *);
int far sub_root_044a_00f4(char *);
int far sub_root_044a_0100(void);
int far sub_root_047c_07ce(char *, char *);
int far sub_root_047c_08c8(int);
int far sub_root_05f8_0064(void);
int far sub_root_0617_011e(char *, void far *);
void far sub_s000_0fb2_0008(void);

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
void far sub_root_0000_1354(void);
void far sub_root_0000_1438(int);
int far sub_root_0265_084e(void *, char *);
void far sub_root_03a1_000a(int);
void far sub_root_047c_0066(void);
int far sub_root_047c_04c7(int);
int far sub_root_047c_04d5(int, int, int, int, int);
void far sub_root_047c_04f8(void);
int far sub_root_047c_051b(int);
void far sub_root_047c_0545(void);
void far sub_root_047c_0576(int, int);
void far sub_root_047c_05a5(void *, int, int, int);
void far sub_root_047c_05ca(int);
void far sub_root_047c_0767(void);
void far sub_root_047c_0783(int, int, int, int, int, int, int);
void far sub_root_047c_078a(int);
void far sub_root_047c_0791(void);
void far sub_root_047c_11f6(void);
void far sub_root_047c_124f(int, int, int);
int far sub_root_05f8_00a9(int *);
void far sub_root_05f8_00ca(int *, int *);
int far sub_root_05f8_00f3(int, int *);
void far sub_root_05f8_0117(int, int, int);
int far sub_root_0617_016a(char *, void far *, int);
void far vec_1937(void);
void far vec_18bf(int, int, int, int, int, int, int, int, int, int, int, int);
void far vec_17e3(void), vec_183d(void), vec_1847(void), vec_1851(void), vec_185b(void);
void far vec_1941(void), vec_194b(void), vec_1955(void), vec_195f(void), vec_1969(void);
void far vec_1973(void), vec_197d(void), vec_1987(void), vec_1991(void), vec_199b(void);
void far vec_19a5(void), vec_19af(void), vec_19b9(void), vec_19c3(void), vec_19cd(void);
void far vec_19d7(void), vec_19e1(void), vec_19eb(void), vec_19f5(void), vec_19ff(void);
void far vec_1a09(void), vec_1a13(void), vec_1a1d(void), vec_1a27(void), vec_1a31(void);
void far vec_1a3b(void), vec_1a45(void), vec_1a4f(void), vec_1a59(void), vec_1a63(void);
void far vec_1a6d(void), vec_1a77(void), vec_1a81(void), vec_1a8b(void), vec_1a95(void);
void far vec_1a9f(void), vec_1aa9(void), vec_1ab3(void), vec_1abd(void), vec_1ac7(void);
void far vec_1ad1(void);
void far sub_root_0000_1080(void), sub_root_0000_1154(void), sub_root_0000_11f2(void);
void far sub_root_0000_129a(void), sub_root_0000_13a0(void), sub_root_0000_1408(void);
void far sub_root_0000_1418(void), sub_root_0000_1428(void);

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
extern struct font *D_0118;
extern struct font *D_017c;
extern char *D_11f2;
extern char D_c107[];
extern char D_060c[], D_067c[], D_0690[], D_06a4[], D_06aa[], D_06ba[], D_06cc[], D_06de[];
extern char D_06f0[], D_05b2[], D_0706[], D_0718[], D_0726[], D_07aa[], D_07bc[];
extern char D_06b3[], D_06c5[], D_06d7[], D_06e8[], D_06fe[], D_0737[], D_073e[], D_0744[];
extern int D_01a6;
extern int D_08ca, D_08cc, D_08d6;
extern char D_9069, D_906a, D_9071, D_9c4c, D_11dd;
extern char D_9c69[][0x80];
extern int D_9417;
extern int D_940b;
extern int D_9c42;
extern int D_a2e5, D_a2e9, D_a33b, D_a33e, D_a392, D_a3d6, D_a3e5;
extern int D_a626, D_a628, D_a660, D_a662, D_a664, D_a666;
extern char D_a6c1;
extern int D_a6c2;
extern int D_a86e, D_a870, D_a872, D_a874;
extern char D_a876[];
extern int D_7760;
extern char D_9085[];
extern char D_c0fd;
extern int D_c0fe, D_c100;
extern char D_c102;
extern int D_c103, D_c105;
extern int far F_20a5_415e[];
extern int far F_20a5_4a32[];
extern int far F_20a5_46d6[];
extern int far F_20a5_45be[];
extern int far F_20a5_491a[];
extern int far F_20a5_4046[];
extern int far F_20a5_4b4a[];
void far sub_s000_0e76_0114(int, char **, char **);
void far sub_s000_0e76_052a(void);
void far sub_s000_0e76_083c(void);
void far sub_s000_0e76_083e(void);
void far sub_s000_0e76_0f04(void);
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
void far sub_s000_0e76_0000(char *msg)
{
    if (D_0942)
        printf("%s: %s\n", msg, D_0ba0[D_0942 - 0x80]);
}

void far main(int argc, char **argv, char **envp)
{
    sub_s000_0fb2_0008();
    sub_root_047c_08c8(sub_root_047c_07ce("misc.exe", 0));
    D_907a = 0;
    sub_root_0617_011e("banner.dat", (void far *)D_a895);
    if (!(D_907d = !sub_root_05f8_0064())) {
        printf("Unable to find EMS memory\n");
        exit_(0);
    }
    sub_s000_0e76_0114(argc, argv, envp);
    sub_root_044a_0100();
    sub_s000_0e76_052a();
}

/* set the cursor position (int 10h function 2) */
void far sub_s000_0e76_0092(char row, char col)
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

/* print the banner (D_a895: offsets of its lines) */
void far sub_s000_0e76_00b8(void)
{
    register int i;

    sub_s000_0e76_0092(12, 0);
    for (i = 12; i; i--)
        sub_root_044a_00f4((char *)D_a895 + D_a895[0]);
    sub_root_044a_00a9(12, 0, (char *)D_a895 + D_a895[11]);
    sub_root_044a_00a9(13, 0, (char *)D_a895 + D_a895[12]);
    sub_root_044a_00a9(14, 0, (char *)D_a895 + D_a895[13]);
}

/* title screen, config.drk (sound driver) and command line options */
void far sub_s000_0e76_0114(int argc, char **argv, char **envp)
{
    unsigned char i;
    char bad;
    FILE *f;
    char *name;
    char buf[64];
    char nm[13];
    union REGS r;

    r.x.ax = 3;
    int86(0x10, &r, &r);
    sub_root_044a_00a9(0, 0, "\xff\x7f");
    sub_root_044a_00a9(1, 0, (char *)D_a895 + D_a895[1]);
    sub_root_044a_00a9(2, 0, (char *)D_a895 + D_a895[2]);
    sub_root_044a_00a9(3, 0, (char *)D_a895 + D_a895[3]);
    sub_root_044a_00a9(4, 0, (char *)D_a895 + D_a895[4]);
    sub_root_044a_00a9(5, 0, (char *)D_a895 + D_a895[5]);
    sub_root_044a_00a9(6, 0, (char *)D_a895 + D_a895[6]);
    sub_root_044a_00a9(7, 0, (char *)D_a895 + D_a895[7]);
    sub_root_044a_00a9(8, 0, (char *)D_a895 + D_a895[8]);
    sub_root_044a_00a9(9, 0, (char *)D_a895 + D_a895[9]);
    sub_root_044a_00a9(10, 0, (char *)D_a895 + D_a895[10]);
    if ((f = fopen("config.drk", "rb")) == 0)
        name = "nsound.dlc";
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
        name = "sound.dlc";
    }
    strcat(D_a876, name);
    for (i = 0; i < argc; i++)
        sub_root_044a_000a(argv[i]);
    sub_s000_0e76_0092(0x16, 0);
    bad = 0;
    for (i = 1; i < argc; i++) {
        if (*argv[i] == '-' || *argv[i] == '/')
            argv[i]++;
        else {
            strcpy(buf, "Unknown argument: ");
            strcat(buf, argv[i]);
            strcat(buf, "\r\n$");
            sub_root_044a_00f4(buf);
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
                sub_root_044a_00f4(buf);
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
            sub_root_044a_00f4(buf);
            bad = 1;
            break;
        }
    }
    if (bad == 1)
        exit_(0);
    sub_s000_0e76_1150();
}

/* game start: graphics, EMS work segment D_c105, swap stores, menus, tables, battle */
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
    if (D_907d == 1 && !sub_root_05f8_00f3(11, &D_940b)) {
        sub_root_05f8_0117(0, 0, D_940b);
        sub_root_05f8_0117(1, 1, D_940b);
        sub_root_05f8_0117(2, 2, D_940b);
        sub_root_05f8_0117(3, 3, D_940b);
        sub_root_05f8_00a9(&seg);
        sub_root_047c_0576(1, seg);
        D_c105 = seg;
    } else
        sub_root_047c_0576(1, D_c105 = sub_root_047c_04c7(1));
    D_c103 = 0;
    *(char far **)&D_08ca = (char far *)D_9085;
    D_08d6 = 0x2b;
    vec_1937();
    sub_s000_0e76_1010();
    sub_root_047c_0545();
    sub_root_047c_05a5(D_0118, 3, 1, D_a626);
    D_a6c2 = sub_root_047c_04d5(1, 6, 0x1f, 10, sub_root_047c_051b(D_0118->x10));
    sub_root_047c_04f8();
    sub_root_047c_124f(0, 0, D_9c42);
    sub_root_047c_08c8(D_9417 = sub_root_047c_07ce(D_a876, 0));
    sub_root_047c_0783(0, 0, 0, D_a870, D_a86e, D_a872, D_a874);
    sub_root_047c_0066();
    D_906a = 1;
    D_9071 = 0;
    D_c102 = D_a6c1 = 0;
    sub_s000_0e76_083e();
    sub_root_044a_019c();
    sub_root_047c_078a(7);
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
            vec_18bf(0, D_0bf6, D_0bf8, D_0bfa[0], D_0bfa[1], D_0bfa[2], D_0bfa[3], D_0bfa[4],
                     D_0bfa[5], D_0bfa[6], D_0bfa[7], D_0bfa[8]);
        else
            vec_18bf(0, 10, 0, 3, 2, 5, 0, 1, 1, -1, 0, 0);
    } else {
        D_9c69[0][0] = D_9c69[1][0] = D_9c69[2][0] = D_9c69[3][0] = D_9c69[4][0] = D_11dd = 0;
        for (;;)
            sub_root_03a1_000a(4);
    }
    sub_root_044a_01f4();
    sub_root_0000_1354();
    sub_root_047c_0791();
    exit_(0);
}

void far sub_s000_0e76_083c(void)
{
}

/* TODO */
void far sub_s000_0e76_083e(void) { }

/* TODO */
void far sub_s000_0e76_0f04(void) { }

/* TODO */
void far sub_s000_0e76_1010(void) { }

/* TODO */
void far sub_s000_0e76_1150(void) { }
