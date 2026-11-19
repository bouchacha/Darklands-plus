/* Battle scenery, shared by the replacement-art hooks. */
#ifndef DL_BATTLETILES_H
#define DL_BATTLETILES_H
#include "../runtime/cpu.h"
void battletiles_init(void);
void battletiles_compare(void);
int battletiles_art_enabled(void);
void battletiles_snapshot(void);
int battletiles_sprite(u16 win, int x, int y, u16 seg);
void battletiles_copy(int src, int sx, int sy, int dst, int dx, int dy, int w, int h);
int battletiles_pick(int x, int y, int *px, int *py, int *pw, int *ph);
void battletiles_colour(u32 *rgb, const u8 *idx);
void battletiles_templates(void);
int hdart_register_tile(const char *name, int w, int h, const u8 *pixels);
#endif
