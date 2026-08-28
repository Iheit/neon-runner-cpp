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
constexpr int W = 1280;
constexpr int H = 720;
constexpr float LANE = 3.2f;
constexpr float PLAYER_Z = 2.5f;
constexpr float SEG = 14.0f;
constexpr int SEGMENTS = 24;

struct Segment {
    float z = 0.0f;
    int obstacleLane = 0;
    int coinLane = 1;
    int style = 0;
    float obstacleHeight = 2.0f;
    bool coinCollected = false;
};

float LaneX(int lane) {
    return (lane - 1) * LANE;
}

float RandomFloat(float a, float b) {
    return a + static_cast<float>(std::rand()) / RAND_MAX * (b - a);
}

void GenerateSegment(Segment& s, float z) {
    s.z = z;
    s.obstacleLane = std::rand() % 3;
    s.coinLane = (s.obstacleLane + 1 + std::rand() % 2) % 3;
    s.style = std::rand() % 6;
    s.obstacleHeight = RandomFloat(1.7f, 2.8f);
    s.coinCollected = false;
}

Model MakeCube(float x, float y, float z) {
    return LoadModelFromMesh(GenMeshCube(x, y, z));
}

Model MakeCylinder(float radius, float height) {
    return LoadModelFromMesh(GenMeshCylinder(radius, height, 24));
}

Model MakeSphere(float radius) {
    return LoadModelFromMesh(GenMeshSphere(radius, 24, 16));
}

void DrawWindow(Vector3 p, Color color) {
    DrawCube(p, 0.52f, 0.48f, 0.045f, color);
    DrawCube({p.x, p.y, p.z - 0.035f}, 0.60f, 0.56f, 0.025f,
             Color{40, 50, 75, 255});
}

void DrawBuilding(Vector3 p, float width, float height, float depth,
                  int style, bool left) {
    const Color body = style % 2 ? Color{30, 35, 58, 255}
                                 : Color{23, 29, 48, 255};
    const Color neon = left ? Color{55, 220, 255, 255}
                            : Color{245, 70, 215, 255};

    DrawCube(p, width, height, depth, body);
    DrawCubeWires(p, width + 0.08f, height + 0.08f, depth + 0.08f,
                  Color{120, 135, 165, 255});

    const int floors = std::max(2, static_cast<int>(height / 1.6f));
    for (int floor = 0; floor < floors; ++floor) {
        const float y = p.y - height / 2.0f + 0.85f + floor * 1.6f;
        for (int col = 0; col < 4; ++col) {
            const float x = p.x - width / 2.0f + 0.65f +
                            col * (width - 1.3f) / 3.0f;
            if ((col + floor + style) % 3 != 0) {
                DrawWindow({x, y, p.z - depth / 2.0f - 0.035f}, neon);
            }
        }
    }

    for (int i = 0; i < 3; ++i) {
        const float x = p.x - width / 2.0f + 0.5f + i * (width - 1.0f) / 2.0f;
        DrawCube({x, p.y + height / 2.0f + 0.10f, p.z},
                 0.12f, 0.20f, depth * 0.7f, neon);
    }

    if (style % 2 == 0) {
        DrawCube({p.x, p.y + height / 2.0f + 0.32f, p.z},
                 width * 0.65f, 0.10f, depth * 0.45f, neon);
        DrawCube({p.x, p.y + height / 2.0f + 0.52f, p.z},
                 width * 0.32f, 0.28f, depth * 0.25f,
                 Color{55, 65, 90, 255});
    }
}

void DrawLamp(float z, int side) {
    const float x = side * 5.15f;
    const Color glow = side < 0 ? Color{55, 220, 255, 255}
                                : Color{245, 70, 215, 255};
    DrawCylinder({x, 1.7f, z}, 0.07f, 3.4f, 12,
                 Color{65, 70, 90, 255});
    DrawCube({x, 3.35f, z}, 0.5f, 0.12f, 0.42f, glow);
    DrawSphere({x, 3.23f, z}, 0.18f, glow);
}

