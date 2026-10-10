/* place: s004 1237:0002 */
/* orig: size=02b0 */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* status (e6, 2026-10-09): FILE MATCH; twin of s026 1f1d (copied, functions renamed). Original note (m0):  `md.sseg = ((unsigned *)&buf)[1]` (not bsegd) keeps the
   segment in cx; the compare direction of both functions follows the symbol table: two
   unused extern declarations before 0000 give the original `cmp ax,cx` / `cmp cx,ax`. */
/* EMS block moves (EMS 4.0 function 57h, move memory region) between conventional memory
   and the EMS swap store.  Every function contains an inline _asm block (int 67h) in the
   original.  A buffer inside the EMS page frame (segment D_c097) is addressed as expanded
   memory of the page-frame handle D_940b; the page map is saved around the move. */

struct emove {                  /* EMS move memory region */
    long len;
    char st;                    /* source: 0 = conventional, 1 = expanded */
    int sh;
    unsigned soff;
    unsigned sseg;              /* segment or logical page */
    char dt;                    /* destination */
    int dh;
    unsigned doff;
    unsigned dseg;
};

extern int D_08be;              /* EMS version (BCD) */
extern int D_08c0;              /* last EMS error */
extern unsigned D_c097;         /* EMS page frame segment */
extern int D_940b;              /* EMS handle of the page frame */

int far sub_s004_1237_027e(int h);
int far sub_s004_1237_0298(int h);

/* unused: two more symbols flip both page-frame compares to the original order */
extern int D_ff00;
extern int D_ff01;

/* copy len bytes from buf to EMS block handle at off; 1 on success */
int far sub_s004_1237_0002(long len, void far *buf, int handle, unsigned long off)
{
    int padk;
    struct emove md;
    unsigned fsegm;
    struct emove far *pmf;
    int rb;
    unsigned bsegd;

    rb = 1;
    pmf = &md;
    if (D_08be < 0x40)
        return 0;
    if (len > 0x100000L)
        return 0;
    if (len & 1)
        len++;
    if ((bsegd = ((unsigned *)&buf)[1]) == (fsegm = D_c097)) {
        md.len = len;
        md.sh = D_940b;
        md.soff = (unsigned)buf & 0x3fff;
        md.sseg = (unsigned)((unsigned long)buf >> 14) & 3;
        md.st = 1;
        md.dt = 1;
    } else {
        md.len = len;
        md.st = 0;
        md.sh = 0;
        md.soff = (unsigned)buf;
        md.sseg = ((unsigned *)&buf)[1];
        md.dt = 1;
    }
    md.dh = handle;
    md.dseg = (unsigned)(off >> 14);
    md.doff = (unsigned)off & 0x3fff;
    sub_s004_1237_027e(D_940b);
    _asm {
        push si
        push ds
        mov ah, 57h
        mov al, 0
        lds si, pmf
        int 67h
        pop ds
        pop si
        cmp ah, 0
        jne fail
        jmp ok
    fail:
        xor al, al
        mov D_08c0, ax
    }
    sub_s004_1237_0298(D_940b);
    return 0;
ok:
    sub_s004_1237_0298(D_940b);
    return rb;
}

/* copy len bytes from EMS block handle at off to buf; 1 on success */
int far sub_s004_1237_0140(long len, void far *buf, int handle, unsigned long off)
{
    int padk;
    struct emove md;
    unsigned fsegm;
    struct emove far *pmf;
    int rb;
    unsigned bsegd;

    rb = 1;
    pmf = &md;
    if (D_08be < 0x40)
        return 0;
    if (len > 0x100000L)
        return 0;
    if (len & 1)
        len++;
    if ((bsegd = ((unsigned *)&buf)[1]) == (fsegm = D_c097)) {
        md.len = len;
        md.dh = D_940b;
        md.doff = (unsigned)buf & 0x3fff;
        md.dseg = (unsigned)((unsigned long)buf >> 14) & 3;
        md.dt = 1;
        md.st = 1;
    } else {
        md.len = len;
        md.dt = 0;
        md.dh = 0;
        md.doff = (unsigned)buf;
        md.dseg = ((unsigned *)&buf)[1];
        md.st = 1;
    }
    md.sh = handle;
    md.sseg = (unsigned)(off >> 14);
    md.soff = (unsigned)off & 0x3fff;
    sub_s004_1237_027e(D_940b);
    _asm {
        push si
        push ds
        mov ah, 57h
        mov al, 0
        lds si, pmf
        int 67h
        pop ds
        pop si
        cmp ah, 0
        jne fail
        jmp ok
    fail:
        xor al, al
        mov D_08c0, ax
    }
    sub_s004_1237_0298(D_940b);
    return 0;
ok:
    sub_s004_1237_0298(D_940b);
    return rb;
}

/* save the EMS page map of handle h (function 47h); 1 on success */
int far sub_s004_1237_027e(int h)
{
    _asm {
        mov ah, 47h
        mov dx, h
        int 67h
        cmp ah, 0
        je ok
    }
    return 0;
ok:
    return 1;
}

/* restore the EMS page map of handle h (function 48h); 1 on success */
int far sub_s004_1237_0298(int h)
{
    _asm {
        mov ah, 48h
        mov dx, h
        int 67h
        cmp ah, 0
        je ok
    }
    return 0;
ok:
    return 1;
}
