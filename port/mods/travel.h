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
extern int travel_step_result;          /* the last step on the map: 1000 a random encounter, 2000 a place's own, <92 a city ... */
void travel_init(void);
void travel_set(int on, int fast);
int travel_minutes(int x, int y);             /* from the party, -1 unreachable / not on the map */
int travel_minutes_loc(int loc, int *gx, int *gy);
int travel_minutes_loc_from(int ox, int oy, int loc);   /* from tile (ox, oy): -1 no way */
int travel_to_tile(int x, int y, int loc, const char *name);
int travel_to_loc(int loc, const char *name);
void travel_after_leaving(int loc, const char *name);   /* set off for loc once the party is out on the travel map */
void travel_stop(void);
int travel_boat_way(int a, int b, const short **xs, const short **ys);   /* the tiles a boat from town a to b sails by (water), 0 none */
void travel_party_tile(int *x, int *y);
int travel_map_live(void);                   /* the overland loop is running (no card or screen over the map) */
int travel_terrain(int x, int y);
#ifdef __cplusplus
}
#endif
#endif
