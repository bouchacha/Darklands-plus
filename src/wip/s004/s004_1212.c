/* place: s004 1212:0002 */
/* orig: size=011c */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* status (e6, 2026-10-09): FILE MATCH; twin of s026 1ef8 (copied, functions renamed). Original note: 2/2 match (e9). Inline _asm as in the original; local names
   chosen for the 6.00A name-hash slot order. */
/* XMS block moves through the XMS driver entry point (D_c0f9, function 0Bh).
   Both functions contain an inline _asm block in the original. */

struct xmove {                  /* XMS move structure */
    long len;
    int sh;                     /* source handle (0 = conventional memory) */
    long so;                    /* source offset / far pointer */
    int dh;                     /* destination handle */
    long dof;                   /* destination offset / far pointer */
};

extern int D_08c2;              /* XMS present */
extern int D_08c8;              /* last XMS error */
extern void (far *D_c0f9)(void);/* XMS driver entry */

/* copy len bytes from XMS block handle at off to buf; 1 on success */
int far sub_s004_1212_0002(long len, void far *buf, int handle, long off)
{
    void (far *xmsh)(void);
    char padm[12];
    int ra;
    struct xmove md;
    struct xmove far *pmg;

    xmsh = D_c0f9;
    ra = 1;
    pmg = &md;
    if (!D_08c2)
        return 0;
    if (len & 1)
        len++;
    md.len = len;
    md.sh = handle;
    md.so = off;
    md.dh = 0;
    md.dof = (long)buf;
    _asm {
        push ds
        push si
        mov ah, 0Bh
        lds si, pmg
        call xmsh
        pop si
        pop ds
        cmp ax, 0
        je fail
        jmp done
    fail:
        xor bh, bh
        mov D_08c8, bx
    }
    ra = 0;
done:
    return ra;
}

/* copy len bytes from buf to XMS block handle at off; 1 on success */
int far sub_s004_1212_0090(long len, void far *buf, int handle, long off)
{
    void (far *xmsh)(void);
    int ra;
    struct xmove md;
    struct xmove far *pmg;

    xmsh = D_c0f9;
    ra = 1;
    pmg = &md;
    if (!D_08c2)
        return 0;
    if (len & 1)
        len++;
    md.len = len;
    md.sh = 0;
    md.so = (long)buf;
    md.dh = handle;
    md.dof = off;
    _asm {
        push ds
        push si
        mov ah, 0Bh
        lds si, pmg
        call xmsh
        pop si
        pop ds
        cmp ax, 0
        je fail
        jmp done
    fail:
        xor bh, bh
        mov D_08c8, bx
    }
    ra = 0;
done:
    return ra;
}
