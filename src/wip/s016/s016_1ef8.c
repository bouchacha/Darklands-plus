/* place: s016 1ef8:0000 */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* s016: combat command keys - party movement modes (group / individual / single file),
   active-member selection, stairs/portal commands, command letter display.
   status: all 59 functions written; 54/59 match. Blockers (MSC 6.00 vs 6.00A): 0000 switch
   layout (6.00: tree spine inline, bodies in source order), 0bd6/0cdc odd-size memset
   (rep stosw + stosb), 0da6 placement of the shared rebuild block; 15be differs only in its
   jump table (absolute offsets, follows from the earlier lengths). Notes: build/agent/e7/NOTES.md */

struct cbt {                    /* combatant record, 0x80 bytes, D_9c55[13] */
    char pad0[4];
    int x;                      /* +04 */
    int y;                      /* +06 */
    int tx;                     /* +08 move target x */
    int ty;                     /* +0a move target y */
    char padc[4];
    int f10;                    /* +10 condition bits */
    char pad12[2];
    char status;                /* +14 1 = active */
    char pad15[5];
    char f1a;                   /* +1a facing */
    char pad1b[0x2f];
    char f4a;                   /* +4a leader in single file (-1 = none) */
    char pad4b[0x35];
};
struct res { int a; int b; };
struct pf {                     /* D_a696[13]: per-combatant flags */
    unsigned lo : 5;
    unsigned b5 : 1;
    unsigned b6 : 1;
    unsigned mid : 5;
    unsigned b12 : 1;           /* "F" instead of "W" */
    unsigned hi : 3;
};
struct q16 { int w0; char f2; char name[4]; char f7; char f8; char f9; char pada[12]; };
struct win { char pad0[0xc]; int f0c; char pade[2]; int f10; };
struct r2c { char b[0x2c]; };
struct r0e { char b[0xe]; };

extern struct cbt D_9c55[];
extern char D_064a[];
extern char D_0880;
extern char D_0881;
extern char D_9631;
extern char D_9645;
extern char D_9659;
extern char D_97dd;
extern char D_97f1;
extern char D_9805;
extern char D_9819;
extern char D_9841;
extern char D_9c26;
extern char D_9c2f;
extern char D_9c3c;
extern char D_9c41;
extern char D_a394;
extern char D_c623;
extern char D_c8b2[];
extern char D_ca30[];
extern char D_d73b;
extern char D_e079;
int far sub_root_0000_13a0();
int far vec_19eb();
int far vec_1a4f();
int far vec_1d65();
#define KBFLAGS (*(char far *)0x00400017L)
extern struct pf D_a696[];
extern struct q16 D_9a47[];
extern struct win *D_0168;
extern int D_0190;
extern char D_074a[];
extern char D_0768[];
extern char D_0788[];
extern char D_07aa;
extern char D_07bc;
extern int D_07d4;
extern char D_07d3;
extern char D_11b1;
extern char D_11ca;
extern char D_17b0;
extern char D_907b;
extern char D_9085[];
extern char D_9064[];
extern char D_9609;
extern char D_966d;
extern char D_9681;
extern char D_9695;
extern char D_96a9;
extern char D_96bd;
extern char D_96d1;
extern char D_96e5;
extern char D_96f9;
extern char D_970d;
extern char D_9749;
extern char D_982d;
extern char D_a695;
extern int D_a6b0;
extern char D_a6b2;
extern char D_a6b3;
extern char D_a6b4[];
extern char D_a6c1;
extern int D_a6c2;
extern int D_a6d1;
extern char D_a6d3[];
extern char D_c102;
extern char D_c1ce;
extern char D_c1f2;
extern char D_c624[];
extern char D_c629;
extern unsigned char D_c64c[];
extern int D_c652;
extern unsigned char D_c77e[];
extern unsigned char D_c784[];
extern char D_c7cc;
extern char D_c7cd;
extern char D_c866;
extern char D_c886;
extern char D_ca64[];
extern char D_ca99;
extern char D_d493[];
extern char D_d505[][8];
extern char D_d610;
extern char D_d6ea;
extern char D_d6ed;
extern char D_d6f1;
extern char D_d6fb[];
extern char D_d7fe;
extern char D_d845;
extern unsigned char D_d848[];
extern unsigned char D_d855[];
extern int D_d86f[];
extern int D_d889[];
extern int D_d97c[];
extern char D_e07b;
extern int D_e09d;
extern int D_e09f;
extern char D_e0a1;
extern int D_e1bc;
extern char D_e1c8;
extern char D_e1ca;
extern char far F_286b_0231[];
extern char far F_20a5_6238[][0xfa];
extern struct r2c far F_20a5_3016[];
extern struct r0e far F_20a5_1f5b[];

