/* repwatch.h - changes in the towns' opinion of the party, and what led to them (repwatch.cpp) */
#pragma once
#include <string>
#include <vector>
struct RepChange { int loc, delta, value; unsigned t; int day, month, year; std::string card, choice; };
extern "C" void repwatch_tick(void);                         /* (each frame) */
const RepChange *repwatch_last(int loc);          /* the latest change in that town, or none */
void repwatch_list(int loc, std::vector<RepChange> &out);   /* every change seen this session there, oldest first */
