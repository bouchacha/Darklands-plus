/* dos.c - high-level emulation of the DOS services the game uses (int 21h) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#ifdef _WIN32
#include <io.h>
#include <direct.h>
#define mkdir_(p) _mkdir(p)
#else
#include <strings.h>
#define mkdir_(p) mkdir(p, 0777)
#endif
#include "cpu.h"
#include "hle.h"
#include "dispatch.h"

char game_dir[1024];
static char dos_cwd[128] = "";          /* relative to game_dir, '/'-separated, no leading slash */
static FILE *handles[64];
static u8 handle_cfg[64];       /* CONFIG.DRK: the sound card choice is replaced (see 3Fh) */
static u16 dta_seg, dta_off;
u16 psp_seg;
int dos_trace;
static int ems_dev_open;

#define SETCF(c) do { u16 f_ = get_flags(); set_flags((u16)((c) ? (f_ | 1) : (f_ & ~1))); } while (0)

static void dos_error(u16 code) { AX = code; SETCF(1); }

static void get_str(u16 seg, u16 off, char *buf, int n)
{
    int i = 0;
    for (; i < n - 1; i++) { char c = (char)rb(seg, (u16)(off + i)); if (!c) break; buf[i] = c; }
    buf[i] = 0;
}

/* case-insensitive resolution of one path component inside dir */
static int find_ci(const char *dir, const char *name, char *out, size_t outn)
{
    DIR *d = opendir(dir);
    int found = 0;
    if (d) {
        struct dirent *e;
        while ((e = readdir(d))) {
            if (!strcasecmp(e->d_name, name)) { snprintf(out, outn, "%s/%s", dir, e->d_name); found = 1; break; }
        }
        closedir(d);
    }
    if (!found) snprintf(out, outn, "%s/%s", dir, name);
    return found;
}

/* DOS path -> host path under game_dir */
static void host_path(const char *dos, char *out, size_t outn)
{
    char tmp[512], rel[512];
    const char *p = dos;
    if (p[0] && p[1] == ':') p += 2;
    if (*p == '\\' || *p == '/') { snprintf(rel, sizeof rel, "%s", p + 1); }
    else if (dos_cwd[0]) snprintf(rel, sizeof rel, "%s/%s", dos_cwd, p);
    else snprintf(rel, sizeof rel, "%s", p);
    for (char *q = rel; *q; q++) if (*q == '\\') *q = '/';
    snprintf(tmp, sizeof tmp, "%s", game_dir);
    for (char *tok = rel; *tok;) {
        char *sl = strchr(tok, '/');
        if (sl) *sl = 0;
        if (*tok && strcmp(tok, ".")) {
            char nxt[1024];
            find_ci(tmp, tok, nxt, sizeof nxt);
            snprintf(tmp, sizeof tmp, "%s", nxt);
        }
        if (!sl) break;
        tok = sl + 1;
    }
    snprintf(out, outn, "%s", tmp);
}

void dos_host_path(const char *dos, char *out, size_t outn) { host_path(dos, out, outn); }

/* the DOS name relative to the game folder */
static void dos_rel(const char *dos, char *rel, size_t n)
{
    const char *p = dos;
    if (p[0] && p[1] == ':') p += 2;
    if (*p == '\\' || *p == '/') snprintf(rel, n, "%s", p + 1);
    else if (dos_cwd[0]) snprintf(rel, n, "%s/%s", dos_cwd, p);
    else snprintf(rel, n, "%s", p);
}
/* the host file to read for a DOS name: a mod's (mods/datamods.c) or the game's own */
const char *datamods_redirect(const char *rel, const char *game_path);
void dos_host_path_read(const char *dos, char *out, size_t outn)
{
    host_path(dos, out, outn);
    char rel[600]; dos_rel(dos, rel, sizeof rel);
    const char *m = datamods_redirect(rel, out);
    if (m) snprintf(out, outn, "%s", m);
}

