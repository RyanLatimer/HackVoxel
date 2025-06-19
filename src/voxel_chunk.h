#pragma once
#include <glm/glm.hpp>
#include <vector>

class VoxelChunk {
public:
    static const int CHUNK_SIZE = 16;

    VoxelChunk();
    void render(unsigned int shaderID);

private:
    int blocks[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
};
