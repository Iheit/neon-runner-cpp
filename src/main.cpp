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
constexpr int SCREEN_WIDTH = 1280;
constexpr int SCREEN_HEIGHT = 720;
constexpr float LANE_WIDTH = 3.2f;
constexpr float PLAYER_Z = 2.5f;

struct Obstacle {
    Vector3 position{};
    float width = 1.8f;
    float height = 2.0f;
    float depth = 2.0f;
};

struct Coin {
    Vector3 position{};
    bool collected = false;
    float time = 0.0f;
};

float LaneX(int lane) {
    return (lane - 1) * LANE_WIDTH;
}

float RandomFloat(float min, float max) {
    return min + static_cast<float>(std::rand()) /
                       static_cast<float>(RAND_MAX) * (max - min);
}

void DrawWorld(float distance) {
    DrawPlane({0.0f, -0.08f, 0.0f}, {24.0f, 180.0f}, Color{20, 24, 38, 255});

    DrawCube({-5.25f, 0.02f, -25.0f}, 0.08f, 0.12f, 180.0f,
             Color{70, 220, 255, 255});
    DrawCube({5.25f, 0.02f, -25.0f}, 0.08f, 0.12f, 180.0f,
             Color{70, 220, 255, 255});

    const float offset = std::fmod(distance * 0.9f, 8.0f);
    for (int divider = 0; divider < 2; ++divider) {
        const float x = divider == 0 ? -LANE_WIDTH / 2.0f : LANE_WIDTH / 2.0f;
        for (int i = -12; i < 14; ++i) {
            DrawCube({x, 0.055f, i * 8.0f + offset}, 0.09f, 0.04f, 3.5f,
                     Color{115, 125, 150, 255});
        }
    }

    for (int i = -9; i <= 9; i += 2) {
        const float height = 4.0f + std::abs(i % 5) * 1.8f;
        DrawCube({i * 3.6f, height / 2.0f, -45.0f}, 2.6f, height, 2.6f,
                 Color{24, 28, 48, 255});
    }
}

void DrawObstacle(const Obstacle& obstacle) {
    DrawCube(obstacle.position, obstacle.width, obstacle.height,
             obstacle.depth, Color{255, 70, 95, 255});
    DrawCubeWires(obstacle.position, obstacle.width + 0.05f,
                  obstacle.height + 0.05f, obstacle.depth + 0.05f,
                  Color{255, 150, 165, 255});
}

void DrawCoin(const Coin& coin) {
    if (coin.collected) {
        return;
    }

    const float bob = std::sin(coin.time * 3.0f) * 0.15f;
    const Vector3 position = {
        coin.position.x,
        coin.position.y + bob,
        coin.position.z
    };

    DrawCylinder(position, 0.48f, 0.48f, 0.10f, 32,
                 Color{255, 220, 70, 255});
    DrawCylinderWires(position, 0.50f, 0.50f, 0.12f, 32,
                      Color{255, 250, 170, 255});
}

void DrawPlayer(Vector3 position) {
    DrawCube(position, 1.45f, 1.45f, 1.45f,
             Color{70, 220, 255, 255});
    DrawCubeWires(position, 1.50f, 1.50f, 1.50f,
                  Color{170, 245, 255, 255});
    DrawCube({position.x, position.y + 0.05f, position.z - 0.74f},
             0.75f, 0.65f, 0.06f, Color{10, 16, 30, 255});
}
}