void *memset(void *, int, unsigned);
char *strcpy(char *, const char *);
char far * far _fstrcpy(char far *, const char far *);
int far random_n(int);
int far sub_root_0000_0caa();
int far sub_root_0000_1588();
int far sub_root_0000_1716();
int far sub_root_0000_24bc();
int far sub_root_0265_006e();
int far sub_root_047c_051b();
int far sub_root_047c_0553();
int far sub_root_047c_05a5();
int far sub_root_047c_073d();
int far sub_root_047c_0744();
int far sub_s014_0e76_27ac();
int far sub_s014_0e76_27cc();
int far sub_s014_0e76_38b6();
int far sub_s014_0e76_3d0c();
int far sub_s014_0e76_4780();
int far sub_s014_12f7_000a();
int far sub_s014_12f7_1554();
int far sub_s014_14f7_343c();
int far sub_s014_18bf_0fa4();
int far sub_s014_19cd_069c();
int far sub_s014_19cd_0738();
int far sub_s014_19cd_28a2();
int far vec_1b3f();
int far vec_1c2f();
int far vec_1cd9();
int far vec_1d01();
int far vec_1d0b();
int far vec_1d15();
int far vec_1d1f(void far *);

void far sub_s016_1ef8_0000();
void far sub_s016_1ef8_09bc();
void far sub_s016_1ef8_0a2c();
void far sub_s016_1ef8_0aba();
void far sub_s016_1ef8_0b92();
void far sub_s016_1ef8_0bd6();
void far sub_s016_1ef8_0cdc();
void far sub_s016_1ef8_0da6();
void far sub_s016_1ef8_0e2e();
void far sub_s016_1ef8_0e7e();
void far sub_s016_1ef8_0f76();
void far sub_s016_1ef8_101e();
void far sub_s016_1ef8_1072();
void far sub_s016_1ef8_10b2();
void far sub_s016_1ef8_1182();
void far sub_s016_1ef8_1242();
void far sub_s016_1ef8_130a();
void far sub_s016_1ef8_1314();
void far sub_s016_1ef8_131e();
void far sub_s016_1ef8_1328();
void far sub_s016_1ef8_1332();
void far sub_s016_1ef8_133e();
void far sub_s016_1ef8_1364();
void far sub_s016_1ef8_136e();
void far sub_s016_1ef8_137a();
void far sub_s016_1ef8_1384();
void far sub_s016_1ef8_138e();
void far sub_s016_1ef8_1398();
void far sub_s016_1ef8_13a2();
void far sub_s016_1ef8_13ac();
void far sub_s016_1ef8_13b6();
void far sub_s016_1ef8_13c0();
void far sub_s016_1ef8_13ca();
void far sub_s016_1ef8_13d4();
void far sub_s016_1ef8_1436();
void far sub_s016_1ef8_1466();
void far sub_s016_1ef8_1470();
void far sub_s016_1ef8_147a();
void far sub_s016_1ef8_1484();
void far sub_s016_1ef8_148e();
void far sub_s016_1ef8_1498();
void far sub_s016_1ef8_14a2();
void far sub_s016_1ef8_14ac();
void far sub_s016_1ef8_14b6();
void far sub_s016_1ef8_14c0();
void far sub_s016_1ef8_14ca();
void far sub_s016_1ef8_14d4();
void far sub_s016_1ef8_14de();
void far sub_s016_1ef8_14e8();
void far sub_s016_1ef8_14f2();
void far sub_s016_1ef8_14fc();
void far sub_s016_1ef8_1506();
void far sub_s016_1ef8_1510();
void far sub_s016_1ef8_151a();
void far sub_s016_1ef8_1524();
void far sub_s016_1ef8_152e();
int far sub_s016_1ef8_156e();
void far sub_s016_1ef8_1590();
void far sub_s016_1ef8_15be();

