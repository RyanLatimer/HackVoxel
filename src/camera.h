#pragma once

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

/**
 * Camera class for handling first-person camera movement and mouse look
 * Provides WASD movement, QE for up/down, and mouse look functionality
 */
class Camera {
public:
    // ============================================================================
    // CAMERA POSITION AND ORIENTATION VECTORS
    // ============================================================================
    glm::vec3 position;     // Camera position in world space
    glm::vec3 front;        // Direction the camera is looking
    glm::vec3 up;           // Up vector for camera orientation
    glm::vec3 worldUp;      // World up vector (usually Y-axis)
    glm::vec3 right;        // Right vector for strafing

    // ============================================================================
    // EULER ANGLES FOR CAMERA ROTATION
    // ============================================================================
    float yaw;              // Horizontal rotation (left/right)
    float pitch;            // Vertical rotation (up/down)

    // ============================================================================
    // CAMERA SETTINGS AND CONSTRAINTS
    // ============================================================================
    float movementSpeed;    // Speed of WASD movement
    float mouseSensitivity; // Mouse look sensitivity
    float zoom;             // Field of view for perspective projection

    // ============================================================================
    // MOUSE INPUT STATE TRACKING
    // ============================================================================
    float lastX;            // Last mouse X position
    float lastY;            // Last mouse Y position
    bool firstMouse;        // Flag to handle first mouse movement

    // ============================================================================
    // CONSTRUCTOR AND INITIALIZATION
    // ============================================================================
    Camera(glm::vec3 startPosition = glm::vec3(0.0f, 0.0f, 3.0f),
           glm::vec3 startUp = glm::vec3(0.0f, 1.0f, 0.0f),
           float startYaw = -90.0f,
           float startPitch = 0.0f);

    // ============================================================================
    // CAMERA MATRIX GENERATION
    // ============================================================================
    glm::mat4 getViewMatrix();

    // ============================================================================
    // INPUT PROCESSING METHODS
    // ============================================================================
    void processKeyboard(GLFWwindow* window, float deltaTime);
    void processMouseMovement(double xpos, double ypos);

    // ============================================================================
    // CAMERA STATE UPDATE
    // ============================================================================
    void updateCameraVectors();
};