static int new_handle(FILE *f)
{
    for (int h = 5; h < 64; h++) if (!handles[h]) { handles[h] = f; return h; }
    return -1;
}

/* ---------------------------------------------------------------- memory: a real MCB chain
 * (the game walks it through int 21h 52h). Each block is preceded by a paragraph:
 * +0 'M' or 'Z', +1 owner PSP (0 = free), +3 size in paragraphs, +8 name. */
#define MEM_TOP 0xA000
static u16 first_mcb;
static int trace_mem(void) { static int t = -1; if (t < 0) t = getenv("DL_TRACE_MEM") != 0; return t; }
#define MCB_T(m) M[(u32)(m) * 16]
#define MCB_OWNER(m) rw((m), 1)
#define MCB_SIZE(m) rw((m), 3)

static void mcb_set(u16 m, u8 t, u16 owner, u16 size)
{
    MCB_T(m) = t; ww(m, 1, owner); ww(m, 3, size);
}
static void coalesce(void)
{
    for (u16 m = first_mcb; MCB_T(m) == 'M';) {
        u16 n = (u16)(m + 1 + MCB_SIZE(m));
        if (!MCB_OWNER(m) && !MCB_OWNER(n)) { mcb_set(m, MCB_T(n), 0, (u16)(MCB_SIZE(m) + 1 + MCB_SIZE(n))); continue; }
        m = n;
    }
}
static u16 largest_free(void)
{
    u16 best = 0;
    for (u16 m = first_mcb;; m = (u16)(m + 1 + MCB_SIZE(m))) {
        if (!MCB_OWNER(m) && MCB_SIZE(m) > best) best = MCB_SIZE(m);
        if (MCB_T(m) != 'M') break;
    }
    return best;
}
/* env: environment block segment (its MCB is at env-1); psp: program block (MCB at psp-1) */
void dos_mem_init2(u16 env, u16 env_paras, u16 psp)
{
    first_mcb = (u16)(env - 1);
    mcb_set(first_mcb, 'M', psp, env_paras);
    u16 m = (u16)(psp - 1);
    if (m != (u16)(env + env_paras)) fatal("dos_mem_init: layout");
    mcb_set(m, 'Z', psp, (u16)(MEM_TOP - psp));
    /* DOS "list of lists" at 0080:0026, first MCB at [-2] */
    ww(0x80, 0x24, first_mcb);
    /* device chain (list of lists + 22h): NUL -> EMMXXXX0 (the EMS driver at F100:0000) -> end */
    ww(0x80, 0x48, 0x0000); ww(0x80, 0x4a, EMS_SEG); ww(0x80, 0x4c, 0x8004);
    memcpy(M + 0x800 + 0x48 + 10, "NUL     ", 8);
    ww(EMS_SEG, 0, 0xffff); ww(EMS_SEG, 2, 0xffff); ww(EMS_SEG, 4, 0xc000);
}
void dos_mem_init(u16 first_seg, u16 prog_paras) { (void)first_seg; (void)prog_paras; }
static void split(u16 m, u16 paras)
{
    u16 size = MCB_SIZE(m);
    if (size <= paras) return;
    u16 n = (u16)(m + 1 + paras);
    mcb_set(n, MCB_T(m), 0, (u16)(size - paras - 1));
    MCB_T(m) = 'M'; ww(m, 3, paras);
}
static int mem_alloc(u16 paras, u16 *seg)
{
    for (u16 m = first_mcb;; m = (u16)(m + 1 + MCB_SIZE(m))) {
        if (!MCB_OWNER(m) && MCB_SIZE(m) >= paras) {
            split(m, paras);
            ww(m, 1, psp_seg);
            *seg = (u16)(m + 1);
            return 1;
        }
        if (MCB_T(m) != 'M') break;
    }
    return 0;
}
static int valid_block(u16 seg)
{
    for (u16 m = first_mcb;; m = (u16)(m + 1 + MCB_SIZE(m))) {
        if ((u16)(m + 1) == seg) return 1;
        if (MCB_T(m) != 'M') return 0;
    }
}
static int mem_free(u16 seg)
{
    if (!valid_block(seg)) return 0;
    ww((u16)(seg - 1), 1, 0);
    coalesce();
    return 1;
}
static int mem_resize(u16 seg, u16 paras, u16 *maxp)
{
    if (!valid_block(seg)) { *maxp = 0; return 0; }
    u16 m = (u16)(seg - 1);
    u16 avail = MCB_SIZE(m);
    if (MCB_T(m) == 'M') {
        u16 n = (u16)(m + 1 + MCB_SIZE(m));
        if (!MCB_OWNER(n)) avail = (u16)(avail + 1 + MCB_SIZE(n));
    }
    if (paras > avail) { *maxp = avail; return 0; }
    if (avail != MCB_SIZE(m)) {     /* absorb the free block that follows */
        u16 n = (u16)(m + 1 + MCB_SIZE(m));
        mcb_set(m, MCB_T(n), MCB_OWNER(m), avail);
    }
    split(m, paras);
    coalesce();
    return 1;
}

