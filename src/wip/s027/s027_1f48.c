/* place: s027 1f48:0000 */
/* orig: size=0d42 CONST=7e14+0004 */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* Overlay s027: named swap blocks ("heap/swap").  A block of data is saved under a name
   into one of the swap stores and read back later.  Store types (D_8d4 / entry.type):
     1 = conventional memory, 2 = XMS (s026 1ef8), 3 = EMS (s026 1f1d),
     5/6/7 = temporary files (s026 1f09), 8 = none.
   The directory is a table of 100 8-byte entries (0x320 bytes) kept at D_8ca (D_8ce
   when loaded).  Every block in a store starts with a 0x18-byte header (struct hdr) kept
   in far data at 2e38:001e; the entry found last is at 2e38:0036.
   Functions 0a78, 0b04, 0ba8 and 0c4a contain inline _asm blocks (DOS int 21h calls) in
   the original, written the same way here: with D_8d6 & 0xff00 (no swap store could be set
   up, s028) blocks are plain files called name.
   status (m3, 2026-10-09; w4: 18/21 with 05fe): 17/21 match (m3: 0000, 09aa, 015a/00a2 jump tables).
   - 0000/076c: found/hash/tbl in one struct `s` (aggregate, first member lowest) gives the
     original slots; `for (i = 0, off = 0; ..)` gives off in SI; pragma "a" gives `or dx,ax`
     on the returned table pointer.  076c left: the second tbl[i].off read (6.00 recomputes
     [bp-22h]+tbl, 6.00A CSEs it; a volatile read of tbl gives es:[bx+si+4], 8 lines).
   - 09aa: `if (!0000(name)) return r;` + `break` in the switch arms.
   - 0298: return-0 sites as `goto fail` at the end; type/hash in a struct for the slots;
     left: 6.00A keeps the free-entry pointer e in SI in the search loop (orig: bx, stored
     after the bound check).
   - 0480: rewritten from the asm (frame right); 6.00 puts the two `if` bodies of the loop
     (shift and store switch) and the first `return D_c099` out of line - 6.00A lays the
     blocks out in source order whatever the source (gotos give identical code).
   - 05fe MATCHED (w4, 18/21): the long compare direction follows the symbol-table position:
     13 unused externs (D_ffe0..) in front of it. */
#include "darklands/card.h"
#include "darklands/game.h"

struct hdr {                    /* block header, 0x18 bytes */
    int type;                   /* +00 1 = used, 2 = free */
    int hash;                   /* +02 */
    char name[16];              /* +04 */
    long size;                  /* +14 */
};
struct ent {                    /* directory entry, 8 bytes */
    int hash;                   /* +00 0 = unused */
    int type;                   /* +02 store */
    long off;                   /* +04 offset of the block header in the store */
};
struct store {                  /* per store type, 10 bytes (swap_stores[type], as in s028) */
    int handle;
    long size;                  /* end of the store (end marker at size - 0x18) */
    long base;                  /* first block (after the directory, if held here) */
};

int far _fstrcmp(const char far *, const char far *);
void far *far _fmemcpy(void far *, const void far *, unsigned);
void far *far _fmemset(void far *, int, unsigned);
char far *far _fstrcpy(char far *, const char far *);
int far xms_read(long, void far *, int, long);
int far xms_write(long, void far *, int, long);
int far swapfile_write(long, void far *, int, long);
int far swapfile_read(long, void far *, int, long);
int far ems_write(long, void far *, int, long);
int far ems_read(long, void far *, int, long);
void far swap_init_vec(void);

extern struct hdr D_8fe;               /* an empty header */
extern int D_8d4;                       /* store type of the directory */
extern struct ent far *D_8ca;           /* directory buffer */
extern struct ent far *D_8ce;           /* directory, when loaded */
extern unsigned char D_8d6, D_8d7;
extern int D_8d8;
extern int D_8da;                       /* current store type */
extern long D_8dc[];                    /* per store: offset of the free block */
extern int D_c099;
extern struct store swap_stores[];
extern int (far *D_2674)(long);         /* allocate a block: returns the store type */
extern int D_2678;
extern struct hdr far F_2e38_001e;      /* header of the current block */
extern struct ent far F_2e38_0036;      /* entry of the current block */

