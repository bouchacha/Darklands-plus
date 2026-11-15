/* Custom portraits shared by the party panel and the native character sheet. */
#include "portraits.h"
#include "gamedata.h"
#include "imgui_internal.h"
#include "ui.h"
#include "../third_party/stb/stb_image.h"
#include "../third_party/stb/stb_image_write.h"
#include <algorithm>
#include <cmath>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <map>
#include <memory>
#include <vector>

extern "C" char dlp_user_dir[], game_dir[];
extern "C" int sheetview_showing(void);

namespace fs = std::filesystem;

namespace {

SDL_Renderer *renderer;
const char *const extensions[] = { ".png", ".jpg", ".jpeg", ".PNG", ".JPG" };
constexpr Uint32 CHECK_INTERVAL = 2000;
constexpr int MAX_SIDE = 4096;
constexpr int PORTRAIT_W = 360, PORTRAIT_H = 800;
static_assert(static_cast<float>(PORTRAIT_W) / PORTRAIT_H == PORTRAIT_ASPECT);
constexpr char PICKER_ID[] = "###portrait-picker";

fs::path portrait_folder()
{
    return fs::path(dlp_user_dir[0] ? dlp_user_dir : game_dir) / "portraits";
}

fs::path assigned_stem(const char *short_name, const char *full_name)
{
    const auto *name = reinterpret_cast<const unsigned char *>(*full_name ? full_name : short_name);
    if (!*name) return {};
    /* Keep imports separate from hand-installed images. Escape DOS name bytes that
       cannot be used in a filename. The prefix also avoids Windows device names. */
    std::string stem = "Member ";
    for (; *name; ++name) {
        if (*name >= 32 && *name < 127 && !strchr("<>:\"/\\|?*%", *name)) stem += (char)*name;
        else {
            const char *hex = "0123456789ABCDEF";
            stem += '%';
            stem += hex[*name >> 4];
            stem += hex[*name & 15];
        }
    }
    return portrait_folder() / "assigned" / stem;
}

struct Pixels {
    std::unique_ptr<unsigned char, decltype(&stbi_image_free)> data{nullptr, stbi_image_free};
    int w = 0, h = 0;
};

Pixels read_image(const fs::path &file, bool importing = false)
{
    Pixels pixels;
#ifdef _WIN32
    FILE *in = _wfopen(file.c_str(), L"rb");
#else
    FILE *in = fopen(file.c_str(), "rb");
#endif
    if (!in) return pixels;
    int channels;
    if (!importing || (stbi_info_from_file(in, &pixels.w, &pixels.h, &channels) &&
                       pixels.w <= MAX_SIDE && pixels.h <= MAX_SIDE))
        pixels.data.reset(stbi_load_from_file(in, &pixels.w, &pixels.h, &channels, 4));
    fclose(in);
    return pixels;
}

GamePic make_picture(const Pixels &pixels)
{
    if (!pixels.data || !renderer) return {};
    SDL_Texture *tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, pixels.w, pixels.h);
    if (!tex) return {};
    if (SDL_UpdateTexture(tex, nullptr, pixels.data.get(), pixels.w * 4) != 0) {
        SDL_DestroyTexture(tex);
        return {};
    }
    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
    SDL_SetTextureScaleMode(tex, SDL_ScaleModeLinear);
    return { tex, pixels.w, pixels.h };
}

struct CachedPortrait {
    GamePic pic;
    fs::path file;
    fs::file_time_type modified;
    Uint32 checked = 0;
    int frame = -1;
    bool checked_once = false;
    bool original = false;
};
std::map<fs::path, CachedPortrait> cache;

