#include "block_interaction.h"
#include <iostream>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Highlight shader sources
const char* highlightVertexShader = R"(
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)";

const char* highlightFragmentShader = R"(
#version 330 core
out vec4 FragColor;

void main() {
    FragColor = vec4(1.0, 1.0, 1.0, 0.8);  // White outline like Minecraft
}
)";

BlockInteraction::BlockInteraction() {
    highlightVAO = 0;
    highlightVBO = 0;
    highlightEBO = 0;
    highlightShaderProgram = 0;
}

bool BlockInteraction::initialize() {
    // Create highlight shader
    highlightShaderProgram = createHighlightShader();
    if (highlightShaderProgram == 0) {
        std::cerr << "Failed to create highlight shader" << std::endl;
        return false;
    }
    
    // Initialize highlight rendering
    initializeHighlight();
    
    std::cout << "Block interaction system initialized" << std::endl;
    return true;
}

void BlockInteraction::cleanup() {
    if (highlightVAO) glDeleteVertexArrays(1, &highlightVAO);
    if (highlightVBO) glDeleteBuffers(1, &highlightVBO);
    if (highlightEBO) glDeleteBuffers(1, &highlightEBO);
    if (highlightShaderProgram) glDeleteProgram(highlightShaderProgram);
}

RaycastHit BlockInteraction::raycastToBlock(const Camera& camera, ChunkManager& chunkManager, float maxDistance) {
    RaycastHit hit;
    
    glm::vec3 rayOrigin = camera.position;
    glm::vec3 rayDirection = camera.front;
    
    // Step size for ray marching (smaller = more accurate, slower)
    float stepSize = 0.1f;
    float currentDistance = 0.0f;
    
    while (currentDistance < maxDistance) {
        glm::vec3 currentPos = rayOrigin + rayDirection * currentDistance;
        
        // Test if there's a solid block at this position
        if (testBlockAt(currentPos, chunkManager)) {
            hit.hit = true;
            hit.blockPosition = glm::floor(currentPos);
            hit.distance = currentDistance;
            
            // Calculate face normal based on which face was hit
            hit.normal = calculateFaceNormal(rayDirection, hit.blockPosition, currentPos);
            
            break;
        }
        
        currentDistance += stepSize;
    }
    
    return hit;
}

bool BlockInteraction::placeBlock(const glm::vec3& position, BlockType blockType, ChunkManager& chunkManager) {
    // Convert world position to chunk coordinates and local block coordinates
    int chunkX = (int)floor(position.x / VoxelChunk::CHUNK_SIZE);
    int chunkZ = (int)floor(position.z / VoxelChunk::CHUNK_SIZE);
    
    // Get local coordinates within the chunk
    int localX = (int)position.x - (chunkX * VoxelChunk::CHUNK_SIZE);
    int localY = (int)position.y;
    int localZ = (int)position.z - (chunkZ * VoxelChunk::CHUNK_SIZE);
    
    // Handle negative coordinates properly
    if (position.x < 0 && localX != 0) {
        chunkX--;
        localX = VoxelChunk::CHUNK_SIZE + localX;
    }
    if (position.z < 0 && localZ != 0) {
        chunkZ--;
        localZ = VoxelChunk::CHUNK_SIZE + localZ;
    }
    
    // Get the chunk
    VoxelChunk* chunk = chunkManager.getChunkAt(chunkX, chunkZ);
    if (!chunk) {
        return false; // Chunk not loaded
    }
    
    // Check bounds and if position is empty
    if (localX < 0 || localX >= VoxelChunk::CHUNK_SIZE ||
        localY < 0 || localY >= VoxelChunk::CHUNK_SIZE ||
        localZ < 0 || localZ >= VoxelChunk::CHUNK_SIZE) {
        return false;
    }
    
    // Don't place block if position is already occupied
    if (chunk->isBlockSolid(localX, localY, localZ)) {
        return false;
    }
    
    // Place the block
    chunk->setBlock(localX, localY, localZ, blockType);
    chunk->regenerateMesh();
    
    return true;
}

bool BlockInteraction::mineBlock(const glm::vec3& position, ChunkManager& chunkManager) {
    // Convert world position to chunk coordinates and local block coordinates
    int chunkX = (int)floor(position.x / VoxelChunk::CHUNK_SIZE);
    int chunkZ = (int)floor(position.z / VoxelChunk::CHUNK_SIZE);
    
    // Get local coordinates within the chunk
    int localX = (int)position.x - (chunkX * VoxelChunk::CHUNK_SIZE);
    int localY = (int)position.y;
    int localZ = (int)position.z - (chunkZ * VoxelChunk::CHUNK_SIZE);
    
    // Handle negative coordinates properly
    if (position.x < 0 && localX != 0) {
        chunkX--;
        localX = VoxelChunk::CHUNK_SIZE + localX;
    }
    if (position.z < 0 && localZ != 0) {
        chunkZ--;
        localZ = VoxelChunk::CHUNK_SIZE + localZ;
    }
    
    // Get the chunk
    VoxelChunk* chunk = chunkManager.getChunkAt(chunkX, chunkZ);
    if (!chunk) {
        return false; // Chunk not loaded
    }
    
    // Check bounds
    if (localX < 0 || localX >= VoxelChunk::CHUNK_SIZE ||
        localY < 0 || localY >= VoxelChunk::CHUNK_SIZE ||
        localZ < 0 || localZ >= VoxelChunk::CHUNK_SIZE) {
        return false;
    }
    
    // Don't mine air or bedrock
    BlockType blockType = chunk->getBlockType(localX, localY, localZ);
    if (blockType == BlockType::AIR || blockType == BlockType::BEDROCK) {
        return false;
    }
    
    // Mine the block (set to air)
    chunk->setBlock(localX, localY, localZ, BlockType::AIR);
    chunk->regenerateMesh();
    
    return true;
}

