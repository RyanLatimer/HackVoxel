// ============================================================================
// SYSTEM AND LIBRARY INCLUDES
// ============================================================================
#include <glad/gl.h>                    // OpenGL function loader
#include <GLFW/glfw3.h>                 // Window and input management
#include <glm/glm.hpp>                  // Math library for graphics
#include <glm/gtc/matrix_transform.hpp> // Matrix transformation functions
#include <glm/gtc/type_ptr.hpp>         // Type conversion utilities
#include <iostream>                     // Standard I/O for error messages

// ============================================================================
// PROJECT INCLUDES
// ============================================================================
#include "shader.h"                     // Shader compilation utilities
#include "camera.h"                     // Camera management system

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

// ============================================================================
// GLOBAL VARIABLES FOR TIMING AND CAMERA
// ============================================================================
float deltaTime = 0.0f;    // Time between current frame and last frame
float lastFrame = 0.0f;    // Time of last frame
Camera camera;             // Main camera instance

// ============================================================================
// MOUSE CALLBACK FUNCTION
// Routes mouse movement to the camera system
// ============================================================================
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    camera.processMouseMovement(xpos, ypos);
}

// ============================================================================
// VERTEX SHADER SOURCE CODE
// Handles vertex positioning and color passing to fragment shader
// ============================================================================
const char* vertexSrc = R"(
#version 330 core
layout (location = 0) in vec3 aPos;        // Vertex position attribute
layout (location = 1) in vec3 aColor;      // Vertex color attribute

out vec3 fColor;                            // Color output to fragment shader

uniform mat4 model;                         // Model transformation matrix
uniform mat4 view;                          // View transformation matrix  
uniform mat4 projection;                    // Projection transformation matrix

void main() {
    fColor = aColor;                        // Pass color to fragment shader
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)";

// ============================================================================
// FRAGMENT SHADER SOURCE CODE
// Handles final pixel color output
// ============================================================================
const char* fragmentSrc = R"(
#version 330 core
in vec3 fColor;                             // Color input from vertex shader
out vec4 FragColor;                         // Final pixel color output

void main() {
    FragColor = vec4(fColor, 1.0);          // Set pixel color with full opacity
}
)";

// ============================================================================
// MAIN FUNCTION - APPLICATION ENTRY POINT
// ============================================================================
int main() {
    // ========================================================================
    // STEP 1: INITIALIZE GLFW LIBRARY
    // ========================================================================
    if (!glfwInit()) {
        std::cerr << "ERROR: Failed to initialize GLFW library" << std::endl;
        return -1;
    }
    std::cout << "SUCCESS: GLFW initialized successfully" << std::endl;

    // ========================================================================
    // STEP 2: CONFIGURE OPENGL CONTEXT VERSION AND PROFILE
    // ========================================================================
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);  // OpenGL 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    std::cout << "INFO: OpenGL context set to version 3.3 core profile" << std::endl;

    // ========================================================================
    // STEP 3: CREATE WINDOW AND OPENGL CONTEXT
    // ========================================================================
    GLFWwindow* window = glfwCreateWindow(800, 600, "HackVoxel - 3D Voxel Renderer", nullptr, nullptr);
    if (!window) {
        std::cerr << "ERROR: Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    std::cout << "SUCCESS: Window created (800x600)" << std::endl;

    // ========================================================================
    // STEP 4: CONFIGURE MOUSE INPUT FOR CAMERA CONTROL
    // ========================================================================
    glfwSetCursorPosCallback(window, mouse_callback);           // Set mouse callback
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // Hide and capture cursor
    std::cout << "INFO: Mouse input configured for camera control" << std::endl;

    // ========================================================================
    // STEP 5: LOAD OPENGL FUNCTIONS VIA GLAD
    // ========================================================================
    if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) {
        std::cerr << "ERROR: Failed to initialize GLAD OpenGL loader" << std::endl;
        return -1;
    }
    std::cout << "SUCCESS: OpenGL functions loaded via GLAD" << std::endl;

    // ========================================================================
    // STEP 6: CONFIGURE OPENGL RENDERING STATE
    // ========================================================================
    glEnable(GL_DEPTH_TEST);                                    // Enable depth testing for 3D
    glViewport(0, 0, 800, 600);                                // Set viewport size
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback); // Set resize callback
    std::cout << "INFO: OpenGL depth testing enabled, viewport configured" << std::endl;

    // ========================================================================
    // STEP 7: CREATE AND CONFIGURE VERTEX BUFFER OBJECTS
    // ========================================================================
    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);                                 // Generate Vertex Array Object
    glGenBuffers(1, &VBO);                                      // Generate Vertex Buffer Object
    glGenBuffers(1, &EBO);                                      // Generate Element Buffer Object

    // Bind and upload vertex data
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    // Bind and upload index data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    std::cout << "INFO: Vertex and index buffers created and uploaded" << std::endl;

    // ========================================================================
    // STEP 8: CONFIGURE VERTEX ATTRIBUTES
    // ========================================================================
    // Position attribute (location = 0): 3 floats starting at offset 0
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Color attribute (location = 1): 3 floats starting at offset 3
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    std::cout << "INFO: Vertex attributes configured (position + color)" << std::endl;

    // ========================================================================
    // STEP 9: COMPILE AND LINK SHADER PROGRAM
    // ========================================================================
    GLuint shaderProgram = createShader(vertexSrc, fragmentSrc);
    if (shaderProgram == 0) {
        std::cerr << "ERROR: Failed to create shader program" << std::endl;
        return -1;
    }
    std::cout << "SUCCESS: Shader program compiled and linked" << std::endl;

    // ========================================================================
    // STEP 10: MAIN RENDER LOOP
    // ========================================================================
    std::cout << "INFO: Entering main render loop..." << std::endl;
    while (!glfwWindowShouldClose(window)) {
        
        // ====================================================================
        // FRAME TIMING CALCULATIONS
        // ====================================================================
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // ====================================================================
        // PROCESS USER INPUT
        // ====================================================================
        camera.processKeyboard(window, deltaTime);

        // ====================================================================
        // CLEAR SCREEN AND DEPTH BUFFER
        // ====================================================================
        glClearColor(0.1f, 0.1f, 0.2f, 1.0f);                  // Dark blue background
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // ====================================================================
        // CALCULATE TRANSFORMATION MATRICES
        // ====================================================================
        glm::mat4 model = glm::mat4(1.0f);                      // Identity matrix (no transformation)
        glm::mat4 view = camera.getViewMatrix();                // Camera view matrix
        glm::mat4 projection = glm::perspective(glm::radians(camera.zoom), 800.0f/600.0f, 0.1f, 100.0f);

        // ====================================================================
        // UPLOAD MATRICES TO SHADER UNIFORMS
        // ====================================================================
        glUseProgram(shaderProgram);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        // ====================================================================
        // RENDER THE CUBE
        // ====================================================================
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);   // Draw 36 indices (12 triangles)

        // ====================================================================
        // SWAP BUFFERS AND POLL EVENTS
        // ====================================================================
        glfwSwapBuffers(window);                                // Display rendered frame
        glfwPollEvents();                                       // Process window events
    }

    // ========================================================================
    // STEP 11: CLEANUP AND SHUTDOWN
    // ========================================================================
    std::cout << "INFO: Cleaning up resources..." << std::endl;
    
    // Delete OpenGL objects
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);
    
    // Terminate GLFW
    glfwDestroyWindow(window);
    glfwTerminate();
    
    std::cout << "SUCCESS: Application terminated cleanly" << std::endl;
    return 0;
}