const GamePic *find_portrait(const fs::path &stem, bool assigned = false)
{
    if (stem.empty() || !renderer) return nullptr;
    CachedPortrait &cached = cache[stem];
    Uint32 now = SDL_GetTicks();
    int frame = ImGui::GetFrameCount();
    if (cached.frame != frame && (!cached.checked_once || now - cached.checked >= CHECK_INTERVAL)) {
        cached.checked = now;
        cached.checked_once = true;
        std::error_code ec;
        fs::path file;
        fs::file_time_type modified;
        for (const char *ext : extensions) {
            file = stem;
            file += ext;
            modified = fs::last_write_time(file, ec);
            if (!ec) break;
            file.clear();
            if (assigned) break;
        }
        if (assigned) {
            fs::path original = stem;
            original += ".original";
            cached.original = file.empty() && fs::is_regular_file(original, ec);
        }
        if (file != cached.file || (!file.empty() && modified != cached.modified)) {
            if (cached.pic.tex) SDL_DestroyTexture(cached.pic.tex);
            cached.pic = file.empty() ? GamePic{} : make_picture(read_image(file));
            cached.file = file;
            cached.modified = modified;
        }
    }
    /* The same texture can be referenced by both the party panel and sheet in one frame. */
    cached.frame = frame;
    return cached.pic.tex ? &cached.pic : nullptr;
}

const GamePic *folder_portrait(const char *name)
{
    if (!*name || strpbrk(name, "/\\:")) return nullptr;
    return find_portrait(portrait_folder() / name);
}

bool image_extension(const fs::path &file)
{
    std::string ext = file.extension().u8string();
    for (char &c : ext) c = (char)std::tolower((unsigned char)c);
    return ext == ".png" || ext == ".jpg" || ext == ".jpeg";
}

struct FolderEntry {
    fs::path path;
    bool directory;
};

struct PortraitCrop {
    ImVec2 center{0.5f, 0.5f};
    float zoom = 1.0f;

    bool edited() const { return zoom != 1.0f || center.x != 0.5f || center.y != 0.5f; }
};

/* The image's origin and extent in units of the portrait frame, before clipping. */
struct CropAxis {
    float start, extent;

    float uv(float position) const { return std::clamp((position - start) / extent, 0.0f, 1.0f); }
};

struct Framing { CropAxis x, y; };

Framing portrait_framing(int w, int h, const PortraitCrop &crop)
{
    float aspect = static_cast<float>(w) / h;
    auto axis = [](float extent, float center) {
        float start = extent > 1.0f ? std::clamp(0.5f - center * extent, 1.0f - extent, 0.0f)
                                    : (1.0f - extent) * 0.5f;
        return CropAxis{start, extent};
    };
    return { axis(crop.zoom * std::max(aspect / PORTRAIT_ASPECT, 1.0f), crop.center.x),
             axis(crop.zoom * std::max(PORTRAIT_ASPECT / aspect, 1.0f), crop.center.y) };
}

void draw_framed_portrait(ImDrawList *draw, const GamePic &pic, ImVec2 a, ImVec2 b, float rounding,
                         const PortraitCrop &crop)
{
    Framing f = portrait_framing(pic.w, pic.h, crop);
    ImVec2 lo(std::max(0.0f, f.x.start), std::max(0.0f, f.y.start));
    ImVec2 hi(std::min(1.0f, f.x.start + f.x.extent), std::min(1.0f, f.y.start + f.y.extent));
    ImVec2 pos(a.x + (b.x - a.x) * lo.x, a.y + (b.y - a.y) * lo.y);
    ImVec2 end(a.x + (b.x - a.x) * hi.x, a.y + (b.y - a.y) * hi.y);
    if (f.x.extent < 1.0f || f.y.extent < 1.0f) rounding = 0.0f;
    draw->AddImageRounded((ImTextureID)(intptr_t)pic.tex, pos, end,
                          ImVec2(f.x.uv(lo.x), f.y.uv(lo.y)), ImVec2(f.x.uv(hi.x), f.y.uv(hi.y)),
                          IM_COL32_WHITE, rounding);
}

struct PortraitPicker {
    bool open = false, opening = false, scroll_top = false, expanded = false;
    fs::path home, folder, target, selected;
    char location[4096] = {};
    std::string short_name, full_name, title, error;
    std::vector<FolderEntry> entries;
    Pixels pixels;
    GamePic preview;
    PortraitCrop crop;
} picker;

void clear_preview()
{
    if (picker.preview.tex) SDL_DestroyTexture(picker.preview.tex);
    picker.preview = {};
    picker.pixels = {};
    picker.crop = {};
    picker.selected.clear();
}