void DrawObstacle(const Segment& s) {
    const float x = LaneX(s.obstacleLane);
    const float z = s.z;
    const float h = s.obstacleHeight;
    const Color red{255, 70, 95, 255};

    if (s.style % 3 == 0) {
        DrawCube({x, h / 2.0f, z}, 1.9f, h, 1.8f,
                 Color{55, 28, 50, 255});
        for (int i = 0; i < 5; ++i) {
            DrawCube({x - 0.72f + i * 0.36f, h * 0.55f, z - 0.92f},
                     0.16f, 0.75f, 0.06f, red);
        }
        DrawCubeWires({x, h / 2.0f, z}, 1.98f, h + 0.08f, 1.88f,
                      Color{255, 150, 170, 255});
    } else if (s.style % 3 == 1) {
        DrawCylinder({x, h / 2.0f, z}, 0.92f, h, 10,
                     Color{45, 48, 68, 255});
        DrawCylinder({x, h * 0.72f, z}, 0.70f, 0.14f, 24, red);
        DrawCylinder({x, h * 0.38f, z}, 0.60f, 0.10f, 24,
                     Color{255, 170, 185, 255});
    } else {
        DrawCube({x, h / 2.0f, z}, 1.8f, h, 1.75f,
                 Color{35, 40, 62, 255});
        for (int i = 0; i < 4; ++i) {
            DrawCube({x - 0.62f + i * 0.41f, h * 0.55f, z - 0.89f},
                     0.18f, h * 0.50f, 0.06f, red);
        }
        DrawCube({x, h + 0.10f, z}, 1.95f, 0.12f, 1.95f,
                 Color{255, 125, 150, 255});
    }
}

void DrawCoin(Vector3 p) {
    DrawCylinder(p, 0.48f, 0.12f, 32, Color{255, 210, 45, 255});
    DrawCylinder({p.x, p.y, p.z - 0.01f}, 0.29f, 0.14f, 20,
                 Color{255, 245, 130, 255});
}

void DrawSegment(const Segment& s, Model road) {
    DrawModel(road, {0, -0.1f, s.z}, 1.0f, Color{43, 48, 65, 255});

    const float leftHeight = 6.0f + s.style * 1.15f;
    const float rightHeight = 7.0f + (5 - s.style) * 0.9f;
    DrawBuilding({-7, leftHeight / 2.0f, s.z}, 4.3f, leftHeight,
                 11.5f, s.style, true);
    DrawBuilding({7, rightHeight / 2.0f, s.z + 1.0f}, 4.5f,
                 rightHeight, 11.5f, (s.style + 2) % 6, false);
    DrawLamp(s.z, -1);
    DrawLamp(s.z, 1);

    DrawCube({-5.25f, 2.8f, s.z}, 0.10f, 5.5f, 0.10f,
             Color{60, 225, 255, 255});
    DrawCube({5.25f, 2.8f, s.z}, 0.10f, 5.5f, 0.10f,
             Color{245, 75, 215, 255});

    DrawObstacle(s);
    if (!s.coinCollected) {
        const float bob = std::sin(static_cast<float>(GetTime()) * 4.0f + s.z) * 0.18f;
        DrawCoin({LaneX(s.coinLane), 1.45f + bob, s.z - 4.0f});
    }
}

