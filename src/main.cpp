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

// ============================================================================
// CUBE VERTEX DATA - POSITIONS AND COLORS
// Each vertex contains: X, Y, Z position followed by R, G, B color
// ============================================================================
float vertices[] = {
    // positions           // colors
    -0.5f, -0.5f,  0.5f,    1.0f, 0.0f, 0.0f,  // 0 front-bottom-left (red)
     0.5f, -0.5f,  0.5f,    0.0f, 1.0f, 0.0f,  // 1 front-bottom-right (green)
     0.5f,  0.5f,  0.5f,    0.0f, 0.0f, 1.0f,  // 2 front-top-right (blue)
    -0.5f,  0.5f,  0.5f,    1.0f, 1.0f, 0.0f,  // 3 front-top-left (yellow)
    -0.5f, -0.5f, -0.5f,    1.0f, 0.0f, 1.0f,  // 4 back-bottom-left (magenta)
     0.5f, -0.5f, -0.5f,    0.0f, 1.0f, 1.0f,  // 5 back-bottom-right (cyan)
     0.5f,  0.5f, -0.5f,    1.0f, 1.0f, 1.0f,  // 6 back-top-right (white)
    -0.5f,  0.5f, -0.5f,    0.0f, 0.0f, 0.0f   // 7 back-top-left (black)
};

// ============================================================================
// CUBE INDEX DATA - DEFINES TRIANGLES FOR EACH FACE
// Two triangles per face, 6 faces total = 12 triangles = 36 indices
// ============================================================================
unsigned int indices[] = {
    // front face (Z+)
    0, 1, 2,  2, 3, 0,
    // right face (X+)
    1, 5, 6,  6, 2, 1,
    // back face (Z-)
    5, 4, 7,  7, 6, 5,
    // left face (X-)
    4, 0, 3,  3, 7, 4,
    // bottom face (Y-)
    4, 5, 1,  1, 0, 4,
    // top face (Y+)
    3, 2, 6,  6, 7, 3
};

// ============================================================================
// WINDOW RESIZE CALLBACK
// Called whenever the window is resized to update the OpenGL viewport
// ============================================================================
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// Timing
float deltaTime = 0.0f, lastFrame = 0.0f;
Camera camera;

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    camera.processMouseMovement(xpos, ypos);
}

// Shader sources
const char* vertexSrc = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
out vec3 fColor;
uniform mat4 model, view, projection;
void main() {
    fColor = aColor;
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)";
const char* fragmentSrc = R"(
#version 330 core
in vec3 fColor;
out vec4 FragColor;
void main() { FragColor = vec4(fColor, 1.0); }
)";

// ============================================================================
// MAIN FUNCTION - APPLICATION ENTRY POINT
// ============================================================================
int main() {
    // Init GLFW
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "HackVoxel", nullptr, nullptr);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) return -1;
    glEnable(GL_DEPTH_TEST);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Setup cube data
    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    // Compile shaders
    GLuint shaderProgram = createShader(vertexSrc, fragmentSrc);

    while (!glfwWindowShouldClose(window)) {
        // Frame timing
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        // Input
        camera.processKeyboard(window, deltaTime);
        // Clear
        glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // Matrices
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = camera.getViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.zoom), 800.0f/600.0f, 0.1f, 100.0f);
        glUseProgram(shaderProgram);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        // Draw cubes in grid
        for (int x=0; x<10; ++x)
            for (int y=0; y<3; ++y)
                for (int z=0; z<10; ++z) {
                    glm::mat4 m = glm::translate(glm::mat4(1.0f), glm::vec3(x,y,z));
                    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(m));
                    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
                }
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