/* combat keyboard handler (also takes joystick directions from D_0880/D_0881) */
/* combat keyboard handler (also takes joystick directions from D_0880/D_0881) */
/* combat keyboard handler (also takes joystick directions from D_0880/D_0881) */
void far sub_s016_1ef8_0000(void)
{
    int k;
    int i;

    k = 0;
    if (sub_root_047c_073d()) {
        sub_root_0000_0caa();
        if (sub_root_047c_073d())
            k = sub_root_047c_0744();
    } else if (D_0880 < 0 && D_0881 < 0)
        k = 0x4700;
    else if (D_0880 < 0 && D_0881 > 0)
        k = 0x4f00;
    else if (D_0880 > 0 && D_0881 < 0)
        k = 0x4900;
    else if (D_0880 > 0 && D_0881 > 0)
        k = 0x5100;
    else if (D_0880 < 0)
        k = 0x4b00;
    else if (D_0880 > 0)
        k = 0x4d00;
    else if (D_0881 < 0)
        k = 0x4800;
    else if (D_0881 > 0)
        k = 0x5000;

    if (D_c623 == D_11ca)
        goto main;
    /* party split over two levels: only some commands */
    switch (k) {
    case 0x4800: case 0x4838: case 0x5000: case 0x5032: case 0x4b00: case 0x4b34:
    case 0x4d00: case 0x4d36: case 0x4700: case 0x4737: case 0x4f00: case 0x4f31:
    case 0x5100: case 0x5133: case 0x4900: case 0x4939:
        sub_root_0000_24bc(D_064a, 0x96);
        return;
    case 0x011b: case 0x0231: case 0x0332: case 0x0433: case 0x0534: case 0x0635:
    case 0x1c0d: case 0x3920: case 0x3b00: case 0x3c00: case 0x3d00: case 0x3e00:
    case 0x3f00: case 0x5e00: case 0x5f00: case 0x6000: case 0x6100: case 0x6200:
main:
            switch (k) {
            case 0x4800:
                if ((KBFLAGS & 2) || (KBFLAGS & 1)) {
                    D_c866--;
                    D_c886--;
                    sub_s016_1ef8_0b92();
                } else if (D_a6b4[D_17b0] == D_11ca && D_17b0 < 5 && D_c102 != 1) {
                    D_e079 = 1;
                    D_9c55[D_17b0].f10 = 1;
                    D_ca30[D_17b0] = 0;
                }
                break;
            case 0x5000:
                if ((KBFLAGS & 2) || (KBFLAGS & 1)) {
                    D_c866++;
                    D_c886++;
                    sub_s016_1ef8_0b92();
                } else if (D_a6b4[D_17b0] == D_11ca && D_17b0 < 5 && D_c102 != 1) {
                    D_e079 = 1;
                    D_9c55[D_17b0].f10 = 1;
                    D_ca30[D_17b0] = 0x44;
                }
                break;
            case 0x4b00:
                if ((KBFLAGS & 2) || (KBFLAGS & 1)) {
                    D_c866--;
                    sub_s016_1ef8_0b92();
                } else if (D_a6b4[D_17b0] == D_11ca && D_17b0 < 5 && D_c102 != 1) {
                    D_e079 = 1;
                    D_9c55[D_17b0].f10 = 1;
                    D_ca30[D_17b0] = 0x66;
                }
                break;
            case 0x4d00:
                if ((KBFLAGS & 2) || (KBFLAGS & 1)) {
                    D_c866++;
                    sub_s016_1ef8_0b92();
                } else if (D_a6b4[D_17b0] == D_11ca && D_17b0 < 5 && D_c102 != 1) {
                    D_e079 = 1;
                    D_9c55[D_17b0].f10 = 1;
                    D_ca30[D_17b0] = 0x22;
                }
                break;
            case 0x4700:
                if ((KBFLAGS & 2) || (KBFLAGS & 1)) {
                    D_c866 -= 2;
                    D_c886--;
                    sub_s016_1ef8_0b92();
                } else if (D_a6b4[D_17b0] == D_11ca && D_17b0 < 5 && D_c102 != 1) {
                    D_e079 = 1;
                    D_9c55[D_17b0].f10 = 1;
                    D_ca30[D_17b0] = 0x77;
                }
                break;
            case 0x4f00:
                if (KBFLAGS & 3) {
                    D_c886++;
                    sub_s016_1ef8_0b92();
                } else if (D_a6b4[D_17b0] == D_11ca && D_17b0 < 5 && D_c102 != 1) {
                    D_e079 = 1;
                    D_9c55[D_17b0].f10 = 1;
                    D_ca30[D_17b0] = 0x55;
                }
                break;
            case 0x5100:
                if ((KBFLAGS & 2) || (KBFLAGS & 1)) {
                    D_c866 += 2;
                    D_c886++;
                    sub_s016_1ef8_0b92();
                } else if (D_a6b4[D_17b0] == D_11ca && D_17b0 < 5 && D_c102 != 1) {
                    D_e079 = 1;
                    D_9c55[D_17b0].f10 = 1;
                    D_ca30[D_17b0] = 0x33;
                }
                break;
            case 0x4900:
                if (KBFLAGS & 3) {
                    D_c886--;
                    sub_s016_1ef8_0b92();
                } else if (D_a6b4[D_17b0] == D_11ca && D_17b0 < 5 && D_c102 != 1) {
                    D_e079 = 1;
                    D_9c55[D_17b0].f10 = 1;
                    D_ca30[D_17b0] = 0x11;
                }
                break;
            case 0x4838: D_c866--; D_c886--; sub_s016_1ef8_0b92(); break;
            case 0x5032: D_c866++; D_c886++; sub_s016_1ef8_0b92(); break;
            case 0x4b34: D_c866--; sub_s016_1ef8_0b92(); break;
            case 0x4d36: D_c866++; sub_s016_1ef8_0b92(); break;
            case 0x4737: D_c866 -= 2; D_c886--; sub_s016_1ef8_0b92(); break;
            case 0x4f31: D_c886++; sub_s016_1ef8_0b92(); break;
            case 0x5133: D_c866 += 2; D_c886++; sub_s016_1ef8_0b92(); break;
            case 0x4939: D_c886--; sub_s016_1ef8_0b92(); break;
            case 0x3b00: vec_1d65(10); break;
            case 0x3c00: vec_1d65(0x32); break;
            case 0x3d00: vec_1d65(0x5a); break;
            case 0x3e00: vec_1d65(0x82); break;
            case 0x3f00: vec_1d65(0xaa); break;
            case 0x5e00: sub_s016_1ef8_09bc(0); break;
            case 0x5f00: sub_s016_1ef8_09bc(1); break;
            case 0x6000: sub_s016_1ef8_09bc(2); break;
            case 0x6100: sub_s016_1ef8_09bc(3); break;
            case 0x6200: sub_s016_1ef8_09bc(4); break;
            case 0x0231: sub_s016_1ef8_0a2c(0); break;
            case 0x0332: sub_s016_1ef8_0a2c(1); break;
            case 0x0433: sub_s016_1ef8_0a2c(2); break;
            case 0x0534: sub_s016_1ef8_0a2c(3); break;
            case 0x0635: sub_s016_1ef8_0a2c(4); break;
            case 0x1e41:
            case 0x1e61:
                if (D_97f1 == 1 && D_a6b2 != -1)
                    sub_s016_1ef8_130a();
                break;
            case 0x3042:
            case 0x3062:
                if (D_9819 == 1 && D_a6b2 != -1)
                    sub_s016_1ef8_131e();
                break;
            case 0x2044:
            case 0x2064:
                sub_s016_1ef8_0f76();
                break;
            case 0x1245:
            case 0x1265:
                if (D_9631 == 1)
                    vec_1a4f();
                break;
            case 0x2146:
            case 0x2166:
                if (D_a695 == 1 && D_9659 == 1 && D_a6b2 != -1)
                    sub_s016_1ef8_133e();
                break;
            case 0x2247:
            case 0x2267:
                if (D_9681 == 1)
                    sub_s016_1ef8_0bd6();
                break;
            case 0x2348:
            case 0x2368:
                sub_s016_1ef8_101e();
                break;
            case 0x324d:
            case 0x326d:
                if (D_9841 == 1)
                    sub_s016_1ef8_136e();
                break;
            case 0x184f:
            case 0x186f:
                sub_s016_1ef8_1072();
                break;
            case 0x1950:
            case 0x1970:
                sub_s016_1ef8_10b2();
                break;
            case 0x1051:
            case 0x1071:
                if (D_9695 == 1)
                    sub_s016_1ef8_0cdc();
                break;
            case 0x1f53:
            case 0x1f73:
                vec_19eb();
                break;
            case 0x1454:
            case 0x1474:
                if (D_97dd == 1) {
                    D_9c26 = 1;
                    D_9c2f = 2;
                    D_9c3c = D_9c41 = -1;
                    D_a394 = 3;
                }
                break;
            case 0x1655:
            case 0x1675:
                sub_s016_1ef8_1182();
                break;
            case 0x2f56:
            case 0x2f76:
                if (D_9805 == 1 && D_a6b2 != -1)
                    sub_s016_1ef8_1314();
                break;
            case 0x1157:
            case 0x1177:
                if (D_a695 == 1 && D_9645 == 1 && D_a6b2 != -1)
                    sub_s016_1ef8_1332();
                break;
            case 0x2d58:
            case 0x2d78:
                sub_s016_1ef8_1242();
                break;
            case 0x1800:
                for (i = 0; i < 13; i++) {
                    D_9c55[i].f10 = 0;
                    D_a6d3[i] = 0;
                }
                D_a6c1 = 0;
                sub_s014_19cd_28a2();
                break;
            case 0x1c0d:
                if (D_d7fe == 1 && D_c102 == 1)
                    D_d73b = 1;
                else if (D_c102 == 0)
                    sub_s014_19cd_0738();
                break;
            case 0x3920:
                if (D_a695 == 0) {
                    D_a695 = 1;
                    D_07d3++;
                    sub_s014_0e76_4780();
                    D_9609 = 1;
                } else
                    sub_s016_1ef8_1436();
                break;
            case 0x011b:
                if (D_a695 == 1)
                    sub_s016_1ef8_13d4();
                break;
            }
        break;
    case 0x1655:
    case 0x1675:
        if (D_a695 == 1) {
            if (D_96bd == 1) {
                D_c77e[D_a6b2] = D_c8b2[D_a6b2];
                sub_s016_1ef8_1384();
            }
            return;
        }
        if (D_c8b2[D_907b] != -1)
            vec_1cd9(D_c8b2[D_907b], D_907b);
        return;
    case 0x1000:
        sub_root_0000_13a0();
        return;
    }
}

