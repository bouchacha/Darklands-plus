/* place: s037 18c8:0004 */
/* orig: size=3b4e CONST=7eae+0016 _DATA=442e+0370 */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* status: ALL 15 FUNCTIONS WRITTEN; _DATA matches. 7/15 match (0004 047c 04b2 053e 1738 3890 39ca).
   058e (key handler): same length; left are 6.00 code-placement choices in the key
   compare tree: which `return` sites get an inline epilogue vs `jmp` to the shared one
   (0eb6, 1032) and which copy of the cross-jumped 0x2247/0x3042 tail survives (1049..107c).
   0004 (main loop) MATCHES: #pragma optimize("a", on) (D_8c1e kept in BX) and no separate
   key copy - k itself is tested/passed after the mouse part (6.00A splits k: stack home,
   loaded into DI for the tail). 10da (left panel): 6.00 temp slot [bp-2] for the strength-reduced
   pointer init of the two list loops (`mov [bp-2],20abh; mov si,[bp-2]`). 20fa (reset):
   far memsets of F_2e38_3b1a/3b04 are `rep stosw` intrinsics in 6.00 (known 6.00A limit), and
   memset(D_4402, 0, 0x25) (odd size) stays `rep stosb` with DX. 22be (profession list,
   conditions extracted mechanically, see e4/NOTES): 22 bytes short - 6.00A threads
   `je L; jmp skip` into a short jump to a nearby `jmp skip`, and one cross-jumped tail (case
   31 into case 30) differs. 3a9e (add item): `id` in SI in 6.00, BX in 6.00A.
   356c (career step): `w` in SI instead of BX. 38a6 (virtues list): i/n SI/DI swapped.
   372a (career events): 6.00 layout of the jump-table bodies (7/0x12 after 0x0c-group, 6-group
   after 0x1a, 4-group last; case 9 jumps BACK into case 8) - 6.00A sorts bodies by smallest
   label and keeps the later cross-jump copy; splitting groups fixed 4/6/7, not 8/9.
   New character being built: D_e81c age (unsigned), D_e86e[6] attributes, D_e875[0x13]
   skills, D_8c48/D_8bfa years of the last step; D_4428/D_442a/D_8c12 last three
   professions, D_4402[prof] bit 0 = profession taken. Profession tables (0x3a bytes:
   int years; char attr[6]; char skill[0x13]; char maxsk[0x13]; ...) F_290e_2b65 (childhood),
   F_290e_2cc1; aging penalties F_290e_3521[][6]; virtues bits F_2e38_3b04[22] (names
   F_2e38_1736[][3]); saints known bit set F_2e38_3b1a[17] (names F_2e38_183e).
   Screen state D_8c1e: 0 name/gender, 1 childhood list, 2 attribute EPs, 3 profession list,
   4 skill EPs. List D_8bd2[] (D_8c00 rows), cursor D_43ee (mouse) / D_8bfc (attr/skill row),
   EPs spent this step D_8c06[6] (attributes) / D_8c22[0x13] (skills). */

struct res { int a; int b; };
/* screen / text window (D_27f4): text colour at +0c (D_2800) */
struct scr { char _p0[0xc]; int colour; };
extern char far * far F_290e_1df7[];   /* chargen texts: menu entries, list titles */
extern char far * far F_290e_20ab[6];  /* childhood backgrounds */
extern char far * far F_290e_1ee7[];   /* profession names */
/* one inventory entry (6 bytes) */
struct item { int id; char f2; char qual; char cnt; char f5; };
/* item definition (F_2e38_0890[id]) */
struct itemdef { char name[0x1e]; char f1e; char _p1f[6]; char f25; char f26; };
extern struct itemdef far * far F_2e38_0890[];
extern struct item far F_2e38_3984[0x40];  /* starting equipment of the new character */
extern unsigned char far F_2e38_3b04[];
extern unsigned char far F_2e38_3b1a[];
extern char far * far F_2e38_1736[][3];   /* virtue names, 3 per entry */
extern char far * far F_2e38_183e[];      /* saint names */
extern unsigned char D_00e8[];            /* 1 << n */
/* profession table entry (0x3a bytes): childhood F_290e_2b65, adult F_290e_2cc1 */
struct prof { int years; char attr[6]; char skill[0x13]; char maxsk[0x13]; char _p2e[0xc]; };
extern char far F_290e_3521[][6];      /* aging attribute penalties */
extern struct prof far F_290e_2b65[];
extern struct prof far F_290e_2cc1[];
extern unsigned D_e81c;                 /* age of the new character */
extern char D_e86e[6];                  /* attributes */
extern char D_e875[0x13];               /* skills */
extern int D_8c12, D_442a, D_4428, D_8bfa, D_8c48;
extern char D_4402[];
struct F_2e38_0000_2_t { char f_0000[2]; char f_0002[2]; char f_0004[2]; char f_0006[2]; char f_0008[2]; char f_000a[1]; };
extern struct F_2e38_0000_2_t far F_2e38_0000_2;
extern char D_27f4[];
extern int D_2800;
extern int D_2804;
extern int D_2812;
extern int D_2816;
extern volatile int D_6ef0, D_6eec, D_6eee;   /* mouse buttons, x, y */
extern int D_8c14[5];                   /* bubble animation pictures */
extern int D_8c16;
extern int D_8c18;
extern int D_8c1a;
extern int D_8c1c;
extern int D_8c20;
extern char D_e48c[];
extern int D_e7a0;
extern int D_e7a2;
extern char D_e888;                     /* entries in F_2e38_3984 */
extern int D_e88c;
extern int D_8c1e;                      /* screen state */
extern int D_43ee;                      /* highlighted list row */
extern int D_43f8;
extern char D_43f0[];
extern char D_2806[];
extern int D_8bfc;                      /* selected attribute/skill row */
extern int D_8c00;                      /* rows in the list */
extern char D_e82f[];                   /* name being edited */
extern char D_e848[];                   /* short name */
extern char D_e867[7];                  /* attributes at the start */
extern int D_e80e, D_e810, D_e818, D_8bfe, D_8c02;
extern char D_e81e, D_e81f, D_e820, D_e821, D_e822, D_e823, D_e824, D_e825, D_e826, D_e82c;
extern char D_e853, D_e854, D_e855, D_e856, D_e857, D_e858, D_e859, D_e85a, D_e85b;
extern char D_e862, D_e864, D_e865, D_e866;
extern int D_8bd2[0x14];
extern int D_8c22[0x13];
extern int D_8c06[6];
extern char far F_2e38_1a96[];
void *memset(void *, int, unsigned);
char *strcpy(char *, const char *);
void far *_fmemset(void far *, int, unsigned);
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
int far sub_root_0000_13a0();
int far sub_s037_1462_38aa();
int far sub_s037_1462_43e6();
int far sub_s029_1367_0066();
int far sub_s029_1367_0db4();
void far sub_s037_18c8_0004();
int far sub_s037_18c8_047c();
int far sub_s037_18c8_04b2();
void far sub_s037_18c8_053e();
int far sub_s037_18c8_058e();
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

/* character generator main loop. D_8c1e = screen state (0 name/gender, 1/3 choose
   profession (childhood/adult), 2 attributes with EPs, 4 skills with EPs); mouse over
   the list (rows of 11 pixels from y 0x2d) highlights row D_43ee, click = key row+1;
   attribute rows (state 2) / skill rows (state 4) of 8 pixels move D_8bfc; other click
   areas give key codes; every 0x5dd passes the bubble animation advances. */
