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
constexpr int W = 1280, H = 720;
constexpr float LANE = 3.2f, PLAYER_Z = 2.5f, SEG = 14.0f;
constexpr int SEGMENTS = 20;

struct Segment {
    float z = 0;
    int obstacleLane = 0;
    int coinLane = 1;
    int style = 0;
    float obstacleHeight = 2;
    bool coinCollected = false;
};

float LaneX(int lane) { return (lane - 1) * LANE; }
float RandomFloat(float a, float b) {
    return a + static_cast<float>(std::rand()) / RAND_MAX * (b - a);
}

void GenerateSegment(Segment& s, float z) {
    s.z = z;
    s.obstacleLane = std::rand() % 3;
    s.coinLane = (s.obstacleLane + 1 + std::rand() % 2) % 3;
    s.style = std::rand() % 4;
    s.obstacleHeight = RandomFloat(1.7f, 2.6f);
    s.coinCollected = false;
}

Model Cube(float x, float y, float z) { return LoadModelFromMesh(GenMeshCube(x, y, z)); }
Model Cylinder(float radius, float height) { return LoadModelFromMesh(GenMeshCylinder(radius, height, 24)); }
Model Sphere(float radius) { return LoadModelFromMesh(GenMeshSphere(radius, 20, 12)); }

void DrawSegment(const Segment& s, Model road, Model building, Model obstacle,
                 Model neon, Model coin) {
    DrawModel(road, {0, -0.1f, s.z}, 1, Color{45, 50, 68, 255});

    const float lh = 5.0f + s.style * 1.7f;
    const float rh = 7.0f + (3 - s.style) * 1.2f;
    DrawModelEx(building, {-7, lh / 2, s.z}, {0, 1, 0}, 0,
                {1, lh / 5.0f, 0.9f}, Color{38, 48, 75, 255});
    DrawModelEx(building, {7, rh / 2, s.z + 1}, {0, 1, 0}, 0,
                {1, rh / 5.0f, 0.9f}, Color{30, 40, 65, 255});

    for (int i = 0; i < 4; ++i) {
        float y = 1.2f + i * 1.15f;
        DrawCube({-5.2f, y, s.z - 0.25f}, 0.08f, 0.55f, 0.35f,
                  Color{80, 220, 255, 255});
        DrawCube({5.2f, y, s.z + 0.25f}, 0.08f, 0.55f, 0.35f,
                  Color{220, 80, 255, 255});
    }

    DrawModel(neon, {-5.25f, 2.5f, s.z}, 1, Color{60, 230, 255, 255});
    DrawModel(neon, {5.25f, 2.5f, s.z}, 1, Color{255, 80, 220, 255});

    Vector3 op = {LaneX(s.obstacleLane), s.obstacleHeight / 2, s.z};
    DrawModelEx(obstacle, op, {0, 1, 0}, s.z * 17,
                {1, s.obstacleHeight / 2, 1}, Color{235, 55, 80, 255});
    DrawCubeWires(op, 1.9f, s.obstacleHeight + 0.08f, 1.9f,
                  Color{255, 150, 165, 255});

    if (!s.coinCollected) {
        float bob = std::sin(s.z * 0.17f) * 0.18f;
        DrawModelEx(coin, {LaneX(s.coinLane), 1.45f + bob, s.z - 4},
                    {0, 1, 0}, static_cast<float>(GetTime() * 180),
                    {1, 1, 1}, Color{255, 215, 60, 255});
    }
}

