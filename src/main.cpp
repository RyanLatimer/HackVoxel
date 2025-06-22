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

// Global variables
ChunkManager chunkManager;
TextureAtlas* textureAtlas = nullptr;
Skybox* skybox = nullptr;
float timeOfDay = 0.5f; // 0.0 = midnight, 0.5 = noon, 1.0 = midnight

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

// Updated shader sources with texture support
const char *vertexSrc = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

uniform mat4 model, view, projection;

out vec2 TexCoord;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    TexCoord = aTexCoord;
}
)";

const char *fragmentSrc = R"(
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
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) {
        std::cerr << "Failed to load OpenGL" << std::endl;
        return -1;
    }
    std::cout << "OpenGL loaded" << std::endl;
      glEnable(GL_DEPTH_TEST);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);    // Create texture atlas
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
        
        // Update skybox animation
        skybox->update(deltaTime);
        
        // Update time of day (slow cycle for demonstration)
        timeOfDay += deltaTime * 0.01f; // Very slow day/night cycle
        if (timeOfDay > 1.0f) timeOfDay = 0.0f;
        
        // Set clear color based on time of day for better atmosphere
        glm::vec3 horizonColor;
        if (timeOfDay < 0.25f || timeOfDay > 0.75f) {
            // Night
            horizonColor = glm::vec3(0.02f, 0.02f, 0.1f);
        } else if (timeOfDay < 0.35f || timeOfDay > 0.65f) {
            // Sunrise/Sunset
            horizonColor = glm::vec3(0.4f, 0.2f, 0.3f);
        } else {
            // Day
            horizonColor = glm::vec3(0.6f, 0.8f, 1.0f);
        }
        
        // Clear with atmospheric color
        glClearColor(horizonColor.r, horizonColor.g, horizonColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);        // Setup matrices
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
    glDeleteProgram(shaderProgram);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
