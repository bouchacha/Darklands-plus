/* place: s030 1767:000e */
/* orig: size=010e _DATA=2a3a+001b */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* Low-level system helpers used by the overland map overlay (s030):
   BIOS video mode, exit to DOS, timed delay, vertical-retrace wait,
   free-memory report and byte-order swaps. */

struct WORDREGS { unsigned ax, bx, cx, dx, si, di, cflag; };
struct BYTEREGS { unsigned char al, ah, bl, bh, cl, ch, dl, dh; };
union REGS { struct WORDREGS x; struct BYTEREGS h; };

extern union REGS D_e792;           /* shared in/out register block for int86 */

int int86(int, union REGS *, union REGS *);
long clock(void);
int fcloseall(void);
int printf(const char *, ...);
int inp(unsigned);                  /* intrinsic under /Oi -> in al,dx */

void far sub_root_044a_01f4(void);
void far sub_root_047c_123f(void);

/* BIOS int 10h, AH=0: set video mode */
void far set_video_mode(char mode)
{
    D_e792.h.ah = 0;
    D_e792.h.al = mode;
    int86(0x10, &D_e792, &D_e792);
}

/* shut down subsystems, close files and return to 80x25 text mode */
void far shutdown_to_text(void)
{
    sub_root_044a_01f4();
    sub_root_047c_123f();
    fcloseall();
    set_video_mode(3);
}

/* busy-wait for the given number of clock() ticks */
void far delay_ticks(long ticks)
{
    long until;
    until = clock() + ticks;
    while (clock() < until)
        ;
}

/* wait for n vertical retraces (VGA input status register 3DAh, bit 3) */
void far wait_vretrace(int n)
{
    while (n--) {
        while (!(inp(0x3da) & 8))
            ;
        while (inp(0x3da) & 8)
            ;
    }
}

/* DOS int 21h AH=48h with BX=FFFF fails and returns the largest free block */
void far print_free_memory(void)
{
    D_e792.h.ah = 0x48;
    D_e792.x.bx = 0xffff;
    int86(0x21, &D_e792, &D_e792);
    printf("\x1b[s%8ld bytes available\x1b[u", (unsigned long)D_e792.x.bx << 4);
}

/* reverse byte order of a 16-bit value in place */
void far swap16(unsigned char *p)
{
    unsigned char t;
    t = p[0];
    p[0] = p[1];
    p[1] = t;
}

/* reverse byte order of a 32-bit value in place */
void far swap32(unsigned char *p)
{
    unsigned char t;
    t = p[0];
    p[0] = p[3];
    p[3] = t;
    t = p[1];
    p[1] = p[2];
    p[2] = t;
}
