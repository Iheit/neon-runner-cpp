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
constexpr float SEGMENT_LENGTH = 14.0f;
constexpr int SEGMENT_COUNT = 18;

struct Segment {
    float z = 0.0f;
    int obstacleLane = -1;
    int coinLane = -1;
    int buildingStyle = 0;
    float obstacleHeight = 2.0f;
    bool coinCollected = false;
};

float LaneX(int lane) {
    return (lane - 1) * LANE_WIDTH;
}

float RandomFloat(float min, float max) {
    return min + static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) * (max - min);
}

int RandomLane() {
    return std::rand() % 3;
}

void GenerateSegment(Segment& segment, float z) {
    segment.z = z;
    segment.obstacleLane = RandomLane();
    segment.coinLane = (segment.obstacleLane + 1 + std::rand() % 2) % 3;
    segment.buildingStyle = std::rand() % 4;
    segment.obstacleHeight = RandomFloat(1.7f, 2.5f);
    segment.coinCollected = false;
}

void DrawSegment(const Segment& segment, Texture2D roadTexture,
                Texture2D buildingTexture, Texture2D obstacleTexture,
                Texture2D neonTexture) {
    DrawCubeTexture(roadTexture, {0.0f, -0.10f, segment.z}, 10.0f, 0.18f,
                    SEGMENT_LENGTH, WHITE);

    const float side = segment.buildingStyle % 2 == 0 ? 1.0f : -1.0f;
    const float leftHeight = 5.0f + segment.buildingStyle * 1.7f;
    const float rightHeight = 7.0f + (3 - segment.buildingStyle) * 1.2f;

    DrawCubeTexture(buildingTexture,
                    {-7.2f, leftHeight / 2.0f, segment.z + side},
                    3.5f, leftHeight, SEGMENT_LENGTH * 0.86f, WHITE);
    DrawCubeTexture(buildingTexture,
                    {7.2f, rightHeight / 2.0f, segment.z - side},
                    3.5f, rightHeight, SEGMENT_LENGTH * 0.86f, WHITE);

    DrawCubeTexture(neonTexture,
                    {-5.25f, 2.5f, segment.z},
                    0.08f, 5.0f, 0.08f, WHITE);
    DrawCubeTexture(neonTexture,
                    {5.25f, 2.5f, segment.z},
                    0.08f, 5.0f, 0.08f, WHITE);

    if (segment.obstacleLane >= 0) {
        const Vector3 obstaclePosition = {
            LaneX(segment.obstacleLane),
            segment.obstacleHeight / 2.0f,
            segment.z
        };
        DrawCubeTexture(obstacleTexture, obstaclePosition, 1.8f,
                        segment.obstacleHeight, 1.8f, WHITE);
        DrawCubeWires(obstaclePosition, 1.85f,
                      segment.obstacleHeight + 0.05f, 1.85f,
                      Color{255, 130, 150, 255});
    }

    if (segment.coinLane >= 0 && !segment.coinCollected) {
        const float bob = std::sin(segment.z * 0.17f) * 0.18f;
        DrawCylinder({LaneX(segment.coinLane), 1.45f + bob, segment.z - 4.0f},
                     0.48f, 0.48f, 0.12f, 24,
                     Color{255, 215, 65, 255});
        DrawCylinderWires({LaneX(segment.coinLane), 1.45f + bob, segment.z - 4.0f},
                          0.51f, 0.51f, 0.14f, 24,
                          Color{255, 245, 170, 255});
    }
}

