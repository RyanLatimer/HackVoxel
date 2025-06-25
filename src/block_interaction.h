#pragma once

#include <glad/gl.h>
#include <glm/glm.hpp>
#include "camera.h"
#include "chunk_manager.h"
#include "voxel_chunk.h"

struct RaycastHit {
    bool hit;
    glm::vec3 blockPosition;    // Position of the hit block
    glm::vec3 normal;           // Face normal of the hit
    float distance;             // Distance from ray origin
    
    RaycastHit() : hit(false), distance(0.0f) {}
};

class BlockInteraction {
public:
    BlockInteraction();
    
    // Raycast from camera to find block intersections
    RaycastHit raycastToBlock(const Camera& camera, ChunkManager& chunkManager, float maxDistance = 8.0f);
    
    // Place a block at the specified position
    bool placeBlock(const glm::vec3& position, BlockType blockType, ChunkManager& chunkManager);
    
    // Mine (remove) a block at the specified position
    bool mineBlock(const glm::vec3& position, ChunkManager& chunkManager);
    
    // Get the position where a new block should be placed (adjacent to hit block)
    glm::vec3 getPlacementPosition(const RaycastHit& hit);
    
    // Highlight the currently targeted block
    void renderBlockHighlight(const RaycastHit& hit, GLuint shaderProgram, 
                             const glm::mat4& view, const glm::mat4& projection);
    
    // Initialize highlighting system
    bool initialize();
    
    // Cleanup
    void cleanup();

private:
    // Step along ray and test for block intersections
    bool testBlockAt(const glm::vec3& worldPos, ChunkManager& chunkManager);
    
    // Get the face normal based on ray direction and hit position
    glm::vec3 calculateFaceNormal(const glm::vec3& rayDir, const glm::vec3& blockPos, const glm::vec3& hitPos);
    
    // OpenGL objects for block highlighting
    GLuint highlightVAO, highlightVBO, highlightEBO;
    GLuint highlightShaderProgram;
    
    // Create highlight shader
    GLuint createHighlightShader();
    
    // Initialize highlight rendering
    void initializeHighlight();
};
