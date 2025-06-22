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
    std::fill(atlasData.begin(), atlasData.end(), 255);    // Generate each block texture
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
            case BlockType::SNOW:
                generateSnowTexture(atlasData.data(), atlasX, atlasY);
                break;
            case BlockType::ICE:
                generateIceTexture(atlasData.data(), atlasX, atlasY);
                break;
            case BlockType::GLOWSTONE:
                generateGlowstoneTexture(atlasData.data(), atlasX, atlasY);
                break;
            case BlockType::OBSIDIAN:
                generateObsidianTexture(atlasData.data(), atlasX, atlasY);
                break;
            case BlockType::BRICK:
                generateBrickTexture(atlasData.data(), atlasX, atlasY);
                break;
            case BlockType::MOSSY_STONE:
                generateMossyStoneTexture(atlasData.data(), atlasX, atlasY);
                break;
            case BlockType::GRAVEL:
                generateGravelTexture(atlasData.data(), atlasX, atlasY);
                break;
            case BlockType::GOLD_ORE:
                generateGoldOreTexture(atlasData.data(), atlasX, atlasY);
                break;
            case BlockType::IRON_ORE:
                generateIronOreTexture(atlasData.data(), atlasX, atlasY);
                break;
            case BlockType::DIAMOND_ORE:
                generateDiamondOreTexture(atlasData.data(), atlasX, atlasY);
                break;
            case BlockType::EMERALD_ORE:
                generateEmeraldOreTexture(atlasData.data(), atlasX, atlasY);
                break;
            case BlockType::REDSTONE_ORE:
                generateRedstoneOreTexture(atlasData.data(), atlasX, atlasY);
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
            
            // More vibrant and realistic grass
            float n1 = noise(x, y, 1000) * 0.4f + 0.6f;
            float n2 = noise(x * 3, y * 3, 1001) * 0.3f + 0.7f;
            float grassBlade = noise(x * 6, y * 6, 1002) > 0.6f ? 1.3f : 0.9f;
            float detail = noise(x * 8, y * 8, 1003) * 0.2f + 0.9f;
            
            // Rich, lush green with natural variation
            data[index] = clamp(35 * n1 * detail);                    // R - minimal red
            data[index + 1] = clamp(220 * n1 * n2 * grassBlade);     // G - rich vibrant green
            data[index + 2] = clamp(85 * n1 * detail);               // B - subtle blue for depth
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
            
            // Rich, earthy brown dirt with organic variation
            float n1 = noise(x, y, 2000) * 0.4f + 0.6f;
            float n2 = noise(x * 2, y * 2, 2001) * 0.3f + 0.7f;
            float moisture = noise(x * 3, y * 3, 2002) * 0.2f + 0.8f;
            float organic = noise(x * 4, y * 4, 2003) > 0.7f ? 1.2f : 0.9f;
            
            data[index] = clamp(135 * n1 * n2 * organic);      // R - rich brown
            data[index + 1] = clamp(95 * n1 * n2 * moisture); // G - earthy tone
            data[index + 2] = clamp(55 * n1 * n2);             // B - dark earth
        }
    }
}

void TextureAtlas::generateStoneTexture(unsigned char* data, int startX, int startY) {
    for (int y = 0; y < textureSize; y++) {
        for (int x = 0; x < textureSize; x++) {
            int index = ((startY + y) * atlasSize + (startX + x)) * 3;
            
            // More realistic stone with layered noise and mineral variation
            float n1 = noise(x, y, 3000) * 0.5f + 0.5f;
            float n2 = noise(x * 2, y * 2, 3001) * 0.3f + 0.7f;
            float n3 = noise(x * 4, y * 4, 3002) * 0.2f + 0.8f;
            float crack = abs(noise(x * 3, y * 3, 3003)) > 0.75f ? 0.6f : 1.0f;
            float mineral = noise(x * 5, y * 5, 3004) > 0.8f ? 1.2f : 1.0f;
            
            unsigned char base = clamp(140 * n1 * n2 * n3 * crack);
            data[index] = clamp(base * 1.1f * mineral);      // R - warm stone
            data[index + 1] = clamp(base * mineral);         // G - natural gray
            data[index + 2] = clamp(base * 0.9f * mineral);  // B - slightly cool
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
            
            // Lush, vibrant foliage with natural depth
            float n1 = noise(x, y, 5000) * 0.4f + 0.6f;
            float n2 = noise(x * 2, y * 2, 5001) * 0.3f + 0.7f;
            float leafDetail = noise(x * 4, y * 4, 5002) > 0.55f ? 1.3f : 0.8f;
            float sunlight = noise(x * 3, y * 3, 5003) * 0.2f + 0.8f;
            
            // Create natural transparency/gaps for realism
            if (noise(x * 6, y * 6, 5004) < 0.12f) {
                // Small gaps between leaves
                data[index] = clamp(40 * n1);
                data[index + 1] = clamp(80 * n1);
                data[index + 2] = clamp(40 * n1);
            } else {
                // Rich green foliage
                data[index] = clamp(45 * n1 * sunlight);               // R - forest green
                data[index + 1] = clamp(180 * n1 * n2 * leafDetail);  // G - vibrant green
                data[index + 2] = clamp(70 * n1 * sunlight);          // B - natural depth
            }
        }
    }
}

