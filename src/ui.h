#pragma once

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <vector>
#include <unordered_map>
#include "voxel_chunk.h"

class UI {
public:
    UI();
    ~UI();
    
    // Initialize UI system (call after OpenGL is ready)
    bool initialize();
    
    // Render all UI elements
    void render(int windowWidth, int windowHeight);
    
    // Hotbar management
    void selectSlot(int slot);
    int getSelectedSlot() const { return selectedSlot; }
    BlockType getSelectedBlockType() const;
    void setSelectedBlockType(BlockType blockType); // For block picking
    
    // Input handling
    void handleScrollInput(double yOffset);
    void handleNumberKey(int key);
    void toggleInventory(); // F key to open/close inventory
    bool isInventoryOpen() const { return inventoryOpen; }
    
    // Game state display
    void setTargetedBlock(BlockType blockType, const glm::vec3& position);
    void clearTargetedBlock();
    std::string getBlockName(BlockType blockType);
    
    // UI elements
    void renderCrosshair(int windowWidth, int windowHeight);
    void renderHotbar(int windowWidth, int windowHeight);
    void renderHUD(int windowWidth, int windowHeight); // Health, coordinates, etc.
    void renderInventory(int windowWidth, int windowHeight);
    void renderBlockInfo(int windowWidth, int windowHeight); // Show targeted block info
    
    // Constants for UI sizing
    static const float CROSSHAIR_SIZE;
    static const float HOTBAR_SLOT_SIZE;
    static const float HOTBAR_HEIGHT;
    
private:
    // UI state
    int selectedSlot;
    static const int HOTBAR_SIZE = 9;
    static const int INVENTORY_ROWS = 4;
    static const int INVENTORY_COLS = 9;
    BlockType hotbarBlocks[HOTBAR_SIZE];
    BlockType inventoryBlocks[INVENTORY_ROWS * INVENTORY_COLS];
    bool inventoryOpen;
    
    // Targeted block info
    bool hasTargetedBlock;
    BlockType targetedBlockType;
    glm::vec3 targetedBlockPosition;
    
    // OpenGL objects for UI rendering
    GLuint uiShaderProgram;
    GLuint crosshairVAO, crosshairVBO;
    GLuint hotbarVAO, hotbarVBO, hotbarEBO;
    GLuint hudVAO, hudVBO;
    GLuint inventoryVAO, inventoryVBO, inventoryEBO;
    
    // Shader creation helper
    GLuint createUIShader();
    
    // Initialize specific UI elements
    void initializeCrosshair();
    void initializeHotbar();
    void initializeHUD();
    void initializeInventory();
    
    // Helper functions
    void drawQuad(float x, float y, float width, float height, const glm::vec3& color, float alpha = 1.0f);
    void drawBorder(float x, float y, float width, float height, const glm::vec3& color, float thickness = 0.002f);
    glm::vec3 getBlockColor(BlockType blockType);
    
    // Text rendering (simple bitmap font)
    void renderText(const std::string& text, float x, float y, float scale = 1.0f);
    
    // Block inventory management
    void initializeInventoryBlocks();
    void addBlockToInventory(BlockType blockType);
    bool removeBlockFromInventory(BlockType blockType);
    int getBlockCount(BlockType blockType);
};