void DrawAnimatedPlayer(Vector3 basePosition, float runTime, bool jumping) {
    const float run = std::sin(runTime * 12.0f);
    const float bob = jumping ? 0.0f : std::fabs(run) * 0.06f;
    const float bodyY = basePosition.y + bob;

    DrawCylinder({basePosition.x, bodyY + 0.55f, basePosition.z},
                 0.48f, 0.62f, 1.35f, 20,
                 Color{45, 175, 220, 255});
    DrawSphere({basePosition.x, bodyY + 1.45f, basePosition.z},
               0.47f, Color{105, 230, 255, 255});

    DrawSphere({basePosition.x - 0.17f, bodyY + 1.50f, basePosition.z - 0.39f},
               0.07f, Color{10, 20, 35, 255});
    DrawSphere({basePosition.x + 0.17f, bodyY + 1.50f, basePosition.z - 0.39f},
               0.07f, Color{10, 20, 35, 255});

    const float legA = run * 0.32f;
    const float legB = -run * 0.32f;
    DrawCube({basePosition.x - 0.23f + legA, bodyY - 0.38f,
              basePosition.z}, 0.28f, 0.75f, 0.30f,
             Color{35, 90, 145, 255});
    DrawCube({basePosition.x + 0.23f + legB, bodyY - 0.38f,
              basePosition.z}, 0.28f, 0.75f, 0.30f,
             Color{35, 90, 145, 255});

    DrawCube({basePosition.x - 0.55f + legB, bodyY + 0.58f,
              basePosition.z}, 0.72f, 0.18f, 0.18f,
             Color{70, 220, 255, 255});
    DrawCube({basePosition.x + 0.55f + legA, bodyY + 0.58f,
              basePosition.z}, 0.72f, 0.18f, 0.18f,
             Color{70, 220, 255, 255});
}

} // namespace

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
            DrawText(progressText.c_str(), 430, 315, 22, RAYWHITE);
            DrawRectangle(360, 365, 560, 20, Color{30, 35, 55, 255});
            DrawRectangle(360, 365, 560 * progress / 100, 20,
                          Color{70, 220, 255, 255});
            DrawText(TextFormat("%d%%", progress), 610, 405, 20, RAYWHITE);
            EndDrawing();
        });

    const std::string assetRoot =
        (std::filesystem::current_path() / "assets" / "generated").string();

    Texture2D roadTexture = LoadTexture((assetRoot + "/road.png").c_str());
    Texture2D buildingTexture = LoadTexture((assetRoot + "/building.png").c_str());
    Texture2D obstacleTexture = LoadTexture((assetRoot + "/obstacle.png").c_str());
    Texture2D neonTexture = LoadTexture((assetRoot + "/neon_panel.png").c_str());

    InitAudioDevice();
    Sound coinSound = LoadSound((assetRoot + "/coin.wav").c_str());
    Sound jumpSound = LoadSound((assetRoot + "/jump.wav").c_str());
    Sound hitSound = LoadSound((assetRoot + "/hit.wav").c_str());
    Sound stepSound = LoadSound((assetRoot + "/step.wav").c_str());

    Camera3D camera{};
    camera.position = {0.0f, 6.7f, 11.0f};
    camera.target = {0.0f, 1.3f, -12.0f};
    camera.up = {0.0f, 1.0f, 0.0f};
    camera.fovy = 58.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    std::vector<Segment> segments(SEGMENT_COUNT);
    for (int i = 0; i < SEGMENT_COUNT; ++i) {
        GenerateSegment(segments[i], -i * SEGMENT_LENGTH);
    }

    int lane = 1;
    int score = 0;
    float playerY = 1.0f;
    float verticalVelocity = 0.0f;
    float speed = 18.0f;
    float distance = 0.0f;
    float runTime = 0.0f;
    float stepTimer = 0.0f;
    bool gameOver = false;

    auto reset = [&]() {
        lane = 1;
        score = 0;
        playerY = 1.0f;
        verticalVelocity = 0.0f;
        speed = 18.0f;
        distance = 0.0f;
        runTime = 0.0f;
        stepTimer = 0.0f;
        gameOver = false;
        for (int i = 0; i < SEGMENT_COUNT; ++i) {
            GenerateSegment(segments[i], -i * SEGMENT_LENGTH);
        }
    };

    reset();

    while (!WindowShouldClose()) {
        const float dt = std::min(GetFrameTime(), 0.033f);

        if (!gameOver) {
            if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT)) {
                lane = std::max(0, lane - 1);
            }
            if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT)) {
                lane = std::min(2, lane + 1);
            }
            if (IsKeyPressed(KEY_SPACE) && playerY <= 1.01f) {
                verticalVelocity = 10.0f;
                PlaySound(jumpSound);
            }

            verticalVelocity -= 25.0f * dt;
            playerY += verticalVelocity * dt;
            if (playerY < 1.0f) {
                playerY = 1.0f;
                verticalVelocity = 0.0f;
            }

            speed = std::min(42.0f, speed + 0.42f * dt);
            distance += speed * dt;
            runTime += dt;
            stepTimer += dt;
            score = static_cast<int>(distance * 1.25f);

            if (stepTimer > 0.38f && playerY <= 1.05f) {
                PlaySound(stepSound);
                stepTimer = 0.0f;
            }

            float farthestZ = 0.0f;
            for (const auto& segment : segments) {
                farthestZ = std::min(farthestZ, segment.z);
            }

            for (auto& segment : segments) {
                segment.z += speed * dt;
            }

            for (auto& segment : segments) {
                if (segment.z > 22.0f) {
                    segment.z = farthestZ - SEGMENT_LENGTH;
                    GenerateSegment(segment, segment.z);
                    farthestZ = segment.z;
                }
            }

            const Vector3 playerPosition = {LaneX(lane), playerY, PLAYER_Z};
            const BoundingBox playerBox = {
                {playerPosition.x - 0.55f, playerPosition.y - 0.75f,
                 playerPosition.z - 0.45f},
                {playerPosition.x + 0.55f, playerPosition.y + 0.80f,
                 playerPosition.z + 0.45f}
            };

            for (auto& segment : segments) {
                if (segment.obstacleLane >= 0 &&
                    std::fabs(segment.z - PLAYER_Z) < 1.15f &&
                    segment.obstacleLane == lane) {
                    const BoundingBox obstacleBox = {
                        {LaneX(lane) - 0.9f, 0.0f, segment.z - 0.9f},
                        {LaneX(lane) + 0.9f, segment.obstacleHeight, segment.z + 0.9f}
                    };
                    if (CheckCollisionBoxes(playerBox, obstacleBox)) {
                        gameOver = true;
                        PlaySound(hitSound);
                    }
                }

                const float coinZ = segment.z - 4.0f;
                if (!segment.coinCollected && segment.coinLane == lane &&
                    std::fabs(coinZ - PLAYER_Z) < 0.95f &&
                    std::fabs((playerY + 0.0f) - 1.45f) < 1.0f) {
                    segment.coinCollected = true;
                    score += 25;
                    PlaySound(coinSound);
                }
            }
        } else if (IsKeyPressed(KEY_R)) {
            reset();
        }

        BeginDrawing();
        ClearBackground(Color{7, 10, 20, 255});

        BeginMode3D(camera);
        for (const auto& segment : segments) {
            DrawSegment(segment, roadTexture, buildingTexture,
                        obstacleTexture, neonTexture);
        }
        DrawAnimatedPlayer({LaneX(lane), playerY, PLAYER_Z}, runTime,
                           playerY > 1.05f);
        EndMode3D();

        DrawRectangle(0, 0, SCREEN_WIDTH, 78, Color{5, 8, 18, 225});
        DrawText("NEON RUNNER", 28, 18, 28, Color{100, 225, 255, 255});
        DrawText(TextFormat("SCORE %05d", score), 275, 20, 26, RAYWHITE);
        DrawText(TextFormat("SPEED %.1f", speed), 480, 22, 20,
                 Color{175, 185, 210, 255});
        DrawText("A/D MOVE   SPACE JUMP", 900, 24, 17,
                 Color{175, 185, 210, 255});

        if (gameOver) {
            DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT,
                          Color{5, 7, 15, 175});
            DrawText("SYSTEM FAILURE", 450, 255, 54,
                     Color{255, 90, 110, 255});
            DrawText(TextFormat("SCORE %d", score), 565, 330, 25, RAYWHITE);
            DrawText("PRESS R TO RESTART", 505, 390, 22,
                     Color{150, 220, 255, 255});
        }

        EndDrawing();
    }

    UnloadTexture(roadTexture);
    UnloadTexture(buildingTexture);
    UnloadTexture(obstacleTexture);
    UnloadTexture(neonTexture);
    UnloadSound(coinSound);
    UnloadSound(jumpSound);
    UnloadSound(hitSound);
    UnloadSound(stepSound);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}
