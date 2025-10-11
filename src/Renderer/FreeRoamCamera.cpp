#include "pch.h"
#include "Renderer/FreeRoamCamera.h"

// Assuming these paths/classes exist in the user's project setup
#include "Core/Application.h" 
#include "Core/Input.h"
#include "Core/KeyCodes.h"
#include "Core/MouseCodes.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <algorithm> // For std::min/max/clamp

FreeRoamCamera::FreeRoamCamera(const glm::vec3& start_pos)
{
	position = start_pos;
    UpdateCameraVectors();
    UpdateViewMatrix();
    projection_matrix = glm::perspective(glm::radians(fov), aspect_ratio, near_plane, far_plane);
}

void FreeRoamCamera::OnUpdate(float dt)
{
    spdlog::info("FREE ROAM UPDATE");
    if (Input::IsMouseButtonPressed(Mouse::ButtonRight)) {
        if (!m_IsRMBDown)
        {
            // Initial capture: Lock and hide the cursor, record starting mouse position
            inital_mouse_position = { Input::GetMouseX(), Input::GetMouseY() };
            m_IsRMBDown = true;

            glfwGetCursorPos(Application::Get().window->GetHandle(), &original_cursor_x, &original_cursor_y);
            glfwSetInputMode(Application::Get().window->GetHandle(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }

        const glm::vec2& mouse{ Input::GetMouseX(), Input::GetMouseY() };
        glm::vec2 delta = (mouse - inital_mouse_position) * 0.003f;
        inital_mouse_position = mouse;

        yaw += delta.x * rotation_speed;
        pitch -= delta.y * rotation_speed; // Subtract because positive Y is usually down on screen

        // Clamp pitch to prevent camera flip (standard 89-degree limit)
        pitch = std::clamp(pitch, -89.9f, 89.9f);

        UpdateCameraVectors();
    }
    else {
        if (m_IsRMBDown)
        {
            // Release: Unlock and show the cursor
            m_IsRMBDown = false;

            glfwSetCursorPos(Application::Get().window->GetHandle(), original_cursor_x, original_cursor_y);
            glfwSetInputMode(Application::Get().window->GetHandle(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }

    float velocity = movement_speed * dt;

    // Optional: Increase speed with LeftShift (Unreal default)
    if (Input::IsKeyPressed(Key::LeftShift))
        velocity *= 3.0f; // Faster movement

    // W & S: Forward/Backward (along local m_Front vector)
    if (Input::IsKeyPressed(Key::W))
        position += front * velocity;
    if (Input::IsKeyPressed(Key::S))
        position -= front * velocity;

    // A & D: Strafe Left/Right (along local m_Right vector)
    if (Input::IsKeyPressed(Key::A))
        position -= right * velocity;
    if (Input::IsKeyPressed(Key::D))
        position += right * velocity;

    // E & Q: World Up/Down (along world Y axis)
    if (Input::IsKeyPressed(Key::E))
        position += WORLD_UP * velocity;
    if (Input::IsKeyPressed(Key::Q))
        position -= WORLD_UP * velocity;
    UpdateViewMatrix();
}

void FreeRoamCamera::OnEvent(Event& e)
{
}

void FreeRoamCamera::UpdateCameraVectors() {
    glm::vec3 front_temp;
    // Angles must be converted to radians for cos/sin functions
    front_temp.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front_temp.y = sin(glm::radians(pitch));
    front_temp.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

    front = glm::normalize(front_temp);

    // Also recalculate the Right and Up vectors
    // Right = normalize(cross(Front, WorldUp))
    right = glm::normalize(glm::cross(front, WORLD_UP));

    // Up = normalize(cross(Right, Front))
    up = glm::normalize(glm::cross(right, front));
}

void FreeRoamCamera::UpdateViewMatrix()
{
    view_matrix = glm::lookAt(position, position + front, up);
    projection_matrix = glm::perspective(glm::radians(fov), aspect_ratio, near_plane, far_plane);
}
