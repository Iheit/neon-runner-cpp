cmake_minimum_required(VERSION 3.20)

project(NeonRunner LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 5.5
)

FetchContent_MakeAvailable(raylib)

add_executable(
    neon_runner
    src/main.cpp
    src/asset_generator.cpp
)

target_include_directories(neon_runner PRIVATE src)
target_link_libraries(neon_runner PRIVATE raylib)

if(MSVC)
    target_compile_options(neon_runner PRIVATE /W4)
else()
    target_compile_options(neon_runner PRIVATE -Wall -Wextra -Wpedantic)
endif()
