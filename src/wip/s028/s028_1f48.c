/* place: s028 1f48:0000 */
/* orig: size=0fbc _DATA=2682+003a */
/* compat600-off: sub_s028_1f48_079c */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* status (m3, 2026-10-09): all 36 functions written; 33/36 match.
 *   m3: 0398, 0470, 0e5e match under `#pragma optimize("egl", off)`: without global
 *   optimisation 6.00A honours `register` (0398: kb/drv declaration order = SI/DI; 0e5e: n in
 *   SI with the `add sp,2`) and keeps a stack slot for a local whose value stays in AX
 *   (0470: `int r; if ((r = 08e8()) == -1)` gives the original's `enter 2`).
 *   0626: int 25h RAM-disk probe: device-chain loop shape (orig `sub bx,bx; adc ax,bx`,
 *         separate 0x80/0x20 tests, loop exit placed after the loop) not reproduced.
 *   0b5a: orig keeps the device pointer offset in SI and its segment at [bp-4] with an unused
 *         word above (frame 6); 6.00A gives frame 4 (name dropped) or, with egl off / an
 *         `int name[1]`, the right frame but p in memory or p's segment at [bp-2].
 *   0d5a: 6.00A cross-jumps the two `return ro;` tails (pragmas, volatile, casts, asm `mov
 *         ax,ro` endings tried). */
/* Overlay s028: set-up of the swap stores used by s027 (XMS, EMS, temporary files) and
   the XMS/EMS/DOS wrappers.  Store types: 2 = XMS, 3 = EMS, 5 = file on a RAM disk,
   6 = file in %TMP%/%TEMP%, 7 = heap.fng in the current directory.
   D_8d6: low byte = stores allowed (bit t-1 per type, cleared when tried), 0xff00 =
   initialisation failed; D_8d8 = stores set up; D_8d4 = store holding the directory
   (1 = conventional memory at D_8ce).  Each store starts with the 0x320-byte directory
   (when it holds it), then a free block header and a 0x18-byte end marker at the end; the
   end marker of a temporary file holds its name.
   The XMS wrappers (0cb2, 0d5a..0f78), EMS wrappers (0aee, 0bc0..0c80) and DOS helpers
   (0626 int 25h, 06fa/079c/08e8/0942 int 21h) contain inline _asm blocks, written here
   like the original. */
#include "darklands/card.h"
#include "darklands/game.h"
struct store { int handle; long size; long base; };    /* swap_stores[type] */
struct hdr { int type; int hash; char name[16]; long size; };
extern struct store swap_stores[];

struct WORDREGS { unsigned ax, bx, cx, dx, si, di, cflag; };
struct BYTEREGS { unsigned char al, ah, bl, bh, cl, ch, dl, dh; };
union REGS { struct WORDREGS x; struct BYTEREGS h; };
struct SREGS { unsigned es, cs, ss, ds; };

struct res { int a; int b; };
extern int ems_version;                      /* EMS version (BCD) */
extern int ems_error;                      /* last EMS error */
extern int xms_version;                      /* XMS version, 0 = no XMS */
extern int D_08c4;
extern int D_08c6;
extern int xms_error;                      /* last XMS error */
extern char far *D_08ca;                /* directory buffer */
extern char far *D_08ce;                /* directory in conventional memory (UMB) */
extern unsigned D_08d0;                 /* its segment */
extern int D_08d2;
extern int swap_dir_store;
extern int swap_allowed;
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
extern int D_08fc;                      /* size of a temporary file store in KB */
extern union REGS D_0916, D_0924;
extern struct SREGS D_0932;
extern char far *D_c095;                /* EMS page frame */
extern int D_c0af;
extern int D_c0b9;
extern char D_c0eb[14];                 /* name of the temporary file */
extern void (far *xms_entry)(void);        /* XMS driver entry */
extern int D_e23c;
int far _bios_timeofday(int, long *);
void far *far _fmemset(void far *, int, unsigned);
char *far getenv(const char *);
int far intdosx(union REGS *, union REGS *, struct SREGS *);
char *far ultoa(unsigned long, char *, int);
long far lseek(int, long, int);
int far remove(const char *);
int far toupper(int);
void *memset(void *, int, unsigned);
void *memcpy(void *, const void *, unsigned);
unsigned strlen(const char *);
char *strcpy(char *, const char *);
char *strcat(char *, const char *);
int far sub_root_06a1_12fa(int);
int far sub_root_06a1_13ae(char *, int);
int far sub_root_06a1_2f58(char far *, char far *, int);
int far swapfile_write(long, void far *, int, long);
int far swapfile_read(long, void far *, int, long);
int far xms_write(long, void far *, int, long);
int far ems_write(long, void far *, int, long);
int far vec_1e0f();
int far vec_1e19();
int far swap_init(void);
int far sub_s028_1f48_00fc(void);
int far sub_s028_1f48_0258(void);
int far sub_s028_1f48_0398(void);
int far sub_s028_1f48_03fe(void);
int far sub_s028_1f48_0470(void);
int far sub_s028_1f48_04ca(struct store *);
int far sub_s028_1f48_05ae();
int far sub_s028_1f48_0626(void);
int far sub_s028_1f48_06fa(int, int);
int far sub_s028_1f48_079c(int, int);
int far sub_s028_1f48_08e8(void);
int far sub_s028_1f48_08fc(void);
unsigned far sub_s028_1f48_0942(int);
int far sub_s028_1f48_09b4(struct store *);
int far sub_s028_1f48_09fa(struct store *);
int far ems_present(void);
int far sub_s028_1f48_0b2c(unsigned, char *);
int far sub_s028_1f48_0b5a(void);
struct res far sub_s028_1f48_0bc0(void);
char far *far sub_s028_1f48_0bea(void);
int far sub_s028_1f48_0c1c(long);
int far sub_s028_1f48_0c3c(int);
int far sub_s028_1f48_0c80(int);
int far xms_present(void);
int far sub_s028_1f48_0d0a(unsigned, char *);
int far sub_s028_1f48_0d5a(void);
struct res far sub_s028_1f48_0d94(void);
long far sub_s028_1f48_0dd6(void);
long far sub_s028_1f48_0e1a(void);
int far sub_s028_1f48_0e5e(long);
int far sub_s028_1f48_0e78(int);
int far sub_s028_1f48_0ebc(int);
int far sub_s028_1f48_0efe(void);
unsigned far sub_s028_1f48_0f34(int);
int far sub_s028_1f48_0f78(int);

/* choose the first usable swap store; 1 when there is one */
int far swap_init(void)
{
    unsigned t;

    t = swap_allowed;
    if (t & 0xff00)
        goto fail;
    if (!D_08d2)
        D_e23c = t;
    D_08d2 = 1;
    if ((t & 1) && sub_s028_1f48_00fc()) {
        D_08da = 2;
        *(long *)&D_08e4 = swap_stores[2].base;
    } else if ((swap_allowed & 2) && sub_s028_1f48_0258()) {
        D_08da = 3;
        *(long *)&D_08e8 = swap_stores[3].base;
    } else {
        if (swap_allowed & 8) {
            vec_1e19();
            if (sub_s028_1f48_0398()) {
                D_08da = 5;
                *(long *)&D_08f0 = swap_stores[5].base;
                vec_1e0f();
                return 1;
            }
            vec_1e0f();
        }
        if ((swap_allowed & 0x10) && sub_s028_1f48_03fe()) {
            D_08da = 6;
            *(long *)&D_08f4 = swap_stores[6].base;
        } else if ((swap_allowed & 0x20) && sub_s028_1f48_0470()) {
            D_08da = 7;
            *(long *)&D_08f8 = swap_stores[7].base;
        } else {
fail:
            if (!D_08d8) {
                swap_allowed = 0xff00;
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
    struct res fr;
    struct store far *fst;
    register unsigned seg;

    fst = &swap_stores[2];
    if (!xms_present())
        return 0;
    fr = sub_s028_1f48_0d94();
    if (!fr.b)
        return 0;
    if ((fst->handle = sub_s028_1f48_0e78(fr.b)) == 0) {
        swap_allowed ^= 1;
        return 0;
    }
    swap_allowed ^= 1;
    D_08d8 |= 1;
    fst->size = (long)fr.b << 10;
    if ((seg = sub_s028_1f48_0f34(0x32)) != 0) {
        swap_dir_store = 1;
        D_08ce = (char far *)((long)seg << 16);
        _fmemset((char far *)((long)seg << 16), 0, 0x320);
        fst->base = 0;
    } else {
        swap_dir_store = 2;
        _fmemset(D_08ca, 0, 0x320);
        xms_write(0x320L, D_08ca, fst->handle, 0L);
        fst->base = 0x320;
    }
    h.type = 2;
    h.hash = 0;
    h.name[0] = 0;
    size = fst->size - fst->base - 0x30;
    h.size = size;
    xms_write(0x18L, &h, fst->handle, fst->base);
    memset(&end, 0xff, 0x18);
    xms_write(0x18L, &end, fst->handle, fst->size - 0x18);
    return 1;
}

#pragma optimize("a", on)
/* set up EMS: allocate all free pages, put the directory into EMS unless another store
   holds it, and write an empty free block plus an end marker */
int far sub_s028_1f48_0258(void)
{
    struct hdr end;
    struct hdr h;
    long size;
    struct res fr;
    struct store far *fst;

    fst = &swap_stores[3];
    if (!ems_present())
        return 0;
    fr = sub_s028_1f48_0bc0();
    if ((fst->handle = sub_s028_1f48_0c3c(fr.b)) == -1) {
        swap_allowed ^= 2;
        return 0;
    }
    swap_allowed ^= 2;
    D_08d8 |= 2;
    fst->size = (long)fr.b << 14;
    D_c095 = sub_s028_1f48_0bea();
    if (swap_dir_store == 0) {
        swap_dir_store = 3;
        _fmemset(D_08ca, 0, 0x320);
        ems_write(0x320L, D_08ca, fst->handle, 0L);
        fst->base = 0x320;
    }
    h.type = 2;
    h.hash = 0;
    h.name[0] = 0;
    size = fst->size - fst->base - 0x30;
    h.size = size;
    ems_write(0x18L, &h, fst->handle, fst->base);
    memset(&end, 0xff, 0x18);
    ems_write(0x18L, &end, fst->handle, fst->size - 0x18);
    return 1;
}

#pragma optimize("", on)

/* temporary file on the last RAM disk (type 5) */
#pragma optimize("egl", off)
int far sub_s028_1f48_0398(void)
{
    register unsigned kb;
    register int drv;

    if ((drv = sub_s028_1f48_0626()) == 0)
        return 0;
    kb = sub_s028_1f48_0942(drv);
    if ((swap_stores[5].handle = sub_s028_1f48_06fa(drv, kb)) == 0) {
        swap_allowed ^= 8;
        return 0;
    }
    swap_allowed ^= 8;
    D_08d8 |= 8;
    swap_stores[5].size = (long)kb << 10;
    return sub_s028_1f48_04ca(&swap_stores[5]);
}

/* temporary file in %TMP% / %TEMP% (type 6), at most D_8fc KB */
#pragma optimize("egl", on)
int far sub_s028_1f48_03fe(void)
{
    register int drv;
    register unsigned kb;

    if ((drv = sub_s028_1f48_08fc()) == 0)
        return 0;
    kb = sub_s028_1f48_0942(drv);
    kb = D_08fc < kb ? D_08fc : kb;
    if ((swap_stores[6].handle = sub_s028_1f48_079c(drv, kb)) == 0) {
        swap_allowed ^= 0x10;
        return 0;
    }
    swap_allowed ^= 0x10;
    D_08d8 |= 0x10;
    swap_stores[6].size = (long)kb << 10;
    return sub_s028_1f48_04ca(&swap_stores[6]);
}

/* heap.fng in the current directory (type 7), D_8fc KB */
#pragma optimize("egl", off)
int far sub_s028_1f48_0470(void)
{
    int r;

    if ((r = sub_s028_1f48_08e8()) == -1)
        return 0;
    if ((swap_stores[7].handle = sub_root_06a1_13ae("heap.fng", 0x8102)) == 0) {
        swap_allowed ^= 0x20;
        return 0;
    }
    swap_allowed ^= 0x20;
    D_08d8 |= 0x20;
    swap_stores[7].size = (long)D_08fc << 10;
    return sub_s028_1f48_04ca(&swap_stores[7]);
}

/* initialise a temporary file store: directory (unless another store holds it), free
   block header and an end marker holding the file name */
#pragma optimize("egl", on)
int far sub_s028_1f48_04ca(struct store *st)
{
    struct hdr end;
    struct hdr h;
    register int t;

    t = st - swap_stores;
    if (swap_dir_store == 0) {
        swap_dir_store = t;
        _fmemset(D_08ca, 0, 0x320);
        swapfile_write(0x320L, D_08ca, st->handle, 0L);
        st->base = 0x320;
    }
    h.type = 2;
    h.hash = 0;
    h.name[0] = 0;
    h.size = st->size - st->base - 0x30;
    swapfile_write(0x18L, &h, st->handle, st->base);
    memset(&end, 0xff, 0x18);
    memcpy(end.name, D_c0eb, 14);
    swapfile_write(0x18L, &end, st->handle, st->size - 0x18);
    return 1;
}

int far sub_s028_1f48_05ae(void)
{
    if ((char)D_08d8 & 1) {
        sub_s028_1f48_0ebc(D_c0af);
    }
    if (swap_dir_store == 1) {
        sub_s028_1f48_0f78(D_08d0);
    }
    if ((char)D_08d8 & 2) {
        sub_s028_1f48_0c80(D_c0b9);
    }
    if ((char)D_08d8 & 8) {
        sub_s028_1f48_09b4(&swap_stores[5]);
    }
    if ((char)D_08d8 & 0x10) {
        sub_s028_1f48_09fa(&swap_stores[6]);
    }
    if ((char)D_08d8 & 0x20) {
        sub_s028_1f48_09b4(&swap_stores[7]);
    }
    swap_allowed = D_e23c;
    D_08d8 = 0;
    return D_08d2 = 0;
}

/* find a RAM disk: count the drives of the block device drivers (DOS list of lists),
   then from the last drive down to C: read the boot sector (int 25h) and take the first
   drive whose boot sector has no hidden sectors; 0 = none */
int far sub_s028_1f48_0626(void)
{
    int n;
    int i;
    char far *p;
    char hdr[16];
    char far *lol;

    n = 0;
    D_0916.h.ah = 0x52;
    intdosx(&D_0916, &D_0924, &D_0932);
    lol = (char far *)((long)D_0932.es << 16);
    p = (char far *)((long)lol + (unsigned)(D_0924.x.bx + 0x22));
    for (;;) {
        if ((int)p == -1)
            break;
        for (i = 0; i < 8; i++)
            hdr[i] = (p + 10)[i];
        if (p[5] & 0x80)
            ;
        else if (!(p[5] & 0x20))
            n += hdr[0];
        p = *(char far * far *)p;
    }
    for (i = n - 1; i > 1; i--) {
        _asm {
            mov ax, i
            mov cx, 1
            mov dx, 0
            push ds
            mov bx, SEG D_08ca
            mov ds, bx
            lds bx, D_08ca
            int 25h
            jb bad
            mov ax, [bx+1Ch]
            cmp ax, 0
            jne bad
            popf
            pop ds
            mov ax, i
            jmp found
        bad:
            popf
            pop ds
        }
    }
    return 0;
found:
    return i;
}

/* create the swap file "<drive>:<time>" of kb KB on drive (0 = A:); its handle or 0 */
int far sub_s028_1f48_06fa(int drive, int kb)
{
    int fdf;
    long tm;

    fdf = 0;
    tm = 0;
    if (kb == 0)
        goto fail;
    memset(D_c0eb, 0, 14);
    _bios_timeofday(0, &tm);
    ultoa(tm, D_c0eb + 1, 10);
    D_c0eb[0] = drive + 'A';
    D_c0eb[1] = ':';
    _asm {
        push ds
        mov ax, 3C00h
        mov cx, 0
        mov dx, SEG D_c0eb
        mov ds, dx
        mov dx, offset D_c0eb
        int 21h
        pop ds
        jb fail
        mov fdf, ax
    }
    if (fdf == -1)
        return 0;
    if ((int)lseek(fdf, (long)kb << 10, 0) == -1)
        return 0;
    return fdf;
fail:
    return 0;
}

/* create the swap file %TMP%\<time> of kb KB; its handle or 0 */
int far sub_s028_1f48_079c(int drive, int kb)
{
    char *envg;
    char pathd[64];
    char *ppb;
    int fdi;
    long t;

    fdi = 0;
    t = 0;
    ppb = pathd;
    if (kb == 0)
        goto fail;
    memset(D_c0eb, 0, 14);
    if (!(envg = getenv("TMP")) && !(envg = getenv("TEMP")))
        return 0;
    strcpy(pathd, envg);
    _bios_timeofday(0, &t);
    ultoa(t, D_c0eb, 10);
    if (pathd[strlen(pathd) - 1] != '\\')
        strcat(pathd, "\\");
    strcat(pathd, D_c0eb);
    _asm {
        push ds
        mov ax, 3C00h
        mov cx, 0
        push ss
        pop ds
        mov dx, ppb
        int 21h
        pop ds
        jb fail
        mov fdi, ax
    }
    if (fdi == -1)
        return 0;
    if ((int)lseek(fdi, (long)kb << 10, 0) == -1)
        return 0;
    return fdi;
fail:
    return 0;
}

/* current drive (0 = A:) */
int far sub_s028_1f48_08e8(void)
{
    int d;

    _asm {
        mov ax, 1901h
        int 21h
        xor ah, ah
        mov d, ax
    }
    return d;
}

/* drive of %TMP% / %TEMP% (0 = A:, 0 also when not set) */
int far sub_s028_1f48_08fc(void)
{
    register char *env;

    if (!(env = getenv("TMP")) && !(env = getenv("TEMP")))
        return 0;
    if (env[1] == ':')
        return toupper(*env) - 'A';
    return sub_s028_1f48_08e8();
}

/* free space on drive (0 = A:) in KB */
unsigned far sub_s028_1f48_0942(int drive)
{
    long spck;
    long availc;
    long bpsn;
    long totaln;

    spck = 0;
    availc = 0;
    bpsn = 0;
    totaln = 0;
    if (drive)
        drive++;
    _asm {
        mov ah, 36h
        mov dx, drive
        int 21h
        cmp ax, 0FFFFh
        jne ok
        jmp fail
    ok:
        mov word ptr spck, ax
        mov word ptr availc, bx
        mov word ptr bpsn, cx
    }
    return (unsigned)((unsigned long)(spck * (availc * bpsn)) >> 10);
fail:
    return 0;
}

/* close and delete a temporary file store on the RAM disk */
int far sub_s028_1f48_09b4(struct store *st)
{
    struct hdr end;

    swapfile_read(0x18L, &end, st->handle, st->size - 0x18);
    sub_root_06a1_12fa(st->handle);
    remove(end.name);
    return 1;
}

/* close and delete a temporary file store in %TMP% */
int far sub_s028_1f48_09fa(struct store *st)
{
    char path[64];
    struct hdr end;
    register char *env;

    swapfile_read(0x18L, &end, st->handle, st->size - 0x18);
    if (!(env = getenv("TMP")) && !(env = getenv("TEMP")))
        return 0;
    strcpy(path, env);
    if (path[strlen(path) - 1] != '\\')
        strcat(path, "\\");
    strcat(path, end.name);
    sub_root_06a1_12fa(st->handle);
    remove(path);
    return 1;
}

/* EMS present (driver EMMXXXX0, status ok, version >= 4.0) */
int far ems_present(void)
{
    int r;

    r = 0;
    if (!sub_s028_1f48_0b5a())
        return 0;
    _asm {
        mov ah, 46h
        int 67h
        cmp ah, 0
        je ok
    }
    r = 0;
    goto done;
ok:
    _asm mov ems_version, ax
    r = 1;
    if ((ems_version & 0xf0) < 0x40)
        return 0;
done:
    return r;
}

/* format a BCD version byte (e.g. 0x40) as "4.0" */
int far sub_s028_1f48_0b2c(unsigned v, char *buf)
{
    *buf++ = (v >> 4 & 0xf) + '0';
    *buf++ = '.';
    *buf++ = (v & 0xf) + '0';
    *buf = 0;
    return 0;
}

/* look for the EMS driver (device EMMXXXX0) in the DOS device chain */
int far sub_s028_1f48_0b5a(void)
{
    char far *p;
    char *name;

    name = "EMMXXXX0";

    D_0916.h.ah = 0x52;
    intdosx(&D_0916, &D_0924, &D_0932);
    p = (char far *)(((long)D_0932.es << 16) | (unsigned)(D_0924.x.bx + 0x22));
    while ((int)p != -1) {
        if (sub_root_06a1_2f58("EMMXXXX0", p + 10, 8) == 0)
            return 1;
        p = *(char far * far *)p;
    }
    return 0;
}

/* EMS pages: a = total, b = free */
struct res far sub_s028_1f48_0bc0(void)
{
    struct res r;

    r.b = r.a = 0;
    _asm {
        mov ah, 42h
        int 67h
        cmp ah, 0
        jne err
        mov r.a, dx
        mov r.b, bx
        jmp done
    err:
        mov byte ptr ems_error, ah
    }
done:
    return r;
}

/* EMS page frame */
char far *far sub_s028_1f48_0bea(void)
{
    char far *p;

    p = 0;
    if (ems_version < 0x40)
        return 0;
    _asm {
        mov ah, 41h
        int 67h
        cmp ah, 0
        jne err
        mov word ptr p+2, bx
        jmp done
    err:
        xor al, al
        mov ems_error, ax
    done:
    }
    return p;
}

/* allocate EMS for size bytes (16K pages) */
int far sub_s028_1f48_0c1c(long size)
{
    register int n;

    n = (int)size >> 14;
    if ((int)size & 0x3fff)
        n++;
    return sub_s028_1f48_0c3c(n);
}

/* allocate pages EMS pages; the handle or -1 */
int far sub_s028_1f48_0c3c(int pages)
{
    int pad;
    int h;

    h = 1;
    if (ems_version < 0x40)
        return 0;
    if (pages == 0)
        return -1;
    _asm {
        mov ah, 5Ah
        mov al, 0
        mov bx, pages
        int 67h
        cmp ah, 0
        jne err
        mov h, dx
        jmp done
    err:
        xor al, al
        mov ems_error, ax
    }
    h = -1;
done:
    return h;
}

/* free EMS handle */
int far sub_s028_1f48_0c80(int handle)
{
    int r;

    r = 1;
    if (ems_version < 0x40)
        return 0;
    _asm {
        mov ah, 45h
        mov dx, handle
        int 67h
        cmp ah, 0
        jne err
        jmp done
    err:
        xor al, al
        mov ems_error, ax
    }
    r = 0;
done:
    return r;
}

/* XMS present: get the driver entry and its version (D_8c2) */
int far xms_present(void)
{
    int ro;
    void (far *xmsh)(void);

    ro = 0;
    xmsh = xms_entry;
    if (sub_s028_1f48_0d5a()) {
        xmsh = xms_entry;
        ro = 1;
        _asm {
            mov ah, 0
            call xmsh
            cmp ax, 0
            je err
            mov xms_version, ax
            mov D_08c4, bx
            mov D_08c6, dx
            jmp done
        err:
            xor bh, bh
            mov xms_error, bx
        }
        ro = 0;
    done:
        return ro;
    }
}

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

/* XMS driver installed (int 2Fh 4300h); stores its entry point in xms_entry */
int far sub_s028_1f48_0d5a(void)
{
    int ro;
    void (far *ek)(void);

    ro = 0;
    _asm {
        mov ax, 4300h
        int 2Fh
        cmp al, 80h
        je ok
    }
    return ro;
ok:
    ro = 1;
    _asm {
        mov ax, 4310h
        int 2Fh
        mov word ptr ek, bx
        mov word ptr ek+2, es
    }
    xms_entry = ek;
    return ro;
}

/* XMS free memory in KB: a = total, b = largest block */
struct res far sub_s028_1f48_0d94(void)
{
    struct res ro;
    void (far *xmsh)(void);

    ro.b = ro.a = 0;
    xmsh = xms_entry;
    if (!xms_version)
        return ro;
    _asm {
        mov ah, 8
        call xmsh
        cmp ax, 0
        je err
        mov ro.a, dx
        mov ro.b, ax
        jmp done
    err:
        xor bh, bh
        mov xms_error, bx
    }
done:
    return ro;
}

/* XMS total free memory in KB */
long far sub_s028_1f48_0dd6(void)
{
    long ro;
    void (far *xmsh)(void);

    xmsh = xms_entry;
    if (!xms_version)
        return 0;
    _asm {
        mov ah, 8
        call xmsh
        cmp ax, 0
        je err
        mov word ptr ro, dx
        jmp done
    err:
        xor bh, bh
        mov xms_error, bx
    }
    ro = 0;
done:
    return ro;
}

/* XMS largest free block in KB */
long far sub_s028_1f48_0e1a(void)
{
    long ro;
    void (far *xmsh)(void);

    xmsh = xms_entry;
    if (!xms_version)
        return 0;
    _asm {
        mov ah, 8
        call xmsh
        cmp ax, 0
        je err
        mov word ptr ro, ax
        jmp done
    err:
        xor bh, bh
        mov xms_error, bx
    }
    ro = 0;
done:
    return ro;
}

/* XMS allocate size bytes in 64K units: (size >> 16) + 1 when size is non-zero */
#pragma optimize("egl", off)
int far sub_s028_1f48_0e5e(long size)
{
    register int n;

    n = ((int *)&size)[1];
    if (size)
        n++;
    return sub_s028_1f48_0e78(n);
}

#pragma optimize("egl", on)
/* allocate an XMS block of kb KB; the handle or 0 */
int far sub_s028_1f48_0e78(int kb)
{
    void (far *xmsh)(void);
    int ro;

    xmsh = xms_entry;
    ro = 1;
    if (!xms_version)
        return 0;
    _asm {
        mov ah, 9
        mov dx, kb
        call xmsh
        cmp ax, 0
        je err
        mov ro, dx
        jmp done
    err:
        xor bh, bh
        mov xms_error, bx
    }
    ro = 0;
done:
    return ro;
}

/* free XMS block handle */
int far sub_s028_1f48_0ebc(int handle)
{
    void (far *xmsh)(void);
    int ro;

    xmsh = xms_entry;
    ro = 1;
    if (!xms_version)
        return 0;
    _asm {
        mov ah, 0Ah
        mov dx, handle
        call xmsh
        cmp ax, 0
        je err
        jmp done
    err:
        xor bh, bh
        mov xms_error, bx
    }
    ro = 0;
done:
    return ro;
}

/* size of the largest free upper memory block in paragraphs */
int far sub_s028_1f48_0efe(void)
{
    void (far *xmsh)(void);
    int ro;

    xmsh = xms_entry;
    ro = 1;
    if (!xms_version)
        return 0;
    _asm {
        mov ah, 10h
        mov dx, 0FFFFh
        call xmsh
        mov ro, dx
        jmp done
    }
done:
    return ro;
}

/* allocate an upper memory block of n paragraphs; its segment or 0 */
unsigned far sub_s028_1f48_0f34(int n)
{
    void (far *xmsh)(void);
    unsigned ro;

    xmsh = xms_entry;
    ro = 1;
    if (!xms_version)
        return 0;
    _asm {
        mov ah, 10h
        mov dx, n
        call xmsh
        cmp ax, 0
        je err
        mov ro, bx
        jmp done
    err:
        xor bh, bh
        mov xms_error, bx
    }
    ro = 0;
done:
    return ro;
}

/* free the upper memory block at segment seg */
int far sub_s028_1f48_0f78(int sg)
{
    void (far *xmsh)(void);
    int ro;

    xmsh = xms_entry;
    ro = 1;
    if (!xms_version)
        return 0;
    _asm {
        mov ah, 11h
        mov dx, sg
        call xmsh
        cmp ax, 0
        je err
        mov ro, bx
        jmp done
    err:
        xor bh, bh
        mov xms_error, bx
    }
    ro = 0;
done:
    return ro;
}
