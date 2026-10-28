// boattrip.h - by boat from the world map (boattrip.cpp): the way by boat from the town the party is in
// to another, how long it may take against the way by land, and the journey itself: walked to the docks,
// a stay at the inn until the boat sails, aboard, and on from stop to stop while the boats sail on.
#pragma once

struct BoatPlan {
    int ok = 0;
    int from = -1, to = -1;
    int legs[8] = {}, nlegs = 0;     // the towns after `from`, the last `to`
    int wait_days = 0;               // until the first boat sails (a stay at the inn)
    float days = 0, lo = 0, hi = 0;  // the boat's time: mean, and most journeys between lo and hi (days)
    float land_days = -1;            // the way by land from the town's gate, -1 none
    float aboard = 0;                // the share of journeys that stay aboard to the end (no stay ashore on the way)
};
/* the best way by boat from town `from` to town `to` (cities with docks); ok 0 when none */
BoatPlan boattrip_plan(int from, int to);
/* the boat saves at least a day over the way by land */
static inline bool boattrip_worth(const BoatPlan &p) { return p.ok && (p.land_days < 0 || p.days + 1.0f <= p.land_days); }
void boattrip_start(const BoatPlan &p);
extern "C" int boattrip_active(void);
extern "C" void boattrip_stop(const char *why);
extern "C" void boattrip_tick(void);
