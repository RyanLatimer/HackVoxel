#include "voxel_chunk.h"
#include <glad/gl.h>
#include <vector>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>

// Static member definition
TextureAtlas* VoxelChunk::textureAtlas = nullptr;

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

    // Note: Terrain generation is now handled by ChunkManager
    // Mesh generation will be called after terrain is set
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
                    continue;                // Front face (positive Z) - face direction 0
                if (isTransparent(x, y, z + 1))
                {
                    addFace(x, y, z + 1, x + 1, y, z + 1, x + 1, y + 1, z + 1, x, y + 1, z + 1, indexOffset, blockType, 0);
                }

                // Back face (negative Z) - face direction 1
                if (isTransparent(x, y, z - 1))
                {
                    addFace(x + 1, y, z, x, y, z, x, y + 1, z, x + 1, y + 1, z, indexOffset, blockType, 1);
                }

                // Right face (positive X) - face direction 2
                if (isTransparent(x + 1, y, z))
                {
                    addFace(x + 1, y, z, x + 1, y, z + 1, x + 1, y + 1, z + 1, x + 1, y + 1, z, indexOffset, blockType, 2);
                }

                // Left face (negative X) - face direction 3
                if (isTransparent(x - 1, y, z))
                {
                    addFace(x, y, z + 1, x, y, z, x, y + 1, z, x, y + 1, z + 1, indexOffset, blockType, 3);
                }

                // Top face (positive Y) - face direction 4
                if (isTransparent(x, y + 1, z))
                {
                    addFace(x, y + 1, z, x + 1, y + 1, z, x + 1, y + 1, z + 1, x, y + 1, z + 1, indexOffset, blockType, 4);
                }

                // Bottom face (negative Y) - face direction 5
                if (isTransparent(x, y - 1, z))
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

        // Position attribute (location 0) - 3 floats
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Normal attribute (location 1) - 3 floats
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Texture coordinate attribute (location 2) - 2 floats
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