int far swap_hash(char *name);
struct ent far *far sub_s027_1f48_00a2(void);
struct ent far *far sub_s027_1f48_015a(void);
int far sub_s027_1f48_020e(struct ent far *e);
int far sub_s027_1f48_0238(struct ent far *e);
int far swap_create(long size, char *name);
long far sub_s027_1f48_05fe(long size);
int far sub_s027_1f48_076c(char *name);
int far sub_s027_1f48_0876(long len, void far *buf, int type, long off);
int far sub_s027_1f48_0910(long len, void far *buf, int type, long off);
int far sub_s027_1f48_09aa(char *name, long size);
int far sub_s027_1f48_0a14(long size);

/* find the block called name: reads its header into F_2e38_001e and its entry into
   F_2e38_0036; 1 when found */
#pragma optimize("a", on)
int far sub_s027_1f48_0000(char *name)
{
    struct { struct ent far *tbl; int hash; int found; } s;
    struct hdr h;
    int i;
    int off;

    h = D_8fe;
    *(volatile int *)&s.found = 0;
    s.hash = swap_hash(name);
    s.tbl = sub_s027_1f48_00a2();
    if (s.tbl != 0) {
        for (i = 0, off = 0; off < 0x320; i++, off += 8) {
            if (*(int far *)((char far *)s.tbl + off) == s.hash) {
                if (!sub_s027_1f48_020e((struct ent far *)((char far *)s.tbl + off)))
                    return 0;
                if (_fstrcmp(name, F_2e38_001e.name) == 0) {
                    _fmemcpy(&F_2e38_0036, &s.tbl[i], 8);
                    return 1;
                }
            }
        }
    }
    return 0;
}

/* load the directory; returns it (0 when there is none) */
#pragma optimize("a", off)
struct ent far *far sub_s027_1f48_00a2(void)
{
    struct ent far *p = 0;

    switch (D_8d4) {
    case 1:
        p = D_8ce;
        break;
    case 2:
        xms_read(0x320L, D_8ca, swap_stores[2].handle, (long)p);
        D_8ce = p = D_8ca;
        break;
    case 3:
        ems_read(0x320L, D_8ca, swap_stores[3].handle, (long)p);
        D_8ce = p = D_8ca;
        break;
    case 5:
        swapfile_read(0x320L, D_8ca, swap_stores[5].handle, (long)p);
        D_8ce = p = D_8ca;
        break;
    case 6:
        swapfile_read(0x320L, D_8ca, swap_stores[6].handle, (long)p);
        D_8ce = p = D_8ca;
        break;
    case 7:
        swapfile_read(0x320L, D_8ca, swap_stores[7].handle, (long)p);
        D_8ce = p = D_8ca;
        break;
    case 8:
        D_8ce = p = (struct ent far *)0L;
        break;
    }
    return p;
}

/* write the directory back */
struct ent far *far sub_s027_1f48_015a(void)
{
    struct ent far *r = 0;

    switch (D_8d4) {
    case 1:
        r = D_8ce;
        break;
    case 2:
        xms_write(0x320L, D_8ca, swap_stores[2].handle, (long)r);
        break;
    case 3:
        ems_write(0x320L, D_8ca, swap_stores[3].handle, (long)r);
        break;
    case 5:
        swapfile_write(0x320L, D_8ca, swap_stores[5].handle, (long)r);
        break;
    case 6:
        swapfile_write(0x320L, D_8ca, swap_stores[6].handle, (long)r);
        break;
    case 7:
        swapfile_write(0x320L, D_8ca, swap_stores[7].handle, (long)r);
        break;
    case 8:
        D_8ce = r = (struct ent far *)0L;
        break;
    }
    return r;
}

/* read the header of entry e */
int far sub_s027_1f48_020e(struct ent far *e)
{
    return sub_s027_1f48_0910(0x18L, &F_2e38_001e, e->type, e->off);
}

/* write the header of entry e */
int far sub_s027_1f48_0238(struct ent far *e)
{
    return sub_s027_1f48_0876(0x18L, &F_2e38_001e, e->type, e->off);
}

/* hash of a block name */
int far swap_hash(char *name)
{
    int h = 1;
    int i = 0;

    while (*name) {
        h += *name << i % 16;
        i++;
        name++;
    }
    return h;
}

