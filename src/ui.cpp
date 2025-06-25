#include "ui.h"
#include <iostream>
#include <glm/gtc/type_ptr.hpp>
#include "block_database.h"

// UI constants
const float UI::CROSSHAIR_SIZE = 0.02f;
const float UI::HOTBAR_SLOT_SIZE = 0.08f;
const float UI::HOTBAR_HEIGHT = 0.12f;

// UI Shaders
const char* uiVertexShader = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

uniform mat4 projection;
uniform mat4 model;

out vec2 TexCoord;

void main() {
    gl_Position = projection * model * vec4(aPos, 0.0, 1.0);
    TexCoord = aTexCoord;
}
)";

const char* uiFragmentShader = R"(
#version 330 core
in vec2 TexCoord;
out vec4 FragColor;

uniform vec3 color;
uniform float alpha;

void main() {
    FragColor = vec4(color, alpha);
}
)";

UI::UI() : selectedSlot(0), inventoryOpen(false), hasTargetedBlock(false) {
    // Initialize hotbar with common building blocks
    hotbarBlocks[0] = BlockType::GRASS;
    hotbarBlocks[1] = BlockType::DIRT;
    hotbarBlocks[2] = BlockType::STONE;
    hotbarBlocks[3] = BlockType::COBBLESTONE;
    hotbarBlocks[4] = BlockType::WOOD_PLANK;
    hotbarBlocks[5] = BlockType::WOOD_LOG;
    hotbarBlocks[6] = BlockType::SAND;
    hotbarBlocks[7] = BlockType::BRICK;
    hotbarBlocks[8] = BlockType::OBSIDIAN;
    
    // Initialize inventory with available blocks
    initializeInventoryBlocks();
}

UI::~UI() {
    if (crosshairVAO) glDeleteVertexArrays(1, &crosshairVAO);
    if (crosshairVBO) glDeleteBuffers(1, &crosshairVBO);
    if (hotbarVAO) glDeleteVertexArrays(1, &hotbarVAO);
    if (hotbarVBO) glDeleteBuffers(1, &hotbarVBO);
    if (hotbarEBO) glDeleteBuffers(1, &hotbarEBO);
    if (hudVAO) glDeleteVertexArrays(1, &hudVAO);
    if (hudVBO) glDeleteBuffers(1, &hudVBO);
    if (inventoryVAO) glDeleteVertexArrays(1, &inventoryVAO);
    if (inventoryVBO) glDeleteBuffers(1, &inventoryVBO);
    if (inventoryEBO) glDeleteBuffers(1, &inventoryEBO);
    if (uiShaderProgram) glDeleteProgram(uiShaderProgram);
}

bool UI::initialize() {
    // Create UI shader program
    uiShaderProgram = createUIShader();
    if (uiShaderProgram == 0) {
        std::cerr << "Failed to create UI shader program" << std::endl;
        return false;
    }
    
    // Initialize UI elements
    initializeCrosshair();
    initializeHotbar();
    initializeHUD();
    initializeInventory();
    
    std::cout << "UI system initialized successfully" << std::endl;
    return true;
}

void UI::render(int windowWidth, int windowHeight) {
    // Disable depth testing for UI
    glDisable(GL_DEPTH_TEST);
    
    // Enable blending for UI transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Render main UI elements
    renderCrosshair(windowWidth, windowHeight);
    renderHotbar(windowWidth, windowHeight);
    renderHUD(windowWidth, windowHeight);
    renderBlockInfo(windowWidth, windowHeight);
    
    // Render inventory if open
    if (inventoryOpen) {
        renderInventory(windowWidth, windowHeight);
    }
    
    // Re-enable depth testing
    glEnable(GL_DEPTH_TEST);
}

void UI::selectSlot(int slot) {
    if (slot >= 0 && slot < HOTBAR_SIZE) {
        selectedSlot = slot;
    }
}

BlockType UI::getSelectedBlockType() const {
    return hotbarBlocks[selectedSlot];
}

void UI::handleScrollInput(double yOffset) {
    // Scroll through hotbar slots
    selectedSlot -= (int)yOffset;
    
    // Wrap around
    if (selectedSlot < 0) {
        selectedSlot = HOTBAR_SIZE - 1;
    } else if (selectedSlot >= HOTBAR_SIZE) {
        selectedSlot = 0;
    }
}

void UI::handleNumberKey(int key) {
    // Handle number keys 1-9 for hotbar selection
    if (key >= 1 && key <= 9) {
        selectedSlot = key - 1;
    }
}

