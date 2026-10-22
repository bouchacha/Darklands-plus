#ifndef LOSAIM_H
#define LOSAIM_H
#ifdef __cplusplus
extern "C" {
#endif
enum { LOS_WALL = 1, LOS_THING = 2, LOS_PERSON = 3 };
typedef struct { int x0, y0, x1, y1, kind, id; } LosBox;   /* map pixels; id: furniture id or combatant */
/* the boxes across the line from combatant a to b, as line_blocked (s014 1d69:000c) tests it
   (mode 0x80 a shot, 0x100 a throw): none, a clear line */
int los_trace(int a, int b, int mode, LosBox *out, int max);
/* the member about to shoot (0x80) or throw (0x100) at the foe under the pointer: 0 none */
int orders_aim(int *who, int *foe);
#ifdef __cplusplus
}
#endif
#endif
