#include "voxel_chunk.h"
#include <glad/gl.h>
#include <vector>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>

// Simple noise function for terrain generation
float simpleNoise(float x, float z) {
    return sin(x * 0.1f) * cos(z * 0.1f) * 0.5f + 
           sin(x * 0.05f) * cos(z * 0.05f) * 0.3f +
           sin(x * 0.02f) * cos(z * 0.02f) * 0.2f;
}

VoxelChunk::VoxelChunk(int worldX, int worldZ) : worldX(worldX), worldZ(worldZ)
{
    // Initialize all blocks to air
    for (int x = 0; x < CHUNK_SIZE; x++) {
        for (int y = 0; y < CHUNK_SIZE; y++) {
            for (int z = 0; z < CHUNK_SIZE; z++) {
                blocks[x][y][z] = BlockType::AIR;
            }
        }
    }

    // Generate terrain with different block types
    for (int x = 0; x < CHUNK_SIZE; x++)
    {
        for (int z = 0; z < CHUNK_SIZE; z++)
        {
            float globalX = worldX * CHUNK_SIZE + x;
            float globalZ = worldZ * CHUNK_SIZE + z;

            float heightValue = simpleNoise(globalX, globalZ);
            int height = (int)((heightValue + 1.0f) * 0.5f * CHUNK_SIZE);

            for (int y = 0; y < CHUNK_SIZE; y++)
            {
                if (y <= height) {
                    // Generate different layers
                    if (y == 0) {
                        blocks[x][y][z] = BlockType::BEDROCK;
                    } else if (y == height && height > 3) {
                        blocks[x][y][z] = BlockType::GRASS;
                    } else if (y > height - 3) {
                        blocks[x][y][z] = BlockType::DIRT;
                    } else {
                        blocks[x][y][z] = BlockType::STONE;
                    }
                } else {
                    blocks[x][y][z] = BlockType::AIR;
                }
            }
        }
    }

    generateMesh();
    std::cout << "Chunk (" << worldX << "," << worldZ << ") created with " 
              << vertices.size()/5 << " vertices, " << indices.size()/3 << " triangles" << std::endl;
}