#pragma optimize("a", on)
void far sub_s037_18c8_0004(void)
{
    int t;
    int f;
    int k;
    int row;
    int key;    /* unused: keeps the symbol table (operand order in 1738) */
    t = 0x5dd;
    f = 0;
    D_8c1e = 0;
    D_e7a2 = D_2816;
    D_e7a0 = D_2804;
    sub_s037_18c8_20fa();
    sub_s037_18c8_04b2();
    sub_s037_18c8_053e();
    while (D_6ef0 & 1)
        ;
    for (;;) {
        k = 0;
        if (t++ > 0x5dc) {
            t = 0;
            if (D_8c1e == 2 || D_8c1e == 4)
                D_43f8 = (D_8bfa * -100 / D_8c48 + 100) / 2;
            else
                D_43f8 = 0;
            if (D_6eec > 0xc3 && D_6eec < 0xda && D_6eee > 0x5f)
                sub_root_047c_12c3();
            sub_root_047c_05a5(D_2806, 0xd0, 0x69, D_8c14[f++]);
            sub_root_047c_04ef(D_43f0, 0, 0, D_8c20);
            sub_root_047c_0553(D_2806, 0xd0, 0x69, 0xc, 0x32, D_27f4, 0xd0, 0x69);
            if (f >= 5)
                f = 0;
            sub_root_047c_1293();
        }
        if (!(D_6ef0 & 1) && D_6eec > 0x40 && D_6eec < 0xae && D_6eee > 0x2d && D_6eee < 0xc7) {
            row = (D_6eee - 0x2d) / 0xb;
            if (D_43ee != row) {
                sub_root_047c_12c3();
                if (D_43ee != -1)
                    sub_root_047c_04dc(D_27f4, 0x41, D_43ee * 0xb + 0x2f, 0x6c, 0xb, 9, 0);
                D_43ee = row;
                sub_root_047c_04dc(D_27f4, 0x41, row * 0xb + 0x2f, 0x6c, 0xb, 0, 9);
                if ((D_8c1e == 1 || D_8c1e == 3) && row < D_8c00)
                    sub_s037_18c8_1738(D_27f4);
                sub_root_047c_1293();
            }
        } else if ((D_6ef0 & 1) && D_6eec > 0x40 && D_6eec < 0xae && D_6eee > 0x2d && D_6eee < 0xc7) {
            while (D_6ef0 & 1)
                ;
            k = (D_6eee - 0x2d) / 0xb + 1;
            if (D_8c1e == 4 && k == 1 && D_e81c > 0x41)
                k = 0;
        } else if ((D_6ef0 & 1) && D_8c1e == 2 && D_6eec > 0xc4 && D_6eec < 0xec && D_6eee > 0x12 && D_6eee < 0x4d) {
            while (D_6ef0 & 1)
                ;
            row = (D_6eee - 0x12) / 8;
            if (row >= 6)
                continue;
            sub_root_047c_12c3();
            if (D_8bfc == row)
                k = 0xd2b;
            sub_root_047c_04dc(D_27f4, 0xc5, D_8bfc * 8 + 0x14, 0x26, 8, 10, 0xbf);
            D_8bfc = row;
            sub_root_047c_04dc(D_27f4, 0xc5, row * 8 + 0x14, 0x26, 8, 0xbf, 10);
            sub_root_047c_1293();
        } else if ((D_6ef0 & 1) && D_8c1e == 4 && D_6eec > 0xf5 && D_6eec < 0x13b && D_6eee > 0x12 && D_6eee < 0xb9) {
            while (D_6ef0 & 1)
                ;
            row = (D_6eee - 0x12) / 8;
            if (row == 0xe || row == 7)
                continue;
            sub_root_047c_12c3();
            if (D_8bfc == row)
                k = 0xd2b;
            sub_root_047c_04dc(D_27f4, 0xf6, D_8bfc * 8 + 0x14, 0x44, 8, 10, 0xbf);
            D_8bfc = row;
            sub_root_047c_04dc(D_27f4, 0xf6, row * 8 + 0x14, 0x44, 8, 0xbf, 10);
            sub_root_047c_1293();
        } else if ((D_6ef0 & 1) && (D_8c1e == 4 || D_8c1e == 2) && D_6eec > 0xc3 && D_6eec < 0xeb && D_6eee > 0x5f && D_6eee < 0xa5) {
            while (D_6ef0 & 1)
                ;
            k = 0xc2d;
        } else if ((D_6ef0 & 1) && D_8c1e >= 3 && D_6eec > 0xae && D_6eec < 0xea && D_6eee > 0xa8 && D_6eee < 0xb4) {
            k = 0x5032;
        } else if ((D_6ef0 & 1) && D_8c1e >= 3 && D_6eec > 0xae && D_6eec < 0xea && D_6eee > 0xb8 && D_6eee < 0xc4) {
            k = 0x5133;
        }
        if (sub_root_047c_073d())
            k = sub_root_047c_0744();
        if (k == 0x11b) {
            if (D_8c1e == 2)
                D_8c1e = 1;
            else if (D_8c1e == 4)
                D_8c1e = 3;
            else
                break;
            sub_s037_18c8_053e();
        }
        if (k && sub_s037_18c8_058e(k) == 0x11b)
            break;
    }
    sub_s037_18c8_047c();
}

#pragma optimize("", on)

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
    D_8c14[0] = sub_root_0617_045c("pics\\bubble01.pic");
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

/* unused: their position in the symbol table fixes operand order in 058e */
extern int D_ff00;
extern int D_ff02;
extern int D_ff04;
extern int D_ff06;
extern int D_ff08;
extern int D_ff0a;
/* key / click handler of the character generator (key = scan<<8|ascii, or 1..15 = list
   row clicked, or one of the click-area codes of sub_0004). Returns 0x11b to leave the
   generator (start the game / quit), otherwise garbage. */
