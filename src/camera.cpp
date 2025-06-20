#include "camera.h"
#include <algorithm>

// ============================================================================
// CAMERA CONSTRUCTOR - INITIALIZE ALL CAMERA PARAMETERS
// ============================================================================
Camera::Camera(glm::vec3 startPosition, glm::vec3 startUp, float startYaw, float startPitch)
    : position(startPosition)
    , worldUp(startUp)
    , yaw(startYaw)
    , pitch(startPitch)
    , movementSpeed(2.5f)
    , mouseSensitivity(0.1f)
    , zoom(45.0f)
    , lastX(400.0f)
    , lastY(300.0f)
    , firstMouse(true)
{
    // Calculate the initial front, right, and up vectors based on yaw and pitch
    updateCameraVectors();
}

// ============================================================================
// GENERATE VIEW MATRIX FOR RENDERING
// Creates the view matrix that transforms world coordinates to camera space
// ============================================================================
glm::mat4 Camera::getViewMatrix() {
    return glm::lookAt(position, position + front, up);
}

// ============================================================================
// PROCESS KEYBOARD INPUT FOR CAMERA MOVEMENT
// Handles WASD movement, QE for vertical movement, and ESC to close window
// ============================================================================
void Camera::processKeyboard(GLFWwindow* window, float deltaTime) {
    // Calculate movement speed based on frame time for smooth movement
    float velocity = movementSpeed * deltaTime;

    // Forward movement (W key)
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        position += front * velocity;
    }
    
    // Backward movement (S key)
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        position -= front * velocity;
    }
    
    // Left strafe movement (A key)
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        position -= right * velocity;
    }
    
    // Right strafe movement (D key)
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        position += right * velocity;
    }
    
    // Upward movement (Q key)
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        position -= up * velocity;
    }
    
    // Downward movement (E key)
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        position += up * velocity;
    }
    
    // Exit application (ESC key)
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}

// ============================================================================
// PROCESS MOUSE MOVEMENT FOR CAMERA ROTATION
// Handles mouse look functionality with sensitivity and pitch constraints
// ============================================================================
void Camera::processMouseMovement(double xpos, double ypos) {
    // Handle first mouse movement to prevent camera jump
    if (firstMouse) {
        lastX = static_cast<float>(xpos);
        lastY = static_cast<float>(ypos);
        firstMouse = false;
    }

    // Calculate mouse movement offset from last frame
    float xoffset = static_cast<float>(xpos) - lastX;
    float yoffset = lastY - static_cast<float>(ypos); // Reversed since y-coordinates go from bottom to top
    lastX = static_cast<float>(xpos);
    lastY = static_cast<float>(ypos);

    // Apply mouse sensitivity to movement
    xoffset *= mouseSensitivity;
    yoffset *= mouseSensitivity;

    // Update yaw and pitch angles
    yaw += xoffset;
    pitch += yoffset;

    // Constrain pitch to prevent screen flipping
    pitch = std::clamp(pitch, -89.0f, 89.0f);

    // Update camera vectors based on new angles
    updateCameraVectors();
}

// ============================================================================
// UPDATE CAMERA DIRECTION VECTORS
// Calculates front, right, and up vectors based on yaw and pitch angles
// ============================================================================
void Camera::updateCameraVectors() {
    // Calculate the new front vector from yaw and pitch
    glm::vec3 newFront;
    newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    newFront.y = sin(glm::radians(pitch));
    newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(newFront);
    
    // Calculate right and up vectors
    // Right vector is perpendicular to front and world up
    right = glm::normalize(glm::cross(front, worldUp));
    
    // Up vector is perpendicular to front and right
    up = glm::normalize(glm::cross(right, front));
}
