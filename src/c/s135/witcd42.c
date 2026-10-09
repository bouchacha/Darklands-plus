/* place: s135 1838:0000 */
/* orig: size=06f4 CONST=829e+0002 _DATA=6a76+0009 */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* Card script module for card "$Witcd42" (overlay s135). */

struct res { int a; int b; };
struct world { char pad[0xa47]; char far *loc_name; };

extern char D_e3d4[];
extern char D_e3e3[];
extern char D_9085[];
extern char D_6a76[];             /* "$Witcd42" */
extern char D_4f48[];             /* "Success is assured." */
extern char D_4ec4[];             /* "screwed up." */
extern struct world far F_2caf_0000;
extern int D_ee72, D_e48a, D_ee70, D_a766, D_ee6c, D_ee8c, D_ee6a;
extern int D_ee76, D_ee78, D_ee7a, D_ee7c, D_ee7e, D_ee80, D_ee82, D_ee84, D_ee86, D_ee88;
extern int D_ee4b[], D_ee43[];
extern char D_e99c[];
extern void (far *D_ea14[20])(void);
extern void (far *D_ec00[5][4])(void);
extern void (far *D_ecf6[5][4])(void);

char far *_fstrcpy(char far *, const char far *);
char *strcpy(char *, const char *);
void *memset(void *, int, unsigned);

void far sub_s043_150b_000a(char *, char far *);
void far sub_s043_150b_03b0(int, int);
void far sub_s029_1367_05c8(int);
int far sub_s029_0e76_20c6(int, int);
int far sub_s029_0e76_2116(int, int);
void far sub_s029_1367_0c52(char *, int, int);

void far act_0082(void);  struct res far hlp_0094(void);
void far act_00c2(void);  struct res far hlp_00d4(void);
void far act_0102(void);  struct res far hlp_0114(void);
void far act_0142(void);  struct res far hlp_0154(void);
void far act_0182(void);  struct res far hlp_0194(void);
void far act_01c2(void);  struct res far hlp_01d4(void);
void far act_0202(void);  struct res far hlp_0214(void);
void far act_0242(void);  struct res far hlp_0254(void);
void far act_0282(void);  struct res far hlp_0294(void);
void far act_02c2(void);  struct res far hlp_02d4(void);
struct res far hlp_03a4(void);
struct res far hlp_03dc(void); struct res far hlp_0418(void);
struct res far hlp_0454(void); struct res far hlp_0490(void);
struct res far opt_04cc(void); struct res far opt_0556(void);
struct res far opt_05e0(void); struct res far opt_066a(void);

int far card_witcd42(void)
{
    strcpy(D_e3d4, "$Witcd42");
    _fstrcpy((char far *)D_e3e3, F_2caf_0000.loc_name);
    sub_s043_150b_000a(D_e3d4, (char far *)D_e3e3);
    D_ee72 = 7;
    D_e48a = 0x16c;
    D_ee76 = 1;
    D_ee78 = 1;
    D_ee7a = 1;
    D_ee7c = 1;
    D_ee7e = 1;
    D_ee80 = 1;
    D_ee82 = 1;
    D_ee84 = 1;
    D_ee86 = 1;
    D_ee88 = 1;
    D_ee70 = 0;
    sub_s043_150b_03b0(0, 0);
    D_a766 = 1;
    return -3;
}

void far act_0082(void)
{
    sub_s029_1367_05c8(0);
    D_e48a = 0x62;
}

struct res far hlp_0094(void)
{
    struct res r;
    strcpy(D_9085, D_4f48);
    return r;
}

void far act_00c2(void)
{
    sub_s029_1367_05c8(0);
    D_e48a = 0x62;
}

struct res far hlp_00d4(void)
{
    struct res r;
    strcpy(D_9085, D_4f48);
    return r;
}

void far act_0102(void)
{
    sub_s029_1367_05c8(0);
    D_e48a = 0x62;
}

struct res far hlp_0114(void)
{
    struct res r;
    strcpy(D_9085, D_4f48);
    return r;
}

void far act_0142(void)
{
    sub_s029_1367_05c8(0);
    D_e48a = 0x62;
}

struct res far hlp_0154(void)
{
    struct res r;
    strcpy(D_9085, D_4f48);
    return r;
}

void far act_0182(void)
{
    sub_s029_1367_05c8(0);
    D_e48a = 0x62;
}

struct res far hlp_0194(void)
{
    struct res r;
    strcpy(D_9085, D_4f48);
    return r;
}

void far act_01c2(void)
{
    sub_s029_1367_05c8(0);
    D_e48a = 0x62;
}

struct res far hlp_01d4(void)
{
    struct res r;
    strcpy(D_9085, D_4f48);
    return r;
}

