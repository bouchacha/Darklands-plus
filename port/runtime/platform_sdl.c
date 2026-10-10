/* platform_sdl.c - window, screen, input and time on SDL2.
 *
 * Test hooks (environment variables), used by the automated play tests:
 *   DL_SCRIPT="t:cmd;t:cmd;..."  t in ms since start; cmd is one of
 *        key:NAME        press+release an SDL key name (e.g. Return, Escape, a, F1)
 *        down:NAME / up:NAME
 *        move:X,Y        mouse position in game pixels (0..319, 0..199)
 *        click:X,Y       left click at X,Y   rclick:X,Y   right click
 *        shot:FILE.bmp   save the screen
 *        quit
 *   DL_HEADLESS=1        no window (SDL dummy video driver)
 *   DL_FAST=1            don't throttle presentation
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL.h>
#include "cpu.h"
#include "hle.h"
#include "../ui/ui.h"

static SDL_Window *win;
static SDL_Renderer *ren;
static SDL_Texture *tex;
static u32 frame[320 * 200];
static int headless;
static int mouse_x = 160, mouse_y = 100, mouse_b;

/* ---------------------------------------------------------------- time */
static Uint64 t0, freq;
u32 plat_ticks_ms(void) { return (u32)((SDL_GetPerformanceCounter() - t0) * 1000 / freq); }
uint64_t plat_ticks_us(void) { return (uint64_t)((SDL_GetPerformanceCounter() - t0) * 1000000 / freq); }
void plat_sleep_ms(int ms) { SDL_Delay((Uint32)ms); }

static Uint32 tick_cb(Uint32 iv, void *p) { (void)p; poll_flag = 1; return iv; }

/* ---------------------------------------------------------------- script */
typedef struct { u32 t; char cmd[16]; char arg[200]; } Step;
static Step steps[512];
static int nsteps, step_i;

static void parse_script(const char *s)
{
    while (s && *s && nsteps < 512) {
        const char *e = strchr(s, ';');
        size_t n = e ? (size_t)(e - s) : strlen(s);
        char buf[256];
        if (n >= sizeof buf) n = sizeof buf - 1;
        memcpy(buf, s, n); buf[n] = 0;
        char *c1 = strchr(buf, ':');
        if (c1) {
            *c1 = 0;
            Step *st = &steps[nsteps++];
            st->t = (u32)atoi(buf);
            char *c2 = strchr(c1 + 1, ':');
            if (c2) { *c2 = 0; snprintf(st->arg, sizeof st->arg, "%s", c2 + 1); }
            snprintf(st->cmd, sizeof st->cmd, "%s", c1 + 1);
        }
        s = e ? e + 1 : 0;
    }
}

