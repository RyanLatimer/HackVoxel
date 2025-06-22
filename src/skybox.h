#pragma once

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

/**
 * Skybox renders a beautiful procedural sky around the player
 */
class Skybox {
public:
    Skybox();
    ~Skybox();
    
    // Initialize the skybox (call after OpenGL context is ready)
    bool initialize();
    
    // Render the skybox
    void render(const glm::mat4& view, const glm::mat4& projection, float timeOfDay = 0.5f);
    
    // Update time-based effects
    void update(float deltaTime);
    
private:
    void createSkyboxMesh();
    void createSkyboxShaders();
    unsigned int createShader(const char* vertexSource, const char* fragmentSource);
    
    // Generate procedural sky colors based on time
    glm::vec3 getSkyColor(float timeOfDay, float height) const;
    glm::vec3 getHorizonColor(float timeOfDay) const;
    glm::vec3 getSunColor(float timeOfDay) const;
    
private:
    GLuint VAO, VBO;
    GLuint shaderProgram;
    float currentTime;
    
    // Vertex data for a cube
    std::vector<float> vertices;
};
