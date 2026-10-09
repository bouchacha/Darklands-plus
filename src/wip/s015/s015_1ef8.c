/* place: s015 1ef8:0000 */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* status: wip 4/7 (0000, 0046, 00dc, 01a8). 01c6/0708/0c0e: dead induction-variable home
 * stores and SI/DI choice of the 6.00 compiler (`mov [bp-4],0`, `mov [bp-2],3Ch; mov si,[bp-2]`). Combat overlay s015 (child of s014): debug/statistics screens.
 * 0000 tracks the smallest free DOS memory seen (D_1fc2, bytes); 0046 shows it;
 * 00dc debug screen loop (ESC leaves, keys 1..6 page through 3 tables of 10 combatants,
 * starting at 0 or 3); 01a8 row colour (odd rows 9, even 15); 01c6 combatant record dump
 * (D_9c55, labels D_1f2c[26]); 0708 attributes/skills (D_9cb2, labels D_1f60[25]);
 * 0c0e skills and far table F_20a5_6234[k][D_d6fb[k]] (labels D_1f92[24]). */

struct gfx { char f00; char pad[0xb]; int f0c; };

extern unsigned long D_1fc2;
extern int D_0190;
extern char D_9085[];
extern struct gfx *D_0104;
extern struct gfx *D_0154, *D_017c;
extern int D_11c8;
extern int D_17b0;
extern char *D_1f2c[], *D_1f60[], *D_1f92[];
extern char D_9c55[][0x80], D_9cb2[][0x80], D_9cc0[][0x80];
extern char D_ca57[], D_d493[], D_ca64[], D_caa7[], D_d3fe[], D_a6d3[], D_d41b[], D_a6b4[];
extern unsigned char D_d6fb[];
extern unsigned char far F_20a5_6234[][0xfa];

unsigned _dos_allocmem(unsigned, unsigned *);
void far sub_root_047c_0952();
void far sub_root_0265_0882();
void far sub_root_0265_006e();
void far sub_root_0410_0302(int);
void far sub_root_047c_12c3(void);
void far sub_root_0000_1588(int);
void far sub_root_0000_1716(int);
void far sub_root_047c_1293(void);
void far sub_root_047c_059e(int);
int far sub_root_047c_053e(int);
int far sub_root_047c_0744(void);
void far sub_root_0000_13a0(void);
void far vec_1c2f(int);
void far sub_s014_14f7_0008(void);
void far sub_root_0265_09b2();
void far sub_s015_1ef8_0000(void);
void far sub_s015_1ef8_0046(void);
void far sub_s015_1ef8_00dc(void);
void far sub_s015_1ef8_01a8(int);
void far sub_s015_1ef8_01c6(int);
void far sub_s015_1ef8_0708(int);
void far sub_s015_1ef8_0c0e(int);

void far sub_s015_1ef8_0000(void)
{
    unsigned seg;
    unsigned long sz;

    _dos_allocmem(0x7530, &seg);
    sz = (unsigned long)seg << 4;
    if (sz < D_1fc2)
        D_1fc2 = sz;
}

void far sub_s015_1ef8_0046(void)
{
    sub_root_047c_0952(D_0190, 0, 0, 0x3c, 0x3c, 0);
    sub_root_0265_0882(D_1fc2, D_9085);
    sub_root_0265_006e(D_0190, 3, 3, D_9085);
    sub_root_0265_0882(0x9b080L - D_1fc2, D_9085);
    sub_root_0265_006e(D_0190, 3, 0xd, D_9085);
    sub_root_0410_0302(200);
    sub_root_047c_12c3();
    sub_root_0000_1588(0x100);
    sub_root_0000_1716(0x100);
    sub_root_047c_1293();
}

void far sub_s015_1ef8_00dc(void)
{
    int run;

    if (D_11c8 == 0) {
        sub_root_047c_059e(D_11c8);
        if (sub_root_047c_053e(0)) {
            D_11c8 ^= 1;
            D_0154->f00 ^= 1;
            D_017c->f00 ^= 1;
        }
    }
    run = 1;
    do {
        if (run == 1) {
            sub_s015_1ef8_01c6(0);
            run++;
        }
        switch (sub_root_047c_0744()) {
        case 0x11b:
            run = 0;
            break;
        case 0x231:
            sub_s015_1ef8_01c6(0);
            break;
        case 0x332:
            sub_s015_1ef8_01c6(3);
            break;
        case 0x433:
            sub_s015_1ef8_0708(0);
            break;
        case 0x534:
            sub_s015_1ef8_0708(3);
            break;
        case 0x635:
            sub_s015_1ef8_0c0e(0);
            break;
        case 0x736:
            sub_s015_1ef8_0c0e(3);
            break;
        case 0x1000:
            sub_root_0000_13a0();
            break;
        }
    } while (run);
    D_0104->f0c = 15;
    vec_1c2f(D_17b0);
    sub_s014_14f7_0008();
}