/* ---------------------------------------------------------------- keyboard map (XT set 1) */
static int xt_code(SDL_Scancode sc, int *ext)
{
    *ext = 0;
    if (sc >= SDL_SCANCODE_A && sc <= SDL_SCANCODE_Z) {
        static const u8 t[26] = {0x1e,0x30,0x2e,0x20,0x12,0x21,0x22,0x23,0x17,0x24,0x25,0x26,0x32,
                                 0x31,0x18,0x19,0x10,0x13,0x1f,0x14,0x16,0x2f,0x11,0x2d,0x15,0x2c};
        return t[sc - SDL_SCANCODE_A];
    }
    if (sc >= SDL_SCANCODE_1 && sc <= SDL_SCANCODE_0) return 2 + (sc - SDL_SCANCODE_1);
    if (sc >= SDL_SCANCODE_F1 && sc <= SDL_SCANCODE_F10) return 0x3b + (sc - SDL_SCANCODE_F1);
    switch (sc) {
    case SDL_SCANCODE_RETURN: return 0x1c;
    case SDL_SCANCODE_ESCAPE: return 0x01;
    case SDL_SCANCODE_BACKSPACE: return 0x0e;
    case SDL_SCANCODE_TAB: return 0x0f;
    case SDL_SCANCODE_SPACE: return 0x39;
    case SDL_SCANCODE_MINUS: return 0x0c;
    case SDL_SCANCODE_EQUALS: return 0x0d;
    case SDL_SCANCODE_LEFTBRACKET: return 0x1a;
    case SDL_SCANCODE_RIGHTBRACKET: return 0x1b;
    case SDL_SCANCODE_BACKSLASH: return 0x2b;
    case SDL_SCANCODE_SEMICOLON: return 0x27;
    case SDL_SCANCODE_APOSTROPHE: return 0x28;
    case SDL_SCANCODE_GRAVE: return 0x29;
    case SDL_SCANCODE_COMMA: return 0x33;
    case SDL_SCANCODE_PERIOD: return 0x34;
    case SDL_SCANCODE_SLASH: return 0x35;
    case SDL_SCANCODE_F11: return 0x57;
    case SDL_SCANCODE_F12: return 0x58;
    case SDL_SCANCODE_LSHIFT: return 0x2a;
    case SDL_SCANCODE_RSHIFT: return 0x36;
    case SDL_SCANCODE_LCTRL: return 0x1d;
    case SDL_SCANCODE_LALT: return 0x38;
    case SDL_SCANCODE_RCTRL: *ext = 1; return 0x1d;
    case SDL_SCANCODE_RALT: *ext = 1; return 0x38;
    case SDL_SCANCODE_CAPSLOCK: return 0x3a;
    case SDL_SCANCODE_KP_7: return 0x47;
    case SDL_SCANCODE_KP_8: return 0x48;
    case SDL_SCANCODE_KP_9: return 0x49;
    case SDL_SCANCODE_KP_MINUS: return 0x4a;
    case SDL_SCANCODE_KP_4: return 0x4b;
    case SDL_SCANCODE_KP_5: return 0x4c;
    case SDL_SCANCODE_KP_6: return 0x4d;
    case SDL_SCANCODE_KP_PLUS: return 0x4e;
    case SDL_SCANCODE_KP_1: return 0x4f;
    case SDL_SCANCODE_KP_2: return 0x50;
    case SDL_SCANCODE_KP_3: return 0x51;
    case SDL_SCANCODE_KP_0: return 0x52;
    case SDL_SCANCODE_KP_PERIOD: return 0x53;
    case SDL_SCANCODE_KP_ENTER: *ext = 1; return 0x1c;
    case SDL_SCANCODE_KP_DIVIDE: *ext = 1; return 0x35;
    case SDL_SCANCODE_KP_MULTIPLY: return 0x37;
    /* the game predates the separate cursor block: send the keypad codes */
    case SDL_SCANCODE_HOME: return 0x47;
    case SDL_SCANCODE_UP: return 0x48;
    case SDL_SCANCODE_PAGEUP: return 0x49;
    case SDL_SCANCODE_LEFT: return 0x4b;
    case SDL_SCANCODE_RIGHT: return 0x4d;
    case SDL_SCANCODE_END: return 0x4f;
    case SDL_SCANCODE_DOWN: return 0x50;
    case SDL_SCANCODE_PAGEDOWN: return 0x51;
    case SDL_SCANCODE_INSERT: return 0x52;
    case SDL_SCANCODE_DELETE: return 0x53;
    default: return -1;
    }
}

static void key(SDL_Scancode sc, int release)
{
    int ext, c = xt_code(sc, &ext);
    if (c < 0) return;
    if (ext) kbd_push_scancode(0xe0, 0);
    kbd_push_scancode((u8)c, release);
}

static void send_mouse(void) { mouse_update(mouse_x * 2, mouse_y, mouse_b); }

/* ---------------------------------------------------------------- screen */
static void to_rgb(const u8 *vga, const u8 *pal)
{
    u32 lut[256];
    for (int i = 0; i < 256; i++) {
        u32 r = pal[i * 3] & 63, g = pal[i * 3 + 1] & 63, b = pal[i * 3 + 2] & 63;
        lut[i] = 0xff000000u | (((r << 2) | (r >> 4)) << 16) | (((g << 2) | (g >> 4)) << 8) | ((b << 2) | (b >> 4));
    }
    for (int i = 0; i < 320 * 200; i++) frame[i] = lut[vga[i]];
}

