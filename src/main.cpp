#include "asset_generator.h"
#include <raylib.h>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <string>
#include <vector>

namespace {
constexpr int SCREEN_W = 1280;
constexpr int SCREEN_H = 720;
constexpr float LANE_WIDTH = 3.2f;
constexpr float PLAYER_Z = 4.0f;
constexpr float SEGMENT_LENGTH = 18.0f;
constexpr int SEGMENT_COUNT = 28;

struct Segment {
    float z = 0.0f;
    int style = 0;
    int obstacleLane = 0;
    int coinLane = 1;
    float obstacleHeight = 2.0f;
    bool coinCollected = false;
};

enum class Screen { Menu, Settings, Game };

float LaneX(int lane) { return (lane - 1) * LANE_WIDTH; }

float RandFloat(float a, float b) {
    return a + static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) * (b - a);
}

void GenerateSegment(Segment& s, float z) {
    s.z = z;
    s.style = std::rand() % 6;
    s.obstacleLane = std::rand() % 3;
    s.coinLane = (s.obstacleLane + 1 + std::rand() % 2) % 3;
    s.obstacleHeight = RandFloat(1.7f, 2.9f);
    s.coinCollected = false;
}

Model MeshModel(Mesh mesh) { return LoadModelFromMesh(mesh); }

void DrawNeonBox(Vector3 position, Vector3 size, Color body, Color neon) {
    DrawCube(position, size.x, size.y, size.z, body);
    DrawCubeWires(position, size.x + 0.06f, size.y + 0.06f, size.z + 0.06f, neon);
}

void DrawWindow(Vector3 p, Color glow) {
    DrawCube(p, 0.58f, 0.48f, 0.06f, glow);
    DrawCube({p.x, p.y, p.z - 0.045f}, 0.70f, 0.58f, 0.025f, Color{18, 24, 42, 255});
}

void DrawBuilding(float x, float z, float width, float height, float depth,
                  int style, bool leftSide) {
    const Color body = style % 2 == 0 ? Color{24, 29, 49, 255} : Color{34, 38, 62, 255};
    const Color neon = leftSide ? Color{55, 220, 255, 255} : Color{245, 70, 215, 255};
    DrawNeonBox({x, height / 2.0f, z}, {width, height, depth}, body, Color{70, 80, 110, 255});

    const int floors = std::max(3, static_cast<int>(height / 1.55f));
    const int columns = std::max(2, static_cast<int>(width / 1.25f));
    for (int floor = 0; floor < floors; ++floor) {
        const float y = 0.65f + floor * 1.55f;
        for (int col = 0; col < columns; ++col) {
            if ((floor + col + style) % 4 == 0) continue;
            const float px = x - width / 2.0f + 0.65f +
                             col * (width - 1.3f) / std::max(1, columns - 1);
            DrawWindow({px, y, z - depth / 2.0f - 0.045f}, neon);
        }
    }

    DrawCube({x, height + 0.12f, z}, width * 0.78f, 0.12f, depth * 0.58f, neon);
    if (style % 2 == 0) {
        DrawCube({x, height + 0.45f, z}, width * 0.32f, 0.55f, depth * 0.25f,
                 Color{48, 55, 80, 255});
        DrawCube({x, height + 0.74f, z}, width * 0.38f, 0.05f, depth * 0.30f, neon);
    }
    if (style % 3 == 0) {
        DrawCube({x + width * 0.28f, height * 0.5f, z - depth / 2.0f - 0.08f},
                 0.10f, height * 0.78f, 0.08f, neon);
    }
}

void DrawStreetLamp(float z, int side) {
    const float x = side * 5.35f;
    const Color glow = side < 0 ? Color{65, 225, 255, 255} : Color{245, 75, 220, 255};
    Model pole = MeshModel(GenMeshCylinder(0.075f, 3.5f, 12));
    DrawModel(pole, {x, 1.75f, z}, 1.0f, Color{65, 70, 90, 255});
    UnloadModel(pole);
    DrawCube({x, 3.42f, z}, 0.55f, 0.12f, 0.42f, glow);
    DrawSphere({x, 3.30f, z}, 0.18f, glow);
}