/* ---------------------------------------------------------------- find first / next */
static char find_pat[64], find_dir[1024];
static long find_pos;
static int wild_match(const char *pat, const char *name)
{
    /* DOS 8.3 wildcard match, case-insensitive */
    char pb[16] = "", pe[8] = "", nb[16] = "", ne[8] = "";
    const char *d;
    d = strchr(pat, '.'); if (d) { snprintf(pb, sizeof pb, "%.*s", (int)(d - pat), pat); snprintf(pe, sizeof pe, "%s", d + 1); } else snprintf(pb, sizeof pb, "%s", pat);
    d = strrchr(name, '.'); if (d) { snprintf(nb, sizeof nb, "%.*s", (int)(d - name), name); snprintf(ne, sizeof ne, "%s", d + 1); } else snprintf(nb, sizeof nb, "%s", name);
    if (strlen(nb) > 8 || strlen(ne) > 3) return 0;
    const char *ps[2] = { pb, pe }, *ns[2] = { nb, ne };
    for (int k = 0; k < 2; k++) {
        const char *p = ps[k], *n = ns[k];
        for (;;) {
            if (*p == '*') break;
            if (!*p) { if (*n) return 0; break; }
            if (*p == '?') { p++; if (*n) n++; continue; }
            if (!*n || toupper((u8)*p) != toupper((u8)*n)) return 0;
            p++; n++;
        }
    }
    return 1;
}
/* the matches are listed at find-first, sorted: by name, saved games (*.SAV) newest first (the
 * load screen shows them in this order, and the directory order of today's file systems means
 * nothing) */
