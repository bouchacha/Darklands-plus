/* place: s000 0e76:0000 */
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
int far sub_root_047c_07ce(char *, int);
int far sub_root_047c_08c8(int);
int far sub_root_05f8_0064(void);
int far sub_root_0617_011e(char *, void far *);
void far sub_s000_0fb2_0008(void);
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

/* TODO: command line, config.drk and the sound driver */
void far sub_s000_0e76_0114(int argc, char **argv, char **envp) { }

/* TODO: game start-up */
void far sub_s000_0e76_052a(void) { }

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