void DrawPlayer(Model torso, Model head, Model limb, Model boot,
                Vector3 p, float t, bool jumping) {
    const float run = std::sin(t * 12.0f);
    const float bob = jumping ? 0.0f : std::fabs(run) * 0.06f;
    const float y = p.y + bob;
    const Color suit{45, 175, 220, 255};
    const Color glow{70, 225, 255, 255};
    const Color dark{24, 48, 82, 255};

    DrawModel(torso, {p.x, y + 0.72f, p.z}, 1.0f, suit);
    DrawModel(head, {p.x, y + 1.58f, p.z}, 1.0f,
              Color{105, 230, 255, 255});
    DrawSphere({p.x, y + 1.58f, p.z - 0.43f}, 0.11f, glow);

    const float arm = run * 0.34f;
    DrawModelEx(limb, {p.x - 0.27f + arm, y - 0.30f, p.z},
                {0, 0, 1}, run * 18.0f, {0.70f, 1.0f, 0.75f}, dark);
    DrawModelEx(limb, {p.x + 0.27f - arm, y - 0.30f, p.z},
                {0, 0, 1}, -run * 18.0f, {0.70f, 1.0f, 0.75f}, dark);
    DrawModelEx(boot, {p.x - 0.27f + arm * 1.5f, y - 0.72f, p.z - 0.18f},
                {0, 0, 1}, 0.0f, {0.90f, 0.30f, 1.35f}, glow);
    DrawModelEx(boot, {p.x + 0.27f - arm * 1.5f, y - 0.72f, p.z - 0.18f},
                {0, 0, 1}, 0.0f, {0.90f, 0.30f, 1.35f}, glow);
    DrawModelEx(limb, {p.x - 0.60f - arm, y + 0.60f, p.z},
                {0, 0, 1}, -run * 16.0f, {1.05f, 0.30f, 0.55f}, suit);
    DrawModelEx(limb, {p.x + 0.60f + arm, y + 0.60f, p.z},
                {0, 0, 1}, run * 16.0f, {1.05f, 0.30f, 0.55f}, suit);
}

void DrawButton(const char* text, int y, bool selected) {
    const Color fill = selected ? Color{55, 110, 145, 255}
                                : Color{22, 30, 48, 255};
    const Color outline = selected ? Color{100, 230, 255, 255}
                                   : Color{65, 80, 110, 255};
    DrawRectangle(430, y, 420, 58, fill);
    DrawRectangleLines(430, y, 420, 58, outline);
    const int width = MeasureText(text, 22);
    DrawText(text, 640 - width / 2, y + 17, 22, RAYWHITE);
}
}

