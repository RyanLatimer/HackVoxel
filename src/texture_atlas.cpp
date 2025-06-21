#include "texture_atlas.h"
#include <iostream>
#include <cmath>
#include <random>

TextureAtlas::TextureAtlas() 
    : textureID(0)
    , atlasSize(512)
    , textureSize(16)
    , texturesPerRow(atlasSize / textureSize)
{
}

TextureAtlas::~TextureAtlas() {
    if (textureID != 0) {
        glDeleteTextures(1, &textureID);
    }
}

bool TextureAtlas::initialize() {
    // Generate OpenGL texture
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Create texture data
    std::vector<unsigned char> atlasData(atlasSize * atlasSize * 3);
    
    // Fill with default background color
    std::fill(atlasData.begin(), atlasData.end(), 255);

    // Generate each block texture
    int blockCount = static_cast<int>(BlockType::COUNT);
    for (int i = 0; i < blockCount; i++) {
        BlockType blockType = static_cast<BlockType>(i);
        
        // Calculate position in atlas
        int atlasX = (i % texturesPerRow) * textureSize;
        int atlasY = (i / texturesPerRow) * textureSize;
        
        // Generate texture based on type
        switch (blockType) {
            case BlockType::GRASS_TOP:
                generateGrassTopTexture(atlasData.data(), atlasX, atlasY);
                break;
            case BlockType::GRASS_SIDE:
                generateGrassSideTexture(atlasData.data(), atlasX, atlasY);
                break;
            case BlockType::DIRT:
                generateDirtTexture(atlasData.data(), atlasX, atlasY);
                break;
            case BlockType::STONE:
                generateStoneTexture(atlasData.data(), atlasX, atlasY);
                break;
            case BlockType::COBBLESTONE:
                generateCobblestoneTexture(atlasData.data(), atlasX, atlasY);
                break;
            case BlockType::WOOD_PLANK:
                generateWoodPlankTexture(atlasData.data(), atlasX, atlasY);
                break;
            case BlockType::WOOD_LOG_TOP:
                generateWoodLogTopTexture(atlasData.data(), atlasX, atlasY);
                break;
            case BlockType::WOOD_LOG_SIDE:
                generateWoodLogSideTexture(atlasData.data(), atlasX, atlasY);
                break;
            case BlockType::LEAVES:
                generateLeavesTexture(atlasData.data(), atlasX, atlasY);
                break;
            case BlockType::SAND:
                generateSandTexture(atlasData.data(), atlasX, atlasY);
                break;
            case BlockType::WATER:
                generateWaterTexture(atlasData.data(), atlasX, atlasY);
                break;
            case BlockType::BEDROCK:
                generateBedrockTexture(atlasData.data(), atlasX, atlasY);
                break;
        }
    }

    // Upload texture to GPU
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, atlasSize, atlasSize, 0, GL_RGB, GL_UNSIGNED_BYTE, atlasData.data());
    glGenerateMipmap(GL_TEXTURE_2D);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glBindTexture(GL_TEXTURE_2D, 0);

    std::cout << "Texture atlas created: " << atlasSize << "x" << atlasSize 
              << " with " << blockCount << " textures" << std::endl;

    return true;
}

TextureAtlas::TextureUV TextureAtlas::getUV(BlockType blockType) const {
    int blockIndex = static_cast<int>(blockType);
    int x = blockIndex % texturesPerRow;
    int y = blockIndex / texturesPerRow;
    
    float texelSize = 1.0f / texturesPerRow;
    float padding = 0.001f; // Small padding to prevent bleeding
    
    TextureUV uv;
    uv.u1 = x * texelSize + padding;
    uv.v1 = y * texelSize + padding;
    uv.u2 = (x + 1) * texelSize - padding;
    uv.v2 = (y + 1) * texelSize - padding;
    
    return uv;
}

void TextureAtlas::bind(int textureUnit) const {
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_2D, textureID);
}

// ============================================================================
// PROCEDURAL TEXTURE GENERATION FUNCTIONS
// ============================================================================

