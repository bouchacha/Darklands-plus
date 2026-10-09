/* place: s028 1f48:0000 */
/* orig: size=0fbc */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* Overlay s028: set-up of the swap stores used by s027 (XMS, EMS, temporary files) and
   the XMS/EMS driver wrappers.  Store types: 2 = XMS, 3 = EMS, 5/6/7 = temporary files.
   D_8d6: low byte = stores allowed (bit t-1 per type), high byte 0xff = initialisation
   failed; D_8d8 = stores available.  The XMS wrappers (0cb2, 0d5a..0f78), EMS wrappers
   (0aee..0c80) and DOS helpers (0626, 06fa, 079c, 08e8, 0942) contain inline assembly in
   the original (call through the XMS entry point, int 67h, int 21h, int 25h, int 2Fh).
   Partly written from build/agent/gen_engine output (dec16). */
struct store { int handle; long size; long base; };    /* D_c09b[type] */
struct hdr { int type; int hash; char name[16]; long size; };
extern struct store D_c09b[];

struct res { int a; int b; };
extern int D_08c2;
extern int D_08ce;
extern int D_08d0;
extern int D_08d2;
extern int D_08d4;
extern int D_08d6;
extern int D_08d8;
extern int D_08da;
extern int D_08e4;
extern int D_08e6;
extern int D_08e8;
extern int D_08ea;
extern int D_08f0;
extern int D_08f2;
extern int D_08f4;
extern int D_08f6;
extern int D_08f8;
extern int D_08fa;
extern int D_c0af;
extern int D_c0b5;
extern int D_c0b7;
extern int D_c0b9;
extern int D_c0bf;
extern int D_c0c1;
extern char D_c0cd[];
extern int D_c0d3;
extern int D_c0d5;
extern char D_c0d7[];
extern int D_c0dd;
extern int D_c0df;
extern char D_c0e1[];
extern int D_c0e7;
extern int D_c0e9;
extern int D_c0f9;
extern int D_c0fb;
extern int D_e23c;
int far _bios_timeofday();
int far _fmemset();
int far getenv();
int far intdosx();
int far ultoa();
void *memset(void *, int, unsigned);
int far sub_root_06a1_13ae();
int far sub_s026_1f09_000c();
int far sub_s026_1ef8_008e(long, void far *, int, long);
extern char far *D_08ca;
int far vec_1e0f();
int far vec_1e19();
int far sub_s028_1f48_0000(void);
int far sub_s028_1f48_00fc(void);
int far sub_s028_1f48_0258(void);
int far sub_s028_1f48_0398(void);
int far sub_s028_1f48_03fe(void);
int far sub_s028_1f48_0470(void);
void far sub_s028_1f48_04ca();
int far sub_s028_1f48_05ae();
void far sub_s028_1f48_0626();
void far sub_s028_1f48_06fa();
void far sub_s028_1f48_079c();
void far sub_s028_1f48_08e8();
void far sub_s028_1f48_08fc();
void far sub_s028_1f48_0942();
void far sub_s028_1f48_09b4();
void far sub_s028_1f48_09fa();
void far sub_s028_1f48_0aee();
void far sub_s028_1f48_0b2c();
void far sub_s028_1f48_0b5a();
void far sub_s028_1f48_0bc0();
void far sub_s028_1f48_0bea();
void far sub_s028_1f48_0c1c();
void far sub_s028_1f48_0c3c();
void far sub_s028_1f48_0c80();
int far sub_s028_1f48_0cb2(void);
int far sub_s028_1f48_0d0a(unsigned, char *);
void far sub_s028_1f48_0d5a();
long far sub_s028_1f48_0d94(void);
void far sub_s028_1f48_0dd6();
void far sub_s028_1f48_0e1a();
int far sub_s028_1f48_0e5e(long);
int far sub_s028_1f48_0e78(int);
void far sub_s028_1f48_0ebc();
void far sub_s028_1f48_0efe();
unsigned far sub_s028_1f48_0f34(int);
void far sub_s028_1f48_0f78();