int far sub_s037_18c8_058e(register int key)
{
    int cba;
    int padn;
    char nameba[20] = "Character Name:";
    char nickoa[20] = "Character Nickname:";
    char bufe[80];
    int jg;
    int ig;
    /* state 4 (adult): B = begin adventure, K = restart, N = next profession list */
    switch (key) {
    case 0x314e:
    case 0x316e:
        if (D_8c1e == 4)
            key = 1;
        break;
    case 0x3042:
    case 0x3062:
        if (D_8c1e == 4)
            key = 2;
        break;
    case 0x254b:
    case 0x256b:
        if (D_8c1e == 4)
            key = 3;
        break;
    }
    switch (key) {
    case 0x4800:
    case 0x4b00:
    case 0x4d00:
        if (D_8c1e == 2) {
            sub_root_047c_04dc(D_27f4, 0xc6, D_8bfc * 8 + 0x14, 0x25, 8, 10, 0xbf);
            if (--D_8bfc < 0 || D_8bfc >= 6)
                D_8bfc = 5;
            sub_root_047c_04dc(D_27f4, 0xc6, D_8bfc * 8 + 0x14, 0x25, 8, 0xbf, 10);
        } else if (D_8c1e == 4) {
            sub_root_047c_04dc(D_27f4, 0xf7, D_8bfc * 8 + 0x14, 0x43, 8, 10, 0xbf);
            D_8bfc--;
            if (D_8bfc == 7)
                D_8bfc = 6;
            if (D_8bfc == 0xe)
                D_8bfc = 0xd;
            if (D_8bfc < 0)
                D_8bfc = 0x14;
            sub_s037_18c8_1738(D_27f4);
            sub_root_047c_04dc(D_27f4, 0xf7, D_8bfc * 8 + 0x14, 0x43, 8, 0xbf, 10);
        }
        break;
    case 0x5000:
        if (D_8c1e == 2) {
            sub_root_047c_04dc(D_27f4, 0xc6, D_8bfc * 8 + 0x14, 0x25, 8, 10, 0xbf);
            if (++D_8bfc >= 6)
                D_8bfc = 0;
            sub_root_047c_04dc(D_27f4, 0xc6, D_8bfc * 8 + 0x14, 0x25, 8, 0xbf, 10);
        } else if (D_8c1e == 4) {
            sub_root_047c_04dc(D_27f4, 0xf7, D_8bfc * 8 + 0x14, 0x43, 8, 10, 0xbf);
            D_8bfc++;
            if (D_8bfc == 7)
                D_8bfc = 8;
            if (D_8bfc == 0xe)
                D_8bfc = 0xf;
            if (D_8bfc >= 0x15)
                D_8bfc = 0;
            sub_root_047c_04dc(D_27f4, 0xf7, D_8bfc * 8 + 0x14, 0x43, 8, 0xbf, 10);
        }
        break;
    case 0xd2b:
    case 0xd3d:
    case 0x4e2b:
        /* '+': spend EPs (cost 1/2/3 by the current value) */
        if (D_8c1e == 2) {
            if (D_e86e[D_8bfc] >= 0x27)
                cba = 3;
            else if (D_e86e[D_8bfc] >= 0x1d)
                cba = 2;
            else
                cba = 1;
        } else if (D_8c1e == 4) {
            if (D_e875[sub_s037_18c8_3890(D_8bfc)] >= 0x4f)
                cba = 3;
            else if (D_e875[sub_s037_18c8_3890(D_8bfc)] >= 0x31)
                cba = 2;
            else
                cba = 1;
        }
        if (D_8c1e == 2 && D_8bfa - cba >= 0 && D_8c06[D_8bfc] < 0x28 && D_e86e[D_8bfc] < 0x28) {
            D_8c06[D_8bfc]++;
            D_e86e[D_8bfc]++;
            D_8bfa -= cba;
            sub_s037_18c8_1738(D_27f4);
        } else if (D_8c1e == 4 && D_8bfa - cba >= 0
                   && F_290e_2cc1[D_4428].maxsk[sub_s037_18c8_3890(D_8bfc)] > D_8c22[sub_s037_18c8_3890(D_8bfc)]
                   && D_e875[sub_s037_18c8_3890(D_8bfc)] < 0x59) {
            D_8c22[sub_s037_18c8_3890(D_8bfc)]++;
            D_e875[sub_s037_18c8_3890(D_8bfc)]++;
            D_8bfa -= cba;
            sub_s037_18c8_1738(D_27f4);
        }
        break;
    case 0xc2d:
    case 0x4a2d:
        /* '-': take back EPs spent in this step */
        if (D_8c1e == 2 && D_8bfa < D_8c48 && D_8c06[D_8bfc]) {
            if (D_e86e[D_8bfc] >= 0x28)
                D_8bfa += 3;
            else if (D_e86e[D_8bfc] >= 0x1e)
                D_8bfa += 2;
            else
                D_8bfa++;
            D_8c06[D_8bfc]--;
            D_e86e[D_8bfc]--;
            sub_s037_18c8_1738(D_27f4);
        } else if (D_8c1e == 4 && D_8bfa < D_8c48 && D_8c22[sub_s037_18c8_3890(D_8bfc)]) {
            if (D_e875[sub_s037_18c8_3890(D_8bfc)] >= 0x50)
                D_8bfa += 3;
            else if (D_e875[sub_s037_18c8_3890(D_8bfc)] >= 0x32)
                D_8bfa += 2;
            else
                D_8bfa++;
            D_8c22[sub_s037_18c8_3890(D_8bfc)]--;
            D_e875[sub_s037_18c8_3890(D_8bfc)]--;
            sub_s037_18c8_1738(D_27f4);
        }
        break;
    case 0x332:
    case 0x5032:
        sub_s037_18c8_38a6();
        break;
    case 0x433:
    case 0x5133:
        sub_s037_18c8_39ca();
        break;
    case 1:
    case 0x2247:
    case 0x2267:
    case 0x3042:
    case 0x3062:
        if (D_8c1e == 0) {
            D_8c1e = 1;
            sub_s037_18c8_053e();
        } else if (D_8c1e == 1 || D_8c1e == 3) {
            if (D_8bd2[key - 1] == -1)
                break;
            if (D_e81c < 15) {
                D_8c02 = 0;
                D_8c1e = 2;
            } else
                D_8c1e = 4;
            sub_s037_18c8_356c(D_8bd2[0]);
            sub_s037_18c8_053e();
        } else if (D_8c1e == 2) {
            D_8c1e = 3;
            sub_s037_18c8_053e();
        } else if (D_8c1e == 4) {
            for (ig = 0; ig < 0x13; ig++)
                D_8c22[ig] = 0;
            for (ig = 0; ig < 6; ig++)
                D_8c06[ig] = 0;
            D_8c1e = 3;
            sub_s037_18c8_053e();
        }
        break;
    case 2:
    case 0x324d:
    case 0x326d:
        if (D_8c1e == 0) {
            /* gender */
            if ((D_e821 = !D_e821) == 0) {
                D_e86e[1] = 0xf;
                D_e86e[0] = 0xc;
                D_e86e[2] = 0xc;
                D_e86e[3] = 0xc;
                D_e86e[4] = 0xc;
                D_e86e[5] = 0xb;
            } else {
                D_e86e[0] = 0xe;
                D_e86e[1] = 0xc;
                D_e86e[2] = 0xc;
                D_e86e[3] = 0xc;
                D_e86e[4] = 0xc;
                D_e86e[5] = 0xc;
            }
            if (D_8bfe) {
                D_8bfe = 1;
                sub_s029_1367_0db4(time(0), D_e821, 0);
                _fstrcpy((char far *)D_e82f, F_2e38_1a96);
                strncpy(D_e848, D_e82f, sub_s029_1367_0066(0, 9, sub_root_06a1_2824(D_e82f, " ")));
                ig = sub_s029_1367_0066(0, 9, sub_root_06a1_2824(D_e82f, " "));
                D_e848[ig] = 0;
                sub_root_047c_12c3();
                sub_root_047c_0553(D_2806, 0x40, 1, 0xb4, 0xc, D_27f4, 0x40, 1);
                D_2800 = 0xbf;
                sub_root_047c_055a(D_27f4, 0x47, 5, D_e848);
                sub_root_047c_055a(D_27f4, 0x83, 5, D_e82f);
                D_2800 = 0;
                sub_root_047c_055a(D_27f4, 0x47, 4, D_e848);
                sub_root_047c_055a(D_27f4, 0x83, 4, D_e82f);
                sub_root_047c_1293();
            }
            sub_s037_18c8_10da(D_27f4);
            sub_s037_18c8_1738(D_27f4);
        } else if (D_8c1e == 1 || D_8c1e == 3) {
            if (D_8bd2[key - 1] == -1)
                break;
            if (D_e81c < 15) {
                D_8c02 = 1;
                D_8c1e = 2;
            } else
                D_8c1e = 4;
            sub_s037_18c8_356c(D_8bd2[1]);
            sub_s037_18c8_053e();
        } else if (D_8c1e == 4) {
            /* begin the adventure: a weapon for the best weapon skill, items for virtues */
            for (ig = 0, jg = 0; ig < 7; ig++) {
                D_e867[ig] = D_e86e[ig];
                if (D_e875[ig] > D_e875[jg])
                    jg = ig;
            }
            switch (jg) {
            case 0:
                sub_s037_18c8_3a9e(5);
                break;
            case 1:
                sub_s037_18c8_3a9e(0xf);
                break;
            case 2:
                sub_s037_18c8_3a9e(0x11);
                break;
            case 3:
                sub_s037_18c8_3a9e(0x15);
                break;
            case 4:
                sub_s037_18c8_3a9e(1);
                break;
            case 5:
                sub_s037_18c8_3a9e(0x1c);
                break;
            case 6:
                sub_s037_18c8_3a9e(0x1e);
                break;
            }
            for (ig = 0; ig < 0x16; ig++)
                if (F_2e38_3b04[ig])
                    sub_s037_18c8_3a9e(ig + 0x5f);
            D_e81e = 0;
            sub_s037_1462_38aa();
            return 0x11b;
        }
        break;
    case 3:
    case 0x1f53:
    case 0x1f73:
        if (D_8c1e == 0) {
            /* random nameba */
            D_8bfe = 1;
            sub_s029_1367_0db4(time(0), D_e821, 0);
            _fstrcpy((char far *)D_e82f, F_2e38_1a96);
            strncpy(D_e848, D_e82f, sub_s029_1367_0066(0, 9, sub_root_06a1_2824(D_e82f, " ")));
            ig = sub_s029_1367_0066(0, 9, sub_root_06a1_2824(D_e82f, " "));
            D_e848[ig] = 0;
            sub_root_047c_12c3();
            sub_root_047c_0553(D_2806, 0x40, 1, 0xb4, 0xc, D_27f4, 0x40, 1);
            D_2800 = 0xbf;
            sub_root_047c_055a(D_27f4, 0x47, 5, D_e848);
            sub_root_047c_055a(D_27f4, 0x83, 5, D_e82f);
            D_2800 = 0;
            sub_root_047c_055a(D_27f4, 0x47, 4, D_e848);
            sub_root_047c_055a(D_27f4, 0x83, 4, D_e82f);
            sub_root_047c_1293();
        } else if (D_8c1e == 1 || D_8c1e == 3) {
            if (D_8bd2[key - 1] == -1)
                break;
            if (D_e81c < 15) {
                D_8c02 = 2;
                D_8c1e = 2;
            } else
                D_8c1e = 4;
            sub_s037_18c8_356c(D_8bd2[2]);
            sub_s037_18c8_053e();
        } else if (D_8c1e == 4) {
            /* restart */
            sub_s037_18c8_20fa();
            D_8c1e = 0;
            sub_s037_18c8_053e();
        }
        break;
    case 4:
    case 0x1245:
    case 0x1265:
        if (D_8c1e == 0) {
            /* enter the nameba */
            D_8bfe = 0;
            bufe[0] = 0;
            sub_s037_1462_43e6((char far *)bufe, (char far *)nameba, 0x17);
            strcpy(D_e82f, bufe);
            sub_root_047c_12c3();
            sub_root_047c_0553(D_2806, 0x40, 1, 0xb4, 0xc, D_27f4, 0x40, 1);
            D_2800 = 0xbf;
            sub_root_047c_055a(D_27f4, 0x47, 5, D_e848);
            sub_root_047c_055a(D_27f4, 0x83, 5, D_e82f);
            D_2800 = 0;
            sub_root_047c_055a(D_27f4, 0x47, 4, D_e848);
            sub_root_047c_055a(D_27f4, 0x83, 4, D_e82f);
            sub_root_047c_1293();
        } else if (D_8c1e == 1 || D_8c1e == 3) {
            if (D_8bd2[key - 1] == -1)
                break;
            if (D_e81c < 15) {
                D_8c02 = 3;
                D_8c1e = 2;
            } else
                D_8c1e = 4;
            sub_s037_18c8_356c(D_8bd2[3]);
            sub_s037_18c8_053e();
        }
        break;
    case 5:
    case 0x2e43:
    case 0x2e63:
        if (D_8c1e == 0) {
            /* enter the nickname */
            D_8bfe = 0;
            bufe[0] = 0;
            sub_s037_1462_43e6((char far *)bufe, (char far *)nickoa, 9);
            strcpy(D_e848, bufe);
            sub_root_047c_12c3();
            sub_root_047c_0553(D_2806, 0x40, 1, 0xb4, 0xc, D_27f4, 0x40, 1);
            D_2800 = 0xbf;
            sub_root_047c_055a(D_27f4, 0x47, 5, D_e848);
            sub_root_047c_055a(D_27f4, 0x83, 5, D_e82f);
            D_2800 = 0;
            sub_root_047c_055a(D_27f4, 0x47, 4, D_e848);
            sub_root_047c_055a(D_27f4, 0x83, 4, D_e82f);
            sub_root_047c_1293();
        } else if (D_8c1e == 1 || D_8c1e == 3) {
            if (D_8bd2[key - 1] == -1)
                break;
            if (D_e81c < 15) {
                D_8c02 = 4;
                D_8c1e = 2;
            } else
                D_8c1e = 4;
            sub_s037_18c8_356c(D_8bd2[4]);
            sub_s037_18c8_053e();
        }
        break;
    case 6:
    case 0x1352:
    case 0x1372:
        if (D_8c1e == 0)
            return 0x11b;
        if (D_8c1e == 1 || D_8c1e == 3) {
            if (D_8bd2[key - 1] == -1)
                break;
            if (D_e81c < 15) {
                D_8c02 = 5;
                D_8c1e = 2;
            } else
                D_8c1e = 4;
            sub_s037_18c8_356c(D_8bd2[5]);
            sub_s037_18c8_053e();
        }
        break;
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
        /* adult professions after the sixth */
        if (D_8c1e == 3) {
            if (D_8bd2[key - 1] == -1)
                break;
            sub_s037_18c8_356c(D_8bd2[key - 1]);
            D_8c1e = 4;
            sub_s037_18c8_053e();
        }
        break;
    case 0x1000:
        sub_s037_18c8_047c();
        sub_root_0000_13a0();
        break;
    case 0x231:
    case 0x534:
    case 0x635:
    case 0x4b34:
    case 0x4c35:
    case 0x4f31:
        break;
    }
}

