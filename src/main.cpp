#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include "shader.h"

float vertices[] = {
    // positions           // colors
    -0.5f, -0.5f,  0.5f,    1.0f, 0.0f, 0.0f,  // 0 front-bottom-left
     0.5f, -0.5f,  0.5f,    0.0f, 1.0f, 0.0f,  // 1 front-bottom-right
     0.5f,  0.5f,  0.5f,    0.0f, 0.0f, 1.0f,  // 2 front-top-right
    -0.5f,  0.5f,  0.5f,    1.0f, 1.0f, 0.0f,  // 3 front-top-left
    -0.5f, -0.5f, -0.5f,    1.0f, 0.0f, 1.0f,  // 4 back-bottom-left
     0.5f, -0.5f, -0.5f,    0.0f, 1.0f, 1.0f,  // 5 back-bottom-right
     0.5f,  0.5f, -0.5f,    1.0f, 1.0f, 1.0f,  // 6 back-top-right
    -0.5f,  0.5f, -0.5f,    0.0f, 0.0f, 0.0f   // 7 back-top-left
};

unsigned int indices[] = {
    // front face
    0, 1, 2,  2, 3, 0,
    // right face
    1, 5, 6,  6, 2, 1,
    // back face
    5, 4, 7,  7, 6, 5,
    // left face
    4, 0, 3,  3, 7, 4,
    // bottom face
    4, 5, 1,  1, 0, 4,
    // top face
    3, 2, 6,  6, 7, 3
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

int main() {
    if (!glfwInit())  {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "HackVoxel", nullptr, nullptr);

    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glViewport(0, 0, 800, 600);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // --- setup VAO/VBO/EBO & shaders -----------------------------------
    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // positions (3 floats) + colors (3 floats) ⇒ stride = 6 * sizeof(float)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    const char* vertexSrc = R"(
    #version 330 core
    layout(location = 0) in vec3 aPos;
    layout(location = 1) in vec3 aColor;
    out vec3 fColor;
    void main() {
        gl_Position = vec4(aPos, 1.0);
        fColor = aColor;
    }
    )";

    const char* fragmentSrc = R"(
    #version 330 core
    in vec3 fColor;
    out vec4 FragColor;
    void main() {
        FragColor = vec4(fColor, 1.0);
    }
    )";

    GLuint shader = createShader(vertexSrc, fragmentSrc);
   

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shader);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
