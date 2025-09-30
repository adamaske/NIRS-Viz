#include "Camera.h"
#include <cmath> 
#include <algorithm>
#include <spdlog/spdlog.h>

Camera::Camera(glm::vec3 _position, float _yaw, float _pitch)
{
	position = _position;
    Init(_yaw, _pitch);
}

glm::mat4 Camera::GetViewMatrix() const
{
    return glm::lookAt(position, position + front, up);
}

glm::mat4 Camera::GetProjectionMatrix() const
{
    return glm::perspective(glm::radians(fov), aspect_ratio, near_plane, far_plane);
}

void Camera::UpdateCameraVectors()
{
    glm::vec3 _front;

	auto rad_yaw = glm::radians(yaw);
	auto rad_pitch = glm::radians(pitch);
     _front.x = cos(rad_yaw) * cos(rad_pitch);
     _front.y = sin(rad_pitch);
     _front.z = sin(rad_yaw) * cos(rad_pitch);
    front = glm::normalize(_front);

    right = glm::normalize(glm::cross(front, WORLD_UP));
    up = glm::normalize(glm::cross(right, front));
}


void Camera::UpdateAspectRatio(float new_aspect_ratio)
{
    if (!fixed_aspect_ratio)
    {
        aspect_ratio = new_aspect_ratio;
    }
}

void Camera::Update()
{
    if (!orbit_target) {
        return;
    }

    glm::vec3 target_pos = orbit_target->model_matrix[3]; // Extract translation from model matrix
    orbit_phi = glm::clamp(orbit_phi, -89.99f, 89.99f);
    float theta = glm::radians(orbit_theta); 
    float phi = glm::radians(orbit_phi);   

    float x = orbit_radius * cos(phi) * cos(theta);
    float y = orbit_radius * sin(phi);
    float z = orbit_radius * cos(phi) * sin(theta);

    position = target_pos + glm::vec3(x, y, z);

    front = glm::normalize(target_pos - position);

    right = glm::normalize(glm::cross(front, WORLD_UP));
    up = glm::normalize(glm::cross(right, front));

}

void Camera::Init(float _yaw, float _pitch)
{

    // Ensure Pitch stays within bounds to prevent camera "flipping"

    //_pitch = std::clamp(_pitch, -89.999999999f, 89.99999999f);

    // Set initial Euler angles
    this->yaw = _yaw;
    this->pitch = _pitch;

    // Calculate initial direction vectors
    UpdateCameraVectors();
}
