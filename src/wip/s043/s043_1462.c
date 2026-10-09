/* place: s043 1462:0000 */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* status (e2, 2026-10-09): all 6 functions are right; fmatch says 5/6 + DATA DIFFERS only
 * because of two tool limitations (do NOT change the C):
 *   - __AHINCR (huge pointer increment in 09c8, `add [bp-2],1000h`) is not resolved;
 *   - near pointers in initialised data (D_4fce[] -> "very poor"...) are not relocated in
 *     the data comparison (checked: with the table written as ints the data matches).
 * Card-engine overlay start-up / shut-down: loads the world data
 * (darkland.lst items, darkland.cty city record, the location file, darkland.dsc city
 * description, events.tmp), saves locs.tmp/events.tmp on exit, loads the common pictures.
 *
 * This file also DEFINES the shared card strings D_4ec4..D_4fc0 and the item-quality name
 * table D_4fce (used by the card scripts in s044..s135).  MSC emits an initialised global
 * after the literals of the following function, so they are declared before function 0000:
 * DGROUP order = quality names (4e54), 0000's literals (4e88), the globals (4ec4..4fdb),
 * then 0100's literals (4fdc..).
 */

struct res { int a; int b; };

typedef struct _iobuf {
    char *_ptr;
    int _cnt;
    char *_base;
    char _flag;
    char _file;
} FILE;

struct find_t {
    char reserved[21];
    char attrib;
    unsigned wr_time;
    unsigned wr_date;
    long size;
    char name[13];
};

struct WORDREGS { unsigned ax, bx, cx, dx, si, di, cflag; };
struct BYTEREGS { unsigned char al, ah, bl, bh, cl, ch, dl, dh; };
union REGS { struct WORDREGS x; struct BYTEREGS h; };

#define FP_SEG(fp) (*((unsigned far *)&(fp) + 1))
#define FP_OFF(fp) (*((unsigned far *)&(fp)))

extern unsigned D_7760;
extern union REGS D_e792;
extern char D_e3e3[];
extern int D_00e0, D_00e2, D_00e4, D_00e6;
extern char D_01bf;
extern int D_907e;                       /* current city */
extern unsigned char far *D_a889;        /* darkland.lst buffer */
extern unsigned char far *D_a885;        /* location file buffer */
extern char far *D_9785;                 /* city record (0x26e bytes) */
extern char D_978b[];                    /* city description (0x50 bytes) */
extern char D_a668[][4];
extern char *D_1166[];
extern int D_a776;
extern unsigned D_a6e0[];                /* 26 cap pictures */
extern int D_a61e, D_a620, D_a622, D_a624, D_a62a, D_a62e, D_a630, D_a632, D_a65e;
extern int D_a67e;                       /* party size */
extern unsigned D_a2d7[];                /* shield pictures per party member */
extern char D_9c6a[];                    /* party records (128 bytes, +1) */
extern long D_a74a;
extern char huge *D_a74e;

extern char far * far F_2e38_0890[];     /* item definitions (46 bytes each) */
extern char far * far F_2e38_183e[];
extern char far * far F_2e38_3bc4[];
extern char far * far F_2e38_1736[];
extern char far * far F_2e38_36ec[];
extern char far * far F_2e38_10be[];     /* locations (0x3a bytes each), 0x19e of them */
extern char far * far F_2e38_1b2c[];     /* event records (0x30 bytes each), 300 slots */

int printf(const char *, ...);
int fprintf(FILE *, const char *, ...);
int puts(const char *);
FILE *fopen(const char *, const char *);
FILE *fdopen(int, const char *);
int fclose(FILE *);
int fgetc(FILE *);
int fseek(FILE *, long, int);
unsigned fread(void *, unsigned, unsigned, FILE *);
unsigned _dos_findfirst(char *, unsigned, struct find_t *);
unsigned _dos_read(int, void far *, unsigned, unsigned *);
unsigned _dos_write(int, void far *, unsigned, unsigned *);
unsigned _dos_close(int);
unsigned _dos_creat(char *, unsigned, int *);
int int86(int, union REGS *, union REGS *);
void exit_(int);
void srand(unsigned);
long time(long *);