void far sub_s015_1ef8_01a8(int row)
{
    D_0104->f0c = (row & 1) == 1 ? 9 : 15;
}

void far sub_s015_1ef8_01c6(int first)
{
    int i, y, k, x, j;

    sub_root_047c_0952(D_0104, 0, 0, 0x140, 0xc8, 0);
    for (i = 0, y = 8; i < 26; i++) {
        sub_s015_1ef8_01a8(i + 1);
        sub_root_0265_006e(D_0104, 0, y, D_1f2c[i]);
        y += 7;
    }
    *(volatile int *)&j = 0;
    for (k = first, x = 0x3c; k < first + 10; k++) {
        sub_root_0265_09b2(k, D_9085);
        sub_s015_1ef8_01a8(0);
        sub_root_0265_006e(D_0104, x, 0, D_9085);
        sub_root_0265_09b2(*(int *)&D_9c55[k][0], D_9085);
        sub_s015_1ef8_01a8(7);
        sub_root_0265_006e(D_0104, x, 7, D_9085);
        sub_root_0265_09b2(*(int *)&D_9c55[k][2], D_9085);
        sub_s015_1ef8_01a8(0x0e);
        sub_root_0265_006e(D_0104, x, 0x0e, D_9085);
        sub_root_0265_09b2(*(int *)&D_9c55[k][4], D_9085);
        sub_s015_1ef8_01a8(0x15);
        sub_root_0265_006e(D_0104, x, 0x15, D_9085);
        sub_root_0265_09b2(*(int *)&D_9c55[k][6], D_9085);
        sub_s015_1ef8_01a8(0x1c);
        sub_root_0265_006e(D_0104, x, 0x1c, D_9085);
        sub_root_0265_09b2(*(int *)&D_9c55[k][8], D_9085);
        sub_s015_1ef8_01a8(0x23);
        sub_root_0265_006e(D_0104, x, 0x23, D_9085);
        sub_root_0265_09b2(*(int *)&D_9c55[k][0x0a], D_9085);
        sub_s015_1ef8_01a8(0x2a);
        sub_root_0265_006e(D_0104, x, 0x2a, D_9085);
        sub_root_0265_09b2(*(int *)&D_9c55[k][0x10], D_9085);
        sub_s015_1ef8_01a8(0x31);
        sub_root_0265_006e(D_0104, x, 0x31, D_9085);
        sub_root_0265_09b2(D_9c55[k][0x14], D_9085);
        sub_s015_1ef8_01a8(0x38);
        sub_root_0265_006e(D_0104, x, 0x38, D_9085);
        sub_root_0265_09b2(D_9c55[k][0x18], D_9085);
        sub_s015_1ef8_01a8(0x3f);
        sub_root_0265_006e(D_0104, x, 0x3f, D_9085);
        sub_root_0265_09b2(D_9c55[k][0x19], D_9085);
        sub_s015_1ef8_01a8(0x46);
        sub_root_0265_006e(D_0104, x, 0x46, D_9085);
        sub_root_0265_09b2(D_9c55[k][0x1a], D_9085);
        sub_s015_1ef8_01a8(0x4d);
        sub_root_0265_006e(D_0104, x, 0x4d, D_9085);
        sub_root_0265_09b2(D_9c55[k][0x1b], D_9085);
        sub_s015_1ef8_01a8(0x54);
        sub_root_0265_006e(D_0104, x, 0x54, D_9085);
        sub_root_0265_09b2(D_9c55[k][0x4a], D_9085);
        sub_s015_1ef8_01a8(0x5b);
        sub_root_0265_006e(D_0104, x, 0x5b, D_9085);
        sub_root_0265_09b2((unsigned char)D_9c55[k][0x1f], D_9085);
        sub_s015_1ef8_01a8(0x62);
        sub_root_0265_006e(D_0104, x, 0x62, D_9085);
        sub_root_0265_09b2(D_9c55[k][0x22], D_9085);
        sub_s015_1ef8_01a8(0x69);
        sub_root_0265_006e(D_0104, x, 0x69, D_9085);
        sub_root_0265_09b2(D_ca57[k], D_9085);
        sub_s015_1ef8_01a8(0x70);
        sub_root_0265_006e(D_0104, x, 0x70, D_9085);
        sub_root_0265_09b2(D_d493[k], D_9085);
        sub_s015_1ef8_01a8(0x77);
        sub_root_0265_006e(D_0104, x, 0x77, D_9085);
        sub_root_0265_09b2(D_ca64[k], D_9085);
        sub_s015_1ef8_01a8(0x7e);
        sub_root_0265_006e(D_0104, x, 0x7e, D_9085);
        sub_root_0265_09b2(D_caa7[k], D_9085);
        sub_s015_1ef8_01a8(0x85);
        sub_root_0265_006e(D_0104, x, 0x85, D_9085);
        sub_root_0265_09b2(D_d3fe[k], D_9085);
        sub_s015_1ef8_01a8(0x8c);
        sub_root_0265_006e(D_0104, x, 0x8c, D_9085);
        sub_root_0265_09b2((unsigned char)D_a6d3[k], D_9085);
        sub_s015_1ef8_01a8(0x93);
        sub_root_0265_006e(D_0104, x, 0x93, D_9085);
        sub_root_0265_09b2(D_9c55[k][0x57], D_9085);
        sub_s015_1ef8_01a8(0x9a);
        sub_root_0265_006e(D_0104, x, 0x9a, D_9085);
        sub_root_0265_09b2(D_9c55[k][0x56], D_9085);
        sub_s015_1ef8_01a8(0x0a1);
        sub_root_0265_006e(D_0104, x, 0x0a1, D_9085);
        sub_root_0265_09b2(D_d41b[k], D_9085);
        sub_s015_1ef8_01a8(0x0a8);
        sub_root_0265_006e(D_0104, x, 0x0a8, D_9085);
        sub_root_0265_09b2((unsigned char)D_9c55[k][0x20], D_9085);
        sub_s015_1ef8_01a8(0x0af);
        sub_root_0265_006e(D_0104, x, 0x0af, D_9085);
        sub_root_0265_09b2((unsigned char)D_9c55[k][0x21], D_9085);
        sub_s015_1ef8_01a8(0x0b6);
        sub_root_0265_006e(D_0104, x, 0x0b6, D_9085);
        x += 0x19;
    }
}