void open_folder(const fs::path &folder)
{
    std::error_code ec;
    fs::directory_iterator it(folder, ec), end;
    if (ec) {
        picker.error = "Could not open this folder.";
        return;
    }
    std::vector<FolderEntry> entries;
    const fs::path assigned = (portrait_folder() / "assigned").lexically_normal();
    for (; it != end && !ec; it.increment(ec)) {
        if (it->path().lexically_normal() == assigned) continue;
        std::error_code status_error;
        bool dir = it->is_directory(status_error);
        if (dir || (image_extension(it->path()) && it->is_regular_file(status_error)))
            entries.push_back({ it->path(), dir });
    }
    if (ec) {
        picker.error = "Could not read the whole folder.";
        return;
    }
    std::sort(entries.begin(), entries.end(), [](const FolderEntry &a, const FolderEntry &b) {
        return a.directory != b.directory ? a.directory : a.path < b.path;
    });
    clear_preview();
    picker.error.clear();
    picker.folder = folder.lexically_normal();
    if (picker.folder.has_relative_path() && !picker.folder.has_filename())
        picker.folder = picker.folder.parent_path();
    picker.location[0] = '\0';
    picker.entries = std::move(entries);
    picker.scroll_top = true;
}

bool preview_file(const fs::path &file)
{
    if (picker.selected == file && picker.preview.tex) {
        picker.error.clear();
        return true;
    }
    Pixels pixels;
    std::error_code ec;
    if (image_extension(file) && fs::is_regular_file(file, ec))
        pixels = read_image(file, true);
    GamePic preview = make_picture(pixels);
    if (!preview.tex) {
        picker.error = "Could not preview that image.";
        return false;
    }
    clear_preview();
    picker.selected = file;
    picker.pixels = std::move(pixels);
    picker.preview = preview;
    picker.error.clear();
    return true;
}

void open_location()
{
    std::string input = picker.location;
    if (input.size() >= 2 && input.front() == '"' && input.back() == '"')
        input = input.substr(1, input.size() - 2);    /* Windows' Copy as path */
    fs::path location;
    try {
        if (!input.empty()) location = (picker.folder / fs::u8path(input)).lexically_normal();
    } catch (const fs::filesystem_error &) {
        picker.error = "This path could not be read.";
        return;
    }
    std::error_code ec;
    if (fs::is_directory(location, ec)) open_folder(location);
    else if (image_extension(location)) {
        if (preview_file(location)) picker.location[0] = '\0';
    } else {
        picker.error = "Could not open that path.";
    }
}

void write_png(void *context, void *data, int size)
{
    static_cast<std::ofstream *>(context)->write(static_cast<const char *>(data), size);
}

std::vector<unsigned char> framed_pixels(const Pixels &source, const PortraitCrop &crop)
{
    Framing f = portrait_framing(source.w, source.h, crop);
    std::vector<unsigned char> pixels(PORTRAIT_W * PORTRAIT_H * 4);
    const unsigned char *src = source.data.get();
    for (int y = 0; y < PORTRAIT_H; ++y) {
        float v = (y + 0.5f) / PORTRAIT_H;
        if ((y + 1.0f) / PORTRAIT_H <= f.y.start || static_cast<float>(y) / PORTRAIT_H >= f.y.start + f.y.extent)
            continue;
        float sy = std::clamp(f.y.uv(v) * source.h - 0.5f, 0.0f, static_cast<float>(source.h - 1));
        int y0 = static_cast<int>(sy), y1 = std::min(y0 + 1, source.h - 1);
        for (int x = 0; x < PORTRAIT_W; ++x) {
            float u = (x + 0.5f) / PORTRAIT_W;
            if ((x + 1.0f) / PORTRAIT_W <= f.x.start || static_cast<float>(x) / PORTRAIT_W >= f.x.start + f.x.extent)
                continue;
            float sx = std::clamp(f.x.uv(u) * source.w - 0.5f, 0.0f, static_cast<float>(source.w - 1));
            int x0 = static_cast<int>(sx), x1 = std::min(x0 + 1, source.w - 1);
            const unsigned char *tl = src + (y0 * source.w + x0) * 4, *tr = src + (y0 * source.w + x1) * 4;
            const unsigned char *bl = src + (y1 * source.w + x0) * 4, *br = src + (y1 * source.w + x1) * 4;
            unsigned char *pixel = pixels.data() + (y * PORTRAIT_W + x) * 4;
            for (int c = 0; c < 4; ++c) {
                float top = tl[c] + (tr[c] - tl[c]) * (sx - x0);
                float bottom = bl[c] + (br[c] - bl[c]) * (sx - x0);
                pixel[c] = static_cast<unsigned char>(std::lround(top + (bottom - top) * (sy - y0)));
            }
        }
    }
    return pixels;
}

