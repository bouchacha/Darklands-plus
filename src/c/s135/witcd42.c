/* place: s135 1838:0000 */
/* orig: size=06f4 CONST=829e+0002 _DATA=6a76+0009 */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* Card script module for card "$Witcd42" (overlay s135). */
#include "darklands/card.h"

struct res { int a; int b; };
struct world { char pad[0xa47]; char far *loc_name; };

extern char card_name[];
extern char pic_buf[];
extern char msg_buf[];
extern char D_6a76[];             /* "$Witcd42" */
extern char str_success_assured[];             /* "Success is assured." */
extern char str_screwed_up[];             /* "screwed up." */
extern struct world far pic_names;
extern int D_ee72, next_card, D_ee70, battle_mode, menu_member, var_chosen1, menu_slot;
extern int opt_flag_0, opt_flag_1, opt_flag_2, opt_flag_3, opt_flag_4, opt_flag_5, opt_flag_6, opt_flag_7, opt_flag_8, opt_flag_9;
extern int card_saints[], card_items[];
extern char opt_on[];
extern void (far *opt_fn[20])(void);
extern void (far *item_fn[5][4])(void);
extern void (far *saint_fn[5][4])(void);

char far *_fstrcpy(char far *, const char far *);
char *strcpy(char *, const char *);
void *memset(void *, int, unsigned);

void far card_load(char *, char far *);
void far card_text(int, int);
void far advance_hours(int);
int far saint_chance_lo(int, int);
int far saint_chance_hi(int, int);
void far chance_text(char *, int, int);

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
    strcpy(card_name, "$Witcd42");
    _fstrcpy((char far *)pic_buf, pic_names.loc_name);
    card_load(card_name, (char far *)pic_buf);
    D_ee72 = 7;
    next_card = 0x16c;
    opt_flag_0 = 1;
    opt_flag_1 = 1;
    opt_flag_2 = 1;
    opt_flag_3 = 1;
    opt_flag_4 = 1;
    opt_flag_5 = 1;
    opt_flag_6 = 1;
    opt_flag_7 = 1;
    opt_flag_8 = 1;
    opt_flag_9 = 1;
    D_ee70 = 0;
    card_text(0, 0);
    battle_mode = 1;
    return -3;
}

void far act_0082(void)
{
    advance_hours(0);
    next_card = 0x62;
}

struct res far hlp_0094(void)
{
    struct res r;
    strcpy(msg_buf, str_success_assured);
    return r;
}

void far act_00c2(void)
{
    advance_hours(0);
    next_card = 0x62;
}

struct res far hlp_00d4(void)
{
    struct res r;
    strcpy(msg_buf, str_success_assured);
    return r;
}

void far act_0102(void)
{
    advance_hours(0);
    next_card = 0x62;
}

struct res far hlp_0114(void)
{
    struct res r;
    strcpy(msg_buf, str_success_assured);
    return r;
}

void far act_0142(void)
{
    advance_hours(0);
    next_card = 0x62;
}

struct res far hlp_0154(void)
{
    struct res r;
    strcpy(msg_buf, str_success_assured);
    return r;
}

void far act_0182(void)
{
    advance_hours(0);
    next_card = 0x62;
}

struct res far hlp_0194(void)
{
    struct res r;
    strcpy(msg_buf, str_success_assured);
    return r;
}

void far act_01c2(void)
{
    advance_hours(0);
    next_card = 0x62;
}

struct res far hlp_01d4(void)
{
    struct res r;
    strcpy(msg_buf, str_success_assured);
    return r;
}

void far act_0202(void)
{
    advance_hours(0);
    next_card = 0x62;
}

struct res far hlp_0214(void)
{
    struct res r;
    strcpy(msg_buf, str_success_assured);
    return r;
}

void far act_0242(void)
{
    advance_hours(0);
    next_card = 0x62;
}

struct res far hlp_0254(void)
{
    struct res r;
    strcpy(msg_buf, str_success_assured);
    return r;
}