/* left panel: name, help text and the list of the current state (D_8c00 rows):
   0 = menu (F_290e_1df7[0..6], [1]/[2] by gender), 1 = childhood backgrounds
   (F_290e_20ab), 2 = attributes, 3 = professions D_8bd2[] (names F_290e_1ee7), 4 = skills */
void far sub_s037_18c8_10da(struct scr *p)
{
    int i;
    int t;
    int y;
    sub_root_047c_12c3();
    p->colour = 0;
    if (D_8c1e == 0) {
        p->colour = 0xbf;
        sub_root_047c_055a(p, 0x47, 5, D_e848);
        sub_root_047c_055a(p, 0x83, 5, D_e82f);
        sub_root_047c_055a(p, 0x47, 0x13, "Select character name");
        sub_root_047c_055a(p, 0x47, 0x1b, "and gender, then start");
        sub_root_047c_055a(p, 0x47, 0x23, "childhood.");
        p->colour = 0;
        sub_root_047c_055a(p, 0x47, 4, D_e848);
        sub_root_047c_055a(p, 0x83, 4, D_e82f);
        sub_root_047c_055a(p, 0x47, 0x12, "Select character name");
        sub_root_047c_055a(p, 0x47, 0x1a, "and gender, then start");
        sub_root_047c_055a(p, 0x47, 0x22, "childhood.");
        p->colour = 0;
        sub_root_047c_05a5(p, 0x40, 0x38, D_e88c);
        if (D_e821 == 0)
            sub_root_0265_0004(p, 0x44, 0x3a, F_290e_1df7[1]);
        else
            sub_root_0265_0004(p, 0x44, 0x3a, F_290e_1df7[2]);
        sub_root_047c_05a5(p, 0x40, 0x2d, D_e88c);
        sub_root_0265_0004(p, 0x44, 0x2f, F_290e_1df7[0]);
        sub_root_047c_05a5(p, 0x40, 0x43, D_e88c);
        sub_root_0265_0004(p, 0x44, 0x45, F_290e_1df7[3]);
        sub_root_047c_05a5(p, 0x40, 0x4e, D_e88c);
        sub_root_0265_0004(p, 0x44, 0x50, F_290e_1df7[4]);
        sub_root_047c_05a5(p, 0x40, 0x59, D_e88c);
        sub_root_0265_0004(p, 0x44, 0x5b, F_290e_1df7[5]);
        sub_root_047c_05a5(p, 0x40, 0x64, D_e88c);
        sub_root_0265_0004(p, 0x44, 0x66, F_290e_1df7[6]);
        D_8c00 = 6;
    } else if ((t = D_8c1e) == 1) {
        p->colour = 0xbf;
        sub_root_047c_055a(p, 0x47, 5, D_e848);
        sub_root_047c_055a(p, 0x83, 5, D_e82f);
        sub_root_0265_0004(p, 0x47, 0x13, F_290e_1df7[7]);
        p->colour = 0;
        sub_root_047c_055a(p, 0x47, 4, D_e848);
        sub_root_047c_055a(p, 0x83, 4, D_e82f);
        sub_root_0265_0004(p, 0x47, 0x12, F_290e_1df7[7]);
        p->colour = 0;
        for (i = 0, y = 0x2d; i < 6; i++, y += 0xb) {
            sub_root_047c_05a5(p, 0x40, y, D_e88c);
            p->colour = 0;
            sub_root_0265_0004(p, 0x44, y + 2, F_290e_20ab[i]);
        }
        sub_root_047c_04dc(p, 0x44, D_8bfc * 0xb + 0x2f, 0x6c, 0xb, 0, 9);
        D_8c00 = 6;
    } else if (t == 2) {
        p->colour = 0xbf;
        sub_root_047c_055a(p, 0x47, 5, D_e848);
        sub_root_047c_055a(p, 0x83, 5, D_e82f);
        sub_root_047c_055a(p, 0x47, 0x13, "Use EPs to improve attr");
        sub_root_047c_055a(p, 0x47, 0x1b, "Select attribute and");
        sub_root_047c_055a(p, 0x47, 0x23, "use + or - to change");
        p->colour = 0;
        sub_root_047c_055a(p, 0x47, 4, D_e848);
        sub_root_047c_055a(p, 0x83, 4, D_e82f);
        sub_root_047c_055a(p, 0x47, 0x12, "Use EPs to improve attr");
        sub_root_047c_055a(p, 0x47, 0x1a, "Select attribute and");
        sub_root_047c_055a(p, 0x47, 0x22, "use + or - to change");
        sub_root_047c_05a5(p, 0x40, 0x2d, D_e88c);
        p->colour = 0;
        sub_root_0265_0004(p, 0x44, 0x2f, F_290e_1df7[8]);
        D_8c00 = 1;
    } else if (t == 3) {
        p->colour = 0xbf;
        sub_root_047c_055a(p, 0x47, 5, D_e848);
        sub_root_047c_055a(p, 0x83, 5, D_e82f);
        sub_root_0265_0004(p, 0x47, 0x13, F_290e_1df7[10]);
        p->colour = 0;
        sub_root_047c_055a(p, 0x47, 4, D_e848);
        sub_root_047c_055a(p, 0x83, 4, D_e82f);
        sub_root_0265_0004(p, 0x47, 0x12, F_290e_1df7[10]);
        sub_s037_18c8_22be();
        p->colour = 0;
        for (i = 0, y = 0x2d; i < D_8c00; i++, y += 0xb) {
            if (i >= 14)
                break;
            sub_root_047c_05a5(p, 0x40, y, D_e88c);
            p->colour = 0;
            sub_root_0265_0004(p, 0x44, y + 2, F_290e_1ee7[D_8bd2[i]]);
        }
        sub_root_047c_04dc(p, 0x44, D_8bfc * 0xb + 0x2f, 0x6c, 0xb, 0, 9);
    } else if (t == 4) {
        p->colour = 0xbf;
        sub_root_047c_055a(p, 0x47, 5, D_e848);
        sub_root_047c_055a(p, 0x83, 5, D_e82f);
        sub_root_047c_055a(p, 0x47, 0x13, "Use EPs to improve skills");
        sub_root_047c_055a(p, 0x47, 0x1b, "Select skill and use + or");
        sub_root_047c_055a(p, 0x47, 0x23, "- to change");
        p->colour = 0;
        sub_root_047c_055a(p, 0x47, 4, D_e848);
        sub_root_047c_055a(p, 0x83, 4, D_e82f);
        sub_root_047c_055a(p, 0x47, 0x12, "Use EPs to improve skills");
        sub_root_047c_055a(p, 0x47, 0x1a, "Select skill and use + or");
        sub_root_047c_055a(p, 0x47, 0x22, "- to change");
        sub_root_047c_05a5(p, 0x40, 0x2d, D_e88c);
        p->colour = 0;
        sub_root_0265_0004(p, 0x44, 0x2f, F_290e_1df7[11]);
        sub_root_047c_05a5(p, 0x40, 0x38, D_e88c);
        sub_root_0265_0004(p, 0x44, 0x3a, F_290e_1df7[12]);
        sub_root_047c_05a5(p, 0x40, 0x43, D_e88c);
        sub_root_0265_0004(p, 0x44, 0x45, F_290e_1df7[13]);
        D_8c00 = 3;
    }
    sub_root_047c_1293();
}