bool apply_portrait()
{
    std::error_code ec;
    fs::path dir = picker.target.parent_path();
    fs::create_directories(dir, ec);
    if (ec) return false;

    /* Write beside the destination and replace it only after the image is complete.
       Each import has its own temporary directory, including between game processes. */
    fs::path temp = dir / (".import-" + std::to_string(SDL_GetPerformanceCounter()));
    if (!fs::create_directory(temp, ec)) return false;
    fs::path file = temp / "portrait.png", dest = picker.target;
    dest += ".png";
    std::ofstream out(file, std::ios::binary);
    const Pixels &p = picker.pixels;
    std::vector<unsigned char> framed;
    if (out && picker.crop.edited()) framed = framed_pixels(p, picker.crop);
    int w = framed.empty() ? p.w : PORTRAIT_W, h = framed.empty() ? p.h : PORTRAIT_H;
    bool written = out && stbi_write_png_to_func(write_png, &out, w, h, 4,
                                                framed.empty() ? p.data.get() : framed.data(), w * 4);
    out.close();
    if (written && out) fs::rename(file, dest, ec);
    bool installed = written && out && !ec;
    std::error_code ignored;
    fs::remove(file, ignored);
    fs::remove(temp, ignored);
    if (installed) {
        CachedPortrait &cached = cache[picker.target];
        cached.checked_once = false;
        cached.file.clear();               /* reload next frame, even with the same file timestamp */
    }
    return installed;
}

bool revert_portrait()
{
    std::error_code ec;
    fs::create_directories(picker.target.parent_path(), ec);
    if (ec) return false;
    fs::path imported = picker.target, original = picker.target;
    imported += ".png";
    original += ".original";
    /* Move an imported image out of use in one operation. For a hand-installed image,
       just create the override. A later import takes priority over the override. */
    const bool has_import = fs::exists(imported, ec);
    if (ec) return false;
    if (has_import) {
        fs::rename(imported, original, ec);
        if (ec) return false;
    } else {
        std::ofstream override_file(original, std::ios::binary | std::ios::app);
        override_file.close();
        if (!override_file) return false;
    }
    cache[picker.target].checked_once = false;
    return true;
}

void draw_label(const std::string &label, ImVec2 pos, float width)
{
    ImVec2 end(pos.x + width, pos.y + ImGui::GetTextLineHeight());
    ImGui::RenderTextEllipsis(ImGui::GetWindowDrawList(), pos, end, end.x, end.x,
                              label.data(), label.data() + label.size(), nullptr);
}

void full_label_tooltip(const char *text)
{
    if (!ImGui::BeginItemTooltip()) return;
    float wrap = std::min(UIS(500), ImGui::GetIO().DisplaySize.x * 0.75f);
    ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + wrap);
    ImGui::TextUnformatted(text);
    ImGui::PopTextWrapPos();
    ImGui::EndTooltip();
}

void label_line(const std::string &label, const char *tooltip = nullptr)
{
    ImVec2 pos = ImGui::GetCursorScreenPos();
    float width = ImGui::GetContentRegionAvail().x;
    ImGui::Dummy(ImVec2(width, ImGui::GetTextLineHeight()));
    if (ImGui::IsItemVisible()) draw_label(label, pos, width);
    if (tooltip || ImGui::CalcTextSize(label.c_str()).x > width)
        full_label_tooltip(tooltip ? tooltip : label.c_str());
}

