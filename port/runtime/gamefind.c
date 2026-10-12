/* gamefind.c - where the game is when darklands.exe is not in the Darklands folder: a folder
 * chosen before (darklands_plus/game_folder.txt next to the program), the GOG and Steam
 * installations the registry knows, the usual places, and on Windows a folder dialog. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#endif

static int exists(const char *p) { FILE *f = fopen(p, "rb"); if (f) fclose(f); return f != 0; }

/* dir, or a folder in it (GOG: DARKLAND), holding DARKLAND.EXE */
static int has_game(const char *dir, char *out, int n)
{
    char p[1400];
    if (!dir || !*dir) return 0;
    snprintf(p, sizeof p, "%s/DARKLAND.EXE", dir);
    if (exists(p)) { snprintf(out, n, "%s", dir); return 1; }
    snprintf(p, sizeof p, "%s/DARKLAND/DARKLAND.EXE", dir);
    if (exists(p)) { snprintf(out, n, "%s/DARKLAND", dir); return 1; }
#ifdef _WIN32
    char pat[1400]; WIN32_FIND_DATAA fd;
    snprintf(pat, sizeof pat, "%s\\*", dir);
    HANDLE h = FindFirstFileA(pat, &fd);
    if (h != INVALID_HANDLE_VALUE) {
        do {
            if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) || fd.cFileName[0] == '.') continue;
            snprintf(p, sizeof p, "%s\\%s\\DARKLAND.EXE", dir, fd.cFileName);
            if (exists(p)) { snprintf(out, n, "%s\\%s", dir, fd.cFileName); FindClose(h); return 1; }
        } while (FindNextFileA(h, &fd));
        FindClose(h);
    }
#endif
    return 0;
}

static void remember(const char *exedir, const char *dir)
{
    char p[1400]; snprintf(p, sizeof p, "%s/darklands_plus", exedir);
#ifdef _WIN32
    CreateDirectoryA(p, NULL);
#else
    mkdir(p, 0755);
#endif
    snprintf(p, sizeof p, "%s/darklands_plus/game_folder.txt", exedir);
    FILE *f = fopen(p, "w");
    if (f) { fprintf(f, "%s\n", dir); fclose(f); }
}

#ifdef _WIN32
static int reg_str(HKEY root, const char *key, const char *name, char *out, DWORD n)
{
    HKEY h; DWORD type, len = n;
    if (RegOpenKeyExA(root, key, 0, KEY_READ, &h) != ERROR_SUCCESS) return 0;
    LONG r = RegQueryValueExA(h, name, 0, &type, (BYTE *)out, &len);
    RegCloseKey(h);
    if (r != ERROR_SUCCESS || type != REG_SZ) return 0;
    out[n - 1] = 0;
    return 1;
}

/* GOG: SOFTWARE\(WOW6432Node\)GOG.com\Games\<id> with gameName "Darklands" and path */
static int from_gog(char *out, int n)
{
    static const char *roots[] = { "SOFTWARE\\WOW6432Node\\GOG.com\\Games", "SOFTWARE\\GOG.com\\Games" };
    for (int r = 0; r < 2; r++) {
        HKEY h;
        if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, roots[r], 0, KEY_READ, &h) != ERROR_SUCCESS) continue;
        char sub[256];
        for (DWORD i = 0; ; i++) {
            DWORD sn = sizeof sub;
            if (RegEnumKeyExA(h, i, sub, &sn, 0, 0, 0, 0) != ERROR_SUCCESS) break;
            char key[600], name[256], path[1024];
            snprintf(key, sizeof key, "%s\\%s", roots[r], sub);
            if ((reg_str(HKEY_LOCAL_MACHINE, key, "gameName", name, sizeof name) || reg_str(HKEY_LOCAL_MACHINE, key, "GAMENAME", name, sizeof name))
                && !_stricmp(name, "Darklands")
                && (reg_str(HKEY_LOCAL_MACHINE, key, "path", path, sizeof path) || reg_str(HKEY_LOCAL_MACHINE, key, "PATH", path, sizeof path))
                && has_game(path, out, n)) { RegCloseKey(h); return 1; }
        }
        RegCloseKey(h);
    }
    return 0;
}

