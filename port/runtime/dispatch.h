#ifndef DL_DISPATCH_H
#define DL_DISPATCH_H
typedef struct { unsigned lin; int sect; void (*f)(void); } FuncEntry;
typedef struct { unsigned off; int sect; void (*f)(void); } VecEntry;
typedef struct { const char *name; const FuncEntry *funcs; int count; const unsigned *rets; int nrets; unsigned hash; } DriverImage;
extern const FuncEntry FUNC_TABLE[]; extern const int FUNC_COUNT;
extern const VecEntry VEC_TABLE[]; extern const int VEC_COUNT;
typedef struct { unsigned lin; int sect; } RetSite;
extern const RetSite RET_TABLE[]; extern const int RET_COUNT;
/* drivers loaded at run time (DOS 4B03): their functions are keyed by offset from the load segment */
void driver_register(const char *filename, unsigned short seg, unsigned size, unsigned hash);
extern int ovl_loaded[256];
#endif