void portrait_preview(const GamePic &pic, float w, float h, float rounding, bool candidate)
{
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x - w) * 0.5f);
    ImVec2 a = ImGui::GetCursorScreenPos(), b(a.x + w, a.y + h);
    ImDrawList *draw = ImGui::GetWindowDrawList();
    bool highlighted = false;
    if (candidate) {
        ImGui::InvisibleButton("##frame", ImVec2(w, h));
        Framing f = portrait_framing(pic.w, pic.h, picker.crop);
        highlighted = (f.x.extent > 1.0f || f.y.extent > 1.0f) && (ImGui::IsItemHovered() || ImGui::IsItemActive());
        if (highlighted) ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
        if (highlighted && ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
            ImVec2 delta = ImGui::GetIO().MouseDelta;
            auto move_axis = [](CropAxis axis, float delta) {
                if (axis.extent <= 1.0f) return 0.5f;
                float start = std::clamp(axis.start + delta, 1.0f - axis.extent, 0.0f);
                return (0.5f - start) / axis.extent;
            };
            picker.crop.center = ImVec2(move_axis(f.x, delta.x / w), move_axis(f.y, delta.y / h));
        }
    } else {
        ImGui::Dummy(ImVec2(w, h));
    }
    draw->AddRectFilled(a, b, IM_COL32(64, 46, 29, 255), rounding);
    draw_framed_portrait(draw, pic, a, b, rounding, candidate ? picker.crop : PortraitCrop{});
    if (highlighted) draw->AddRect(a, b, ImGui::GetColorU32(ImGuiCol_Border), rounding);
}

} // namespace

void portraits_set_renderer(SDL_Renderer *ren) { renderer = ren; }

const GamePic *member_portrait(const char *short_name, const char *full_name)
{
    const fs::path assigned = assigned_stem(short_name, full_name);
    if (!assigned.empty()) {
        if (const GamePic *pic = find_portrait(assigned, true)) return pic;
        if (cache[assigned].original) return nullptr;
    }
    if (const GamePic *pic = folder_portrait(short_name)) return pic;
    return strcmp(short_name, full_name) ? folder_portrait(full_name) : nullptr;
}

void draw_portrait(ImDrawList *draw, const GamePic &pic, ImVec2 a, ImVec2 b, float rounding)
{
    draw_framed_portrait(draw, pic, a, b, rounding, {});
}

void portrait_picker_open(const char *short_name, const char *full_name)
{
    if (picker.open) return;
    picker.target = assigned_stem(short_name, full_name);
    if (picker.target.empty()) return;
    picker.short_name = short_name;
    picker.full_name = full_name;
    picker.error.clear();
    picker.expanded = false;
    const char *name = *full_name ? full_name : short_name;
    picker.title = "Choose Portrait for " + game_text(name, strlen(name)) + PICKER_ID;
    std::error_code ec;
#ifdef _WIN32
    const wchar_t *home = _wgetenv(L"USERPROFILE");
#else
    const char *home = getenv("HOME");
#endif
    picker.home = home && fs::is_directory(home, ec) ? fs::path(home) : portrait_folder().parent_path();
    fs::path folder = picker.folder;
    if (!fs::is_directory(folder, ec)) folder = picker.home / "Downloads";
    if (!fs::is_directory(folder, ec)) folder = picker.home;
    open_folder(folder);
    picker.open = picker.opening = true;
}

bool portrait_picker_is_open(void) { return picker.open; }

