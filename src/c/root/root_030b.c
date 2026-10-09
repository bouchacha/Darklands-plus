/* place: root 030b:0004 */
/* orig: size=0882 */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* status: FILE MATCH (e2, 2026-10-09): 20/20 (the 17 listed functions + 014e (empty),
 *   0450 (interrupt handler) and 0508 (timer tick), which nothing references directly; all
 *   named sub_root_030b_<offset> so fmatch pairs them by address).
 *   - assert(): <assert.h> with `#line N "player.c"` before each assert (line numbers 125,
 *     126, 127, 156, 210, 222 and the file name are pushed); `#define mseg D_a714` and
 *     `#define cels D_83b6` / `FAB_close` make the stringised expressions right.
 *   - 0150 loop: `D_a71a + *cel` written twice (compiler temp at the bottom of the frame,
 *     no local); 0318: `keep + lim` likewise ([bp-8] is the long CSE of keep); `if (lim <
 *     D_a716) return 1; return 0;` puts the shared `return 0` at the later site.
 *   - 16 unused prototypes between the head/tail externs: symbol-table position decides
 *     which long global is loaded into DX:AX in the compares of 00fe, 0318 and 0508.
 *   - pragma a: 0150, 06b4, 06ce, 07c2; 05ca needs optimize("egl", off) (fail block inline).
 *   - 07c2: pushing the screen offset register for the constant 0 (`push di`) came out of
 *     6.00A unchanged. */
/* PAN animation player ("player.c"; the asserts name the file).  A PAN file is streamed
 * through FAB (root 05d9: 002a open, 006a eof, 0070 read to a linear address, 0059 close)
 * into a DOS memory block (mseg) used as a ring buffer of linear addresses:
 *   D_a732  lin(mseg)            cel offset table (cels+1 words)
 *   D_a726  lin(mseg + 0x80)     ring start
 *   D_a72a  lin(mseg + size)     ring end
 *   D_a72e  fill position (next read)
 *   D_a71a  tail: end of the last complete cel
 *   D_a716  head: next cel to show (advanced by the timer routine 0508)
 *   D_a71e/D_a722  wrap point (new start / old end) after the reader wrapped.
 * Cel records: 'Z' rate pal... (palette + speed), 'B' 1 (compressed screen, root 09c0 1883
 * unpacks it to A000:0000).  07c2 writes a status line to the monochrome screen (B000). */

#include <assert.h>
#include <dos.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>

#define mseg D_a714
#define cels D_83b6
#define FAB_close sub_root_05d9_0059

extern unsigned D_a714;
extern unsigned long D_a716;    /* head */
/* 16 unused prototypes: the symbol-table position decides which of two long globals
   MSC loads into DX:AX for a compare (00fe, 0318, 0508 need head/tail this way). */
int far X_0();
int far X_1();
int far X_2();
int far X_3();
int far X_4();
int far X_5();
int far X_6();
int far X_7();
int far X_8();
int far X_9();
int far X_10();
int far X_11();
int far X_12();
int far X_13();
int far X_14();
int far X_15();
extern unsigned long D_a71a;    /* tail */
extern unsigned long D_a71e;
extern unsigned long D_a722;
extern unsigned long D_a726;    /* ring start */
extern unsigned long D_a72a;    /* ring end */
extern unsigned long D_a72e;    /* fill position */
extern unsigned long D_a732;    /* cel table */
extern int D_a736;
extern int D_a738;
extern int D_a73a;              /* timer routine busy */
extern int D_a73e;
extern char **D_a748;
extern int D_83b6;              /* cels */
extern int D_83b8;              /* frame delay counter */
extern int D_83ba;              /* frame delay */
extern int D_83bc;              /* ticks */
extern int D_83be;              /* underruns */
extern char D_83c0[];
extern char *D_8460;
extern int D_5738;
extern int D_573a;
extern int D_573c;

int far sub_root_05d9_002a(char *);
int far sub_root_05d9_0059(void);
int far sub_root_05d9_006a(void);
unsigned far sub_root_05d9_0070(unsigned long);
char * far sub_root_06a1_2760(char *, int);
void far vec_1879(void);
void far *far vec_1883(void far *, void far *, long);
void far vec_188d(void);
void far vec_1897(int, int, void far *);
void far vec_18a1(void);
void far vec_18ab(void);

void far sub_root_030b_001a(void);
void far sub_root_030b_00fe(void);
void far sub_root_030b_0150(char *name);
int far sub_root_030b_0318(void);
void far sub_root_030b_0465(unsigned long src, unsigned long dst, unsigned n);
unsigned far sub_root_030b_04ce(unsigned long a);
void far sub_root_030b_04fc(void);
unsigned long far sub_root_030b_05ca(unsigned long a);
void far sub_root_030b_06ac(void);
void far sub_root_030b_06b4(char c);
void far sub_root_030b_06ce(char *s);
void far sub_root_030b_06fa(int v, int n);
void far sub_root_030b_0728(int v);
void far sub_root_030b_075a(long v);
void far sub_root_030b_078e(int v, int n);
void far sub_root_030b_07c2(void);

