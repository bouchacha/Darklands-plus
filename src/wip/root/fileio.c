/* place: root 0617:000e */
/* orig: size=08ac */
/* flags: /AM /Gs /Zp1 */
/* status (e3): 8/29 match (locals renamed for the 6.00A name-hash slot order in 000e/00d4/011e). Functions renamed to their sub_root_0617_ names (old names in the
   comments). This module was not built like the rest of the game: no /G2, loop variables
   spilled back after loops, and `add sp,N` directly before `pop bp` in frame functions without
   locals - a pattern found nowhere else in DARKLAND.EXE, which 6.00A always turns into
   `mov sp,bp`. Default flags (/Ot) come closest; probably compiled with an older MSC (5.x),
   so a FILE MATCH looks out of reach for 0230/024a/0264/027c/0298/04d0/0644 at least.
   Loops (01b2, 045c, 04e2) keep the counter in si and store it back to its stack slot after
   the loop (`mov [bp-2],si`); `register int i` gets the register but not the store-back. */
/* File helpers (root segment 0617): open files directly or from inside the open catalog
   (.CAT) archive, whole-file load/save, and readers that unpack images and fonts through
   the graphics driver's byte stream (refilled from a file or from memory).
   The region's first 5 bytes (0617:0009, push bp/mov bp,sp/pop bp/retf) sit at an odd
   address in front of this module and stay assembly. */
#include "darklands/card.h"
#include "darklands/game.h"

struct catentry {                   /* .CAT directory entry, 24 bytes */
    char name[12];
    long stamp;
    long length;
    long offset;
};

extern int D_6ba8;                  /* handle of the open catalog, -1 = none */
extern int D_8e36;                  /* catalog_find(NULL): entries enumerated so far */
extern unsigned D_8e26;             /* bytes to read from the file opened last (-1 = all) */
extern int D_775a;                  /* errno */
extern int D_8e30;                  /* stream source: file handle */
extern unsigned D_8e32;             /* stream source: memory offset */
extern unsigned D_8e34;             /* stream source: memory segment */
extern char *D_f1fc;                /* stream read pointer */
extern char *D_6ba6;                /* stream buffer end */
extern unsigned (far *D_eeb8)(void); /* stream refill function */
extern char D_effc[];               /* stream buffer, 0x200 bytes */
extern char D_eebc[];               /* one unpacked image line */
extern int D_8e38;                  /* unpacked image: width */
extern int D_8e3a;                  /* unpacked image: lines */
extern int D_8e3c;                  /* unpacked data: length */
extern unsigned char D_8e28[8];     /* font header */

int sub_root_06a1_2c9e(char *, int, int *);          /* _dos_open */
void sub_root_06a1_1b7c(char *);                     /* perror */
unsigned _dos_creat(char *, unsigned, int *);
unsigned _dos_close(int);
unsigned _dos_read(int, void far *, unsigned, unsigned *);
unsigned _dos_write(int, void far *, unsigned, unsigned *);
void exit_(int);                    /* exit(): '_exit_' in symbols/names.txt */
void movedata(unsigned, unsigned, unsigned, unsigned, unsigned);
int strnicmp(const char *, const char *, unsigned);

union REGS {
    struct { unsigned ax, bx, cx, dx, si, di, cflag; } x;
    struct { unsigned char al, ah, bl, bh, cl, ch, dl, dh; } h;
};
int int86(int, union REGS *, union REGS *);
int intdos(union REGS *, union REGS *);

void far sub_root_047c_0de6(int);
void far sub_root_047c_0efe(void);
void far sub_root_047c_0f46(char far *, int);
void far sub_root_047c_0f60(char *);
void far sub_root_047c_054c(char *, int, int, int, int);
void far sub_root_047c_05ae(void);
void far sub_root_047c_05b5(char *);
void far sub_root_047c_05bc(int, int);
char far *far sub_root_047c_0514(int);

#define FP_SEG(fp) (*((unsigned *)&(fp) + 1))
#define FP_OFF(fp) (*((unsigned *)&(fp)))

/* catalog_find */
int far catalog_find(char *name, struct catentry *ent);
/* file_seek */
int far sub_root_0617_0868(int h, long pos);
/* file_error */
void far sub_root_0617_067e(char *name);
/* set_video_mode */
void far sub_root_0617_06a6(int mode);
/* stream_from_file */
void far sub_root_0617_05f0(int h);
/* stream_refill_mem */
unsigned far sub_root_0617_060e(void);
/* stream_refill_file */
unsigned far sub_root_0617_0654(void);

