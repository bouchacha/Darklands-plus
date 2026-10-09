/* place: s037 18c8:0004 */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* status: 4/15 match (047c 04b2 053e 3890). Character generator screen (pics\chargen.pic).
   Written, same length, only CONST slot order / strings left (they settle once the stubs
   0004 058e 10da 1738 20fa 22be and their literals are written): 39ca (saints list), 356c
   (career step; plus `w` in SI instead of BX), 38a6 (virtues list; plus i/n SI/DI swapped).
   372a (career events): 6.00 layout of the jump-table bodies (7/0x12 after 0x0c-group, 6-group
   after 0x1a, 4-group last; case 9 jumps BACK into case 8) - 6.00A sorts bodies by smallest
   label and keeps the later cross-jump copy; splitting groups fixed 4/6/7, not 8/9.
   New character being built: D_e81c age (unsigned), D_e86e[6] attributes, D_e875[0x13]
   skills, D_8c48/D_8bfa years of the last step; D_4428/D_442a/D_8c12 last three
   professions, D_4402[prof] bit 0 = profession taken. Profession tables (0x3a bytes:
   int years; char attr[6]; char skill[0x13]; ...) F_290e_2b65 (childhood), F_290e_2cc1;
   aging penalties F_290e_3521[][6]; virtues bits F_2e38_3b04[22] (names F_2e38_1736[][3]);
   saints known bit set F_2e38_3b1a[17] (names F_2e38_183e). */

struct res { int a; int b; };
extern unsigned char far F_2e38_3b04[];
extern unsigned char far F_2e38_3b1a[];
extern char far * far F_2e38_1736[][3];   /* virtue names, 3 per entry */
extern char far * far F_2e38_183e[];      /* saint names */
extern unsigned char D_00e8[];            /* 1 << n */
/* profession table entry (0x3a bytes): childhood F_290e_2b65, adult F_290e_2cc1 */
struct prof { int years; char attr[6]; char skill[0x13]; char _p1b[0x1f]; };
extern struct prof far F_290e_2b65[];
extern struct prof far F_290e_2cc1[];
extern char far F_290e_3521[][6];      /* aging attribute penalties */
extern unsigned D_e81c;                 /* age of the new character */
extern char D_e86e[6];                  /* attributes */
extern char D_e875[0x13];               /* skills */
extern int D_8c12, D_442a, D_4428, D_8c48, D_8bfa;
extern char D_4402[];
struct F_2e38_0000_2_t { char f_0000[2]; char f_0002[2]; char f_0004[2]; char f_0006[2]; char f_0008[2]; char f_000a[1]; };
extern struct F_2e38_0000_2_t far F_2e38_0000_2;
extern char D_27f4[];
extern int D_2800;
extern int D_2804;
extern int D_2812;
extern int D_2816;
extern char D_6ef0;
extern int D_8c14;
extern int D_8c16;
extern int D_8c18;
extern int D_8c1a;
extern int D_8c1c;
extern int D_8c20;
extern char D_e48c[];
extern int D_e7a0;
extern int D_e7a2;
extern char D_e888;
extern int D_e88c;
int far _fstrcpy();
int far sprintf();
int far strncpy();
int far time();
int far sub_root_0000_0c70();
int far sub_root_0265_0004();
int far sub_root_047c_04dc();
int far sub_root_047c_04f8();
int far sub_root_047c_0545();
int far sub_root_047c_0553();
int far sub_root_047c_055a();
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
int far sub_root_06a1_2824();
int far sub_s029_1367_0066();
int far sub_s029_1367_0db4();
void far sub_s037_18c8_0004();
int far sub_s037_18c8_047c();
int far sub_s037_18c8_04b2();
void far sub_s037_18c8_053e();
void far sub_s037_18c8_058e();
void far sub_s037_18c8_10da();
void far sub_s037_18c8_1738();
void far sub_s037_18c8_20fa();
void far sub_s037_18c8_22be();
void far sub_s037_18c8_356c();
void far sub_s037_18c8_372a();
int far sub_s037_18c8_3890();
void far sub_s037_18c8_38a6();
void far sub_s037_18c8_39ca();
void far sub_s037_18c8_3a9e();

/* sub_s037_18c8_0004: not decompiled: read of undefined bx */
void far sub_s037_18c8_0004(void) { }

int far sub_s037_18c8_047c(void)
{
    sub_root_05ba_00c2(D_e88c, 0x442c);
    D_2816 = D_e7a2;
    D_2804 = D_e7a0;
    do {
    } while (D_6ef0 & 1);
    sub_root_047c_1320(6, 0, 0, 0);
    return 0;
}

