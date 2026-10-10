/* place: root 05ba:0004 */
/* orig: size=01f8 _DATA=07fa+0085 */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* DOS memory buffers (root segment 05ba): allocate / free / resize a block of DOS memory,
   given as a byte count, and abort the game with a message when DOS refuses. The block's
   name is written into its memory control block (the 8-byte owner name at MCB+8). */
#include "darklands/card.h"
#include "darklands/game.h"

unsigned far dos_alloc(unsigned long paras);              /* DOS 48h: allocate */
int far dos_free(unsigned seg);                          /* DOS 49h: free */
unsigned far dos_resize(unsigned seg, unsigned long paras); /* DOS 4Ah: resize */
void far dos_print(char *msg);                            /* DOS 09h: print '$' string */
void far game_shutdown(void);
void far F_0000_2652(int);          /* empty function at 0000:2652 (see report) */

int printf(const char *, ...);
void exit_(int);                    /* exit(): '_exit_' in symbols/names.txt */
void far *_fmemcpy(void far *, const void far *, unsigned);
void far *_fmemset(void far *, int, unsigned);
char *strcpy(char *, const char *);
char *strcat(char *, const char *);

#define FP_SEG(fp) (*((unsigned *)&(fp) + 1))
#define FP_OFF(fp) (*((unsigned *)&(fp)))

unsigned far alloc_buffer(unsigned long size, char *name)
{
    unsigned long paras;
    unsigned seg;
    char far *mcb;
    register char *n;

    paras = size >> 4;
    if (size & 0xf)
        paras++;
    n = name;
    seg = dos_alloc(paras);
    if (seg < 0x10) {
        game_shutdown();
        printf("Wanted: %lu - Insufficient system memory  %s AllocBuffer", size, n);
        F_0000_2652(0x32);
        exit_(0);
    }
    FP_SEG(mcb) = seg - 1;
    FP_OFF(mcb) = 0;
    if (n)
        _fmemcpy(mcb + 8, (char far *)n, 8);
    else
        _fmemset(mcb + 8, 0, 8);
    return seg;
}

void far free_buffer(unsigned seg, char *name)
{
    char msg[30];

    if (dos_free(seg)) {
        game_shutdown();
        strcpy(msg, name);
        strcat(msg, " - $");
        dos_print(msg);
        dos_print("Buffer dealloc error$");
        exit_(0);
    }
}

unsigned far realloc_buffer(unsigned seg, unsigned long size, char *name)
{
    char msg[30];
    unsigned long paras;
    unsigned r;

    paras = size >> 4;
    if (size & 0xf)
        paras++;
    r = dos_resize(seg, paras);
    if (r < 0x10) {
        game_shutdown();
        strcpy(msg, name);
        strcat(msg, " - $");
        dos_print(msg);
        dos_print("Insufficient system memory - ReallocBuffer$");
        exit_(0);
    }
    return r;
}
