/* alchemydesk.h - where the alchemy screen's workbench puts things (ui/alchemydesk.cpp draws them,
 * ui/alchemyview.cpp writes on them), in game coordinates (x 60..320, y 0..200) */
#ifndef DL_ALCHEMYDESK_H
#define DL_ALCHEMYDESK_H
/* the formulary, open: the index on its left page, the chosen formula on its right */
#define AD_BOOK_X0 63.0f
#define AD_BOOK_X1 237.0f
#define AD_BOOK_Y0 22.0f
#define AD_BOOK_Y1 160.0f
#define AD_GUTTER 150.0f
/* the apothecary's chest: its lid's strip, then 3 x 8 compartments */
#define AD_CHEST_X0 241.0f
#define AD_CHEST_X1 316.0f
#define AD_CHEST_LID 20.0f
#define AD_CHEST_Y0 30.0f
#define AD_CHEST_Y1 162.0f
/* the wax tablet: batches and the chance */
#define AD_TAB_X0 63.0f
#define AD_TAB_X1 200.0f
#define AD_TAB_Y0 166.0f
#define AD_TAB_Y1 198.0f
/* the furnace, and the Start button on its brass plate */
#define AD_FURN_X0 205.0f
#define AD_FURN_X1 316.0f
#define AD_FURN_Y0 167.0f
#define AD_FURN_Y1 198.0f
#define AD_START_X0 240.0f
#define AD_START_X1 310.0f
#define AD_START_Y0 174.0f
#define AD_START_Y1 189.0f
#endif