static FindEnt *find_list; static int find_n;
static int find_cmp_name(const void *a, const void *b) { return strcmp(((const FindEnt *)a)->name, ((const FindEnt *)b)->name); }
static int find_cmp_new(const void *a, const void *b)
{
    const FindEnt *x = a, *y = b;
    if (x->mtime != y->mtime) return x->mtime > y->mtime ? -1 : 1;
    return strcmp(x->name, y->name);
}
int dos_list(const char *dir, const char *pat, FindEnt **out)
{
    DIR *dd = opendir(dir);
    *out = 0;
    if (!dd) return 0;
    int n = 0, cap = 0;
    FindEnt *l = 0;
    struct dirent *e;
    while ((e = readdir(dd))) {
        if (e->d_name[0] == '.' || !wild_match(pat, e->d_name)) continue;
        char full[2048]; struct stat st;
        snprintf(full, sizeof full, "%s/%s", dir, e->d_name);
        if (stat(full, &st) || !S_ISREG(st.st_mode)) continue;
        if (n == cap) { cap = cap ? cap * 2 : 32; l = realloc(l, sizeof *l * (size_t)cap); }
        int k = 0;
        for (const char *c = e->d_name; *c && k < 12; c++) l[n].name[k++] = (char)toupper((u8)*c);
        l[n].name[k] = 0;
        snprintf(l[n].real, sizeof l[n].real, "%s", e->d_name);
        l[n].size = (long)st.st_size; l[n].mtime = st.st_mtime;
        n++;
    }
    closedir(dd);
    const char *dot = strrchr(pat, '.');
    qsort(l, (size_t)n, sizeof *l, dot && !strcasecmp(dot, ".SAV") ? find_cmp_new : find_cmp_name);
    *out = l;
    return n;
}
static int find_next(void)
{
    if (find_pos >= find_n) return 0;
    FindEnt *f = &find_list[find_pos++];
    /* DTA: +15h attr, +16h time, +18h date, +1Ah size, +1Eh name */
    u32 a = LIN(dta_seg, dta_off);
    struct tm *t = localtime(&f->mtime);
    u16 tm = 0, dt = 0x1221;
    if (t && t->tm_year >= 80) { tm = (u16)(t->tm_hour << 11 | t->tm_min << 5 | t->tm_sec / 2); dt = (u16)((t->tm_year - 80) << 9 | (t->tm_mon + 1) << 5 | t->tm_mday); }
    M[a + 0x15] = 0x20;
    M[a + 0x16] = (u8)tm; M[a + 0x17] = (u8)(tm >> 8); M[a + 0x18] = (u8)dt; M[a + 0x19] = (u8)(dt >> 8);
    for (int k = 0; k < 4; k++) M[a + 0x1a + k] = (u8)((u32)f->size >> (8 * k));
    memset(M + a + 0x1e, 0, 13);
    memcpy(M + a + 0x1e, f->name, strlen(f->name));
    return 1;
}

/* ---------------------------------------------------------------- overlay load (4B03) */
const char *sound_pick(const char *base);
static int load_overlay(const char *dosname, u16 seg, u16 relfac)
{
    char hp[1024];
    /* sound drivers: the device chosen in the port's settings (sounddev.c) */
    char subst[160];
    {
        const char *b = dosname;
        for (const char *p = dosname; *p; p++) if (*p == '\\' || *p == '/' || *p == ':') b = p + 1;
        const char *pick = sound_pick(b);
        if (pick) { snprintf(subst, sizeof subst, "%.*s%s", (int)(b - dosname), dosname, pick); dosname = subst; }
    }
    host_path(dosname, hp, sizeof hp);
    FILE *f = fopen(hp, "rb");
    if (!f) return 0;
    u8 h[28];
    fread(h, 1, 28, f);
    u32 hdr = (u32)(h[8] | (h[9] << 8)) * 16u;
    u32 pages = (u32)(h[4] | (h[5] << 8)), last = (u32)(h[2] | (h[3] << 8));
    u32 total = (pages - 1) * 512u + (last ? last : 512);
    u32 nrel = (u32)(h[6] | (h[7] << 8)), roff = (u32)(h[24] | (h[25] << 8));
    fseek(f, hdr, SEEK_SET);
    fread(M + (u32)seg * 16u, 1, total - hdr, f);
    unsigned hash = 0x811c9dc5u;                 /* FNV-1a of the image: which recompiled driver it is */
    for (u32 k = 0; k < total - hdr; k++) hash = (hash ^ M[(u32)seg * 16u + k]) * 0x01000193u;
    for (u32 k = 0; k < nrel; k++) {
        u8 r[4];
        fseek(f, roff + 4 * k, SEEK_SET); fread(r, 1, 4, f);
        u32 a = ((u32)seg + (u16)(r[2] | (r[3] << 8))) * 16u + (u16)(r[0] | (r[1] << 8));
        u16 v = (u16)(M[a] | (M[a + 1] << 8)) + relfac;
        M[a] = (u8)v; M[a + 1] = (u8)(v >> 8);
    }
    fclose(f);
    driver_register(dosname, seg, total - hdr, hash);
    return 1;
}