/* make party member i the active one (no-op for absent / unconscious / fled members) */
void far sub_s016_1ef8_09bc(int i)
{
    union { char t; int d; } u;     /* status copy (kept in a word slot) */

    if (i < 5 && (u.t = *(volatile char *)&u.t = D_9c55[i].status) != 0 && D_a6b4[i] != -99 && u.t != 3 && u.t != 5) {
        D_11b1 = 1;
        D_17b0 = i;
        D_907b = i;
        if (D_a6b4[i] != D_11ca) {
            vec_1b3f(D_a6b4[i]);
            D_11ca = D_a6b4[i];
            D_e07b = 1;
        }
        sub_root_0000_1716(0x100);
        sub_s014_19cd_28a2();
    }
}

#pragma optimize("a", on)
void far sub_s016_1ef8_0a2c(int i)
{
    union { char t; int d; } u;     /* status copy (kept in a word slot) */

    if (i < 5 && (u.t = *(volatile char *)&u.t = D_9c55[i].status) != 0 && D_a6b4[i] != -99 && u.t != 3 && u.t != 5) {
        if (D_a6b2 != -1)
            sub_s014_19cd_0738();
        D_11b1 = 1;
        D_17b0 = i;
        D_d6ea = D_a696[i].b12;
        if (D_a6b4[i] != D_11ca) {
            vec_1b3f(D_a6b4[i]);
            D_11ca = D_a6b4[i];
            vec_1c2f(i);
        }
        sub_s014_19cd_28a2();
        D_d6f1 = 1;
        D_d6ed = i;
    }
}
#pragma optimize("", on)