void far sub_s015_1ef8_0708(int first)
{
    int i, y, k, x, j;

    sub_root_047c_0952(D_0104, 0, 0, 0x140, 0xc8, 0);
    for (i = 0, y = 8; i < 25; i++) {
        sub_s015_1ef8_01a8(i + 1);
        sub_root_0265_006e(D_0104, 0, y, D_1f60[i]);
        y += 7;
    }
    *(volatile int *)&j = 0;
    for (k = first, x = 0x3c; k < first + 10; k++) {
        sub_root_0265_09b2(k, D_9085);
        sub_s015_1ef8_01a8(0);
        sub_root_0265_006e(D_0104, x, 0, D_9085);
        sub_root_0265_09b2(D_9cb2[k][0], D_9085);
        sub_s015_1ef8_01a8(7);
        sub_root_0265_006e(D_0104, x, 7, D_9085);
        sub_root_0265_09b2(D_9cb2[k][7], D_9085);
        sub_s015_1ef8_01a8(0x0e);
        sub_root_0265_006e(D_0104, x, 0x0e, D_9085);
        sub_root_0265_09b2(D_9cb2[k][1], D_9085);
        sub_s015_1ef8_01a8(0x15);
        sub_root_0265_006e(D_0104, x, 0x15, D_9085);
        sub_root_0265_09b2(D_9cb2[k][8], D_9085);
        sub_s015_1ef8_01a8(0x1c);
        sub_root_0265_006e(D_0104, x, 0x1c, D_9085);
        sub_root_0265_09b2(D_9cb2[k][2], D_9085);
        sub_s015_1ef8_01a8(0x23);
        sub_root_0265_006e(D_0104, x, 0x23, D_9085);
        sub_root_0265_09b2(D_9cb2[k][3], D_9085);
        sub_s015_1ef8_01a8(0x2a);
        sub_root_0265_006e(D_0104, x, 0x2a, D_9085);
        sub_root_0265_09b2(D_9cb2[k][4], D_9085);
        sub_s015_1ef8_01a8(0x31);
        sub_root_0265_006e(D_0104, x, 0x31, D_9085);
        sub_root_0265_09b2(D_9cb2[k][5], D_9085);
        sub_s015_1ef8_01a8(0x38);
        sub_root_0265_006e(D_0104, x, 0x38, D_9085);
        sub_root_0265_09b2(D_9cb2[k][6], D_9085);
        sub_s015_1ef8_01a8(0x3f);
        sub_root_0265_006e(D_0104, x, 0x3f, D_9085);
        sub_root_0265_09b2(D_a6b4[k], D_9085);
        sub_s015_1ef8_01a8(0x46);
        sub_root_0265_006e(D_0104, x, 0x46, D_9085);
        sub_root_0265_09b2(0, D_9085);
        sub_s015_1ef8_01a8(0x4d);
        sub_root_0265_006e(D_0104, x, 0x4d, D_9085);
        sub_root_0265_09b2(D_9cb2[k][0], D_9085);
        sub_s015_1ef8_01a8(0x54);
        sub_root_0265_006e(D_0104, x, 0x54, D_9085);
        sub_root_0265_09b2((unsigned char)D_9cb2[k][0], D_9085);
        sub_s015_1ef8_01a8(0x5b);
        sub_root_0265_006e(D_0104, x, 0x5b, D_9085);
        sub_root_0265_09b2((unsigned char)D_9cb2[k][0], D_9085);
        sub_s015_1ef8_01a8(0x62);
        sub_root_0265_006e(D_0104, x, 0x62, D_9085);
        sub_root_0265_09b2(D_9cb2[k][0], D_9085);
        sub_s015_1ef8_01a8(0x69);
        sub_root_0265_006e(D_0104, x, 0x69, D_9085);
        sub_root_0265_09b2(D_9cb2[k][0], D_9085);
        sub_s015_1ef8_01a8(0x70);
        sub_root_0265_006e(D_0104, x, 0x70, D_9085);
        sub_root_0265_09b2(D_9cb2[k][0], D_9085);
        sub_s015_1ef8_01a8(0x77);
        sub_root_0265_006e(D_0104, x, 0x77, D_9085);
        sub_root_0265_09b2(D_9cb2[k][0], D_9085);
        sub_s015_1ef8_01a8(0x7e);
        sub_root_0265_006e(D_0104, x, 0x7e, D_9085);
        sub_root_0265_09b2(D_9cb2[k][0], D_9085);
        sub_s015_1ef8_01a8(0x85);
        sub_root_0265_006e(D_0104, x, 0x85, D_9085);
        sub_root_0265_09b2(D_9cb2[k][0], D_9085);
        sub_s015_1ef8_01a8(0x8c);
        sub_root_0265_006e(D_0104, x, 0x8c, D_9085);
        sub_root_0265_09b2(D_9cb2[k][0], D_9085);
        sub_s015_1ef8_01a8(0x93);
        sub_root_0265_006e(D_0104, x, 0x93, D_9085);
        sub_root_0265_09b2(D_9cb2[k][0], D_9085);
        sub_s015_1ef8_01a8(0x9a);
        sub_root_0265_006e(D_0104, x, 0x9a, D_9085);
        sub_root_0265_09b2(D_9cb2[k][0], D_9085);
        sub_s015_1ef8_01a8(0x0a1);
        sub_root_0265_006e(D_0104, x, 0x0a1, D_9085);
        sub_root_0265_09b2(D_9cb2[k][0], D_9085);
        sub_s015_1ef8_01a8(0x0a8);
        sub_root_0265_006e(D_0104, x, 0x0a8, D_9085);
        sub_root_0265_09b2(D_9cb2[k][0], D_9085);
        sub_s015_1ef8_01a8(0x0af);
        sub_root_0265_006e(D_0104, x, 0x0af, D_9085);
        x += 0x19;
    }
}