int far sub_s037_18c8_04b2(void)
{
    sub_root_0617_024a(1, "pics\\chargen.pic", D_e48c);
    sub_root_047c_0545();
    D_e88c = sub_root_0617_045c("pics\\buttona.pic");
    D_8c14 = sub_root_0617_045c("pics\\bubble01.pic");
    D_8c16 = sub_root_0617_045c("pics\\bubble02.pic");
    D_8c18 = sub_root_0617_045c("pics\\bubble03.pic");
    D_8c1a = sub_root_0617_045c("pics\\bubble04.pic");
    D_8c1c = sub_root_0617_045c("pics\\bubble05.pic");
    D_8c20 = sub_root_0617_045c("pics\\bublback.pic");
    sub_root_047c_04f8();
    D_2812 = 0;
    D_2816 = 1;
    return D_2804 = 1;
}

void far sub_s037_18c8_053e(void)
{
    sub_root_047c_0568();
    sub_root_047c_12c3();
    sub_root_047c_0553(0x2806, 0, 0, 0x140, 0xc8, D_27f4, 0, 0);
    sub_s037_18c8_10da(D_27f4);
    sub_s037_18c8_1738(D_27f4);
    sub_root_047c_058b(D_e48c);
    sub_root_047c_1293();
    sub_root_047c_056f();
}

/* sub_s037_18c8_058e: not decompiled: unhandled instruction rep movsw word ptr es:[di], word ptr [si] */
void far sub_s037_18c8_058e(void) { }

/* sub_s037_18c8_10da: not decompiled: memory via unknown si */
void far sub_s037_18c8_10da(void) { }

/* sub_s037_18c8_1738: not decompiled: al */
void far sub_s037_18c8_1738(void) { }

/* sub_s037_18c8_20fa: not decompiled: memory via unknown si */
void far sub_s037_18c8_20fa(void) { }

/* sub_s037_18c8_22be: not decompiled: branch without flags */
void far sub_s037_18c8_22be(void) { }

/* one career step for profession p: attribute/skill deltas, age +15 (childhood) or +5,
   aging penalties from age 30 on */
void far sub_s037_18c8_356c(int p)
{
    int w;
    int k;
    int i;
    for (i = 0; i < 6; i++) {
        if (D_e81c < 15)
            D_e86e[i] += F_290e_2b65[p].attr[i];
        else if (D_e81c <= 0x28 || i >= 2)
            D_e86e[i] += F_290e_2cc1[p].attr[i];
        D_e86e[i] = sub_s029_1367_0066(1, 0x63, D_e86e[i]);
    }
    for (i = 0; i < 0x13; i++) {
        if (D_e81c < 15)
            D_e875[i] += F_290e_2b65[p].skill[i];
        else if (D_e81c < 0x13)
            D_e875[i] += F_290e_2cc1[p].skill[i] + 2;
        else
            D_e875[i] += F_290e_2cc1[p].skill[i];
        D_e875[i] = sub_s029_1367_0066(0, 0x63, D_e875[i]);
    }
    if (D_e81c >= 15) {
        D_8c12 = D_442a;
        D_442a = D_4428;
        D_4428 = p;
        D_4402[p] |= 1;
        sub_s037_18c8_372a(p);
    }
    if (D_e81c >= 15)
        w = F_290e_2cc1[p].years;
    else
        w = F_290e_2b65[p].years;
    if (D_e81c == 15)
        w += 0x14;
    if (D_e81c == 0x14)
        w += 5;
    D_8c48 = w;
    D_e81c += D_e81c >= 15 ? 5 : 15;
    if (D_e81c >= 0x1e) {
        D_8bfa = w;
        k = sub_s029_1367_0066(0, 8, (int)(D_e81c - 0x1e) / 5);
        D_e86e[0] += F_290e_3521[k][0];
        D_e86e[1] += F_290e_3521[k][1];
        D_e86e[2] += F_290e_3521[k][2];
        D_e86e[3] += F_290e_3521[k][3];
        D_e86e[4] += F_290e_3521[k][4];
        D_e86e[5] += F_290e_3521[k][5];
    } else
        D_8bfa = w;
    for (i = 0; i < 6; i++)
        D_e86e[i] = sub_s029_1367_0066(1, 0x63, D_e86e[i]);
}

