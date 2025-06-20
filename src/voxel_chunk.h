#pragma once
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <vector>

class VoxelChunk
{
public:
    static const int CHUNK_SIZE = 16;

    // Constructor: optionally specify world position (defaults to 0,0)
    VoxelChunk(int worldX = 0, int worldZ = 0);
    void render(unsigned int shaderID);

private:
    void generateMesh();
    void addFace(float x1, float y1, float z1, float x2, float y2, float z2,
                 float x3, float y3, float z3, float x4, float y4, float z4,
                 unsigned int& indexOffset);
    bool isAir(int x, int y, int z) const;

private:
    int blocks[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
    int worldX, worldZ;
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    GLuint VAO = 0, VBO = 0, EBO = 0;
};