/* unused: their position in the symbol table fixes operand order in 1738 */
extern int D_fe00;
extern int D_fe02;
extern int D_fe04;
extern int D_fe06;
extern int D_fe08;
extern int D_fe0a;
/* right panel: EPs / years of the profession under the cursor, age, attributes (with the
   change the profession would bring) and the 19 skills ("now:max" while choosing) */
void far sub_s037_18c8_1738(struct scr *p)
{
    int prn;
    int ih;
    int vl;
    int agk;
    char bufg[40];
    sub_root_047c_12c3();
    p->colour = 0xbf;
    if ((prn = (D_8c1e == 1 || D_8c1e == 3) ? D_8bd2[D_43ee] : D_8c1e == 2 ? D_8c02 : D_4428) == -1)
        prn = 0;
    sub_root_047c_0553(D_2806, 0xc5, 0x57, 0x14, 9, D_27f4, 0xc5, 0x57);
    sub_root_047c_055a(p, 0xd8, 0x57, "EP");
    if (D_8c1e == 1) {
        sprintf(bufg, "%02d", F_290e_2b65[prn].years);
        sub_root_047c_055a(p, 0xc6, 0x57, bufg);
    } else if (D_8c1e == 3 && D_e81c >= 0x19) {
        sprintf(bufg, "%02d", F_290e_2cc1[prn].years);
        sub_root_047c_055a(p, 0xc6, 0x57, bufg);
    } else if (D_8c1e == 3 && D_e81c >= 0x14) {
        sprintf(bufg, "%02d", F_290e_2cc1[prn].years + 5);
        sub_root_047c_055a(p, 0xc6, 0x57, bufg);
    } else if (D_8c1e == 3) {
        sprintf(bufg, "%02d", F_290e_2cc1[prn].years + 0x14);
        sub_root_047c_055a(p, 0xc6, 0x57, bufg);
    } else if (D_8c1e != 0) {
        sprintf(bufg, "%02d", D_8bfa);
        sub_root_047c_055a(p, 0xc6, 0x57, bufg);
    }
    sub_root_047c_0553(D_2806, 0x10a, 4, 0x23, 9, D_27f4, 0x10a, 4);
    sprintf(bufg, "%02d", D_e81c);
    p->colour = 0xbf;
    sub_root_047c_055a(p, 0x11e, 5, "Age");
    sub_root_047c_055a(p, 0x10c, 5, bufg);
    p->colour = 0;
    sub_root_047c_055a(p, 0x11e, 4, "Age");
    sub_root_047c_055a(p, 0x10c, 4, bufg);
    p->colour = 0xbf;
    /* attributes */
    sub_root_047c_0553(D_2806, 0xc5, 0xf, 0x24, 0x3c, D_27f4, 0xc5, 0xf);
    sub_root_047c_055a(p, 0xd8, 0x14, "End");
    sub_root_047c_055a(p, 0xd8, 0x1c, "Str");
    sub_root_047c_055a(p, 0xd8, 0x24, "Agl");
    sub_root_047c_055a(p, 0xd8, 0x2c, "Per");
    sub_root_047c_055a(p, 0xd8, 0x34, "Int");
    sub_root_047c_055a(p, 0xd8, 0x3c, "Chr");
    sub_root_047c_055a(p, 0xd8, 0x44, "DF ");
    for (ih = 0; ih < 6; ih++) {
        if ((agk = (int)(D_e81c - 0x19) / 5) < 0)
            agk = 0;
        if (D_8c1e == 2 || D_8c1e == 4)
            vl = D_e86e[ih];
        else if (D_e81c < 15)
            vl = D_e86e[ih] + F_290e_2b65[prn].attr[ih];
        else if (D_e81c <= 0x28 || ih >= 2)
            vl = D_e86e[ih] + (F_290e_3521[agk][ih] + F_290e_2cc1[prn].attr[ih]);
        else
            vl = D_e86e[ih] + F_290e_3521[agk][ih];
        sprintf(bufg, "%02d", vl);
        sub_root_047c_055a(p, 0xc9, ih * 8 + 0x14, bufg);
    }
    sprintf(bufg, "%02d", D_e86e[6]);
    sub_root_047c_055a(p, 0xc9, 0x44, bufg);
    if (D_8c1e == 2)
        sub_root_047c_04dc(p, 0xc5, D_8bfc * 8 + 0x14, 0x26, 8, 0xbf, 10);
    /* weapon skills */
    sub_root_047c_0553(D_2806, 0x113, 0xf, 0x28, 0x3c, D_27f4, 0x113, 0xf);
    sub_root_047c_055a(p, 0x122, 0x14, "wEdg");
    sub_root_047c_055a(p, 0x122, 0x1c, "wImp");
    sub_root_047c_055a(p, 0x122, 0x24, "wFll");
    sub_root_047c_055a(p, 0x122, 0x2c, "wPol");
    sub_root_047c_055a(p, 0x122, 0x34, "wThr");
    sub_root_047c_055a(p, 0x122, 0x3c, "wBow");
    sub_root_047c_055a(p, 0x122, 0x44, "wMsD");
    sub_root_047c_0553(D_2806, 0xf5, 0xf, 0x18, 0x3f, D_27f4, 0xf5, 0xf);
    for (ih = 0; ih < 7; ih++) {
        sprintf(bufg, "%02d", D_e875[ih]);
        sub_root_047c_055a(p, 0x116, ih * 8 + 0x14, bufg);
        if (D_e81c >= 0x19 - (D_8c1e == 3 ? 5 : 0) && D_8c1e != 2) {
            sprintf(bufg, "%2d:%02d", F_290e_2cc1[prn].skill[ih], F_290e_2cc1[prn].maxsk[ih] - D_8c22[ih]);
            sub_root_047c_055a(p, 0xf6, ih * 8 + 0x14, bufg);
        } else {
            if (D_e81c >= 15 && D_8c1e != 2)
                sprintf(bufg, "%2d:%02d", F_290e_2cc1[prn].skill[ih] + 2, F_290e_2cc1[prn].maxsk[ih] - D_8c22[ih]);
            else if (D_8c1e == 0)
                sprintf(bufg, "0:00");
            else
                sprintf(bufg, "%2d:00", F_290e_2b65[prn].skill[ih]);
            sub_root_047c_055a(p, 0xf6, ih * 8 + 0x14, bufg);
        }
    }
    /* knowledge skills */
    sub_root_047c_0553(D_2806, 0x113, 0x4e, 0x28, 0x34, D_27f4, 0x113, 0x4e);
    sub_root_047c_055a(p, 0x122, 0x54, "Alch");
    sub_root_047c_055a(p, 0x122, 0x5c, "Relg");
    sub_root_047c_055a(p, 0x122, 0x64, "Virt");
    sub_root_047c_055a(p, 0x122, 0x6c, "SpkC");
    sub_root_047c_055a(p, 0x122, 0x74, "SpkL");
    sub_root_047c_055a(p, 0x122, 0x7c, "R&W");
    sub_root_047c_0553(D_2806, 0xf5, 0x4e, 0x18, 0x34, D_27f4, 0xf5, 0x4e);
    for (ih = 0; ih < 6; ih++) {
        sprintf(bufg, "%02d", D_e875[7 + ih]);
        sub_root_047c_055a(p, 0x116, ih * 8 + 0x54, bufg);
        if (D_e81c >= 0x19 - (D_8c1e == 3 ? 5 : 0) && D_8c1e != 2) {
            sprintf(bufg, "%2d:%02d", F_290e_2cc1[prn].skill[7 + ih], F_290e_2cc1[prn].maxsk[7 + ih] - D_8c22[7 + ih]);
            sub_root_047c_055a(p, 0xf6, ih * 8 + 0x54, bufg);
        } else {
            if (D_e81c >= 15 && D_8c1e != 2)
                sprintf(bufg, "%2d:%02d", F_290e_2cc1[prn].skill[7 + ih] + 2, F_290e_2cc1[prn].maxsk[7 + ih] - D_8c22[7 + ih]);
            else if (D_8c1e == 0)
                sprintf(bufg, "0:00");
            else
                sprintf(bufg, "%2d:00", F_290e_2b65[prn].skill[7 + ih]);
            sub_root_047c_055a(p, 0xf6, ih * 8 + 0x54, bufg);
        }
    }
    /* other skills */
    sub_root_047c_0553(D_2806, 0x113, 0x86, 0x34, 0x36, D_27f4, 0x113, 0x86);
    sub_root_047c_055a(p, 0x122, 0x8e, "Heal");
    sub_root_047c_055a(p, 0x122, 0x96, "Artf");
    sub_root_047c_055a(p, 0x122, 0x9e, "Stlh");
    sub_root_047c_055a(p, 0x122, 0xa6, "StrW");
    sub_root_047c_055a(p, 0x122, 0xae, "Ride");
    sub_root_047c_055a(p, 0x122, 0xb6, "WdWs");
    sub_root_047c_0553(D_2806, 0xf5, 0x86, 0x18, 0x34, D_27f4, 0xf5, 0x86);
    for (ih = 0; ih < 6; ih++) {
        sprintf(bufg, "%02d", D_e875[13 + ih]);
        sub_root_047c_055a(p, 0x116, ih * 8 + 0x8e, bufg);
        if (D_e81c >= 0x19 - (D_8c1e == 3 ? 5 : 0) && D_8c1e != 2) {
            sprintf(bufg, "%2d:%02d", F_290e_2cc1[prn].skill[13 + ih], F_290e_2cc1[prn].maxsk[13 + ih] - D_8c22[13 + ih]);
            sub_root_047c_055a(p, 0xf6, ih * 8 + 0x8e, bufg);
        } else {
            if (D_e81c >= 15 && D_8c1e != 2)
                sprintf(bufg, "%2d:%02d", F_290e_2cc1[prn].skill[13 + ih] + 2, F_290e_2cc1[prn].maxsk[13 + ih] - D_8c22[13 + ih]);
            else if (D_8c1e == 0)
                sprintf(bufg, "0:00");
            else
                sprintf(bufg, "%2d:00", F_290e_2b65[prn].skill[13 + ih]);
            sub_root_047c_055a(p, 0xf6, ih * 8 + 0x8e, bufg);
        }
    }
    if (D_8c1e == 4)
        sub_root_047c_04dc(p, 0xf6, D_8bfc * 8 + 0x14, 0x44, 8, 0xbf, 10);
    sub_root_047c_1293();
}