unsigned far alloc_buffer(unsigned long size, char *name);
void far free_buffer(unsigned seg, char *name);
int far sub_root_0617_011e(char *, unsigned char far *);
unsigned far sub_root_06a1_2c9e(char *, unsigned, int *);   /* _dos_open */
char huge * far sub_root_06a1_2114(long, int);              /* halloc */
void far sub_root_047c_078a(int);
void far sub_root_047c_0545(void);
void far sub_root_047c_04f8(void);
int far sub_root_047c_04d5(int, int, int, int, int);
void far sub_root_047c_0584(int);
int far sub_root_0617_045c(char *);
void far sub_root_0617_0230(int, char *);
void far sub_root_0000_1588(int);
void far sub_root_0000_1716(int);
int far vec_1c1b();
void far vec_18d3(void);

void far sub_s043_1462_0100(char *);

char D_4ec4[] = "screwed up.";
char D_4ed0[] = "You'll sleep until dawn.";
char D_4eea[] = "You'll wait for night.";
char D_4f02[] = "No risk.";
char D_4f0c[] = "Select a person and a potion.";
char D_4f2a[] = "Select a person and a saint.";
char D_4f48[] = "Success is assured.";
char D_4f5c[] = "XXX Saint % success msg XXX.";
char D_4f7a[] = "Of course the potion will work!";
char D_4f9a[] = "You can always encounter the unknown";
char D_4fc0[] = "Enter Combat";
char *D_4fce[] = {"very poor", "poor", "fair", "good", "very good", "excellent", "superb"};

/* debug: append a memory report line to MEM.LOG */
#pragma optimize("a", on)
void far sub_s043_1462_0000(char *who, char *mode)
{
    FILE *f;
    unsigned seg;
    char far * far *pp;
    int far *pn;
    int i, cnt, nev;
    unsigned paras;

    seg = D_7760;
    FP_SEG(pp) = seg;
    FP_OFF(pp) = 0x34;
    FP_SEG(pn) = seg;
    FP_OFF(pn) = 0x32;
    for (i = cnt = 0; i < *pn; i++)
        if ((*pp)[i] != -1)
            cnt++;
    D_e792.h.ah = 0x48;
    D_e792.x.bx = 0xffff;
    int86(0x21, &D_e792, &D_e792);
    paras = D_e792.x.bx;
    for (nev = 0, i = 0; i < 300; i++)
        if (F_2e38_1b2c[i])
            nev++;
    f = fopen("MEM.LOG", mode);
    if (f == 0)
        exit_(1);
    fprintf(f, "%8ld %9s %9s fls:%2d evnts:%3d pwr:%d  %d:%d:%d:%d\n", (long)paras * 16, who, D_e3e3, cnt, nev,
            vec_1c1b(), D_00e4, D_00e2, D_00e6, D_00e0);
    fclose(f);
}
#pragma optimize("", on)