void DrawPlayer(Model body, Model head, Model limb, Vector3 p, float t, bool jump) {
    float run = std::sin(t * 12), bob = jump ? 0 : std::fabs(run) * 0.06f;
    float y = p.y + bob;
    DrawModel(body, {p.x, y + 0.7f, p.z}, 1, Color{45, 175, 220, 255});
    DrawModel(head, {p.x, y + 1.55f, p.z}, 1, Color{105, 230, 255, 255});

    float a = run * 0.30f, b = -a;
    DrawModelEx(limb, {p.x - 0.25f + a, y - 0.30f, p.z}, {0, 0, 1}, run * 18,
                {0.75f, 1, 0.75f}, Color{35, 90, 145, 255});
    DrawModelEx(limb, {p.x + 0.25f + b, y - 0.30f, p.z}, {0, 0, 1}, -run * 18,
                {0.75f, 1, 0.75f}, Color{35, 90, 145, 255});
    DrawModelEx(limb, {p.x - 0.58f + b, y + 0.58f, p.z}, {0, 0, 1}, -run * 16,
                {1.15f, 0.30f, 0.55f}, Color{70, 220, 255, 255});
    DrawModelEx(limb, {p.x + 0.58f + a, y + 0.58f, p.z}, {0, 0, 1}, run * 16,
                {1.15f, 0.30f, 0.55f}, Color{70, 220, 255, 255});
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
            DrawText("NEON RUNNER", 440, 220, 48, Color{100, 225, 255, 255});
            DrawText(status.c_str(), 430, 315, 22, RAYWHITE);
            DrawRectangle(360, 365, 560, 20, Color{30, 35, 55, 255});
            DrawRectangle(360, 365, 560 * progress / 100, 20,
                          Color{70, 220, 255, 255});
            DrawText(TextFormat("%d%%", progress), 610, 405, 20, RAYWHITE);
            EndDrawing();
        });

    const std::string root = (std::filesystem::current_path() / "assets" / "generated").string();
    InitAudioDevice();
    Sound coinSound = LoadSound((root + "/coin.wav").c_str());
    Sound jumpSound = LoadSound((root + "/jump.wav").c_str());
    Sound hitSound = LoadSound((root + "/hit.wav").c_str());
    Sound stepSound = LoadSound((root + "/step.wav").c_str());

    Model road = Cube(10, 0.18f, SEG);
    Model building = Cube(3.5f, 5, 11.5f);
    Model obstacle = Cube(1.8f, 2, 1.8f);
    Model neon = Cube(0.10f, 5, 0.10f);
    Model coin = Cylinder(0.48f, 0.12f);
    Model body = Cylinder(0.55f, 1.3f);
    Model head = Sphere(0.48f);
    Model limb = Cube(0.55f, 0.75f, 0.55f);

    Camera3D camera{};
    camera.position = {0, 6.7f, 11};
    camera.target = {0, 1.3f, -12};
    camera.up = {0, 1, 0};
    camera.fovy = 58;
    camera.projection = CAMERA_PERSPECTIVE;

    std::vector<Segment> segments(SEGMENTS);
    for (int i = 0; i < SEGMENTS; ++i) GenerateSegment(segments[i], -i * SEG);

    int lane = 1, score = 0;
    float playerY = 1, velocityY = 0, speed = 18, distance = 0, runTime = 0, stepTimer = 0;
    bool gameOver = false;

    auto reset = [&]() {
        lane = 1; score = 0; playerY = 1; velocityY = 0; speed = 18;
        distance = 0; runTime = 0; stepTimer = 0; gameOver = false;
        for (int i = 0; i < SEGMENTS; ++i) GenerateSegment(segments[i], -i * SEG);
    };

    while (!WindowShouldClose()) {
        float dt = std::min(GetFrameTime(), 0.033f);
        if (!gameOver) {
            if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT)) lane = std::max(0, lane - 1);
            if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT)) lane = std::min(2, lane + 1);
            if (IsKeyPressed(KEY_SPACE) && playerY <= 1.01f) {
                velocityY = 10; PlaySound(jumpSound);
            }
            velocityY -= 25 * dt;
            playerY += velocityY * dt;
            if (playerY < 1) { playerY = 1; velocityY = 0; }
            speed = std::min(42.0f, speed + 0.42f * dt);
            distance += speed * dt;
            runTime += dt;
            stepTimer += dt;
            score = static_cast<int>(distance * 1.25f);
            if (stepTimer > 0.38f && playerY <= 1.05f) {
                PlaySound(stepSound); stepTimer = 0;
            }

            float farthest = 0;
            for (const auto& s : segments) farthest = std::min(farthest, s.z);
            for (auto& s : segments) s.z += speed * dt;
            for (auto& s : segments) if (s.z > 22) {
                s.z = farthest - SEG;
                GenerateSegment(s, s.z);
                farthest = s.z;
            }

            Vector3 pp = {LaneX(lane), playerY, PLAYER_Z};
            BoundingBox pb = {{pp.x - .55f, pp.y - .75f, pp.z - .45f},
                              {pp.x + .55f, pp.y + .80f, pp.z + .45f}};
            for (auto& s : segments) {
                if (s.obstacleLane == lane && std::fabs(s.z - PLAYER_Z) < 1.15f) {
                    BoundingBox ob = {{LaneX(lane) - .9f, 0, s.z - .9f},
                                      {LaneX(lane) + .9f, s.obstacleHeight, s.z + .9f}};
                    if (CheckCollisionBoxes(pb, ob)) { gameOver = true; PlaySound(hitSound); }
                }
                float cz = s.z - 4;
                if (!s.coinCollected && s.coinLane == lane &&
                    std::fabs(cz - PLAYER_Z) < .95f && std::fabs(playerY - 1.45f) < 1) {
                    s.coinCollected = true; score += 25; PlaySound(coinSound);
                }
            }
        } else if (IsKeyPressed(KEY_R)) reset();

        BeginDrawing();
        ClearBackground(Color{7, 10, 20, 255});
        BeginMode3D(camera);
        for (const auto& s : segments) DrawSegment(s, road, building, obstacle, neon, coin);
        DrawPlayer(body, head, limb, {LaneX(lane), playerY, PLAYER_Z}, runTime, playerY > 1.05f);
        EndMode3D();

        DrawRectangle(0, 0, W, 78, Color{5, 8, 18, 225});
        DrawText("NEON RUNNER", 28, 18, 28, Color{100, 225, 255, 255});
        DrawText(TextFormat("SCORE %05d", score), 275, 20, 26, RAYWHITE);
        DrawText(TextFormat("SPEED %.1f", speed), 480, 22, 20, Color{175, 185, 210, 255});
        DrawText("A/D MOVE   SPACE JUMP", 900, 24, 17, Color{175, 185, 210, 255});
        if (gameOver) {
            DrawRectangle(0, 0, W, H, Color{5, 7, 15, 175});
            DrawText("SYSTEM FAILURE", 450, 255, 54, Color{255, 90, 110, 255});
            DrawText(TextFormat("SCORE %d", score), 565, 330, 25, RAYWHITE);
            DrawText("PRESS R TO RESTART", 505, 390, 22, Color{150, 220, 255, 255});
        }
        EndDrawing();
    }

    UnloadModel(road); UnloadModel(building); UnloadModel(obstacle); UnloadModel(neon);
    UnloadModel(coin); UnloadModel(body); UnloadModel(head); UnloadModel(limb);
    UnloadSound(coinSound); UnloadSound(jumpSound); UnloadSound(hitSound); UnloadSound(stepSound);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}
