/* uitabs.h - the pages of the Darklands+ window (F12), by number: ui_open_tab(page), and the
 * test command utab:N. A page inside another's tabs: UI_PAGE_SUB(page, sub), as the Journal's
 * Quests (its fourth). The window groups some pages under one tab (Settings: Helpers, Mods,
 * Display, About); the numbers stay the same. */
#ifndef DL_UITABS_H
#define DL_UITABS_H
enum {
    UI_PAGE_JOURNAL = 0, UI_PAGE_SAVES, UI_PAGE_HELPERS, UI_PAGE_MODS, UI_PAGE_DISPLAY, UI_PAGE_RECORDS, UI_PAGE_ABOUT,
    UI_PAGE_EDITOR, UI_PAGE_GUIDE, UI_PAGE_RESTORED, UI_PAGE_N
};
/* (the party's things, skills, saints and formulae are on the character sheet, the journeys on the
   world map: the game's own screens, not pages here) */
#define UI_PAGE_SUB(page, sub) ((page) + 100 * ((sub) + 1))
#define UI_JOURNAL_QUESTS 3                      /* (the Journal's Quests tab) */
#endif