/* load the world data for the current city; locfile = the location file */
void far sub_s043_1462_0100(char *locfile)
{
    char *msg;
    struct find_t ft;
    unsigned char far *p;
    unsigned i, n, na, nb;
    int fd;
    FILE *f;
    unsigned nr;
    int nev;

    msg = "Could not open %s file\n";
    if (_dos_findfirst("darkland.lst", 0, &ft) != 0) {
        printf(msg, "list");
        exit_(1);
    } else {
        p = (unsigned char far *)((unsigned long)alloc_buffer(ft.size, "Item memory") << 16);
        D_a889 = p;
        i = sub_root_0617_011e("darkland.lst", p);
        n = *p++;
        na = *p++;
        nb = *p++;
        for (i = 0; i < n; i++) {
            F_2e38_0890[i] = p;
            p += 0x2e;
        }
        for (i = 0; i < na; i++) {
            F_2e38_183e[i] = p;
            while (*p++)
                ;
        }
        for (i = 0; i < na; i++) {
            F_2e38_3bc4[i] = p;
            while (*p++)
                ;
        }
        for (i = 0; i < nb; i++) {
            F_2e38_1736[i] = p;
            while (*p++)
                ;
        }
        for (i = 0; i < nb; i++) {
            F_2e38_36ec[i] = p;
            while (*p++)
                ;
        }
    }
    if (_dos_findfirst("darkland.cty", 0, &ft) != 0) {
        printf(msg, "city");
        exit_(1);
    } else {
        D_9785 = (char far *)((unsigned long)alloc_buffer(0x26eL, "city memory") << 16);
        sub_root_06a1_2c9e("darkland.cty", 0, &fd);
        f = fdopen(fd, "r");
        fseek(f, (long)D_907e * 0x26e + 1, 0);
        _dos_read(fd, D_9785, 0x26e, &i);
        fclose(f);
    }
    if (_dos_findfirst(locfile, 0, &ft) != 0) {
        printf(msg, "location");
        exit_(1);
    } else {
        p = (unsigned char far *)((unsigned long)alloc_buffer(ft.size, "loc memory") << 16);
        D_a885 = p;
        i = sub_root_0617_011e(locfile, p);
        nr = *p++;
        n = *p++;
        n = (n << 8) + nr;
        for (i = 0; i < n; i++) {
            F_2e38_10be[i] = p;
            p += 0x3a;
        }
    }
    if (_dos_findfirst("darkland.dsc", 0, &ft) != 0) {
        printf(msg, "city description");
        exit_(1);
    } else {
        f = fopen("darkland.dsc", "r");
        fgetc(f);
        fseek(f, (long)(D_907e * 0x50 + 1), 0);
        fread(D_978b, 0x50, 1, f);
        fclose(f);
    }
    if (_dos_findfirst("events.tmp", 0, &ft) == 0) {
        sub_root_06a1_2c9e("events.tmp", 0, &fd);
        f = fdopen(fd, "rb");
        fread(&nev, 2, 1, f);
        fseek(f, 2L, 0);
        for (i = 0; i < nev; i++) {
            F_2e38_1b2c[i] = (char far *)((unsigned long)alloc_buffer(0x30L, 0) << 16);
            _dos_read(fd, F_2e38_1b2c[i], 0x30, &nr);
        }
        fclose(f);
    }
}

/* shut-down: write charactr.tmp names back, save locs.tmp and events.tmp, free everything */
void far sub_s043_1462_054e(void)
{
    int fd, fd2;
    unsigned nr, nw;
    int hdr;
    int ids[5];
    int i;
    int cnt, nev;

    sub_root_06a1_2c9e("charactr.tmp", 0, &fd);
    _dos_read(fd, (void far *)&hdr, 2, &nr);
    _dos_read(fd, (void far *)ids, 10, &nr);
    _dos_read(fd, (void far *)D_a668, 0x14, &nr);
    _dos_close(fd);
    for (i = 0; i < 5; i++)
        if (ids[i] != -1)
            strcpy(D_1166[i], D_a668[i]);
    _dos_creat("locs.tmp", 0, &fd2);
    cnt = 0x19e;
    _dos_write(fd2, (void far *)&cnt, 2, &nw);
    for (i = 0; i < 0x19e; i++)
        _dos_write(fd2, F_2e38_10be[i], 0x3a, &nw);
    _dos_close(fd2);
    _dos_creat("events.tmp", 0, &fd2);
    nev = 0;
    for (i = 0; i < 300; i++)
        if (F_2e38_1b2c[i])
            nev++;
    _dos_write(fd2, (void far *)&nev, 2, &nw);
    for (i = 0; i < 300; i++) {
        if (F_2e38_1b2c[i]) {
            _dos_write(fd2, F_2e38_1b2c[i], 0x30, &nw);
            free_buffer(FP_SEG(F_2e38_1b2c[i]), "event");
            F_2e38_1b2c[i] = 0;
        }
    }
    _dos_close(fd2);
    sub_root_047c_078a(1);
    if (D_a776)
        free_buffer(D_a776, "b");
    D_a776 = 0;
    free_buffer(D_a6e0[0], "Caps");
    free_buffer(FP_SEG(D_a889), "S&S");
    free_buffer(FP_SEG(D_9785), "City");
    free_buffer(FP_SEG(D_a885), "Maploc");
    for (i = 0; i < D_a67e; i++)
        if (D_a2d7[i])
            free_buffer(D_a2d7[i], "sg");
}

