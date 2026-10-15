// Forge runtime: <tilemap> - levels drawn as text, one character per tile.
//
//     ####......####
//     #..P....C....#
//     ##############
//
// The game decides what each character means: walls become static physics bodies (neighbouring wall tiles are
// merged into bigger boxes, so a level doesn't need thousands of bodies), coins and the start are found with
// find(), and tiles are drawn as colored rectangles or as parts of a tileset image.
#pragma once

#include "forge_assets.hpp"
#include "forge_file.hpp"
#include "forge_physics.hpp"
#include "forge_window.hpp"

namespace frg::tilemap {

struct TileMap_ : std::enable_shared_from_this<TileMap_> {
    std::vector<std::string> grid;  // one string per row, all the same length (padded with spaces)
    float tileSize_ = 32;

    TileMap_() = default;
    virtual ~TileMap_() = default;
    void frg$init(const std::string& text, float size) {
        if (!(size > 0)) panic("TileMap(...)", "the tile size must be more than 0, but it's " + str(size));
        tileSize_ = size;
        size_t start = 0;
        while (start <= text.size()) {
            size_t end = text.find('\n', start);
            if (end == std::string::npos) end = text.size();
            std::string row = text.substr(start, end - start);
            if (!row.empty() && row.back() == '\r') row.pop_back();
            grid.push_back(row);
            start = end + 1;
        }
        while (!grid.empty() && grid.back().find_first_not_of(' ') == std::string::npos) grid.pop_back();  // blank lines at the end
        size_t w = 0;
        for (auto& r : grid) w = std::max(w, r.size());
        for (auto& r : grid) r.resize(w, ' ');
    }
    static std::shared_ptr<TileMap_> frg$new(const std::string& text, float size) {
        auto o = std::make_shared<TileMap_>();
        o->frg$init(text, size);
        return o;
    }

    int32_t cols() const { return grid.empty() ? 0 : (int32_t)grid[0].size(); }
    bool inside(int32_t c, int32_t r) const { return r >= 0 && r < (int32_t)grid.size() && c >= 0 && c < cols(); }
    char cell(int32_t c, int32_t r) const { return inside(c, r) ? grid[r][c] : ' '; }
    static bool isOneOf(char ch, const std::string& tiles) { return ch != ' ' && tiles.find(ch) != std::string::npos; }

    virtual int32_t columns_() { return cols(); }
    virtual int32_t rows_() { return (int32_t)grid.size(); }
    virtual float width_() { return cols() * tileSize_; }
    virtual float height_() { return grid.size() * tileSize_; }
    virtual std::string get_(int32_t c, int32_t r) { return std::string(1, cell(c, r)); }
    virtual void set_(int32_t c, int32_t r, const std::string& tile) {
        if (!inside(c, r))
            panic("tilemap.set()", "column " + str(c) + ", row " + str(r) + " is outside the map (it has columns 0 to " +
                                       str(cols() - 1) + " and rows 0 to " + str((int32_t)grid.size() - 1) + ")");
        if (tile.size() != 1) panic("tilemap.set()", "a tile is one character, like \"#\", but this is \"" + tile + "\"");
        grid[r][c] = tile[0];
    }
    virtual int32_t columnAt_(float x) { return (int32_t)std::floor(x / tileSize_); }
    virtual int32_t rowAt_(float y) { return (int32_t)std::floor(y / tileSize_); }
    virtual core::rect_ tileRect_(int32_t c, int32_t r) { return {c * tileSize_, r * tileSize_, tileSize_, tileSize_}; }

    // The top-left corner (in pixels) of every tile that's one of `tiles`, row by row.
    virtual List<core::vec2_> find_(const std::string& tiles) {
        List<core::vec2_> out;
        for (int32_t r = 0; r < (int32_t)grid.size(); r++)
            for (int32_t c = 0; c < cols(); c++)
                if (isOneOf(grid[r][c], tiles)) out.push_back({c * tileSize_, r * tileSize_});
        return out;
    }

