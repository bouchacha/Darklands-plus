/* place: s022 1ef8:0000 */
/* orig: size=07da CONST=7df2+0002 _DATA=2358+008f */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* status: wip 3/5, all functions written (03e4, 04e4, 07aa match).
 * 20a5 data: `extern int far F_20a5_1f44[];` (a far ARRAY gives `mov ax,SEG; mov es,ax`).
 * 051c blocked: 5-byte struct copy is `mov cx,2; rep movsw; movsb` (6.00A: movsw; movsw; movsb).
 * 0000 blocked: loop-pointer home store scheduled before the 13-byte struct copy, copy uses
 * `mov cx,6` first and `push ss; pop es` (6.00 vs 6.00A block-copy codegen). Combat overlay s022 (child of s014): 0000 character sheet from combat,
 * 03e4 surrender, 04e4 "%Fs%s%Fs" message about a combatant, 051c choose party order
 * (F1..F5 or click), 07aa (near) take member into the order list.
 * Combatants: 13 records of 0x80 bytes, D_9c65[0..4] party, [5..12] enemies
 * (the party records of D_9c69, based 4 bytes earlier); +4 status (1 active, 2 ?, 5 surrendered),
 * D_a6b4[i] side/kind (0x9d = ?), D_9064[5] chosen flags. */

struct s13 { char b[13]; };
struct s5 { char b[5]; };
struct gfx { char pad[0xc]; int f0c; int f0e; };
struct mouse { int f0; int f2; int f4; int f6; };
struct cmb {
    int w0;   /* +00 */
    char pad2[2];
    char status;   /* +04 */
    char pad5[1];
    char f06;   /* +06 */
    char pad7[1];
    char f08;   /* +08 */
    char pad9[2];
    char f0b;   /* +0b */
    char padc[6];
    char f12;   /* +12 */
    char pad13[27];
    char name[1];   /* +2e */
    char pad2f[11];
    char f3a;   /* +3a */
    char pad3b[6];
    char f41;   /* +41 */
    char pad42[62];
};

extern struct cmb D_9c65[];
extern unsigned char D_a6b4[];
extern char D_a6c1;
extern int D_a893;
extern int D_ca3d;
extern unsigned char D_c651;
/* original: `mov ax,SEG 20a5` = communal `int far x;` (COMDEF), which omflink rejects */
extern int far F_20a5_1f44[];  /* 20a5:1f44 */
extern char far F_286b_0212[];
extern char far F_286b_0222[];
extern char far * far F_286b_0267[];
extern struct gfx *D_017c;
extern int D_0154;
extern char D_9064[];
extern char D_c629;
extern char D_c624[];
extern int D_c62a;
extern struct mouse *D_c4fe;
extern char D_907a;
extern char D_e0a1;
extern unsigned char D_d848[];
extern int D_d8f1[], D_d8d7[], D_d8bd[], D_d8a3[];
extern char D_c102;
extern struct s13 D_c96b;
extern char D_07d3, D_07d8;
extern char D_9085[];
extern char D_9429, D_9433, D_943d, D_9893;
extern int D_d65f, D_d67e;
extern int D_d6ee, D_d6f2, D_d6f9, D_d729, D_d70a, D_d73c, D_d726, D_d800;
extern char D_9069;
extern int D_ca53, D_ca55;
extern char D_9c4c;
extern int D_9c42, D_a68f;
extern char *D_11bc[];
extern int D_ca45[];
extern char D_ca64[];
extern char D_a6b3, D_a6b2;
extern int D_a6b0;
extern char D_c7cc;
extern char D_a695;
extern int D_a693, D_a691;
extern char D_d493[];

int sprintf(char *, const char *, ...);
char *strcpy(char *, const char *);
void *memset(void *, int, unsigned);
int far sub_root_0000_2522(char *);
void far sub_root_0000_24bc(char *, int);
void far sub_root_0000_1588(int);
void far sub_root_047c_12c3(void);
void far sub_root_047c_1293(void);
void far sub_root_0000_23e6();
int far sub_root_047c_073d(void);
int far sub_root_047c_0744(void);
struct mouse * far sub_root_047c_1320(int, int);
void far copy_rect_saved();
void far vec_1de7(void);
void far vec_1cb1(void);
void far free_buffer(int, char *);
int far alloc_buffer(int, int, char *);
void far sub_root_0410_031e(int, int);
void far sub_root_0410_0364(int, int);
void far sub_root_0000_2646(char *);
void far sub_root_047c_124f(int, int, int);
void far sub_root_047c_0568(void);
void far sub_root_047c_056f(void);
void far sub_s014_19cd_36f0(void);
void far sub_s014_19cd_3766(void);
void far sub_root_0000_1716(int);
void far vec_1dfb(int);
void far vec_1ae5(void);
void far vec_1dbf(void);
void far vec_1c7f(void);
void far vec_1db5(int);
void far sub_s014_0e76_38b6(int, int);
void far sub_s014_18bf_0a40(int, int, int);
void far sub_root_047c_04ea(int, int);
void far vec_1c2f(int);
void far sub_s014_14f7_0008(void);
void far sub_s022_1ef8_0000(int);
void far sub_s022_1ef8_03e4(void);
void far sub_s022_1ef8_04e4(int);
void far sub_s022_1ef8_051c(void);
static void near sub_s022_1ef8_07aa(int, int *);