void far sub_s016_1ef8_0aba(int i)
{
    union { char t; int d; } u;     /* status copy (kept in a word slot) */
    int k;

    if (i < 5 && (u.t = *(volatile char *)&u.t = D_9c55[i].status) != 0 && D_a6b4[i] != -99 && u.t != 3 && u.t != 5) {
        D_11b1 = 1;
        D_17b0 = i;
        if (D_a6b4[i] != D_11ca) {
            vec_1b3f(D_a6b4[i]);
            D_11ca = D_a6b4[i];
            D_e07b = 1;
        }
        D_a6b2 = i;
        for (k = 0; k < D_e0a1; k++) {
            if (D_d848[k] == i) {
                D_e09f = D_d505[sub_s014_14f7_343c(D_a6b2)][D_9c55[D_a6b2].f1a] + D_d889[k] - 4;
                D_e09d = (D_d855[k] >> 1) + D_d86f[k] - 0xd;
                break;
            }
        }
        sub_s014_19cd_069c();
    }
}

void far sub_s016_1ef8_0b92(void)
{
    if (D_c866 < 1) {
        D_c866 = 1;
    } else {
        if (D_c866 > 0x27) {
            D_c866 = 0x27;
        }
    }
    if (D_c886 < 1) {
        D_c886 = 1;
    } else {
        if (D_c886 > 0x27) {
            D_c886 = 0x27;
        }
    }
    D_11b1 = 0;
    vec_1c2f(-1);
}

/* toggle "travel as a group" */
void far sub_s016_1ef8_0bd6(void)
{
    char msg[80];
    int k;

    D_c102 = D_c102 != 1 ? 1 : 0;
    if (D_c102 == 1) {
        strcpy(msg, D_074a);
        D_07bc = 2;
        D_07aa = 1;
        D_d7fe = 1;
        sub_s016_1ef8_0da6(1);
        memset(D_a6d3, 0, 5);
        for (k = 0; k < 5; k++) {
            D_9c55[k].f4a = -1;
            sub_s014_0e76_3d0c(k);
            D_9c55[k].f10 = 0;
        }
    } else {
        strcpy(msg, D_0768);
        D_07aa = 2;
        D_07bc = 2;
        D_d7fe = 0;
        if (D_d610 == 0)
            D_9681 = 2;
        memset(D_a6d3, 0, 5);
        for (k = 0; k < 5; k++) {
            D_9c55[k].f4a = -1;
            D_9c55[k].f10 = 0;
        }
    }
    sub_root_0000_24bc(msg, 0x3c);
    sub_s014_19cd_28a2();
    if (D_a695 == 1)
        D_9609 = 1;
}

/* toggle "travel in single file" */
void far sub_s016_1ef8_0cdc(void)
{
    char msg[80];
    int k;

    D_d7fe = 0;
    D_c102 = D_c102 != 2 ? 2 : 0;
    if (D_c102 == 2) {
        strcpy(msg, D_0788);
        D_07aa = 2;
        D_07bc = 1;
        sub_s016_1ef8_0da6(1);
        sub_s016_1ef8_0e7e();
    } else {
        strcpy(msg, D_0768);
        D_07aa = 2;
        D_07bc = 2;
        if (D_d610 == 0)
            D_9695 = 2;
        memset(D_a6d3, 0, 5);
        for (k = 0; k < 5; k++) {
            D_9c55[k].f4a = -1;
            D_9c55[k].f10 = 0;
        }
    }
    sub_root_0000_24bc(msg, 0x3c);
    sub_s014_19cd_28a2();
    if (D_a695 == 1)
        D_9609 = 1;
}