int main() {
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    InitWindow(W, H, "Neon Runner");
    SetTargetFPS(60);

    int progress = 0;
    std::string status = "Generating procedural assets...";
    GenerateAssets(std::filesystem::current_path().string(),
        [&](int value, const std::string& text) {
            progress = value;
            status = text;
            BeginDrawing();
            ClearBackground(Color{7, 10, 20, 255});
            DrawText("NEON RUNNER", 440, 220, 48,
                     Color{100, 225, 255, 255});
            DrawText(status.c_str(), 430, 315, 22, RAYWHITE);
            DrawRectangle(360, 365, 560, 20, Color{30, 35, 55, 255});
            DrawRectangle(360, 365, 560 * progress / 100, 20,
                          Color{70, 220, 255, 255});
            DrawText(TextFormat("%d%%", progress), 610, 405, 20, RAYWHITE);
            EndDrawing();
        });

    const std::string assetRoot =
        (std::filesystem::current_path() / "assets" / "generated").string();
    InitAudioDevice();
    Sound coinSound = LoadSound((assetRoot + "/coin.wav").c_str());
    Sound jumpSound = LoadSound((assetRoot + "/jump.wav").c_str());
    Sound hitSound = LoadSound((assetRoot + "/hit.wav").c_str());
    Sound stepSound = LoadSound((assetRoot + "/step.wav").c_str());

    Model road = MakeCube(10.0f, 0.18f, SEG);
    Model torso = MakeCylinder(0.55f, 1.3f);
    Model head = MakeSphere(0.48f);
    Model limb = MakeCube(0.55f, 0.75f, 0.55f);
    Model boot = MakeCube(0.4f, 0.25f, 0.4f);

    Camera3D camera{};
    camera.position = {0, 6.7f, 11};
    camera.target = {0, 1.3f, -12};
    camera.up = {0, 1, 0};
    camera.fovy = 58;
    camera.projection = CAMERA_PERSPECTIVE;

    std::vector<Segment> segments(SEGMENTS);
    for (int i = 0; i < SEGMENTS; ++i) {
        GenerateSegment(segments[i], -i * SEG);
    }

    int lane = 1;
    int score = 0;
    int menu = 0;
    int menuChoice = 0;
    int collectedCoins = 0;
    float playerY = 1.0f;
    float velocityY = 0.0f;
    float speed = 18.0f;
    float distance = 0.0f;
    float runTime = 0.0f;
    float stepTimer = 0.0f;
    float masterVolume = 1.0f;
    float musicVolume = 1.0f;
    float sfxVolume = 1.0f;
    bool gameOver = false;
    bool fullscreen = false;

    auto reset = [&]() {
        lane = 1;
        score = 0;
        collectedCoins = 0;
        playerY = 1.0f;
        velocityY = 0.0f;
        speed = 18.0f;
        distance = 0.0f;
        runTime = 0.0f;
        stepTimer = 0.0f;
        gameOver = false;
        for (int i = 0; i < SEGMENTS; ++i) {
            GenerateSegment(segments[i], -i * SEG);
        }
    };

    while (!WindowShouldClose()) {
        const float dt = std::min(GetFrameTime(), 0.033f);

        if (menu == 0) {
            if (IsKeyPressed(KEY_UP)) menuChoice = (menuChoice + 2) % 3;
            if (IsKeyPressed(KEY_DOWN)) menuChoice = (menuChoice + 1) % 3;
            if (IsKeyPressed(KEY_ENTER)) {
                if (menuChoice == 0) menu = 1;
                else if (menuChoice == 1) { menu = 2; menuChoice = 0; }
                else break;
            }
        } else if (menu == 2) {
            if (IsKeyPressed(KEY_UP)) menuChoice = (menuChoice + 4) % 5;
            if (IsKeyPressed(KEY_DOWN)) menuChoice = (menuChoice + 1) % 5;
            if (IsKeyPressed(KEY_ESCAPE)) menu = 0;
            float* value = nullptr;
            if (menuChoice == 0) value = &masterVolume;
            if (menuChoice == 1) value = &musicVolume;
            if (menuChoice == 2) value = &sfxVolume;
            if (value && IsKeyPressed(KEY_LEFT)) *value = std::max(0.0f, *value - 0.1f);
            if (value && IsKeyPressed(KEY_RIGHT)) *value = std::min(1.0f, *value + 0.1f);
            if (menuChoice == 3 && IsKeyPressed(KEY_ENTER)) {
                fullscreen = !fullscreen;
                ToggleFullscreen();
            }
            if (menuChoice == 4 && IsKeyPressed(KEY_ENTER)) menu = 0;
        } else {
            if (IsKeyPressed(KEY_ESCAPE)) { menu = 0; continue; }

            if (!gameOver) {
                if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT)) lane = std::max(0, lane - 1);
                if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT)) lane = std::min(2, lane + 1);
                if (IsKeyPressed(KEY_SPACE) && playerY <= 1.01f) {
                    velocityY = 10.0f;
                    SetSoundVolume(jumpSound, masterVolume * sfxVolume);
                    PlaySound(jumpSound);
                }

                velocityY -= 25.0f * dt;
                playerY += velocityY * dt;
                if (playerY < 1.0f) { playerY = 1.0f; velocityY = 0.0f; }

                speed = std::min(42.0f, speed + 0.42f * dt);
                distance += speed * dt;
                runTime += dt;
                stepTimer += dt;
                if (stepTimer > 0.4f && playerY <= 1.05f) {
                    SetSoundVolume(stepSound, masterVolume * sfxVolume);
                    PlaySound(stepSound);
                    stepTimer = 0.0f;
                }

                float farthest = 0.0f;
                for (auto& s : segments) {
                    s.z += speed * dt;
                    farthest = std::min(farthest, s.z);
                }
                for (auto& s : segments) {
                    if (s.z > 22.0f) {
                        s.z = farthest - SEG;
                        GenerateSegment(s, s.z);
                        farthest = s.z;
                    }
                }

                const Vector3 player = {LaneX(lane), playerY, PLAYER_Z};
                const BoundingBox playerBox = {
                    {player.x - 0.55f, player.y - 0.75f, player.z - 0.45f},
                    {player.x + 0.55f, player.y + 0.80f, player.z + 0.45f}
                };

                for (auto& s : segments) {
                    if (s.obstacleLane == lane && std::fabs(s.z - PLAYER_Z) < 1.15f) {
                        const BoundingBox obstacleBox = {
                            {LaneX(lane) - 0.9f, 0.0f, s.z - 0.9f},
                            {LaneX(lane) + 0.9f, s.obstacleHeight, s.z + 0.9f}
                        };
                        if (CheckCollisionBoxes(playerBox, obstacleBox)) {
                            gameOver = true;
                            SetSoundVolume(hitSound, masterVolume * sfxVolume);
                            PlaySound(hitSound);
                        }
                    }

                    const float coinZ = s.z - 4.0f;
                    if (!s.coinCollected && s.coinLane == lane &&
                        std::fabs(coinZ - PLAYER_Z) < 0.95f &&
                        std::fabs(playerY - 1.45f) < 1.0f) {
                        s.coinCollected = true;
                        ++collectedCoins;
                        SetSoundVolume(coinSound, masterVolume * sfxVolume);
                        PlaySound(coinSound);
                    }
                }

                score = static_cast<int>(distance * 1.25f) + collectedCoins * 25;
            } else if (IsKeyPressed(KEY_R)) {
                reset();
            }
        }

        BeginDrawing();
        ClearBackground(Color{7, 10, 20, 255});

        if (menu == 1) {
            BeginMode3D(camera);
            for (const auto& s : segments) DrawSegment(s, road);
            DrawPlayer(torso, head, limb, boot,
                       {LaneX(lane), playerY, PLAYER_Z}, runTime, playerY > 1.05f);
            EndMode3D();

            DrawRectangle(0, 0, W, 78, Color{5, 8, 18, 225});
            DrawText("NEON RUNNER", 28, 18, 28, Color{100, 225, 255, 255});
            DrawText(TextFormat("SCORE %05d", score), 275, 20, 26, RAYWHITE);
            DrawText(TextFormat("SPEED %.1f", speed), 480, 22, 20,
                     Color{175, 185, 210, 255});
            DrawText("A/D MOVE  SPACE JUMP  ESC MENU", 820, 24, 16,
                     Color{175, 185, 210, 255});

            if (gameOver) {
                DrawRectangle(0, 0, W, H, Color{5, 7, 15, 175});
                DrawText("SYSTEM FAILURE", 450, 255, 54,
                         Color{255, 90, 110, 255});
                DrawText(TextFormat("SCORE %d", score), 565, 330, 25, RAYWHITE);
                DrawText("PRESS R TO RESTART", 505, 390, 22,
                         Color{150, 220, 255, 255});
            }
        } else if (menu == 0) {
            DrawText("NEON", 455, 150, 78, Color{80, 225, 255, 255});
            DrawText("RUNNER", 448, 220, 62, Color{245, 75, 215, 255});
            DrawText("INFINITE CITY", 505, 290, 20, Color{175, 185, 210, 255});
            DrawButton("PLAY", 350, menuChoice == 0);
            DrawButton("SETTINGS", 420, menuChoice == 1);
            DrawButton("QUIT", 490, menuChoice == 2);
            DrawText("UP / DOWN SELECT     ENTER CONFIRM", 420, 610, 18,
                     Color{125, 145, 175, 255});
        } else {
            DrawText("SETTINGS", 495, 100, 46, Color{100, 225, 255, 255});
            DrawButton(TextFormat("MASTER VOLUME  %d%%", static_cast<int>(masterVolume * 100)), 210, menuChoice == 0);
            DrawButton(TextFormat("MUSIC VOLUME   %d%%", static_cast<int>(musicVolume * 100)), 280, menuChoice == 1);
            DrawButton(TextFormat("SFX VOLUME     %d%%", static_cast<int>(sfxVolume * 100)), 350, menuChoice == 2);
            DrawButton(fullscreen ? "FULLSCREEN: ON" : "FULLSCREEN: OFF", 420, menuChoice == 3);
            DrawButton("BACK", 490, menuChoice == 4);
            DrawText("LEFT / RIGHT CHANGE   ENTER TO TOGGLE", 405, 610, 18,
                     Color{125, 145, 175, 255});
        }

        EndDrawing();
    }

    UnloadSound(coinSound);
    UnloadSound(jumpSound);
    UnloadSound(hitSound);
    UnloadSound(stepSound);
    UnloadModel(road);
    UnloadModel(torso);
    UnloadModel(head);
    UnloadModel(limb);
    UnloadModel(boot);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}
