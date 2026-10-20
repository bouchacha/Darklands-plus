// quests.h - the party's quests as rows (ui/quests.cpp reads them from the game's events), for
// the margin and the journal: a short title, the whole task, its deadline and how far it is.
#pragma once
#include <string>
#include <vector>
#include <utility>

struct ImDrawList; struct ImVec2;

/* what a quest asks for, for its picture (ui/questicons.cpp): the game's kinds of task (the
   merchants' fetching errands as one: a thing from a cave, tomb, lake ... or papers to be got in
   a town), a reward waiting, and a town's trouble with the party (not a task: shown at the town) */
enum QuestCat {
    QC_NONE, QC_ROBBER, QC_REWARD, QC_RETRIEVE, QC_PAPERS, QC_MINE, QC_DRAGON, QC_HUNT,
    QC_UNWELCOME, QC_WANTED, QC_SABBAT, QC_POLITICS, QC_WAR, QC_N
};
extern const char *QUEST_CAT_NAME[QC_N];
void ui_draw_quest_icon(ImDrawList *dl, ImVec2 c, float s, int cat);   // a roundel about s pixels across, centred on c

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
    int cat;                 // QuestCat
    int giver = -1;          // who waits for the party in the town (a reward): the game's giver kinds (0 a merchant, 4 a foreign trader,
                             // 5 a pharmacist, 6 the Medici, 7 the Hanse, 8 the Fugger ...), -1 none
    int office = -1;         // a thing kept in an office above the town's marketplace: 0 the leading merchant, 1 the Medici, 2 the Fugger, 3 the Hanse
};

enum { QSORT_DEADLINE, QSORT_NEAREST, QSORT_TAKEN, QSORT_N };
extern const char *QSORT_NAME[QSORT_N];

void quest_rows(std::vector<QuestRow> &out, int sort, bool all = false);   // tasks and rewards (all: rumours, town troubles and secrets too), sorted
void quest_sort(std::vector<QuestRow> &rows, int sort);
void quest_marks_cat(std::vector<std::pair<int, std::string>> &out, std::vector<int> &cats);
struct TownMark { int loc, cat; std::string until; };     // a town that bars the party (QC_UNWELCOME) or whose watch wants it (QC_WANTED)
void town_marks(std::vector<TownMark> &out);   // (place, its task) with each one's QuestCat
std::string quest_days(const QuestRow &r);                 // "37 days", "last day", "" (no deadline)
std::string quest_distance(const QuestRow &r);             // "here", "north-east, about 2 days away"
bool party_xy(double *x, double *y);      // the party on the travel map: the place it is at, else its tile (false: unknown)