/* rebuild the marching order D_c624 unless it still lists exactly the active members */
void far sub_s016_1ef8_0da6(int p)
{
    int ok;
    int i;
    int j;
    int found;

    if (D_c629 == 0) {
rebuild:
        sub_s016_1ef8_0e2e(p);
        return;
    } else {
        ok = 1;
        for (j = 0; j < D_c629; j++)
            if (D_9c55[D_c624[j]].status != 1)
                ok = 0;
        if (!ok)
            goto rebuild;
        for (i = 0; i < 5; i++) {
            if (D_9c55[i].status == 1) {
                found = 0;
                for (j = 0; j < D_c629; j++) {
                    if (D_c624[j] == i) {
                        found = 1;
                        break;
                    }
                }
                if (!found)
                    goto rebuild;
            }
        }
    }
}

void far sub_s016_1ef8_0e2e(int p)
{
    char msg[80];
    int i;
    int n;

    n = 0;
    for (i = 0; i < 5; i++)
        if (D_9c55[i].status == 1)
            D_c624[n++] = i;
    D_c629 = n;
    if (p == 1) {
        _fstrcpy(msg, F_286b_0231);
        sub_root_0000_24bc(msg, 0x3c);
    }
}

/* single file: every member follows the previous one in the marching order */
void far sub_s016_1ef8_0e7e(void)
{
    int k;

    sub_s016_1ef8_09bc(D_c624[0]);
    *(volatile char far *)F_20a5_6238[D_c624[0]] = -1;   /* clear the path */
    D_d6fb[D_c624[0]] = 0;
    D_9c55[D_c624[0]].f4a = -1;
    sub_s014_0e76_3d0c(D_c624[0]);
    D_9c55[D_c624[0]].f10 = 0;
    for (k = 1; k < D_c629; k++) {
        sub_s014_0e76_3d0c(D_c624[k]);
        D_9c55[D_c624[k]].f10 = 0;
        D_9c55[D_c624[k]].f4a = k - 1;
        D_9c55[D_c624[k]].tx = D_9c55[D_c624[k - 1]].x;
        D_9c55[D_c624[k]].ty = D_9c55[D_c624[k - 1]].y;
        if (sub_s014_12f7_000a(D_c624[k]) == 1) {
            sub_s014_12f7_1554(D_c624[k]);
            D_d6fb[D_c624[k]] = 0;
            D_d97c[D_c624[k]] = random_n(0x3c) + D_07d4 + 0x1e;
            D_a6d3[D_c624[k]] = 1;
            D_9c55[D_c624[k]].f10 = 1;
        }
    }
}

void far sub_s016_1ef8_0f76(void)
{
    if (D_a695 == 1) {
        if (D_96f9 == 1) {
            sub_s016_1ef8_13c0();
            return;
        }
        if (D_970d != 1) {
            return;
        }
        sub_s016_1ef8_13b6();
        return;
    }
    if (D_96f9 == 1) {
        sub_root_0000_24bc(0x590, 0x96);
        sub_s014_18bf_0fa4(D_a6b2, (unsigned char)D_c7cd);
        if (!D_ca99) {
            goto L_0ffd;
        }
        vec_1d0b((unsigned char)D_c64c[D_907b], D_907b);
        goto L_1006;
    }
    if (D_970d != 1) {
        goto L_1014;
    }
    if (vec_1d15(D_907b) != 1) {
        goto L_1014;
    }
    goto L_0ffd;
    goto L_0ffd;
L_0ffd:
    vec_1d01(D_907b);
L_1006:
    vec_1c2f(-1);
    goto L_1014;
    goto L_1014;
L_1014:
    sub_s014_0e76_27ac(D_907b);
}

void far sub_s016_1ef8_101e(void)
{
    int i;

    if (D_966d == 1) {
        if (D_c102 == 0) {
            sub_s016_1ef8_1364();
            return;
        }
        for (i = 0; i < 5; i++) {
            if (D_9c55[i].status == 1) {
                D_9c55[i].f10 = 0;
                D_a6d3[i] = 0;
                sub_root_0000_1716(i);
                sub_s014_0e76_38b6(i, 5);
                D_ca64[i] = 0;
            }
        }
    }
}

void far sub_s016_1ef8_1072(void)
{
    if (D_96d1 == 1) {
        if (D_a695 == 1) {
            if (D_a6b2 == -1) {
                return;
            }
            sub_s016_1ef8_13a2();
            return;
        }
        vec_1d01(D_907b);
        vec_1c2f(-1);
        sub_s014_0e76_27ac(D_907b);
    }
}

void far sub_s016_1ef8_10b2(void)
{
    if (D_a695 == 1) {
        if (D_982d == 1 && D_a6b2 != -1) {
            sub_s016_1ef8_1328();
            return;
        }
        if (D_96e5 == 1) {
            if (D_ca99 == 0) {
                sub_s016_1ef8_13ac();
                return;
            }
            sub_s016_1ef8_1398();
            return;
        }
    } else if (D_96e5 == 1) {
        D_e1ca = D_907b;
        if (D_ca99 == 0) {
            if (vec_1d1f(&F_20a5_3016[D_c784[D_907b]]) != 1)
                goto done;
            vec_1d01(D_907b);
        } else {
            if (vec_1d1f(&F_20a5_1f5b[D_c64c[D_907b]]) != 1)
                goto done;
            vec_1d0b(D_c64c[D_907b], D_907b);
        }
        vec_1c2f(-1);
done:
        sub_s014_0e76_27ac(D_907b);
    }
}

