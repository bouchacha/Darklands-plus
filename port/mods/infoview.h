/* infoview.h - the party information screen as a native page (mods/infoview.c runs it,
 * ui/infoview.cpp draws it): fame, the time, where the party is, its wealth, and its name in
 * every city (what the original shows for one town at a time under the pointer on its map). */
#ifndef DL_INFOVIEW_H
#define DL_INFOVIEW_H
#ifdef __cplusplus
extern "C" {
#endif

#define IV_CITIES 92
typedef struct {
    int open;
    int fame; char fame_word[40];
    int hour, day, month, year; char bell[24], month_name[24];
    char where[80];                  /* the city, or "North of X" */
    int here;                        /* the city the party is in (-1 none), or the nearest */
    int in_city;
    int money[3]; long total_pf;
    int notes, phstone;
    struct { int rep; int band; char band_word[24]; int size; } city[IV_CITIES];
    unsigned seq;
} InfoModel;

extern InfoModel info_model;

enum { IV_CLOSE = 1 };

void infoview_init(void);
int infoview_is_on(void);
void infoview_set(int on);

#ifdef __cplusplus
}
#endif
#endif
