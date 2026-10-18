/* options.h - every switch Darklands+ adds, in one table: where it is kept (a config key), what it
 * is called and how it is explained, what kind of change it is, and its value in the original game,
 * in Darklands+ and in the Easy preset. The F12 pages draw the table (ui/settings.cpp) and the
 * presets set it whole (Buggy Purist ... Easy). No interface here: a host draws the table its own way.
 *
 * Adding a switch: one line in OPTIONS[] (options.c) - the key the feature reads, its section and
 * class, a short title and the description. The page, the presets and "Custom" follow by
 * themselves. A feature that keeps its value in a variable gives the line an apply function (or
 * the interface reloads it in its options listener). */
#ifndef DL_OPTIONS_H
#define DL_OPTIONS_H
#ifdef __cplusplus
extern "C" {
#endif

/* what kind of change a switch makes: decides the presets */
enum OptClass {
    OC_FIX = 1,     /* puts right a bug of the original */
    OC_SHOW,        /* draws a screen anew or tells more; what happens is the game's own */
    OC_EASE,        /* saves clicks and waiting: what the player could do by hand */
    OC_PLUS,        /* changes a rule a little, for less chore (one pack for the party ...) */
    OC_EASY,        /* makes the game easier: the Easy preset only */
    OC_EXPERIMENT,  /* not yet sure to work well: off in every preset */
    OC_TASTE,       /* a matter of taste (speed, typeface, pictures): as the original in the
                       purist presets, the player's own in the others */
    OC_TOOL,        /* a tool (the editor): no preset touches it */
};
enum {
    OF_ELSEWHERE = 1,   /* shown on another page (Display, Sound, Saves, Travel): the presets set it */
    OF_COMBO = 2,       /* a choice drawn as a drop-down list (else as buttons in a row) */
};

typedef struct Option {
    const char *key;            /* config key */
    const char *section;        /* the heading it is under ("Battles") */
    const char *parent;         /* the key of the switch it belongs to: shown under it, while that is on */
    const char *title;          /* short, bold: "Party panel" */
    const char *desc;           /* what it does, after the title's colon */
    int cls;                    /* enum OptClass */
    int orig, def, easy;        /* its value in the original game, in Darklands+, in Easy */
    const char *choices;        /* a choice: "label|label|...", the values in vals; NULL: on/off */
    const int *vals;
    void (*apply)(int v);       /* puts the value in force where a module keeps it in a variable */
    const char *note;           /* what it means for saved games (shown in amber), or NULL */
    int flags;                  /* OF_... */
    int arg;                    /* (fixes and built-in changes: their index) */
    void (*apply_i)(int arg, int v);
} Option;

int options_count(void);                    /* the table, then the fixes (FIXES[]) and the built-in changes (MODS[]) */
const Option *options_get(int i);
const Option *options_find(const char *key);
int option_value(const Option *o);
void option_set(const Option *o, int v);    /* config, apply, then the listener */
int option_nchoices(const Option *o);
const char *option_choice(const Option *o, int k, char *buf, int n);   /* label k of a choice */
/* the interface's listener: called after a switch changes (key), to reload what it keeps itself */
void options_set_listener(void (*fn)(const char *key, int v));

/* ---------------------------------------------------------------- presets */
enum { PRESET_CUSTOM = -1, PRESET_BUGGY_PURIST = 0, PRESET_PURIST, PRESET_PURIST_UI, PRESET_SEMI,
       PRESET_PLUS, PRESET_EASY, PRESET_COUNT };
typedef struct { const char *name, *desc, *saves; } Preset;
extern const Preset PRESETS[PRESET_COUNT];
#define PRESET_KEEP (-999999)                /* preset_value: the player's own (OC_TASTE, OC_TOOL) */
int preset_value(const Option *o, int preset);
int preset_differences(int preset);         /* how many switches differ from the preset (0: it is in force) */
int preset_current(void);                   /* the preset in force, or PRESET_CUSTOM */
int preset_nearest(int *diffs);             /* the preset with the fewest differences */
void preset_apply(int preset);              /* the player's own choices are kept first, to come back to */
int preset_custom_saved(void);              /* own choices kept (and not in force) */
void preset_restore_custom(void);

#ifdef __cplusplus
}
#endif
#endif