/* open a file for reading, from the catalog when it holds it */
/* file_open */
int far file_open(char *name, int mode)
{
    int hh;
    struct catentry ent;

    if (catalog_find(name, &ent)) {
        if (sub_root_0617_0868(D_6ba8, ent.offset))
            sub_root_0617_067e(name);
        D_8e26 = ent.length;
        return D_6ba8;
    }
    if (sub_root_06a1_2c9e(name, mode, &hh))
        sub_root_0617_067e(name);
    D_8e26 = -1;
    return hh;
}

/* file_create */
int far file_create(char *name)
{
    int h;

    if (_dos_creat(name, 0, &h))
        sub_root_0617_067e(name);
    return h;
}

/* file_close */
void far file_close(int h)
{
    if (h != D_6ba8)
        if (_dos_close(h))
            sub_root_0617_067e(0);
}

/* file_load */
unsigned far file_load(char *name, char *buf)
{
    int h;
    unsigned nb;

    h = file_open(name, 0);
    if (_dos_read(h, (char far *)buf, D_8e26, &nb))
        sub_root_0617_067e(0);
    file_close(h);
    return nb;
}

/* file_load_far */
unsigned far file_load_far(char *name, char far *buf)
{
    int hqn;
    unsigned nqa;

    hqn = file_open(name, 0);
    if (_dos_read(hqn, buf, D_8e26, &nqa))
        sub_root_0617_067e(0);
    file_close(hqn);
    return nqa;
}

/* file_save */
unsigned far file_save(char *name, char far *buf, unsigned len)
{
    int h;

    h = file_create(name);
    if (_dos_write(h, buf, len, &len))
        sub_root_0617_067e(0);
    file_close(h);
    return len;
}

/* unpack a picture file and draw it line by line */
/* pic_draw */
void far pic_draw(int x, int y, int z, char *name, int mode)
{
    int h;
    int i;

    h = file_open(name, 0);
    sub_root_0617_05f0(h);
    sub_root_047c_0de6(mode);
    if (x < 0)
        D_8e3a = 0;
    for (i = 0; i < D_8e3a; i++) {
        sub_root_047c_0f60(D_eebc);
        sub_root_047c_054c(D_eebc, x, y, z + i, D_8e38);
    }
    file_close(h);
}

/* pic_draw_1 */
void far pic_draw_1(int x, char *name)
{
    pic_draw(x, 0, 0, name, 1);
}

/* pic_draw_m */
void far pic_draw_pal(int x, char *name, int mode)
{
    pic_draw(x, 0, 0, name, mode);
}

/* pic_draw_0 */
void far pic_draw_0(int x, char *name)
{
    pic_draw(x, 0, 0, name, 0);
}

/* pic_load_1 */
void far pic_load_1(char *name)
{
    pic_draw(-1, 0, 0, name, 1);
}

/* pic_load_m */
void far pic_load_m(char *name, int mode)
{
    pic_draw(-1, 0, 0, name, mode);
}

/* unpack a whole file to memory */
/* file_unpack */
int far file_unpack(char far *dst, char *name)
{
    int h;

    h = file_open(name, 0);
    sub_root_0617_05f0(h);
    sub_root_047c_0efe();
    sub_root_047c_0f46(dst, D_8e3c);
    file_close(h);
    return D_8e3c;
}

/* load font number idx of a font file into font slot 'slot' */
/* font_load */
int far font_load(int slot, int idx, char *name)
{
    int h;
    unsigned nread;
    unsigned size;
    char far *p;
    unsigned cnt;
    unsigned w;

    if (sub_root_06a1_2c9e(name, 0, &h))
        return D_775a;
    if (_dos_read(h, (char far *)&w, 2, &nread))
        return D_775a;
    D_775a = 2;
    if (idx > w)
        return D_775a;
    if (sub_root_0617_0868(h, (long)(idx * 2)))
        return D_775a;
    if (_dos_read(h, (char far *)&w, 2, &nread))
        return D_775a;
    w -= 8;
    if (sub_root_0617_0868(h, (long)w))
        return D_775a;
    if (_dos_read(h, (char far *)D_8e28, 8, &nread))
        return D_775a;
    cnt = D_8e28[1] - D_8e28[0] + 1;
    size = (D_8e28[4] + D_8e28[6]) * cnt * D_8e28[2] + 8;
    p = sub_root_047c_0514(slot) - 8;
    if (D_8e28[3] == 0) {
        p -= cnt;
        w -= cnt;
        size += cnt;
    }
    if (sub_root_0617_0868(h, (long)w))
        return D_775a;
    if (_dos_read(h, p, size, &nread))
        return D_775a;
    if (_dos_close(h))
        return D_775a;
    return 0;
}