void VoxelChunk::render(unsigned int shaderID)
{
    if (VAO == 0 || indices.empty()) {
        std::cerr << "Warning: Chunk (" << worldX << "," << worldZ << ") has no mesh to render" << std::endl;
        return;
    }

    glm::vec3 chunkOffset = glm::vec3(worldX * CHUNK_SIZE, 0, worldZ * CHUNK_SIZE);
    glm::mat4 model = glm::translate(glm::mat4(1.0f), chunkOffset);
    glUniformMatrix4fv(glGetUniformLocation(shaderID, "model"), 1, GL_FALSE, glm::value_ptr(model));
    
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

bool VoxelChunk::isAir(int x, int y, int z) const
{
    if (x < 0 || x >= CHUNK_SIZE ||
        y < 0 || y >= CHUNK_SIZE ||
        z < 0 || z >= CHUNK_SIZE)
        return true; // Out of bounds is considered air

    return blocks[x][y][z] == BlockType::AIR;
}

// Public method for collision detection
bool VoxelChunk::isBlockSolid(int x, int y, int z) const
{
    if (x < 0 || x >= CHUNK_SIZE ||
        y < 0 || y >= CHUNK_SIZE ||
        z < 0 || z >= CHUNK_SIZE)
        return false; // Out of bounds is not solid

    return blocks[x][y][z] != BlockType::AIR;
}

BlockType VoxelChunk::getBlockType(int x, int y, int z) const
{
    if (x < 0 || x >= CHUNK_SIZE ||
        y < 0 || y >= CHUNK_SIZE ||
        z < 0 || z >= CHUNK_SIZE)
        return BlockType::AIR;

    return blocks[x][y][z];
}

void VoxelChunk::generateMesh()
{
    vertices.clear();
    indices.clear();

    unsigned int indexOffset = 0;

    for (int x = 0; x < CHUNK_SIZE; x++)
    {
        for (int y = 0; y < CHUNK_SIZE; y++)
        {
            for (int z = 0; z < CHUNK_SIZE; z++)
            {
                BlockType blockType = blocks[x][y][z];
                if (blockType == BlockType::AIR)
                    continue;

                // Front face (positive Z) - face direction 0
                if (isAir(x, y, z + 1))
                {
                    addFace(x, y, z + 1, x + 1, y, z + 1, x + 1, y + 1, z + 1, x, y + 1, z + 1, indexOffset, blockType, 0);
                }

                // Back face (negative Z) - face direction 1
                if (isAir(x, y, z - 1))
                {
                    addFace(x + 1, y, z, x, y, z, x, y + 1, z, x + 1, y + 1, z, indexOffset, blockType, 1);
                }

                // Right face (positive X) - face direction 2
                if (isAir(x + 1, y, z))
                {
                    addFace(x + 1, y, z, x + 1, y, z + 1, x + 1, y + 1, z + 1, x + 1, y + 1, z, indexOffset, blockType, 2);
                }

                // Left face (negative X) - face direction 3
                if (isAir(x - 1, y, z))
                {
                    addFace(x, y, z + 1, x, y, z, x, y + 1, z, x, y + 1, z + 1, indexOffset, blockType, 3);
                }

                // Top face (positive Y) - face direction 4
                if (isAir(x, y + 1, z))
                {
                    addFace(x, y + 1, z, x + 1, y + 1, z, x + 1, y + 1, z + 1, x, y + 1, z + 1, indexOffset, blockType, 4);
                }

                // Bottom face (negative Y) - face direction 5
                if (isAir(x, y - 1, z))
                {
                    addFace(x, y, z + 1, x + 1, y, z + 1, x + 1, y, z, x, y, z, indexOffset, blockType, 5);
                }
            }
        }
    }

    // Setup GPU buffers after mesh generation
    if (!VAO) {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);
    }

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Position attribute (3 floats)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Texture coordinate attribute (2 floats)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void VoxelChunk::addFace(float x1, float y1, float z1, float x2, float y2, float z2,
                         float x3, float y3, float z3, float x4, float y4, float z4,
                         unsigned int& indexOffset, BlockType blockType, int faceDirection)
{
    // Get texture coordinates for this block type and face
    TextureAtlas::TextureUV uv = getTextureForBlock(blockType, faceDirection);
    
    // Add vertex positions and texture coordinates
    // Each vertex has 5 components: x, y, z, u, v
    float face[] = {
        x1, y1, z1, uv.u1, uv.v1,  // bottom-left
        x2, y2, z2, uv.u2, uv.v1,  // bottom-right
        x3, y3, z3, uv.u2, uv.v2,  // top-right
        x4, y4, z4, uv.u1, uv.v2   // top-left
    };

    // Add face indices (two triangles)
    unsigned int inds[] = {
        0, 1, 2, 2, 3, 0
    };

    // Append vertices
    for (float f : face)
    {
        vertices.push_back(f);
    }

    // Append indices with offset
    for (unsigned int i : inds)
    {
        indices.push_back(i + indexOffset);
    }

    indexOffset += 4;
}

TextureAtlas::TextureUV VoxelChunk::getTextureForBlock(BlockType blockType, int faceDirection) const
{
    TextureAtlas atlas; // This is temporary - we'll fix this later
    
    switch (blockType) {
        case BlockType::GRASS:
            if (faceDirection == 4) { // Top face
                return atlas.getUV(TextureAtlas::BlockType::GRASS_TOP);
            } else if (faceDirection == 5) { // Bottom face
                return atlas.getUV(TextureAtlas::BlockType::DIRT);
            } else { // Side faces
                return atlas.getUV(TextureAtlas::BlockType::GRASS_SIDE);
            }
        case BlockType::DIRT:
            return atlas.getUV(TextureAtlas::BlockType::DIRT);
        case BlockType::STONE:
            return atlas.getUV(TextureAtlas::BlockType::STONE);
        case BlockType::COBBLESTONE:
            return atlas.getUV(TextureAtlas::BlockType::COBBLESTONE);
        case BlockType::WOOD_PLANK:
            return atlas.getUV(TextureAtlas::BlockType::WOOD_PLANK);
        case BlockType::WOOD_LOG:
            if (faceDirection == 4 || faceDirection == 5) { // Top/bottom faces
                return atlas.getUV(TextureAtlas::BlockType::WOOD_LOG_TOP);
            } else { // Side faces
                return atlas.getUV(TextureAtlas::BlockType::WOOD_LOG_SIDE);
            }
        case BlockType::LEAVES:
            return atlas.getUV(TextureAtlas::BlockType::LEAVES);
        case BlockType::SAND:
            return atlas.getUV(TextureAtlas::BlockType::SAND);
        case BlockType::WATER:
            return atlas.getUV(TextureAtlas::BlockType::WATER);
        case BlockType::BEDROCK:
            return atlas.getUV(TextureAtlas::BlockType::BEDROCK);
        default:
            return atlas.getUV(TextureAtlas::BlockType::STONE);
    }
}