/* start-up: read charactr.tmp, load the party shields and the common pictures, load the world */
void far sub_s043_1462_078e(void)
{
    int fd;
    unsigned nr;
    int hdr;
    int ids[5];
    char name[20];
    int i;
    unsigned j;

    srand((unsigned)time(0));
    sub_root_06a1_2c9e("charactr.tmp", 0, &fd);
    _dos_read(fd, (void far *)&hdr, 2, &nr);
    _dos_read(fd, (void far *)ids, 10, &nr);
    _dos_read(fd, (void far *)D_a668, 0x14, &nr);
    _dos_close(fd);
    for (i = 0; i < 5; i++) {
        if (ids[i] != -1) {
            sub_root_047c_0545();
            strcpy(name, "pics\\");
            strcat(name, "shield");
            name[11] = D_9c6a[i * 0x80];
            name[12] = 0;
            strcat(name, ".pic");
            D_a2d7[i] = sub_root_0617_045c(name);
            sub_root_047c_04f8();
        }
    }
    sub_root_0617_0230(1, "pics\\Illmcaps.pic");
    sub_root_047c_0545();
    for (j = 0; j < 26; j++)
        D_a6e0[j] = sub_root_047c_04d5(1, j % 15 * 21, j / 15 * 21, 20, 20);
    D_a61e = sub_root_0617_045c("pics\\rpbdrtop.pic");
    D_a620 = sub_root_0617_045c("pics\\rpbdrbtm.pic");
    D_a622 = sub_root_0617_045c("pics\\rpbdrlft.pic");
    D_a624 = sub_root_0617_045c("pics\\rpbdrrgt.pic");
    D_a62a = sub_root_0617_045c("pics\\timer01.pic");
    D_a62e = sub_root_0617_045c("pics\\timer03.pic");
    D_a630 = sub_root_0617_045c("pics\\timer04.pic");
    D_a632 = sub_root_0617_045c("pics\\timer05.pic");
    D_a65e = sub_root_0617_045c("pics\\textback.pic");
    sub_root_047c_04f8();
    sub_root_047c_0584(0x1c0);
    vec_18d3();
    sub_s043_1462_0100("locs.tmp");
    sub_root_0000_1588(0x100);
    sub_root_0000_1716(0x100);
}

void far sub_s043_1462_09ae(int x)
{
    if (D_01bf)
        sub_root_047c_078a(x);
    else
        sub_root_047c_078a(1);
}

/* load a whole file (sound) into a huge buffer D_a74e, size D_a74a */
void far sub_s043_1462_09c8(char *name)
{
    struct find_t ft;
    char huge *p;
    FILE *f;

    if (_dos_findfirst(name, 0, &ft) != 0) {
        printf("snd");
        exit_(1);
    } else {
        p = sub_root_06a1_2114(D_a74a = ft.size, 1);
        if (p == 0) {
            puts("Not enough memory for file");
            D_a74e = 0;
        }
        D_a74e = p;
        f = fopen(name, "rb");
        if (f == 0) {
            printf("Couldn't open %s\n", name);
            D_a74e = 0;
        }
        while (!(f->_flag & 0x10))
            *p++ = fgetc(f);
        fclose(f);
    }
}