void far sub_s016_1ef8_1182(void)
{
    int i;

    if (D_a695 == 1) {
        if (D_a6b2 != -1) {
            if (D_96bd == 1) {
                sub_s016_1ef8_1384();
                return;
            }
            if (D_96a9 == 1) {
                sub_s016_1ef8_137a();
                return;
            }
        }
    } else {
        if (D_96bd == 1) {
            if (D_c102 == 0) {
                vec_1cd9(D_c77e[D_907b], D_907b);
            } else {
                for (i = 0; i < 5; i++)
                    if (D_9c55[i].status == 1)
                        vec_1cd9(D_c77e[D_907b], i);
            }
        } else if (D_96a9 == 1) {
            vec_1d0b(D_c64c[D_907b], D_907b);
            vec_1c2f(-1);
        }
        sub_s014_0e76_27ac(D_907b);
    }
}

void far sub_s016_1ef8_1242(void)
{
    int i;

    if (D_9749 == 1) {
        if (D_a695 == 1) {
            if (D_a6b2 != -1) {
                sub_s016_1ef8_138e();
                return;
            }
        } else if (D_c102 == 0) {
            D_a6b4[D_907b] = -99;
            D_a696[D_907b].b5 = 0;
            D_a696[D_907b].b6 = 1;
            sub_s014_0e76_27ac(D_907b);
            sub_root_0000_1716(D_907b);
            sub_s014_0e76_27cc(D_907b);
            vec_1c2f(-1);
        } else if (D_a6c1 == 0) {
            for (i = 0; i < 5; i++) {
                if (D_9c55[i].status == 1) {
                    D_a6b4[i] = -99;
                    D_a696[i].b5 = 0;
                    D_a696[i].b6 = 1;
                    sub_s014_0e76_27cc(i);
                    sub_s014_0e76_27ac(i);
                }
            }
            sub_root_0000_1716(0x100);
        }
    }
}

void far sub_s016_1ef8_130a(void)
{
    sub_s016_1ef8_156e(0x12);
}

void far sub_s016_1ef8_1314(void)
{
    sub_s016_1ef8_156e(0xa);
}

void far sub_s016_1ef8_131e(void)
{
    sub_s016_1ef8_156e(6);
}

void far sub_s016_1ef8_1328(void)
{
    sub_s016_1ef8_156e(0x22);
}

void far sub_s016_1ef8_1332(void)
{
    D_a6b0 = 1;
    sub_s016_1ef8_15be();
}

void far sub_s016_1ef8_133e(void)
{
    D_a6b0 = 1;
    D_a696[D_a6b2].b12 = 1;
    sub_s016_1ef8_15be();
    D_a696[D_a6b2].b12 = 1;
}

void far sub_s016_1ef8_1364(void)
{
    sub_s016_1ef8_1590(0xa);
}

void far sub_s016_1ef8_136e(void)
{
    sub_s016_1ef8_156e(0x80);
}

void far sub_s016_1ef8_137a(void)
{
    sub_s016_1ef8_1590(1);
}

void far sub_s016_1ef8_1384(void)
{
    sub_s016_1ef8_1590(2);
}

void far sub_s016_1ef8_138e(void)
{
    sub_s016_1ef8_1590(8);
}

void far sub_s016_1ef8_1398(void)
{
    sub_s016_1ef8_1590(3);
}

void far sub_s016_1ef8_13a2(void)
{
    sub_s016_1ef8_1590(5);
}

void far sub_s016_1ef8_13ac(void)
{
    sub_s016_1ef8_1590(6);
}

void far sub_s016_1ef8_13b6(void)
{
    sub_s016_1ef8_1590(4);
}

void far sub_s016_1ef8_13c0(void)
{
    sub_s016_1ef8_1590(7);
}

void far sub_s016_1ef8_13ca(void)
{
    sub_s016_1ef8_1590(9);
}

void far sub_s016_1ef8_13d4(void)
{
    D_a6b0 = -1;
    D_c652 = 0;
    D_a696[D_a6b2].b12 = D_d6ea;
    *(volatile char *)&D_a6b3 = 0;
    D_9064[D_a6b2] = D_c7cc = 0;
    sub_root_0000_1588(D_a6b2);
    sub_root_0000_1716(0x100);
    D_e1bc = 0;
    D_d845 = D_a6b2 = -1;
    sub_s014_19cd_28a2();
}

void far sub_s016_1ef8_1436(void)
{
    if (D_a6b2 != -1) {
        sub_s014_19cd_0738();
    }
    D_a695 = 0;
    sub_s014_0e76_4780();
    D_07d3--;
    D_9609 = 2;
    memset(D_d493, 0xff, 0xd);
}

