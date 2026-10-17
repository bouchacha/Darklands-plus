// quests.h - the party's quests as rows (ui/quests.cpp reads them from the game's events), for
// the margin and the journal: a short title, the whole task, its deadline and how far it is.
#pragma once
#include <string>
#include <vector>

struct QuestRow {
    std::string id;          // stays the same while the quest lasts (its open / closed state)
    std::string title;       // "Raubritter @ Altenberg", "Debt Note from Lübeck"
    std::string text;        // the whole task: who wants it, where
    std::string when;        // "by 23 November 1401", "until ...", ""
    int group;               // 0 tasks, 1 rewards waiting, 2 rumours, 3 towns, 4 secrets
    int loc;                 // where to go (-1 none)
    int days_left;           // to the deadline (-1 none; 0 the last day)
    double dist;             // from the party, map units (-1 unknown)
    std::string dir;         // "north-east" (from the party), "" here or unknown
    int order;               // as the game keeps them (the oldest first, roughly)
};

enum { QSORT_DEADLINE, QSORT_NEAREST, QSORT_TAKEN, QSORT_N };
extern const char *QSORT_NAME[QSORT_N];

void quest_rows(std::vector<QuestRow> &out, int sort, bool all = false);   // tasks, rewards and town troubles (all: rumours and secrets too), sorted
void quest_sort(std::vector<QuestRow> &rows, int sort);
std::string quest_days(const QuestRow &r);                 // "37 days", "last day", "" (no deadline)
std::string quest_distance(const QuestRow &r);             // "here", "north-east, about 2 days away"
