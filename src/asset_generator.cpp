#include "asset_generator.h"

#include <raylib.h>

#include <filesystem>
#include <string>

namespace fs = std::filesystem;

namespace {

void MakeTexture(const std::string& path,
                 Color base,
                 Color accent,
                 int pattern) {
    Image image = GenImageColor(64, 64, base);

    ImageDrawRectangle(&image, 0, 0, 63, 3, accent);
    ImageDrawRectangle(&image, 0, 60, 63, 3, accent);

    for (int y = 0; y < 64; y += 8) {
        for (int x = 0; x < 64; x += 8) {
            if ((x / 8 + y / 8 + pattern) % 3 == 0) {
                ImageDrawRectangle(
                    &image,
                    x + 1,
                    y + 1,
                    5,
                    5,
                    Color{accent.r, accent.g, accent.b, 100});
            }
        }
    }

    ExportImage(image, path.c_str());
    UnloadImage(image);
}

} // namespace

bool GenerateAssets(
    const std::string& root,
    const std::function<void(int, const std::string&)>& progress) {
    try {
        const fs::path directory = fs::path(root) / "assets" / "generated";
        fs::create_directories(directory);

        struct TextureDefinition {
            const char* name;
            Color base;
            Color accent;
            int pattern;
        };

        const TextureDefinition textures[] = {
            {"road.png", {20, 24, 38, 255}, {70, 220, 255, 255}, 0},
            {"road_edge.png", {25, 30, 48, 255}, {255, 70, 95, 255}, 1},
            {"metal.png", {45, 50, 65, 255}, {120, 130, 155, 255}, 2},
            {"obstacle.png", {90, 20, 35, 255}, {255, 70, 95, 255}, 3},
            {"coin.png", {100, 70, 10, 255}, {255, 220, 70, 255}, 4},
            {"building.png", {24, 28, 48, 255}, {70, 80, 110, 255}, 5},
            {"windows.png", {12, 25, 40, 255}, {100, 220, 255, 255}, 6},
            {"neon_panel.png", {20, 15, 35, 255}, {220, 80, 255, 255}, 7}
        };

        constexpr int textureCount =
            static_cast<int>(sizeof(textures) / sizeof(textures[0]));

        for (int i = 0; i < textureCount; ++i) {
            progress(
                10 + i * 9,
                std::string("Generating ") + textures[i].name);

            MakeTexture(
                (directory / textures[i].name).string(),
                textures[i].base,
                textures[i].accent,
                textures[i].pattern);
        }

        progress(85, "Saving generated assets");
        progress(100, "Assets ready");
        return true;
    } catch (...) {
        return false;
    }
}