void far act_0282(void)
{
    advance_hours(0);
    next_card = 0x62;
}

struct res far hlp_0294(void)
{
    struct res r;
    strcpy(msg_buf, str_success_assured);
    return r;
}

void far act_02c2(void)
{
    advance_hours(0);
    next_card = 0x62;
}

struct res far hlp_02d4(void)
{
    struct res r;
    strcpy(msg_buf, str_success_assured);
    return r;
}

void far register_0302(void)
{
    int i, k;
    for (i = 0; i < 20; i++) {
        opt_on[i] = 0;
        opt_fn[i] = (void (far *)(void))hlp_03a4;
    }
    for (k = 0; k < 5; k++) {
        item_fn[k][0] = (void (far *)(void))hlp_03dc;
        item_fn[k][1] = (void (far *)(void))hlp_0418;
        item_fn[k][2] = (void (far *)(void))hlp_0454;
        item_fn[k][3] = (void (far *)(void))hlp_0490;
        saint_fn[k][0] = (void (far *)(void))opt_04cc;
        saint_fn[k][1] = (void (far *)(void))opt_0556;
        saint_fn[k][2] = (void (far *)(void))opt_05e0;
        saint_fn[k][3] = (void (far *)(void))opt_066a;
    }
    memset(card_saints, -1, 8);
    memset(card_items, -1, 8);
}

struct res far hlp_03a4(void)
{
    struct res r;
    strcpy(msg_buf, str_screwed_up);
    r.a = -1;
    r.b = -2;
    return r;
}

struct res far hlp_03dc(void)
{
    struct res r;
    strcpy(msg_buf, str_success_assured);
    r.a = -1;
    r.b = var_chosen1 = menu_member;
    return r;
}

struct res far hlp_0418(void)
{
    struct res r;
    strcpy(msg_buf, str_success_assured);
    r.a = -1;
    r.b = var_chosen1 = menu_member;
    return r;
}

struct res far hlp_0454(void)
{
    struct res r;
    strcpy(msg_buf, str_success_assured);
    r.a = -1;
    r.b = var_chosen1 = menu_member;
    return r;
}

struct res far hlp_0490(void)
{
    struct res r;
    strcpy(msg_buf, str_success_assured);
    r.a = -1;
    r.b = var_chosen1 = menu_member;
    return r;
}

struct res far opt_04cc(void)
{
    char buf[50] = {0};
    int t;
    struct res r;
    t = saint_chance_lo(var_chosen1 = menu_member, card_saints[menu_slot]);
    chance_text(buf, t, saint_chance_hi(menu_member, card_saints[menu_slot]));
    strcpy(msg_buf, buf);
    r.a = -1;
    r.b = -2;
    return r;
}

struct res far opt_0556(void)
{
    char buf[50] = {0};
    int t;
    struct res r;
    t = saint_chance_lo(var_chosen1 = menu_member, card_saints[menu_slot]);
    chance_text(buf, t, saint_chance_hi(menu_member, card_saints[menu_slot]));
    strcpy(msg_buf, buf);
    r.a = -1;
    r.b = -2;
    return r;
}

struct res far opt_05e0(void)
{
    char buf[50] = {0};
    int t;
    struct res r;
    t = saint_chance_lo(var_chosen1 = menu_member, card_saints[menu_slot]);
    chance_text(buf, t, saint_chance_hi(menu_member, card_saints[menu_slot]));
    strcpy(msg_buf, buf);
    r.a = -1;
    r.b = -2;
    return r;
}

struct res far opt_066a(void)
{
    char buf[50] = {0};
    int t;
    struct res r;
    t = saint_chance_lo(var_chosen1 = menu_member, card_saints[menu_slot]);
    chance_text(buf, t, saint_chance_hi(menu_member, card_saints[menu_slot]));
    strcpy(msg_buf, buf);
    r.a = -1;
    r.b = -2;
    return r;
}
