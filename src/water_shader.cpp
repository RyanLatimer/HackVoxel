#include "water_shader.h"
#include <iostream>
#include <cmath>

WaterShader::WaterShader() : shaderProgram(0) {
}

WaterShader::~WaterShader() {
    if (shaderProgram) {
        glDeleteProgram(shaderProgram);
    }
}

bool WaterShader::initialize() {
    const char* vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec2 aTexCoord;
        
        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;
        uniform float time;
        uniform float waterLevel;
        
        out vec3 WorldPos;
        out vec2 TexCoord;
        out vec3 Normal;
        out float WaveHeight;
        
        void main() {
            vec3 worldPos = (model * vec4(aPos, 1.0)).xyz;
            
            // Create animated waves
            float wave1 = sin(worldPos.x * 0.5 + time * 2.0) * 0.1;
            float wave2 = cos(worldPos.z * 0.3 + time * 1.5) * 0.08;
            float wave3 = sin((worldPos.x + worldPos.z) * 0.2 + time * 1.8) * 0.05;
            
            WaveHeight = wave1 + wave2 + wave3;
            worldPos.y += WaveHeight;
            
            WorldPos = worldPos;
            TexCoord = aTexCoord;
            
            // Calculate normal for lighting (simplified)
            vec3 tangent = vec3(1.0, cos(worldPos.x * 0.5 + time * 2.0) * 0.05, 0.0);
            vec3 bitangent = vec3(0.0, -sin(worldPos.z * 0.3 + time * 1.5) * 0.024, 1.0);
            Normal = normalize(cross(tangent, bitangent));
            
            gl_Position = projection * view * vec4(worldPos, 1.0);
        }
    )";
    
    const char* fragmentShaderSource = R"(
        #version 330 core
        in vec3 WorldPos;
        in vec2 TexCoord;
        in vec3 Normal;
        in float WaveHeight;
        
        uniform float time;
        uniform vec3 lightDirection;
        
        out vec4 FragColor;
        
        void main() {
            // Animated water texture coordinates
            vec2 uv1 = TexCoord + vec2(time * 0.02, time * 0.03);
            vec2 uv2 = TexCoord + vec2(-time * 0.015, time * 0.025);
            
            // Create water caustics pattern
            float caustic1 = sin(uv1.x * 20.0) * cos(uv1.y * 20.0);
            float caustic2 = sin(uv2.x * 15.0) * cos(uv2.y * 15.0);
            float caustics = (caustic1 + caustic2) * 0.1 + 0.9;
            
            // Water color with depth variation
            vec3 shallowColor = vec3(0.4, 0.8, 1.0);    // Light blue
            vec3 deepColor = vec3(0.0, 0.3, 0.8);       // Deep blue
            
            // Simple depth calculation based on wave height
            float depth = clamp(-WaveHeight + 0.5, 0.0, 1.0);
            vec3 waterColor = mix(shallowColor, deepColor, depth);
            
            // Simple lighting
            vec3 lightDir = normalize(-lightDirection);
            float lightIntensity = max(dot(Normal, lightDir), 0.3);
            
            // Add foam on wave peaks
            float foam = smoothstep(0.08, 0.12, abs(WaveHeight));
            vec3 foamColor = vec3(1.0, 1.0, 1.0);
            
            // Final color combination
            vec3 finalColor = mix(waterColor * lightIntensity * caustics, foamColor, foam);
            
            // Add transparency and refraction effect
            float alpha = 0.8 + foam * 0.2;
            FragColor = vec4(finalColor, alpha);
        }
    )";
    
    shaderProgram = createShader(vertexShaderSource, fragmentShaderSource);
    if (shaderProgram == 0) {
        return false;
    }
    
    getUniformLocations();
    std::cout << "Water shader initialized successfully" << std::endl;
    return true;
}

void WaterShader::use() {
    glUseProgram(shaderProgram);
}

void WaterShader::setMatrices(const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection) {
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
}

void WaterShader::setTime(float time) {
    glUniform1f(timeLoc, time);
}

void WaterShader::setWaterLevel(float level) {
    glUniform1f(waterLevelLoc, level);
}

void WaterShader::setLightDirection(const glm::vec3& direction) {
    glUniform3fv(lightDirLoc, 1, glm::value_ptr(direction));
}

GLuint WaterShader::createShader(const char* vertexSource, const char* fragmentSource) {
    // Vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, nullptr);
    glCompileShader(vertexShader);
    
    // Check vertex shader compilation
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        std::cerr << "Water vertex shader compilation failed: " << infoLog << std::endl;
        return 0;
    }
    
    // Fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, nullptr);
    glCompileShader(fragmentShader);
    
    // Check fragment shader compilation
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        std::cerr << "Water fragment shader compilation failed: " << infoLog << std::endl;
        return 0;
    }
    
    // Link shaders
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    
    // Check linking
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "Water shader program linking failed: " << infoLog << std::endl;
        return 0;
    }
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    return program;
}

void WaterShader::getUniformLocations() {
    modelLoc = glGetUniformLocation(shaderProgram, "model");
    viewLoc = glGetUniformLocation(shaderProgram, "view");
    projectionLoc = glGetUniformLocation(shaderProgram, "projection");
    timeLoc = glGetUniformLocation(shaderProgram, "time");
    waterLevelLoc = glGetUniformLocation(shaderProgram, "waterLevel");
    lightDirLoc = glGetUniformLocation(shaderProgram, "lightDirection");
}