int main() {
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Neon Runner");
    SetTargetFPS(60);

    int progress = 0;
    std::string progressText = "Starting...";

    GenerateAssets(
        std::filesystem::current_path().string(),
        [&](int value, const std::string& text) {
            progress = value;
            progressText = text;

            BeginDrawing();
            ClearBackground(Color{7, 10, 20, 255});
            DrawText("NEON RUNNER", 440, 220, 48,
                     Color{100, 225, 255, 255});
            DrawText(progressText.c_str(), 460, 315, 22, RAYWHITE);
            DrawRectangle(360, 365, 560, 20,
                          Color{30, 35, 55, 255});
            DrawRectangle(360, 365, 560 * progress / 100, 20,
                          Color{70, 220, 255, 255});
            DrawText(TextFormat("%d%%", progress), 610, 405, 20, RAYWHITE);
            EndDrawing();
        });

    Camera3D camera{};
    camera.position = {0.0f, 7.0f, 10.5f};
    camera.target = {0.0f, 1.1f, -8.0f};
    camera.up = {0.0f, 1.0f, 0.0f};
    camera.fovy = 55.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    int lane = 1;
    int score = 0;
    float playerY = 1.0f;
    float verticalVelocity = 0.0f;
    float speed = 18.0f;
    float distance = 0.0f;
    bool gameOver = false;

    std::vector<Obstacle> obstacles;
    std::vector<Coin> coins;

    auto reset = [&]() {
        lane = 1;
        score = 0;
        playerY = 1.0f;
        verticalVelocity = 0.0f;
        speed = 18.0f;
        distance = 0.0f;
        gameOver = false;
        obstacles.clear();
        coins.clear();

        for (int i = 0; i < 10; ++i) {
            const float z = -18.0f - i * 15.0f;

            if (i % 2 == 0) {
                obstacles.push_back({{LaneX((i + 1) % 3), 1.0f, z}});
            }

            coins.push_back({
                {LaneX((i + 2) % 3), 1.3f, z - 5.0f},
                false,
                RandomFloat(0.0f, 6.28f)
            });
        }
    };

    reset();

    while (!WindowShouldClose()) {
        const float deltaTime = std::min(GetFrameTime(), 0.033f);

        if (!gameOver) {
            if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT)) {
                lane = std::max(0, lane - 1);
            }

            if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT)) {
                lane = std::min(2, lane + 1);
            }

            if (IsKeyPressed(KEY_SPACE) && playerY <= 1.01f) {
                verticalVelocity = 10.0f;
            }

            verticalVelocity -= 25.0f * deltaTime;
            playerY += verticalVelocity * deltaTime;

            if (playerY < 1.0f) {
                playerY = 1.0f;
                verticalVelocity = 0.0f;
            }

            speed = std::min(34.0f, speed + 0.55f * deltaTime);
            distance += speed * deltaTime;
            score = static_cast<int>(distance * 1.25f);

            for (auto& obstacle : obstacles) {
                obstacle.position.z += speed * deltaTime;
            }

            for (auto& coin : coins) {
                coin.position.z += speed * deltaTime;
                coin.time += deltaTime * 5.0f;
            }

            float farthestZ = -100.0f;
            for (const auto& obstacle : obstacles) {
                farthestZ = std::min(farthestZ, obstacle.position.z);
            }
            for (const auto& coin : coins) {
                farthestZ = std::min(farthestZ, coin.position.z);
            }

            if (farthestZ > -100.0f) {
                const float z = farthestZ - RandomFloat(24.0f, 34.0f);
                const int obstacleLane = std::rand() % 3;

                obstacles.push_back({
                    {LaneX(obstacleLane), 1.0f, z},
                    1.8f,
                    RandomFloat(1.7f, 2.5f),
                    2.0f
                });

                const int coinLane =
                    (obstacleLane + 1 + std::rand() % 2) % 3;

                coins.push_back({
                    {LaneX(coinLane), RandomFloat(1.2f, 1.8f), z - 5.0f},
                    false,
                    RandomFloat(0.0f, 6.28f)
                });
            }

            const Vector3 playerPosition = {
                LaneX(lane), playerY, PLAYER_Z
            };

            const BoundingBox playerBox = {
                {playerPosition.x - 0.62f,
                 playerPosition.y - 0.68f,
                 playerPosition.z - 0.62f},
                {playerPosition.x + 0.62f,
                 playerPosition.y + 0.68f,
                 playerPosition.z + 0.62f}
            };

            for (const auto& obstacle : obstacles) {
                const BoundingBox obstacleBox = {
                    {obstacle.position.x - obstacle.width / 2.0f,
                     obstacle.position.y - obstacle.height / 2.0f,
                     obstacle.position.z - obstacle.depth / 2.0f},
                    {obstacle.position.x + obstacle.width / 2.0f,
                     obstacle.position.y + obstacle.height / 2.0f,
                     obstacle.position.z + obstacle.depth / 2.0f}
                };

                if (CheckCollisionBoxes(playerBox, obstacleBox)) {
                    gameOver = true;
                    break;
                }
            }

            for (auto& coin : coins) {
                const float coinY =
                    coin.position.y + std::sin(coin.time * 3.0f) * 0.15f;

                if (!coin.collected &&
                    std::fabs(coin.position.x - playerPosition.x) < 0.9f &&
                    std::fabs(coin.position.z - playerPosition.z) < 0.9f &&
                    std::fabs(coinY - playerPosition.y) < 1.0f) {
                    coin.collected = true;
                    score += 25;
                }
            }

            obstacles.erase(
                std::remove_if(
                    obstacles.begin(), obstacles.end(),
                    [](const Obstacle& obstacle) {
                        return obstacle.position.z > 18.0f;
                    }),
                obstacles.end());

            coins.erase(
                std::remove_if(
                    coins.begin(), coins.end(),
                    [](const Coin& coin) {
                        return coin.position.z > 18.0f || coin.collected;
                    }),
                coins.end());
        } else if (IsKeyPressed(KEY_R)) {
            reset();
        }

        BeginDrawing();
        ClearBackground(Color{7, 10, 20, 255});

        BeginMode3D(camera);
        DrawWorld(distance);

        for (const auto& obstacle : obstacles) {
            DrawObstacle(obstacle);
        }

        for (const auto& coin : coins) {
            DrawCoin(coin);
        }

        DrawPlayer({LaneX(lane), playerY, PLAYER_Z});
        EndMode3D();

        DrawRectangle(0, 0, SCREEN_WIDTH, 78,
                      Color{5, 8, 18, 225});
        DrawText("NEON RUNNER", 28, 18, 28,
                 Color{100, 225, 255, 255});
        DrawText(TextFormat("SCORE %05d", score), 275, 20, 26, RAYWHITE);
        DrawText(TextFormat("SPEED %.1f", speed), 480, 22, 20,
                 Color{175, 185, 210, 255});
        DrawText("A/D MOVE   SPACE JUMP", 900, 24, 17,
                 Color{175, 185, 210, 255});

        if (gameOver) {
            DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT,
                          Color{5, 7, 15, 170});
            DrawText("SYSTEM FAILURE", 450, 255, 54,
                     Color{255, 90, 110, 255});
            DrawText(TextFormat("SCORE %d", score), 565, 330, 25,
                     RAYWHITE);
            DrawText("PRESS R TO RESTART", 505, 390, 22,
                     Color{150, 220, 255, 255});
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
