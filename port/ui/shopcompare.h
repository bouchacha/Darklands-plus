/* shopcompare.h - party equipment comparisons on the shop counter. */
#pragma once

struct NsDraw;
struct NsStripThing;

/* Filled arrow for an improvement, hollow if every opportunity is an empty slot. */
int shop_upgrade_count(int id, int quality, bool &only_empty);
void shop_compare_strip(NsDraw &page, float top, const NsStripThing &ware);
