/* ovl.c - RTLink/Plus overlay sections: loaded into memory on demand, as RTLink does.
 * The code itself is native (recompiled); loading the section bytes keeps code-segment data,
 * relocated segment values and "which sibling overlay is present" faithful. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cpu.h"
#include "dispatch.h"
#include "hle.h"

typedef struct { u16 load_seg, file_para_lo, file_para_hi_flags, mem_paras, nrelocs, parent, sect_id, file_paras; } Sect;
static Sect sect[256];
static int nsect;
int ovl_loaded[256];
static u32 ovl_gen;                         /* counts loads (ovl_restore: nothing to do when none happened) */
static FILE *exef;
static u32 sect_file_off[256];
static u8 *sect_file[256];                  /* each section's relocations and bytes as in the EXE, once read */

static u16 le16(const u8 *p) { return (u16)(p[0] | (p[1] << 8)); }

void ovl_init(const char *exe_path)
{
    exef = fopen(exe_path, "rb");
    if (!exef) fatal("cannot open %s", exe_path);
    u8 h[32];
    fread(h, 1, 32, exef);
    u32 hdr = le16(h + 8) * 16u;
    /* RTLink section table: manager segment 09c0, count word at +0b74, table at +0b7a */
    u32 tab = hdr + 0x9c00 + 0xb7a, cnt = hdr + 0x9c00 + 0xb74;
    u8 b[2];
    fseek(exef, cnt, SEEK_SET); fread(b, 1, 2, exef);
    nsect = le16(b);
    if (nsect <= 0 || nsect > 256) fatal("bad section count %d", nsect);
    for (int i = 0; i < nsect; i++) {
        u8 e[18];
        fseek(exef, tab + 18 * i, SEEK_SET); fread(e, 1, 18, exef);
        Sect *s = &sect[i];
        s->load_seg = le16(e); s->file_para_lo = le16(e + 4); s->file_para_hi_flags = le16(e + 6);
        s->mem_paras = le16(e + 8); s->nrelocs = le16(e + 10); s->parent = le16(e + 12);
        s->sect_id = le16(e + 14); s->file_paras = le16(e + 16);
        sect_file_off[i] = (s->file_para_lo | ((u32)(s->file_para_hi_flags & 0xff) << 16)) * 16u;
    }
}

/* An interrupt handler may call into an overlay (the game's timer tick runs a card-section
 * routine, s043, through the RTLink manager). RTLink reloads the interrupted code's section
 * when the handler returns; here the overlay area is saved before the first load inside a
 * handler and put back afterwards (with which sections are present), so the interrupted
 * code finds its own section, data and all, as it left it. Without this, pictures decoded
 * while a timer tick hit came out as garbage (white blocks on the travel map, grey blocks
 * on cards). */
static int irq_depth; static u8 *irq_area; static u32 irq_a0, irq_a1; static int irq_loaded[256], irq_saved;
void ovl_area(u32 *lin_start, u32 *lin_end);
void ovl_irq_begin(void) { if (irq_depth++ == 0) irq_saved = 0; }
void ovl_irq_end(void)
{
    if (--irq_depth > 0 || !irq_saved) return;
    memcpy(M + ((u32)LOADSEG << 4) + irq_a0, irq_area, irq_a1 - irq_a0);
    memcpy(ovl_loaded, irq_loaded, sizeof irq_loaded);
    ovl_gen++;
    irq_saved = 0;
    static int tr = -1; if (tr < 0) tr = getenv("DL_TRACE_OVL") != 0;
    if (tr) fprintf(stderr, "ovl: overlay area put back after an interrupt handler\n");
}
static void irq_save(void)
{
    if (!irq_depth || irq_saved) return;
    ovl_area(&irq_a0, &irq_a1);
    if (irq_a1 <= irq_a0) return;
    irq_area = realloc(irq_area, irq_a1 - irq_a0);
    memcpy(irq_area, M + ((u32)LOADSEG << 4) + irq_a0, irq_a1 - irq_a0);
    memcpy(irq_loaded, ovl_loaded, sizeof irq_loaded);
    irq_saved = 1;
}