/* choose the first usable swap store; 1 when there is one */
int far sub_s028_1f48_0000(void)
{
    unsigned t;

    t = D_08d6;
    if (t & 0xff00)
        goto fail;
    if (!D_08d2)
        D_e23c = t;
    D_08d2 = 1;
    if ((t & 1) && sub_s028_1f48_00fc()) {
        D_08da = 2;
        *(long *)&D_08e4 = D_c09b[2].base;
    } else if ((D_08d6 & 2) && sub_s028_1f48_0258()) {
        D_08da = 3;
        *(long *)&D_08e8 = D_c09b[3].base;
    } else {
        if (D_08d6 & 8) {
            vec_1e19();
            if (sub_s028_1f48_0398()) {
                D_08da = 5;
                *(long *)&D_08f0 = D_c09b[5].base;
                vec_1e0f();
                return 1;
            }
            vec_1e0f();
        }
        if ((D_08d6 & 0x10) && sub_s028_1f48_03fe()) {
            D_08da = 6;
            *(long *)&D_08f4 = D_c09b[6].base;
        } else if ((D_08d6 & 0x20) && sub_s028_1f48_0470()) {
            D_08da = 7;
            *(long *)&D_08f8 = D_c09b[7].base;
        } else {
fail:
            if (!D_08d8) {
                D_08d6 = 0xff00;
                return 0;
            }
        }
    }
    return 1;
}

/* set up XMS: allocate the largest block, put the directory into conventional memory
   (when 0x32 paragraphs are free) or into XMS, and write an empty free block plus an
   end marker */
int far sub_s028_1f48_00fc(void)
{
    struct hdr end;
    struct hdr h;
    long size;
    long fr;
    struct store far *fst;
    unsigned seg;

    fst = &D_c09b[2];
    if (!sub_s028_1f48_0cb2())
        return 0;
    fr = sub_s028_1f48_0d94();
    if (!(int)(fr >> 16))
        return 0;
    if ((D_c09b[2].handle = sub_s028_1f48_0e78((int)(fr >> 16))) == 0) {
        D_08d6 ^= 1;
        return 0;
    }
    D_08d6 ^= 1;
    D_08d8 |= 1;
    D_c09b[2].size = (long)(int)(fr >> 16) << 10;
    if ((seg = sub_s028_1f48_0f34(0x32)) != 0) {
        D_08d4 = 1;
        *(long *)&D_08ce = (long)seg << 16;
        _fmemset((char far *)((long)seg << 16), 0, 0x320);
        D_c09b[2].base = 0;
    } else {
        D_08d4 = 2;
        _fmemset(D_08ca, 0, 0x320);
        sub_s026_1ef8_008e(0x320L, D_08ca, D_c09b[2].handle, 0L);
        ((struct store *)&D_c0af)->base = 0x320;
    }
    h.type = 2;
    h.hash = 0;
    h.name[0] = 0;
    size = ((struct store *)&D_c0af)->size - ((struct store *)&D_c0af)->base - 0x30;
    h.size = size;
    sub_s026_1ef8_008e(0x18L, &h, ((struct store *)&D_c0af)->handle, ((struct store *)&D_c0af)->base);
    memset(&end, 0xff, 0x18);
    sub_s026_1ef8_008e(0x18L, &end, ((struct store *)&D_c0af)->handle, ((struct store *)&D_c0af)->size - 0x18);
    return 1;
}

/* sub_s028_1f48_0258: not decompiled: 16-bit read of partial register ax */
int far sub_s028_1f48_0258(void) { return 0; }   /* TODO */

/* sub_s028_1f48_0398: not decompiled: 16-bit read of partial register ax */
int far sub_s028_1f48_0398(void) { return 0; }   /* TODO */

/* sub_s028_1f48_03fe: not decompiled: 16-bit read of partial register ax */
int far sub_s028_1f48_03fe(void) { return 0; }   /* TODO */

/* sub_s028_1f48_0470: not decompiled: 16-bit read of partial register ax */
int far sub_s028_1f48_0470(void) { return 0; }   /* TODO */

/* sub_s028_1f48_04ca: not decompiled: adc/sbb arithmetic */
void far sub_s028_1f48_04ca(void) { }

int far sub_s028_1f48_05ae(void)
{
    if ((char)D_08d8 & 1) {
        sub_s028_1f48_0ebc(D_c0af);
    }
    if (D_08d4 == 1) {
        sub_s028_1f48_0f78(D_08d0);
    }
    if ((char)D_08d8 & 2) {
        sub_s028_1f48_0c80(D_c0b9);
    }
    if ((char)D_08d8 & 8) {
        sub_s028_1f48_09b4(D_c0cd);
    }
    if ((char)D_08d8 & 0x10) {
        sub_s028_1f48_09fa(D_c0d7);
    }
    if ((char)D_08d8 & 0x20) {
        sub_s028_1f48_09b4(D_c0e1);
    }
    D_08d6 = D_e23c;
    D_08d8 = 0;
    return D_08d2 = 0;
}

/* sub_s028_1f48_0626: not decompiled: adc/sbb arithmetic */
void far sub_s028_1f48_0626(void) { }

