/* place: s043 17e5:0008 */
/* flags: /AM /Zp1 */
/* status: FILE MATCH. Palette fade helpers (compiled WITHOUT /G2 /Gs /Ox: chkstk + push bp frames). */

struct res { int a; int b; };

extern int D_5734;            /* calibrated busy-wait count per 10 BIOS ticks (0 = not yet measured) */
extern int *D_5732;
extern char *D_5730;          /* palette work buffer (points at D_560a?) */
extern char D_530a[][3];      /* palette read from the VGA DAC */
extern char D_536a[];
extern char D_54ea[];
extern char D_560a[];
extern char D_5304[];

void far sub_root_047c_058b();
unsigned char far sub_s043_1810_0006(unsigned, unsigned, unsigned char, int);
int far sub_s043_17e5_01ec(void);
void far sub_s043_17e5_024c(char *p);

/* fade: a/c = target colour index, b/d = percentage, e = duration */
void far sub_s043_17e5_0008(int a, int b, int c, int d, int e)
{
    int x1g, y1i, x2h, y2i;
    int i, jl, stepsk, saa, sca;
    char *ph;
    unsigned char rgbf[3];

    sub_s043_17e5_024c(D_530a[0]);
    if (D_5734 == 0)
        D_5734 = sub_s043_17e5_01ec();
    stepsk = e * D_5734 / 10;
    stepsk = stepsk < 1 ? 1 : stepsk;
    stepsk = stepsk > 1000 ? 1000 : stepsk;
    b = b > 100 ? 100 : b;
    b = b < 0 ? 0 : b;
    saa = b * 0xffffL / (stepsk * 100);
    d = d > 100 ? 100 : d;
    d = d < 0 ? 0 : d;
    sca = d * 0xffffL / (stepsk * 100);
    x1g = x2h = -1;
    y1i = y2i = 0;
    for (i = 0; i < stepsk; i++) {
        x1g -= saa;
        x2h -= sca;
        y1i += saa;
        y2i += sca;
        *D_5732 = 0x7f20;
        ph = D_536a;
        for (jl = 0; jl < 3; jl++)
            rgbf[jl] = D_530a[a][jl];
        for (jl = 0; jl < 0x120; jl++)
            D_5730[jl] = sub_s043_1810_0006(x1g, y1i, ph[jl], rgbf[jl % 3]);
        sub_root_047c_058b(D_560a);
        *D_5732 = 0xffa0;
        ph = D_54ea;
        for (jl = 0; jl < 3; jl++)
            rgbf[jl] = D_530a[c][jl];
        for (jl = 0; jl < 0x120; jl++)
            D_5730[jl] = sub_s043_1810_0006(x2h, y2i, ph[jl], rgbf[jl % 3]);
        sub_root_047c_058b(D_560a);
    }
}

int far sub_s043_17e5_01ec(void)
{
    unsigned long tm;
    int nb;

    nb = 0;
    tm = *(unsigned long far *)0x0040006cL + 10;
    while (tm > *(unsigned long far *)0x0040006cL) {
        sub_root_047c_058b(D_5304);
        nb++;
    }
    return nb;
}

void far sub_s043_17e5_024c(char *p)
{
    int i;

    for (i = 0; i < 0x100; i++) {
        outp(0x3c7, i);
        *p++ = inp(0x3c9);
        *p++ = inp(0x3c9);
        *p++ = inp(0x3c9);
    }
}
