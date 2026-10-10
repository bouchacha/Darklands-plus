/* strops.h - x86 string instructions. rep: 0 none, 1 rep/repe, 2 repne */
#ifndef DL_STROPS_H
#define DL_STROPS_H
#include "cpu.h"
#define STEP(n) (R.df ? -(n) : (n))
static inline void str_movs8(int rep, u16 src) {
    if (rep) { while (CX) { wb(ES, DI, rb(src, SI)); SI += STEP(1); DI += STEP(1); CX--; } }
    else { wb(ES, DI, rb(src, SI)); SI += STEP(1); DI += STEP(1); }
}
static inline void str_movs16(int rep, u16 src) {
    if (rep) { while (CX) { ww(ES, DI, rw(src, SI)); SI += STEP(2); DI += STEP(2); CX--; } }
    else { ww(ES, DI, rw(src, SI)); SI += STEP(2); DI += STEP(2); }
}
static inline void str_stos8(int rep, u16 src) { (void)src;
    if (rep) { while (CX) { wb(ES, DI, AL); DI += STEP(1); CX--; } } else { wb(ES, DI, AL); DI += STEP(1); } }
static inline void str_stos16(int rep, u16 src) { (void)src;
    if (rep) { while (CX) { ww(ES, DI, AX); DI += STEP(2); CX--; } } else { ww(ES, DI, AX); DI += STEP(2); } }
static inline void str_lods8(int rep, u16 src) {
    do { AL = rb(src, SI); SI += STEP(1); } while (rep && --CX); }
static inline void str_lods16(int rep, u16 src) {
    do { AX = rw(src, SI); SI += STEP(2); } while (rep && --CX); }
static inline void str_scas8(int rep, u16 src) { (void)src;
    if (!rep) { f_sub(AL, rb(ES, DI), 8); DI += STEP(1); return; }
    while (CX) { f_sub(AL, rb(ES, DI), 8); DI += STEP(1); CX--; if (rep == 1 ? !get_zf() : get_zf()) break; } }
static inline void str_scas16(int rep, u16 src) { (void)src;
    if (!rep) { f_sub(AX, rw(ES, DI), 16); DI += STEP(2); return; }
    while (CX) { f_sub(AX, rw(ES, DI), 16); DI += STEP(2); CX--; if (rep == 1 ? !get_zf() : get_zf()) break; } }
static inline void str_cmps8(int rep, u16 src) {
    if (!rep) { f_sub(rb(src, SI), rb(ES, DI), 8); SI += STEP(1); DI += STEP(1); return; }
    while (CX) { f_sub(rb(src, SI), rb(ES, DI), 8); SI += STEP(1); DI += STEP(1); CX--; if (rep == 1 ? !get_zf() : get_zf()) break; } }
static inline void str_cmps16(int rep, u16 src) {
    if (!rep) { f_sub(rw(src, SI), rw(ES, DI), 16); SI += STEP(2); DI += STEP(2); return; }
    while (CX) { f_sub(rw(src, SI), rw(ES, DI), 16); SI += STEP(2); DI += STEP(2); CX--; if (rep == 1 ? !get_zf() : get_zf()) break; } }
static inline void str_outs8(int rep, u16 src) {
    do { if (rep && !CX) break; hle_out8(DX, rb(src, SI)); SI += STEP(1); if (rep) CX--; } while (rep && CX); }
static inline void str_outs16(int rep, u16 src) {
    do { if (rep && !CX) break; hle_out8(DX, rb(src, SI)); hle_out8((u16)(DX + 1), rb(src, (u16)(SI + 1))); SI += STEP(2); if (rep) CX--; } while (rep && CX); }
static inline void str_ins8(int rep, u16 src) { (void)src;
    do { if (rep && !CX) break; wb(ES, DI, hle_in8(DX)); DI += STEP(1); if (rep) CX--; } while (rep && CX); }
static inline void str_ins16(int rep, u16 src) { (void)src;
    do { if (rep && !CX) break; wb(ES, DI, hle_in8(DX)); wb(ES, (u16)(DI + 1), hle_in8((u16)(DX + 1))); DI += STEP(2); if (rep) CX--; } while (rep && CX); }
#endif
