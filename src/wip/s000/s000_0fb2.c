/* place: s000 0fb2:0008 */
/* orig: size=00e8 */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* status (e9): both functions written; 00be matches.  0008: code identical except which
   `return` gets an inline epilogue copy (orig: the D_e28 = 0 exit inline, the D_e2c = 0x24
   exit as `jmp`; 6.00A the other way round; goto/else layouts tried). */
/* Start-up video set-up (EGA/VGA): 0008 asks the BIOS for the display combination
   (int 10h 1Ah..1Ch) and the current video state (int 10h 1Bh) and remembers the text mode
   (D_e2a), the character-generator function (D_e2c: 11h/14h/12h, 22h/23h/24h) and the rows
   (D_e2e); 00be restores that text mode at exit.  Both contain inline _asm (int 10h). */

extern int D_0e28;              /* video state known */
extern int D_0e2a;              /* video mode */
extern int D_0e2c;              /* int 10h AH=11h font function */
extern int D_0e2e;              /* rows */

void far sub_s000_0fb2_0008(void)
{
    int bxvc;
    char infof[64];
    char far *pih;
    int rn;

    pih = infof;
    _asm {
        mov ah, 1Ch
        mov al, 0
        mov cx, 7
        int 10h
        mov bxvc, bx
        mov rn, ax
    }
    if ((rn & 0xff) != 0x1c) {
        D_0e28 = 0;
        return;
    }
    _asm {
        push es
        mov ah, 1Bh
        mov bx, 0
        les di, pih
        int 10h
        pop es
    }
    if ((D_0e2a = infof[4]) <= 3 || D_0e2a == 7) {
        if (infof[0x23] == 8) {
            D_0e2c = 0x12;
            return;
        }
        if (infof[0x23] == 0x0e) {
            D_0e2c = 0x11;
            return;
        }
        if (infof[0x23] == 0x10) {
            D_0e2c = 0x14;
            return;
        }
    } else {
        D_0e2e = infof[0x22];
        if (infof[0x23] == 8) {
            D_0e2c = 0x23;
            return;
        }
        if (infof[0x23] == 0x0e) {
            D_0e2c = 0x22;
            return;
        }
        if (infof[0x23] == 0x10) {
            D_0e2c = 0x24;
            return;
        }
    }
    D_0e2a = 3;
    D_0e2c = 0x12;
}

void far sub_s000_0fb2_00be(void)
{
    int u1j;
    int fontk;
    int model;
    int rowsh;
    int u2m;

    if (D_0e28) {
        fontk = D_0e2c;
        model = D_0e2a;
        rowsh = D_0e2e;
        _asm {
            mov ah, 0
            mov al, byte ptr model
            int 10h
            mov ah, 11h
            mov al, byte ptr fontk
            mov bl, 0
            mov dl, byte ptr rowsh
            int 10h
        }
    }
}