/* create (or resize) the block called name with size bytes; 1 on success */
int far swap_create(long size, char *name)
{
    long old;
    struct ent far *e;
    struct { int hash; int type; } k;

    if (size & 1)
        size++;
    if (sub_s027_1f48_0000(name)) {
        if (F_2e38_001e.size == size)
            return 1;
        if (sub_s027_1f48_09aa(name, size) == 1)
            return 1;
        sub_s027_1f48_076c(name);
    }
    k.type = (*D_2674)(size);
    if (!k.type)
        goto fail;
    k.hash = swap_hash(name);
    e = D_8ce - 1;
    do
        e++;
    while (e->hash != 0);
    if (e - D_8ce >= 100)
        goto fail;
    e->hash = k.hash;
    e->type = k.type;
    e->off = F_2e38_0036.off;
    F_2e38_0036 = *e;
    sub_s027_1f48_015a();
    old = F_2e38_001e.size;
    F_2e38_001e.type = 2;
    *(volatile int far *)&F_2e38_001e.hash = 0;
    _fmemset(F_2e38_001e.name, 0, 16);
    F_2e38_001e.size = old - size - 0x18;
    sub_s027_1f48_0876(0x18L, &F_2e38_001e, k.type, e->off + size + 0x18);
    D_8dc[k.type] = e->off + size + 0x18;
    F_2e38_001e.type = 1;
    F_2e38_001e.hash = k.hash;
    _fstrcpy(F_2e38_001e.name, name);
    F_2e38_001e.size = size;
    sub_s027_1f48_0876(0x18L, &F_2e38_001e, k.type, e->off);
    return 1;
fail:
    return 0;
}

/* find a store with a free block of size bytes (the default D_2674); returns its type,
   0 when none */
int far sub_s027_1f48_0480(long size)
{
    long r;
    int bit;
    int next;
    int start;
    int type;
    int k;

    size += 0x18;
    if (D_8d6 & 1)
        swap_init_vec();
    D_c099 = D_8da > 7 ? 7 : D_8da;
    r = sub_s027_1f48_05fe(size);
    if (r != -1L) {
        F_2e38_0036.off = r;
        return D_c099;
    }
    D_8dc[D_c099] = swap_stores[D_c099].size - 0x18;
    type = 2;
    next = type;
    bit = 1;
    start = D_c099;
    do {
        if (*(volatile unsigned *)&D_8d6 & bit) {
            D_8da = type;
            swap_init_vec();
            type = D_8da;
        }
        if (type != next) {
            k = type - next;
            D_8da = type;
            do
                bit <<= 1;
            while (--k);
            type = D_8da;
        }
        if (D_8d8 & bit) {
            D_c099 = type;
            if (start != type) {
                D_8da = type;
                D_8dc[type] = swap_stores[type].base;
                r = sub_s027_1f48_05fe(size);
                if (r != -1L) {
                    F_2e38_0036.off = r;
                    return D_8da;
                }
                type = D_8da;
                D_8dc[type] = swap_stores[type].size - 0x18;
            }
        }
        type++;
        next = type;
        bit <<= 1;
    } while (r == -1L && type < 8);
    D_8da = type;
    return 0;
}

/* search the current store (D_c099) for a free block of at least size bytes, merging
   neighbouring free blocks; returns its offset or -1 */
/* w4: 13 unused externs: symbol-table position for the long compares below (size vs h.size) */
extern int D_ffe0;
extern int D_ffe1;
extern int D_ffe2;
extern int D_ffe3;
extern int D_ffe4;
extern int D_ffe5;
extern int D_ffe6;
extern int D_ffe7;
extern int D_ffe8;
extern int D_ffe9;
extern int D_ffe10;
extern int D_ffe11;
extern int D_ffe12;
long far sub_s027_1f48_05fe(long size)
{
    long off;
    struct hdr h;
    struct hdr h2;
    int merged;

    off = D_8dc[D_c099];
    h = D_8fe;
    h2 = D_8fe;
    size += 0x18;
    do {
        sub_s027_1f48_0910(0x18L, &h, D_c099, off);
        if (h.type != 2) {
            off += h.size + 0x18;
        } else {
            if (size <= h.size)
                break;
            merged = 0;
            do {
                sub_s027_1f48_0910(0x18L, &h2, D_c099, h.size + off + 0x18);
                if (h2.type == 2) {
                    h.size += h2.size + 0x18;
                    merged = 1;
                }
            } while (h2.type == 2);
            if (merged)
                sub_s027_1f48_0876(0x18L, &h, D_c099, off);
            if (size <= h.size)
                break;
            off += h2.size + h.size + 0x30;
        }
    } while (h.size != -1L && h2.size != -1L);
    if (size > h.size)
        return -1L;
    _fmemcpy(&F_2e38_001e, &h, 0x18);
    return off;
}

