// ============================================================================
// SYSTEM AND LIBRARY INCLUDES
// ============================================================================
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

// ============================================================================
// PROJECT INCLUDES
// ============================================================================
#include "shader.h"
#include "camera.h"
#include "voxel_chunk.h"
#include "player.h"
#include "texture_atlas.h"
#include "chunk_manager.h"
#include "skybox.h"
#include "water_shader.h"
#include "ui.h"
#include "block_interaction.h"

// Global variables
ChunkManager chunkManager;
TextureAtlas* textureAtlas = nullptr;
Skybox* skybox = nullptr;
WaterShader* waterShader = nullptr;
UI* gameUI = nullptr;
BlockInteraction* blockInteraction = nullptr;
float timeOfDay = 0.5f; // 0.0 = midnight, 0.5 = noon, 1.0 = midnight

// Mouse state tracking for click detection
bool leftMousePressed = false;
bool rightMousePressed = false;
bool leftMouseJustPressed = false;
bool rightMouseJustPressed = false;

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// Timing
float deltaTime = 0.0f, lastFrame = 0.0f;
Camera camera;
Player player;

void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
    camera.processMouseMovement(xpos, ypos);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            leftMousePressed = true;
            leftMouseJustPressed = true;
        } else if (action == GLFW_RELEASE) {
            leftMousePressed = false;
        }
    }
    
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) {
            rightMousePressed = true;
            rightMouseJustPressed = true;
        } else if (action == GLFW_RELEASE) {
            rightMousePressed = false;
        }
    }
    
    // Middle mouse button - block picking (like Minecraft)
    if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS) {
        if (blockInteraction && gameUI) {
            RaycastHit hit = blockInteraction->raycastToBlock(camera, chunkManager);
            if (hit.hit) {
                BlockType targetedBlock = chunkManager.getBlockType(hit.blockPosition);
                if (targetedBlock != BlockType::AIR) {
                    gameUI->setSelectedBlockType(targetedBlock);
                    std::cout << "Picked block: " << gameUI->getBlockName(targetedBlock) << std::endl;
                }
            }
        }
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    if (gameUI && !gameUI->isInventoryOpen()) {
        // Only handle hotbar scrolling when inventory is closed
        gameUI->handleScrollInput(yoffset);
    }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS && gameUI) {
        // Handle number keys for hotbar selection
        if (key >= GLFW_KEY_1 && key <= GLFW_KEY_9) {
            gameUI->handleNumberKey(key - GLFW_KEY_0);
        }
        
        // Toggle inventory with E key (like Minecraft)
        if (key == GLFW_KEY_E) {
            gameUI->toggleInventory();
        }
        
        // Toggle inventory with I key (alternative)
        if (key == GLFW_KEY_I) {
            gameUI->toggleInventory();
        }
        
        // F3 for debug info toggle (placeholder for future implementation)
        if (key == GLFW_KEY_F3) {
            std::cout << "Debug info toggled (not implemented yet)" << std::endl;
        }
    }
}

// Enhanced atmospheric color calculation for smooth transitions
glm::vec3 calculateAtmosphericColor(float timeOfDay) {
    // Smooth interpolation between different times of day
    glm::vec3 deepNight = glm::vec3(0.01f, 0.01f, 0.08f);      // Deep blue night
    glm::vec3 earlyDawn = glm::vec3(0.15f, 0.1f, 0.25f);       // Purple dawn
    glm::vec3 sunrise = glm::vec3(0.8f, 0.4f, 0.2f);          // Orange sunrise
    glm::vec3 morning = glm::vec3(0.6f, 0.8f, 1.0f);          // Blue morning
    glm::vec3 midday = glm::vec3(0.5f, 0.7f, 1.0f);           // Bright day
    glm::vec3 evening = glm::vec3(0.7f, 0.6f, 0.9f);          // Purple evening
    glm::vec3 sunset = glm::vec3(1.0f, 0.3f, 0.1f);           // Red sunset
    glm::vec3 dusk = glm::vec3(0.2f, 0.1f, 0.3f);             // Purple dusk
    
    // Smooth interpolation based on time
    if (timeOfDay < 0.1f) {
        // Deep night to early dawn
        float t = timeOfDay / 0.1f;
        return glm::mix(deepNight, earlyDawn, glm::smoothstep(0.0f, 1.0f, t));
    } else if (timeOfDay < 0.2f) {
        // Early dawn to sunrise
        float t = (timeOfDay - 0.1f) / 0.1f;
        return glm::mix(earlyDawn, sunrise, glm::smoothstep(0.0f, 1.0f, t));
    } else if (timeOfDay < 0.3f) {
        // Sunrise to morning
        float t = (timeOfDay - 0.2f) / 0.1f;
        return glm::mix(sunrise, morning, glm::smoothstep(0.0f, 1.0f, t));
    } else if (timeOfDay < 0.5f) {
        // Morning to midday
        float t = (timeOfDay - 0.3f) / 0.2f;
        return glm::mix(morning, midday, glm::smoothstep(0.0f, 1.0f, t));
    } else if (timeOfDay < 0.7f) {
        // Midday to evening
        float t = (timeOfDay - 0.5f) / 0.2f;
        return glm::mix(midday, evening, glm::smoothstep(0.0f, 1.0f, t));
    } else if (timeOfDay < 0.8f) {
        // Evening to sunset
        float t = (timeOfDay - 0.7f) / 0.1f;
        return glm::mix(evening, sunset, glm::smoothstep(0.0f, 1.0f, t));
    } else if (timeOfDay < 0.9f) {
        // Sunset to dusk
        float t = (timeOfDay - 0.8f) / 0.1f;
        return glm::mix(sunset, dusk, glm::smoothstep(0.0f, 1.0f, t));
    } else {
        // Dusk to deep night
        float t = (timeOfDay - 0.9f) / 0.1f;
        return glm::mix(dusk, deepNight, glm::smoothstep(0.0f, 1.0f, t));
    }
}

