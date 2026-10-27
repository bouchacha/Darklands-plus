/* alerts.h - news the player must read, a window to click through (ui/alerts.cpp); for the mods (C) too */
#ifndef DL_ALERTS_H
#define DL_ALERTS_H
#ifdef __cplusplus
extern "C" {
#endif
void ui_alert(const char *title, const char *body, const char *foot);
/* a member's changes, drawn as a row: the name, the age (an hourglass and the years; -1 not shown),
   each attribute or skill (key 0..6 the attributes, 7..25 the skills) as its shield, was and now */
typedef struct { int key, was, now; } UiAlertChange;
typedef struct { char name[40]; int age, n; UiAlertChange ch[26]; } UiAlertRow;
/* the same as ui_alert with the rows drawn in place of the body (which the log keeps) */
void ui_alert_rows(const char *title, const char *body, const UiAlertRow *rows, int nrows, const char *foot);
#ifdef __cplusplus
}
#endif
#endif
