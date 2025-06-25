cmake_minimum_required(VERSION 3.10)
project(HackVoxel)

set(CMAKE_CXX_STANDARD 17)

# Include directories
include_directories(include)
include_directories(libs/glad/include)
include_directories(libs/glfw/include)
include_directories(libs/glm)
include_directories(libs/FastNoiseLite)


# Link GLFW and GLAD
add_library(glad STATIC libs/glad/src/gl.c)
add_executable(HackVoxel src/main.cpp src/shader.cpp src/camera.cpp src/voxel_chunk.cpp src/player.cpp src/texture_atlas.cpp src/chunk_manager.cpp src/skybox.cpp src/water_shader.cpp src/ui.cpp src/block_interaction.cpp)

# Platform-specific GLFW link
if (WIN32)
    target_link_libraries(HackVoxel glad glfw3 opengl32)
    target_link_directories(HackVoxel PRIVATE libs/glfw/lib-mingw-w64)
else()
    target_link_libraries(HackVoxel glad glfw GL)
endif()
