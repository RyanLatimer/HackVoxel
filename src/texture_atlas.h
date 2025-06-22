#pragma once

#include <glad/gl.h>
#include <string>
#include <vector>
#include <unordered_map>

/**
 * TextureAtlas manages a collection of block textures in a single OpenGL texture.
 * This is more efficient than binding different textures for each block type.
 */
class TextureAtlas {
public:    // Block texture types
    enum class BlockType {
        GRASS_TOP = 0,
        GRASS_SIDE,
        DIRT,
        STONE,
        COBBLESTONE,
        WOOD_PLANK,
        WOOD_LOG_TOP,
        WOOD_LOG_SIDE,
        LEAVES,
        SAND,
        WATER,
        BEDROCK,
        SNOW,
        ICE,
        GLOWSTONE,
        OBSIDIAN,
        BRICK,
        MOSSY_STONE,
        GRAVEL,
        GOLD_ORE,
        IRON_ORE,
        DIAMOND_ORE,
        EMERALD_ORE,
        REDSTONE_ORE,
        COUNT  // Keep this last for counting
    };

    // UV coordinates for a texture in the atlas
    struct TextureUV {
        float u1, v1;  // Top-left corner
        float u2, v2;  // Bottom-right corner
    };

    // Constructor
    TextureAtlas();
    ~TextureAtlas();

    // Initialize the texture atlas with procedural textures
    bool initialize();

    // Get UV coordinates for a specific block type
    TextureUV getUV(BlockType blockType) const;

    // Bind the texture atlas for rendering
    void bind(int textureUnit = 0) const;

    // Get the OpenGL texture ID
    GLuint getTextureID() const { return textureID; }

private:
    GLuint textureID;
    int atlasSize;          // Size of the atlas (e.g., 512x512)
    int textureSize;        // Size of each individual texture (e.g., 16x16)
    int texturesPerRow;     // Number of textures per row in atlas

    // Generate procedural textures for different block types
    void generateGrassTopTexture(unsigned char* data, int startX, int startY);
    void generateGrassSideTexture(unsigned char* data, int startX, int startY);
    void generateDirtTexture(unsigned char* data, int startX, int startY);
    void generateStoneTexture(unsigned char* data, int startX, int startY);
    void generateCobblestoneTexture(unsigned char* data, int startX, int startY);
    void generateWoodPlankTexture(unsigned char* data, int startX, int startY);
    void generateWoodLogTopTexture(unsigned char* data, int startX, int startY);
    void generateWoodLogSideTexture(unsigned char* data, int startX, int startY);
    void generateLeavesTexture(unsigned char* data, int startX, int startY);
    void generateSandTexture(unsigned char* data, int startX, int startY);
    void generateWaterTexture(unsigned char* data, int startX, int startY);
    void generateBedrockTexture(unsigned char* data, int startX, int startY);
    
    // New beautiful texture generation methods
    void generateSnowTexture(unsigned char* data, int startX, int startY);
    void generateIceTexture(unsigned char* data, int startX, int startY);
    void generateGlowstoneTexture(unsigned char* data, int startX, int startY);
    void generateObsidianTexture(unsigned char* data, int startX, int startY);
    void generateBrickTexture(unsigned char* data, int startX, int startY);
    void generateMossyStoneTexture(unsigned char* data, int startX, int startY);
    void generateGravelTexture(unsigned char* data, int startX, int startY);
    void generateGoldOreTexture(unsigned char* data, int startX, int startY);
    void generateIronOreTexture(unsigned char* data, int startX, int startY);
    void generateDiamondOreTexture(unsigned char* data, int startX, int startY);
    void generateEmeraldOreTexture(unsigned char* data, int startX, int startY);
    void generateRedstoneOreTexture(unsigned char* data, int startX, int startY);

    // Helper functions for procedural generation
    float noise(int x, int y, int seed = 12345) const;
    unsigned char clamp(float value) const;
};
