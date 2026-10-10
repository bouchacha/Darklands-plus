/* main.c - start-up of the native Darklands: load the original DARKLAND.EXE image into the
 * emulated address space (the recompiled code reads its data, code-segment tables and
 * overlay sections from there), set up DOS structures, and run the program. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cpu.h"
#include "hle.h"
#include "funcs.h"
#include <SDL.h>
void dos_mem_init2(u16 env, u16 env_paras, u16 psp);



u16 LOADSEG = 0x0200;
extern int dos_trace, bios_trace;

static int file_exists(const char *p) { FILE *f = fopen(p, "rb"); if (f) { fclose(f); return 1; } return 0; }

static int find_game_dir(int argc, char **argv)
{
    char p[1100];
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') continue;
        snprintf(game_dir, sizeof game_dir, "%s", argv[i]);
        snprintf(p, sizeof p, "%s/DARKLAND.EXE", game_dir);
        if (file_exists(p)) return 1;
        fprintf(stderr, "no DARKLAND.EXE in %s\n", game_dir);
        return 0;
    }
    /* next to the executable, then the current directory */
    const char *cands[4] = { 0, ".", 0, 0 };
    char sub[1100] = "";
    char exedir[1024] = "";
    {
        char *b = SDL_GetBasePath();
        if (b) { snprintf(exedir, sizeof exedir, "%s", b); SDL_free(b);
                 size_t n = strlen(exedir); if (n && (exedir[n - 1] == '/' || exedir[n - 1] == '\\')) exedir[n - 1] = 0;
                 cands[0] = exedir; snprintf(sub, sizeof sub, "%s/DARKLAND", exedir); cands[2] = sub; }
    }
    for (int i = 0; i < 3; i++) {
        if (!cands[i]) continue;
        snprintf(p, sizeof p, "%s/DARKLAND.EXE", cands[i]);
        if (file_exists(p)) { snprintf(game_dir, sizeof game_dir, "%s", cands[i]); return 1; }
    }
    return 0;
}

static u16 le16(const u8 *p) { return (u16)(p[0] | (p[1] << 8)); }

int main(int argc, char **argv)
{
    int skip_rtlink = 1;
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--trace-dos")) dos_trace = 1;
        else if (!strcmp(argv[i], "--trace-bios")) bios_trace = 1;
        else if (!strcmp(argv[i], "--rtlink")) skip_rtlink = 0;
    }
    if (!find_game_dir(argc, argv)) {
        const char *msg = "Put darklands.exe into your Darklands folder (the one that contains DARKLAND.EXE),\n"
                          "or start it with that folder as its argument.";
        fprintf(stderr, "%s\n", msg);
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Darklands", msg, 0);
        return 1;
    }
#ifdef _WIN32
    {   /* no console on Windows: keep a log next to the game */
        char lp[1100]; snprintf(lp, sizeof lp, "%s/darklands_port.log", game_dir);
        freopen(lp, "w", stderr);
    }
#endif
    char exe[1100];
    snprintf(exe, sizeof exe, "%s/DARKLAND.EXE", game_dir);

    M = calloc(1, MEM_SIZE);
#ifdef FTRACE
    { extern u32 watch_lo, watch_len; const char *w = getenv("DL_WATCH");   /* linear-hex[:len] */
      const char *ir = getenv("DL_INTERP");      /* lo-hi[@sect] link addresses, hex */
      if (ir) { extern u32 interp_lo, interp_hi; extern int interp_sect; interp_lo = (u32)strtoul(ir, 0, 16);
                const char *d = strchr(ir, '-'); interp_hi = d ? (u32)strtoul(d + 1, 0, 16) : interp_lo + 1;
                const char *at = strchr(ir, '@'); if (at) interp_sect = atoi(at + 1); }
      if (w) { watch_lo = (u32)strtoul(w, 0, 16); const char *c = strchr(w, ':'); watch_len = c ? (u32)strtoul(c + 1, 0, 16) : 1; } }
