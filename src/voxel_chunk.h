#pragma once
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <vector>
#include "texture_atlas.h"

// Block type enumeration
enum class BlockType : int {
    AIR = 0,
    GRASS = 1,
    DIRT = 2,
    STONE = 3,
    COBBLESTONE = 4,
    WOOD_PLANK = 5,
    WOOD_LOG = 6,
    LEAVES = 7,
    SAND = 8,
    WATER = 9,
    BEDROCK = 10
};

class VoxelChunk
{
public:
    static const int CHUNK_SIZE = 16;
    static TextureAtlas* textureAtlas; // Static reference to shared texture atlas

    // Constructor: optionally specify world position (defaults to 0,0)
    VoxelChunk(int worldX = 0, int worldZ = 0);
    void render(unsigned int shaderID);

    // Public methods for collision detection
    bool isBlockSolid(int x, int y, int z) const;
    int getWorldX() const { return worldX; }
    int getWorldZ() const { return worldZ; }
    BlockType getBlockType(int x, int y, int z) const;
    
    // Methods for chunk management
    void setBlock(int x, int y, int z, BlockType blockType);
    void regenerateMesh();

private:
    void generateMesh();
    void addFace(float x1, float y1, float z1, float x2, float y2, float z2,
                 float x3, float y3, float z3, float x4, float y4, float z4,
                 unsigned int& indexOffset, BlockType blockType, int faceDirection);
    bool isAir(int x, int y, int z) const;
    TextureAtlas::TextureUV getTextureForBlock(BlockType blockType, int faceDirection) const;

private:
    BlockType blocks[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
    int worldX, worldZ;
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    GLuint VAO = 0, VBO = 0, EBO = 0;
};
