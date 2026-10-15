// Forge runtime: <assets> - images (textures) and fonts.
//
// Files are read when they're loaded (so a missing file is reported right away), but they're only
// sent to the graphics card the first time they're drawn - so loading works even before window.open().
#pragma once

#include "forge_window.hpp"

namespace frg::assets {

struct Texture_;
struct Font_;
inline std::vector<Texture_*>& liveTextures() {
    static std::vector<Texture_*> v;
    return v;
}
inline std::vector<Font_*>& liveFonts() {
    static std::vector<Font_*> v;
    return v;
}
inline void unloadAll();

struct Texture_ : std::enable_shared_from_this<Texture_> {
    std::string path;
    ::Image image{};
    ::Texture2D tex{};
    bool hasImage = false;
    bool uploaded = false;
    int32_t w = 0, h = 0;

    Texture_() = default;
    Texture_(const Texture_&) = delete;
    virtual ~Texture_() {
        auto& live = liveTextures();
        live.erase(std::remove(live.begin(), live.end(), this), live.end());
        if (uploaded && window::st().open) UnloadTexture(tex);
        if (hasImage) UnloadImage(image);
    }
    static std::shared_ptr<Texture_> frg$new() { return std::make_shared<Texture_>(); }

    // The texture on the graphics card, uploading it the first time.
    bool ready(const char* what) {
        if (!hasImage && !uploaded) return false;  // made with Texture() instead of assets.loadTexture
        window::ensureFrame(what);
        if (!uploaded) {
            tex = LoadTextureFromImage(image);
            UnloadImage(image);
            hasImage = false;
            uploaded = true;
            liveTextures().push_back(this);
            addHook(window::beforeCloseHooks(), &unloadAll);
        }
        return true;
    }

    virtual int32_t width_() { return w; }
    virtual int32_t height_() { return h; }
    virtual void draw_(float x, float y, core::color_ tint) {
        if (ready("texture.draw()")) DrawTextureV(tex, ::Vector2{x, y}, rl(tint));
    }
    virtual void drawScaled_(float x, float y, float scale, core::color_ tint) {
        if (ready("texture.drawScaled()")) DrawTextureEx(tex, ::Vector2{x, y}, 0, scale, rl(tint));
    }
    virtual void drawRotated_(core::vec2_ center, float degrees, float scale, core::color_ tint) {
        if (!ready("texture.drawRotated()")) return;
        float dw = w * scale, dh = h * scale;
        DrawTexturePro(tex, ::Rectangle{0, 0, (float)w, (float)h}, ::Rectangle{center.x_, center.y_, dw, dh},
                       ::Vector2{dw / 2, dh / 2}, degrees, rl(tint));
    }
    virtual void drawPart_(core::rect_ source, core::rect_ dest, core::color_ tint) {
        if (ready("texture.drawPart()")) DrawTexturePro(tex, rl(source), rl(dest), ::Vector2{0, 0}, 0, rl(tint));
    }
};

struct Font_ : std::enable_shared_from_this<Font_> {
    std::string path;
    int32_t quality = 64;
    ::Font font{};
    bool uploaded = false;

    Font_() = default;
    Font_(const Font_&) = delete;
    virtual ~Font_() {
        auto& live = liveFonts();
        live.erase(std::remove(live.begin(), live.end(), this), live.end());
        if (uploaded && window::st().open) UnloadFont(font);
    }
    static std::shared_ptr<Font_> frg$new() { return std::make_shared<Font_>(); }

    ::Font& get(const char* what) {
        window::ensureFrame(what);
        if (!uploaded) {
            font = path.empty() ? GetFontDefault() : LoadFontEx(path.c_str(), quality, nullptr, 0);
            if (!path.empty()) SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);
            uploaded = !path.empty();
            if (uploaded) {
                liveFonts().push_back(this);
                addHook(window::beforeCloseHooks(), &unloadAll);
            }
        }
        return font;
    }
    virtual void drawText_(const std::string& text, float x, float y, float size, core::color_ c) {
        ::Font& f = get("font.drawText()");
        DrawTextEx(f, text.c_str(), ::Vector2{x, y}, size, size / 10, rl(c));
    }
    virtual float measure_(const std::string& text, float size) {
        ::Font& f = get("font.measure()");
        return MeasureTextEx(f, text.c_str(), size, size / 10).x;
    }
};

// Called just before the window closes: graphics memory has to be freed while it still exists.
inline void unloadAll() {
    for (Texture_* t : liveTextures()) {
        UnloadTexture(t->tex);
        t->uploaded = false;
    }
    liveTextures().clear();
    for (Font_* f : liveFonts()) {
        UnloadFont(f->font);
        f->uploaded = false;
    }
    liveFonts().clear();
}

inline std::shared_ptr<Texture_> loadTexture_(const std::string& path) {
    std::string file = requireGameFile(path, "assets.loadTexture()");
    auto t = std::make_shared<Texture_>();
    t->path = file;
    t->image = LoadImage(file.c_str());
    if (!t->image.data) panic("assets.loadTexture()", "couldn't read '" + path + "' as an image (use .png, .jpg, .bmp, .tga, .gif or .qoi)");
    t->hasImage = true;
    t->w = t->image.width;
    t->h = t->image.height;
    return t;
}

inline std::shared_ptr<Font_> loadFont_(const std::string& path, int32_t quality) {
    auto f = std::make_shared<Font_>();
    f->path = requireGameFile(path, "assets.loadFont()");
    f->quality = quality < 8 ? 8 : quality;
    return f;
}

inline bool exists_(const std::string& path) { return !findGameFile(path).empty(); }

inline std::string frg$str(const std::shared_ptr<Texture_>& t) {
    return t ? "Texture(" + str(t->w) + "x" + str(t->h) + ")" : "null";
}
inline std::string frg$str(const std::shared_ptr<Font_>& f) { return f ? "Font(" + f->path + ")" : "null"; }

}  // namespace frg::assets