#endif
    plat_init(1);
    bios_init();

    /* ---- MZ load */
    FILE *f = fopen(exe, "rb");
    if (!f) fatal("cannot open %s", exe);
    u8 h[32];
    if (fread(h, 1, 32, f) != 32) fatal("short read");
    u32 hdr = le16(h + 8) * 16u;
    u32 size = (le16(h + 4) - 1) * 512u + (le16(h + 2) ? le16(h + 2) : 512) - hdr;
    u16 nrel = le16(h + 6), relo = le16(h + 0x18);
    u16 ss = le16(h + 0x0e), sp = le16(h + 0x10);
    fseek(f, hdr, SEEK_SET);
    if (fread(M + (u32)LOADSEG * 16, 1, size, f) != size) fatal("short read of load image");
    for (int k = 0; k < nrel; k++) {
        u8 r[4];
        fseek(f, relo + 4 * k, SEEK_SET);
        if (fread(r, 1, 4, f) != 4) fatal("short read");
        u32 a = ((u32)LOADSEG + le16(r + 2)) * 16u + le16(r);
        u16 v = (u16)(M[a] | (M[a + 1] << 8)) + LOADSEG;
        M[a] = (u8)v; M[a + 1] = (u8)(v >> 8);
    }
    fclose(f);

    /* ---- environment and PSP */
    u16 env = 0x0101;
    psp_seg = (u16)(LOADSEG - 0x10);
    {
        const char *vars[] = { "COMSPEC=C:\\COMMAND.COM", "PATH=C:\\", 0 };
        u32 a = (u32)env * 16;
        for (int i = 0; vars[i]; i++) { strcpy((char *)M + a, vars[i]); a += (u32)strlen(vars[i]) + 1; }
        M[a++] = 0;
        M[a++] = 1; M[a++] = 0;
        strcpy((char *)M + a, "C:\\DARKLAND.EXE");
    }
    {
        u32 p = (u32)psp_seg * 16;
        M[p] = 0xcd; M[p + 1] = 0x20;
        ww(psp_seg, 2, 0xa000);
        ww(psp_seg, 0x2c, env);
        M[p + 0x50] = 0xcd; M[p + 0x51] = 0x21; M[p + 0x52] = 0xcb;
        M[p + 0x80] = 0; M[p + 0x81] = 0x0d;
        for (int i = 0; i < 20; i++) M[p + 0x18 + i] = (u8)(i < 5 ? (i < 3 ? 1 : i) : 0xff);
        ww(psp_seg, 0x32, 20); ww(psp_seg, 0x34, 0x18); ww(psp_seg, 0x36, psp_seg);
    }
    dos_mem_init2(env, (u16)(psp_seg - 1 - env), psp_seg);

    /* ---- overlays */
    ovl_init(exe);
    ovl_preload();

    /* ---- registers as DOS leaves them */
    DS = ES = psp_seg;
    SS = (u16)(LOADSEG + ss); SP = sp;
    AX = BX = 0; CX = 0xff; DX = psp_seg; SI = 0; DI = SP; BP = 0x091c;
    set_flags(0x0202);
    R.iflag = 1;
    CS = (u16)(LOADSEG + 0x09c0);

    if (skip_rtlink) {
        /* what the RTLink start-up leaves behind: the PSP saved in its segment */
        ww((u16)(LOADSEG + 0x09c0), 2534, psp_seg);
        F_root_06a2a();         /* C start-up (__astart) */
    } else {
        F_root_0a2fd();         /* RTLink start-up, which then runs __astart */
    }
    if (unw_active) fprintf(stderr, "program left through an unmatched return (slot %04x -> %04x:%04x)\n", unw_sp, unw_cs, unw_ip);
    else fprintf(stderr, "program returned from its entry point\n");
    plat_quit(0);
    return 0;
}
