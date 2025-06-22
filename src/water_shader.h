#pragma once

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

/**
 * WaterShader handles animated water rendering with waves and transparency
 */
class WaterShader {
public:
    WaterShader();
    ~WaterShader();
    
    // Initialize the water shader system
    bool initialize();
    
    // Use the water shader for rendering
    void use();
    
    // Set shader uniforms
    void setMatrices(const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection);
    void setTime(float time);
    void setWaterLevel(float level);
    void setLightDirection(const glm::vec3& direction);
    
    // Get shader program ID
    GLuint getProgram() const { return shaderProgram; }
    
private:
    GLuint shaderProgram;
    
    // Uniform locations
    GLint modelLoc;
    GLint viewLoc;
    GLint projectionLoc;
    GLint timeLoc;
    GLint waterLevelLoc;
    GLint lightDirLoc;
    
    // Shader creation helper
    GLuint createShader(const char* vertexSource, const char* fragmentSource);
    void getUniformLocations();
};
