#include "asset_generator.h"
#include <raylib.h>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <string>
#include <vector>

struct Obstacle { Vector3 p; float w=1.8f, h=2.0f, d=2.0f; };
struct Coin { Vector3 p; bool got=false; float t=0.0f; };

static float laneX(int lane) { return (lane - 1) * 3.2f; }
static float rnd(float a, float b) { return a + float(std::rand()) / float(RAND_MAX) * (b - a); }

int main() {
    std::srand(unsigned(std::time(nullptr)));
    InitWindow(1280, 720, "Neon Runner");
    SetTargetFPS(60);

    int loadPct = 0;
    std::string loadText = "Starting...";
    const std::string root = std::filesystem::current_path().string();
    GenerateAssets(root, [&](int p, const std::string& s) {
        loadPct = p;
        loadText = s;
        BeginDrawing();
        ClearBackground(Color{7,10,20,255});
        DrawText("NEON RUNNER", 440, 220, 48, Color{100,225,255,255});
        DrawText(loadText.c_str(), 460, 315, 22, RAYWHITE);
        DrawRectangle(360, 365, 560, 20, Color{30,35,55,255});
        DrawRectangle(360, 365, 560 * loadPct / 100, 20, Color{70,220,255,255});
        DrawText(TextFormat("%d%%", loadPct), 610, 405, 20, RAYWHITE);
        EndDrawing();
    });

    Camera3D cam{};
    cam.position = {0.0f, 7.0f, 10.5f};
    cam.target = {0.0f, 1.1f, -8.0f};
    cam.up = {0.0f, 1.0f, 0.0f};
    cam.fovy = 55.0f;
    cam.projection = CAMERA_PERSPECTIVE;

    int lane = 1, score = 0;
    float y = 1.0f, vy = 0.0f, speed = 18.0f, dist = 0.0f;
    bool dead = false;
    std::vector<Obstacle> obs;
    std::vector<Coin> coins;

    auto reset = [&]() {
        lane = 1; score = 0; y = 1.0f; vy = 0.0f; speed = 18.0f; dist = 0.0f; dead = false;
        obs.clear(); coins.clear();
        for (int i = 0; i < 10; ++i) {
            const float z = -18.0f - i * 15.0f;
            if (i % 2 == 0) obs.push_back({{laneX((i + 1) % 3), 1.0f, z}});
            coins.push_back({{laneX((i + 2) % 3), 1.3f, z - 5.0f}, false, rnd(0.0f, 6.28f)});
        }
    };
    reset();

    while (!WindowShouldClose()) {
        const float dt = std::min(GetFrameTime(), 0.033f);

        if (!dead) {
            if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT)) lane = std::max(0, lane - 1);
            if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT)) lane = std::min(2, lane + 1);
            if (IsKeyPressed(KEY_SPACE) && y <= 1.01f) vy = 10.0f;

            vy -= 25.0f * dt;
            y += vy * dt;
            if (y < 1.0f) { y = 1.0f; vy = 0.0f; }
            speed = std::min(34.0f, speed + 0.55f * dt);
            dist += speed * dt;
            score = int(dist * 1.25f);

            for (auto& o : obs) o.p.z += speed * dt;
            for (auto& c : coins) { c.p.z += speed * dt; c.t += dt * 5.0f; }

            float far = -100.0f;
            for (const auto& o : obs) far = std::min(far, o.p.z);
            for (const auto& c : coins) far = std::min(far, c.p.z);
            if (far > -100.0f) {
                const float z = far - rnd(24.0f, 34.0f);
                const int ol = std::rand() % 3;
                obs.push_back({{laneX(ol), 1.0f, z}, 1.8f, rnd(1.7f, 2.5f), 2.0f});
                const int cl = (ol + 1 + std::rand() % 2) % 3;
                coins.push_back({{laneX(cl), rnd(1.2f, 1.8f), z - 5.0f}, false, rnd(0.0f, 6.28f)});
            }

            const Vector3 pp{laneX(lane), y, 2.5f};
            const BoundingBox pb{{pp.x-.62f, pp.y-.68f, pp.z-.62f}, {pp.x+.62f, pp.y+.68f, pp.z+.62f}};
            for (const auto& o : obs) {
                const BoundingBox b{{o.p.x-o.w/2, o.p.y-o.h/2, o.p.z-o.d/2}, {o.p.x+o.w/2, o.p.y+o.h/2, o.p.z+o.d/2}};
                if (CheckCollisionBoxes(pb, b)) { dead = true; break; }
            }
            for (auto& c : coins) {
                if (!c.got && std::fabs(c.p.x-pp.x) < .9f && std::fabs(c.p.z-pp.z) < .9f && std::fabs(c.p.y + std::sin(c.t*3.0f)*.15f - pp.y) < 1.0f) {
                    c.got = true; score += 25;
                }
            }
            obs.erase(std::remove_if(obs.begin(), obs.end(), [](const auto& o) { return o.p.z > 18.0f; }), obs.end());
            coins.erase(std::remove_if(coins.begin(), coins.end(), [](const auto& c) { return c.p.z > 18.0f || c.got; }), coins.end());
        } else if (IsKeyPressed(KEY_R)) {
            reset();
        }

        BeginDrawing();
        ClearBackground(Color{7,10,20,255});
        BeginMode3D(cam);
        DrawPlane({0,-.08f,0}, {24,180}, Color{20,24,38,255});
        DrawCube({-5.25f,.02f,-25}, .08f, .12f, 180.0f, Color{70,220,255,255});
        DrawCube({5.25f,.02f,-25}, .08f, .12f, 180.0f, Color{70,220,255,255});
        const float off = std::fmod(dist * .9f, 8.0f);
        for (int d = 0; d < 2; ++d) {
            const float x = d ? -1.6f : 1.6f;
            for (int i = -12; i < 14; ++i) DrawCube({x,.055f,i*8.0f+off}, .09f, .04f, 3.5f, Color{115,125,150,255});
        }
        for (int i = -9; i <= 9; i += 2) {
            const float h = 4.0f + std::abs(i % 5) * 1.8f;
            DrawCube({i*3.6f,h/2.0f,-45}, 2.6f, h, 2.6f, Color{24,28,48,255});
        }
        for (const auto& o : obs) {
            DrawCube(o.p, o.w, o.h, o.d, Color{255,70,95,255});
            DrawCubeWires(o.p, o.w+.05f, o.h+.05f, o.d+.05f, Color{255,150,165,255});
        }
        for (const auto& c : coins) if (!c.got) {
            const float b = std::sin(c.t*3.0f)*.15f;
            DrawCylinder({c.p.x,c.p.y+b,c.p.z}, .48f, .1f, .1f, 32, Color{255,220,70,255});
        }
        DrawCube({laneX(lane),y,2.5f}, 1.45f, 1.45f, 1.45f, Color{70,220,255,255});
        DrawCubeWires({laneX(lane),y,2.5f}, 1.5f, 1.5f, 1.5f, Color{170,245,255,255});
        EndMode3D();

        DrawRectangle(0,0,1280,78, Color{5,8,18,225});
        DrawText("NEON RUNNER", 28,18,28, Color{100,225,255,255});
        DrawText(TextFormat("SCORE %05d",score), 275,20,26,RAYWHITE);
        DrawText(TextFormat("SPEED %.1f",speed),480,22,20,Color{175,185,210,255});
        DrawText("A/D MOVE   SPACE JUMP",900,24,17,Color{175,185,210,255});
        if (dead) {
            DrawRectangle(0,0,1280,720,Color{5,7,15,170});
            DrawText("SYSTEM FAILURE",450,255,54,Color{255,90,110,255});
            DrawText(TextFormat("SCORE %d",score),565,330,25,RAYWHITE);
            DrawText("PRESS R TO RESTART",505,390,22,Color{150,220,255,255});
        }
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
