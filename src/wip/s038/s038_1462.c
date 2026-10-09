/* place: s038 1462:0000 */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* status: 5/7 match (0292 23ec 25ca 01fe 024e; _DATA complete once 02f8's literals are in).
   02f8 (8.4 KB) written in full, 8428/8436 bytes, frame/slots match (hash-ordered local names,
   fillers x1g/x2h/x3l), all ten colour jump tables match. Left (6.00 vs 6.00A, see e4 NOTES):
     - 'A' case: 6.00 placed the `if (392c) {..}` block of the party-search loop BEFORE the
       loop increment (`jmp 097f` / `jmp 08fa`); 6.00A emits it inline (goto version: no help);
     - i*2 kept in DI across calls (`mov di,bx` before _dos_freemem, push di/pop di around the
       inline strcpy/strcat in 'H'); pragma "a" gives the DI CSE but breaks the colour tables;
     - cross-jump direction: 6.00 keeps the 'A' no-free-slot copy of 0553(..)+1293() and 'H'/'S'
       jump back into it (`jmp 09c3`), 6.00A keeps the later copy; 'C' then jumps to `call 1293`;
     - `if (D_a67e <= i) D_a67e = i + 1;` operand order (`mov ax,i; cmp [a67e],ax; jg`); struct
       wrapper and declpos search did not help.
   0000 differs only in the frame (key at [bp-4], row di / k si).
   Character creation main menu (pics\cretscr3.pic): list of saved characters from charactr.tmp.

   charactr.tmp layout: int count; int ids[5] (party slot -> character index, -1 = empty);
   char shield[5][4] (D_a668, "F01"/"F60"/"C00"/"A00" + 0); then count records of 0x22a bytes:
   0x80-byte member record (read to D_9c55 + slot*0x80, i.e. D_9c69[slot] - 0x14) + far tables
   F_2e38_082c[0x14], F_2e38_07be[0x16], F_2e38_003e[0x180] (cf. D_8d2e / D_9085 in 23ec).
   02f8 keys: up/left/right & down = previous/next character (portrait palette from
   F_20a5_33fc[slot], 24 bytes = 8 RGB triples, copied to D_e753..D_e76a); R = leave;
   A = add/remove party member (sub_s029_0e76_392c/3cde/2a32 with 0x4c); C = new character
   (vec_20cb); H = next shield letter (D_9c69[slot][1] 'A'..'O', pics\shieldX.pic);
   S = next shield code (D_e7e2[D_e7e0], D_e7de codes); 1/2/3 = cycle the colours of shield
   parts (D_e890/D_e892/D_e894, palette bytes depend on the shield code); Alt-Q = quit game. */

struct res { int a; int b; };
extern int D_907e;   /* position matters: compare direction in 23ec */
struct font { char _p0[0xc]; int color; };
/* character record as stored in charactr.tmp (0x80 bytes, cf. party member records D_9c69) */
struct chr { int id; char _p2[0x12]; char status; char _p15[0x10]; char name[0x5b]; };
extern struct chr D_8d2e;
extern int D_8d06[];
extern char D_9085[];
extern char D_a668[][4];      /* shield code per party slot ("F01", ...) */
extern char D_9c69[][0x80];
extern struct { char _p[0x1ddf]; char far *s[0x41]; } far F_290e_0000;
int far sub_root_0000_1588();
int far sub_root_0000_1716();
int far sub_root_047c_055a();
int far _dos_close();
extern char D_27f4[];
extern int D_2804;
extern int D_2812;
extern int D_2816;
extern volatile int D_6ef0, D_6eec, D_6eee;
extern char D_9419[];
extern char D_9920;
extern char D_9934;
extern int D_9c46;
extern char D_e48c[];
extern int D_e7a0;
extern int D_e7a2;
extern int D_e7da;
extern int D_e7dc;
extern int D_e88a;
extern int D_e88e;
typedef struct { char _p[16]; } FILE;
extern char D_e753, D_e754, D_e755, D_e756, D_e757, D_e758, D_e759, D_e75a, D_e75b, D_e75c,
    D_e75d, D_e75e, D_e75f, D_e760, D_e761, D_e762, D_e763, D_e764, D_e765, D_e766, D_e767,
    D_e768, D_e769, D_e76a;     /* current portrait palette (24 bytes) */
extern char D_e7e2[][4];        /* shield codes */
extern int D_e7de, D_e7e0;      /* number of shield codes, current one */
extern int D_e890, D_e892, D_e894;  /* colour indices for keys 1/2/3 */
extern int D_2800;
extern unsigned D_a2d7[];
extern char D_9c55[];
extern struct { int v; } D_a67e;   /* number of party members (struct: compare operand order) */
extern char far F_20a5_33fc[5][0x18];   /* palette per party slot */
extern char far F_2e38_082c[][0x14];
extern char far F_2e38_07be[][0x16];
extern char far F_2e38_003e[][0x180];
FILE *far fdopen();
int far fread();
int far fclose();
int far fseek();
int far _dos_write();
int memcmp();
char *strcpy();
char *strcat();
int far sub_root_0000_13a0();
int far sub_s029_0e76_2a32();
int far sub_s029_0e76_3cde();
int far vec_2049();
int far vec_20cb();
int far _dos_freemem();
int far _dos_read();
int far sub_root_0000_0000();
int far sub_root_0265_0004();
int far sub_root_047c_04dc();
int far sub_root_047c_04f8();
int far sub_root_047c_0545();
int far sub_root_047c_0553();
int far sub_root_047c_0568();
int far sub_root_047c_056f();
int far sub_root_047c_058b();
int far sub_root_047c_05a5();
int far sub_root_047c_073d();
int far sub_root_047c_0744();
int far sub_root_047c_1293();
int far sub_root_047c_12c3();
int far sub_root_047c_1320();
int far sub_root_05ba_00c2();
int far sub_root_0617_024a();
int far sub_root_0617_045c();
int far sub_root_06a1_2c9e();
int far sub_s029_0e76_392c();
int far vec_20ad();
int far vec_20c1();
int far sub_s038_1462_0000();
int far sub_s038_1462_01fe();
int far sub_s038_1462_024e();
void far sub_s038_1462_0292();
int far sub_s038_1462_02f8();
void far sub_s038_1462_23ec(struct font *);
void far sub_s038_1462_25ca(struct font *);

int far sub_s038_1462_0000(void)
{
    volatile int key;   /* TODO: original key at [bp-4], row in di, k in si; 6.00A shares si for row/k */
    int row;
    int k;
    sub_root_047c_0568();
    D_9920 = 2;
    D_9934 = 2;
    D_e7a2 = D_2816;
    D_e7a0 = D_2804;
    D_e88e = 0;
    vec_20c1();
    sub_s038_1462_024e();
    sub_s038_1462_0292();
    sub_root_047c_056f();
    while (D_6ef0 & 1)
        ;
    do {
        key = 0;
        if ((D_6ef0 & 1) && D_6eec > 0x46 && D_6eec < 0xc8 && D_6eee > 0x2d && D_6eee < 0xa5) {
            while (D_6ef0 & 1)
                ;
            switch (row = (D_6eee - 0x2d) / 15) {
            case 0:
                key = 0x2e43;
                break;
            case 1:
                key = 0x1e41;
                break;
            case 2:
                key = 0x2348;
                break;
            case 3:
                key = 0x1f53;
                break;
            case 4:
                key = 0x231;
                break;
            case 5:
                key = 0x332;
                break;
            case 6:
                key = 0x433;
                break;
            case 7:
                key = 0x11b;
                break;
            }
        } else if ((D_6ef0 & 1) && D_6eec > 0xc5 && D_6eec < 0x138 && D_6eee > 0x14 && D_6eee < 0xb4) {
            while (D_6ef0 & 1)
                ;
            row = (D_6eee - 0x14) / 8;
            if (D_e7dc > row) {
                sub_root_047c_12c3();
                sub_root_047c_04dc(D_27f4, 0xc6, (D_e7da << 3) + 0x16, 0x71, 8, 0x12, 0x10);
                D_e7da = row;
                sub_root_047c_04dc(D_27f4, 0xc6, (row << 3) + 0x16, 0x71, 8, 0x10, 0x12);
                sub_root_047c_1293();
            }
        }
        sub_root_0000_0000(D_9419, 0);
        k = key;
        if (D_9c46) {
            D_9c46 = 0;
            sub_root_047c_0568();
            sub_s038_1462_024e();
            sub_s038_1462_0292();
            sub_root_047c_056f();
            k = 0;
        }
        if (sub_root_047c_073d())
            k = sub_root_047c_0744();
            if (k == 0x11b || k == 0x1352 || k == 0x1372)
            return sub_s038_1462_01fe(0);
    } while (!k || sub_s038_1462_02f8(k) != 0x11b);
    return sub_s038_1462_01fe(1);
}

int far sub_s038_1462_01fe(int p_6)
{
    if (D_e88e) {
        _dos_freemem(D_e88e);
    }
    sub_root_05ba_00c2(D_e88a, "b");
    D_2816 = D_e7a2;
    D_2804 = D_e7a0;
    D_9920 = 1;
    D_9934 = 1;
    do {
    } while (D_6ef0 & 1);
    sub_root_047c_1320(6, 0, 0, 0);
    return p_6;
}

int far sub_s038_1462_024e(void)
{
    sub_root_0617_024a(1, "pics\\cretscr3.pic", D_e48c);
    sub_root_047c_0545();
    D_e88a = sub_root_0617_045c("pics\\buttnCR2.pic");
    sub_root_047c_04f8();
    sub_root_047c_058b(D_e48c);
    D_2812 = 8;
    D_2816 = 1;
    return D_2804 = 1;
}

void far sub_s038_1462_0292(void)
{
    int i;
    sub_s038_1462_25ca((struct font *)0x2806);
    sub_s038_1462_23ec((struct font *)0x2806);
    sub_root_047c_12c3();
    sub_root_047c_0553(0x2806, 0, 0, 0x140, 0xc8, D_27f4, 0, 0);
    for (i = 0; i < 5; i++) {
        if (D_9c69[i][0]) {
            sub_root_0000_1588(i);
            sub_root_0000_1716(i);
        }
    }
    sub_root_047c_1293();
}

/* character-creation menu commands (key code from 0000); returns 0x11b to leave the menu */
int far sub_s038_1462_02f8(register int key)
{
    int x1g;
    int fhc;
    long x2h;
    FILE *fpm;
    int nrd;
    int il;
    int hn;
    int x3l;
    int idsh[5];
    char nameh[20];
    switch (key) {
    case 0x4800: case 0x4b00: case 0x4d00:
        sub_root_047c_04dc(D_27f4, 0xc6, (D_e7da << 3) + 0x16, 0x71, hn = 8, 0x12, 0x10);
        if (--D_e7da < 0)
            D_e7da = D_e7dc - 1;
        sub_root_047c_04dc(D_27f4, 0xc6, hn * D_e7da + 0x16, 0x71, hn, 0x10, 0x12);
        sub_root_047c_0553(0x2806, 0x46, 0xa5, 0x23, 0x23, D_27f4, 0x46, 0xa5);
        sub_root_06a1_2c9e("charactr.tmp", 2, &fhc);
        fpm = fdopen(fhc, "r+b");
        _dos_read(fhc, (int far *)&il, 2, &nrd);
        _dos_read(fhc, (int far *)idsh, 10, &nrd);
        _dos_read(fhc, (char far *)D_a668, 0x14, &nrd);
        fclose(fpm);
        for (il = 0; il < 4; il++)
            if (idsh[il] == D_e7da)
                break;
        if (il < 4) {
            if (D_e88e)
                _dos_freemem(D_e88e);
            sub_root_047c_0545();
            strcpy(nameh, "pics\\");
            strcat(nameh, D_e7e2[D_e7da]);
            strcat(nameh, "small.pic");
            D_e88e = sub_root_0617_045c(nameh);
            sub_root_047c_04f8();
            D_e753 = F_20a5_33fc[il][0];
            D_e754 = F_20a5_33fc[il][1];
            D_e755 = F_20a5_33fc[il][2];
            D_e756 = F_20a5_33fc[il][3];
            D_e757 = F_20a5_33fc[il][4];
            D_e758 = F_20a5_33fc[il][5];
            D_e759 = F_20a5_33fc[il][6];
            D_e75a = F_20a5_33fc[il][7];
            D_e75b = F_20a5_33fc[il][8];
            D_e75c = F_20a5_33fc[il][9];
            D_e75d = F_20a5_33fc[il][10];
            D_e75e = F_20a5_33fc[il][11];
            D_e75f = F_20a5_33fc[il][12];
            D_e760 = F_20a5_33fc[il][13];
            D_e761 = F_20a5_33fc[il][14];
            D_e762 = F_20a5_33fc[il][15];
            D_e763 = F_20a5_33fc[il][16];
            D_e764 = F_20a5_33fc[il][17];
            D_e765 = F_20a5_33fc[il][18];
            D_e766 = F_20a5_33fc[il][19];
            D_e767 = F_20a5_33fc[il][20];
            D_e768 = F_20a5_33fc[il][21];
            D_e769 = F_20a5_33fc[il][22];
            D_e76a = F_20a5_33fc[il][23];
            sub_root_047c_058b(D_e48c);
            sub_root_047c_05a5(D_27f4, 0xc, 0xa4, D_e88e);
        }
        break;
    case 0x5000:
        sub_root_047c_04dc(D_27f4, 0xc6, (D_e7da << 3) + 0x16, 0x71, hn = 8, 0x12, 0x10);
        if (++D_e7da >= D_e7dc)
            D_e7da = 0;
        sub_root_047c_04dc(D_27f4, 0xc6, hn * D_e7da + 0x16, 0x71, hn, 0x10, 0x12);
        sub_root_047c_0553(0x2806, 0x46, 0xa5, 0x23, 0x23, D_27f4, 0x46, 0xa5);
        sub_root_06a1_2c9e("charactr.tmp", 2, &fhc);
        fpm = fdopen(fhc, "r+b");
        _dos_read(fhc, (int far *)&il, 2, &nrd);
        _dos_read(fhc, (int far *)idsh, 10, &nrd);
        _dos_read(fhc, (char far *)D_a668, 0x14, &nrd);
        fclose(fpm);
        for (il = 0; il < 4; il++)
            if (idsh[il] == D_e7da)
                break;
        if (il < 4) {
            if (D_e88e)
                _dos_freemem(D_e88e);
            sub_root_047c_0545();
            strcpy(nameh, "pics\\");
            strcat(nameh, D_e7e2[D_e7da]);
            strcat(nameh, "small.pic");
            D_e88e = sub_root_0617_045c(nameh);
            sub_root_047c_04f8();
            D_e753 = F_20a5_33fc[il][0];
            D_e754 = F_20a5_33fc[il][1];
            D_e755 = F_20a5_33fc[il][2];
            D_e756 = F_20a5_33fc[il][3];
            D_e757 = F_20a5_33fc[il][4];
            D_e758 = F_20a5_33fc[il][5];
            D_e759 = F_20a5_33fc[il][6];
            D_e75a = F_20a5_33fc[il][7];
            D_e75b = F_20a5_33fc[il][8];
            D_e75c = F_20a5_33fc[il][9];
            D_e75d = F_20a5_33fc[il][10];
            D_e75e = F_20a5_33fc[il][11];
            D_e75f = F_20a5_33fc[il][12];
            D_e760 = F_20a5_33fc[il][13];
            D_e761 = F_20a5_33fc[il][14];
            D_e762 = F_20a5_33fc[il][15];
            D_e763 = F_20a5_33fc[il][16];
            D_e764 = F_20a5_33fc[il][17];
            D_e765 = F_20a5_33fc[il][18];
            D_e766 = F_20a5_33fc[il][19];
            D_e767 = F_20a5_33fc[il][20];
            D_e768 = F_20a5_33fc[il][21];
            D_e769 = F_20a5_33fc[il][22];
            D_e76a = F_20a5_33fc[il][23];
            sub_root_047c_058b(D_e48c);
            sub_root_047c_05a5(D_27f4, 0x46, 0xa5, D_e88e);
        }
        break;
    case 0x1352: case 0x1372:
        return 0x11b;
    case 0x1e41: case 0x1e61:
        if (D_e7dc > 0) {
            sub_root_047c_12c3();
            sub_root_047c_05a5(D_27f4, 0x43, 0x3b, D_e88a);
            D_2800 = 8;
            sub_root_0265_0004(D_27f4, 0x4a, 0x3f, F_290e_0000.s[0]);
            sub_root_06a1_2c9e("charactr.tmp", 2, &fhc);
            fpm = fdopen(fhc, "r+b");
            _dos_read(fhc, (int far *)&il, 2, &nrd);
            _dos_read(fhc, (int far *)idsh, 10, &nrd);
            _dos_read(fhc, (char far *)D_a668, 0x14, &nrd);
            for (il = 0; il < 4; il++)
                if (idsh[il] == D_8d06[D_e7da]) {
                    if (sub_s029_0e76_392c(0x4c, D_8d06[D_e7da], D_907e)) {
                        sub_s029_0e76_3cde(0x4c, D_8d06[D_e7da], D_907e, -1, -1);
                        _dos_freemem(D_a2d7[il]);
                        D_a2d7[il] = 0;
                        idsh[il] = -1;
                        D_9c69[il][0] = 0;
                        sub_root_0000_1716(il);
                    }
                    break;
                }
            if (il >= 4) {
                for (il = 0; il < 4; il++)
                    if (idsh[il] == -1)
                        break;
                if (il >= 4) {
                    fclose(fpm);
                    sub_root_047c_0553(0x2806, 0x43, 0x3b, 0x82, 0xe, D_27f4, 0x43, 0x3b);
                    sub_root_047c_1293();
                    break;
                }
                sub_s029_0e76_2a32(0x4c, D_8d06[D_e7da], 0xa8);
                idsh[il] = D_8d06[D_e7da];
                strcpy(D_a668[il], D_e7e2[0]);
                if (D_e88e)
                    _dos_freemem(D_e88e);
                sub_root_047c_0545();
                strcpy(nameh, "pics\\shielda.pic");
                D_a2d7[il] = sub_root_0617_045c(nameh);
                strcpy(nameh, "pics\\");
                strcat(nameh, D_e7e2[0]);
                strcat(nameh, "small.pic");
                D_e88e = sub_root_0617_045c(nameh);
                sub_root_047c_04f8();
                D_e753 = 0x39;
                F_20a5_33fc[il][0] = 0x39;
                D_e754 = 0x36;
                F_20a5_33fc[il][1] = 0x36;
                D_e755 = 0;
                D_e759 = 0;
                D_e75c = 0;
                D_e75f = 0;
                D_e763 = 0;
                D_e764 = 0;
                D_e766 = 0;
                D_e767 = 0;
                D_e769 = 0;
                D_e76a = 0;
                F_20a5_33fc[il][2] = 0;
                D_e756 = 0x3d;
                F_20a5_33fc[il][3] = 0x3d;
                D_e757 = 0x38;
                F_20a5_33fc[il][4] = 0x38;
                D_e758 = 0x15;
                D_e760 = 0x15;
                F_20a5_33fc[il][5] = 0x15;
                F_20a5_33fc[il][6] = 0;
                D_e75a = 0x18;
                F_20a5_33fc[il][7] = 0x18;
                D_e75b = 0x28;
                F_20a5_33fc[il][8] = 0x28;
                F_20a5_33fc[il][9] = 0;
                D_e75d = 0x11;
                D_e768 = 0x11;
                F_20a5_33fc[il][10] = 0x11;
                D_e75e = 0x25;
                D_e761 = 0x25;
                F_20a5_33fc[il][11] = 0x25;
                F_20a5_33fc[il][12] = 0;
                F_20a5_33fc[il][13] = 0x15;
                F_20a5_33fc[il][14] = 0x25;
                D_e762 = 0x1c;
                F_20a5_33fc[il][15] = 0x1c;
                F_20a5_33fc[il][16] = 0;
                F_20a5_33fc[il][17] = 0;
                D_e765 = 0x17;
                F_20a5_33fc[il][18] = 0x17;
                F_20a5_33fc[il][19] = 0;
                F_20a5_33fc[il][20] = 0;
                F_20a5_33fc[il][21] = 0x11;
                F_20a5_33fc[il][22] = 0;
                F_20a5_33fc[il][23] = 0;
                sub_root_047c_058b(D_e48c);
                sub_root_047c_0553(0x2806, 0x46, 0xa5, 0x23, 0x24, D_27f4, 0x46, 0xa5);
                sub_root_047c_05a5(D_27f4, 0x46, 0xa5, D_e88e);
                fseek(fpm, (long)(unsigned)(D_8d06[D_e7da] * 0x22a), 1);
                fread(&D_9c55[il << 7], 0x80, 1, fpm);
                fseek(fpm, 0L, 1);
                _dos_read(fhc, (char far *)F_2e38_082c[il], 0x14, &nrd);
                _dos_read(fhc, (char far *)F_2e38_07be[il], 0x16, &nrd);
                _dos_read(fhc, (char far *)F_2e38_003e[il], 0x180, &nrd);
                if (!D_9c69[il][0])
                    D_9c69[il][0] = 1;
                D_9c69[il][0x3d] = -1;
                D_9c69[il][0x44] = 0;
                D_9c69[il][0x0e] = -1;
                D_9c69[il][0x46] = 0;
                D_9c69[il][0x37] = -1;
                D_9c69[il][0x38] = -1;
                D_9c69[il][0x3b] = 0;
                D_9c69[il][0x3c] = 0;
                D_9c69[il][0x39] = 0;
                D_9c69[il][0x3a] = 0;
                D_9c69[il][0x48] = -1;
                D_9c69[il][0x47] = 0;
                D_9c69[il][0x6a] = 0;
                if (D_a67e.v <= il)
                    D_a67e.v = il + 1;
                vec_2049(il);
                sub_root_0000_1588(il);
                sub_root_0000_1716(il);
            }
            fseek(fpm, 2L, 0);
            _dos_write(fhc, (int far *)idsh, 10, &nrd);
            _dos_write(fhc, (char far *)D_a668, 0x14, &nrd);
            fclose(fpm);
            sub_root_047c_1293();
            sub_root_047c_12c3();
            sub_root_047c_0553(0x2806, 0x43, 0x3b, 0x82, 0xe, D_27f4, 0x43, 0x3b);
            sub_root_047c_1293();
            sub_s038_1462_23ec((struct font *)D_27f4);
        }
        break;
    case 0x2e43: case 0x2e63:
        sub_root_047c_12c3();
        sub_root_047c_05a5(D_27f4, 0x43, 0x2c, D_e88a);
        D_2800 = 8;
        sub_root_0265_0004(D_27f4, 0x4a, 0x31, F_290e_0000.s[1]);
        vec_20cb();
        if (D_e88a)
            sub_root_05ba_00c2(D_e88a, "b");
        sub_s038_1462_024e();
        sub_s038_1462_0292();
        sub_root_047c_1293();
        break;
    case 0x2348: case 0x2368:
        if (D_e7dc > 0) {
            sub_root_047c_12c3();
            sub_root_047c_05a5(D_27f4, 0x43, 0x4a, D_e88a);
            D_2800 = 8;
            sub_root_0265_0004(D_27f4, 0x4a, 0x4e, F_290e_0000.s[0x40]);
            sub_root_06a1_2c9e("charactr.tmp", 2, &fhc);
            fpm = fdopen(fhc, "r+b");
            _dos_read(fhc, (int far *)&il, 2, &nrd);
            _dos_read(fhc, (int far *)idsh, 10, &nrd);
            _dos_read(fhc, (char far *)D_a668, 0x14, &nrd);
            for (il = 0; il < 5; il++)
                if (idsh[il] == D_8d06[D_e7da])
                    break;
            if (il >= 5)
                fclose(fpm);
            else {
                if (++D_9c69[il][1] >= 'P')
                    D_9c69[il][1] = 'A';
                _dos_freemem(D_a2d7[il]);
                sub_root_047c_0545();
                strcpy(nameh, "pics\\");
                strcat(nameh, "shield");
                nameh[11] = D_9c69[il][1];
                nameh[12] = 0;
                strcat(nameh, ".pic");
                D_a2d7[il] = sub_root_0617_045c(nameh);
                sub_root_047c_04f8();
                sub_root_0000_1716(il);
                fclose(fpm);
                sub_root_047c_1293();
                sub_root_047c_12c3();
            }
            sub_root_047c_0553(0x2806, 0x43, 0x4a, 0x82, 0xe, D_27f4, 0x43, 0x4a);
            sub_root_047c_1293();
        }
        break;
    case 0x1f53: case 0x1f73:
        if (D_e7dc > 0) {
            sub_root_047c_12c3();
            sub_root_047c_05a5(D_27f4, 0x43, 0x59, D_e88a);
            D_2800 = 8;
            sub_root_0265_0004(D_27f4, 0x4a, 0x5d, F_290e_0000.s[0x1a]);
            sub_root_06a1_2c9e("charactr.tmp", 2, &fhc);
            fpm = fdopen(fhc, "r+b");
            _dos_read(fhc, (int far *)&il, 2, &nrd);
            _dos_read(fhc, (int far *)idsh, 10, &nrd);
            _dos_read(fhc, (char far *)D_a668, 0x14, &nrd);
            for (il = 0; il < 5; il++)
                if (idsh[il] == D_8d06[D_e7da])
                    break;
            if (il >= 5)
                fclose(fpm);
            else {
                if (++D_e7e0 >= D_e7de)
                    D_e7e0 = 0;
                _dos_freemem(D_e88e);
                strcpy(D_a668[il], D_e7e2[D_e7e0]);
                sub_root_047c_0545();
                strcpy(nameh, "pics\\");
                strcat(nameh, D_e7e2[D_e7e0]);
                strcat(nameh, "small.pic");
                D_e88e = sub_root_0617_045c(nameh);
                sub_root_047c_04f8();
                sub_root_047c_0553(0x2806, 0x46, 0xa5, 0x23, 0x24, D_27f4, 0x46, 0xa5);
                sub_root_047c_05a5(D_27f4, 0x46, 0xa5, D_e88e);
                fseek(fpm, -20L, 1);
                _dos_write(fhc, (char far *)D_a668, 0x14, &nrd);
                fclose(fpm);
                sub_root_047c_1293();
                sub_root_047c_12c3();
            }
            sub_root_047c_0553(0x2806, 0x43, 0x59, 0x82, 0xe, D_27f4, 0x43, 0x59);
            sub_root_047c_1293();
        }
        break;
    case 0x231: case 0x4f31:
        if (D_e7dc > 0) {
            sub_root_06a1_2c9e("charactr.tmp", 2, &fhc);
            fpm = fdopen(fhc, "r+b");
            _dos_read(fhc, (int far *)&il, 2, &nrd);
            _dos_read(fhc, (int far *)idsh, 10, &nrd);
            _dos_read(fhc, (char far *)D_a668, 0x14, &nrd);
            fclose(fpm);
            for (il = 0; il < 5; il++)
                if (idsh[il] == D_e7da)
                    break;
            if (il < 5) {
                if (!memcmp(D_a668[il], "F01", 4)) {
                    if (++D_e890 >= 6)
                        D_e890 = 0;
                    switch (D_e890) {
                    case 0:
                        D_e753 = 0x39;
                        D_e754 = 0x36;
                        D_e755 = 0x0;
                        D_e756 = 0x3d;
                        D_e757 = 0x38;
                        D_e758 = 0x15;
                        break;
                    case 1:
                        D_e754 = 0x1c;
                        D_e755 = 0x1c;
                        D_e756 = 0x1c;
                        D_e753 = 0x1a;
                        D_e757 = 0x1a;
                        D_e758 = 0x1a;
                        break;
                    case 2:
                        D_e753 = 0x0;
                        D_e754 = 0x0;
                        D_e755 = 0x0;
                        D_e756 = 0x0;
                        D_e757 = 0x0;
                        D_e758 = 0x0;
                        break;
                    case 3:
                        D_e754 = 0x9;
                        D_e753 = 0x23;
                        D_e756 = 0x23;
                        D_e757 = 0xc;
                        D_e755 = 0x0;
                        D_e758 = 0x0;
                        break;
                    case 4:
                        D_e753 = 0x23;
                        D_e755 = 0x9;
                        D_e754 = 0x21;
                        D_e756 = 0x21;
                        D_e757 = 0x1a;
                        D_e758 = 0x0;
                        break;
                    case 5:
                        D_e753 = 0x21;
                        D_e754 = 0x15;
                        D_e756 = 0x19;
                        D_e755 = 0x12;
                        D_e757 = 0x12;
                        D_e758 = 0xe;
                        break;
                    }
                    F_20a5_33fc[il][0] = D_e753;
                    F_20a5_33fc[il][1] = D_e754;
                    F_20a5_33fc[il][2] = D_e755;
                    F_20a5_33fc[il][3] = D_e756;
                    F_20a5_33fc[il][4] = D_e757;
                    F_20a5_33fc[il][5] = D_e758;
                } else if (!memcmp(D_a668[il], "F60", 4)) {
                    if (++D_e890 >= 6)
                        D_e890 = 0;
                    switch (D_e890) {
                    case 0:
                        D_e762 = 0x21;
                        D_e765 = 0x28;
                        D_e768 = 0x2d;
                        D_e763 = 0x0;
                        D_e764 = 0x0;
                        D_e766 = 0x0;
                        D_e767 = 0x0;
                        D_e769 = 0x0;
                        D_e76a = 0x0;
                        break;
                    case 1:
                        D_e762 = 0x2f;
                        D_e763 = 0x2a;
                        D_e764 = 0x12;
                        D_e765 = 0x37;
                        D_e766 = 0x32;
                        D_e767 = 0x1a;
                        D_e768 = 0x3e;
                        D_e769 = 0x3c;
                        D_e76a = 0x25;
                        break;
                    case 2:
                        D_e762 = 0x2d;
                        D_e763 = 0x1e;
                        D_e764 = 0x0;
                        D_e765 = 0x35;
                        D_e766 = 0x28;
                        D_e767 = 0x8;
                        D_e768 = 0x3e;
                        D_e769 = 0x33;
                        D_e76a = 0x12;
                        break;
                    case 3:
                        D_e762 = 0x1c;
                        D_e763 = 0xd;
                        D_e765 = 0x25;
                        D_e766 = 0x11;
                        D_e768 = 0x2d;
                        D_e769 = 0x15;
                        D_e764 = 0x0;
                        D_e767 = 0x0;
                        D_e76a = 0x0;
                        break;
                    case 4:
                        D_e762 = 0x0;
                        D_e763 = 0x0;
                        D_e764 = 0x0;
                        D_e765 = 0xa;
                        D_e766 = 0xa;
                        D_e767 = 0xa;
                        D_e768 = 0x12;
                        D_e769 = 0x12;
                        D_e76a = 0x12;
                        break;
                    case 5:
                        D_e762 = 0x15;
                        D_e763 = 0x9;
                        D_e765 = 0x1a;
                        D_e766 = 0xb;
                        D_e768 = 0x21;
                        D_e769 = 0xd;
                        D_e764 = 0x0;
                        D_e767 = 0x0;
                        D_e76a = 0x0;
                        break;
                    }
                    F_20a5_33fc[il][15] = D_e762;
                    F_20a5_33fc[il][16] = D_e763;
                    F_20a5_33fc[il][17] = D_e764;
                    F_20a5_33fc[il][18] = D_e765;
                    F_20a5_33fc[il][19] = D_e766;
                    F_20a5_33fc[il][20] = D_e767;
                    F_20a5_33fc[il][21] = D_e768;
                    F_20a5_33fc[il][22] = D_e769;
                    F_20a5_33fc[il][23] = D_e76a;
                } else if (!memcmp(D_a668[il], "C00", 4)) {
                    if (++D_e890 >= 6)
                        D_e890 = 0;
                    switch (D_e890) {
                    case 0:
                        D_e756 = 0x1c;
                        D_e753 = 0x1c;
                        D_e757 = 0x13;
                        D_e754 = 0x13;
                        D_e758 = 0xa;
                        D_e755 = 0xa;
                        break;
                    case 1:
                        D_e756 = 0x23;
                        D_e757 = 0x23;
                        D_e758 = 0x23;
                        D_e753 = 0x23;
                        D_e754 = 0x23;
                        D_e755 = 0x23;
                        break;
                    case 2:
                        D_e756 = 0x0;
                        D_e757 = 0x0;
                        D_e758 = 0x0;
                        D_e753 = 0x0;
                        D_e754 = 0x0;
                        D_e755 = 0x0;
                        break;
                    case 3:
                        D_e756 = 0x1c;
                        D_e757 = 0x13;
                        D_e758 = 0xa;
                        D_e753 = 0x3e;
                        D_e754 = 0x28;
                        D_e755 = 0x19;
                        break;
                    case 4:
                        D_e756 = 0x23;
                        D_e757 = 0x23;
                        D_e758 = 0x23;
                        D_e753 = 0x3e;
                        D_e754 = 0x28;
                        D_e755 = 0x19;
                        break;
                    case 5:
                        D_e756 = 0x0;
                        D_e757 = 0x0;
                        D_e758 = 0x0;
                        D_e753 = 0x3e;
                        D_e754 = 0x28;
                        D_e755 = 0x19;
                        break;
                    }
                    F_20a5_33fc[il][0] = D_e753;
                    F_20a5_33fc[il][1] = D_e754;
                    F_20a5_33fc[il][2] = D_e755;
                    F_20a5_33fc[il][3] = D_e756;
                    F_20a5_33fc[il][4] = D_e757;
                    F_20a5_33fc[il][5] = D_e758;
                } else if (!memcmp(D_a668[il], "A00", 4)) {
                    if (++D_e890 >= 6)
                        D_e890 = 0;
                    switch (D_e890) {
                    case 0:
                        D_e753 = 0x28;
                        D_e756 = 0x21;
                        D_e757 = 0xc;
                        D_e755 = 0x19;
                        D_e759 = 0x19;
                        D_e75a = 0x7;
                        D_e754 = 0x12;
                        D_e758 = 0x12;
                        D_e75b = 0x12;
                        break;
                    case 1:
                        D_e754 = 0x20;
                        D_e755 = 0x33;
                        D_e757 = 0x1c;
                        D_e758 = 0x2d;
                        D_e753 = 0x0;
                        D_e756 = 0x0;
                        D_e759 = 0x0;
                        D_e75a = 0x18;
                        D_e75b = 0x28;
                        break;
                    case 2:
                        D_e753 = 0x1a;
                        D_e755 = 0x26;
                        D_e756 = 0x15;
                        D_e757 = 0x19;
                        D_e754 = 0x20;
                        D_e758 = 0x20;
                        D_e759 = 0x13;
                        D_e75a = 0x17;
                        D_e75b = 0x1e;
                        break;
                    case 3:
                        D_e753 = 0x2e;
                        D_e754 = 0x25;
                        D_e755 = 0x25;
                        D_e756 = 0x2a;
                        D_e757 = 0x21;
                        D_e758 = 0x21;
                        D_e759 = 0x23;
                        D_e75a = 0x19;
                        D_e75b = 0x19;
                        break;
                    case 4:
                        D_e753 = 0x33;
                        D_e754 = 0x2f;
                        D_e756 = 0x2e;
                        D_e758 = 0x19;
                        D_e755 = 0x25;
                        D_e757 = 0x25;
                        D_e759 = 0x25;
                        D_e75a = 0x1c;
                        D_e75b = 0x15;
                        break;
                    case 5:
                        D_e754 = 0x1e;
                        D_e755 = 0x15;
                        D_e757 = 0x15;
                        D_e758 = 0x12;
                        D_e753 = 0x0;
                        D_e756 = 0x0;
                        D_e759 = 0x0;
                        D_e75a = 0x10;
                        D_e75b = 0xb;
                        break;
                    }
                    F_20a5_33fc[il][0] = D_e753;
                    F_20a5_33fc[il][1] = D_e754;
                    F_20a5_33fc[il][2] = D_e755;
                    F_20a5_33fc[il][3] = D_e756;
                    F_20a5_33fc[il][4] = D_e757;
                    F_20a5_33fc[il][5] = D_e758;
                    F_20a5_33fc[il][6] = D_e759;
                    F_20a5_33fc[il][7] = D_e75a;
                    F_20a5_33fc[il][8] = D_e75b;
                }
                sub_root_047c_058b(D_e48c);
            }
        }
        break;
    case 0x332: case 0x5032:
        if (D_e7dc > 0) {
            sub_root_06a1_2c9e("charactr.tmp", 2, &fhc);
            fpm = fdopen(fhc, "r+b");
            _dos_read(fhc, (int far *)&il, 2, &nrd);
            _dos_read(fhc, (int far *)idsh, 10, &nrd);
            _dos_read(fhc, (char far *)D_a668, 0x14, &nrd);
            fclose(fpm);
            for (il = 0; il < 5; il++)
                if (idsh[il] == D_e7da)
                    break;
            if (il < 5) {
                if (!memcmp(D_a668[il], "F01", 4)) {
                    if (++D_e892 >= 6)
                        D_e892 = 0;
                    switch (D_e892) {
                    case 0:
                        D_e75a = 0x18;
                        D_e75b = 0x28;
                        D_e75d = 0x11;
                        D_e759 = 0x0;
                        D_e75c = 0x0;
                        D_e75f = 0x0;
                        D_e760 = 0x15;
                        D_e75e = 0x25;
                        D_e761 = 0x25;
                        break;
                    case 1:
                        D_e75b = 0x0;
                        D_e75c = 0xc;
                        D_e75a = 0x7;
                        D_e75d = 0x7;
                        D_e759 = 0x6;
                        D_e75e = 0x6;
                        D_e75f = 0x8;
                        D_e760 = 0x8;
                        D_e761 = 0x5;
                        break;
                    case 2:
                        D_e75a = 0x18;
                        D_e759 = 0x0;
                        D_e75b = 0x0;
                        D_e75c = 0xf;
                        D_e75d = 0x12;
                        D_e75e = 0xe;
                        D_e760 = 0x13;
                        D_e75f = 0xc;
                        D_e761 = 0xc;
                        break;
                    case 3:
                        D_e759 = 0x35;
                        D_e75a = 0x35;
                        D_e75b = 0xb;
                        D_e75c = 0x2f;
                        D_e75d = 0x2f;
                        D_e75e = 0x8;
                        D_e75f = 0x39;
                        D_e760 = 0x2e;
                        D_e761 = 0x9;
                        break;
                    case 4:
                        D_e759 = 0x31;
                        D_e75a = 0x1a;
                        D_e75b = 0x11;
                        D_e75d = 0x13;
                        D_e75e = 0x5;
                        D_e75c = 0x25;
                        D_e75f = 0x25;
                        D_e760 = 0x17;
                        D_e761 = 0x2;
                        break;
                    case 5:
                        D_e759 = 0x1c;
                        D_e75a = 0xa;
                        D_e75c = 0x28;
                        D_e75d = 0x10;
                        D_e75e = 0x11;
                        D_e75f = 0x21;
                        D_e75b = 0xb;
                        D_e760 = 0xb;
                        D_e761 = 0xc;
                        break;
                    }
                    F_20a5_33fc[il][6] = D_e759;
                    F_20a5_33fc[il][7] = D_e75a;
                    F_20a5_33fc[il][8] = D_e75b;
                    F_20a5_33fc[il][9] = D_e75c;
                    F_20a5_33fc[il][10] = D_e75d;
                    F_20a5_33fc[il][11] = D_e75e;
                    F_20a5_33fc[il][12] = D_e75f;
                    F_20a5_33fc[il][13] = D_e760;
                    F_20a5_33fc[il][14] = D_e761;
                } else if (!memcmp(D_a668[il], "A00", 4)) {
                    if (++D_e892 >= 6)
                        D_e892 = 0;
                    switch (D_e892) {
                    case 0:
                        D_e75c = 0x23;
                        D_e75f = 0x21;
                        D_e75d = 0xc;
                        D_e760 = 0xc;
                        D_e762 = 0x1a;
                        D_e763 = 0x7;
                        D_e75e = 0x12;
                        D_e761 = 0x12;
                        D_e764 = 0x12;
                        D_e765 = 0x13;
                        D_e768 = 0x13;
                        D_e766 = 0x6;
                        D_e767 = 0x6;
                        D_e769 = 0x6;
                        D_e76a = 0x6;
                        break;
                    case 1:
                        D_e75c = 0x2b;
                        D_e75f = 0x26;
                        D_e760 = 0x1e;
                        D_e762 = 0x25;
                        D_e763 = 0x1a;
                        D_e75d = 0x21;
                        D_e765 = 0x21;
                        D_e768 = 0x21;
                        D_e766 = 0x18;
                        D_e769 = 0x18;
                        D_e75e = 0x9;
                        D_e761 = 0x9;
                        D_e764 = 0x9;
                        D_e767 = 0x9;
                        D_e76a = 0x9;
                        break;
                    case 2:
                        D_e75c = 0x8;
                        D_e75d = 0x1c;
                        D_e75f = 0x5;
                        D_e760 = 0x18;
                        D_e75e = 0x14;
                        D_e761 = 0x14;
                        D_e763 = 0x14;
                        D_e762 = 0x0;
                        D_e765 = 0x0;
                        D_e764 = 0x10;
                        D_e766 = 0x10;
                        D_e767 = 0xe;
                        D_e768 = 0x9;
                        D_e769 = 0x9;
                        D_e76a = 0x9;
                        break;
                    case 3:
                        D_e75d = 0x14;
                        D_e75e = 0x15;
                        D_e761 = 0x1c;
                        D_e763 = 0xd;
                        D_e764 = 0x17;
                        D_e766 = 0xa;
                        D_e760 = 0x11;
                        D_e767 = 0x11;
                        D_e75c = 0x0;
                        D_e75f = 0x0;
                        D_e762 = 0x0;
                        D_e765 = 0x0;
                        D_e768 = 0x0;
                        D_e769 = 0x8;
                        D_e76a = 0xe;
                        break;
                    case 4:
                        D_e75c = 0x10;
                        D_e75e = 0x10;
                        D_e75f = 0xb;
                        D_e760 = 0xb;
                        D_e75d = 0xe;
                        D_e761 = 0xe;
                        D_e762 = 0xc;
                        D_e763 = 0x9;
                        D_e764 = 0x9;
                        D_e765 = 0x9;
                        D_e766 = 0x9;
                        D_e767 = 0x9;
                        D_e768 = 0x0;
                        D_e769 = 0x0;
                        D_e76a = 0x0;
                        break;
                    case 5:
                        D_e75c = 0x2a;
                        D_e75d = 0x15;
                        D_e760 = 0x14;
                        D_e75f = 0x28;
                        D_e762 = 0x28;
                        D_e763 = 0xf;
                        D_e765 = 0x23;
                        D_e768 = 0x23;
                        D_e766 = 0xe;
                        D_e769 = 0xe;
                        D_e75e = 0xb;
                        D_e761 = 0xb;
                        D_e764 = 0xb;
                        D_e767 = 0xb;
                        D_e76a = 0xb;
                        break;
                    }
                    F_20a5_33fc[il][9] = D_e75c;
                    F_20a5_33fc[il][10] = D_e75d;
                    F_20a5_33fc[il][11] = D_e75e;
                    F_20a5_33fc[il][12] = D_e75f;
                    F_20a5_33fc[il][13] = D_e760;
                    F_20a5_33fc[il][14] = D_e761;
                    F_20a5_33fc[il][15] = D_e762;
                    F_20a5_33fc[il][16] = D_e763;
                    F_20a5_33fc[il][17] = D_e764;
                    F_20a5_33fc[il][18] = D_e765;
                    F_20a5_33fc[il][19] = D_e766;
                    F_20a5_33fc[il][20] = D_e767;
                    F_20a5_33fc[il][21] = D_e768;
                    F_20a5_33fc[il][22] = D_e769;
                    F_20a5_33fc[il][23] = D_e76a;
                } else if (!memcmp(D_a668[il], "C00", 4)) {
                    if (++D_e892 >= 6)
                        D_e892 = 0;
                    switch (D_e892) {
                    case 0:
                        D_e762 = 0x18;
                        D_e763 = 0x11;
                        D_e764 = 0x9;
                        D_e765 = 0x15;
                        D_e766 = 0x10;
                        D_e767 = 0x8;
                        D_e768 = 0x13;
                        D_e769 = 0xe;
                        D_e76a = 0x7;
                        break;
                    case 1:
                        D_e762 = 0x12;
                        D_e763 = 0x12;
                        D_e764 = 0x12;
                        D_e765 = 0xd;
                        D_e766 = 0xd;
                        D_e767 = 0xd;
                        D_e768 = 0xa;
                        D_e769 = 0xa;
                        D_e76a = 0xa;
                        break;
                    case 2:
                        D_e763 = 0x1c;
                        D_e766 = 0x15;
                        D_e769 = 0x10;
                        D_e762 = 0x0;
                        D_e764 = 0x0;
                        D_e765 = 0x0;
                        D_e767 = 0x0;
                        D_e768 = 0x0;
                        D_e76a = 0x0;
                        break;
                    case 3:
                        D_e762 = 0x28;
                        D_e763 = 0x25;
                        D_e764 = 0x14;
                        D_e765 = 0x21;
                        D_e766 = 0x20;
                        D_e767 = 0x11;
                        D_e768 = 0x1c;
                        D_e769 = 0x1a;
                        D_e76a = 0xe;
                        break;
                    case 4:
                        D_e762 = 0x21;
                        D_e763 = 0x1e;
                        D_e764 = 0x1e;
                        D_e765 = 0x1e;
                        D_e766 = 0x1a;
                        D_e767 = 0x1a;
                        D_e768 = 0x18;
                        D_e769 = 0x14;
                        D_e76a = 0x14;
                        break;
                    case 5:
                        D_e762 = 0x32;
                        D_e763 = 0x32;
                        D_e764 = 0x25;
                        D_e765 = 0x2a;
                        D_e766 = 0x2a;
                        D_e767 = 0x1c;
                        D_e768 = 0x23;
                        D_e769 = 0x23;
                        D_e76a = 0x15;
                        break;
                    }
                    F_20a5_33fc[il][15] = D_e762;
                    F_20a5_33fc[il][16] = D_e763;
                    F_20a5_33fc[il][17] = D_e764;
                    F_20a5_33fc[il][18] = D_e765;
                    F_20a5_33fc[il][19] = D_e766;
                    F_20a5_33fc[il][20] = D_e767;
                    F_20a5_33fc[il][21] = D_e768;
                    F_20a5_33fc[il][22] = D_e769;
                    F_20a5_33fc[il][23] = D_e76a;
                } else if (!memcmp(D_a668[il], "F60", 4)) {
                    if (++D_e892 >= 6)
                        D_e892 = 0;
                    switch (D_e892) {
                    case 0:
                        D_e756 = 0x10;
                        D_e757 = 0x10;
                        D_e758 = 0x12;
                        D_e759 = 0x14;
                        D_e75a = 0x14;
                        D_e75b = 0x15;
                        D_e75c = 0x18;
                        D_e75d = 0x18;
                        D_e75e = 0x1a;
                        D_e75f = 0x1c;
                        D_e760 = 0x1c;
                        D_e761 = 0x1e;
                        break;
                    case 1:
                        D_e757 = 0x10;
                        D_e75a = 0x14;
                        D_e75d = 0x18;
                        D_e760 = 0x1c;
                        D_e756 = 0x0;
                        D_e758 = 0x0;
                        D_e759 = 0x0;
                        D_e75b = 0x0;
                        D_e75c = 0x0;
                        D_e75e = 0x0;
                        D_e75f = 0x0;
                        D_e761 = 0x0;
                        break;
                    case 2:
                        D_e758 = 0xd;
                        D_e75a = 0x4;
                        D_e75b = 0x11;
                        D_e75d = 0xa;
                        D_e75e = 0x14;
                        D_e756 = 0x0;
                        D_e757 = 0x0;
                        D_e759 = 0x0;
                        D_e75c = 0x0;
                        D_e75f = 0x0;
                        D_e760 = 0x12;
                        D_e761 = 0x19;
                        break;
                    case 3:
                        D_e756 = 0x11;
                        D_e757 = 0xb;
                        D_e75a = 0xe;
                        D_e75c = 0x19;
                        D_e75d = 0x12;
                        D_e75f = 0x1e;
                        D_e759 = 0x15;
                        D_e760 = 0x15;
                        D_e758 = 0x0;
                        D_e75b = 0x0;
                        D_e75e = 0x0;
                        D_e761 = 0x0;
                        break;
                    case 4:
                        D_e756 = 0x15;
                        D_e759 = 0x19;
                        D_e75c = 0x1c;
                        D_e75f = 0x21;
                        D_e757 = 0x0;
                        D_e758 = 0x0;
                        D_e75a = 0x0;
                        D_e75b = 0x0;
                        D_e75d = 0x0;
                        D_e75e = 0x0;
                        D_e760 = 0x0;
                        D_e761 = 0x0;
                        break;
                    case 5:
                        D_e756 = 0x0;
                        D_e757 = 0x0;
                        D_e758 = 0x0;
                        D_e759 = 0x9;
                        D_e75a = 0x9;
                        D_e75b = 0x9;
                        D_e75c = 0xe;
                        D_e75d = 0xe;
                        D_e75e = 0xe;
                        D_e75f = 0x15;
                        D_e760 = 0x15;
                        D_e761 = 0x15;
                        break;
                    }
                    F_20a5_33fc[il][3] = D_e756;
                    F_20a5_33fc[il][4] = D_e757;
                    F_20a5_33fc[il][5] = D_e758;
                    F_20a5_33fc[il][6] = D_e759;
                    F_20a5_33fc[il][6] = D_e75a;
                    F_20a5_33fc[il][8] = D_e75b;
                    F_20a5_33fc[il][9] = D_e75c;
                    F_20a5_33fc[il][10] = D_e75d;
                    F_20a5_33fc[il][11] = D_e75e;
                    F_20a5_33fc[il][12] = D_e75f;
                    F_20a5_33fc[il][13] = D_e760;
                    F_20a5_33fc[il][14] = D_e761;
                }
                sub_root_047c_058b(D_e48c);
            }
        }
        break;
    case 0x433: case 0x5133:
        if (D_e7dc > 0) {
            sub_root_06a1_2c9e("charactr.tmp", 2, &fhc);
            fpm = fdopen(fhc, "r+b");
            _dos_read(fhc, (int far *)&il, 2, &nrd);
            _dos_read(fhc, (int far *)idsh, 10, &nrd);
            _dos_read(fhc, (char far *)D_a668, 0x14, &nrd);
            fclose(fpm);
            for (il = 0; il < 5; il++)
                if (idsh[il] == D_e7da)
                    break;
            if (il < 5) {
                if (!memcmp(D_a668[il], "F01", 4)) {
                    if (++D_e894 >= 6)
                        D_e894 = 0;
                    switch (D_e894) {
                    case 0:
                        D_e762 = 0x1c;
                        D_e765 = 0x17;
                        D_e768 = 0x11;
                        D_e763 = 0x0;
                        D_e764 = 0x0;
                        D_e766 = 0x0;
                        D_e767 = 0x0;
                        D_e769 = 0x0;
                        D_e76a = 0x0;
                        break;
                    case 1:
                        D_e764 = 0x0;
                        D_e765 = 0xc;
                        D_e763 = 0x7;
                        D_e766 = 0x7;
                        D_e762 = 0x6;
                        D_e767 = 0x6;
                        D_e768 = 0x8;
                        D_e769 = 0x8;
                        D_e76a = 0x5;
                        break;
                    case 2:
                        D_e762 = 0x2b;
                        D_e763 = 0x2a;
                        D_e764 = 0x1c;
                        D_e765 = 0x23;
                        D_e766 = 0x23;
                        D_e767 = 0x12;
                        D_e768 = 0x28;
                        D_e769 = 0x25;
                        D_e76a = 0x13;
                        break;
                    case 3:
                        D_e763 = 0x2a;
                        D_e764 = 0x2a;
                        D_e766 = 0x20;
                        D_e767 = 0x21;
                        D_e762 = 0x0;
                        D_e765 = 0x0;
                        D_e768 = 0x0;
                        D_e769 = 0x25;
                        D_e76a = 0x25;
                        break;
                    case 4:
                        D_e762 = 0x15;
                        D_e763 = 0x0;
                        D_e764 = 0xc;
                        D_e765 = 0x13;
                        D_e766 = 0x8;
                        D_e767 = 0xe;
                        D_e768 = 0x11;
                        D_e769 = 0x2;
                        D_e76a = 0x18;
                        break;
                    case 5:
                        D_e762 = 0x26;
                        D_e763 = 0x28;
                        D_e764 = 0x2e;
                        D_e765 = 0x1e;
                        D_e766 = 0x1c;
                        D_e767 = 0x25;
                        D_e768 = 0x2c;
                        D_e769 = 0x2b;
                        D_e76a = 0x33;
                        break;
                    }
                    F_20a5_33fc[il][15] = D_e762;
                    F_20a5_33fc[il][16] = D_e763;
                    F_20a5_33fc[il][17] = D_e764;
                    F_20a5_33fc[il][18] = D_e765;
                    F_20a5_33fc[il][19] = D_e766;
                    F_20a5_33fc[il][20] = D_e767;
                    F_20a5_33fc[il][21] = D_e768;
                    F_20a5_33fc[il][22] = D_e769;
                    F_20a5_33fc[il][23] = D_e76a;
                } else if (!memcmp(D_a668[il], "F60", 4)) {
                    if (++D_e894 >= 6)
                        D_e894 = 0;
                    switch (D_e894) {
                    case 0:
                        D_e753 = 0x19;
                        D_e754 = 0x12;
                        D_e755 = 0x0;
                        break;
                    case 1:
                        D_e754 = 0x18;
                        D_e753 = 0x0;
                        D_e755 = 0x0;
                        break;
                    case 2:
                        D_e753 = 0x0;
                        D_e754 = 0xa;
                        D_e755 = 0x15;
                        break;
                    case 3:
                        D_e753 = 0x1c;
                        D_e754 = 0x0;
                        D_e755 = 0x0;
                        break;
                    case 4:
                        D_e753 = 0xe;
                        D_e754 = 0xe;
                        D_e755 = 0xe;
                        break;
                    case 5:
                        D_e753 = 0x3f;
                        D_e754 = 0x28;
                        D_e755 = 0x19;
                        break;
                    }
                    F_20a5_33fc[il][0] = D_e753;
                    F_20a5_33fc[il][1] = D_e754;
                    F_20a5_33fc[il][2] = D_e755;
                } else if (!memcmp(D_a668[il], "C00", 4)) {
                    if (++D_e894 >= 6)
                        D_e894 = 0;
                    switch (D_e894) {
                    case 0:
                        D_e759 = 0x2a;
                        D_e75a = 0x1a;
                        D_e75b = 0x11;
                        D_e75c = 0x25;
                        D_e75d = 0x18;
                        D_e75e = 0xe;
                        D_e75f = 0x1c;
                        D_e760 = 0x13;
                        D_e761 = 0xa;
                        break;
                    case 1:
                        D_e759 = 0x11;
                        D_e75a = 0x11;
                        D_e75b = 0x11;
                        D_e75c = 0xa;
                        D_e75d = 0xa;
                        D_e75e = 0xa;
                        D_e75f = 0x3;
                        D_e760 = 0x3;
                        D_e761 = 0x3;
                        break;
                    case 2:
                        D_e75a = 0x2a;
                        D_e759 = 0x9;
                        D_e75b = 0x9;
                        D_e75d = 0x25;
                        D_e760 = 0x1a;
                        D_e75c = 0xa;
                        D_e75e = 0xa;
                        D_e75f = 0xa;
                        D_e761 = 0xa;
                        break;
                    case 3:
                        D_e759 = 0x21;
                        D_e75a = 0x21;
                        D_e75b = 0x21;
                        D_e75c = 0x1c;
                        D_e75d = 0x1c;
                        D_e75e = 0x1c;
                        D_e75f = 0x18;
                        D_e760 = 0x18;
                        D_e761 = 0x18;
                        break;
                    case 4:
                        D_e759 = 0x30;
                        D_e75a = 0x2d;
                        D_e75c = 0x28;
                        D_e75d = 0x28;
                        D_e75e = 0x1c;
                        D_e75b = 0x21;
                        D_e75f = 0x21;
                        D_e760 = 0x21;
                        D_e761 = 0x18;
                        break;
                    case 5:
                        D_e759 = 0x2f;
                        D_e75a = 0x2c;
                        D_e75b = 0x18;
                        D_e75c = 0x2b;
                        D_e75e = 0x15;
                        D_e75d = 0x28;
                        D_e75f = 0x28;
                        D_e760 = 0x25;
                        D_e761 = 0x13;
                        break;
                    }
                    F_20a5_33fc[il][6] = D_e759;
                    F_20a5_33fc[il][7] = D_e75a;
                    F_20a5_33fc[il][8] = D_e75b;
                    F_20a5_33fc[il][9] = D_e75c;
                    F_20a5_33fc[il][10] = D_e75d;
                    F_20a5_33fc[il][11] = D_e75e;
                    F_20a5_33fc[il][12] = D_e75f;
                    F_20a5_33fc[il][13] = D_e760;
                    F_20a5_33fc[il][14] = D_e761;
                } else if (!memcmp(D_a668[il], "A00", 4)) {
                    if (++D_e894 >= 6)
                        D_e894 = 0;
                    switch (D_e894) {
                    case 0:
                        D_e75c = 0x23;
                        D_e75f = 0x21;
                        D_e75d = 0xc;
                        D_e760 = 0xc;
                        D_e762 = 0x1a;
                        D_e763 = 0x7;
                        D_e75e = 0x12;
                        D_e761 = 0x12;
                        D_e764 = 0x12;
                        D_e765 = 0x13;
                        D_e768 = 0x13;
                        D_e766 = 0x6;
                        D_e767 = 0x6;
                        D_e769 = 0x6;
                        D_e76a = 0x6;
                        break;
                    case 1:
                        D_e75c = 0x2b;
                        D_e75f = 0x26;
                        D_e760 = 0x1e;
                        D_e762 = 0x25;
                        D_e763 = 0x1a;
                        D_e75d = 0x21;
                        D_e765 = 0x21;
                        D_e768 = 0x21;
                        D_e766 = 0x18;
                        D_e769 = 0x18;
                        D_e75e = 0x9;
                        D_e761 = 0x9;
                        D_e764 = 0x9;
                        D_e767 = 0x9;
                        D_e76a = 0x9;
                        break;
                    case 2:
                        D_e75c = 0x8;
                        D_e75d = 0x1c;
                        D_e75f = 0x5;
                        D_e760 = 0x18;
                        D_e75e = 0x14;
                        D_e761 = 0x14;
                        D_e763 = 0x14;
                        D_e762 = 0x0;
                        D_e765 = 0x0;
                        D_e764 = 0x10;
                        D_e766 = 0x10;
                        D_e767 = 0xe;
                        D_e768 = 0x9;
                        D_e769 = 0x9;
                        D_e76a = 0x9;
                        break;
                    case 3:
                        D_e75d = 0x14;
                        D_e75e = 0x15;
                        D_e761 = 0x1c;
                        D_e763 = 0xd;
                        D_e764 = 0x17;
                        D_e766 = 0xa;
                        D_e760 = 0x11;
                        D_e767 = 0x11;
                        D_e75c = 0x0;
                        D_e75f = 0x0;
                        D_e762 = 0x0;
                        D_e765 = 0x0;
                        D_e768 = 0x0;
                        D_e769 = 0x8;
                        D_e76a = 0xe;
                        break;
                    case 4:
                        D_e75c = 0x10;
                        D_e75e = 0x10;
                        D_e75f = 0xb;
                        D_e760 = 0xb;
                        D_e75d = 0xe;
                        D_e761 = 0xe;
                        D_e762 = 0xc;
                        D_e763 = 0x9;
                        D_e764 = 0x9;
                        D_e765 = 0x9;
                        D_e766 = 0x9;
                        D_e767 = 0x9;
                        D_e768 = 0x0;
                        D_e769 = 0x0;
                        D_e76a = 0x0;
                        break;
                    case 5:
                        D_e75c = 0x2a;
                        D_e75d = 0x15;
                        D_e760 = 0x14;
                        D_e75f = 0x28;
                        D_e762 = 0x28;
                        D_e763 = 0xf;
                        D_e765 = 0x23;
                        D_e768 = 0x23;
                        D_e766 = 0xe;
                        D_e769 = 0xe;
                        D_e75e = 0xb;
                        D_e761 = 0xb;
                        D_e764 = 0xb;
                        D_e767 = 0xb;
                        D_e76a = 0xb;
                        break;
                    }
                    F_20a5_33fc[il][9] = D_e75c;
                    F_20a5_33fc[il][10] = D_e75d;
                    F_20a5_33fc[il][11] = D_e75e;
                    F_20a5_33fc[il][12] = D_e75f;
                    F_20a5_33fc[il][13] = D_e760;
                    F_20a5_33fc[il][14] = D_e761;
                    F_20a5_33fc[il][15] = D_e762;
                    F_20a5_33fc[il][16] = D_e763;
                    F_20a5_33fc[il][17] = D_e764;
                    F_20a5_33fc[il][18] = D_e765;
                    F_20a5_33fc[il][19] = D_e766;
                    F_20a5_33fc[il][20] = D_e767;
                    F_20a5_33fc[il][21] = D_e768;
                    F_20a5_33fc[il][22] = D_e769;
                    F_20a5_33fc[il][23] = D_e76a;
                }
                sub_root_047c_058b(D_e48c);
            }
        }
        break;

    case 0x1000:
        sub_s038_1462_01fe(0);
        sub_root_0000_13a0();
        break;
    }
}

/* list of the characters saved in charactr.tmp (right-hand column); '+' marks party members */
void far sub_s038_1462_23ec(struct font *f)
{
    int ids[5];
    int n;
    int nr;
    int h;
    int *slot;
    int i;
    int shown;
    int y;
    int j;
    sub_root_047c_12c3();
    sub_root_06a1_2c9e("charactr.tmp", 0, &h);
    _dos_read(h, (int far *)&n, 2, &nr);
    _dos_read(h, (int far *)ids, 10, &nr);
    _dos_read(h, (char far *)D_a668, 0x14, &nr);
    shown = 0;
    for (i = 0; i < n; i++) {
        _dos_read(h, (char far *)&D_8d2e, 0x80, &nr);
        _dos_read(h, (char far *)D_9085, 0x1aa, &nr);
        if ((D_8d2e.id == D_907e || D_8d2e.status) && D_8d2e.status != 5
            && (!D_8d2e.status || !sub_s029_0e76_392c(0x43, vec_20ad(i)))) {
            D_8d06[shown] = i;
            sub_root_047c_0553(0x2806, 0xc5, 0x18 + shown * 8, 0x73, 8, D_27f4, 0xc5, 0x18 + shown * 8);
            for (j = 0; j < 5; j++) {
                if (shown == ids[j]) {
                    f->color = 0x11;
                    sub_root_047c_055a(f, 200, 0x18 + shown * 8, "+");
                    f->color = 0x10;
                    sub_root_047c_055a(f, 199, 0x18 + shown * 8 - 1, "+");
                }
            }
            f->color = 0x11;
            sub_root_047c_055a(f, 0xd0, 0x18 + shown * 8 - 1, D_8d2e.name);
            f->color = 0x10;
            sub_root_047c_055a(f, 0xcf, 0x18 + shown * 8 - 2, D_8d2e.name);
            shown++;
        }
    }
    _dos_close(h);
    D_e7dc = n;
    sub_root_047c_04dc(f, 0xc6, (D_e7da << 3) + 0x16, 0x71, 8, 0x10, 0x12);
    sub_root_047c_1293();
}

/* the eight attribute names */
void far sub_s038_1462_25ca(struct font *f)
{
    sub_root_047c_12c3();
    f->color = 8;
    sub_root_0265_0004(f, 0x48, 0x30, F_290e_0000.s[1]);
    sub_root_0265_0004(f, 0x48, 0x3f, F_290e_0000.s[0]);
    sub_root_0265_0004(f, 0x48, 0x4e, F_290e_0000.s[0x40]);
    sub_root_0265_0004(f, 0x48, 0x5d, F_290e_0000.s[0x1a]);
    sub_root_0265_0004(f, 0x48, 0x6c, F_290e_0000.s[0x3d]);
    sub_root_0265_0004(f, 0x48, 0x7b, F_290e_0000.s[0x3e]);
    sub_root_0265_0004(f, 0x48, 0x8a, F_290e_0000.s[0x3f]);
    sub_root_0265_0004(f, 0x48, 0x99, F_290e_0000.s[0x3c]);
    sub_root_047c_1293();
}
