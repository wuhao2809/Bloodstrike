cmake_minimum_required(VERSION 3.10)
project(Bloodstrike)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)
pkg_check_modules(SDL2_IMAGE REQUIRED SDL2_image)
pkg_check_modules(SDL2_TTF REQUIRED SDL2_ttf)
pkg_check_modules(SDL2_MIXER REQUIRED SDL2_mixer)
pkg_check_modules(SDL2_NET REQUIRED SDL2_net)

# Find nlohmann/json
find_package(nlohmann_json REQUIRED)

# Include directories
include_directories(${SDL2_INCLUDE_DIRS})
include_directories(${SDL2_IMAGE_INCLUDE_DIRS})
include_directories(${SDL2_TTF_INCLUDE_DIRS})
include_directories(${SDL2_MIXER_INCLUDE_DIRS})
include_directories(${SDL2_NET_INCLUDE_DIRS})
include_directories(src)
include_directories(src/core)
include_directories(src/components)
include_directories(src/systems)
include_directories(src/managers)

# Source files
file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.h")

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link libraries
target_link_libraries(${PROJECT_NAME} 
    "/opt/homebrew/lib/libSDL2.dylib"
    "/opt/homebrew/lib/libSDL2_image.dylib"
    "/opt/homebrew/lib/libSDL2_ttf.dylib"
    "/opt/homebrew/lib/libSDL2_mixer.dylib"
    "/opt/homebrew/lib/libSDL2_net.dylib"
    nlohmann_json::nlohmann_json
)

# Compiler flags
target_compile_options(${PROJECT_NAME} PRIVATE ${SDL2_CFLAGS_OTHER})

# Define asset path for the game to find resources
target_compile_definitions(${PROJECT_NAME} PRIVATE 
    ASSET_PATH="${CMAKE_SOURCE_DIR}/"
)

# Copy entities.json to build directory
configure_file(${CMAKE_SOURCE_DIR}/entities.json ${CMAKE_BINARY_DIR}/entities.json COPYONLY)