void far sub_s016_1ef8_1466(void)
{
    sub_s016_1ef8_152e(0);
}

void far sub_s016_1ef8_1470(void)
{
    sub_s016_1ef8_152e(1);
}

void far sub_s016_1ef8_147a(void)
{
    sub_s016_1ef8_152e(2);
}

void far sub_s016_1ef8_1484(void)
{
    sub_s016_1ef8_152e(3);
}

void far sub_s016_1ef8_148e(void)
{
    sub_s016_1ef8_152e(4);
}

void far sub_s016_1ef8_1498(void)
{
    sub_s016_1ef8_152e(5);
}

void far sub_s016_1ef8_14a2(void)
{
    sub_s016_1ef8_152e(6);
}

void far sub_s016_1ef8_14ac(void)
{
    sub_s016_1ef8_152e(7);
}

void far sub_s016_1ef8_14b6(void)
{
    sub_s016_1ef8_152e(8);
}

void far sub_s016_1ef8_14c0(void)
{
    sub_s016_1ef8_152e(9);
}

void far sub_s016_1ef8_14ca(void)
{
    sub_s016_1ef8_152e(0xa);
}

void far sub_s016_1ef8_14d4(void)
{
    sub_s016_1ef8_152e(0xb);
}

void far sub_s016_1ef8_14de(void)
{
    sub_s016_1ef8_152e(0xc);
}

void far sub_s016_1ef8_14e8(void)
{
    sub_s016_1ef8_152e(0xd);
}

void far sub_s016_1ef8_14f2(void)
{
    sub_s016_1ef8_152e(0xe);
}

void far sub_s016_1ef8_14fc(void)
{
    sub_s016_1ef8_152e(0xf);
}

void far sub_s016_1ef8_1506(void)
{
    sub_s016_1ef8_152e(0x10);
}

void far sub_s016_1ef8_1510(void)
{
    sub_s016_1ef8_152e(0x11);
}

void far sub_s016_1ef8_151a(void)
{
    sub_s016_1ef8_152e(0x12);
}

void far sub_s016_1ef8_1524(void)
{
    sub_s016_1ef8_152e(0x13);
}

void far sub_s016_1ef8_152e(int k)
{
    D_a696[D_a6b2].b12 = 0;
    D_a6b0 = 0x100;
    D_c1ce = D_9a47[k].f9;
    D_c1f2 = D_9a47[k].f8;
    D_e1c8 = D_c1ce == 4 ? 1 : 0;
    sub_s016_1ef8_15be();
}

int far sub_s016_1ef8_156e(int p_6)
{
    D_a6b0 = p_6;
    if (D_d845 < -1) {
        D_d845 = -1;
        D_e1bc = 0;
    }
    sub_s016_1ef8_15be();
}

void far sub_s016_1ef8_1590(int v)
{
    D_a6b0 = 0x400;
    D_a6d1 = v;
    if (D_d845 < -1) {
        D_d845 = -1;
        D_e1bc = 0;
    }
    sub_s016_1ef8_15be();
    D_e1ca = D_a6b2;
}

/* show the letter of the current command next to the active member's portrait */
#pragma optimize("a", on)
void far sub_s016_1ef8_15be(void)
{
    unsigned t;
    int s;

    t = D_a6b0;
    if (t != 0x400) {
        switch (t) {
        case 1:
            D_9085[0] = D_a696[D_a6b2].b12 ? 'F' : 'W';
            break;
        case 6:
            D_9085[0] = 'B';
            break;
        case 10:
            D_9085[0] = 'V';
            break;
        case 0x12:
            D_9085[0] = 'A';
            break;
        case 0x22: D_9085[0] = 'P'; break;
        case 0x40:
            D_9085[0] = 'R';
            break;
        case 0x80:
            D_9085[0] = 'M';
            break;
        case 0x100:
            D_9085[0] = 'T';
            break;
        }
    } else {
        switch (D_a6d1) {
        case 1:
        case 2:
            D_9085[0] = 'U';
            break;
        case 3:
        case 6:
            D_9085[0] = 'P';
            break;
        case 4:
        case 7:
            D_9085[0] = 'D';
            break;
        case 5:
            D_9085[0] = 'O';
            break;
        case 8:
            D_9085[0] = 'X';
            break;
        case 10:
            D_9085[0] = 'H';
            break;
        }
    }
    D_9085[1] = 0;
    sub_root_047c_05a5(D_0168, 6, D_a6b2 * 40 + 0x1f, D_a6c2);
    s = D_0168->f0c;
    D_0168->f0c = 0xff;
    sub_root_0265_006e(D_0168, 6, D_a6b2 * 40 + 0x1f, D_9085);
    D_0168->f0c = s;
    sub_root_047c_0553(D_0168, 6, D_a6b2 * 40 + 0x1f, 10, sub_root_047c_051b(D_0168->f10),
                       D_0190, 6, D_a6b2 * 40 + 0x1f);
    D_a696[D_a6b2].b12 = 0;
}
#pragma optimize("", on)