static void save_bmp(const char *path)
{
    to_rgb(M + 0xa0000, vga_dac);
    FILE *f = fopen(path, "wb");
    if (!f) return;
    u8 h[54] = {'B', 'M'};
    u32 sz = 54 + 320 * 200 * 3;
    memcpy(h + 2, &sz, 4); h[10] = 54; h[14] = 40;
    s32 w = 320, hh = -200; memcpy(h + 18, &w, 4); memcpy(h + 22, &hh, 4);
    h[26] = 1; h[28] = 24;
    fwrite(h, 1, 54, f);
    for (int i = 0; i < 320 * 200; i++) { u8 px[3] = {(u8)frame[i], (u8)(frame[i] >> 8), (u8)(frame[i] >> 16)}; fwrite(px, 1, 3, f); }
    fclose(f);
    { int nz = 0, ps = 0; for (int i = 0; i < 64000; i++) nz += M[0xa0000 + i] != 0; for (int i = 0; i < 768; i++) ps += vga_dac[i];
      fprintf(stderr, "saved %s (mode %02x, %d non-zero pixels, palette sum %d)\n", path, video_mode, nz, ps); }
}

/* where the 320x200 picture goes in the window (renderer output pixels) */
static SDL_Rect dest;
static char pending_wshot[300];     /* whole-window screenshot (interface included) at the next present */
static int out_w = 960, out_h = 720, win_w = 960, win_h = 720;
static void layout(void)
{
    SDL_GetRendererOutputSize(ren, &out_w, &out_h);
    SDL_GetWindowSize(win, &win_w, &win_h);
    int ph = ui_settings.aspect ? 240 : 200;      /* picture height in 320-wide units */
    int w, h;
    if (ui_settings.integer && out_w >= 320 && out_h >= ph) {
        int k = out_w / 320; if (out_h / ph < k) k = out_h / ph;
        w = 320 * k; h = ph * k;
    } else {
        w = out_w; h = w * ph / 320;
        if (h > out_h) { h = out_h; w = h * 320 / ph; }
    }
    dest.x = (out_w - w) / 2; dest.y = (out_h - h) / 2; dest.w = w; dest.h = h;
}

void plat_present(const u8 *vga, const u8 *pal)
{
    if (headless) return;
    to_rgb(vga, pal);
    SDL_UpdateTexture(tex, 0, frame, 320 * 4);
    layout();
    SDL_SetTextureScaleMode(tex, ui_settings.smooth ? SDL_ScaleModeLinear : SDL_ScaleModeNearest);
    SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
    SDL_RenderClear(ren);
    SDL_RenderCopy(ren, tex, 0, &dest);
    ui_draw(ren);
    if (pending_wshot[0]) {
        SDL_Surface *sf = SDL_CreateRGBSurfaceWithFormat(0, out_w, out_h, 32, SDL_PIXELFORMAT_ARGB8888);
        if (sf) { SDL_RenderReadPixels(ren, 0, SDL_PIXELFORMAT_ARGB8888, sf->pixels, sf->pitch);
                  SDL_SaveBMP(sf, pending_wshot); SDL_FreeSurface(sf); fprintf(stderr, "saved %s\n", pending_wshot); }
        pending_wshot[0] = 0;
    }
    SDL_RenderPresent(ren);
}

/* window coordinates -> game pixels */
static void to_game(int wx, int wy, int *gx, int *gy)
{
    int ox = win_w ? wx * out_w / win_w : wx, oy = win_h ? wy * out_h / win_h : wy;
    int x = dest.w ? (ox - dest.x) * 320 / dest.w : 0, y = dest.h ? (oy - dest.y) * 200 / dest.h : 0;
    if (x < 0) x = 0; if (x > 319) x = 319; if (y < 0) y = 0; if (y > 199) y = 199;
    *gx = x; *gy = y;
}

int plat_menu_open(void) { return !headless && ui_is_open(); }

