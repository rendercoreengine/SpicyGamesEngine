cmake_minimum_required(VERSION 3.16)
project(SpicyGamesEngine)

set(CMAKE_CXX_STANDARD 17)

# Include your engine sources
file(GLOB_RECURSE ENGINE_SRC CONFIGURE_DEPENDS
    "engine/src/*.cpp"
    "engine/src/*.h"
)

# Only include the necessary ImGui files
set(IMGUI_SRC
    libs/imgui/imgui.cpp
    libs/imgui/imgui_draw.cpp
    libs/imgui/imgui_tables.cpp
    libs/imgui/imgui_widgets.cpp
    libs/imgui/imgui_demo.cpp
    # Optional: Uncomment only if you're using SDL2 + OpenGL3
    # libs/imgui/imgui_impl_sdl.cpp
    # libs/imgui/imgui_impl_opengl3.cpp
)

add_executable(SpicyGamesEngine
    ${ENGINE_SRC}
    ${IMGUI_SRC}
)

target_include_directories(SpicyGamesEngine PRIVATE
    engine/src
    libs/imgui
    libs/glad/include
    libs/SDL2/include
)

# Optional: link SDL2 & OpenGL if used
target_link_libraries(SpicyGamesEngine PRIVATE
    opengl32
    SDL2::SDL2
)
