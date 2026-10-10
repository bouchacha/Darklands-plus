/* place: root 03a1:000a */
/* orig: size=04b4 CONST=7d10+0002 _DATA=038e+013a */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* status: FILE MATCH (w4, 2026-10-10). pragma "a" (D_a764 kept in DX); memset(D_a7a6, 1, 20)
   is an int fill loop (6.00 rep stosw with 0101h, which 6.00A only gives for the loop - it also
   fixes the register swap of the 04d5 loop); the empty function at 0000:2652 is F_0000_2652. */
/* Game main loop: intro (opening animations), startup (message files, world, pictures),
 * then dispatch D_a772 (current screen/card) through the far table F_2dae_0000 until a
 * screen returns 0x62 or -0x63; -2..-6 start a battle. */

struct member { char status; char _p[127]; };
struct flags1 { unsigned b0 : 1; };

extern struct member D_9c69[5];
extern int D_0bf2;
extern char D_a876;
extern int D_a74a, D_a74c, D_a86e, D_a870, D_a872, D_a874;
extern long D_a74e;
extern int D_a6e0[26];
extern char D_9c4c;
extern int far *D_9060;
extern int D_a67e;
extern char D_a7a6[20];
extern int D_a776, D_a774, D_907e, D_a772, D_a764;
extern int D_a61e, D_a620, D_a622, D_a624, D_a62a, D_a62e, D_a630, D_a632, D_a65e;
extern int D_a752[];
extern unsigned char D_9080, D_9081;
extern int D_a766, D_a768, D_a76a, D_a76c, D_a76e, D_a770;
extern int D_a893, D_a891, D_a88f, D_a88d;
extern char D_907d;
extern int D_940b;
extern int D_a88b, D_9787, D_a887;
extern int (far * far F_2dae_0000[])(void);

int far kbhit(void);
int far getch(void);
int far remove(const char *);
void far srand(unsigned);
long far time(long *);
int far setjmp(int *);
void far exit_(int);
void far free_buffer(unsigned, char *);
int far sub_root_0000_0c70();
int far sub_root_0000_1354();
int far sub_root_0000_2646();
int far sub_root_030b_0004();
int far sub_root_044a_01f4();
int far sub_root_047c_04d5();
int far sub_root_047c_04f8();
int far sub_root_047c_0545();
int far sub_root_047c_0584();
int far sub_root_047c_0783();
int far sub_root_047c_0791();
int far sub_root_047c_11f6();
int far sub_root_047c_12c3();
int far sub_root_05f8_0139();
int far sub_root_0617_0230();
int far sub_root_0617_045c();
int far sub_root_0617_06d8();
int far sub_root_06a1_2108();
void far F_0000_2652(int);          /* empty function at 0000:2652 (root_0000.c) */
int far vec_186f();
int far vec_18b5();
int far vec_18bf();
int far vec_18c9();
int far vec_18d3();
int far vec_18dd();
int far vec_18e7();
int far vec_18f1();
int far vec_18fb();
int far vec_1905();
int far vec_190f();
int far vec_1919();

int D_04c6 = 10;

