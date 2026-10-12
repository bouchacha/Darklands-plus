/* prepare_run.c - the first start of a release (DL_RTSPLIT): the game module is made from the
 * player's DARKLAND.EXE by port/tools/prepare.py (the bundled Python and TinyCC), with a
 * progress window. Where the preparer lives:
 *   DL_PREPARE_PY / DL_PREPARE_SCRIPT       given
 *   <program>/darklands_plus/python/python.exe, <program>/darklands_plus/src/port/tools/prepare.py
 *                                           a release (port/tools/package.py)
 *   python3, <program>/../../port/tools/prepare.py   the repository (build/port_split) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL.h>
#include "cpu.h"
#include "hle.h"

#ifdef DL_RTSPLIT
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#endif

void ui_progress_draw(SDL_Renderer *ren, const char *step, int pct, const char *note);
SDL_Renderer *plat_renderer(void);

static int exists(const char *p) { FILE *f = fopen(p, "rb"); if (f) fclose(f); return f != 0; }

static int find_preparer(char *py, char *script, int n)
{
    const char *ep = getenv("DL_PREPARE_PY"), *es = getenv("DL_PREPARE_SCRIPT");
    if (ep && es) { snprintf(py, n, "%s", ep); snprintf(script, n, "%s", es); return 1; }
    char *base = SDL_GetBasePath();
    const char *b = base ? base : "./";
#ifdef _WIN32
    snprintf(py, n, "%sdarklands_plus/python/python.exe", b);
#else
    snprintf(py, n, "python3");
#endif
    snprintf(script, n, "%sdarklands_plus/src/port/tools/prepare.py", b);
    int ok = exists(script);
    if (!ok) { snprintf(script, n, "%s../../port/tools/prepare.py", b); ok = exists(script); }
#ifndef _WIN32
    if (ok && exists("/usr/bin/python3")) snprintf(py, n, "/usr/bin/python3");
#endif
    if (base) SDL_free(base);
    return ok;
}

/* one line of the preparer's output: "@@ pct step" moves the bar, the rest goes to the log */
static void take_line(char *l, int *pct, char *step, int stepn)
{
    fprintf(stderr, "prepare: %s\n", l);
    if (l[0] == '@' && l[1] == '@') {
        int p = 0, k = 0;
        if (sscanf(l + 2, "%d %n", &p, &k) >= 1) { *pct = p; snprintf(step, stepn, "%s", l + 2 + k); }
    }
}

/* make the module at module_path; 1 done */
int prepare_run(const char *game_dir, const char *module_path, char *err, int errn)
{
    char py[1200], script[1200];
    if (!find_preparer(py, script, sizeof py)) {
        snprintf(err, errn, "The game module (%s) is missing, and the files that make it (darklands_plus folder) are not next to the program.", module_path);
        return 0;
    }
    fprintf(stderr, "preparing the game module: %s %s\n", py, script);
    SDL_Renderer *ren = plat_renderer();
    int pct = 0, code = -1, quit = 0;
    char step[256] = "Starting", buf[4096], line[2048]; int ln = 0;
    u32 last_draw = 0;
#ifdef _WIN32
    char cmd[6000];
    snprintf(cmd, sizeof cmd, "\"%s\" -I \"%s\" --game \"%s\" --out \"%s\"", py, script, game_dir, module_path);
    SECURITY_ATTRIBUTES sa = { sizeof sa, NULL, TRUE };
    HANDLE rd, wr;
    if (!CreatePipe(&rd, &wr, &sa, 0)) { snprintf(err, errn, "cannot start the preparation (pipe)"); return 0; }
    SetHandleInformation(rd, HANDLE_FLAG_INHERIT, 0);
    STARTUPINFOA si; memset(&si, 0, sizeof si); si.cb = sizeof si;
    si.dwFlags = STARTF_USESTDHANDLES; si.hStdOutput = wr; si.hStdError = wr; si.hStdInput = NULL;
    PROCESS_INFORMATION pi;
    if (!CreateProcessA(NULL, cmd, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        snprintf(err, errn, "cannot start %s (error %lu)", py, (unsigned long)GetLastError());
        CloseHandle(rd); CloseHandle(wr); return 0;
    }
    CloseHandle(wr);
    for (;;) {
        DWORD avail = 0, got = 0;
        int done = WaitForSingleObject(pi.hProcess, 0) == WAIT_OBJECT_0;
        while (PeekNamedPipe(rd, NULL, 0, NULL, &avail, NULL) && avail > 0 && ReadFile(rd, buf, avail < sizeof buf ? avail : sizeof buf, &got, NULL) && got > 0)
            for (DWORD k = 0; k < got; k++) {
                char c = buf[k];
                if (c == '\n' || ln == (int)sizeof line - 1) { line[ln] = 0; if (ln && line[ln - 1] == '\r') line[ln - 1] = 0; take_line(line, &pct, step, sizeof step); ln = 0; }
                else line[ln++] = c;
            }
        if (done) { DWORD ec = 1; GetExitCodeProcess(pi.hProcess, &ec); code = (int)ec; break; }
#else
    int fd[2];
    if (pipe(fd) != 0) { snprintf(err, errn, "cannot start the preparation (pipe)"); return 0; }
    pid_t pid = fork();
    if (pid == 0) {
        dup2(fd[1], 1); dup2(fd[1], 2); close(fd[0]); close(fd[1]);
        execlp(py, py, "-I", script, "--game", game_dir, "--out", module_path, (char *)0);
        _exit(127);
    }
    close(fd[1]);
    fcntl(fd[0], F_SETFL, O_NONBLOCK);
    for (;;) {
        int st = 0;
        ssize_t got;
        while ((got = read(fd[0], buf, sizeof buf)) > 0)
            for (ssize_t k = 0; k < got; k++) {
                char c = buf[k];
                if (c == '\n' || ln == (int)sizeof line - 1) { line[ln] = 0; take_line(line, &pct, step, sizeof step); ln = 0; }
                else line[ln++] = c;
            }
        if (waitpid(pid, &st, WNOHANG) == pid) {
            while ((got = read(fd[0], buf, sizeof buf)) > 0) fwrite(buf, 1, (size_t)got, stderr);
            code = WIFEXITED(st) ? WEXITSTATUS(st) : 1; close(fd[0]); break;
        }
#endif
        SDL_Event e;
        while (SDL_PollEvent(&e)) if (e.type == SDL_QUIT) quit = 1;
        if (quit) {
#ifdef _WIN32
            TerminateProcess(pi.hProcess, 1);
#else
            kill(pid, SIGTERM);
#endif
            fprintf(stderr, "preparation stopped by the player\n");
            exit(0);
        }
        if (ren && SDL_GetTicks() - last_draw > 50) { ui_progress_draw(ren, step, pct, 0); last_draw = SDL_GetTicks(); }
        SDL_Delay(15);
    }
#ifdef _WIN32
    CloseHandle(pi.hProcess); CloseHandle(pi.hThread); CloseHandle(rd);
#endif
    if (code != 0) {
        snprintf(err, errn, "The preparation failed (%s; exit %d). The details are in darklands_port.log.", step, code);
        return 0;
    }
    return 1;
}
#endif