void portrait_picker_window(void)
{
    if (!picker.open) {
        /* Keep the texture until the last frame of the picker has been rendered. */
        if (!picker.selected.empty()) clear_preview();
        return;
    }
    if (picker.opening) {
        ImGui::OpenPopup(PICKER_ID);
        picker.opening = false;
    }
    ImVec2 display = ImGui::GetIO().DisplaySize;
    const bool expanded = picker.expanded;
    const ImVec2 size(std::min(UIS(690), display.x * 0.96f), std::min(UIS(expanded ? 760 : 525), display.y * 0.94f));
    const float fit = std::min({ 1.0f, size.x / UIS(420), size.y / UIS(350) });
    const ImGuiStyle &style = ImGui::GetStyle();
    auto scaled = [fit](ImVec2 pair) { return ImVec2(pair.x * fit, pair.y * fit); };
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, scaled(style.WindowPadding));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, scaled(style.ItemSpacing));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, scaled(style.FramePadding));
    ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, style.ScrollbarSize * fit);
    ImGui::SetNextWindowPos(ImVec2(display.x * 0.5f, display.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(size);
    if (!ImGui::BeginPopupModal(picker.title.c_str(), nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings |
                               ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse)) {
        ImGui::PopStyleVar(4);
        picker.open = false;
        return;
    }
    /* Keep space for the actions even in a small window or at high UI scale. */
    ImGui::SetWindowFontScale(fit);

    if (!expanded) {
        ImGui::BeginDisabled(picker.folder.parent_path() == picker.folder);
        if (ImGui::Button("Up")) open_folder(picker.folder.parent_path());
        ImGui::EndDisabled();
        ImGui::SameLine();
        if (ImGui::Button("Home")) open_folder(picker.home);
        ImGui::SameLine();
        const float go_width = ImGui::CalcTextSize("Open").x + ImGui::GetStyle().FramePadding.x * 2;
        ImGui::SetNextItemWidth(-go_width - ImGui::GetStyle().ItemSpacing.x);
        bool go = ImGui::InputTextWithHint("##location", "Paste a folder or image path", picker.location, sizeof picker.location,
                                          ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll);
        ImGui::SameLine();
        ImGui::BeginDisabled(!picker.location[0]);
        if (ImGui::Button("Open") || go) open_location();
        ImGui::EndDisabled();

        const fs::path parent = picker.folder.parent_path();
        const std::string folder_name = picker.folder.has_filename() ? picker.folder.filename().u8string() : picker.folder.u8string();
        const std::string folder_label = parent.has_filename() ? parent.filename().u8string() + " / " + folder_name : folder_name;
        label_line("Folder: " + folder_label, picker.folder.u8string().c_str());
        label_line("PNG/JPEG. Max 4096 px per side. Try 360 x 800 px.");
    }

    const float width = ImGui::GetContentRegionAvail().x;
    const float height = std::max(1.0f, ImGui::GetContentRegionAvail().y - ImGui::GetFrameHeightWithSpacing() -
                                          ImGui::GetTextLineHeightWithSpacing());
    if (!expanded) {
        fs::path next_folder;
        if (ImGui::BeginChild("Files", ImVec2(width * 0.63f, height), true)) {
            if (picker.scroll_top) ImGui::SetScrollY(0);
            picker.scroll_top = false;
            if (picker.entries.empty()) ImGui::TextDisabled("No folders or images here.");
            ImGuiListClipper clipper;
            clipper.Begin((int)picker.entries.size());
            while (clipper.Step()) for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i) {
                const FolderEntry &entry = picker.entries[i];
                std::string label = entry.path.filename().u8string() + (entry.directory ? "/" : "");
                ImVec2 pos = ImGui::GetCursorScreenPos();
                float row_width = ImGui::GetContentRegionAvail().x;
                ImGui::PushID(i);
                if (ImGui::Selectable("##entry", picker.selected == entry.path, 0, ImVec2(row_width, ImGui::GetTextLineHeight()))) {
                    if (entry.directory) next_folder = entry.path;
                    else preview_file(entry.path);
                }
                if (ImGui::IsItemVisible()) draw_label(label, pos, row_width);
                if (ImGui::CalcTextSize(label.c_str()).x > row_width)
                    full_label_tooltip(label.c_str());
                ImGui::PopID();
            }
        }
        ImGui::EndChild();
        if (!next_folder.empty()) open_folder(next_folder);
        ImGui::SameLine();
    }
    bool showing_current = false;
    if (ImGui::BeginChild("Preview", ImVec2(0, height), true)) {
        bool candidate = picker.preview.tex != nullptr;
        const GamePic *pic = candidate ? &picker.preview : member_portrait(picker.short_name.c_str(), picker.full_name.c_str());
        const char *heading = candidate ? "New portrait" : pic ? "Current portrait" : "Preview";
        showing_current = !candidate && pic;
        if (candidate) {
            if (ImGui::SmallButton(expanded ? "Back to files" : "Enlarge preview")) picker.expanded = !expanded;
        } else label_line(heading);
        if (pic) {
            const bool show_filename = candidate && height >= ImGui::GetTextLineHeightWithSpacing() * 5;
            const float h = std::min(ImGui::GetContentRegionAvail().y - (show_filename ? ImGui::GetTextLineHeightWithSpacing() : 0.0f),
                                     ImGui::GetContentRegionAvail().x / PORTRAIT_ASPECT);
            std::string filename = candidate ? picker.selected.filename().u8string() : "";
            if (h > 0) {
                portrait_preview(*pic, h * PORTRAIT_ASPECT, h, UIS(3) * fit, candidate);
                if (!show_filename && candidate) full_label_tooltip(filename.c_str());
            }
            if (show_filename) label_line(filename);
        } else ImGui::TextWrapped("Select a PNG or JPEG to preview it.");
    }
    ImGui::EndChild();

    bool apply = false;
    if (showing_current) {
        if (ImGui::Button("Revert to Original")) {
            apply = revert_portrait();
            if (!apply) picker.error = "Could not revert the portrait. Check that the portraits folder is writable.";
        }
        ImGui::SetItemTooltip("Restore the game portrait for this character.");
    } else {
        ImGui::BeginDisabled(!picker.preview.tex);
        apply = ImGui::Button("Use Portrait");
        ImGui::EndDisabled();
        if (apply && !apply_portrait()) {
            picker.error = "Could not save the portrait. Check that the portraits folder is writable.";
            apply = false;
        }
    }
    ImGui::SameLine();
    bool cancel = ImGui::Button("Cancel");
    if (picker.preview.tex) {
        ImGui::SameLine();
        const float reset_width = ImGui::CalcTextSize("Reset").x + ImGui::GetStyle().FramePadding.x * 2;
        const float readout_width = ImGui::CalcTextSize("Zoom 300%").x;
        ImGui::SetNextItemWidth(-reset_width - readout_width - ImGui::GetStyle().ItemSpacing.x * 2);
        Framing cover = portrait_framing(picker.preview.w, picker.preview.h, {});
        const float fit_zoom = 1.0f / std::max(cover.x.extent, cover.y.extent);
        ImGui::SliderFloat("##zoom", &picker.crop.zoom, fit_zoom, 3.0f, "",
                           ImGuiSliderFlags_NoInput | ImGuiSliderFlags_NoRoundToFormat);
        ImGui::SetItemTooltip("Zoom out to fit the whole image, or zoom in for a closer crop.");
        ImGui::SameLine();
        ImGui::AlignTextToFramePadding();
        if (picker.crop.zoom == fit_zoom) ImGui::TextUnformatted("Zoom Fit");
        else ImGui::Text("Zoom %.0f%%", picker.crop.zoom * 100.0f);
        ImGui::SameLine();
        if (ImGui::Button("Reset")) picker.crop = {};
    }
    if (cancel || apply || ImGui::IsKeyPressed(ImGuiKey_Escape) || !sheetview_showing()) {
        ImGui::CloseCurrentPopup();
        picker.open = false;
    }
    if (!picker.error.empty()) label_line(picker.error + (picker.preview.tex ? " Framing kept." : ""));
    else if (picker.preview.tex) {
        Framing frame = portrait_framing(picker.preview.w, picker.preview.h, picker.crop);
        const char *instruction = frame.x.extent < 1.0f || frame.y.extent < 1.0f
            ? "Margins show the panel background. Source: "
            : frame.x.extent > 1.0f || frame.y.extent > 1.0f ? "Drag to reposition. Source: " : "Zoom in to reposition. Source: ";
        label_line(instruction + std::to_string(picker.preview.w) + " x " + std::to_string(picker.preview.h) + " px.");
    } else if (showing_current) label_line("Select the original image to adjust framing.");
    ImGui::EndPopup();
    ImGui::PopStyleVar(4);
}