/* reset the character being built: name from F_2e38_1a96 (first word, max 9 chars),
   age 0, attributes 12 (Agl 15), skills 0, no professions, no virtues/saints/items */
void far sub_s037_18c8_20fa(void)
{
    int i;
    int j;
    D_8c02 = -1;
    D_8c12 = -1;
    D_442a = -1;
    D_4428 = -1;
    sub_s029_1367_0db4(time(0), 0, 0);
    _fstrcpy((char far *)D_e82f, F_2e38_1a96);
    strncpy(D_e848, D_e82f, sub_s029_1367_0066(0, 9, sub_root_06a1_2824(D_e82f, " ")));
    i = sub_s029_1367_0066(0, 9, sub_root_06a1_2824(D_e82f, " "));
    D_e848[i] = 0;
    D_8bfe = 1;
    D_e81c = 0;
    D_e821 = 0;
    D_e81e = 1;
    D_e80e = 0x110;
    D_e810 = 0x50;
    D_e824 = 5;
    D_e823 = 5;
    D_e86e[1] = 0xf;
    D_e86e[0] = 0xc;
    D_e86e[2] = 0xc;
    D_e86e[3] = 0xc;
    D_e86e[4] = 0xc;
    D_e86e[5] = 0xb;
    D_e86e[6] = 0x63;
    for (i = 0; i < 7; i++)
        D_e867[i] = D_e86e[i];
    *(volatile int *)&D_e818 = 0;
    D_e81f = 0x41;
    D_e853 = 0xa;
    D_e822 = -1;
    D_e854 = -1;
    D_e85b = -1;
    D_e82c = -1;
    D_e855 = -1;
    D_e856 = -1;
    D_e866 = -1;
    D_e875[0] = 0;
    D_e875[1] = 0;
    D_e875[2] = 0;
    D_e875[3] = 0;
    D_e875[4] = 0;
    D_e875[5] = 0;
    D_e875[6] = 0;
    D_e875[7] = 0;
    D_e875[8] = 0;
    D_e875[9] = 0;
    D_e875[10] = 0;
    D_e875[11] = 0;
    D_e875[12] = 0;
    D_e875[13] = 0;
    D_e875[14] = 0;
    D_e875[15] = 0;
    D_e875[16] = 0;
    D_e875[17] = 0;
    D_e875[18] = 0;
    D_e820 = 0;
    D_e824 = 0;
    D_e823 = 0;
    D_e825 = 0;
    D_e826 = 0;
    D_e862 = 0;
    D_e864 = 0;
    D_e859 = 0;
    D_e85a = 0;
    D_e857 = 0;
    D_e858 = 0;
    D_e865 = 0;
    D_e888 = 0;
    _fmemset(F_2e38_3b1a, 0, 0x14);
    _fmemset(F_2e38_3b04, 0, 0x16);
    memset(D_8bd2, -1, 0x28);
    for (j = 0; j < 6; j++)
        D_8bd2[j] = j;
    memset(D_4402, 0, 0x25);
    memset(D_8c22, 0, 0x26);
    memset(D_8c06, 0, 0xc);
}

/* build the list D_8bd2[] (D_8c00 entries, max 15) of the professions the character may
   take next: per profession (0..0x24) requirements on age, attributes, skills, gender,
   the last two professions D_4428/D_442a, the childhood D_8c02 and professions taken
   before (D_4402[p] & 1). Conditions extracted mechanically from the original code. */
