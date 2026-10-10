/* mods.h - built-in mods of the native port. Each mod hooks game functions (runtime/hooks.h)
 * or edits game state (runtime/game.h). They are switched on and off in the in-game menu
 * (F12) and the choice is kept in darklands_port.ini. */
#ifndef DL_MODS_H
#define DL_MODS_H
#ifdef __cplusplus
extern "C" {
#endif
typedef struct {
    const char *id, *name, *category, *desc;
    void (*set)(int on);       /* install / remove */
    int enabled;
    int needs_new_game;        /* only takes effect when a new game starts */
} Mod;
extern Mod MODS[];
extern int MOD_COUNT;
void mods_init(void);          /* after the game is loaded: apply the saved choices */
void mod_enable(int i, int on);
#ifdef __cplusplus
}
#endif
#endif
