#pragma once

#include <glad/gl.h>  // Include glad first
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include "camera.h"
#include "voxel_chunk.h"
#include <vector>

// Forward declaration
class ChunkManager;

/**
 * Player class that handles first-person character physics, collision detection,
 * and movement in the voxel world. Controls the camera position based on physics.
 */
class Player {
public:
    // ============================================================================
    // PHYSICAL PROPERTIES
    // ============================================================================
    glm::vec3 position;          // Player position in world space
    glm::vec3 velocity;          // Current velocity vector
    glm::vec3 size;              // Player bounding box size (width, height, depth)
    
    // ============================================================================
    // PHYSICS CONSTANTS
    // ============================================================================
    float gravity;               // Gravity acceleration (negative Y)
    float jumpStrength;          // Initial velocity when jumping
    float walkSpeed;             // Walking speed multiplier
    float sprintSpeed;           // Sprinting speed multiplier
    float friction;              // Ground friction coefficient
    float airResistance;         // Air resistance coefficient
    
    // ============================================================================
    // STATE FLAGS
    // ============================================================================
    bool isOnGround;             // Whether player is touching ground
    bool canJump;                // Whether player can jump (prevents bunny hopping)
    
    // ============================================================================    // CONSTRUCTOR
    // ============================================================================
    Player(glm::vec3 startPosition = glm::vec3(0.0f, 20.0f, 0.0f));
    
    // ============================================================================    // UPDATE METHODS
    // ============================================================================
    void update(float deltaTime, GLFWwindow* window, Camera& camera, 
                ChunkManager& chunkManager);
    
    // ============================================================================
    // COLLISION DETECTION
    // ============================================================================
    bool checkCollision(glm::vec3 newPosition, ChunkManager& chunkManager);
    bool isBlockSolid(int worldX, int worldY, int worldZ, ChunkManager& chunkManager);
    glm::vec3 resolveCollision(glm::vec3 oldPos, glm::vec3 newPos, 
                              ChunkManager& chunkManager);
    
    // ============================================================================
    // INPUT PROCESSING
    // ============================================================================
    void processInput(GLFWwindow* window, Camera& camera, float deltaTime);
    
private:
    // ============================================================================    // INTERNAL PHYSICS HELPERS
    // ============================================================================
    void applyGravity(float deltaTime);
    void applyFriction(float deltaTime);
    void updateGroundState(ChunkManager& chunkManager);
};