    // Rectangles covering every tile that's one of `tiles`, with neighbours merged: each rectangle grows as far
    // right as it can, then as far down as the whole row below allows.
    virtual List<core::rect_> solidAreas_(const std::string& tiles) {
        List<core::rect_> out;
        int32_t R = (int32_t)grid.size(), C = cols();
        std::vector<std::vector<bool>> used(R, std::vector<bool>(C, false));
        auto free = [&](int32_t c, int32_t r) { return !used[r][c] && isOneOf(grid[r][c], tiles); };
        for (int32_t r = 0; r < R; r++) {
            for (int32_t c = 0; c < C; c++) {
                if (!free(c, r)) continue;
                int32_t w = 1;
                while (c + w < C && free(c + w, r)) w++;
                int32_t h = 1;
                for (;; h++) {
                    if (r + h >= R) break;
                    bool rowOk = true;
                    for (int32_t i = 0; i < w && rowOk; i++) rowOk = free(c + i, r + h);
                    if (!rowOk) break;
                }
                for (int32_t y = 0; y < h; y++)
                    for (int32_t x = 0; x < w; x++) used[r + y][c + x] = true;
                out.push_back({c * tileSize_, r * tileSize_, w * tileSize_, h * tileSize_});
            }
        }
        return out;
    }

    // Static physics bodies for those tiles (merged), added to the physics world.
    virtual List<std::shared_ptr<physics::Body_>> addBodies_(const std::string& tiles, bool oneWay) {
        List<std::shared_ptr<physics::Body_>> out;
        for (const core::rect_& a : solidAreas_(tiles)) {
            auto b = physics::Body_::frg$new(a.x_, a.y_, a.width_, a.height_);
            b->isStatic_ = true;
            b->oneWay_ = oneWay;
            physics::add_(b);
            out.push_back(b);
        }
        return out;
    }

    virtual void draw_(const std::string& tiles, core::color_ c) {
        window::ensureFrame("tilemap.draw()");
        for (const core::rect_& a : solidAreas_(tiles)) DrawRectangleRec(rl(a), rl(c));
    }

    // Each character in `tiles` is a picture in the tileset: the first is the tileset's first picture (top-left),
    // the second the next one to its right, and so on (then the next row). Pictures are sourceSize pixels square.
    virtual void drawTiles_(std::shared_ptr<assets::Texture_> tileset, int32_t sourceSize, const std::string& tiles) {
        if (sourceSize <= 0) panic("tilemap.drawTiles()", "the size of a picture in the tileset must be more than 0");
        if (!tileset || !tileset->ready("tilemap.drawTiles()")) return;
        int32_t perRow = std::max(1, tileset->w / sourceSize);
        for (int32_t r = 0; r < (int32_t)grid.size(); r++)
            for (int32_t c = 0; c < cols(); c++) {
                size_t i = tiles.find(grid[r][c]);
                if (grid[r][c] == ' ' || i == std::string::npos) continue;
                ::Rectangle src{float((int32_t)i % perRow * sourceSize), float((int32_t)i / perRow * sourceSize), float(sourceSize),
                                float(sourceSize)};
                DrawTexturePro(tileset->tex, src, ::Rectangle{c * tileSize_, r * tileSize_, tileSize_, tileSize_}, ::Vector2{0, 0}, 0,
                               WHITE);
            }
    }
};

inline std::shared_ptr<TileMap_> load_(const std::string& path, float size) {
    std::string found = findGameFile(path);
    if (found.empty()) panic("tilemap.load()", "can't find the file '" + path + "' - put it in the game's folder (next to main.frg)");
    return TileMap_::frg$new(file::read_(found), size);
}

inline std::string frg$str(const std::shared_ptr<TileMap_>& m) {
    return m ? "TileMap(" + str(m->cols()) + " x " + str((int32_t)m->grid.size()) + " tiles)" : "null";
}

}  // namespace frg::tilemap