glm::vec3 BlockInteraction::getPlacementPosition(const RaycastHit& hit) {
    // Place block adjacent to the hit block in the direction of the face normal
    return hit.blockPosition + hit.normal;
}

void BlockInteraction::renderBlockHighlight(const RaycastHit& hit, GLuint shaderProgram, 
                                           const glm::mat4& view, const glm::mat4& projection) {
    if (!hit.hit) return;
    
    glUseProgram(highlightShaderProgram);
    
    // Set up matrices
    glm::mat4 model = glm::translate(glm::mat4(1.0f), hit.blockPosition);
    model = glm::scale(model, glm::vec3(1.01f)); // Slightly larger than block to avoid z-fighting
    
    glUniformMatrix4fv(glGetUniformLocation(highlightShaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(highlightShaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(highlightShaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    
    // Render wireframe highlight
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glLineWidth(2.0f);
    
    glBindVertexArray(highlightVAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    
    // Reset polygon mode
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

bool BlockInteraction::testBlockAt(const glm::vec3& worldPos, ChunkManager& chunkManager) {
    return chunkManager.isBlockSolid(worldPos);
}

glm::vec3 BlockInteraction::calculateFaceNormal(const glm::vec3& rayDir, const glm::vec3& blockPos, const glm::vec3& hitPos) {
    // Calculate which face of the block was hit based on the hit position relative to block center
    glm::vec3 blockCenter = blockPos + glm::vec3(0.5f);
    glm::vec3 localHit = hitPos - blockCenter;
    
    // Find the axis with the largest absolute component
    glm::vec3 abs_local = glm::abs(localHit);
    
    if (abs_local.x > abs_local.y && abs_local.x > abs_local.z) {
        // Hit X face
        return glm::vec3(localHit.x > 0 ? 1.0f : -1.0f, 0.0f, 0.0f);
    } else if (abs_local.y > abs_local.z) {
        // Hit Y face
        return glm::vec3(0.0f, localHit.y > 0 ? 1.0f : -1.0f, 0.0f);
    } else {
        // Hit Z face
        return glm::vec3(0.0f, 0.0f, localHit.z > 0 ? 1.0f : -1.0f);
    }
}

GLuint BlockInteraction::createHighlightShader() {
    // Compile vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &highlightVertexShader, NULL);
    glCompileShader(vertexShader);
    
    // Check for compilation errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << "Highlight vertex shader compilation failed: " << infoLog << std::endl;
        return 0;
    }
    
    // Compile fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &highlightFragmentShader, NULL);
    glCompileShader(fragmentShader);
    
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cerr << "Highlight fragment shader compilation failed: " << infoLog << std::endl;
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
        std::cerr << "Highlight shader program linking failed: " << infoLog << std::endl;
        return 0;
    }
    
    // Clean up shaders
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    return shaderProgram;
}

void BlockInteraction::initializeHighlight() {
    // Cube vertices for highlighting
    float vertices[] = {
        // Front face
        0.0f, 0.0f, 1.0f,  1.0f, 0.0f, 1.0f,  1.0f, 1.0f, 1.0f,  0.0f, 1.0f, 1.0f,
        // Back face
        1.0f, 0.0f, 0.0f,  0.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f, 0.0f,
        // Left face
        0.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f, 1.0f,  0.0f, 1.0f, 0.0f,
        // Right face
        1.0f, 0.0f, 1.0f,  1.0f, 0.0f, 0.0f,  1.0f, 1.0f, 0.0f,  1.0f, 1.0f, 1.0f,
        // Bottom face
        0.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f,
        // Top face
        0.0f, 1.0f, 1.0f,  1.0f, 1.0f, 1.0f,  1.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f
    };
    
    unsigned int indices[] = {
        0,  1,  2,   2,  3,  0,   // front
        4,  5,  6,   6,  7,  4,   // back
        8,  9,  10,  10, 11, 8,   // left
        12, 13, 14,  14, 15, 12,  // right
        16, 17, 18,  18, 19, 16,  // bottom
        20, 21, 22,  22, 23, 20   // top
    };
    
    glGenVertexArrays(1, &highlightVAO);
    glGenBuffers(1, &highlightVBO);
    glGenBuffers(1, &highlightEBO);
    
    glBindVertexArray(highlightVAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, highlightVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, highlightEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glBindVertexArray(0);
}