#define LIN2FP(p, l) (FP_SEG(p) = (unsigned)((l) >> 4), FP_OFF(p) = (unsigned)(l) & 15)

/* play the animation <name>.PAN */
void far sub_root_030b_0004(char *name)
{
    sub_root_030b_001a();
    sub_root_030b_0150(name);
    sub_root_030b_00fe();
}

/* allocate the largest free DOS block as the ring buffer */
void far sub_root_030b_001a(void)
{
    unsigned size;

#line 125 "player.c"
    assert(_dos_allocmem ( -1, &size) == 8);
    assert(_dos_allocmem (size, &mseg) == 0);
    assert(size >= 0x2000);
    D_a732 = (unsigned long)mseg << 4;
    D_a726 = (unsigned long)(mseg + 0x80) << 4;
    D_a72a = (unsigned long)(size + mseg) << 4;
    D_a716 = D_a71a = D_a71e = D_a722 = D_a72e = D_a726;
    D_a73a = 1;
}

/* wait until every cel was shown, free the buffer */
void far sub_root_030b_00fe(void)
{
    while (D_a716 != D_a71a)
        sub_root_030b_04fc();
    sub_root_030b_04fc();
#line 156
    assert(_dos_freemem(mseg) == 0);
}

void far sub_root_030b_014e(void)
{
}

/* read the file into the ring buffer, publishing complete cels (D_a71a) */
#pragma optimize("a", on)
void far sub_root_030b_0150(char *name)
{
    char buf[32];
    unsigned far *cel;
    int i0;
    unsigned n;
    register unsigned i;

    strcpy(buf, name);
    strcat(buf, ".");
    *sub_root_06a1_2760(buf, '.') = 0;
    strcat(buf, ".PAN");
    if (sub_root_05d9_002a(buf)) {
        cels = 0;
        i = 0;
        while (!sub_root_05d9_006a()) {
            if (kbhit())
                return;
            if (!sub_root_030b_0318()) {
                i0 = i;
                do
                    sub_root_030b_04fc();
                while (!sub_root_030b_0318());
            }
            n = sub_root_05d9_0070(D_a72e);
            if (cels == 0) {
                cels = sub_root_030b_04ce(D_a72e + 2);
#line 210
                assert(cels <= 1024);
                sub_root_030b_0465(D_a72e + 4, D_a732, cels * 2);
                LIN2FP(cel, D_a732);
            }
            D_a72e += n;
            do {
                if (D_a71a + *cel > D_a72e)
                    break;
                D_a71a += *cel;
                cel++;
            } while (++i <= cels);
        }
#line 222
        assert(FAB_close());
    }
}
#pragma optimize("", on)

/* make room for the next read: 0 = no room yet (the reader would overtake the head);
 * wraps to the ring start, copying the incomplete cel there */
int far sub_root_030b_0318(void)
{
    unsigned long lim;
    register unsigned part;
    register unsigned keep;

    lim = D_a72e + 0xc000;
    if (D_a71a < D_a716) {
        if (lim < D_a716)
            return 1;
        return 0;
    }
    if (lim <= D_a72a)
        return 1;
    lim = D_a726 + 0xc000;
    part = (unsigned)D_a72e - (unsigned)D_a71a;
    keep = cels ? (part > 0x2000 ? part : 0x2000) : 0;
    if (keep + lim >= D_a716)
        return 0;
    sub_root_030b_0465(D_a72e - keep, D_a726, keep);
    _disable();
    D_a722 = D_a71a;
    D_a71a = D_a71e = (D_a72e = keep + D_a726) - part;
    if (D_a722 == D_a716)
        D_a716 = D_a71e;
    _enable();
    return 1;
}

void interrupt far sub_root_030b_0450(void)
{
    D_a73e = 3;
}

/* copy n bytes between linear addresses */
void far sub_root_030b_0465(unsigned long src, unsigned long dst, register unsigned n)
{
    char far *s;
    char far *d;

    LIN2FP(s, src);
    LIN2FP(d, dst);
    while (n--)
        *d++ = *s++;
}

/* word at a linear address */
unsigned far sub_root_030b_04ce(unsigned long a)
{
    unsigned far *p;

    LIN2FP(p, a);
    return *p;
}

void far sub_root_030b_04fc(void)
{
    kbhit();
    D_a73a = 0;
}