/* ---------------------------------------------------------------- events */
static void run_script(void)
{
    u32 now = plat_ticks_ms();
    if (step_i && step_i == nsteps) step_i = nsteps = 0;
    while (step_i < nsteps && steps[step_i].t <= now) {
        Step *s = &steps[step_i++];
        int x = 0, y = 0;
        sscanf(s->arg, "%d,%d", &x, &y);
        if (!strcmp(s->cmd, "key") || !strcmp(s->cmd, "down") || !strcmp(s->cmd, "up")) {
            SDL_Scancode sc = SDL_GetScancodeFromName(s->arg);
            if (sc == SDL_SCANCODE_UNKNOWN) { fprintf(stderr, "script: unknown key %s\n", s->arg); continue; }
            if (getenv("DL_TRACE_INPUT")) fprintf(stderr, "script %u ms: %s %s (sc %d)\n", now, s->cmd, s->arg, (int)sc);
            if (sc == SDL_SCANCODE_F12 && !headless) { ui_toggle(); continue; }
            if (strcmp(s->cmd, "up")) key(sc, 0);
            if (!strcmp(s->cmd, "up")) key(sc, 1);
            else if (!strcmp(s->cmd, "key")) {      /* release a little later, like a real key press */
                Step *r = &steps[nsteps < 512 ? nsteps++ : 511];
                r->t = now + 60; snprintf(r->cmd, sizeof r->cmd, "up"); snprintf(r->arg, sizeof r->arg, "%s", s->arg);
                for (int i = nsteps - 1; i > step_i && steps[i].t < steps[i - 1].t; i--) { Step t = steps[i]; steps[i] = steps[i - 1]; steps[i - 1] = t; }
            }
        } else if (!strcmp(s->cmd, "move")) { mouse_x = x; mouse_y = y; send_mouse(); }
        else if (!strcmp(s->cmd, "click") || !strcmp(s->cmd, "rclick")) {
            int b = s->cmd[0] == 'r' ? 2 : 1;
            mouse_x = x; mouse_y = y; send_mouse();
            mouse_b |= b; send_mouse();
            /* release on a later poll so the game sees the press */
            Step *r = &steps[nsteps < 512 ? nsteps++ : 511];
            r->t = now + 80; snprintf(r->cmd, sizeof r->cmd, "release"); snprintf(r->arg, sizeof r->arg, "%d", b);
            /* keep the list sorted by time */
            for (int i = nsteps - 1; i > step_i && steps[i].t < steps[i - 1].t; i--) { Step t = steps[i]; steps[i] = steps[i - 1]; steps[i - 1] = t; }
        } else if (!strcmp(s->cmd, "release")) { mouse_b &= ~atoi(s->arg); send_mouse(); }
        else if (!strcmp(s->cmd, "shot")) save_bmp(s->arg);
        else if (!strcmp(s->cmd, "utab")) ui_select_tab(atoi(s->arg));
        else if (!strcmp(s->cmd, "uclick")) {            /* a click on the interface, window pixels */
            SDL_Event ev; memset(&ev, 0, sizeof ev);
            Uint32 wid = win ? SDL_GetWindowID(win) : 0;
            if (win) SDL_WarpMouseInWindow(win, x, y);
            ev.type = SDL_MOUSEMOTION; ev.motion.windowID = wid; ev.motion.x = x; ev.motion.y = y; SDL_PushEvent(&ev);
            memset(&ev, 0, sizeof ev); ev.type = SDL_MOUSEBUTTONDOWN; ev.button.windowID = wid; ev.button.button = SDL_BUTTON_LEFT; ev.button.state = SDL_PRESSED;
            ev.button.x = x; ev.button.y = y; ev.button.clicks = 1; SDL_PushEvent(&ev);
            ev.type = SDL_MOUSEBUTTONUP; ev.button.state = SDL_RELEASED; SDL_PushEvent(&ev);
        }
        else if (!strcmp(s->cmd, "wshot")) snprintf(pending_wshot, sizeof pending_wshot, "%s", s->arg);
        else if (!strcmp(s->cmd, "quit")) plat_quit(0);
    }
}

/* DL_CTL=<file>: an external driver appends script steps ("cmd:arg", one per line); they run
 * at once. Used for interactive automated play (port/tools/nctl.py). */
