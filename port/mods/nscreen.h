/* nscreen.h - the frame for native screens (docs/NATIVE_SCREENS.md).
 *
 * A native screen replaces how one of the game's screens is shown and used, and keeps what it
 * does: the game's own routines still buy, sell, equip and decide. Each screen has
 *   - a model, built from the game's data by the screen's loop (never while drawing),
 *   - a view (ui/), drawn from the model alone, which turns clicks into actions,
 *   - actions, carried out by the loop through the game's own routines.
 * Input is the game's own pointer and buttons (mouse_x/y, mouse_buttons), so the game
 * controller, Go to and the test scripts work on native screens as on the original ones.
 * The loop runs inside a hook of the original screen function, on the game's stack. */
#ifndef DL_NSCREEN_H
#define DL_NSCREEN_H
#include "../runtime/cpu.h"
#ifdef __cplusplus
extern "C" {
#endif

/* calling the game from a native loop: the game may unwind (a game loaded from the menu bar,
 * Quit); then ns_unwinding is set and the screen must return at once, without its own return */
extern int ns_unwinding;
int ns_call(int sect, u16 seg, u16 off, int n, ...);       /* sect -1 = root; returns AX */
int ns_call_dx(void);                                     /* DX of the last ns_call */
int ns_call_ptr(u16 seg, u16 off, int n, ...);            /* a far pointer the game keeps (runtime segment) */

/* the pointer, in game pixels, and the left button's clicks (on release) */
typedef struct {
    int x, y;            /* now */
    int click;           /* a left click was released this round (taken by ns_take_click) */
    int cx, cy;          /* where */
    int rclick;          /* a right click */
    int down;            /* the left button is held */
    int pressed;         /* it went down this round (taken by the view) */
    int px, py;          /* where it went down */
    float wheel;         /* mouse wheel notches since the last round (up > 0) */
} NsInput;
extern NsInput ns_in;
void ns_input_begin(void);           /* entering a screen: the button now held does not count */
void ns_input_poll(void);            /* once a round of the loop */
int ns_take_click(int *x, int *y);   /* the view: a click, if there is one (only once) */
int ns_get_key(void);                /* the loop: the next key (F10 opens the menus, Alt-S ... go to hot_keys: 0) */
int ns_hot_key(int k);               /* k (taken already) is one of the game's hot keys: done, 1 */

/* actions the view asks for, done by the loop */
typedef struct { int type, a, b, c; } NsAction;
void ns_push(int type, int a, int b, int c);
int ns_pop(NsAction *out);
void ns_clear(void);

/* one round of the waiting the original screens do: the menu bar (hidden meanwhile while it is
 * open or its choice runs), the screen redrawn when the party changed, exit if asked; then a
 * frame and a short rest. Returns 0 when the game unwinds. */
extern int ns_nested;                 /* 1 a game screen runs inside: the view hides; 2 the menu bar may */
extern unsigned ns_nested_since;
int ns_round(void);
int ns_hidden(void);                  /* the view should not draw now (the menu bar open, a sheet ...) */

/* the native screen on show (NS_NONE none). Screens open inside one another (a sheet from a shop's
 * portrait, a shop from a card): ns_enter / ns_leave keep a stack, ns_screen and ns_nested are
 * the top's; a view draws only while its screen is the top. */
extern int ns_screen;
enum { NS_NONE, NS_CARD, NS_BARTER, NS_INVENTORY, NS_SHEET, NS_LOOT, NS_ALCHEMY, NS_SAVES, NS_CACHE, NS_PRAY };
void ns_enter(int screen);
void ns_leave(void);

int ns_showing(void);                 /* a native screen is up and drawn (the overlay's hints over the game's screens stay away) */
/* the interface (F12's tabs ...) asks for something done in the game: done at the next safe point,
 * the start of a round of a game screen's loop (menu_bar_run), or, while the game is held (F12's
 * window open), at the end of the interface's frame. The request calls the game through
 * game_call_far (runtime/game.h: no frame is drawn inside it), not ns_call. */
typedef void (*NsDeferFn)(int a, int b, int c, int d);
void ns_defer(NsDeferFn fn, int a, int b, int c, int d);
void nscreen_init(void);
void ns_defer_frame_end(int game_held);   /* (ui/ui.cpp, after drawing) */

int ns_active(void);                  /* native screens in use: a window, not "Play the original game" */

#ifdef __cplusplus
}
#endif
#endif