void TextureAtlas::generateGrassTopTexture(unsigned char* data, int startX, int startY) {
    for (int y = 0; y < textureSize; y++) {
        for (int x = 0; x < textureSize; x++) {
            int index = ((startY + y) * atlasSize + (startX + x)) * 3;
            
            // Base green color with noise
            float n = noise(x, y, 1000) * 0.3f + 0.7f;
            data[index] = clamp(50 * n);      // R - low red
            data[index + 1] = clamp(150 * n); // G - green
            data[index + 2] = clamp(50 * n);  // B - low blue
        }
    }
}

void TextureAtlas::generateGrassSideTexture(unsigned char* data, int startX, int startY) {
    for (int y = 0; y < textureSize; y++) {
        for (int x = 0; x < textureSize; x++) {
            int index = ((startY + y) * atlasSize + (startX + x)) * 3;
            
            if (y < textureSize / 4) {
                // Top part - grass green
                float n = noise(x, y, 1001) * 0.3f + 0.7f;
                data[index] = clamp(50 * n);
                data[index + 1] = clamp(150 * n);
                data[index + 2] = clamp(50 * n);
            } else {
                // Bottom part - dirt brown
                float n = noise(x, y, 1002) * 0.4f + 0.6f;
                data[index] = clamp(120 * n);
                data[index + 1] = clamp(80 * n);
                data[index + 2] = clamp(40 * n);
            }
        }
    }
}

void TextureAtlas::generateDirtTexture(unsigned char* data, int startX, int startY) {
    for (int y = 0; y < textureSize; y++) {
        for (int x = 0; x < textureSize; x++) {
            int index = ((startY + y) * atlasSize + (startX + x)) * 3;
            
            // Brown dirt with noise
            float n = noise(x, y, 2000) * 0.4f + 0.6f;
            data[index] = clamp(120 * n);     // R - brown
            data[index + 1] = clamp(80 * n);  // G - darker
            data[index + 2] = clamp(40 * n);  // B - darkest
        }
    }
}

void TextureAtlas::generateStoneTexture(unsigned char* data, int startX, int startY) {
    for (int y = 0; y < textureSize; y++) {
        for (int x = 0; x < textureSize; x++) {
            int index = ((startY + y) * atlasSize + (startX + x)) * 3;
            
            // Gray stone with noise
            float n = noise(x, y, 3000) * 0.5f + 0.5f;
            unsigned char gray = clamp(100 * n);
            data[index] = gray;
            data[index + 1] = gray;
            data[index + 2] = gray;
        }
    }
}

void TextureAtlas::generateCobblestoneTexture(unsigned char* data, int startX, int startY) {
    for (int y = 0; y < textureSize; y++) {
        for (int x = 0; x < textureSize; x++) {
            int index = ((startY + y) * atlasSize + (startX + x)) * 3;
            
            // Darker gray with more varied noise for cobblestone look
            float n1 = noise(x, y, 3001) * 0.6f + 0.4f;
            float n2 = noise(x / 2, y / 2, 3002) * 0.3f + 0.7f;
            unsigned char gray = clamp(80 * n1 * n2);
            data[index] = gray;
            data[index + 1] = gray;
            data[index + 2] = gray;
        }
    }
}

void TextureAtlas::generateWoodPlankTexture(unsigned char* data, int startX, int startY) {
    for (int y = 0; y < textureSize; y++) {
        for (int x = 0; x < textureSize; x++) {
            int index = ((startY + y) * atlasSize + (startX + x)) * 3;
            
            // Horizontal wood grain pattern
            float grain = sin(y * 0.8f) * 0.1f + 1.0f;
            float n = noise(x, y, 4000) * 0.3f + 0.7f;
            data[index] = clamp(160 * n * grain);     // R - wood brown
            data[index + 1] = clamp(120 * n * grain); // G
            data[index + 2] = clamp(80 * n * grain);  // B
        }
    }
}