void VoxelChunk::addFace(float x1, float y1, float z1, float x2, float y2, float z2,
                         float x3, float y3, float z3, float x4, float y4, float z4,
                         unsigned int& indexOffset, BlockType blockType, int faceDirection)
{
    // Get texture coordinates for this block type and face
    TextureAtlas::TextureUV uv = getTextureForBlock(blockType, faceDirection);
    
    // Add vertex positions and texture coordinates
    // Each vertex has 8 components: x, y, z, nx, ny, nz, u, v
        // Compute normal based on face direction (axis aligned)
    glm::vec3 normal;
    switch(faceDirection) {
        case 0: normal = glm::vec3(0.0f, 0.0f, 1.0f); break; // front +Z
        case 1: normal = glm::vec3(0.0f, 0.0f, -1.0f); break; // back -Z
        case 2: normal = glm::vec3(1.0f, 0.0f, 0.0f); break; // right +X
        case 3: normal = glm::vec3(-1.0f, 0.0f, 0.0f); break; // left -X
        case 4: normal = glm::vec3(0.0f, 1.0f, 0.0f); break; // top +Y
        case 5: normal = glm::vec3(0.0f, -1.0f, 0.0f); break; // bottom -Y
        default: normal = glm::vec3(0.0f); break;
    }

    float face[] = {
        // pos                  // normal               // uv
        x1, y1, z1, normal.x, normal.y, normal.z, uv.u1, uv.v1,
        x2, y2, z2, normal.x, normal.y, normal.z, uv.u2, uv.v1,
        x3, y3, z3, normal.x, normal.y, normal.z, uv.u2, uv.v2,
        x4, y4, z4, normal.x, normal.y, normal.z, uv.u1, uv.v2
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
    if (!textureAtlas) {
        // Return default UV if no texture atlas is set
        TextureAtlas::TextureUV defaultUV = {0.0f, 0.0f, 1.0f, 1.0f};
        return defaultUV;
    }
    
    switch (blockType) {
        case BlockType::GRASS:
            if (faceDirection == 4) { // Top face
                return textureAtlas->getUV(TextureAtlas::BlockType::GRASS_TOP);
            } else if (faceDirection == 5) { // Bottom face
                return textureAtlas->getUV(TextureAtlas::BlockType::DIRT);
            } else { // Side faces
                return textureAtlas->getUV(TextureAtlas::BlockType::GRASS_SIDE);
            }
        case BlockType::DIRT:
            return textureAtlas->getUV(TextureAtlas::BlockType::DIRT);
        case BlockType::STONE:
            return textureAtlas->getUV(TextureAtlas::BlockType::STONE);
        case BlockType::COBBLESTONE:
            return textureAtlas->getUV(TextureAtlas::BlockType::COBBLESTONE);
        case BlockType::WOOD_PLANK:
            return textureAtlas->getUV(TextureAtlas::BlockType::WOOD_PLANK);
        case BlockType::WOOD_LOG:
            if (faceDirection == 4 || faceDirection == 5) { // Top/bottom faces
                return textureAtlas->getUV(TextureAtlas::BlockType::WOOD_LOG_TOP);
            } else { // Side faces
                return textureAtlas->getUV(TextureAtlas::BlockType::WOOD_LOG_SIDE);
            }        case BlockType::LEAVES:
            return textureAtlas->getUV(TextureAtlas::BlockType::LEAVES);
        case BlockType::SAND:
            return textureAtlas->getUV(TextureAtlas::BlockType::SAND);
        case BlockType::WATER:
            return textureAtlas->getUV(TextureAtlas::BlockType::WATER);
        case BlockType::BEDROCK:
            return textureAtlas->getUV(TextureAtlas::BlockType::BEDROCK);
        case BlockType::SNOW:
            return textureAtlas->getUV(TextureAtlas::BlockType::SNOW);
        case BlockType::ICE:
            return textureAtlas->getUV(TextureAtlas::BlockType::ICE);
        case BlockType::GLOWSTONE:
            return textureAtlas->getUV(TextureAtlas::BlockType::GLOWSTONE);
        case BlockType::OBSIDIAN:
            return textureAtlas->getUV(TextureAtlas::BlockType::OBSIDIAN);
        case BlockType::BRICK:
            return textureAtlas->getUV(TextureAtlas::BlockType::BRICK);
        case BlockType::MOSSY_STONE:
            return textureAtlas->getUV(TextureAtlas::BlockType::MOSSY_STONE);
        case BlockType::GRAVEL:
            return textureAtlas->getUV(TextureAtlas::BlockType::GRAVEL);
        case BlockType::GOLD_ORE:
            return textureAtlas->getUV(TextureAtlas::BlockType::GOLD_ORE);
        case BlockType::IRON_ORE:
            return textureAtlas->getUV(TextureAtlas::BlockType::IRON_ORE);
        case BlockType::DIAMOND_ORE:
            return textureAtlas->getUV(TextureAtlas::BlockType::DIAMOND_ORE);
        case BlockType::EMERALD_ORE:
            return textureAtlas->getUV(TextureAtlas::BlockType::EMERALD_ORE);
        case BlockType::REDSTONE_ORE:
            return textureAtlas->getUV(TextureAtlas::BlockType::REDSTONE_ORE);
        default:
            return textureAtlas->getUV(TextureAtlas::BlockType::STONE);
    }
}

void VoxelChunk::setBlock(int x, int y, int z, BlockType blockType) {
    if (x >= 0 && x < CHUNK_SIZE && y >= 0 && y < CHUNK_SIZE && z >= 0 && z < CHUNK_SIZE) {
        blocks[x][y][z] = blockType;
    }
}

void VoxelChunk::regenerateMesh() {
    // Clear existing mesh data
    vertices.clear();
    indices.clear();
    
    // Regenerate the mesh
    generateMesh();
}

bool VoxelChunk::isTransparent(int x, int y, int z) const
{
    if (x < 0 || x >= CHUNK_SIZE ||
        y < 0 || y >= CHUNK_SIZE ||
        z < 0 || z >= CHUNK_SIZE)
        return true; // Out of bounds is considered transparent

    BlockType blockType = blocks[x][y][z];
    return blockType == BlockType::AIR || blockType == BlockType::WATER;
}
