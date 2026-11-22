#ifndef DL_AFTERMATH_H
#define DL_AFTERMATH_H

#ifdef __cplusplus
extern "C" {
#endif
void aftermath_init(void);
void aftermath_begin(void);
void aftermath_end(void);
void aftermath_open(void);                         /* build the report for the loot screen now opening */
void aftermath_armor_worn(int m, int id, int before, int after, int count);
int aftermath_victory_pending(void);                /* the victory report will include skill practice */
#ifdef __cplusplus
}

struct NsDraw;
float aftermath_draw(NsDraw &p);                    /* height added above the two loot lists */
#endif
#endif