/* pic_show */
void far pic_load(char *name)
{
    int h;
    int i;

    h = file_open(name, 0);
    sub_root_0617_05f0(h);
    sub_root_047c_0de6(0);
    sub_root_047c_05bc(D_8e38, D_8e3a);
    for (i = 0; i < D_8e3a; i++) {
        sub_root_047c_0f60(D_eebc);
        sub_root_047c_05b5(D_eebc);
    }
    file_close(h);
    sub_root_047c_05ae();
}

/* pic_draw_1r */
void far pic_draw_1r(char *name, int x)
{
    pic_draw_1(x, name);
}

/* note: leaves the file open */
/* pic_draw_open */
void far pic_draw_open(char *name, int x)
{
    int h;
    int i;

    h = file_open(name, 0);
    sub_root_0617_05f0(h);
    sub_root_047c_0de6(0);
    for (i = 0; i < D_8e3a; i++) {
        sub_root_047c_0f60(D_eebc);
        sub_root_047c_054c(D_eebc, x, 0, i, D_8e38);
    }
}

/* unpack a picture held in memory at seg:0000 */
/* pic_draw_mem */
void far pic_draw_mem(unsigned seg, int x)
{
    int i;

    D_8e32 = 0;
    D_8e34 = seg;
    D_f1fc = D_6ba6;
    D_eeb8 = sub_root_0617_060e;
    sub_root_047c_0de6(0);
    for (i = 0; i < D_8e3a; i++) {
        sub_root_047c_0f60(D_eebc);
        sub_root_047c_054c(D_eebc, x, 0, i, D_8e38);
    }
}

/* mem_unpack */
int far mem_unpack(char far *src, char far *dst)
{
    int unused;

    D_8e32 = FP_OFF(src);
    D_8e34 = FP_SEG(dst);
    D_f1fc = D_6ba6;
    D_eeb8 = sub_root_0617_060e;
    sub_root_047c_0efe();
    sub_root_047c_0f46(dst, D_8e3c);
    return D_8e3c;
}

void far sub_root_0617_05f0(int h)
{
    D_8e30 = h;
    D_f1fc = D_6ba6;
    D_eeb8 = sub_root_0617_0654;
}

unsigned far sub_root_0617_060e(void)
{
    char far *p;

    p = (char far *)D_effc;
    movedata(D_8e34, D_8e32, FP_SEG(p), FP_OFF(p), 0x200);
    D_f1fc = D_effc;
    D_8e32 += 0x200;
    return 0x200;
}

/* stream_restart_file */
void far stream_restart_file(int h)
{
    D_8e30 = h;
    sub_root_0617_0654();
}

unsigned far sub_root_0617_0654(void)
{
    unsigned n;

    _dos_read(D_8e30, (char far *)D_effc, 0x200, &n);
    D_f1fc = D_effc;
    return n;
}

void far sub_root_0617_067e(char *name)
{
    sub_root_0617_06a6(3);
    sub_root_06a1_1b7c(name);
    exit_(99);
}

void far sub_root_0617_06a6(int mode)
{
    int err;
    union REGS r;

    err = D_775a;
    r.h.ah = 0;
    r.h.al = mode;
    int86(0x10, &r, &r);
    D_775a = err;
}

/* catalog_open */
int far catalog_open(char *name)
{
    int h;

    if (sub_root_06a1_2c9e(name, 0, &h))
        return 0;
    D_6ba8 = h;
    D_8e36 = 0;
    return 1;
}

/* catalog_close */
void far catalog_close(void)
{
    if (D_6ba8 != -1) {
        if (_dos_close(D_6ba8))
            sub_root_0617_067e(0);
        D_6ba8 = -1;
    }
}

/* look name up in the catalog; with name == NULL return the next entry */
int far catalog_find(char *name, struct catentry *ent)
{
    unsigned nread;
    int count;

    if (D_6ba8 == -1)
        return 0;
    if (sub_root_0617_0868(D_6ba8, 0L))
        sub_root_0617_067e(0);
    if (_dos_read(D_6ba8, (char far *)&count, 2, &nread))
        sub_root_0617_067e(0);
    if (name == 0) {
        if (count < ++D_8e36)
            return 0;
        count = D_8e36;
        while (count--)
            if (_dos_read(D_6ba8, (char far *)ent, 0x18, &nread))
                sub_root_0617_067e(0);
        return 1;
    }
    while (count--) {
        if (_dos_read(D_6ba8, (char far *)ent, 0x18, &nread))
            sub_root_0617_067e(0);
        if (strnicmp(ent->name, name, 12) == 0)
            return 1;
    }
    return 0;
}

int far sub_root_0617_0868(int h, long pos)
{
    union REGS r;

    r.x.ax = 0x4200;
    r.x.bx = h;
    r.x.cx = (unsigned)(pos >> 16);
    r.x.dx = (unsigned)pos;
    intdos(&r, &r);
    if (r.x.cflag == 0)
        return 0;
    return r.x.ax;
}
