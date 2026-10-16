/* cardview.h - the card screen drawn by Darklands+ itself (mods/cardview.c runs it inside the
 * game's card engine, ui/cardview.cpp draws it). */
#ifndef DL_CARDVIEW_H
#define DL_CARDVIEW_H
#ifdef __cplusplus
extern "C" {
#endif

#define CV_MAXOPT 20
#define CV_MAXPOP 20

/* one piece of the card's text, in order: a paragraph of the narrative or an option line.
 * Text is in the game's character set (ui converts it). */
typedef struct {
    int is_opt;
    int opt;                  /* option number (the script's index, opt_fn[opt]) */
    int type;                 /* 0 plain, 1 items list, 2 saints list, -1 a battle */
    int state;                /* 1 choosable, 2 shown but not choosable */
    int line;                 /* index among the choosable options (the game's menu line), -1 */
    char lead[48];            /* "..." before the words of an option */
    char text[700];
} CvPiece;

/* the members' saints or items for an option of type 1/2 (the original's pop-up box) */
typedef struct {
    int member, slot;         /* menu_member, menu_slot */
    int on;                   /* can be chosen */
    int code;                 /* the original's colour code: 0x80 normal, 0x87 automatic, 0x84 known, 0x83 none */
    int id;                   /* the saint, or the item */
    char who[24];             /* the member's name (first entry of a member) */
    char text[60];
} CvEntry;

typedef struct {
    int phase;                /* 0 not shown, 1 a text (waits to go on), 2 the option menu */
    int nested;               /* 1 a game screen runs inside (a member's sheet ...): hide; 2 the menu bar may open */
    unsigned nested_since;    /* when the menu bar's call began (a long one is a screen of its own: Load, Save ...) */
    int block, how, dark;     /* text block; card_text's how; light text on a dark picture */
    int x, y, w;              /* the block's box, game pixels (x from the left of the screen) */
    int dropcap;              /* the first letter, drawn large (0 none) */
    int npieces;
    CvPiece piece[48];
    int nlines;               /* choosable options */
    int hover;                /* piece under the pointer (-1) */
    int hover_entry;          /* pop-up entry under the pointer (-1) */
    int key_line;             /* highlighted with the keys (-1) */
    int popup_piece;          /* piece whose saints/items are listed (-1) */
    int popup_kind;
    int nentries;
    CvEntry entry[CV_MAXPOP];
    int want_popup;           /* ui -> engine: list this piece's saints/items (-1) */
    /* where things are, game pixels (set by the drawing each frame) */
    float row_y0[48], row_y1[48];
    float entry_y0[CV_MAXPOP], entry_y1[CV_MAXPOP];
    float box_x0, box_x1;
    unsigned seq;             /* changes with each new text */
} CardView;

extern CardView cardview;
extern int cardview_on;               /* the setting */
int cardview_active(void);            /* drawing the cards (setting on, a window, not "original") */
int cardview_showing(void);           /* something is on screen now */
int cardview_menu(void);              /* card_menu, natively (called from mods/assist.c's hook) */
const char *cardview_option_text(int opt);   /* the words of option opt of the last menu (game charset) */
void cardview_set(int on);
void cardview_init(void);

#ifdef __cplusplus
}
#endif
#endif
