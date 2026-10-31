/* choose.h - a box asking which of the party's things (mods/choose.c runs it, ui/choose.cpp draws
 * it): a line each, the thing's picture, name and quality and the head of who carries it, and a
 * way out. For the relic a cathedral is given (mods/relics.c). */
#ifndef DL_CHOOSE_H
#define DL_CHOOSE_H
#ifdef __cplusplus
extern "C" {
#endif

#define CHOOSE_MAX 24
typedef struct { int id, qual, member, index; } ChooseRow;   /* the item, its quality, who carries it, the record */

typedef struct {
    int open;
    char title[120], none[40];        /* the question; the way out's words */
    int n;
    ChooseRow row[CHOOSE_MAX];
    unsigned seq;
} ChooseModel;
extern ChooseModel choose_model;
enum { CH_PICK = 1, CH_NONE };

/* asks (inside a hook, on the game's stack); the row picked, or -1 for the way out */
int choose_run(const char *title, const ChooseRow *rows, int n, const char *none);

#ifdef __cplusplus
}
#endif
#endif