/* sub_s028_1f48_06fa: not decompiled: unhandled instruction int 0x21 */
void far sub_s028_1f48_06fa(void) { }

/* sub_s028_1f48_079c: not decompiled: unhandled instruction repne scasb al, byte ptr es:[di] */
void far sub_s028_1f48_079c(void) { }

/* sub_s028_1f48_08e8: not decompiled: unhandled instruction int 0x21 */
void far sub_s028_1f48_08e8(void) { }

/* sub_s028_1f48_08fc: not decompiled: memory via unknown si */
void far sub_s028_1f48_08fc(void) { }

/* sub_s028_1f48_0942: not decompiled: unhandled instruction int 0x21 */
void far sub_s028_1f48_0942(void) { }

/* sub_s028_1f48_09b4: not decompiled: memory via unknown si */
void far sub_s028_1f48_09b4(void) { }

/* sub_s028_1f48_09fa: not decompiled: memory via unknown di */
void far sub_s028_1f48_09fa(void) { }

/* sub_s028_1f48_0aee: not decompiled: unhandled instruction int 0x67 */
void far sub_s028_1f48_0aee(void) { }

/* sub_s028_1f48_0b2c: not decompiled: memory via unknown si */
void far sub_s028_1f48_0b2c(void) { }

/* sub_s028_1f48_0b5a: not decompiled: read of undefined dx */
void far sub_s028_1f48_0b5a(void) { }

/* sub_s028_1f48_0bc0: not decompiled: unhandled instruction int 0x67 */
void far sub_s028_1f48_0bc0(void) { }

/* sub_s028_1f48_0bea: not decompiled: unhandled instruction int 0x67 */
void far sub_s028_1f48_0bea(void) { }

/* sub_s028_1f48_0c1c: not decompiled: does not compile */
void far sub_s028_1f48_0c1c(void) { }

/* sub_s028_1f48_0c3c: not decompiled: unhandled instruction int 0x67 */
void far sub_s028_1f48_0c3c(void) { }

/* sub_s028_1f48_0c80: not decompiled: unhandled instruction int 0x67 */
void far sub_s028_1f48_0c80(void) { }

/* sub_s028_1f48_0cb2: not decompiled: read of undefined bx */
int far sub_s028_1f48_0cb2(void) { return 0; }   /* TODO */

/* format a BCD version number (e.g. 0x0300) as "3.00" into buf */
int far sub_s028_1f48_0d0a(unsigned v, char *buf)
{
    char c;

    c = (v >> 8 >> 4 & 0xf) + '0';
    if (c != '0')
        *buf++ = c;
    c = (v >> 8 & 0xf) + '0';
    if (c != '0')
        *buf++ = c;
    *buf++ = '.';
    *buf++ = (v >> 4 & 0xf) + '0';
    *buf++ = (v & 0xf) + '0';
    *buf = 0;
    return 0;
}

/* sub_s028_1f48_0d5a: not decompiled: unhandled instruction int 0x2f */
void far sub_s028_1f48_0d5a(void) { }

/* sub_s028_1f48_0d94: not decompiled: 16-bit read of partial register bx */
long far sub_s028_1f48_0d94(void) { return 0; }   /* TODO (XMS: query free) */

/* sub_s028_1f48_0dd6: not decompiled: 16-bit read of partial register bx */
void far sub_s028_1f48_0dd6(void) { }

/* sub_s028_1f48_0e1a: not decompiled: 16-bit read of partial register bx */
void far sub_s028_1f48_0e1a(void) { }

/* XMS allocate size bytes in 64K units: (size >> 16) + 1 when size is non-zero */
int far sub_s028_1f48_0e5e(long size)
{
    register int n;

    n = (int)(size >> 16);
    if (size)
        n++;
    return sub_s028_1f48_0e78(n);
}

/* sub_s028_1f48_0e78: not decompiled: 16-bit read of partial register bx */
int far sub_s028_1f48_0e78(int kb) { return 0; }   /* TODO (XMS: allocate kb) */

/* sub_s028_1f48_0ebc: not decompiled: 16-bit read of partial register bx */
void far sub_s028_1f48_0ebc(void) { }

/* sub_s028_1f48_0efe: not decompiled: cannot print pair pair(g(49403,), const(0,)) */
void far sub_s028_1f48_0efe(void) { }

/* sub_s028_1f48_0f34: not decompiled: read of undefined bx */
unsigned far sub_s028_1f48_0f34(int n) { return 0; }   /* TODO (XMS: allocate UMB) */

/* sub_s028_1f48_0f78: not decompiled: read of undefined bx */
void far sub_s028_1f48_0f78(void) { }
