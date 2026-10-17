/* menuview.h - the game's menu bar (Game, Orders, Attack, Party ...) drawn and used by
 * Darklands+ (mods/menuview.c runs it, ui/menuview.cpp draws it). */
#ifndef DL_MENUVIEW_H
#define DL_MENUVIEW_H
#include "nscreen.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    char name[48];            /* the game's text (its own letters), without the tick byte */
    char key[16];             /* the shortcut, spelt out ("Alt+S", "F6", "Space") */
    int enabled, tick, count;
    int nsub;                 /* a sub-menu: its index in MenuModel.subs, -1 none */
} MvItem;

typedef struct {
    char name[24];
    int enabled;
    int first, n;             /* its items: MenuModel.items[first .. first + n) */
} MvMenu;

typedef struct {
    int open;
    int nmenus;
    MvMenu menu[6];
    int nitems;
    MvItem items[96];
    int sub_first[96], sub_n[96];   /* by item: its sub-items in items[] (n 0: none) */
    /* where the choice is (the keyboard moves it; the view follows the pointer) */
    int cur, row, subrow;           /* menu, item (-1 none), sub-item (-1 none) */
    int keyboard;                   /* opened from the keyboard (F10) */
    int held;                       /* opened with the right button held: its release over an item chooses */
    unsigned t_open, seq;
} MenuModel;

extern MenuModel menu_model;
extern NsInput mv_in;             /* the pointer's clicks while the menu is open (the views under it get none) */

/* the view's requests: MV_CHOOSE menu, item, sub-item (-1)   MV_CLOSE */
enum { MV_CHOOSE = 1, MV_CLOSE };
void menuview_push(int type, int a, int b, int c);

void menuview_init(void);
int menuview_is_on(void);
void menuview_set(int on);
int menuview_open(void);          /* the native menu is up (the other views take no clicks) */
const MenuModel *menuview_live(void);              /* the menus' entries now (for buttons elsewhere) */
void menuview_request(int menu, int item, int sub);   /* that entry chosen, at the game's next round */

#ifdef __cplusplus
}
#endif
#endif