void DrawObstacle(const Segment& s, Model block, Model cone, Model cylinder) {
    const float x = LaneX(s.obstacleLane);
    const float h = s.obstacleHeight;
    const Color red{255, 70, 100, 255};

    if (s.style % 3 == 0) {
        DrawModel(block, {x, h / 2.0f, s.z}, 1.0f, Color{48, 32, 58, 255});
        for (int i = -2; i <= 2; ++i) {
            DrawCube({x + i * 0.34f, h * 0.58f, s.z - 0.93f},
                     0.16f, h * 0.52f, 0.07f, red);
        }
    } else if (s.style % 3 == 1) {
        DrawModel(cylinder, {x, h / 2.0f, s.z}, 1.0f, Color{42, 48, 70, 255});
        DrawModel(cone, {x, h + 0.35f, s.z}, 0.75f, red);
        DrawCube({x, h * 0.48f, s.z - 0.75f}, 1.25f, 0.16f, 0.08f, red);
    } else {
        DrawModel(block, {x, h / 2.0f, s.z}, 1.0f, Color{38, 44, 67, 255});
        DrawCube({x, h + 0.08f, s.z}, 2.05f, 0.12f, 2.0f, red);
        DrawCube({x, h * 0.52f, s.z - 0.93f}, 1.55f, 0.18f, 0.08f,
                 Color{255, 160, 175, 255});
    }
}

void DrawCoin(Vector3 p, Model coinModel) {
    DrawModelEx(coinModel, p, {0, 1, 0}, static_cast<float>(GetTime()) * 140.0f,
                 {1.0f, 1.0f, 0.22f}, Color{255, 215, 55, 255});
    DrawSphere(p, 0.10f, Color{255, 245, 150, 255});
}

void DrawPlayer(Model body, Model head, Model limb, Model boot,
                Vector3 p, float time, bool jumping) {
    const float swing = std::sin(time * 11.0f) * (jumping ? 0.05f : 0.35f);
    const float bob = jumping ? 0.0f : std::fabs(std::sin(time * 11.0f)) * 0.05f;
    const float y = p.y + bob;
    const Color suit{45, 175, 220, 255};
    const Color dark{20, 38, 68, 255};
    const Color glow{80, 235, 255, 255};

    DrawModel(body, {p.x, y + 0.75f, p.z}, 1.0f, suit);
    DrawModel(head, {p.x, y + 1.62f, p.z}, 1.0f, Color{105, 225, 250, 255});
    DrawCube({p.x, y + 1.62f, p.z - 0.40f}, 0.45f, 0.16f, 0.04f, dark);
    DrawModelEx(limb, {p.x - 0.30f, y + 0.10f, p.z}, {0, 0, 1}, swing * 35.0f,
                 {0.65f, 1.0f, 0.70f}, dark);
    DrawModelEx(limb, {p.x + 0.30f, y + 0.10f, p.z}, {0, 0, 1}, -swing * 35.0f,
                 {0.65f, 1.0f, 0.70f}, dark);
    DrawModelEx(boot, {p.x - 0.30f, y - 0.68f, p.z - 0.18f}, {0, 0, 1}, 0.0f,
                 {0.85f, 0.34f, 1.30f}, glow);
    DrawModelEx(boot, {p.x + 0.30f, y - 0.68f, p.z - 0.18f}, {0, 0, 1}, 0.0f,
                 {0.85f, 0.34f, 1.30f}, glow);
    DrawModelEx(limb, {p.x - 0.63f, y + 0.63f, p.z}, {0, 0, 1}, -swing * 28.0f,
                 {1.0f, 0.30f, 0.55f}, suit);
    DrawModelEx(limb, {p.x + 0.63f, y + 0.63f, p.z}, {0, 0, 1}, swing * 28.0f,
                 {1.0f, 0.30f, 0.55f}, suit);
}

void Button(const char* text, int y, bool selected) {
    const Color fill = selected ? Color{52, 105, 140, 255} : Color{20, 28, 46, 255};
    const Color line = selected ? Color{90, 230, 255, 255} : Color{70, 85, 115, 255};
    DrawRectangle(430, y, 420, 58, fill);
    DrawRectangleLines(430, y, 420, 58, line);
    DrawText(text, 640 - MeasureText(text, 22) / 2, y + 17, 22, RAYWHITE);
}

} // namespace