/* free the block called name; 1 on success */
#pragma optimize("a", on)
int far sub_s027_1f48_076c(char *name)
{
    struct { struct ent far *tbl; int hash; int found; } s;
    struct hdr h;
    int i;
    int off;

    h = D_8fe;
    *(volatile int *)&s.found = 0;
    s.hash = swap_hash(name);
    s.tbl = sub_s027_1f48_00a2();
    if (!s.tbl)
        return 0;
    for (i = 0, off = 0; off < 0x320; i++, off += 8) {
        if (*(int far *)((char far *)s.tbl + off) == s.hash) {
            if (!sub_s027_1f48_020e((struct ent far *)((char far *)s.tbl + off)))
                return 0;
            if (_fstrcmp(name, F_2e38_001e.name) == 0) {
                s.tbl[i].hash = 0;
                F_2e38_001e.type = 2;
                sub_s027_1f48_0238(&s.tbl[i]);
                sub_s027_1f48_015a();
                if (D_2678) {
                    D_8da = s.tbl[i].type;
                    if (D_8dc[D_8da] > s.tbl[i].off)
                        D_8dc[D_8da] = (&(*(struct ent far * volatile *)&s.tbl)[i])->off;
                }
                return 1;
            }
        }
    }
    return 0;
}

/* write len bytes from buf at offset off of store type */
#pragma optimize("a", off)
int far sub_s027_1f48_0876(long len, void far *buf, int type, long off)
{
    int r = 0;

    switch (type) {
    case 2:
        r = xms_write(len, buf, swap_stores[2].handle, off);
        break;
    case 3:
        r = ems_write(len, buf, swap_stores[3].handle, off);
        break;
    case 5:
        r = swapfile_write(len, buf, swap_stores[5].handle, off);
        break;
    case 6:
        r = swapfile_write(len, buf, swap_stores[6].handle, off);
        break;
    case 7:
        r = swapfile_write(len, buf, swap_stores[7].handle, off);
        break;
    }
    return r;
}

/* read len bytes into buf from offset off of store type */
int far sub_s027_1f48_0910(long len, void far *buf, int type, long off)
{
    int r = 0;

    switch (type) {
    case 2:
        r = xms_read(len, buf, swap_stores[2].handle, off);
        break;
    case 3:
        r = ems_read(len, buf, swap_stores[3].handle, off);
        break;
    case 5:
        r = swapfile_read(len, buf, swap_stores[5].handle, off);
        break;
    case 6:
        r = swapfile_read(len, buf, swap_stores[6].handle, off);
        break;
    case 7:
        r = swapfile_read(len, buf, swap_stores[7].handle, off);
        break;
    }
    return r;
}

/* grow the block called name in place to size bytes */
int far sub_s027_1f48_09aa(char *name, long size)
{
    register int r = 0;

    if (!sub_s027_1f48_0000(name))
        return r;
    switch (F_2e38_0036.type) {
    case 2:
        D_c099 = 2;
        r = sub_s027_1f48_0a14(size);
        break;
    case 3:
        D_c099 = 3;
        r = sub_s027_1f48_0a14(size);
        break;
    case 5:
        D_c099 = 5;
        r = sub_s027_1f48_0a14(size);
        break;
    case 6:
        D_c099 = 6;
        r = sub_s027_1f48_0a14(size);
        break;
    case 7:
        D_c099 = 7;
        r = sub_s027_1f48_0a14(size);
        break;
    default:
        break;
    }
    return r;
}

int far sub_s027_1f48_0a14(long size)
{
    struct hdr h2;
    struct hdr h3;
    struct hdr h;

    h2 = D_8fe;
    h3 = D_8fe;
    h = D_8fe;
    h = F_2e38_001e;
    sub_s027_1f48_076c(h.name);
    if (swap_create(size, h.name))
        return 1;
}

/* save n bytes from buf under name; with D_8d6 & 0xff00 (no swap store) into a file
   called name instead (inline _asm, int 21h create/write/close) */
