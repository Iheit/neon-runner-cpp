#include "asset_generator.h"

#include <raylib.h>

#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {

void MakeTexture(const std::string& path, Color base, Color accent, int pattern) {
    Image image = GenImageColor(64, 64, base);
    ImageDrawRectangle(&image, 0, 0, 63, 2, accent);
    ImageDrawRectangle(&image, 0, 61, 63, 2, accent);

    for (int y = 0; y < 64; y += 8) {
        for (int x = 0; x < 64; x += 8) {
            if ((x / 8 + y / 8 + pattern) % 3 == 0) {
                ImageDrawRectangle(&image, x + 1, y + 1, 5, 5,
                                   Color{accent.r, accent.g, accent.b, 90});
            }
        }
    }

    ExportImage(image, path.c_str());
    UnloadImage(image);
}

void WriteU16(std::ofstream& out, std::uint16_t value) {
    out.put(static_cast<char>(value & 0xff));
    out.put(static_cast<char>((value >> 8) & 0xff));
}

void WriteU32(std::ofstream& out, std::uint32_t value) {
    for (int i = 0; i < 4; ++i) {
        out.put(static_cast<char>((value >> (i * 8)) & 0xff));
    }
}

void MakeTone(const fs::path& path, float frequency, float duration, float volume) {
    constexpr std::uint32_t sampleRate = 44100;
    const std::uint32_t sampleCount = static_cast<std::uint32_t>(sampleRate * duration);
    std::vector<std::int16_t> pcm(sampleCount);

    for (std::uint32_t i = 0; i < sampleCount; ++i) {
        const float t = static_cast<float>(i) / sampleRate;
        const float envelope = std::exp(-5.0f * t / duration);
        const float sample = std::sin(2.0f * PI * frequency * t) * envelope * volume;
        pcm[i] = static_cast<std::int16_t>(sample * 32767.0f);
    }

    std::ofstream out(path, std::ios::binary);
    if (!out) return;

    const std::uint32_t dataSize = static_cast<std::uint32_t>(pcm.size() * sizeof(std::int16_t));
    out.write("RIFF", 4);
    WriteU32(out, 36 + dataSize);
    out.write("WAVEfmt ", 8);
    WriteU32(out, 16);
    WriteU16(out, 1);
    WriteU16(out, 1);
    WriteU32(out, sampleRate);
    WriteU32(out, sampleRate * 2);
    WriteU16(out, 2);
    WriteU16(out, 16);
    out.write("data", 4);
    WriteU32(out, dataSize);
    out.write(reinterpret_cast<const char*>(pcm.data()), static_cast<std::streamsize>(dataSize));
}

} // namespace

bool GenerateAssets(const std::string& root,
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

        constexpr int count = static_cast<int>(sizeof(textures) / sizeof(textures[0]));
        for (int i = 0; i < count; ++i) {
            progress(5 + i * 8, std::string("Generating 64x64 ") + textures[i].name);
            MakeTexture((directory / textures[i].name).string(), textures[i].base,
                        textures[i].accent, textures[i].pattern);
        }

        progress(72, "Generating sound effects");
        MakeTone(directory / "coin.wav", 880.0f, 0.16f, 0.30f);
        MakeTone(directory / "jump.wav", 440.0f, 0.18f, 0.25f);
        MakeTone(directory / "hit.wav", 110.0f, 0.30f, 0.35f);
        MakeTone(directory / "step.wav", 150.0f, 0.06f, 0.12f);

        progress(92, "Saving generated assets");
        progress(100, "Assets ready");
        return true;
    } catch (...) {
        return false;
    }
}