#pragma optimize("a", on)
void far sub_root_03a1_000a(void)
{
    unsigned i;
    int n;

    remove("events.tmp");
    vec_1905("1", "w");
    if (!D_0bf2) {
        D_a74e = 0;
        if (D_a876 == 'a' || D_a876 == 'p') {
            vec_18fb("opendark.dgt");
            sub_root_047c_0783(*((int *)&D_a74e + 1), D_a74a, D_a74c, D_a870, D_a86e, D_a872, D_a874);
        }
        vec_18f1(0x2f);
        sub_root_030b_0004("opening2");
        vec_18f1(0x30);
        sub_root_030b_0004("opening3");
        vec_18f1(0x31);
        sub_root_030b_0004("opening4");
        vec_18f1(0x32);
        sub_root_030b_0004("openin6z");
        vec_18f1(0x33);
        sub_root_030b_0004("opening7");
        vec_18f1(0x34);
        sub_root_030b_0004("opening8");
        vec_18f1(0x35);
        sub_root_030b_0004("opening9");
        if (kbhit()) {
            vec_18f1(0);
            getch();
            vec_18f1(7);
        }
        if (D_a74e) {
            sub_root_06a1_2108(D_a74e);
            D_a74e = 0;
        }
    }
    srand((unsigned)time(0));
    sub_root_047c_0584(0x1c0);
    D_9c4c = sub_root_047c_11f6() != 0;
    D_9060 = (int far *)&D_a67e;
    for (i = 0; i < 10; i++)
        ((int *)D_a7a6)[i] = 0x101;
    D_a776 = 0;
    D_a774 = sub_root_0617_06d8("msgfiles");
    D_907e = sub_root_0000_0c70(0x5a);
    D_a772 = 0x29;
    vec_18e7("darkland.loc");
    sub_root_0617_0230(1, "pics\\Illmcaps.pic");
    sub_root_047c_0545();
    for (i = 0; i < 26; i++)
        D_a6e0[i] = sub_root_047c_04d5(1, i % 15 * 21, i / 15 * 21, 20, 20);
    D_a61e = sub_root_0617_045c("pics\\rpbdrtop.pic");
    D_a620 = sub_root_0617_045c("pics\\rpbdrbtm.pic");
    D_a622 = sub_root_0617_045c("pics\\rpbdrlft.pic");
    D_a624 = sub_root_0617_045c("pics\\rpbdrrgt.pic");
    D_a62a = sub_root_0617_045c("pics\\timer01.pic");
    D_a62e = sub_root_0617_045c("pics\\timer03.pic");
    D_a630 = sub_root_0617_045c("pics\\timer04.pic");
    D_a632 = sub_root_0617_045c("pics\\timer05.pic");
    D_a65e = sub_root_0617_045c("pics\\textback.pic");
    sub_root_047c_04f8();
    vec_18dd();
    setjmp(D_a752);
    vec_18d3();
    F_0000_2652(D_04c6++);
    do {
        if (!(D_9081 & 1))
            D_a764 = (*F_2dae_0000[D_a772])();
        if (((struct flags1 *)&D_9081)->b0 == 1 || D_a764 == -2 || D_a764 == -3 ||
            D_a764 == -4 || D_a764 == -5 || D_a764 == -6) {
            vec_18c9();
            if (D_a764 == -3)
                D_9080 |= 4;
            if (D_a764 == -5)
                D_9080 |= 8;
            if (D_a764 == -6)
                D_9080 |= 0x80;
            if (D_a764 == -4)
                D_9080 |= 0xc;
            D_9080 |= 1;
            D_a764 = vec_18bf(D_a766, D_a768, D_a76a, -1, D_a76c, 0, -1, D_a76e, 0, -1, D_a770, 0);
            sub_root_0000_2646("aft bat.");
            for (i = 0; i < 5; i++) {
                if (D_9c69[i].status == 3) {
                    D_9c69[i].status = 0;
                    vec_18b5(i);
                }
            }
            for (i = 0; i < 5; i++) {
                if (D_9c69[i].status != 0 && D_9c69[i].status != 5)
                    D_9c69[i].status = 1;
            }
            n = 0;
            for (i = 0; i < 5; i++) {
                if (D_9c69[i].status != 0)
                    n++;
            }
            if (n == 0 || D_a893 == 5) {
                sub_root_047c_12c3();
                vec_18f1(0x12);
                if (!D_0bf2) {
                    sub_root_030b_0004("death");
                    if (kbhit())
                        getch();
                }
                sub_root_044a_01f4();
                sub_root_047c_0791();
                vec_186f();
                sub_root_0000_1354();
                if (D_907d == 1)
                    sub_root_05f8_0139(D_940b);
                exit_(0);
            }
            vec_1919(0);
            vec_190f(0);
        }
        if (D_a772 != 0xc)
            D_a891 = 0;
        D_a88f = D_a88d;
        D_a88d = D_a772;
        D_a772 = D_a764;
        vec_190f(0);
    } while (D_a764 != 0x62 && D_a764 != -0x63);
    free_buffer(D_a6e0[0], "C");
    free_buffer(D_a88b, "S");
    free_buffer(D_9787, "I");
    free_buffer(D_a887, "M");
}