// Updated shader sources with texture support
const char *vertexSrc = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

uniform mat4 model, view, projection;

out vec2 TexCoord;
out vec3 Normal;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    TexCoord = aTexCoord;
    Normal = aNormal;
}
)";

const char *fragmentSrc = R"(
#version 330 core
in vec2 TexCoord;
in vec3 Normal;
out vec4 FragColor;

uniform sampler2D ourTexture;
uniform vec3 lightDirection; // world-space dir pointing *toward* light

void main() {
    vec3 norm = normalize(Normal);
    float diff = max(dot(norm, -lightDirection), 0.3); // basic lambert + ambient floor
    vec4 texColor = texture(ourTexture, TexCoord);

    // Water transparency heuristic
    if (texColor.r < 0.2 && texColor.g > 0.3 && texColor.g < 0.7 && texColor.b > 0.7) {
        FragColor = vec4(texColor.rgb * diff, 0.6);
    } else {
        FragColor = vec4(texColor.rgb * diff, texColor.a);
    }
}
)";

const char *fragmentSrc_old = R"(
#version 330 core
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D ourTexture;

void main() { 
    FragColor = texture(ourTexture, TexCoord);
}
)";

// ============================================================================
// MAIN FUNCTION - APPLICATION ENTRY POINT
// ============================================================================
int main()
{
    // Init GLFW first
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    std::cout << "GLFW initialized" << std::endl;
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow *window = glfwCreateWindow(800, 600, "HackVoxel", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create window" << std::endl;
        glfwTerminate();
        return -1;
    }
    std::cout << "Window created" << std::endl;
    
    glfwMakeContextCurrent(window);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) {
        std::cerr << "Failed to load OpenGL" << std::endl;
        return -1;
    }
    std::cout << "OpenGL loaded" << std::endl;    glEnable(GL_DEPTH_TEST);
    
    // Enable blending for transparency (water blocks)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);// Create texture atlas
    textureAtlas = new TextureAtlas();
    if (!textureAtlas->initialize()) {
        std::cerr << "Failed to initialize texture atlas!" << std::endl;
        return -1;
    }    VoxelChunk::textureAtlas = textureAtlas; // Set static reference
    std::cout << "Texture atlas created successfully" << std::endl;    // Initialize skybox
    skybox = new Skybox();
    if (!skybox->initialize()) {
        std::cerr << "Failed to initialize skybox!" << std::endl;
        return -1;
    }
    std::cout << "Skybox created successfully" << std::endl;
    
    // Initialize water shader
    waterShader = new WaterShader();
    if (!waterShader->initialize()) {
        std::cerr << "Failed to initialize water shader!" << std::endl;
        return -1;
    }
    std::cout << "Water shader created successfully" << std::endl;
    
    // Initialize UI system
    gameUI = new UI();
    if (!gameUI->initialize()) {
        std::cerr << "Failed to initialize UI system!" << std::endl;
        return -1;
    }
    std::cout << "UI system created successfully" << std::endl;
    
    // Initialize block interaction system
    blockInteraction = new BlockInteraction();
    if (!blockInteraction->initialize()) {
        std::cerr << "Failed to initialize block interaction system!" << std::endl;
        return -1;
    }
    std::cout << "Block interaction system created successfully" << std::endl;
    
    // Initialize chunk manager (infinite world system)
    std::cout << "Initializing chunk manager for infinite world..." << std::endl;
    chunkManager.initialize(player.position);
    
    // Compile shaders
    GLuint shaderProgram = createShader(vertexSrc, fragmentSrc);
    if (shaderProgram == 0) {
        std::cerr << "Failed to create shader program!" << std::endl;
        return -1;
    }
    std::cout << "Shader program created successfully" << std::endl;    std::cout << "Starting render loop..." << std::endl;

    while (!glfwWindowShouldClose(window))
    {        // Frame timing
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
          // Update chunk manager based on player position
        chunkManager.update(player.position);
        
        // Update player physics and input (this will also update camera position)
        player.update(deltaTime, window, camera, chunkManager);
        
        // Handle block interaction
        if (blockInteraction && gameUI) {
            RaycastHit hit = blockInteraction->raycastToBlock(camera, chunkManager);
            
            // Update UI with targeted block info
            if (hit.hit) {
                BlockType targetedBlock = chunkManager.getBlockType(hit.blockPosition);
                gameUI->setTargetedBlock(targetedBlock, hit.blockPosition);
            } else {
                gameUI->clearTargetedBlock();
            }
            
            // Handle left mouse click (mine block)
            if (leftMouseJustPressed && hit.hit) {
                // Determine block type before mining so we can give it to the player
                BlockType targetType = chunkManager.getBlockType(hit.blockPosition);
                if (blockInteraction->mineBlock(hit.blockPosition, chunkManager)) {
                    // Add the mined block to the player's inventory (similar to Minecraft)
                    gameUI->addBlockToInventory(targetType);
                }
                leftMouseJustPressed = false;
            }
            
            // Handle right mouse click (place block)
            if (rightMouseJustPressed && hit.hit) {
                glm::vec3 placePos = blockInteraction->getPlacementPosition(hit);
                BlockType selectedBlock = gameUI->getSelectedBlockType();
                // Only place a block if the player actually has one in their inventory
                if (selectedBlock != BlockType::AIR && gameUI->removeBlockFromInventory(selectedBlock)) {
                    blockInteraction->placeBlock(placePos, selectedBlock, chunkManager);
                } else {
                    std::cout << "No more blocks of this type in inventory!" << std::endl;
                }
                rightMouseJustPressed = false;
            }
        }
        
        // Reset mouse state
        leftMouseJustPressed = false;
        rightMouseJustPressed = false;
        
        // Update skybox animation
        skybox->update(deltaTime);
        
        // Update time of day with smoother, more realistic cycle
        timeOfDay += deltaTime * 0.005f; // Slower, more cinematic cycle
        if (timeOfDay > 1.0f) timeOfDay = 0.0f;
        
        // Enhanced atmospheric color calculation with smooth transitions
        glm::vec3 horizonColor = calculateAtmosphericColor(timeOfDay);
        
        // Clear with dynamic atmospheric color
        glClearColor(horizonColor.r, horizonColor.g, horizonColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);// Setup matrices
        glm::mat4 view = camera.getViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.zoom), 800.0f / 600.0f, 0.1f, 1000.0f);
        
        // Render skybox first (before terrain)
        skybox->render(view, projection, timeOfDay);
        
        // Use shader program for terrain
        glUseProgram(shaderProgram);
        
        // Bind texture atlas
        textureAtlas->bind(0);
        glUniform1i(glGetUniformLocation(shaderProgram, "ourTexture"), 0);

        // Render chunks using ChunkManager
        chunkManager.render(shaderProgram, player.position, view, projection);
        
        // Render block highlight if targeting a block
        if (blockInteraction) {
            RaycastHit hit = blockInteraction->raycastToBlock(camera, chunkManager);
            if (hit.hit) {
                blockInteraction->renderBlockHighlight(hit, shaderProgram, view, projection);
            }
        }
        
        // Render UI
        if (gameUI) {
            int windowWidth, windowHeight;
            glfwGetWindowSize(window, &windowWidth, &windowHeight);
            gameUI->render(windowWidth, windowHeight);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
        
        // Check for OpenGL errors
        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            std::cerr << "OpenGL error: " << error << std::endl;
        }
    }
    
    std::cout << "Exiting render loop..." << std::endl;    // Cleanup
    delete textureAtlas;
    delete skybox;
    delete waterShader;
    delete gameUI;
    delete blockInteraction;
    glDeleteProgram(shaderProgram);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