int main() {
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    InitWindow(SCREEN_W, SCREEN_H, "Neon Runner");
    SetTargetFPS(60);

    int loading = 0;
    std::string loadingText = "Starting asset pipeline...";
    GenerateAssets(std::filesystem::current_path().string(),
        [&](int value, const std::string& text) {
            loading = value;
            loadingText = text;
            BeginDrawing();
            ClearBackground(Color{6, 9, 18, 255});
            DrawText("NEON RUNNER", 440, 220, 48, Color{95, 225, 255, 255});
            DrawText(loadingText.c_str(), 420, 315, 22, RAYWHITE);
            DrawRectangle(360, 365, 560, 20, Color{28, 34, 52, 255});
            DrawRectangle(360, 365, 560 * loading / 100, 20, Color{70, 220, 255, 255});
            DrawText(TextFormat("%d%%", loading), 610, 405, 20, RAYWHITE);
            EndDrawing();
        });

    const std::string assetDir =
        (std::filesystem::current_path() / "assets" / "generated").string();
    InitAudioDevice();
    Sound coinSound = LoadSound((assetDir + "/coin.wav").c_str());
    Sound jumpSound = LoadSound((assetDir + "/jump.wav").c_str());
    Sound hitSound = LoadSound((assetDir + "/hit.wav").c_str());
    Sound stepSound = LoadSound((assetDir + "/step.wav").c_str());

    Model road = MeshModel(GenMeshCube(10.5f, 0.20f, SEGMENT_LENGTH));
    Model block = MeshModel(GenMeshCube(1.9f, 1.0f, 1.9f));
    Model cone = MeshModel(GenMeshCone(0.9f, 1.0f, 16));
    Model cylinder = MeshModel(GenMeshCylinder(0.9f, 2.0f, 20));
    Model coin = MeshModel(GenMeshCylinder(0.55f, 0.16f, 24));
    Model body = MeshModel(GenMeshCube(0.9f, 1.35f, 0.62f));
    Model head = MeshModel(GenMeshSphere(0.48f, 20, 14));
    Model limb = MeshModel(GenMeshCube(0.55f, 0.75f, 0.55f));
    Model boot = MeshModel(GenMeshCube(0.45f, 0.28f, 0.62f));

    Camera3D camera{};
    camera.position = {0.0f, 6.8f, 11.5f};
    camera.target = {0.0f, 1.3f, -13.0f};
    camera.up = {0.0f, 1.0f, 0.0f};
    camera.fovy = 58.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    std::vector<Segment> segments(SEGMENT_COUNT);
    auto resetWorld = [&]() {
        for (int i = 0; i < SEGMENT_COUNT; ++i) GenerateSegment(segments[i], -i * SEGMENT_LENGTH);
    };
    resetWorld();

    Screen screen = Screen::Menu;
    int menuSelection = 0;
    int settingsSelection = 0;
    int lane = 1;
    int score = 0;
    float playerY = 1.0f;
    float verticalVelocity = 0.0f;
    float speed = 17.0f;
    float distance = 0.0f;
    float masterVolume = 0.75f;
    float sfxVolume = 0.85f;
    bool fullscreen = false;
    bool gameOver = false;
    float stepTimer = 0.0f;

    SetMasterVolume(masterVolume);
    SetSoundVolume(coinSound, sfxVolume);
    SetSoundVolume(jumpSound, sfxVolume);
    SetSoundVolume(hitSound, sfxVolume);
    SetSoundVolume(stepSound, sfxVolume);

    while (!WindowShouldClose()) {
        const float dt = std::min(GetFrameTime(), 0.033f);

        if (screen == Screen::Menu) {
            if (IsKeyPressed(KEY_UP)) menuSelection = (menuSelection + 2) % 3;
            if (IsKeyPressed(KEY_DOWN)) menuSelection = (menuSelection + 1) % 3;
            if (IsKeyPressed(KEY_ENTER)) {
                if (menuSelection == 0) {
                    resetWorld(); lane = 1; score = 0; playerY = 1.0f;
                    verticalVelocity = 0.0f; speed = 17.0f; distance = 0.0f;
                    gameOver = false; screen = Screen::Game;
                } else if (menuSelection == 1) {
                    screen = Screen::Settings;
                } else {
                    break;
                }
            }
        } else if (screen == Screen::Settings) {
            if (IsKeyPressed(KEY_UP)) settingsSelection = (settingsSelection + 3) % 4;
            if (IsKeyPressed(KEY_DOWN)) settingsSelection = (settingsSelection + 1) % 4;
            if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_RIGHT)) {
                const float direction = IsKeyPressed(KEY_RIGHT) ? 0.05f : -0.05f;
                if (settingsSelection == 0) masterVolume = std::clamp(masterVolume + direction, 0.0f, 1.0f);
                if (settingsSelection == 1) sfxVolume = std::clamp(sfxVolume + direction, 0.0f, 1.0f);
                if (settingsSelection == 2) { fullscreen = !fullscreen; ToggleFullscreen(); }
                SetMasterVolume(masterVolume);
                SetSoundVolume(coinSound, sfxVolume);
                SetSoundVolume(jumpSound, sfxVolume);
                SetSoundVolume(hitSound, sfxVolume);
                SetSoundVolume(stepSound, sfxVolume);
            }
            if (IsKeyPressed(KEY_ESCAPE) || (IsKeyPressed(KEY_ENTER) && settingsSelection == 3)) screen = Screen::Menu;
        } else {
            if (IsKeyPressed(KEY_ESCAPE)) { screen = Screen::Menu; continue; }
            if (gameOver) {
                if (IsKeyPressed(KEY_R)) {
                    resetWorld(); lane = 1; score = 0; playerY = 1.0f;
                    verticalVelocity = 0.0f; speed = 17.0f; distance = 0.0f; gameOver = false;
                }
            } else {
                if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT)) lane = std::max(0, lane - 1);
                if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT)) lane = std::min(2, lane + 1);
                if (IsKeyPressed(KEY_SPACE) && playerY <= 1.01f) {
                    verticalVelocity = 10.5f;
                    PlaySound(jumpSound);
                }

                verticalVelocity -= 26.0f * dt;
                playerY += verticalVelocity * dt;
                if (playerY < 1.0f) { playerY = 1.0f; verticalVelocity = 0.0f; }
                speed = std::min(35.0f, speed + 0.55f * dt);
                distance += speed * dt;
                score = static_cast<int>(distance * 1.4f);

                for (auto& s : segments) s.z += speed * dt;
                for (auto& s : segments) {
                    if (s.z > SEGMENT_LENGTH) {
                        float farthest = s.z;
                        for (const auto& other : segments) farthest = std::min(farthest, other.z);
                        GenerateSegment(s, farthest - SEGMENT_LENGTH);
                    }
                }

                const Vector3 playerPos{LaneX(lane), playerY, PLAYER_Z};
                const BoundingBox playerBox{
                    {playerPos.x - 0.55f, playerPos.y - 0.70f, playerPos.z - 0.55f},
                    {playerPos.x + 0.55f, playerPos.y + 0.70f, playerPos.z + 0.55f}
                };

                for (auto& s : segments) {
                    const BoundingBox obstacleBox{
                        {LaneX(s.obstacleLane) - 0.9f, 0.0f, s.z - 0.9f},
                        {LaneX(s.obstacleLane) + 0.9f, s.obstacleHeight, s.z + 0.9f}
                    };
                    if (CheckCollisionBoxes(playerBox, obstacleBox) && playerY < s.obstacleHeight / 2.0f + 1.0f) {
                        gameOver = true;
                        PlaySound(hitSound);
                    }

                    if (!s.coinCollected) {
                        const float coinZ = s.z - 4.0f;
                        const float coinY = 1.45f + std::sin(static_cast<float>(GetTime()) * 4.0f + s.z) * 0.16f;
                        if (std::fabs(playerPos.x - LaneX(s.coinLane)) < 0.85f &&
                            std::fabs(playerPos.z - coinZ) < 0.85f &&
                            std::fabs(playerPos.y - coinY) < 1.0f) {
                            s.coinCollected = true;
                            score += 50;
                            PlaySound(coinSound);
                        }
                    }
                }

                stepTimer += dt;
                if (playerY <= 1.01f && stepTimer > 0.34f) {
                    stepTimer = 0.0f;
                    PlaySound(stepSound);
                }
            }
        }

        BeginDrawing();
        ClearBackground(Color{6, 9, 18, 255});

        if (screen == Screen::Menu) {
            DrawText("NEON RUNNER", 420, 145, 64, Color{90, 225, 255, 255});
            DrawText("INFINITE CITY // NIGHT SHIFT", 474, 215, 18, Color{165, 180, 210, 255});
            Button("PLAY", 300, menuSelection == 0);
            Button("SETTINGS", 375, menuSelection == 1);
            Button("QUIT", 450, menuSelection == 2);
            DrawText("ARROWS SELECT   ENTER CONFIRM", 500, 575, 18, Color{125, 145, 175, 255});
        } else if (screen == Screen::Settings) {
            DrawText("SETTINGS", 510, 100, 48, Color{90, 225, 255, 255});
            Button(TextFormat("MASTER VOLUME  %d%%", static_cast<int>(masterVolume * 100)), 220, settingsSelection == 0);
            Button(TextFormat("SFX VOLUME     %d%%", static_cast<int>(sfxVolume * 100)), 295, settingsSelection == 1);
            Button(fullscreen ? "FULLSCREEN     ON" : "FULLSCREEN     OFF", 370, settingsSelection == 2);
            Button("BACK", 445, settingsSelection == 3);
            DrawText("LEFT / RIGHT ADJUST   ESC BACK", 495, 570, 18, Color{125, 145, 175, 255});
        } else {
            BeginMode3D(camera);
            DrawPlane({0, -0.12f, 0}, {24, 1000}, Color{17, 21, 34, 255});
            for (const auto& s : segments) {
                DrawModel(road, {0, -0.02f, s.z}, 1.0f, Color{31, 36, 53, 255});
                DrawCube({-5.25f, 0.04f, s.z}, 0.08f, 0.12f, SEGMENT_LENGTH, Color{55, 220, 255, 255});
                DrawCube({5.25f, 0.04f, s.z}, 0.08f, 0.12f, SEGMENT_LENGTH, Color{245, 70, 215, 255});
                DrawBuilding(-7.4f, s.z, 4.5f, 6.0f + s.style * 1.1f, 15.0f, s.style, true);
                DrawBuilding(7.4f, s.z + 1.0f, 4.6f, 6.5f + (5 - s.style) * 0.9f, 15.0f, (s.style + 2) % 6, false);
                DrawStreetLamp(s.z, -1);
                DrawStreetLamp(s.z, 1);
                DrawObstacle(s, block, cone, cylinder);
                if (!s.coinCollected) {
                    const float bob = std::sin(static_cast<float>(GetTime()) * 4.0f + s.z) * 0.16f;
                    DrawCoin({LaneX(s.coinLane), 1.45f + bob, s.z - 4.0f}, coin);
                }
            }
            DrawPlayer(body, head, limb, boot, {LaneX(lane), playerY, PLAYER_Z},
                       static_cast<float>(GetTime()), playerY > 1.01f);
            EndMode3D();

            DrawRectangle(0, 0, SCREEN_W, 72, Color{5, 8, 18, 230});
            DrawText("NEON RUNNER", 25, 18, 27, Color{90, 225, 255, 255});
            DrawText(TextFormat("SCORE %06d", score), 275, 20, 24, RAYWHITE);
            DrawText(TextFormat("SPEED %.1f", speed), 480, 22, 19, Color{175, 185, 210, 255});
            DrawText("A/D MOVE   SPACE JUMP   ESC MENU", 850, 23, 16, Color{175, 185, 210, 255});

            if (gameOver) {
                DrawRectangle(0, 0, SCREEN_W, SCREEN_H, Color{4, 6, 12, 175});
                DrawText("SYSTEM FAILURE", 445, 250, 55, Color{255, 85, 110, 255});
                DrawText(TextFormat("FINAL SCORE  %d", score), 515, 330, 25, RAYWHITE);
                DrawText("PRESS R TO RESTART", 505, 390, 21, Color{130, 220, 255, 255});
            }
        }

        EndDrawing();
    }

    UnloadModel(road);
    UnloadModel(block);
    UnloadModel(cone);
    UnloadModel(cylinder);
    UnloadModel(coin);
    UnloadModel(body);
    UnloadModel(head);
    UnloadModel(limb);
    UnloadModel(boot);
    UnloadSound(coinSound);
    UnloadSound(jumpSound);
    UnloadSound(hitSound);
    UnloadSound(stepSound);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}
