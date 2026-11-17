/* Native choice dialog for item rows (cathedral relics) or named party members (locks). */
#ifndef DL_CHOOSE_H
#define DL_CHOOSE_H
#ifdef __cplusplus
extern "C" {
#endif

#define CHOOSE_MAX 24
typedef struct {
    int id, qual, member, index;      /* the item, its quality, who carries it, the record */
    char info[120];                   /* what it does (rich text, a line under its name; "" none) */
    int disabled, recommended;       /* unavailable row, or the default selected with Enter */
    char name[32];                    /* member row title; empty for an item row */
    int bad;                          /* (its info in red: e.g. the carrier lacks the Virtue it asks) */
} ChooseRow;

typedef struct {
    int open;
    char title[120], none[40];        /* the question; the way out's words */
    char note[320];                   /* what picking does (rich text, under the question; "" none) */
    int n;
    ChooseRow row[CHOOSE_MAX];
    unsigned seq;
} ChooseModel;
extern ChooseModel choose_model;
enum { CH_PICK = 1, CH_NONE };

/* asks (inside a hook, on the game's stack); the row picked, or -1 for the way out */
int choose_run(const char *title, const char *note, const ChooseRow *rows, int n, const char *none);

#ifdef __cplusplus
}
#endif
#endif