void UI::renderCrosshair(int windowWidth, int windowHeight) {
    glUseProgram(uiShaderProgram);
    
    // Set up orthographic projection
    glm::mat4 projection = glm::ortho(0.0f, (float)windowWidth, 0.0f, (float)windowHeight);
    glUniformMatrix4fv(glGetUniformLocation(uiShaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    
    // Center the crosshair
    float centerX = windowWidth * 0.5f;
    float centerY = windowHeight * 0.5f;
    float size = CROSSHAIR_SIZE * windowHeight;
    
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(centerX - size*0.5f, centerY - size*0.5f, 0.0f));
    model = glm::scale(model, glm::vec3(size, size, 1.0f));
    glUniformMatrix4fv(glGetUniformLocation(uiShaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    
    // Set crosshair color (white)
    glUniform3f(glGetUniformLocation(uiShaderProgram, "color"), 1.0f, 1.0f, 1.0f);
    glUniform1f(glGetUniformLocation(uiShaderProgram, "alpha"), 0.8f);
    
    glBindVertexArray(crosshairVAO);
    glDrawArrays(GL_LINES, 0, 4);
    glBindVertexArray(0);
}

void UI::renderHotbar(int windowWidth, int windowHeight) {
    glUseProgram(uiShaderProgram);
    
    // Set up orthographic projection
    glm::mat4 projection = glm::ortho(0.0f, (float)windowWidth, 0.0f, (float)windowHeight);
    glUniformMatrix4fv(glGetUniformLocation(uiShaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    
    float slotSize = HOTBAR_SLOT_SIZE * windowHeight;
    float totalWidth = HOTBAR_SIZE * slotSize;
    float startX = (windowWidth - totalWidth) * 0.5f;
    float y = slotSize * 0.5f; // Bottom of screen with some padding
    
    for (int i = 0; i < HOTBAR_SIZE; i++) {
        float x = startX + i * slotSize;
        
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(x, y, 0.0f));
        model = glm::scale(model, glm::vec3(slotSize, slotSize, 1.0f));
        glUniformMatrix4fv(glGetUniformLocation(uiShaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        
        // Set slot color (selected slot is brighter)
        if (i == selectedSlot) {
            glUniform3f(glGetUniformLocation(uiShaderProgram, "color"), 1.0f, 1.0f, 1.0f);
            glUniform1f(glGetUniformLocation(uiShaderProgram, "alpha"), 0.8f);
        } else {
            glUniform3f(glGetUniformLocation(uiShaderProgram, "color"), 0.3f, 0.3f, 0.3f);
            glUniform1f(glGetUniformLocation(uiShaderProgram, "alpha"), 0.6f);
        }
        
        glBindVertexArray(hotbarVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
}

GLuint UI::createUIShader() {
    // Compile vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &uiVertexShader, NULL);
    glCompileShader(vertexShader);
    
    // Check for compilation errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << "UI Vertex shader compilation failed: " << infoLog << std::endl;
        return 0;
    }
    
    // Compile fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &uiFragmentShader, NULL);
    glCompileShader(fragmentShader);
    
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cerr << "UI Fragment shader compilation failed: " << infoLog << std::endl;
        return 0;
    }
    
    // Create shader program
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "UI Shader program linking failed: " << infoLog << std::endl;
        return 0;
    }
    
    // Clean up shaders
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    return shaderProgram;
}

void UI::initializeCrosshair() {
    // Crosshair vertices (two lines forming a +)
    float crosshairVertices[] = {
        // Horizontal line
        0.0f, 0.5f,
        1.0f, 0.5f,
        // Vertical line
        0.5f, 0.0f,
        0.5f, 1.0f
    };
    
    glGenVertexArrays(1, &crosshairVAO);
    glGenBuffers(1, &crosshairVBO);
    
    glBindVertexArray(crosshairVAO);
    glBindBuffer(GL_ARRAY_BUFFER, crosshairVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(crosshairVertices), crosshairVertices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glBindVertexArray(0);
}

void UI::initializeHotbar() {
    // Hotbar slot vertices (rectangle)
    float slotVertices[] = {
        // positions
        0.0f, 0.0f,  // bottom left
        1.0f, 0.0f,  // bottom right
        1.0f, 1.0f,  // top right
        0.0f, 1.0f   // top left
    };
    
    unsigned int slotIndices[] = {
        0, 1, 2,  // first triangle
        2, 3, 0   // second triangle
    };
    
    glGenVertexArrays(1, &hotbarVAO);
    glGenBuffers(1, &hotbarVBO);
    glGenBuffers(1, &hotbarEBO);
    
    glBindVertexArray(hotbarVAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, hotbarVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(slotVertices), slotVertices, GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, hotbarEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(slotIndices), slotIndices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glBindVertexArray(0);
}

void UI::setSelectedBlockType(BlockType blockType) {
    hotbarBlocks[selectedSlot] = blockType;
}

void UI::toggleInventory() {
    inventoryOpen = !inventoryOpen;
}

void UI::setTargetedBlock(BlockType blockType, const glm::vec3& position) {
    hasTargetedBlock = true;
    targetedBlockType = blockType;
    targetedBlockPosition = position;
}

void UI::clearTargetedBlock() {
    hasTargetedBlock = false;
}

void UI::renderHUD(int windowWidth, int windowHeight) {
    // Render debug information and coordinates
    glUseProgram(uiShaderProgram);
    
    // Set up orthographic projection
    glm::mat4 projection = glm::ortho(0.0f, (float)windowWidth, 0.0f, (float)windowHeight);
    glUniformMatrix4fv(glGetUniformLocation(uiShaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    
    // Simple coordinate display (top-left corner)
    float x = 10.0f;
    float y = windowHeight - 30.0f;
    
    // Draw a simple background for debug info
    drawQuad(5.0f, y - 10.0f, 200.0f, 25.0f, glm::vec3(0.0f, 0.0f, 0.0f), 0.5f);
    
    // Note: Text rendering would need a proper font system
    // For now, we'll just show visual indicators
}

void UI::renderInventory(int windowWidth, int windowHeight) {
    if (!inventoryOpen) return;
    
    glUseProgram(uiShaderProgram);
    
    // Set up orthographic projection
    glm::mat4 projection = glm::ortho(0.0f, (float)windowWidth, 0.0f, (float)windowHeight);
    glUniformMatrix4fv(glGetUniformLocation(uiShaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    
    // Center the inventory
    float invWidth = INVENTORY_COLS * 50.0f + 20.0f;
    float invHeight = INVENTORY_ROWS * 50.0f + 60.0f;
    float centerX = (windowWidth - invWidth) * 0.5f;
    float centerY = (windowHeight - invHeight) * 0.5f;
    
    // Draw inventory background
    drawQuad(centerX, centerY, invWidth, invHeight, glm::vec3(0.2f, 0.2f, 0.2f), 0.9f);
    drawBorder(centerX, centerY, invWidth, invHeight, glm::vec3(0.6f, 0.6f, 0.6f));
    
    // Draw inventory slots
    for (int row = 0; row < INVENTORY_ROWS; row++) {
        for (int col = 0; col < INVENTORY_COLS; col++) {
            float slotX = centerX + 10.0f + col * 50.0f;
            float slotY = centerY + 50.0f + row * 50.0f;
            
            // Draw slot background
            drawQuad(slotX, slotY, 45.0f, 45.0f, glm::vec3(0.4f, 0.4f, 0.4f), 0.8f);
            drawBorder(slotX, slotY, 45.0f, 45.0f, glm::vec3(0.6f, 0.6f, 0.6f));
            
            // Draw block representation (simplified as colored squares)
            int slotIndex = row * INVENTORY_COLS + col;
            if (slotIndex < INVENTORY_ROWS * INVENTORY_COLS) {
                BlockType blockType = inventoryBlocks[slotIndex];
                if (blockType != BlockType::AIR) {
                    glm::vec3 blockColor = getBlockColor(blockType);
                    drawQuad(slotX + 5.0f, slotY + 5.0f, 35.0f, 35.0f, blockColor, 1.0f);
                }
            }
        }
    }
}

void UI::renderBlockInfo(int windowWidth, int windowHeight) {
    if (!hasTargetedBlock) return;
    
    glUseProgram(uiShaderProgram);
    
    // Set up orthographic projection
    glm::mat4 projection = glm::ortho(0.0f, (float)windowWidth, 0.0f, (float)windowHeight);
    glUniformMatrix4fv(glGetUniformLocation(uiShaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    
    // Show block info in top-right corner
    float infoWidth = 150.0f;
    float infoHeight = 60.0f;
    float x = windowWidth - infoWidth - 10.0f;
    float y = windowHeight - infoHeight - 10.0f;
    
    // Draw background
    drawQuad(x, y, infoWidth, infoHeight, glm::vec3(0.0f, 0.0f, 0.0f), 0.7f);
    drawBorder(x, y, infoWidth, infoHeight, glm::vec3(0.8f, 0.8f, 0.8f));
    
    // Draw block color indicator
    glm::vec3 blockColor = getBlockColor(targetedBlockType);
    drawQuad(x + 10.0f, y + 25.0f, 20.0f, 20.0f, blockColor, 1.0f);
    
    // Note: Block name would be rendered here with proper text rendering
}

void UI::initializeHUD() {
    // Initialize HUD rendering objects
    glGenVertexArrays(1, &hudVAO);
    glGenBuffers(1, &hudVBO);
    
    // Simple quad for HUD elements
    float hudVertices[] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f
    };
    
    glBindVertexArray(hudVAO);
    glBindBuffer(GL_ARRAY_BUFFER, hudVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(hudVertices), hudVertices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glBindVertexArray(0);
}

void UI::initializeInventory() {
    // Initialize inventory rendering objects
    glGenVertexArrays(1, &inventoryVAO);
    glGenBuffers(1, &inventoryVBO);
    glGenBuffers(1, &inventoryEBO);
    
    float inventoryVertices[] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f
    };
    
    unsigned int inventoryIndices[] = {
        0, 1, 2,
        2, 3, 0
    };
    
    glBindVertexArray(inventoryVAO);
    glBindBuffer(GL_ARRAY_BUFFER, inventoryVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(inventoryVertices), inventoryVertices, GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, inventoryEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(inventoryIndices), inventoryIndices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glBindVertexArray(0);
}

void UI::initializeInventoryBlocks() {
    // Initialize inventory with all available block types
    int index = 0;
    BlockType availableBlocks[] = {
        BlockType::GRASS, BlockType::DIRT, BlockType::STONE, BlockType::COBBLESTONE,
        BlockType::WOOD_PLANK, BlockType::WOOD_LOG, BlockType::LEAVES, BlockType::SAND,
        BlockType::WATER, BlockType::BEDROCK, BlockType::SNOW, BlockType::ICE,
        BlockType::GLOWSTONE, BlockType::OBSIDIAN, BlockType::BRICK, BlockType::MOSSY_STONE,
        BlockType::GRAVEL, BlockType::GOLD_ORE, BlockType::IRON_ORE, BlockType::DIAMOND_ORE,
        BlockType::EMERALD_ORE, BlockType::REDSTONE_ORE
    };
    
    for (BlockType blockType : availableBlocks) {
        if (index < INVENTORY_ROWS * INVENTORY_COLS) {
            inventoryBlocks[index++] = blockType;
        }
    }
    
    // Fill remaining slots with air
    while (index < INVENTORY_ROWS * INVENTORY_COLS) {
        inventoryBlocks[index++] = BlockType::AIR;
    }
}

void UI::drawQuad(float x, float y, float width, float height, const glm::vec3& color, float alpha) {
    glUseProgram(uiShaderProgram);
    
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(x, y, 0.0f));
    model = glm::scale(model, glm::vec3(width, height, 1.0f));
    glUniformMatrix4fv(glGetUniformLocation(uiShaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    
    glUniform3f(glGetUniformLocation(uiShaderProgram, "color"), color.r, color.g, color.b);
    glUniform1f(glGetUniformLocation(uiShaderProgram, "alpha"), alpha);
    
    glBindVertexArray(hudVAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindVertexArray(0);
}

void UI::drawBorder(float x, float y, float width, float height, const glm::vec3& color, float thickness) {
    // Draw four rectangles to form a border
    drawQuad(x, y, width, thickness, color, 1.0f); // Bottom
    drawQuad(x, y + height - thickness, width, thickness, color, 1.0f); // Top
    drawQuad(x, y, thickness, height, color, 1.0f); // Left
    drawQuad(x + width - thickness, y, thickness, height, color, 1.0f); // Right
}

std::string UI::getBlockName(BlockType blockType) {
    // Simple block name mapping
    switch (blockType) {
        case BlockType::GRASS: return "Grass";
        case BlockType::DIRT: return "Dirt";
        case BlockType::STONE: return "Stone";
        case BlockType::COBBLESTONE: return "Cobblestone";
        case BlockType::WOOD_PLANK: return "Wood Planks";
        case BlockType::WOOD_LOG: return "Wood Log";
        case BlockType::LEAVES: return "Leaves";
        case BlockType::SAND: return "Sand";
        case BlockType::WATER: return "Water";
        case BlockType::BEDROCK: return "Bedrock";
        case BlockType::SNOW: return "Snow";
        case BlockType::ICE: return "Ice";
        case BlockType::GLOWSTONE: return "Glowstone";
        case BlockType::OBSIDIAN: return "Obsidian";
        case BlockType::BRICK: return "Brick";
        case BlockType::MOSSY_STONE: return "Mossy Stone";
        case BlockType::GRAVEL: return "Gravel";
        case BlockType::GOLD_ORE: return "Gold Ore";
        case BlockType::IRON_ORE: return "Iron Ore";
        case BlockType::DIAMOND_ORE: return "Diamond Ore";
        case BlockType::EMERALD_ORE: return "Emerald Ore";
        case BlockType::REDSTONE_ORE: return "Redstone Ore";
        default: return "Unknown";
    }
}

glm::vec3 UI::getBlockColor(BlockType blockType) {
    // Return representative colors for blocks
    switch (blockType) {
        case BlockType::GRASS: return glm::vec3(0.3f, 0.8f, 0.2f);
        case BlockType::DIRT: return glm::vec3(0.6f, 0.4f, 0.2f);
        case BlockType::STONE: return glm::vec3(0.5f, 0.5f, 0.5f);
        case BlockType::COBBLESTONE: return glm::vec3(0.4f, 0.4f, 0.4f);
        case BlockType::WOOD_PLANK: return glm::vec3(0.8f, 0.6f, 0.3f);
        case BlockType::WOOD_LOG: return glm::vec3(0.6f, 0.4f, 0.2f);
        case BlockType::LEAVES: return glm::vec3(0.2f, 0.6f, 0.1f);
        case BlockType::SAND: return glm::vec3(0.9f, 0.8f, 0.6f);
        case BlockType::WATER: return glm::vec3(0.2f, 0.5f, 0.9f);
        case BlockType::BEDROCK: return glm::vec3(0.1f, 0.1f, 0.1f);
        case BlockType::SNOW: return glm::vec3(0.9f, 0.9f, 0.9f);
        case BlockType::ICE: return glm::vec3(0.7f, 0.9f, 0.9f);
        case BlockType::GLOWSTONE: return glm::vec3(1.0f, 0.8f, 0.2f);
        case BlockType::OBSIDIAN: return glm::vec3(0.2f, 0.1f, 0.2f);
        case BlockType::BRICK: return glm::vec3(0.7f, 0.3f, 0.2f);
        case BlockType::MOSSY_STONE: return glm::vec3(0.4f, 0.5f, 0.3f);
        case BlockType::GRAVEL: return glm::vec3(0.6f, 0.6f, 0.5f);
        case BlockType::GOLD_ORE: return glm::vec3(1.0f, 0.8f, 0.0f);
        case BlockType::IRON_ORE: return glm::vec3(0.7f, 0.6f, 0.5f);
        case BlockType::DIAMOND_ORE: return glm::vec3(0.4f, 0.8f, 0.9f);
        case BlockType::EMERALD_ORE: return glm::vec3(0.2f, 0.8f, 0.3f);
        case BlockType::REDSTONE_ORE: return glm::vec3(0.8f, 0.2f, 0.2f);
        default: return glm::vec3(0.5f, 0.5f, 0.5f);
    }
}

void UI::renderText(const std::string& text, float x, float y, float scale) {
    // TODO: Implement proper text rendering with a bitmap font
    // For now, this is a placeholder
}

void UI::addBlockToInventory(BlockType blockType) {
    // Find empty slot and add block
    for (int i = 0; i < INVENTORY_ROWS * INVENTORY_COLS; i++) {
        if (inventoryBlocks[i] == BlockType::AIR) {
            inventoryBlocks[i] = blockType;
            break;
        }
    }
}

bool UI::removeBlockFromInventory(BlockType blockType) {
    // Find and remove block from inventory
    for (int i = 0; i < INVENTORY_ROWS * INVENTORY_COLS; i++) {
        if (inventoryBlocks[i] == blockType) {
            inventoryBlocks[i] = BlockType::AIR;
            return true;
        }
    }
    return false;
}

int UI::getBlockCount(BlockType blockType) {
    int count = 0;
    for (int i = 0; i < INVENTORY_ROWS * INVENTORY_COLS; i++) {
        if (inventoryBlocks[i] == blockType) {
            count++;
        }
    }
    return count;
}