void far sub_s022_1ef8_0000(int y)
{
    struct s13 save;
    char saved[13];
    char st;
    int idx, freed, i;
    char s1, s2, s3, s4;
    int w1, w2;

    idx = y / 40;
    freed = 0;
    i = 0;
    save = D_c96b;
    for (; i < 13; i++)
        saved[i] = D_9c65[i].f0b;
    st = D_9c65[idx].status;
    if (st == 1) {
        if (D_a6b4[idx] == 0x9d) {
            sub_s022_1ef8_04e4(idx);
            return;
        }
        D_07d3++;
        strcpy(D_9085, "SeqDealloc");
        s1 = D_9429;
        s2 = D_9433;
        s3 = D_943d;
        s4 = D_9893;
        D_9893 = D_943d = D_9433 = D_9429 = 2;
        w1 = D_d65f;
        w2 = D_d67e;
        vec_1cb1();
        if (D_d6ee) {
            free_buffer(D_d6ee, "VarFrn");
            freed = 1;
        }
        if (D_d6f2)
            free_buffer(D_d6f2, "Frn");
        if (D_d6f9)
            free_buffer(D_d6f9, "FLR");
        if (D_d729)
            free_buffer(D_d729, "FFX");
        if (D_d70a)
            free_buffer(D_d70a, "NWL");
        if (D_d73c)
            free_buffer(D_d73c, "NFX");
        if (D_d726)
            free_buffer(D_d726, "WWL");
        if (D_d800)
            free_buffer(D_d800, "WFX");
        D_07d8 = 0;
        if (D_9069 == 0) {
            D_ca55 = alloc_buffer(0x300, 0, "PalBuff");
            D_ca53 = 0;
            sub_root_0410_031e(D_ca53, D_ca55);
        }
        sub_root_0000_2646("BeforeCharData");
        if (D_9c4c == 1) {
            sub_root_047c_12c3();
            sub_root_047c_124f(0, 0, D_9c42);
            D_a68f = D_9c42;
        }
        sub_root_047c_0568();
        sub_s014_19cd_36f0();
        sub_root_0000_1716(0x100);
        vec_1dfb(idx);
        sub_root_047c_1320(6, 0);
        D_c62a = 0;
        sub_root_0000_2646("AftCharData");
        sub_root_047c_0568();
        if (D_9069 == 0) {
            sub_root_0410_0364(D_ca53, D_ca55);
            free_buffer(D_ca55, "PalBuff");
        }
        sub_s014_19cd_3766();
        vec_1ae5();
        if (freed == 1)
            vec_1dbf();
        D_07d8 = 1;
        vec_1c7f();
        for (i = 0; i < 6; i++)
            strcpy(D_11bc[i], "");
        for (i = 5; i < 13; i++) {
            if (D_9c65[i].status) {
                int k = D_9c65[i].f06;
                if (D_ca45[k] == 0)
                    vec_1db5(k);
                sub_s014_0e76_38b6(i, D_9c65[i].f08);
            }
        }
        D_9429 = s1;
        D_9433 = s2;
        D_943d = s3;
        D_9893 = s4;
        D_d65f = w1;
        D_d67e = w2;
        D_c96b = save;
        for (i = 0; i < 13; i++) {
            D_9c65[i].f0b = saved[i];
            if (D_9c65[i].status == 1 &&
                (D_9c65[i].w0 == 0x80 || (D_a6b3 == 1 && D_a6b2 == i && D_a6b0 == 0x80)) &&
                D_9c65[i].f12 == -1) {
                D_9c65[i].w0 = 0;
                D_ca64[i] = 0;
                D_9c65[i].f3a = -1;
                sub_s014_18bf_0a40(i, D_9c65[i].f41, 0);
                if (D_a6b3 == 1 && D_a6b2 == i) {
                    D_a6b0 = -1;
                    D_c7cc = 0;
                }
            }
        }
        if (D_9c4c == 1) {
            sub_root_047c_12c3();
            sub_root_047c_124f(0, 0, D_a695 == 0 ? D_a693 : D_a691);
            D_a68f = D_9c42;
            sub_root_047c_04ea(0, 0);
            sub_root_047c_1293();
        }
        memset(D_d493, 0xff, 13);
        vec_1c2f(-1);
        sub_s014_14f7_0008();
        sub_root_047c_056f();
        sub_root_047c_1320(6, 0);
        D_07d3--;
    } else if (st == 2)
        sub_s022_1ef8_04e4(idx);
}