void far act_0202(void)
{
    sub_s029_1367_05c8(0);
    D_e48a = 0x62;
}

struct res far hlp_0214(void)
{
    struct res r;
    strcpy(D_9085, D_4f48);
    return r;
}

void far act_0242(void)
{
    sub_s029_1367_05c8(0);
    D_e48a = 0x62;
}

struct res far hlp_0254(void)
{
    struct res r;
    strcpy(D_9085, D_4f48);
    return r;
}

void far act_0282(void)
{
    sub_s029_1367_05c8(0);
    D_e48a = 0x62;
}

struct res far hlp_0294(void)
{
    struct res r;
    strcpy(D_9085, D_4f48);
    return r;
}

void far act_02c2(void)
{
    sub_s029_1367_05c8(0);
    D_e48a = 0x62;
}

struct res far hlp_02d4(void)
{
    struct res r;
    strcpy(D_9085, D_4f48);
    return r;
}

void far register_0302(void)
{
    int i, k;
    for (i = 0; i < 20; i++) {
        D_e99c[i] = 0;
        D_ea14[i] = (void (far *)(void))hlp_03a4;
    }
    for (k = 0; k < 5; k++) {
        D_ec00[k][0] = (void (far *)(void))hlp_03dc;
        D_ec00[k][1] = (void (far *)(void))hlp_0418;
        D_ec00[k][2] = (void (far *)(void))hlp_0454;
        D_ec00[k][3] = (void (far *)(void))hlp_0490;
        D_ecf6[k][0] = (void (far *)(void))opt_04cc;
        D_ecf6[k][1] = (void (far *)(void))opt_0556;
        D_ecf6[k][2] = (void (far *)(void))opt_05e0;
        D_ecf6[k][3] = (void (far *)(void))opt_066a;
    }
    memset(D_ee4b, -1, 8);
    memset(D_ee43, -1, 8);
}

struct res far hlp_03a4(void)
{
    struct res r;
    strcpy(D_9085, D_4ec4);
    r.a = -1;
    r.b = -2;
    return r;
}

struct res far hlp_03dc(void)
{
    struct res r;
    strcpy(D_9085, D_4f48);
    r.a = -1;
    r.b = D_ee8c = D_ee6c;
    return r;
}

struct res far hlp_0418(void)
{
    struct res r;
    strcpy(D_9085, D_4f48);
    r.a = -1;
    r.b = D_ee8c = D_ee6c;
    return r;
}

struct res far hlp_0454(void)
{
    struct res r;
    strcpy(D_9085, D_4f48);
    r.a = -1;
    r.b = D_ee8c = D_ee6c;
    return r;
}

struct res far hlp_0490(void)
{
    struct res r;
    strcpy(D_9085, D_4f48);
    r.a = -1;
    r.b = D_ee8c = D_ee6c;
    return r;
}

struct res far opt_04cc(void)
{
    char buf[50] = {0};
    int t;
    struct res r;
    t = sub_s029_0e76_20c6(D_ee8c = D_ee6c, D_ee4b[D_ee6a]);
    sub_s029_1367_0c52(buf, t, sub_s029_0e76_2116(D_ee6c, D_ee4b[D_ee6a]));
    strcpy(D_9085, buf);
    r.a = -1;
    r.b = -2;
    return r;
}

struct res far opt_0556(void)
{
    char buf[50] = {0};
    int t;
    struct res r;
    t = sub_s029_0e76_20c6(D_ee8c = D_ee6c, D_ee4b[D_ee6a]);
    sub_s029_1367_0c52(buf, t, sub_s029_0e76_2116(D_ee6c, D_ee4b[D_ee6a]));
    strcpy(D_9085, buf);
    r.a = -1;
    r.b = -2;
    return r;
}

struct res far opt_05e0(void)
{
    char buf[50] = {0};
    int t;
    struct res r;
    t = sub_s029_0e76_20c6(D_ee8c = D_ee6c, D_ee4b[D_ee6a]);
    sub_s029_1367_0c52(buf, t, sub_s029_0e76_2116(D_ee6c, D_ee4b[D_ee6a]));
    strcpy(D_9085, buf);
    r.a = -1;
    r.b = -2;
    return r;
}

struct res far opt_066a(void)
{
    char buf[50] = {0};
    int t;
    struct res r;
    t = sub_s029_0e76_20c6(D_ee8c = D_ee6c, D_ee4b[D_ee6a]);
    sub_s029_1367_0c52(buf, t, sub_s029_0e76_2116(D_ee6c, D_ee4b[D_ee6a]));
    strcpy(D_9085, buf);
    r.a = -1;
    r.b = -2;
    return r;
}
