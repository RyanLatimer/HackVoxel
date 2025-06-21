#include "player.h"
#include <iostream>
#include <algorithm>
#include <cmath>

// ============================================================================
// PLAYER CONSTRUCTOR - INITIALIZE PHYSICS PROPERTIES
// ============================================================================
Player::Player(glm::vec3 startPosition)
    : position(startPosition)
    , velocity(0.0f, 0.0f, 0.0f)
    , size(0.6f, 1.8f, 0.6f)  // Width, height, depth (typical FPS character)
    , gravity(-20.0f)          // Gravity acceleration
    , jumpStrength(8.0f)       // Jump initial velocity
    , walkSpeed(5.0f)          // Walking speed
    , sprintSpeed(8.0f)        // Sprint speed
    , friction(15.0f)          // Ground friction
    , airResistance(2.0f)      // Air resistance
    , isOnGround(false)
    , canJump(true)
{
    std::cout << "Player created at position: " << position.x << ", " << position.y << ", " << position.z << std::endl;
}

// ============================================================================
// MAIN UPDATE LOOP - PROCESS PHYSICS AND INPUT
// ============================================================================
void Player::update(float deltaTime, GLFWwindow* window, Camera& camera, 
                   const std::vector<VoxelChunk>& chunks) {
    // Process player input
    processInput(window, camera, deltaTime);
    
    // Apply physics
    applyGravity(deltaTime);
    applyFriction(deltaTime);
    
    // Calculate new position
    glm::vec3 newPosition = position + velocity * deltaTime;
    
    // Resolve collisions and update position
    position = resolveCollision(position, newPosition, chunks);
    
    // Update ground state for next frame
    updateGroundState(chunks);
    
    // Update camera to follow player (eye level is at player height - 0.2f)
    camera.position = position + glm::vec3(0.0f, size.y - 0.2f, 0.0f);
}

// ============================================================================
// PROCESS INPUT FOR MOVEMENT AND JUMPING
// ============================================================================
void Player::processInput(GLFWwindow* window, Camera& camera, float deltaTime) {
    // Get movement input
    glm::vec3 moveInput(0.0f);
    
    // Calculate forward and right vectors (only horizontal movement)
    glm::vec3 forward = glm::normalize(glm::vec3(camera.front.x, 0.0f, camera.front.z));
    glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));
    
    // WASD movement
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        moveInput += forward;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        moveInput -= forward;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        moveInput -= right;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        moveInput += right;
    }
    
    // Normalize movement to prevent faster diagonal movement
    if (glm::length(moveInput) > 0.0f) {
        moveInput = glm::normalize(moveInput);
        
        // Apply speed (check for sprint)
        float currentSpeed = (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) ? 
                           sprintSpeed : walkSpeed;
        
        // Apply movement to horizontal velocity
        velocity.x = moveInput.x * currentSpeed;
        velocity.z = moveInput.z * currentSpeed;
    } else {
        // No input - apply friction only to horizontal movement
        if (isOnGround) {
            velocity.x *= 0.1f;  // Quick stop on ground
            velocity.z *= 0.1f;
        }
    }
    
    // Jumping
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && isOnGround && canJump) {
        velocity.y = jumpStrength;
        isOnGround = false;
        canJump = false;  // Prevent bunny hopping
    }
    
    // Reset jump flag when space is released
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE) {
        canJump = true;
    }
    
    // Exit application (ESC key)
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}

// ============================================================================
// APPLY GRAVITY TO VERTICAL VELOCITY
// ============================================================================
void Player::applyGravity(float deltaTime) {
    if (!isOnGround) {
        velocity.y += gravity * deltaTime;
        
        // Apply air resistance to horizontal movement
        velocity.x *= (1.0f - airResistance * deltaTime);
        velocity.z *= (1.0f - airResistance * deltaTime);
    }
}

// ============================================================================
// APPLY FRICTION WHEN ON GROUND
// ============================================================================
void Player::applyFriction(float deltaTime) {
    if (isOnGround) {
        // Apply friction to horizontal movement
        float frictionForce = friction * deltaTime;
        velocity.x *= std::max(0.0f, 1.0f - frictionForce);
        velocity.z *= std::max(0.0f, 1.0f - frictionForce);
    }
}