/* ---------------------------------------------------------------- int 21h */
void dos_int21(void)
{
    char name[256], hp[1024];
    u8 ah = AH;
    if (dos_trace) {
        fprintf(stderr, "int21 ah=%02x al=%02x bx=%04x cx=%04x dx=%04x ds=%04x\n", ah, AL, BX, CX, DX, DS);
        static int stk = -2; if (stk == -2) { const char *e = getenv("DL_STACK_ON_AH"); stk = e ? (int)strtol(e, 0, 16) : -1; }
        if (stk == ah) ftrace_dump();
    }
    switch (ah) {
    case 0x02: putchar(DL); SETCF(0); break;
    case 0x06: if (DL != 0xff) { putchar(DL); break; }
        poll_flag = 1;
        if (bios_kbd_avail()) { AL = (u8)bios_kbd_get(1); flags_explicit(0, 0, 1, 8); }
        else { AL = 0; flags_explicit(0, 0, 0, 8); }
        break;
    case 0x01: case 0x07: case 0x08: { u16 w; while (!(w = bios_kbd_get(1))) { poll_flag = 1; dl_poll(); plat_sleep_ms(5); }
        AL = (u8)w; break; }
    case 0x0c: while (bios_kbd_get(1)) {} if (AL == 1 || AL == 6 || AL == 7 || AL == 8) { AH = AL; dos_int21(); } break;
    case 0x09: { u16 o = DX; char c; while ((c = (char)rb(DS, o++)) != '$') putchar(c); fflush(stdout); break; }
    case 0x0b: { extern int idle_hits; idle_hits += 50; poll_flag = 1; AL = bios_kbd_avail() ? 0xff : 0; break; }
    case 0x0e: AL = 26; break;
    case 0x19: AL = 2; break;                                   /* C: */
    case 0x1a: dta_seg = DS; dta_off = DX; break;
    case 0x25: ww(0, (u16)(AL * 4), DX); ww(0, (u16)(AL * 4 + 2), DS); break;
    case 0x2a: { time_t t = time(0); struct tm *tm = localtime(&t);
        CX = (u16)(tm->tm_year + 1900); DH = (u8)(tm->tm_mon + 1); DL = (u8)tm->tm_mday; AL = (u8)tm->tm_wday; break; }
    case 0x2c: { u32 ms = plat_ticks_ms(); time_t t = time(0); struct tm *tm = localtime(&t);
        CH = (u8)tm->tm_hour; CL = (u8)tm->tm_min; DH = (u8)tm->tm_sec; DL = (u8)((ms / 10) % 100); break; }
    case 0x2f: ES = dta_seg; BX = dta_off; break;
    case 0x2b: case 0x2d: AL = 0; break;        /* set date / time: accepted, not applied */
    case 0x30: AX = 0x0005; BX = 0; CX = 0; break;              /* DOS 5.0 */
    case 0x33: if (AL == 0) DL = 0; break;
    case 0x35: BX = rw(0, (u16)(AL * 4)); ES = rw(0, (u16)(AL * 4 + 2)); break;
    case 0x36: AX = 64; BX = 0x7fff; CX = 512; DX = 0xffff; break;  /* plenty of free space */
    case 0x3c: case 0x5b: {
        get_str(DS, DX, name, sizeof name); host_path(name, hp, sizeof hp);
        FILE *f = fopen(hp, "w+b");
        if (!f) { dos_error(3); break; }
        int h = new_handle(f); if (h < 0) { fclose(f); dos_error(4); break; }
        AX = (u16)h; SETCF(0); break; }
    case 0x3d: {
        get_str(DS, DX, name, sizeof name);
        if (!strcasecmp(name, "EMMXXXX0")) { ems_dev_open = 1; AX = 63; SETCF(0); break; }
        if ((AL & 3) == 0) dos_host_path_read(name, hp, sizeof hp);    /* reading: a mod's file if one is on */
        else host_path(name, hp, sizeof hp);
        const char *mode = (AL & 3) == 0 ? "rb" : "r+b";
        FILE *f = fopen(hp, mode);
        if (!f && (AL & 3)) f = fopen(hp, "rb");
        if (!f) { const char *hdart_virtual_file(const char *); const char *v = hdart_virtual_file(name); if (v) f = fopen(v, "rb"); }   /* a new picture made from art */
        if (dos_trace) fprintf(stderr, "  open %s -> %s %s\n", name, hp, f ? "ok" : "FAIL");
        if (!f) { dos_error(2); break; }
        int h = new_handle(f); if (h < 0) { fclose(f); dos_error(4); break; }
        { const char *b = strrchr(name, '\\'); b = b ? b + 1 : name; handle_cfg[h] = !strcasecmp(b, "CONFIG.DRK"); }
        AX = (u16)h; SETCF(0); break; }
    case 0x3e:
        if (BX == 63 && ems_dev_open) { ems_dev_open = 0; SETCF(0); break; }
        if (BX < 64 && handles[BX]) { fclose(handles[BX]); handles[BX] = 0; SETCF(0); }
        else if (BX < 5) SETCF(0);
        else dos_error(6);
        break;
    case 0x3f: {
        if (BX == 0) { AX = 0; SETCF(0); break; }
        if (BX >= 64 || !handles[BX]) { dos_error(6); break; }
        u32 got = 0, a = LIN(DS, DX);
        long pos0 = ftell(handles[BX]);
        if (CX) { fseek(handles[BX], 0, SEEK_CUR); got = (u32)fread(M + a, 1, CX, handles[BX]); }
        AX = (u16)got; SETCF(0); break; }
    case 0x40: {
        if (BX == 1 || BX == 2) { fwrite(M + LIN(DS, DX), 1, CX, stdout); AX = CX; SETCF(0); break; }
        if (BX >= 64 || !handles[BX]) { dos_error(6); break; }
        if (CX == 0) {               /* truncate at the current position */
            fseek(handles[BX], 0, SEEK_CUR); long pos = ftell(handles[BX]); fflush(handles[BX]);
#ifdef _WIN32
            _chsize(_fileno(handles[BX]), pos);
#else
            if (ftruncate(fileno(handles[BX]), pos)) {}
#endif
            AX = 0; SETCF(0); break;
        }
        fseek(handles[BX], 0, SEEK_CUR);
        AX = (u16)fwrite(M + LIN(DS, DX), 1, CX, handles[BX]); SETCF(0); break; }
    case 0x41: get_str(DS, DX, name, sizeof name); host_path(name, hp, sizeof hp);
        if (remove(hp)) dos_error(2); else SETCF(0); break;
    case 0x42: {
        if (BX >= 64 || !handles[BX]) { dos_error(6); break; }
        long off = (long)(s32)(((u32)CX << 16) | DX);
        if (AL == 0) off = (long)(((u32)CX << 16) | DX);
        fseek(handles[BX], off, AL == 0 ? SEEK_SET : AL == 1 ? SEEK_CUR : SEEK_END);
        long p = ftell(handles[BX]);
        AX = (u16)p; DX = (u16)(p >> 16); SETCF(0); break; }
    case 0x43: get_str(DS, DX, name, sizeof name); host_path(name, hp, sizeof hp);
        { struct stat st; if (stat(hp, &st)) { dos_error(2); break; } CX = 0x20; SETCF(0); } break;
    case 0x44:
        if (BX == 63 && ems_dev_open) { if (AL == 0) DX = 0x0080; else AL = 0xff; SETCF(0); }
        else if (AL == 0) { DX = (BX < 3) ? 0x80d3 : 0x0002; SETCF(0); }
        else if (AL == 8) { AX = 1; SETCF(0); }                  /* not removable */
        else { SETCF(0); }
        break;
    case 0x45: if (BX < 64 && handles[BX]) {
            FILE *f = fdopen(dup(fileno(handles[BX])), "r+b"); int h = new_handle(f); AX = (u16)h; SETCF(0);
        } else dos_error(6);
        break;
    case 0x47: { u32 a = LIN(DS, SI); int n = 0;
        for (const char *c = dos_cwd; *c && n < 63; c++) M[a + n++] = (u8)(*c == '/' ? '\\' : toupper((u8)*c));
        M[a + n] = 0; SETCF(0); break; }
    case 0x48: { u16 s, want = BX; if (mem_alloc(BX, &s)) { AX = s; SETCF(0); } else { BX = largest_free(); dos_error(8); }
        if (trace_mem()) fprintf(stderr, "mem: alloc %04x paras -> %04x%s\n", want, AX, get_cf() ? " FAILED" : ""); break; }
    case 0x49: if (trace_mem()) fprintf(stderr, "mem: free %04x\n", ES); if (mem_free(ES)) SETCF(0); else dos_error(9); break;
    case 0x4a: { u16 mx, want = BX; if (mem_resize(ES, BX, &mx)) SETCF(0); else { BX = mx; dos_error(8); }
        if (trace_mem()) fprintf(stderr, "mem: resize %04x to %04x paras%s\n", ES, want, get_cf() ? " FAILED" : ""); break; }
    case 0x4b:
        if (AL == 3) {
            get_str(DS, DX, name, sizeof name);
            u16 seg = rw(ES, BX), rel = rw(ES, (u16)(BX + 2));
            if (load_overlay(name, seg, rel)) { AX = 0; SETCF(0); } else dos_error(2);
        } else dos_error(1);
        break;
    case 0x4c: fflush(stdout);
        { int game_mode(void); fprintf(stderr, "program exit: int 21h 4Ch code %d (mode %d, card %.12s, DS:07f4 %d)\n", AL, game_mode(), (const char *)M + LIN((u16)(LOADSEG + 0x321a), 0xe3d4), rb((u16)(LOADSEG + 0x321a), 0x07f4)); }
        { int saves_restart_on_exit(void); void plat_restart(void); if (saves_restart_on_exit()) plat_restart(); }
        plat_quit(AL); break;
    case 0x39: get_str(DS, DX, name, sizeof name); host_path(name, hp, sizeof hp); if (mkdir_(hp)) dos_error(5); else SETCF(0); break;
    case 0x3a: get_str(DS, DX, name, sizeof name); host_path(name, hp, sizeof hp); if (rmdir(hp)) dos_error(5); else SETCF(0); break;
    case 0x4e: {
        get_str(DS, DX, name, sizeof name);
        char *bs = strrchr(name, '\\'), dirpart[256] = "";
        const char *pat = name;
        if (bs) { *bs = 0; snprintf(dirpart, sizeof dirpart, "%s", name); pat = bs + 1; }
        if (dirpart[0]) host_path(dirpart, find_dir, sizeof find_dir); else host_path(".", find_dir, sizeof find_dir);
        snprintf(find_pat, sizeof find_pat, "%s", pat);
        free(find_list);
        find_n = dos_list(find_dir, find_pat, &find_list);
        find_pos = 0;
        if (find_next()) { AX = 0; SETCF(0); } else dos_error(18);
        break; }
    case 0x4f: if (find_next()) { AX = 0; SETCF(0); } else dos_error(18); break;
    case 0x56: { char n2[256], h2[1024]; get_str(DS, DX, name, sizeof name); get_str(ES, DI, n2, sizeof n2);
        host_path(name, hp, sizeof hp); host_path(n2, h2, sizeof h2);
        if (rename(hp, h2)) dos_error(5); else SETCF(0); break; }
    case 0x57: if (AL == 0) { CX = 0; DX = 0x1221; } SETCF(0); break;
    case 0x59: AX = 0; BX = 0; CH = 0; break;
    case 0x52: ES = 0x80; BX = 0x26; break;
    case 0x50: psp_seg = BX; break;
    case 0x51: case 0x62: BX = psp_seg; break;
    case 0x58: if (AL == 0) AX = 0; SETCF(0); break;
    default:
        fprintf(stderr, "int 21h: unimplemented AH=%02x AL=%02x\n", ah, AL);
        dos_error(1);
    }
}
