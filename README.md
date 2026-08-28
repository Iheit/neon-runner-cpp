# Neon Runner

A small C++17 3D endless runner built with raylib.

## Controls

- A/D or Left/Right: move lanes
- Space: jump
- R: restart after crashing
- Esc: quit

The project generates its runtime texture assets procedurally and stores them under `assets/generated/`. The first launch uses an in-game loading screen while assets are generated and loaded.

## Build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```
