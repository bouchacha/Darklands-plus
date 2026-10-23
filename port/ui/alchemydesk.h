/* alchemydesk.h - where the alchemy screen's workbench puts things (ui/alchemydesk.cpp draws them,
 * ui/alchemyview.cpp writes on them), in game coordinates (x 60..320, y 0..200) */
#ifndef DL_ALCHEMYDESK_H
#define DL_ALCHEMYDESK_H
/* the formulary, open: the index on its left page, the chosen formula on its right */
#define AD_BOOK_X0 63.0f
#define AD_BOOK_X1 316.0f
#define AD_BOOK_Y0 20.0f
#define AD_BOOK_Y1 123.0f
#define AD_GUTTER 189.0f
/* the apothecary's chest, along the bottom: its lid's strip, then 8 x 3 compartments */
#define AD_CHEST_COLS 8
#define AD_CHEST_ROWS 3
#define AD_CHEST_X0 63.0f
#define AD_CHEST_X1 225.0f
#define AD_CHEST_LID 126.0f
#define AD_CHEST_Y0 134.0f
#define AD_CHEST_Y1 198.0f
/* the wax tablet: batches and the chance */
#define AD_TAB_X0 229.0f
#define AD_TAB_X1 316.0f
#define AD_TAB_Y0 127.0f
#define AD_TAB_Y1 170.0f
/* the furnace, and the Start button on its brass plate */
#define AD_FURN_X0 229.0f
#define AD_FURN_X1 316.0f
#define AD_FURN_Y0 173.0f
#define AD_FURN_Y1 198.0f
#define AD_START_X0 262.0f
#define AD_START_X1 311.0f
#define AD_START_Y0 178.0f
#define AD_START_Y1 193.0f
#endif