void TextureAtlas::generateSandTexture(unsigned char* data, int startX, int startY) {
    for (int y = 0; y < textureSize; y++) {
        for (int x = 0; x < textureSize; x++) {
            int index = ((startY + y) * atlasSize + (startX + x)) * 3;
            
            // Warm, golden desert sand with fine grain detail
            float n1 = noise(x, y, 6000) * 0.3f + 0.7f;
            float n2 = noise(x * 3, y * 3, 6001) * 0.2f + 0.8f;
            float grain = noise(x * 8, y * 8, 6002) * 0.15f + 0.85f;
            float warmth = noise(x * 2, y * 2, 6003) * 0.1f + 0.9f;
            
            data[index] = clamp(245 * n1 * n2 * warmth);      // R - warm golden
            data[index + 1] = clamp(220 * n1 * n2 * grain);  // G - sandy yellow
            data[index + 2] = clamp(165 * n1 * grain);       // B - desert warmth
        }
    }
}

void TextureAtlas::generateWaterTexture(unsigned char* data, int startX, int startY) {
    for (int y = 0; y < textureSize; y++) {
        for (int x = 0; x < textureSize; x++) {
            int index = ((startY + y) * atlasSize + (startX + x)) * 3;
            
            // Beautiful deep blue water with flowing patterns
            float wave1 = sin((x + y) * 0.6f) * 0.15f + 0.85f;
            float wave2 = cos(x * 0.8f - y * 0.7f) * 0.1f + 0.9f;
            float depth = noise(x, y, 7000) * 0.2f + 0.8f;
            float sparkle = noise(x * 4, y * 4, 7001) > 0.8f ? 1.3f : 1.0f;
            
            data[index] = clamp(25 * depth * wave1);                    // R - minimal
            data[index + 1] = clamp(120 * depth * wave2);              // G - turquoise
            data[index + 2] = clamp(255 * depth * wave1 * sparkle);    // B - deep blue
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
// NEW BEAUTIFUL TEXTURE GENERATION FUNCTIONS
// ============================================================================

void TextureAtlas::generateSnowTexture(unsigned char* data, int startX, int startY) {
    for (int y = 0; y < textureSize; y++) {
        for (int x = 0; x < textureSize; x++) {
            int index = ((startY + y) * atlasSize + (startX + x)) * 3;
            
            // Pure, pristine snow with natural sparkles and depth
            float n1 = noise(x, y, 9000) * 0.15f + 0.85f;
            float n2 = noise(x * 2, y * 2, 9001) * 0.1f + 0.9f;
            float sparkle = noise(x * 4, y * 4, 9002) > 0.85f ? 1.3f : 1.0f;
            float depth = noise(x * 3, y * 3, 9003) * 0.05f + 0.95f;
            
            unsigned char white = clamp(255 * n1 * n2 * depth);
            data[index] = clamp(white * sparkle);                    // R - pure white
            data[index + 1] = clamp(white * sparkle);                // G - pure white  
            data[index + 2] = clamp(white * 0.98f * sparkle);       // B - slight blue tint
        }
    }
}

void TextureAtlas::generateIceTexture(unsigned char* data, int startX, int startY) {
    for (int y = 0; y < textureSize; y++) {
        for (int x = 0; x < textureSize; x++) {
            int index = ((startY + y) * atlasSize + (startX + x)) * 3;
            
            // Translucent blue ice with internal structure
            float crack = abs(noise(x * 3, y * 3, 9100)) > 0.7f ? 0.8f : 1.0f;
            float n = noise(x, y, 9200) * 0.2f + 0.8f;
            
            data[index] = clamp(200 * n * crack);     // R - light blue
            data[index + 1] = clamp(230 * n * crack); // G
            data[index + 2] = clamp(255 * n * crack); // B - strong blue
        }
    }
}

void TextureAtlas::generateGlowstoneTexture(unsigned char* data, int startX, int startY) {
    for (int y = 0; y < textureSize; y++) {
        for (int x = 0; x < textureSize; x++) {
            int index = ((startY + y) * atlasSize + (startX + x)) * 3;
            
            // Bright yellow-orange glowing texture
            float glow = noise(x, y, 9300) * 0.3f + 0.7f;
            float bright = noise(x * 2, y * 2, 9400) * 0.2f + 0.8f;
            
            data[index] = clamp(255 * glow * bright);     // R - bright yellow
            data[index + 1] = clamp(220 * glow * bright); // G - orange
            data[index + 2] = clamp(100 * glow * bright); // B - low blue
        }
    }
}

void TextureAtlas::generateObsidianTexture(unsigned char* data, int startX, int startY) {
    for (int y = 0; y < textureSize; y++) {
        for (int x = 0; x < textureSize; x++) {
            int index = ((startY + y) * atlasSize + (startX + x)) * 3;
            
            // Dark purple-black volcanic glass
            float n = noise(x, y, 9500) * 0.3f + 0.7f;
            float shine = noise(x * 4, y * 4, 9600) > 0.8f ? 1.5f : 1.0f;
            
            data[index] = clamp(60 * n * shine);      // R - dark purple
            data[index + 1] = clamp(30 * n * shine);  // G
            data[index + 2] = clamp(80 * n * shine);  // B - purple tint
        }
    }
}

void TextureAtlas::generateBrickTexture(unsigned char* data, int startX, int startY) {
    for (int y = 0; y < textureSize; y++) {
        for (int x = 0; x < textureSize; x++) {
            int index = ((startY + y) * atlasSize + (startX + x)) * 3;
            
            // Red brick pattern with mortar lines
            bool isморtar = (y % 4 == 0) || (x % 8 == 0 && (y / 4) % 2 == 0) || (x % 8 == 4 && (y / 4) % 2 == 1);
            
            if (isморtar) {
                // Light gray mortar
                data[index] = 180;
                data[index + 1] = 180;
                data[index + 2] = 180;
            } else {
                // Red brick with variation
                float n = noise(x, y, 9700) * 0.3f + 0.7f;
                data[index] = clamp(180 * n);     // R - red
                data[index + 1] = clamp(80 * n);  // G
                data[index + 2] = clamp(60 * n);  // B
            }
        }
    }
}

void TextureAtlas::generateMossyStoneTexture(unsigned char* data, int startX, int startY) {
    for (int y = 0; y < textureSize; y++) {
        for (int x = 0; x < textureSize; x++) {
            int index = ((startY + y) * atlasSize + (startX + x)) * 3;
            
            // Stone with green moss patches
            float stone = noise(x, y, 9800) * 0.3f + 0.7f;
            float moss = noise(x * 2, y * 2, 9900) > 0.6f ? 1.0f : 0.0f;
            
            if (moss > 0.5f) {
                // Green moss
                data[index] = clamp(60 * stone);      // R
                data[index + 1] = clamp(120 * stone); // G - green
                data[index + 2] = clamp(40 * stone);  // B
            } else {
                // Gray stone
                unsigned char gray = clamp(120 * stone);
                data[index] = gray;
                data[index + 1] = gray;
                data[index + 2] = gray;
            }
        }
    }
}

void TextureAtlas::generateGravelTexture(unsigned char* data, int startX, int startY) {
    for (int y = 0; y < textureSize; y++) {
        for (int x = 0; x < textureSize; x++) {
            int index = ((startY + y) * atlasSize + (startX + x)) * 3;
            
            // Mixed gray pebbles
            float size = noise(x * 3, y * 3, 10000);
            float color = noise(x, y, 10100) * 0.4f + 0.6f;
            
            unsigned char base = clamp(100 + 60 * color + 40 * size);
            data[index] = base;
            data[index + 1] = base;
            data[index + 2] = base;
        }
    }
}

void TextureAtlas::generateGoldOreTexture(unsigned char* data, int startX, int startY) {
    for (int y = 0; y < textureSize; y++) {
        for (int x = 0; x < textureSize; x++) {
            int index = ((startY + y) * atlasSize + (startX + x)) * 3;
            
            // Stone with rich golden veins
            float stone = noise(x, y, 10200) * 0.3f + 0.7f;
            float gold = noise(x * 2, y * 2, 10300) > 0.65f ? 1.0f : 0.0f;
            float shine = noise(x * 4, y * 4, 10301) * 0.2f + 0.8f;
            
            if (gold > 0.5f) {
                // Rich, lustrous gold
                data[index] = clamp(255 * stone * shine);      // R - bright gold
                data[index + 1] = clamp(230 * stone * shine);  // G - golden yellow
                data[index + 2] = clamp(50 * stone);           // B - warm tone
            } else {
                // Stone with warm tint from nearby gold
                unsigned char gray = clamp(130 * stone);
                data[index] = clamp(gray * 1.1f);
                data[index + 1] = clamp(gray * 1.05f);
                data[index + 2] = gray;
            }
        }
    }
}

void TextureAtlas::generateIronOreTexture(unsigned char* data, int startX, int startY) {
    for (int y = 0; y < textureSize; y++) {
        for (int x = 0; x < textureSize; x++) {
            int index = ((startY + y) * atlasSize + (startX + x)) * 3;
            
            // Stone with iron spots
            float stone = noise(x, y, 10400) * 0.3f + 0.7f;
            float iron = noise(x * 2, y * 2, 10500) > 0.7f ? 1.0f : 0.0f;
            
            if (iron > 0.5f) {
                // Brownish iron
                data[index] = clamp(140 * stone);     // R - brown
                data[index + 1] = clamp(100 * stone); // G
                data[index + 2] = clamp(80 * stone);  // B
            } else {
                // Gray stone
                unsigned char gray = clamp(120 * stone);
                data[index] = gray;
                data[index + 1] = gray;
                data[index + 2] = gray;
            }
        }
    }
}

void TextureAtlas::generateDiamondOreTexture(unsigned char* data, int startX, int startY) {
    for (int y = 0; y < textureSize; y++) {
        for (int x = 0; x < textureSize; x++) {
            int index = ((startY + y) * atlasSize + (startX + x)) * 3;
            
            // Stone with brilliant blue diamond crystals
            float stone = noise(x, y, 10600) * 0.3f + 0.7f;
            float diamond = noise(x * 3, y * 3, 10700) > 0.75f ? 1.0f : 0.0f;
            float sparkle = noise(x * 6, y * 6, 10701) > 0.8f ? 1.4f : 1.0f;
            
            if (diamond > 0.5f) {
                // Brilliant cyan diamond with sparkle
                data[index] = clamp(150 * stone * sparkle);      // R - ice blue
                data[index + 1] = clamp(255 * stone * sparkle);  // G - brilliant cyan
                data[index + 2] = clamp(255 * stone);            // B - deep blue
            } else {
                // Gray stone background
                unsigned char gray = clamp(125 * stone);
                data[index] = gray;
                data[index + 1] = gray;
                data[index + 2] = gray;
            }
        }
    }
}

void TextureAtlas::generateEmeraldOreTexture(unsigned char* data, int startX, int startY) {
    for (int y = 0; y < textureSize; y++) {
        for (int x = 0; x < textureSize; x++) {
            int index = ((startY + y) * atlasSize + (startX + x)) * 3;
            
            // Stone with bright green emerald spots
            float stone = noise(x, y, 10800) * 0.3f + 0.7f;
            float emerald = noise(x * 3, y * 3, 10900) > 0.8f ? 1.0f : 0.0f;
            
            if (emerald > 0.5f) {
                // Bright green emerald
                data[index] = clamp(50 * stone);      // R
                data[index + 1] = clamp(255 * stone); // G - bright green
                data[index + 2] = clamp(100 * stone); // B
            } else {
                // Gray stone
                unsigned char gray = clamp(120 * stone);
                data[index] = gray;
                data[index + 1] = gray;
                data[index + 2] = gray;
            }
        }
    }
}

void TextureAtlas::generateRedstoneOreTexture(unsigned char* data, int startX, int startY) {
    for (int y = 0; y < textureSize; y++) {
        for (int x = 0; x < textureSize; x++) {
            int index = ((startY + y) * atlasSize + (startX + x)) * 3;
            
            // Stone with glowing red redstone veins
            float stone = noise(x, y, 11000) * 0.3f + 0.7f;
            float redstone = noise(x * 2, y * 2, 11100) > 0.7f ? 1.0f : 0.0f;
            
            if (redstone > 0.5f) {
                // Bright red redstone
                data[index] = clamp(255 * stone);     // R - bright red
                data[index + 1] = clamp(50 * stone);  // G
                data[index + 2] = clamp(50 * stone);  // B
            } else {
                // Gray stone
                unsigned char gray = clamp(120 * stone);
                data[index] = gray;
                data[index + 1] = gray;
                data[index + 2] = gray;
            }
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
