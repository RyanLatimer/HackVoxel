#include "skybox.h"
#include <iostream>
#include <cmath>
#include <glm/gtc/type_ptr.hpp>

Skybox::Skybox() : VAO(0), VBO(0), shaderProgram(0), currentTime(0.0f) {
}

Skybox::~Skybox() {
    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (VBO) glDeleteBuffers(1, &VBO);
    if (shaderProgram) glDeleteProgram(shaderProgram);
}

bool Skybox::initialize() {
    createSkyboxMesh();
    createSkyboxShaders();
    
    std::cout << "Skybox initialized successfully" << std::endl;
    return true;
}

void Skybox::render(const glm::mat4& view, const glm::mat4& projection, float timeOfDay) {
    // Disable depth writing for skybox
    glDepthMask(GL_FALSE);
    
    glUseProgram(shaderProgram);
    
    // Remove translation from view matrix (keep only rotation)
    glm::mat4 skyboxView = glm::mat4(glm::mat3(view));
    
    // Set uniforms
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(skyboxView));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniform1f(glGetUniformLocation(shaderProgram, "timeOfDay"), timeOfDay);
    glUniform1f(glGetUniformLocation(shaderProgram, "currentTime"), currentTime);
    
    // Render skybox cube
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    
    // Re-enable depth writing
    glDepthMask(GL_TRUE);
}

void Skybox::update(float deltaTime) {
    currentTime += deltaTime * 0.1f; // Slow animation
}

void Skybox::createSkyboxMesh() {
    // Cube vertices for skybox
    vertices = {
        // Front face
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,
        
        // Back face
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        
        // Left face
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        
        // Right face
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        
        // Bottom face
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        
        // Top face
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f
    };
    
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glBindVertexArray(0);
}

void Skybox::createSkyboxShaders() {
    const char* vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        
        out vec3 WorldPos;
        
        uniform mat4 projection;
        uniform mat4 view;
        
        void main() {
            WorldPos = aPos;
            vec4 pos = projection * view * vec4(aPos, 1.0);
            gl_Position = pos.xyww; // Ensure skybox is always at far plane
        }
    )";
    
    const char* fragmentShaderSource = R"(
        #version 330 core
        out vec4 FragColor;
        
        in vec3 WorldPos;
        
        uniform float timeOfDay;
        uniform float currentTime;
        
        vec3 getSkyColor(vec3 direction) {
            float height = direction.y;
            float azimuth = atan(direction.z, direction.x);
            
            // Sun position based on time of day
            float sunAngle = timeOfDay * 2.0 * 3.14159;
            vec3 sunDir = vec3(cos(sunAngle), sin(sunAngle) * 0.6, sin(sunAngle));
            
            // Distance to sun
            float sunDist = dot(direction, sunDir);
              // Sky gradient colors with enhanced atmosphere
            vec3 skyTop, skyHorizon, sunColor;
            
            if (timeOfDay < 0.2 || timeOfDay > 0.8) {
                // Deep night with stars
                skyTop = vec3(0.01, 0.01, 0.15);
                skyHorizon = vec3(0.05, 0.05, 0.25);
                sunColor = vec3(0.9, 0.9, 1.0); // Bright moon
                
                // Add stars
                float starField = sin(direction.x * 100.0) * cos(direction.y * 100.0) * sin(direction.z * 100.0);
                if (starField > 0.99 && height > 0.3) {
                    skyTop = mix(skyTop, vec3(1.0, 1.0, 0.8), 0.8);
                }
            } else if (timeOfDay < 0.3 || timeOfDay > 0.7) {
                // Dawn/Dusk with rich colors
                float duskFactor = (timeOfDay < 0.3) ? (0.3 - timeOfDay) / 0.1 : (timeOfDay - 0.7) / 0.1;
                skyTop = mix(vec3(0.4, 0.7, 1.0), vec3(0.2, 0.1, 0.5), duskFactor);
                skyHorizon = mix(vec3(0.8, 0.9, 1.0), vec3(1.0, 0.3, 0.1), duskFactor);
                sunColor = vec3(1.0, 0.5, 0.2); // Orange sun
            } else {
                // Bright day with azure sky
                skyTop = vec3(0.3, 0.6, 1.0);
                skyHorizon = vec3(0.7, 0.8, 1.0);
                sunColor = vec3(1.0, 1.0, 0.85); // Warm daylight sun
            }
            
            // Interpolate between horizon and top based on height
            float t = clamp((height + 1.0) * 0.5, 0.0, 1.0);
            t = smoothstep(0.0, 1.0, t);
            vec3 skyColor = mix(skyHorizon, skyTop, t);
            
            // Add sun/moon
            if (sunDist > 0.995) {
                float sunIntensity = (sunDist - 0.995) / 0.005;
                skyColor = mix(skyColor, sunColor, sunIntensity);
            }
              // Add enhanced clouds with better lighting
            float cloudNoise1 = sin(direction.x * 6.0 + currentTime * 0.3) * 
                               cos(direction.z * 4.0 + currentTime * 0.2) * 
                               sin(direction.y * 8.0);
            float cloudNoise2 = sin(direction.x * 12.0 + currentTime * 0.1) * 
                               cos(direction.z * 10.0 + currentTime * 0.15);
            
            float cloudDensity = (cloudNoise1 + cloudNoise2 * 0.5) * 0.5;
            
            if (cloudDensity > 0.2 && height > 0.05) {
                vec3 cloudColor;
                float cloudIntensity = (cloudDensity - 0.2) / 0.8;
                
                // Cloud color varies with time of day
                if (timeOfDay < 0.2 || timeOfDay > 0.8) {
                    // Night clouds - darker with moon glow
                    cloudColor = vec3(0.1, 0.1, 0.3);
                } else if (timeOfDay < 0.3 || timeOfDay > 0.7) {
                    // Sunset/sunrise clouds - golden and pink
                    cloudColor = mix(vec3(1.0, 0.8, 0.6), vec3(1.0, 0.4, 0.8), cloudIntensity);
                } else {
                    // Day clouds - white and fluffy
                    cloudColor = mix(vec3(0.9, 0.9, 0.9), vec3(1.0, 1.0, 1.0), cloudIntensity);
                }
                
                // Apply cloud shading
                float cloudShading = 1.0 - cloudIntensity * 0.3;
                skyColor = mix(skyColor, cloudColor * cloudShading, cloudIntensity * 0.8);
            }
            
            return skyColor;
        }
        
        void main() {
            vec3 direction = normalize(WorldPos);
            vec3 color = getSkyColor(direction);
            FragColor = vec4(color, 1.0);
        }
    )";
    
    shaderProgram = createShader(vertexShaderSource, fragmentShaderSource);
}

