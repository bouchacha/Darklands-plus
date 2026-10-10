/* travel.h - auto-travel on the overland map (travel.c) */
#ifndef DL_TRAVEL_H
#define DL_TRAVEL_H
#ifdef __cplusplus
extern "C" {
#endif
#define TRAVEL_MAXROUTE 4096
typedef struct {
    int active, replan;
    int loc, tx, ty;                  /* destination: location index (-1 = a map tile) and tile */
    int n, i;                         /* route length, next route tile */
    int rx[TRAVEL_MAXROUTE], ry[TRAVEL_MAXROUTE];
    int minutes, left;                /* whole route, still to go */
    char name[48];
} TravelState;
extern TravelState travel;
extern int travel_enabled, travel_fast;
void travel_init(void);
void travel_set(int on, int fast);
int travel_minutes(int x, int y);             /* from the party, -1 unreachable / not on the map */
int travel_minutes_loc(int loc, int *gx, int *gy);
int travel_to_tile(int x, int y, int loc, const char *name);
int travel_to_loc(int loc, const char *name);
void travel_stop(void);
void travel_party_tile(int *x, int *y);
int travel_terrain(int x, int y);
#ifdef __cplusplus
}
#endif
#endif