static void load_sect(int i, int with_parent)
{
    Sect *s = &sect[i];
    irq_save();
    static int tr = -1; if (tr < 0) tr = getenv("DL_TRACE_OVL") != 0;
    if (tr) fprintf(stderr, "ovl: load s%03d at %04x\n", i, s->load_seg);
    if (with_parent && s->parent != 0xffff && s->parent < nsect && !ovl_loaded[s->parent]) load_sect(s->parent, 1);
    u32 rbytes = s->nrelocs * 4u;
    u32 pad = (16 - rbytes % 16) % 16;
    u32 dst = ((u32)LOADSEG + s->load_seg) * 16u;
    u32 fsz = s->file_paras * 16u, msz = s->mem_paras * 16u;
    if (dst + msz > MEM_SIZE) fatal("overlay %d out of memory", i);
    /* the section's relocation table and bytes, read from the EXE once and kept: a battle loads
       sections thousands of times a second (some 3,000 seeks and reads a second, mostly the same few) */
    if (!sect_file[i]) {
        u8 *b = malloc(rbytes + fsz + 1);
        fseek(exef, sect_file_off[i], SEEK_SET);
        size_t got = fread(b, 1, rbytes, exef);
        fseek(exef, sect_file_off[i] + rbytes + pad, SEEK_SET);
        got += fread(b + rbytes, 1, fsz, exef);
        if (got < rbytes) memset(b + got, 0, rbytes - got);
        else if (got < rbytes + fsz) memset(b + got, 0, rbytes + fsz - got);
        sect_file[i] = b;
    }
    const u8 *rel = sect_file[i];
    memset(M + dst, 0, msz);
    memcpy(M + dst, sect_file[i] + rbytes, fsz);
    for (u32 k = 0; k < s->nrelocs; k++) {
        u16 off = le16(rel + 4 * k), seg = le16(rel + 4 * k + 2);
        u32 a = ((u32)LOADSEG + seg) * 16u + off;
        u16 v = (u16)(M[a] | (M[a + 1] << 8)) + LOADSEG;
        M[a] = (u8)v; M[a + 1] = (u8)(v >> 8);
    }
    /* sections overlapping this one are gone, and with them their descendants in the overlay
       tree (whose memory may be untouched, but whose code calls its ancestors directly): as in
       RTLink, a section is present only with its whole chain of parents, so that putting back
       what a call displaced (ovl_restore) restores a state the tree allows */
    for (int j = 0; j < nsect; j++) {
        if (j == i || !ovl_loaded[j]) continue;
        u32 a0 = sect[j].load_seg, a1 = a0 + sect[j].mem_paras;
        u32 b0 = s->load_seg, b1 = b0 + s->mem_paras;
        if (a0 < b1 && b0 < a1) ovl_loaded[j] = 0;
    }
    ovl_loaded[i] = 1;
    for (int changed = 1; changed; ) {
        changed = 0;
        for (int k = 0; k < nsect; k++) {
            u16 pa = sect[k].parent;
            if (ovl_loaded[k] && pa != 0xffff && pa < nsect && !ovl_loaded[pa]) { ovl_loaded[k] = 0; changed = 1; }
        }
    }
    ovl_gen++;
    { void watch_check(const char *); watch_check("overlay load"); }
}
static void load(int i) { load_sect(i, 1); }

void ovl_enter(int i)
{
    if (i < 0 || i >= nsect || ovl_loaded[i]) return;
    load(i);
}

/* RTLink's reload stack. A call through the manager that loads a section displaces the
 * sections sharing its memory - also ones further up the call chain than the caller: the card
 * engine (s043) loading the picture code (s039) displaces the card script (s109) that called
 * it. RTLink reloads what was displaced when the call returns; the translated code brackets
 * every such call with ovl_save / ovl_restore. (Only the caller's own section came back
 * before: a card script then found itself gone, and its option handlers, called through far
 * pointers, ran the function of another card at the same address - travelling with merchants
 * put the party in a grove in the town it was leaving.) */
void ovl_changed(void) { ovl_gen++; }        /* ovl_loaded was put back wholesale (snapshots) */
void ovl_save(OvlSet *o)
{
    static u32 bits[8], bits_gen = 0xffffffffu;
    if (bits_gen != ovl_gen) {
        memset(bits, 0, sizeof bits);
        for (int i = 0; i < nsect; i++) if (ovl_loaded[i]) bits[i >> 5] |= 1u << (i & 31);
        bits_gen = ovl_gen;
    }
    o->gen = ovl_gen;
    memcpy(o->bits, bits, sizeof bits);
}
void ovl_restore(const OvlSet *o)
{
    if (o->gen == ovl_gen) return;
    for (int i = 0; i < nsect; i++)
        if (((o->bits[i >> 5] >> (i & 31)) & 1) && !ovl_loaded[i]) {
            static int tr = -1; if (tr < 0) tr = getenv("DL_TRACE_OVL") != 0;
            if (tr) fprintf(stderr, "ovl: s%03d displaced by the call: reloaded\n", i);
            load_sect(i, 1);
        }
}

/* RTLink's own start-up preloads sections; the native start-up does it here */
void ovl_preload(void)
{
    for (int i = 0; i < nsect; i++)
        if ((sect[i].file_para_hi_flags >> 8) & 1) load(i);
}

void ovl_dump(void)
{
    fprintf(stderr, "  loaded overlay sections:");
    for (int i = 0; i < nsect; i++) if (ovl_loaded[i]) fprintf(stderr, " s%03d@%04x", i, sect[i].load_seg);
    fprintf(stderr, "\n");
}

/* the loaded overlay section containing a link-time linear address (-1: root / none) */
int ovl_section_at(u32 lin)
{
    int best = -1;
    for (int i = 0; i < nsect; i++) {
        if (!ovl_loaded[i] || i == 136) continue;
        u32 a = sect[i].load_seg * 16u, b = a + sect[i].mem_paras * 16u;
        if (lin >= a && lin < b) best = i;
    }
    return best;
}

/* the overlay area (all sections except the preloaded data section): for snapshots */
void ovl_area(u32 *lin_start, u32 *lin_end)
{
    u32 a = 0xffffffff, b = 0;
    for (int i = 0; i < nsect; i++) {
        if ((sect[i].file_para_hi_flags >> 8) & 1) continue;      /* preloaded (data) */
        u32 s0 = sect[i].load_seg * 16u, s1 = s0 + sect[i].mem_paras * 16u;
        if (s0 < a) a = s0;
        if (s1 > b) b = s1;
    }
    *lin_start = a; *lin_end = b;
}
int ovl_count(void) { return nsect; }