static void run_ctl(void)
{
    static const char *path; static long pos; static int init;
    if (!init) { init = 1; path = getenv("DL_CTL"); }
    if (!path) return;
    FILE *f = fopen(path, "rb");
    if (!f) return;
    fseek(f, pos, SEEK_SET);
    char line[300];
    while (fgets(line, sizeof line, f)) {
        size_t n = strlen(line);
        if (!n || line[n - 1] != '\n') break;          /* incomplete line: later */
        pos += (long)n;
        line[n - 1] = 0;
        char *c = strchr(line, ':');
        if (!c || nsteps >= 512) continue;
        *c = 0;
        /* insert as an immediate step */
        Step st; st.t = 0; snprintf(st.cmd, sizeof st.cmd, "%s", line); snprintf(st.arg, sizeof st.arg, "%s", c + 1);
        memmove(&steps[step_i + 1], &steps[step_i], (size_t)(nsteps - step_i) * sizeof(Step));
        steps[step_i] = st; nsteps++;
    }
    fclose(f);
}

void plat_events(void)
{
    SDL_Event e;
    run_ctl();
    run_script();
    while (SDL_PollEvent(&e)) {
        if (!headless && ui_event(&e)) {
            SDL_ShowCursor(ui_is_open() ? SDL_ENABLE : SDL_DISABLE);
            continue;
        }
        switch (e.type) {
        case SDL_QUIT: plat_quit(0); break;
        case SDL_KEYDOWN:
            if (e.key.keysym.sym == SDLK_RETURN && (e.key.keysym.mod & KMOD_ALT)) {
                Uint32 fl = SDL_GetWindowFlags(win);
                SDL_SetWindowFullscreen(win, (fl & SDL_WINDOW_FULLSCREEN_DESKTOP) ? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP);
                break;
            }
            if (e.key.keysym.sym == SDLK_F12 && (e.key.keysym.mod & KMOD_CTRL)) { save_bmp("screenshot.bmp"); break; }
            key(e.key.keysym.scancode, 0);
            break;
        case SDL_KEYUP: key(e.key.keysym.scancode, 1); break;
        case SDL_MOUSEMOTION:
            to_game(e.motion.x, e.motion.y, &mouse_x, &mouse_y);
            send_mouse(); break;
        case SDL_MOUSEBUTTONDOWN: case SDL_MOUSEBUTTONUP: {
            int b = e.button.button == SDL_BUTTON_LEFT ? 1 : e.button.button == SDL_BUTTON_RIGHT ? 2 : 4;
            if (e.type == SDL_MOUSEBUTTONDOWN) mouse_b |= b; else mouse_b &= ~b;
            send_mouse(); break; }
        default: break;
        }
    }
}

/* ---------------------------------------------------------------- init / quit */
void plat_init(int scale)
{
    headless = getenv("DL_HEADLESS") != 0;
    if (headless) SDL_setenv("SDL_VIDEODRIVER", "dummy", 1);
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS) < 0) {
        fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        exit(1);
    }
    t0 = SDL_GetPerformanceCounter(); freq = SDL_GetPerformanceFrequency();
    if (!scale) return;
    if (!headless) {
        int wp = getenv("DL_SCRIPT") ? 0 : SDL_WINDOWPOS_CENTERED;   /* scripted tests: a known window position */
        win = SDL_CreateWindow("Darklands", wp, wp, 960, 720,
                               SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
        if (!win) { fprintf(stderr, "SDL_CreateWindow: %s\n", SDL_GetError()); exit(1); }
        ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
        if (!ren) ren = SDL_CreateRenderer(win, -1, 0);
        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
        tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 320, 200);
        SDL_ShowCursor(SDL_DISABLE);
        ui_init(win, ren);
    }
    SDL_AddTimer(1, tick_cb, 0);
    parse_script(getenv("DL_SCRIPT"));
}

void plat_quit(int code)
{
    fflush(stdout); fflush(stderr);
    SDL_Quit();
    exit(code);
}

void plat_error_box(const char *msg)
{
    char buf[700];
    snprintf(buf, sizeof buf, "The game stopped because of an internal error:\n\n%s\n\n"
             "Details are in darklands_port.log in the game folder.", msg);
    if (!headless) SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Darklands", buf, win);
}
