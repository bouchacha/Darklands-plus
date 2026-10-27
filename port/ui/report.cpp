// report.cpp - "Report a problem" (F12 > About): what a bug report on GitHub needs, in one zip - this session's
// log and the last one's, the settings, the two newest saves (each .SAV with its .BSV) and a short summary (the
// build, the system, the preset) - the user's own name taken out of the paths in them; then the zip's folder and
// GitHub's bug form opened, the form's version, system and preset filled in (.github/ISSUE_TEMPLATE/bug_report.yml,
// its fields by id). GitHub takes a file only from someone signed in: the player drags the zip into the form.
// The zip is written here (deflate from stb_image_write's zlib, its header and checksum left off; CRC-32 below).
#include "imgui.h"
#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sys/stat.h>
#include <string>
#include <vector>
#include <algorithm>
#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#endif
extern "C" {
#include "../mods/saves.h"
#include "../mods/options.h"
unsigned char *stbi_zlib_compress(unsigned char *data, int data_len, int *out_len, int quality);   /* runtime/stb_impl.c */
void dos_host_path_read(const char *dos, char *out, size_t outn);                                    /* runtime/dos.c */
void dos_mkdirs_for(const char *path);
extern char dlp_data_dir[];
extern const char dlp_version[];
}

namespace {
const char *REPO_FORM = "https://github.com/bouchacha/Darklands-plus/issues/new?template=bug_report.yml";

bool read_file(const std::string &path, std::string &out)
{
    FILE *f = fopen(path.c_str(), "rb");
    if (!f) return false;
    out.clear(); char b[65536]; size_t n;
    while ((n = fread(b, 1, sizeof b, f)) > 0) out.append(b, n);
    fclose(f);
    return true;
}
/* the user's own name out of a text: the home folder, then the name wherever a path holds it */
void replace_all(std::string &s, const std::string &from, const std::string &to, bool nocase)
{
    if (from.size() < 2) return;
    for (size_t p = 0; ; ) {
        size_t q = std::string::npos;
        if (nocase) {
            auto it = std::search(s.begin() + (long)p, s.end(), from.begin(), from.end(), [](char a, char b) { return tolower((unsigned char)a) == tolower((unsigned char)b); });
            if (it != s.end()) q = (size_t)(it - s.begin());
        } else q = s.find(from, p);
        if (q == std::string::npos) break;
        s.replace(q, from.size(), to); p = q + to.size();
    }
}
std::string scrub(std::string s)
{
#ifdef _WIN32
    const char *home = getenv("USERPROFILE"), *user = getenv("USERNAME"); const bool nocase = true;
#else
    const char *home = getenv("HOME"), *user = getenv("USER"); const bool nocase = false;
#endif
    if (home && *home) {
        std::string h = home, hs = h;
        for (char &c : hs) if (c == '\\') c = '/';
        replace_all(s, h, "~", nocase); replace_all(s, hs, "~", nocase);
    }
    if (user && *user) {
        std::string u = user;
        for (const char *pre : { "Users\\", "Users/", "home/", "users\\", "users/" }) replace_all(s, std::string(pre) + u, std::string(pre) + "(user)", nocase);
    }
    return s;
}

uint32_t crc32_of(const std::string &d)
{
    static uint32_t T[256]; static bool made;
    if (!made) { for (uint32_t i = 0; i < 256; i++) { uint32_t c = i; for (int k = 0; k < 8; k++) c = c & 1 ? 0xEDB88320u ^ (c >> 1) : c >> 1; T[i] = c; } made = true; }
    uint32_t c = 0xFFFFFFFFu;
    for (unsigned char ch : d) c = T[(c ^ ch) & 255] ^ (c >> 8);
    return c ^ 0xFFFFFFFFu;
}
struct Zip {
    FILE *f = nullptr;
    struct Ent { std::string name; uint32_t crc, csize, usize, off; uint16_t method, time, date; };
    std::vector<Ent> ents;
    void w16(uint16_t v) { fputc(v & 255, f); fputc(v >> 8, f); }
    void w32(uint32_t v) { w16((uint16_t)(v & 0xffff)); w16((uint16_t)(v >> 16)); }
    void add(const std::string &name, const std::string &data)
    {
        time_t now = time(nullptr); struct tm *t = localtime(&now);
        Ent e; e.name = name; e.crc = crc32_of(data); e.usize = (uint32_t)data.size(); e.off = (uint32_t)ftell(f);
        e.time = (uint16_t)((t->tm_hour << 11) | (t->tm_min << 5) | (t->tm_sec / 2)); e.date = (uint16_t)(((t->tm_year - 80) << 9) | ((t->tm_mon + 1) << 5) | t->tm_mday);
        int zl = 0;
        unsigned char *z = data.empty() ? nullptr : stbi_zlib_compress((unsigned char *)data.data(), (int)data.size(), &zl, 8);
        std::string body;
        if (z && zl > 6 && (size_t)(zl - 6) < data.size()) { body.assign((const char *)z + 2, (size_t)zl - 6); e.method = 8; }   /* (raw deflate: the zlib header and Adler-32 off) */
        else { body = data; e.method = 0; }
        if (z) free(z);
        e.csize = (uint32_t)body.size();
        w32(0x04034b50); w16(20); w16(0); w16(e.method); w16(e.time); w16(e.date); w32(e.crc); w32(e.csize); w32(e.usize);
        w16((uint16_t)name.size()); w16(0); fwrite(name.data(), 1, name.size(), f);
        fwrite(body.data(), 1, body.size(), f);
        ents.push_back(e);
    }
    void close()
    {
        uint32_t cd = (uint32_t)ftell(f);
        for (const Ent &e : ents) {
            w32(0x02014b50); w16(20); w16(20); w16(0); w16(e.method); w16(e.time); w16(e.date); w32(e.crc); w32(e.csize); w32(e.usize);
            w16((uint16_t)e.name.size()); w16(0); w16(0); w16(0); w16(0); w32(0); w32(e.off);
            fwrite(e.name.data(), 1, e.name.size(), f);
        }
        uint32_t cl = (uint32_t)ftell(f) - cd;
        w32(0x06054b50); w16(0); w16(0); w16((uint16_t)ents.size()); w16((uint16_t)ents.size()); w32(cl); w32(cd); w16(0);
        fclose(f); f = nullptr;
    }
};

/* the system, as the bug form asks it */
std::string system_name(void)
{
#ifdef _WIN32
    typedef const char *(*WineVer)(void);
    HMODULE nt = GetModuleHandleA("ntdll.dll");
    if (nt && GetProcAddress(nt, "wine_get_version")) { WineVer v = (WineVer)(void *)GetProcAddress(nt, "wine_get_version"); return std::string("Linux (Wine or Proton ") + v() + ")"; }
    typedef LONG (WINAPI *RtlVer)(OSVERSIONINFOW *);
    RtlVer rv = nt ? (RtlVer)(void *)GetProcAddress(nt, "RtlGetVersion") : nullptr;
    OSVERSIONINFOW vi; memset(&vi, 0, sizeof vi); vi.dwOSVersionInfoSize = sizeof vi;
    if (rv && rv(&vi) == 0) {
        char b[64];
        if (vi.dwMajorVersion == 10) snprintf(b, sizeof b, "Windows %s (build %lu)", vi.dwBuildNumber >= 22000 ? "11" : "10", (unsigned long)vi.dwBuildNumber);
        else snprintf(b, sizeof b, "Windows %lu.%lu", (unsigned long)vi.dwMajorVersion, (unsigned long)vi.dwMinorVersion);
        return b;
    }
    return "Windows";
#else
    return std::string(SDL_GetPlatform()) + " (native)";
#endif
}
std::string preset_words(void)
{
    int p = preset_current();
    if (p >= 0 && p < PRESET_COUNT) return PRESETS[p].name;
    int d = 0, n = preset_nearest(&d);
    char b[120]; snprintf(b, sizeof b, "own switches (%d from %s)", d, n >= 0 && n < PRESET_COUNT ? PRESETS[n].name : "?");
    return b;
}
std::string url_encode(const std::string &s)
{
    std::string o; char b[4];
    for (unsigned char c : s) {
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') o += (char)c;
        else { snprintf(b, sizeof b, "%%%02X", c); o += b; }
    }
    return o;
}

std::string made_zip, made_note;    /* the last report: its zip, or what went wrong */
std::string form_url(void)
{
    std::string sys = system_name(), files = "The report from Darklands+ (F12 > About > Report a problem): drag " +
        made_zip.substr(made_zip.find_last_of("/\\") + 1) + " here from the folder it opened. It holds the log, the settings and the two newest saves.";
    return std::string(REPO_FORM) + "&version=" + url_encode(dlp_version) + "&system=" + url_encode(sys) + "&preset=" + url_encode(preset_words()) + "&files=" + url_encode(files);
}
void open_folder(const std::string &file)
{
#ifdef _WIN32
    std::string args = "/select,\"" + file + "\"";
    ShellExecuteA(nullptr, "open", "explorer.exe", args.c_str(), nullptr, SW_SHOWNORMAL);
#else
    std::string dir = file.substr(0, file.find_last_of('/'));
    SDL_OpenURL(("file://" + dir).c_str());
#endif
}

bool make_report(void)
{
    fflush(stderr);
    time_t now = time(nullptr); struct tm *t = localtime(&now);
    char stamp[32]; strftime(stamp, sizeof stamp, "%Y%m%d-%H%M%S", t);
    std::string dir = std::string(dlp_data_dir) + "/reports", zip = dir + "/darklands-report-" + stamp + ".zip";
#ifdef _WIN32
    for (char &c : zip) if (c == '/') c = '\\';
#endif
    dos_mkdirs_for(zip.c_str());
    Zip z; z.f = fopen(zip.c_str(), "wb");
    if (!z.f) { made_note = "Could not write " + zip + "."; made_zip.clear(); return false; }
    std::string d, summary;
    for (const char *log : { "darklands_port.log", "darklands_port_previous.log", "darklands_port.ini" })
        if (read_file(std::string(dlp_data_dir) + "/" + log, d)) z.add(log, scrub(d));
    SaveInfo *v = nullptr; int n = saves_list(&v);
    std::sort(v, v + n, [](const SaveInfo &a, const SaveInfo &b) { return a.mtime > b.mtime; });
    std::string saves;
    for (int i = 0; i < n && i < 2; i++) {
        char dos[40], host[1200];
        snprintf(dos, sizeof dos, "SAVES\\%s", v[i].file); dos_host_path_read(dos, host, sizeof host);
        if (read_file(host, d)) { z.add(std::string("SAVES/") + v[i].file, d); saves += std::string(saves.empty() ? "" : ", ") + v[i].file; }
        std::string b = v[i].file; size_t dot = b.rfind('.'); if (dot != std::string::npos) b = b.substr(0, dot) + ".BSV";
        snprintf(dos, sizeof dos, "SAVES\\%s", b.c_str()); dos_host_path_read(dos, host, sizeof host);
        if (read_file(host, d)) z.add("SAVES/" + b, d);
    }
    free(v);
    char tb[64]; strftime(tb, sizeof tb, "%Y-%m-%d %H:%M", t);
    summary = std::string("Darklands+ build ") + dlp_version + "\nSystem: " + system_name() + "\nPreset: " + preset_words() +
              "\nMade: " + tb + "\nSaves: " + (saves.empty() ? "none" : saves) + "\n";
    z.add("report.txt", summary);
    z.close();
    made_zip = zip; made_note.clear();
    fprintf(stderr, "report: %s\n", scrub(zip).c_str());
    return true;
}
}

/* F12 > About: the section and its button */
void ui_report_section(void)
{
    ImGui::TextWrapped("Something wrong? Make a report: a zip of this session's log and the last one's, your settings and your two "
                       "newest saves (your own name taken out of the folder paths in them). Its folder opens, and GitHub's bug form "
                       "with the version, system and preset filled in: write what happened and drag the zip into the form. "
                       "Posting there takes a free GitHub account.");
    if (ImGui::Button("Make a report and open the form")) {
        if (make_report()) { open_folder(made_zip); SDL_OpenURL(form_url().c_str()); }
    }
    if (!made_zip.empty()) {
        ImGui::TextDisabled("Made: %s", made_zip.c_str());
        if (ImGui::SmallButton("Show the zip")) open_folder(made_zip);
        ImGui::SameLine();
        if (ImGui::SmallButton("Open the form again")) SDL_OpenURL(form_url().c_str());
    }
    if (!made_note.empty()) ImGui::TextColored(ImVec4(0.6f, 0.15f, 0.1f, 1), "%s", made_note.c_str());
}

/* test (ureport): the zip made, its path logged; nothing opened */
extern "C" void ui_report_test(void) { make_report(); }
