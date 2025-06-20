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

std::vector<VoxelChunk> chunks;
int gridSize = 4;

// Function to create a procedural checkerboard texture
GLuint createCheckerboardTexture(int width = 64, int height = 64) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    // Create checkerboard pattern
    std::vector<unsigned char> data(width * height * 3);
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int index = (y * width + x) * 3;
            // Create 8x8 checkerboard pattern
            bool checker = ((x / 8) + (y / 8)) % 2;
            unsigned char color = checker ? 220 : 100; // Light gray vs dark gray
            data[index] = color;     // R
            data[index + 1] = color; // G  
            data[index + 2] = color; // B
        }
    }
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data.data());
    glGenerateMipmap(GL_TEXTURE_2D);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glBindTexture(GL_TEXTURE_2D, 0);
    return textureID;
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// Timing
float deltaTime = 0.0f, lastFrame = 0.0f;
Camera camera;

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
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Create texture
    GLuint texture = createCheckerboardTexture();
    std::cout << "Checkerboard texture created" << std::endl;

    // NOW initialize chunks after OpenGL is ready
    std::cout << "Creating chunks..." << std::endl;
    for (int x = -gridSize; x <= gridSize; x++)
    {
        for (int z = -gridSize; z <= gridSize; z++)
        {
            chunks.emplace_back(x, z);
        }
    }
    
    // Compile shaders
    GLuint shaderProgram = createShader(vertexSrc, fragmentSrc);
    if (shaderProgram == 0) {
        std::cerr << "Failed to create shader program!" << std::endl;
        return -1;
    }
    std::cout << "Shader program created successfully" << std::endl;
    std::cout << "Initialized " << chunks.size() << " chunks" << std::endl;
    std::cout << "Starting render loop..." << std::endl;

    while (!glfwWindowShouldClose(window))
    {
        // Frame timing
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        
        // Input
        camera.processKeyboard(window, deltaTime);
        
        // Clear
        glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Setup matrices once
        glm::mat4 view = camera.getViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.zoom), 800.0f / 600.0f, 0.1f, 100.0f);
        glUseProgram(shaderProgram);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        // Bind texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glUniform1i(glGetUniformLocation(shaderProgram, "ourTexture"), 0);

        // Render all chunks
        for (VoxelChunk &chunk : chunks)
        {
            chunk.render(shaderProgram);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
        
        // Check for OpenGL errors
        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            std::cerr << "OpenGL error: " << error << std::endl;
        }
    }
    
    std::cout << "Exiting render loop..." << std::endl;

    // Cleanup
    glDeleteTextures(1, &texture);
    glDeleteProgram(shaderProgram);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