#pragma optimize("a", on)
void far sub_s022_1ef8_03e4(void)
{
    int i;
    int any, other;

    if (D_a6c1 == 1 && sub_root_0000_2522("Do you really want to surrender?") == 1) {
        D_a893 = 4;
        other = any = 0;
        D_ca3d = F_20a5_1f44[0];
        for (i = 5; i < 13; i++)
            if (D_9c65[i].status == 1) {
                D_c651 = D_a6b4[i];
                break;
            }
        for (i = 0; i < 5; i++) {
            if (D_9c65[i].status == 1) {
                if (D_a6b4[i] == 0x9d) {
                    any = 1;
                    D_a893 = 2;
                }
                if (D_a6b4[i] != D_c651)
                    other = 1;
            }
        }
        if (any == 1) {
            D_ca3d = F_20a5_1f44[1];
            for (i = 0; i < 5; i++)
                if (D_a6b4[i] != 0x9d && (D_9c65[i].status == 1 || D_9c65[i].status == 2))
                    D_9c65[i].status = 5;
            return;
        }
        if (other == 1) {
            D_a893 = -1;
            for (i = 0; i < 5; i++)
                if (D_a6b4[i] == D_c651 && (D_9c65[i].status == 1 || D_9c65[i].status == 2))
                    D_9c65[i].status = 5;
        }
    }
}

#pragma optimize("", on)

void far sub_s022_1ef8_04e4(int i)
{
    char buf[0x50];

    sprintf(buf, "%Fs%s%Fs", F_286b_0212, D_9c65[i].name, F_286b_0222);
    sub_root_0000_24bc(buf, 0x96);
}

void far sub_s022_1ef8_051c(void)
{
    char msg[0x50];
    int rect[4];
    int s1, s2;
    struct s5 saved;
    int cnt, k, more, i, j, x, y;

    s1 = D_017c->f0c;
    s2 = D_017c->f0e;
    for (i = 0; i < 5; i++) {
        saved.b[i] = D_9064[i];
        D_9064[i] = 0;
    }
    sub_root_0000_1588(0x100);
    cnt = 0;
    for (i = 0; i < 5; i++)
        if (D_9c65[i].status == 1) {
            D_9064[i] = 1;
            cnt++;
            sub_root_0000_1588(i);
        }
    D_c629 = 0;
    for (k = 0; k < cnt - 1; k++) {
        sprintf(msg, "Select %Fs", F_286b_0267[k]);
        D_017c->f0c = 1;
        D_017c->f0e = 0xff;
        sub_root_047c_12c3();
        sub_root_0000_23e6(D_017c, -1, 1, msg, D_0154, rect);
        sub_root_047c_1293();
        more = 1;
        do {
            if (sub_root_047c_073d()) {
                switch (sub_root_047c_0744()) {
                case 0x3b00:
                    sub_s022_1ef8_07aa(0, &more);
                    break;
                case 0x3c00:
                    sub_s022_1ef8_07aa(1, &more);
                    break;
                case 0x3d00:
                    sub_s022_1ef8_07aa(2, &more);
                    break;
                case 0x3e00:
                    sub_s022_1ef8_07aa(3, &more);
                    break;
                case 0x3f00:
                    sub_s022_1ef8_07aa(4, &more);
                    break;
                }
            } else {
                if (D_c62a == 0) {
                    D_c4fe = sub_root_047c_1320(6, 0);
                    D_c62a = D_c4fe->f2;
                }
                if (D_c62a > 0) {
                    y = D_c4fe->f6;
                    D_c62a = 0;
                    x = D_c4fe->f4;
                    if (x < 60) {
                        i = y / 40;
                        if (D_9c65[i].status == 1 && D_9064[i] == 1)
                            sub_s022_1ef8_07aa(i, &more);
                    } else if (D_907a == 1) {
                        x -= 60;
                        for (j = D_e0a1 - 1; j >= 0; j--)
                            if (D_d848[j] < 5 && D_d8f1[j] <= x && D_d8d7[j] >= x &&
                                D_d8bd[j] <= y && D_d8a3[j] >= y && D_9064[D_d848[j]] == 1)
                                sub_s022_1ef8_07aa(D_d848[j], &more);
                    }
                }
            }
        } while (more);
        sub_root_047c_12c3();
        copy_rect_saved(D_0154, rect[0], rect[1], rect[2], rect[3], D_017c, rect[0], rect[1]);
        sub_root_047c_1293();
    }
    for (i = 0; i < 5; i++)
        if (D_9064[i] == 1)
            sub_s022_1ef8_07aa(i, &more);
    *(struct s5 *)D_9064 = saved;
    sub_root_0000_1588(0x100);
    if (D_c102 == 2)
        vec_1de7();
    D_017c->f0c = s1;
    D_017c->f0e = s2;
}

static void near sub_s022_1ef8_07aa(int i, int *more)
{
    D_c624[D_c629] = i;
    D_9064[i] = 0;
    sub_root_0000_1588(i);
    D_c629++;
    *more = 0;
}