/* timer tick: show the next cel when its delay has run out */
void far sub_root_030b_0508(void)
{
    if (D_a73a)
        return;
    D_a73a = 1;
    D_83bc++;
    if (D_83b8 && D_573c)
        vec_18ab();
    else if (!D_83b8 || !--D_83b8) {
        if (D_a716 == D_a71a)
            D_83be++;
        else {
            D_a716 = sub_root_030b_05ca(D_a716);
            if (D_a716 > D_a71a && D_a716 >= D_a722)
                D_a716 = D_a71e;
            D_a736++;
            D_a738++;
            D_83b8 = D_83ba;
            if (D_a748)
                sub_root_030b_07c2();
        }
    }
    D_a73a = 0;
}

/* show the cel at linear address a, return the address after it */
#pragma optimize("egl", off)
unsigned long far sub_root_030b_05ca(unsigned long a)
{
    unsigned char far *p;

    LIN2FP(p, a);
    if (p[0] == 'Z') {
        D_a736 = 0;
        if (D_a748)
            D_a748++;
        D_83ba = 60 / p[1];
        vec_18a1();
        p += (*(int far *)(p + 2) + 0x182) * 2;
        vec_1897(0, 0x100, p - 0x300);
    }
    if (*(int far *)p != 'B' || ((int far *)p)[1] != 1) {
        vec_188d();
        D_a73e = 1;
        return a;
    }
    p = vec_1883(p + 4, (void far *)0xa0000000, 0xfa00L);
    vec_1879();
    return ((unsigned long)FP_SEG(p) << 4) + FP_OFF(p);
}
#pragma optimize("", on)

/* status line text buffer D_83c0, write position D_8460 */
void far sub_root_030b_06ac(void)
{
    D_8460 = D_83c0;
}

#pragma optimize("a", on)
void far sub_root_030b_06b4(char c)
{
    *D_8460++ = c;
    *D_8460 = 0;
}

void far sub_root_030b_06ce(register char *s)
{
    while (*s)
        *D_8460++ = *s++;
    *D_8460 = 0;
}
#pragma optimize("", on)

int D_033c[] = { 0x8000, 0, 10, 100, 1000, 10000 };

/* pad with spaces for a number v of width n */
void far sub_root_030b_06fa(register int v, register int n)
{
    while (D_033c[n--] > v)
        sub_root_030b_06b4(' ');
    *D_8460 = 0;
}

void far sub_root_030b_0728(int v)
{
    itoa(v, D_8460, 10);
    D_8460 = D_83c0 + strlen(D_83c0);
}

void far sub_root_030b_075a(long v)
{
    ltoa(v, D_8460, 16);
    D_8460 = D_83c0 + strlen(D_83c0);
}

/* number v, right aligned in n columns (n > 0) or left aligned in -n */
void far sub_root_030b_078e(register int v, register int n)
{
    if (n > 0)
        sub_root_030b_06fa(v, n);
    sub_root_030b_0728(v);
    if (n < 0) {
        n = -n;
        sub_root_030b_06fa(v, n);
    }
}

/* status line on the monochrome screen */
#pragma optimize("a", on)
void far sub_root_030b_07c2(void)
{
    char far *scr;
    long size;
    long used;

    scr = (char far *)0xb0000000;
    size = D_a72a - D_a726;
    used = D_a71a - D_a716;
    if (used < 0)
        used += D_a722 - D_a71e;
    sub_root_030b_06ac();
    sub_root_030b_06ce("time:");
    sub_root_030b_078e(D_83bc, -4);
    sub_root_030b_06ce(" cels:");
    sub_root_030b_078e(D_a738, -4);
    sub_root_030b_06b4(' ');
    sub_root_030b_06ce(*D_a748);
    sub_root_030b_06b4(':');
    sub_root_030b_078e(D_a736, -3);
    sub_root_030b_06ce(" full:");
    sub_root_030b_078e((int)(used * 100 / size), 2);
    sub_root_030b_06b4('%');
    sub_root_030b_06ce(" head:");
    sub_root_030b_075a(D_a716);
    sub_root_030b_06ce(" tail:");
    sub_root_030b_075a(D_a71a);
    sub_root_030b_06ce(" size:");
    sub_root_030b_075a(size);
    sub_root_030b_06ce(" skip:");
    sub_root_030b_078e(D_83be, 0);
    sub_root_030b_06ce(" PALs:");
    sub_root_030b_078e(D_5738, 0);
    sub_root_030b_06ce(" DACs:");
    sub_root_030b_078e(D_573a, 0);
    sub_root_030b_06ce("      ");
    sub_root_030b_06ac();
    while (*D_8460) {
        *scr = *D_8460++;
        scr += 2;
    }
}
#pragma optimize("", on)