/* Steam: <Steam>\steamapps\common\Darklands, in every library of libraryfolders.vdf */
static int from_steam(char *out, int n)
{
    char steam[1024], p[1400];
    if (!reg_str(HKEY_LOCAL_MACHINE, "SOFTWARE\\WOW6432Node\\Valve\\Steam", "InstallPath", steam, sizeof steam)
        && !reg_str(HKEY_CURRENT_USER, "SOFTWARE\\Valve\\Steam", "SteamPath", steam, sizeof steam)) return 0;
    snprintf(p, sizeof p, "%s\\steamapps\\common\\Darklands", steam);
    if (has_game(p, out, n)) return 1;
    snprintf(p, sizeof p, "%s\\steamapps\\libraryfolders.vdf", steam);
    FILE *f = fopen(p, "r");
    if (!f) return 0;
    char line[1400];
    while (fgets(line, sizeof line, f)) {
        char *k = strstr(line, "\"path\"");
        if (!k) continue;
        char *a = strchr(k + 6, '"'), *b = a ? strchr(a + 1, '"') : 0;
        if (!a || !b) continue;
        *b = 0;
        char lib[1024]; int j = 0;
        for (char *s = a + 1; *s && j < (int)sizeof lib - 1; s++) { if (s[0] == '\\' && s[1] == '\\') s++; lib[j++] = *s; }
        lib[j] = 0;
        snprintf(p, sizeof p, "%s\\steamapps\\common\\Darklands", lib);
        if (has_game(p, out, n)) { fclose(f); return 1; }
    }
    fclose(f);
    return 0;
}

static int choose(char *out, int n)
{
    CoInitialize(NULL);
    for (;;) {
        BROWSEINFOA bi; memset(&bi, 0, sizeof bi);
        char disp[MAX_PATH];
        bi.pszDisplayName = disp;
        bi.lpszTitle = "Choose your Darklands folder (the one that contains DARKLAND.EXE):";
        bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
        PIDLIST_ABSOLUTE id = SHBrowseForFolderA(&bi);
        if (!id) return 0;
        char path[MAX_PATH] = "";
        SHGetPathFromIDListA(id, path);
        CoTaskMemFree(id);
        if (has_game(path, out, n)) return 1;
        if (MessageBoxA(NULL, "There is no DARKLAND.EXE in that folder. Choose another?", "Darklands+", MB_RETRYCANCEL | MB_ICONWARNING) != IDRETRY) return 0;
    }
}
#endif

/* a folder chosen by the player (Windows' folder dialog); 0 if none or not available */
int plat_choose_folder(const char *title, char *out, int n)
{
#ifdef _WIN32
    CoInitialize(NULL);
    BROWSEINFOA bi; memset(&bi, 0, sizeof bi);
    char disp[MAX_PATH];
    bi.pszDisplayName = disp;
    bi.lpszTitle = title;
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
    PIDLIST_ABSOLUTE id = SHBrowseForFolderA(&bi);
    if (!id) return 0;
    char path[MAX_PATH] = "";
    int ok = SHGetPathFromIDListA(id, path) && path[0];
    CoTaskMemFree(id);
    if (ok) snprintf(out, (size_t)n, "%s", path);
    return ok;
#else
    (void)title; (void)out; (void)n;
    return 0;
#endif
}

/* 1 found (out: the folder with DARKLAND.EXE) */
int find_game_more(const char *exedir, char *out, int n, int interactive)
{
    char p[1400], line[1100];
    snprintf(p, sizeof p, "%s/darklands_plus/game_folder.txt", exedir);
    FILE *f = fopen(p, "r");
    if (f) {
        if (fgets(line, sizeof line, f)) { line[strcspn(line, "\r\n")] = 0; if (has_game(line, out, n)) { fclose(f); return 1; } }
        fclose(f);
    }
#ifdef _WIN32
    if (from_gog(out, n) || from_steam(out, n)) { remember(exedir, out); return 1; }
    static const char *usual[] = { "C:\\GOG Games\\Darklands", "C:\\Program Files (x86)\\GOG Galaxy\\Games\\Darklands",
                                   "C:\\Program Files (x86)\\GOG.com\\Darklands", "C:\\Program Files\\GOG Galaxy\\Games\\Darklands",
                                   "D:\\GOG Games\\Darklands", "C:\\Games\\Darklands", "D:\\Games\\Darklands", 0 };
    for (int i = 0; usual[i]; i++) if (has_game(usual[i], out, n)) { remember(exedir, out); return 1; }
    if (interactive) {
        MessageBoxA(NULL, "Darklands+ could not find your Darklands folder by itself.\n\nPlease choose the folder that contains "
                    "DARKLAND.EXE. (Or put darklands.exe and the darklands_plus folder into that folder.)", "Darklands+", MB_OK | MB_ICONINFORMATION);
        if (choose(out, n)) { remember(exedir, out); return 1; }
    }
#else
    (void)interactive;
    const char *home = getenv("HOME");
    if (home) {
        snprintf(p, sizeof p, "%s/GOG Games/Darklands", home);
        if (has_game(p, out, n)) { remember(exedir, out); return 1; }
        snprintf(p, sizeof p, "%s/.steam/steam/steamapps/common/Darklands", home);
        if (has_game(p, out, n)) { remember(exedir, out); return 1; }
    }
#endif
    return 0;
}