// ============================================================================
// COLLISION RESOLUTION - SEPARATE AXES FOR BETTER BEHAVIOR
// ============================================================================
glm::vec3 Player::resolveCollision(glm::vec3 oldPos, glm::vec3 newPos, 
                                  const std::vector<VoxelChunk>& chunks) {
    glm::vec3 resolvedPos = oldPos;
    
    // Test X movement
    glm::vec3 testPosX = glm::vec3(newPos.x, oldPos.y, oldPos.z);
    if (!checkCollision(testPosX, chunks)) {
        resolvedPos.x = newPos.x;
    } else {
        velocity.x = 0.0f;  // Stop horizontal movement
    }
    
    // Test Z movement
    glm::vec3 testPosZ = glm::vec3(resolvedPos.x, oldPos.y, newPos.z);
    if (!checkCollision(testPosZ, chunks)) {
        resolvedPos.z = newPos.z;
    } else {
        velocity.z = 0.0f;  // Stop horizontal movement
    }
    
    // Test Y movement (gravity/jumping)
    glm::vec3 testPosY = glm::vec3(resolvedPos.x, newPos.y, resolvedPos.z);
    if (!checkCollision(testPosY, chunks)) {
        resolvedPos.y = newPos.y;
    } else {
        if (velocity.y < 0.0f) {
            // Falling - hit ground
            isOnGround = true;
        }
        velocity.y = 0.0f;  // Stop vertical movement
    }
    
    return resolvedPos;
}

// ============================================================================
// CHECK COLLISION WITH PLAYER BOUNDING BOX
// ============================================================================
bool Player::checkCollision(glm::vec3 newPosition, const std::vector<VoxelChunk>& chunks) {
    // Player bounding box corners (centered on position)
    glm::vec3 minBounds = newPosition - glm::vec3(size.x * 0.5f, 0.0f, size.z * 0.5f);
    glm::vec3 maxBounds = newPosition + glm::vec3(size.x * 0.5f, size.y, size.z * 0.5f);
    
    // Check all blocks that could intersect with player bounding box
    for (int x = (int)floor(minBounds.x); x <= (int)floor(maxBounds.x); x++) {
        for (int y = (int)floor(minBounds.y); y <= (int)floor(maxBounds.y); y++) {
            for (int z = (int)floor(minBounds.z); z <= (int)floor(maxBounds.z); z++) {
                if (isBlockSolid(x, y, z, chunks)) {
                    return true;  // Collision detected
                }
            }
        }
    }
    
    return false;  // No collision
}

// ============================================================================
// CHECK IF A BLOCK IS SOLID AT WORLD COORDINATES
// ============================================================================
bool Player::isBlockSolid(int worldX, int worldY, int worldZ, const std::vector<VoxelChunk>& chunks) {
    // Check bounds
    if (worldY < 0) {
        return true;  // Below world is solid
    }
    if (worldY >= VoxelChunk::CHUNK_SIZE) {
        return false; // Above world is air
    }
    
    // Calculate which chunk contains this world position
    int chunkX = worldX / VoxelChunk::CHUNK_SIZE;
    int chunkZ = worldZ / VoxelChunk::CHUNK_SIZE;
    
    // Handle negative coordinates properly
    if (worldX < 0) chunkX = (worldX + 1) / VoxelChunk::CHUNK_SIZE - 1;
    if (worldZ < 0) chunkZ = (worldZ + 1) / VoxelChunk::CHUNK_SIZE - 1;
    
    // Local coordinates within chunk
    int localX = worldX - chunkX * VoxelChunk::CHUNK_SIZE;
    int localZ = worldZ - chunkZ * VoxelChunk::CHUNK_SIZE;
    
    // Find the corresponding chunk
    for (const auto& chunk : chunks) {
        if (chunk.getWorldX() == chunkX && chunk.getWorldZ() == chunkZ) {
            return chunk.isBlockSolid(localX, worldY, localZ);
        }
    }
    
    // Chunk not found - assume air
    return false;
}

// ============================================================================
// UPDATE GROUND STATE FOR NEXT FRAME
// ============================================================================
void Player::updateGroundState(const std::vector<VoxelChunk>& chunks) {
    // Check if player is standing on ground (slightly below current position)
    glm::vec3 groundCheckPos = position - glm::vec3(0.0f, 0.1f, 0.0f);
    isOnGround = checkCollision(groundCheckPos, chunks);
}
