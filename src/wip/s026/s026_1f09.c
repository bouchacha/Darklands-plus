/* place: s026 1f09:000c */
/* orig: size=0134 */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* status: 2/2 functions match (FILE MATCH, e9). */
/* Temporary swap files: write/read len bytes at offset off of file fd in chunks of at most
   0xffff bytes (DOS int 21h, functions 40h/3Fh, through an inline _asm block as in the
   original). */

long far lseek(int, long, int);

/* write len bytes from buf to file fd at off; 1 on success */
int far sub_s026_1f09_000c(long len, void far *buf, int fd, long off)
{
    unsigned nb;
    int rh;

    if (lseek(fd, off, rh = 0) == -1L)
        return 0;
    if (len & 1)
        len++;
    do {
        if (len < 0x10000L)
            nb = (unsigned)len;
        else
            nb = 0xffff;
        _asm {
            mov bx, fd
            mov cx, nb
            push ds
            lds dx, buf
            mov ah, 40h
            int 21h
            pop ds
            jb fail
        }
        len -= 0x10000L;
        if (len > 0)
            buf = (void far *)((long)buf + 0x10000000L);
    } while (len > 0);
    return 1;
fail:
    return 0;
}

/* read len bytes from file fd at off into buf; 1 on success */
int far sub_s026_1f09_00a6(long len, void far *buf, int fd, long off)
{
    unsigned nb;
    int rh;

    if (lseek(fd, off, rh = 0) == -1L)
        return 0;
    if (len & 1)
        len++;
    do {
        if (len < 0x10000L)
            nb = (unsigned)len;
        else
            nb = 0xffff;
        _asm {
            mov bx, fd
            mov cx, nb
            push ds
            lds dx, buf
            mov ah, 3Fh
            int 21h
            pop ds
            jb fail
        }
        len -= 0x10000L;
        if (len > 0)
            buf = (void far *)((long)buf + 0x10000000L);
    } while (len > 0);
    return 1;
fail:
    return 0;
}