void far sub_s015_1ef8_0c0e(int first)
{
    int i, y, k, x;

    sub_root_047c_0952(D_0104, 0, 0, 0x140, 0xc8, 0);
    for (i = 0, y = 8; i < 24; i++) {
        sub_s015_1ef8_01a8(i + 1);
        sub_root_0265_006e(D_0104, 0, y, D_1f92[i]);
        y += 7;
    }
    for (k = first, x = 0x3c; k < first + 10; k++) {
        sub_root_0265_09b2(k, D_9085);
        sub_s015_1ef8_01a8(0);
        sub_root_0265_006e(D_0104, x, 0, D_9085);
        sub_root_0265_09b2(D_9cc0[k][0], D_9085);
        sub_s015_1ef8_01a8(7);
        sub_root_0265_006e(D_0104, x, 7, D_9085);
        sub_root_0265_09b2(D_9cc0[k][1], D_9085);
        sub_s015_1ef8_01a8(0x0e);
        sub_root_0265_006e(D_0104, x, 0x0e, D_9085);
        sub_root_0265_09b2(D_9cc0[k][2], D_9085);
        sub_s015_1ef8_01a8(0x15);
        sub_root_0265_006e(D_0104, x, 0x15, D_9085);
        sub_root_0265_09b2(D_9cc0[k][3], D_9085);
        sub_s015_1ef8_01a8(0x1c);
        sub_root_0265_006e(D_0104, x, 0x1c, D_9085);
        sub_root_0265_09b2(D_9cc0[k][4], D_9085);
        sub_s015_1ef8_01a8(0x23);
        sub_root_0265_006e(D_0104, x, 0x23, D_9085);
        sub_root_0265_09b2(D_9cc0[k][5], D_9085);
        sub_s015_1ef8_01a8(0x2a);
        sub_root_0265_006e(D_0104, x, 0x2a, D_9085);
        sub_root_0265_09b2(D_9cc0[k][6], D_9085);
        sub_s015_1ef8_01a8(0x31);
        sub_root_0265_006e(D_0104, x, 0x31, D_9085);
        sub_root_0265_09b2(D_9cc0[k][0x11], D_9085);
        sub_s015_1ef8_01a8(0x38);
        sub_root_0265_006e(D_0104, x, 0x38, D_9085);
        sub_root_0265_09b2(D_9cc0[k][0x12], D_9085);
        sub_s015_1ef8_01a8(0x3f);
        sub_root_0265_006e(D_0104, x, 0x3f, D_9085);
        sub_root_0265_09b2(D_9cc0[k][0x10], D_9085);
        sub_s015_1ef8_01a8(0x46);
        sub_root_0265_006e(D_0104, x, 0x46, D_9085);
        sub_root_0265_09b2(D_9cc0[k][0x0d], D_9085);
        sub_s015_1ef8_01a8(0x4d);
        sub_root_0265_006e(D_0104, x, 0x4d, D_9085);
        sub_root_0265_09b2(D_9cc0[k][0x0f], D_9085);
        sub_s015_1ef8_01a8(0x54);
        sub_root_0265_006e(D_0104, x, 0x54, D_9085);
        sub_root_0265_09b2(D_9cc0[k][0x0e], D_9085);
        sub_s015_1ef8_01a8(0x5b);
        sub_root_0265_006e(D_0104, x, 0x5b, D_9085);
        sub_root_0265_09b2(D_9cc0[k][7], D_9085);
        sub_s015_1ef8_01a8(0x62);
        sub_root_0265_006e(D_0104, x, 0x62, D_9085);
        sub_root_0265_09b2(D_9cc0[k][9], D_9085);
        sub_s015_1ef8_01a8(0x69);
        sub_root_0265_006e(D_0104, x, 0x69, D_9085);
        sub_root_0265_09b2(D_9cc0[k][0x0b], D_9085);
        sub_s015_1ef8_01a8(0x70);
        sub_root_0265_006e(D_0104, x, 0x70, D_9085);
        sub_root_0265_09b2(D_9cc0[k][0x0a], D_9085);
        sub_s015_1ef8_01a8(0x77);
        sub_root_0265_006e(D_0104, x, 0x77, D_9085);
        sub_root_0265_09b2(D_9cc0[k][0x0c], D_9085);
        sub_s015_1ef8_01a8(0x7e);
        sub_root_0265_006e(D_0104, x, 0x7e, D_9085);
        sub_root_0265_09b2(F_20a5_6234[k][D_d6fb[k] + 0], D_9085);
        sub_s015_1ef8_01a8(0x85);
        sub_root_0265_006e(D_0104, x, 0x85, D_9085);
        sub_root_0265_09b2(F_20a5_6234[k][D_d6fb[k] + 1], D_9085);
        sub_s015_1ef8_01a8(0x8c);
        sub_root_0265_006e(D_0104, x, 0x8c, D_9085);
        sub_root_0265_09b2(F_20a5_6234[k][D_d6fb[k] + 2], D_9085);
        sub_s015_1ef8_01a8(0x93);
        sub_root_0265_006e(D_0104, x, 0x93, D_9085);
        sub_root_0265_09b2(F_20a5_6234[k][D_d6fb[k] + 3], D_9085);
        sub_s015_1ef8_01a8(0x9a);
        sub_root_0265_006e(D_0104, x, 0x9a, D_9085);
        sub_root_0265_09b2(F_20a5_6234[k][D_d6fb[k] + 4], D_9085);
        sub_s015_1ef8_01a8(0x0a1);
        sub_root_0265_006e(D_0104, x, 0x0a1, D_9085);
        sub_root_0265_09b2(F_20a5_6234[k][D_d6fb[k] + 5], D_9085);
        sub_s015_1ef8_01a8(0x0a8);
        sub_root_0265_006e(D_0104, x, 0x0a8, D_9085);
        x += 0x19;
    }
}

