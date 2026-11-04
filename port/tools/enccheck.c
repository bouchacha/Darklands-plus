/* enccheck.c - check new-encounter scripts without the game: the real parser and checks of
 * port/mods/encounters.c, with stand-ins for the game. For modders (the same messages as
 * darklands_port.log) and for fuzzing (port/tools/encfuzz.py).
 *
 *   cc -O1 -I port/runtime -o build/enccheck port/tools/enccheck.c
 *   (with -g -fsanitize=address,undefined for fuzzing)
 *   ENC_MSG="mods/My mod/MSGFILES" build/enccheck "mods/My mod/ENCOUNTERS/"*.TXT
 *   ENC_DIR="mods/My mod/ENCOUNTERS" ENC_MSG=... build/enccheck: the saints and things they ask for, and
 *   those of them the game's own cards never ask for (exit status 1: Darklands+'s own encounters give
 *   nothing a use the original leaves without one; release_check.py runs it on port/content)
 *
 * ENC_MSG: the mod's MSGFILES folder, so the scripts are checked against their card texts too. */
#include "../mods/encounters.c"
#include "../mods/saintinfo.c"
u8 *M; CPU R; u16 LOADSEG; int unw_active; u16 unw_sp, unw_ip, unw_cs; int unw_far; int tail_pending; u16 tail_sp;
int travel_step_result;
int watch_on; u16 sp_low; int ftrace_depth; u32 ftrace_stack[4096]; u16 ftrace_bp[4096], ftrace_sp[4096], ftrace_ss[4096], ftrace_ret[4096];
void watch_check(const char *s) { (void)s; } void sp_low_hit(void) {}
int config_get_int(const char *k, int d) { (void)k; return d; }
void ui_notice(const char *m) { (void)m; }
void (*game_func(int s, u16 a, u16 b))(void) { (void)s; (void)a; (void)b; return 0; }
void dispatch_far(u16 a, u16 b) { (void)a; (void)b; abort(); }
void hook_call_original(void) {}
int hook_install(int s, unsigned a, unsigned b, void (*f)(void)) { (void)s; (void)a; (void)b; (void)f; return 0; }
void hook_remove(int s, unsigned a, unsigned b, void (*f)(void)) { (void)s; (void)a; (void)b; (void)f; }
int game_mode(void) { return 0; } int ns_screen; int dos_no_writes;
u32 native_far_ptr(NativeFn f, int a) { (void)f; (void)a; return 0; }
void ovl_enter(int n) { (void)n; } void ovl_save(OvlSet *o) { (void)o; } void ovl_restore(const OvlSet *o) { (void)o; }
int datamods_dirs(const char *sub, char out[][1200], int max) { (void)max; const char *m = getenv(strcasecmp(sub, "ENCOUNTERS") ? "ENC_MSG" : "ENC_DIR"); if (!m || (strcasecmp(sub, "MSGFILES") && strcasecmp(sub, "ENCOUNTERS"))) return 0; snprintf(out[0], 1200, "%s", m); return 1; }
int datamods_generation(void) { return 0; }
void saves_on_loaded(void (*fn)(void)) { (void)fn; }
void fixes_clamp_party_items(void) {}
int fix_is_on(const char *id) { (void)id; return 0; } int sheetview_shared(void) { return 1; }   /* (saintinfo.c's texts, unused here) */
int gm_member_present(int m) { (void)m; return 0; } int gm_member_status(int m) { (void)m; return 0; }
void gm_member_name(int m, char *o, int n) { (void)m; if (n) o[0] = 0; }
void gm_member_attr(int m, int k, int *c, int *x) { (void)m; (void)k; *c = *x = 0; } int gm_member_skill(int m, int k) { (void)m; (void)k; return 0; }
int gm_item_count(int m) { (void)m; return 0; } int gm_item(int m, int i, GmItem *o) { (void)m; (void)i; (void)o; return 0; }
int cutcards_restored_on(void) { return 0; } int cutcards_scripts(const char **id, const char **t, int i) { (void)id; (void)t; (void)i; return 0; }
int main(int argc, char **argv)
{
    static u8 mem[0x110000]; M = mem;
    if (getenv("ENC_DIR")) {                    /* ENC_DIR (and ENC_MSG): what the encounters of that folder ask for, as the game counts it */
        printf("%d encounters\n", encounters_count());
        printf("saints:"); for (int k = 0; k < 136; k++) if (encounters_asks_saint(k)) printf(" %d:%d", k, encounters_asks_saint(k));
        printf("\nitems:"); for (int k = 0; k < 200; k++) if (encounters_asks_item(k)) printf(" %#x:%d", k, encounters_asks_item(k));
        printf("\n");
        int bad = 0;                            /* (asked here, never by the game's own cards) */
        for (int k = 0; k < SAINT_INFO_N; k++) if (encounters_asks_saint(k) && !saint_info_cards_game(k)) { if (!bad++) printf("never asked for by the game's own cards:"); printf(" saint %d", k); }
        for (int k = 0; k < 200; k++)
            if (encounters_asks_item(k) && ((k >= 95 && k <= 116 && !potion_cards_game(k)) || item_unasked_game(k))) { if (!bad++) printf("never asked for by the game's own cards:"); printf(" item %#x", k); }
        if (bad) printf("\n");
        return bad != 0;
    }
    for (int i = 1; i < argc; i++) {
        Enc e; load_file(&e, argv[i], "X");
        check(&e);
        printf("%s: %s\n", argv[i], e.ok ? "ok" : "errors");
        free_enc(&e);
    }
    return 0;
}