void far sub_s037_18c8_22be(void)
{
    int i;
    D_8bfc = 0;
    D_8c00 = 0;
    for (i = 0; i < 0x14; i++)
        D_8bd2[i] = -1;
    for (i = 0; i < 0x25; i++) {
        switch (i) {
    case 0:
        if (!(D_4402[0] & 1) && D_e81c < 0x1e)
            D_8bd2[D_8c00++] = i;
        else if (!(D_4402[0] & 1) && D_e81c < 0x1a && D_e86e[1] >= 0x19)
            D_8bd2[D_8c00++] = i;
        break;
    case 1:
        if ((D_4402[0] & 1) || (D_4402[2] & 1) || (D_4402[3] & 1) || (D_4402[5] & 1)
            || (D_4402[20] & 1) || (D_4402[36] & 1) || (D_4402[1] & 1))
            D_8bd2[D_8c00++] = i;
        break;
    case 2:
        if ((D_4402[1] & 1) || (D_4402[2] & 1) || (D_4402[5] & 1) || (D_4402[3] & 1))
            D_8bd2[D_8c00++] = i;
        break;
    case 3:
        if (D_e86e[4] >= 0x14 && D_e86e[3] >= 0x14 && D_e86e[5] >= 0x14 && ((D_4402[6] & 1)
            || (D_4402[2] & 1) || (D_4402[7] & 1) || (D_4402[5] & 1) || (D_4402[9] & 1)
            || (D_4402[20] & 1) || (D_4402[3] & 1)))
            D_8bd2[D_8c00++] = i;
        break;
    case 4:
        if ((D_4402[6] & 1) || (D_4402[4] & 1) || D_8c02 == 0)
            D_8bd2[D_8c00++] = i;
        break;
    case 5:
        if (D_e875[9] > 0xf && ((D_e81c < 0x14 && D_8c02 == 0) || (D_4402[7] & 1)
            || (D_4402[5] & 1) || D_4428 == 4 || D_4428 == 6 || D_4428 == 9 || D_4428 == 0xf
            || D_4428 == 3))
            D_8bd2[D_8c00++] = i;
        break;
    case 6:
        if ((D_e81c < 0x14 && (D_8c02 == 0 || D_8c02 == 1)) || (D_4402[7] & 1)
            || (D_4402[6] & 1) || D_4428 == 5 || D_4428 == 8 || D_4428 == 9 || D_4428 == 0xf
            || D_4428 == 0x14 || D_4428 == 3)
            D_8bd2[D_8c00++] = i;
        break;
    case 7:
        if (D_4428 == 7 || ((D_442a == 4 || D_442a == 6 || D_442a == 9 || D_442a == 0xf)
            && (D_4428 == 4 || D_4428 == 6 || D_4428 == 9 || D_4428 == 0xf)))
            D_8bd2[D_8c00++] = i;
        break;
    case 8:
        if (D_e821 != 1 && D_e86e[4] >= 0x14 && D_e86e[3] >= 0x14 && D_e86e[5] >= 0x14
            && ((D_4402[7] & 1) || (D_4402[9] & 1) || (D_4402[15] & 1) || (D_4402[8] & 1)
            || D_4428 == 6 || D_4428 == 0xe || D_4428 == 0x14 || D_4428 == 0x16
            || D_4428 == 0x18 || D_4428 == 4 || ((D_442a == 0xc || D_442a == 0xd
            || D_442a == 0x15 || D_442a == 0xa) && (D_4428 == 0xc || D_4428 == 0xd
            || D_4428 == 0x15 || D_4428 == 0xa))))
            D_8bd2[D_8c00++] = i;
        break;
    case 9:
        if (D_e821 != 1 && D_e86e[4] >= 0x19 && D_e86e[3] >= 0x19 && D_e86e[5] >= 0x19
            && (D_4428 == 9 || D_4428 == 0xf || ((D_442a == 7 || D_442a == 8 || D_442a == 6)
            && (D_4428 == 7 || D_4428 == 8 || D_4428 == 6))))
            D_8bd2[D_8c00++] = i;
        break;
    case 10:
        if (D_e821 != 1 && (D_4428 == 8 || D_4428 == 0xa || D_4428 == 0xb || D_4428 == 0xd
            || D_4428 == 0xe || D_4428 == 0xf))
            D_8bd2[D_8c00++] = i;
        break;
    case 11:
        if (D_e875[9] >= 0xf)
            D_8bd2[D_8c00++] = i;
        break;
    case 12:
        if ((D_e81c < 0x14 && D_8c02 != 4 && D_8c02 != 5) || (D_e86e[4] >= 0xc
            && !(D_4402[13] & 1) && !(D_4402[14] & 1) && !(D_4402[15] & 1)
            && !(D_4402[8] & 1) && !(D_4402[9] & 1) && !(D_4402[10] & 1)
            && !(D_4402[15] & 1) && !(D_4402[8] & 1) && !(D_4402[22] & 1)
            && !(D_4402[23] & 1) && !(D_4402[24] & 1) && !(D_4402[25] & 1)))
            D_8bd2[D_8c00++] = i;
        break;
    case 13:
        if (D_e81c < 0x14 || (!(D_4402[13] & 1) && !(D_4402[4] & 1) && !(D_4402[6] & 1)
            && !(D_4402[7] & 1) && !(D_4402[14] & 1) && !(D_4402[15] & 1)
            && !(D_4402[8] & 1) && !(D_4402[9] & 1) && !(D_4402[22] & 1)
            && !(D_4402[23] & 1) && !(D_4402[24] & 1) && !(D_4402[25] & 1)
            && !(D_4402[10] & 1)))
            D_8bd2[D_8c00++] = i;
        break;
    case 14:
        if (D_e86e[4] >= 0xf && D_e86e[3] >= 0xf && D_e86e[5] >= 0xf && D_e875[8] >= 5
            && ((D_4402[4] & 1) || (D_4402[6] & 1) || (D_4402[13] & 1) || (D_4402[7] & 1)
            || (D_4402[14] & 1) || (D_4402[15] & 1) || (D_4402[8] & 1) || (D_4402[9] & 1)
            || (D_4402[22] & 1) || (D_4402[23] & 1) || (D_4402[24] & 1) || (D_4402[25] & 1)
            || (D_4402[10] & 1) || (D_4402[21] & 1)))
            D_8bd2[D_8c00++] = i;
        break;
    case 15:
        if (D_e86e[4] >= 0x14 && D_e86e[3] >= 0x14 && D_e86e[5] >= 0x14 && D_e875[8] >= 0xf
            && D_e86e[4] >= 0xc && (D_4428 == 4 || D_4428 == 6 || D_4428 == 8 || D_4428 == 7
            || D_4428 == 9 || D_4428 == 0xf || ((D_442a == 0xe || D_442a == 0x18)
            && (D_4428 == 0xe || D_4428 == 0x18))))
            D_8bd2[D_8c00++] = i;
        break;
    case 16:
        if ((D_e81c < 0x14 && (D_8c02 == 4 || D_8c02 == 5)) || (D_4428 != 4 && D_4428 != 3
            && D_4428 != 5 && D_4428 != 6 && D_4428 != 8 && D_4428 != 0x12 && D_4428 != 0x18
            && D_4428 != 0x19 && D_4428 != 0x1c && (!D_4402[7] & 1) && (!D_4402[15] & 1)
            && (!D_4402[9] & 1) && (!D_4402[19] & 1) && (!D_4402[20] & 1)
            && (!D_4402[26] & 1) && (!D_4402[29] & 1)))
            D_8bd2[D_8c00++] = i;
        break;
    case 17:
        if ((D_e81c < 0x14 && D_8c02 != 4 && D_8c02 != 5) || (D_e875[10] >= 5
            && D_e86e[4] >= 0xc && ((D_4402[3] & 1) || (D_4402[4] & 1) || (D_4402[6] & 1)
            || (D_4402[8] & 1) || (D_4402[9] & 1) || (D_4402[14] & 1) || (D_4402[16] & 1)
            || (D_4402[15] & 1) || (D_4402[18] & 1) || (D_4402[22] & 1) || (D_4402[23] & 1)
            || (D_4402[24] & 1) || (D_4402[25] & 1) || (D_4402[28] & 1) || (D_4402[29] & 1)
            || (D_4402[32] & 1) || (D_4402[17] & 1))))
            D_8bd2[D_8c00++] = i;
        break;
    case 18:
        if ((D_e81c < 0x14 && (D_8c02 == 0 || D_8c02 == 1)) || (D_e875[10] >= 0xa
            && D_e86e[4] >= 0xf && ((D_4402[17] & 1) || (D_4402[4] & 1) || (D_4402[7] & 1)
            || (D_4402[19] & 1) || (D_4402[18] & 1) || (D_4402[24] & 1) || (D_4402[26] & 1)
            || (D_4402[29] & 1))))
            D_8bd2[D_8c00++] = i;
        break;
    case 19:
        if (D_e875[10] >= 0xa && D_e86e[4] >= 0x14 && (D_4428 == 7 || D_4428 == 9
            || D_4428 == 0x12 || D_4428 == 0x13))
            D_8bd2[D_8c00++] = i;
        break;
    case 20:
        if ((D_4402[34] & 1) && ((D_4402[4] & 1) || (D_4402[2] & 1) || (D_4402[3] & 1)
            || (D_4402[5] & 1) || (D_4402[7] & 1) || (D_4402[8] & 1) || (D_4402[15] & 1)
            || (D_4402[9] & 1) || (D_4402[19] & 1) || (D_4402[24] & 1) || (D_4402[29] & 1)
            || (D_4402[26] & 1) || (D_4402[20] & 1)))
            D_8bd2[D_8c00++] = i;
        break;
    case 21:
        if (D_e81c < 0x14 || (D_e875[12] >= 6 && D_e86e[4] >= 0xc && (D_4428 == 0
            || D_4428 == 2 || D_4428 == 1 || D_4428 == 0xa || D_4428 == 0xb || D_4428 == 0xc
            || D_4428 == 0xd || D_4428 == 0xe || D_4428 == 0x10 || D_4428 == 0x11
            || D_4428 == 0x12 || D_4428 == 0x15 || D_4428 == 0x1c || D_4428 == 0x1b
            || D_4428 == 4 || D_4428 == 0x20 || D_4428 == 0x21)))
            D_8bd2[D_8c00++] = i;
        break;
    case 22:
        if ((D_e81c < 0x14 && D_8c02 == 1) || (D_e875[12] >= 0xf && D_e86e[4] >= 0xc
            && ((D_4402[4] & 1) || (D_4402[6] & 1) || (D_4402[3] & 1) || (D_4402[5] & 1)
            || (D_4402[20] & 1) || (D_4402[8] & 1) || (D_4402[15] & 1) || (D_4402[9] & 1)
            || (D_4402[19] & 1) || (D_4402[24] & 1) || (D_4402[21] & 1) || (D_4402[22] & 1)
            || (D_4402[25] & 1) || (D_4402[26] & 1) || ((D_442a == 0xc || D_442a == 0xe
            || D_442a == 0x12) && (D_4428 == 0xc || D_4428 == 0xe || D_4428 == 0x12)))))
            D_8bd2[D_8c00++] = i;
        break;
    case 23:
        if (D_e875[13] >= 0xf && ((D_4402[21] & 1) || (D_4402[22] & 1) || (D_4402[24] & 1)
            || (D_4402[25] & 1) || (D_4402[26] & 1) || (D_4402[23] & 1)))
            D_8bd2[D_8c00++] = i;
        break;
    case 24:
        if (D_e875[12] >= 0x14 && ((D_4402[9] & 1) || (D_4402[22] & 1) || (D_4402[23] & 1)
            || (D_4402[25] & 1) || (D_4402[26] & 1) || (D_4402[15] & 1) || (D_4402[24] & 1)))
            D_8bd2[D_8c00++] = i;
        break;
    case 25:
        if (D_e86e[4] >= 0x1e && ((D_4402[8] & 1) || (D_4402[9] & 1) || (D_4402[15] & 1)
            || (D_4402[21] & 1) || (D_4402[24] & 1) || (D_4402[22] & 1) || (D_4402[25] & 1)
            || (D_4402[26] & 1) || ((D_442a == 0xc || D_442a == 0xe || D_442a == 0xa
            || D_442a == 0x17) && (D_4428 == 0xc || D_4428 == 0xe || D_4428 == 0xa
            || D_4428 == 0x17))))
            D_8bd2[D_8c00++] = i;
        break;
    case 26:
        if (D_e86e[4] >= 0x23 && (D_4402[25] & 1))
            D_8bd2[D_8c00++] = i;
        break;
    case 27:
        if ((D_e81c < 0x14 && D_8c02 != 0) || (D_e81c >= 0x14 && !(D_4402[27] & 1)
            && !(D_4402[7] & 1) && !(D_4402[9] & 1) && !(D_4402[15] & 1)
            && !(D_4402[19] & 1) && !(D_4402[26] & 1) && !(D_4402[28] & 1)
            && !(D_4402[29] & 1) && D_4428 != 3 && D_4428 != 4 && D_4428 != 5 && D_4428 != 6
            && D_4428 != 8 && D_4428 != 0x14 && D_4428 != 0x16 && D_4428 != 0x12
            && D_4428 != 0x17 && D_4428 != 0x18 && D_4428 != 0x19))
            D_8bd2[D_8c00++] = i;
        break;
    case 28:
        if ((D_e81c < 0x14 && (D_8c02 == 2 || D_8c02 == 3)) || (D_4402[23] & 1)
            || (D_4402[25] & 1) || (D_4402[26] & 1) || (D_4402[28] & 1) || (D_4402[29] & 1)
            || (D_4402[27] & 1))
            D_8bd2[D_8c00++] = i;
        break;
    case 29:
        if (D_e86e[4] >= 0xc && ((D_4402[19] & 1) || (D_4402[26] & 1) || (D_4402[28] & 1)))
            D_8bd2[D_8c00++] = i;
        break;
    case 30:
        if ((D_e81c < 0x14 && D_8c02 != 0) || (D_e81c >= 0x14 && !(D_4402[7] & 1)
            && !(D_4402[9] & 1) && !(D_4402[15] & 1) && !(D_4402[19] & 1)
            && !(D_4402[26] & 1) && !(D_4402[29] & 1) && D_4428 != 3 && D_4428 != 4
            && D_4428 != 5 && D_4428 != 6 && D_4428 != 8 && D_4428 != 0x14 && D_4428 != 0x18
            && D_4428 != 0x12 && D_4428 != 0x19))
            D_8bd2[D_8c00++] = i;
        break;
    case 31:
        if ((D_e81c < 0x14 && D_8c02 != 0) || (D_e81c >= 0x14 && !(D_4402[7] & 1)
            && !(D_4402[9] & 1) && !(D_4402[15] & 1) && !(D_4402[19] & 1)
            && !(D_4402[26] & 1) && !(D_4402[28] & 1) && !(D_4402[29] & 1) && D_4428 != 3
            && D_4428 != 4 && D_4428 != 5 && D_4428 != 6 && D_4428 != 8 && D_4428 != 0x14
            && D_4428 != 0x18 && D_4428 != 0x12 && D_4428 != 0x19))
            D_8bd2[D_8c00++] = i;
        break;
    case 32:
        if ((D_e81c < 0x14 && (D_8c02 == 0 || D_8c02 == 1 || D_8c02 == 2))
            || (D_e86e[4] >= 0x19 && D_e875[16] >= 0xf && ((D_4402[1] & 1)
            || (D_4402[2] & 1) || (D_4402[3] & 1) || (D_4402[4] & 1) || (D_4402[8] & 1)
            || (D_4402[10] & 1) || (D_4402[14] & 1) || (D_4402[16] & 1) || (D_4402[17] & 1)
            || (D_4402[18] & 1) || (D_4402[21] & 1) || (D_4402[22] & 1) || (D_4402[24] & 1)
            || (D_4402[25] & 1) || (D_4402[28] & 1) || (D_4402[30] & 1) || (D_4402[33] & 1)
            || (D_4402[31] & 1))))
            D_8bd2[D_8c00++] = i;
        break;
    case 33:
        if ((D_e81c < 0x14 && (D_8c02 != 3 || D_8c02 != 5)) || (D_e875[16] >= 0xa
            && ((D_4402[1] & 1) || (D_4402[2] & 1) || (D_4402[8] & 1) || (D_4402[10] & 1)
            || (D_4402[35] & 1) || (D_4402[16] & 1) || (D_4402[17] & 1) || (D_4402[18] & 1)
            || (D_4402[21] & 1) || (D_4402[22] & 1) || (D_4402[24] & 1) || (D_4402[25] & 1)
            || (D_4402[28] & 1) || (D_4402[30] & 1) || (D_4402[33] & 1) || (D_4402[31] & 1)
            || (D_4402[36] & 1))))
            D_8bd2[D_8c00++] = i;
        break;
    case 34:
        if ((D_e81c < 0x14 && (D_8c02 != 0 || D_8c02 != 1)) || (!(D_4402[7] & 1)
            && !(D_4402[9] & 1) && !(D_4402[15] & 1) && !(D_4402[19] & 1)
            && !(D_4402[26] & 1) && !(D_4402[29] & 1) && D_4428 != 3 && D_4428 != 4
            && D_4428 != 5 && D_4428 != 6 && D_4428 != 8 && D_4428 != 0x1c && D_4428 != 0x18
            && D_4428 != 0x12 && D_4428 != 0x19))
            D_8bd2[D_8c00++] = i;
        break;
    case 35:
        if ((D_e81c < 0x14 && (D_8c02 == 0 || D_8c02 == 3 || D_8c02 == 5))
            || (D_e875[18] >= 0xf && (D_4428 != 7 || D_4428 != 0xf || D_4428 != 0x1a
            || D_4428 != 0x13 || D_4428 != 0x1d) && ((D_4402[0] & 1) || (D_4402[1] & 1)
            || (D_4402[2] & 1) || (D_4402[3] & 1) || (D_4402[5] & 1) || (D_4402[10] & 1)
            || (D_4402[16] & 1) || (D_4402[11] & 1) || (D_4402[20] & 1) || (D_4402[18] & 1)
            || (D_4402[34] & 1) || (D_4402[35] & 1) || (D_4402[36] & 1))))
            D_8bd2[D_8c00++] = i;
        break;
    case 36:
        if (D_e81c < 0x14 || (D_4402[0] & 1) || (D_4402[1] & 1) || (D_4402[2] & 1)
            || (D_4402[3] & 1) || (D_4402[5] & 1) || (D_4402[33] & 1) || (D_4402[36] & 1)
            || (D_4402[35] & 1) || D_4428 == 0xa || D_4428 == 0xb || D_4428 == 0x14
            || D_4428 == 0x10 || D_4428 == 0x1e || D_4428 == 0x1f || D_4428 == 0x20
            || D_4428 == 0x22)
            D_8bd2[D_8c00++] = i;
        break;
        }
        if (D_8c00 > 0xe)
            break;
    }
}

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

/* add one item of type id to the starting equipment list F_2e38_3984 (D_e888 entries,
   max 0x40): an entry with the same type and the same two properties gets cnt + 1 */
void far sub_s037_18c8_3a9e(int id)
{
    struct item it;
    struct item far *p;
    int i;
    it.id = id;
    it.f2 = F_2e38_0890[id]->f1e;
    it.qual = F_2e38_0890[id]->f26;
    it.f5 = F_2e38_0890[id]->f25;
    it.cnt = 1;
    for (i = 0; i < D_e888; i++)
        if (F_2e38_3984[i].id == it.id && F_2e38_3984[i].f2 == it.f2
            && F_2e38_3984[i].qual == it.qual) {
            F_2e38_3984[i].cnt++;
            return;
        }
    if (D_e888 < 0x40) {
        p = &F_2e38_3984[D_e888++];
        *p = it;
    }
}