unsigned int Skybox::createShader(const char* vertexSource, const char* fragmentSource) {
    // Vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, nullptr);
    glCompileShader(vertexShader);
    
    // Check vertex shader compilation
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        std::cerr << "Vertex shader compilation failed: " << infoLog << std::endl;
    }
    
    // Fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, nullptr);
    glCompileShader(fragmentShader);
    
    // Check fragment shader compilation
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        std::cerr << "Fragment shader compilation failed: " << infoLog << std::endl;
    }
    
    // Link shaders
    unsigned int program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    
    // Check linking
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "Shader program linking failed: " << infoLog << std::endl;
    }
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    return program;
}

glm::vec3 Skybox::getSkyColor(float timeOfDay, float height) const {
    // This is a helper function for future use
    glm::vec3 skyTop, skyHorizon;
    
    if (timeOfDay < 0.25f || timeOfDay > 0.75f) {
        // Night
        skyTop = glm::vec3(0.02f, 0.02f, 0.1f);
        skyHorizon = glm::vec3(0.05f, 0.05f, 0.2f);
    } else if (timeOfDay < 0.35f || timeOfDay > 0.65f) {
        // Sunrise/Sunset
        skyTop = glm::vec3(0.3f, 0.1f, 0.4f);
        skyHorizon = glm::vec3(1.0f, 0.4f, 0.2f);
    } else {
        // Day
        skyTop = glm::vec3(0.4f, 0.7f, 1.0f);
        skyHorizon = glm::vec3(0.8f, 0.9f, 1.0f);
    }
    
    float t = glm::clamp((height + 1.0f) * 0.5f, 0.0f, 1.0f);
    return glm::mix(skyHorizon, skyTop, t);
}

glm::vec3 Skybox::getHorizonColor(float timeOfDay) const {
    return getSkyColor(timeOfDay, 0.0f);
}

glm::vec3 Skybox::getSunColor(float timeOfDay) const {
    if (timeOfDay < 0.25f || timeOfDay > 0.75f) {
        return glm::vec3(0.8f, 0.8f, 1.0f); // Moon
    } else if (timeOfDay < 0.35f || timeOfDay > 0.65f) {
        return glm::vec3(1.0f, 0.6f, 0.3f); // Sunrise/Sunset sun
    } else {
        return glm::vec3(1.0f, 1.0f, 0.9f); // Day sun
    }
}
