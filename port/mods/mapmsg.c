/* mapmsg.c - the travel map's messages (a robber knight's land, a blighted country ...) as a
 * native box (display.native_mapmsg; the view is ui/mapmsg.cpp).
 *
 * The original (map_message, s030 1462:2bb0) reads record msg (400 bytes after a count byte) of
 * darkland.msg, draws it in a grey box over the map and waits for a key or a click. Here the same
 * text is shown in the cards' style and waited for the same way; the click is taken (it does not
 * walk the party). */
#include <stdio.h>
#include <string.h>
#include "../runtime/cpu.h"
#include "../runtime/hooks.h"
#include "../runtime/game.h"
#include "../runtime/gamevars.h"
#include "../runtime/config.h"
#include "../runtime/hle.h"
#include "nscreen.h"
#include "mapmsg.h"

void plat_sleep_ms(int ms);
void dl_poll(void);

MapMsgModel mapmsg_model;
static int mm_on = 1;

int mapmsg_is_on(void) { return mm_on; }
void mapmsg_set(int on) { mm_on = on; config_set_int("display.native_mapmsg", on); }

static int read_msg(int msg, char *out, int n)
{
    char p[1200]; dos_host_path_read("darkland.msg", p, sizeof p);
    FILE *f = fopen(p, "rb");
    if (!f) return 0;
    int ok = fseek(f, (long)msg * 400 + 1, SEEK_SET) == 0 && fread(out, 1, (size_t)(n - 1 < 400 ? n - 1 : 400), f) > 0;
    fclose(f);
    out[n - 1] = 0;
    return ok;
}

static void msg_hook(void)                   /* map_message (msg), s030 1462:2bb0 */
{
    int msg = (s16)rw(SS, (u16)(SP + 4));
    MapMsgModel *m = &mapmsg_model;
    if (!mm_on || !ns_active() || msg < 0 || !read_msg(msg, m->text, sizeof m->text) || !m->text[0]) { hook_call_original(); return; }
    ns_unwinding = 0;
    while (g_rb(GV_mouse_buttons) & 3) { dl_poll(); plat_sleep_ms(3); }     /* (as the original: the button let go first) */
    m->open = 1; m->seq++;
    fprintf(stderr, "map message %d: open\n", msg);
    ns_enter(NS_MAPMSG);
    ns_clear();
    ns_input_begin();
    for (;;) {
        dl_poll();
        plat_sleep_ms(3);
        ns_input_poll();
        ns_call(GF_exit_if_requested, 0);
        if (ns_unwinding) break;
        if (ns_call(GF_key_pressed, 0)) { ns_call(GF_get_key, 0); break; }
        if (ns_take_click(0, 0) || ns_in.rclick) { ns_in.rclick = 0; break; }
    }
    m->open = 0;
    ns_leave();
    if (ns_unwinding) return;
    while (g_rb(GV_mouse_buttons) & 3) { dl_poll(); plat_sleep_ms(3); }
    ns_call(-1, 0x047c, 0x1320, 4, 6, 0, 0, 0);                             /* (the click taken: it does not walk the party) */
    fprintf(stderr, "map message %d: closed\n", msg);
    hook_return_far(0);
}

void mapmsg_init(void)
{
    mm_on = config_get_int("display.native_mapmsg", 1);
    hook_install(GF_map_message, msg_hook);
}
