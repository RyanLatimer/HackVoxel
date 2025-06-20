#pragma once
#include <glm/glm.hpp>
#include <vector>

class VoxelChunk {
public:
    static const int CHUNK_SIZE = 16;

    // Constructor: optionally specify world position (defaults to 0,0)
    VoxelChunk(int worldX = 0, int worldZ = 0);
    void render(unsigned int shaderID);

private:
    int blocks[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
    int worldX, worldZ;
};