void far sub_s037_18c8_372a(int k)
{
    int a;
    int b;
    D_e888 = 0;
    switch (k) {
    case 0:
        sub_s037_18c8_3a9e(0x28);
        sub_s037_18c8_3a9e(0x2f);
        break;
    case 1:
        sub_s037_18c8_3a9e(0x2a);
        sub_s037_18c8_3a9e(0x30);
        break;
    case 2:
        sub_s037_18c8_3a9e(0x2b);
        sub_s037_18c8_3a9e(0x31);
        break;
    case 3:
        sub_s037_18c8_3a9e(0x2b);
        sub_s037_18c8_3a9e(0x30);
        break;
    case 5:
        sub_s037_18c8_3a9e(0x2b);
        sub_s037_18c8_3a9e(0x33);
        break;
    case 8:
    case 0xa:
    case 0xb:
    case 0xe:
        a = sub_root_0000_0c70(0x88);
        F_2e38_3b1a[a / 8] |= 1 << a % 8;
        break;
    case 9:
    case 0xf:
        b = sub_root_0000_0c70(0x88);
        F_2e38_3b1a[b / 8] |= 1 << b % 8;
        a = sub_root_0000_0c70(0x88);
        F_2e38_3b1a[a / 8] |= 1 << a % 8;
        break;
    case 0xc:
    case 0x18:
    case 0x19:
        F_2e38_3b04[sub_root_0000_0c70(0x16)]++;
        break;
    case 7:
        sub_s037_18c8_3a9e(0x2a);
        sub_s037_18c8_3a9e(0x2f);
        break;
    case 0x12:
        sub_s037_18c8_3a9e(0x2a);
        sub_s037_18c8_3a9e(0x2f);
        break;
    case 0x14:
        sub_s037_18c8_3a9e(0x28);
        sub_s037_18c8_3a9e(0x30);
        break;
    case 0x1a:
        F_2e38_3b04[sub_root_0000_0c70(0x16)]++;
        F_2e38_3b04[sub_root_0000_0c70(0x16)]++;
        break;
    case 6:
    case 0x10:
    case 0x11:
        sub_s037_18c8_3a9e(0x26);
        break;
    case 0x1b:
    case 0x1c:
    case 0x1d:
        sub_s037_18c8_3a9e(0x26);
        break;
    case 0x21:
        sub_s037_18c8_3a9e(0x26);
        sub_s037_18c8_3a9e(0x2f);
        break;
    case 4:
        sub_s037_18c8_3a9e(0x27);
        sub_s037_18c8_3a9e(0x2f);
        break;
    case 0x23:
    case 0x24:
        sub_s037_18c8_3a9e(0x27);
        sub_s037_18c8_3a9e(0x2f);
        break;
    }
}

int far sub_s037_18c8_3890(int x)
{
    if (x >= 14)
        x--;
    if (x >= 8)
        x--;
    return x;
}

/* list of the virtues (3 flags per entry of F_2e38_3b04) */
void far sub_s037_18c8_38a6(void)
{
    int i;
    int n;
    sub_root_047c_12c3();
    sub_root_047c_0545();
    i = sub_root_0617_045c("pics\\armbrs13.pic");
    sub_root_047c_04f8();
    sub_root_047c_05a5(D_27f4, 0x29, 0x31, i);
    if (i)
        sub_root_05ba_00c2(i, "x");
    D_2800 = 8;
    n = 0;
    for (i = 0; i < 22; i++) {
        if (F_2e38_3b04[i] & 1)
            sub_root_0265_0004(D_27f4, 0x32, (n++ + 8) << 3, F_2e38_1736[i][0]);
        if (F_2e38_3b04[i] & 2)
            sub_root_0265_0004(D_27f4, 0x32, (n++ + 8) << 3, F_2e38_1736[i][1]);
        if (F_2e38_3b04[i] & 4)
            sub_root_0265_0004(D_27f4, 0x32, (n++ + 8) << 3, F_2e38_1736[i][2]);
    }
    while (!sub_root_047c_073d() && !(D_6ef0 & 1))
        ;
    if (sub_root_047c_073d())
        sub_root_047c_0744();
    sub_s037_18c8_053e();
    sub_root_047c_1293();
}

/* list of the known saints (bit set F_2e38_3b1a, names F_2e38_183e) */
void far sub_s037_18c8_39ca(void)
{
    unsigned b;
    int i;
    int j;
    int n;
    D_2800 = 8;
    sub_root_047c_12c3();
    sub_root_047c_0545();
    b = sub_root_0617_045c("pics\\armbrs13.pic");
    sub_root_047c_04f8();
    sub_root_047c_05a5(D_27f4, 0x29, 0x41, b);
    if (b)
        sub_root_05ba_00c2(b, "x");
    i = 0;
    n = 0;
    for (; i < 0x11; i++)
        for (j = 0; j < 8; j++)
            if (F_2e38_3b1a[i] & D_00e8[j % 8])
                sub_root_0265_0004(D_27f4, 0x32, (n++ << 3) + 0x4e, F_2e38_183e[i * 8 + j]);
    while (!sub_root_047c_073d() && !(D_6ef0 & 1))
        ;
    if (sub_root_047c_073d())
        sub_root_047c_0744();
    sub_s037_18c8_053e();
    sub_root_047c_1293();
}

/* sub_s037_18c8_3a9e: not decompiled: es:[reg] unknown es=farslot(32450, 11832) ptr=None */
void far sub_s037_18c8_3a9e(void) { }