void TextureAtlas::generateWoodLogTopTexture(unsigned char* data, int startX, int startY) {
    int centerX = textureSize / 2;
    int centerY = textureSize / 2;
    
    for (int y = 0; y < textureSize; y++) {
        for (int x = 0; x < textureSize; x++) {
            int index = ((startY + y) * atlasSize + (startX + x)) * 3;
            
            // Circular rings for log top
            float distance = sqrt((x - centerX) * (x - centerX) + (y - centerY) * (y - centerY));
            float ring = sin(distance * 0.8f) * 0.2f + 1.0f;
            float n = noise(x, y, 4001) * 0.2f + 0.8f;
            
            data[index] = clamp(140 * n * ring);
            data[index + 1] = clamp(100 * n * ring);
            data[index + 2] = clamp(60 * n * ring);
        }
    }
}

void TextureAtlas::generateWoodLogSideTexture(unsigned char* data, int startX, int startY) {
    for (int y = 0; y < textureSize; y++) {
        for (int x = 0; x < textureSize; x++) {
            int index = ((startY + y) * atlasSize + (startX + x)) * 3;
            
            // Vertical wood grain
            float grain = sin(x * 0.5f) * 0.15f + 1.0f;
            float n = noise(x, y, 4002) * 0.3f + 0.7f;
            
            data[index] = clamp(140 * n * grain);
            data[index + 1] = clamp(100 * n * grain);
            data[index + 2] = clamp(60 * n * grain);
        }
    }
}

void TextureAtlas::generateLeavesTexture(unsigned char* data, int startX, int startY) {
    for (int y = 0; y < textureSize; y++) {
        for (int x = 0; x < textureSize; x++) {
            int index = ((startY + y) * atlasSize + (startX + x)) * 3;
            
            // Green leaves with transparency effect
            float n = noise(x, y, 5000) * 0.6f + 0.4f;
            if (noise(x, y, 5001) < 0.2f) {
                // Gaps in leaves
                data[index] = clamp(40 * n);
                data[index + 1] = clamp(80 * n);
                data[index + 2] = clamp(40 * n);
            } else {
                data[index] = clamp(60 * n);
                data[index + 1] = clamp(120 * n);
                data[index + 2] = clamp(60 * n);
            }
        }
    }
}

void TextureAtlas::generateSandTexture(unsigned char* data, int startX, int startY) {
    for (int y = 0; y < textureSize; y++) {
        for (int x = 0; x < textureSize; x++) {
            int index = ((startY + y) * atlasSize + (startX + x)) * 3;
            
            // Sandy yellow/beige color
            float n = noise(x, y, 6000) * 0.3f + 0.7f;
            data[index] = clamp(220 * n);     // R - yellow
            data[index + 1] = clamp(200 * n); // G
            data[index + 2] = clamp(140 * n); // B
        }
    }
}

void TextureAtlas::generateWaterTexture(unsigned char* data, int startX, int startY) {
    for (int y = 0; y < textureSize; y++) {
        for (int x = 0; x < textureSize; x++) {
            int index = ((startY + y) * atlasSize + (startX + x)) * 3;
            
            // Blue water with wave-like pattern
            float wave = sin((x + y) * 0.5f) * 0.2f + 1.0f;
            float n = noise(x, y, 7000) * 0.3f + 0.7f;
            
            data[index] = clamp(40 * n * wave);      // R - low
            data[index + 1] = clamp(80 * n * wave);  // G - medium
            data[index + 2] = clamp(200 * n * wave); // B - high blue
        }
    }
}

void TextureAtlas::generateBedrockTexture(unsigned char* data, int startX, int startY) {
    for (int y = 0; y < textureSize; y++) {
        for (int x = 0; x < textureSize; x++) {
            int index = ((startY + y) * atlasSize + (startX + x)) * 3;
            
            // Very dark, almost black with subtle variation
            float n = noise(x, y, 8000) * 0.2f + 0.8f;
            unsigned char dark = clamp(30 * n);
            data[index] = dark;
            data[index + 1] = dark;
            data[index + 2] = dark;
        }
    }
}

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

float TextureAtlas::noise(int x, int y, int seed) const {
    int n = x + y * 57 + seed * 131;
    n = (n << 13) ^ n;
    return (1.0f - ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f);
}

unsigned char TextureAtlas::clamp(float value) const {
    if (value < 0.0f) return 0;
    if (value > 255.0f) return 255;
    return static_cast<unsigned char>(value);
}