int far swap_save(long n, void far *buf, char *name)
{
    int oki;
    unsigned cntl;
    int fhd;
    int padk;

    oki = 0;
    cntl = (unsigned)n;
    if (*(int *)&D_8d6 & 0xff00) {
        _asm {
            push ds
            mov dx, name
            mov ah, 3Ch
            mov cx, 0
            push ss
            pop ds
            int 21h
            pop ds
            jb done
            mov fhd, ax
            mov bx, ax
            mov cx, cntl
            push ds
            lds dx, buf
            mov ah, 40h
            int 21h
            pop ds
            jb done
            mov oki, 1
            mov bx, fhd
            mov ah, 3Eh
            int 21h
        }
    done:
        return oki;
    }
    if (!swap_create(n, name))
        return oki;
    oki = sub_s027_1f48_0876(n, buf, F_2e38_0036.type, F_2e38_0036.off + 0x18);
    return oki;
}

/* load n bytes of the block (or file) name into buf */
int far swap_load_n(long n, void far *buf, char *name)
{
    int okh;
    int fhc;
    unsigned cntk;

    okh = 0;
    cntk = (unsigned)n;
    if (*(int *)&D_8d6 & 0xff00) {
        _asm {
            push ds
            mov dx, name
            mov al, 0
            push ss
            pop ds
            mov ah, 3Dh
            int 21h
            pop ds
            jb done
            mov fhc, ax
            mov bx, ax
            mov cx, cntk
            push ds
            lds dx, buf
            mov ah, 3Fh
            int 21h
            pop ds
            jb done
            mov bx, fhc
            mov ah, 3Eh
            int 21h
            jb done
            mov okh, 1
        }
    done:
        return okh;
    }
    if (!sub_s027_1f48_0000(name))
        return okh;
    if (F_2e38_001e.size >= n)
        okh = sub_s027_1f48_0910(n, buf, F_2e38_0036.type, F_2e38_0036.off + 0x18);
    goto done;
}

/* load the whole block (or file, at most 0xffff bytes) name into buf; its size or 0 */
int far swap_load(void far *buf, char *name)
{
    int okh;
    int fhc;
    unsigned cntk;

    okh = 0;
    cntk = 0xffff;
    if (*(int *)&D_8d6 & 0xff00) {
        _asm {
            push ds
            mov dx, name
            mov al, 0
            push ss
            pop ds
            mov ah, 3Dh
            int 21h
            pop ds
            jb done
            mov fhc, ax
            mov bx, ax
            mov cx, cntk
            push ds
            lds dx, buf
            mov ah, 3Fh
            int 21h
            pop ds
            jb done
            mov bx, fhc
            mov ah, 3Eh
            int 21h
            jb done
            mov okh, 1
        }
    done:
        return okh;
    }
    if (!sub_s027_1f48_0000(name))
        return okh;
    if ((okh = sub_s027_1f48_0910(F_2e38_001e.size, buf, F_2e38_0036.type, F_2e38_0036.off + 0x18)) == 1)
        return (int)F_2e38_001e.size;
    return 0;
}

/* load the whole block (or file) name into buf and free (delete) it; its size or 0 */
int far swap_load_free(void far *buf, char *name)
{
    int okh;
    int fhc;
    unsigned cntk;

    okh = 0;
    cntk = 0xffff;
    if (*(int *)&D_8d6 & 0xff00) {
        _asm {
            push ds
            mov dx, name
            mov al, 0
            push ss
            pop ds
            mov ah, 3Dh
            int 21h
            pop ds
            jb done
            mov fhc, ax
            mov bx, ax
            mov cx, cntk
            push ds
            lds dx, buf
            mov ah, 3Fh
            int 21h
            pop ds
            jb done
            mov bx, fhc
            mov ah, 3Eh
            int 21h
            push ds
            mov dx, name
            push ss
            pop ds
            mov ah, 41h
            int 21h
            pop ds
            jb done
            mov okh, 1
        }
    done:
        return okh;
    }
    if (!sub_s027_1f48_0000(name))
        return okh;
    if ((okh = sub_s027_1f48_0910(F_2e38_001e.size, buf, F_2e38_0036.type, F_2e38_0036.off + 0x18)) == 1
        && sub_s027_1f48_076c(name))
        return (int)F_2e38_001e.size;
    return 0;
}

/* entry points with the arguments reordered */
int far swap_save2(char *name, void far *buf, long n)
{
    return swap_save(n, buf, name);
}

int far sub_s027_1f48_0d1e(char *name, void far *buf)
{
    return swap_load_free(buf, name);
}

int far swap_load2(char *name, void far *buf)
{
    return swap_load(buf, name);
}
