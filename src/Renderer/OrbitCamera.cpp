#include "pch.h"
#include "Renderer/OrbitCamera.h"

void OrbitCamera::OnUpdate(float dt)
{
    glm::vec3 target_pos = glm::vec3(0.0f);
    if (orbit_target) {
        target_pos = orbit_target->position;
    }
    
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

    view_matrix = glm::lookAt(position, position + front, up);
    projection_matrix = glm::perspective(glm::radians(fov), aspect_ratio, near_plane, far_plane);
}

void OrbitCamera::OnEvent(Event& e)
{
}

void OrbitCamera::InsertTarget(const std::string& name, Transform* target)
{
    orbit_target_map[name] = target;
}

void OrbitCamera::SetCurrentTarget(const std::string& name)
{
    if (orbit_target_map.find(name) != orbit_target_map.end()) {
        orbit_target = orbit_target_map[name];
        orbit_target_name = name;
    };
}

void OrbitCamera::SetOrbitPosition(float theta, float phi, float distance)
{

}

void OrbitCamera::SetOrbitPosition(const std::string& _name)
{
    for (auto& [name, pos] : orbit_positions) {
        if (name == _name) {
            orbit_theta = std::get<0>(pos);
            orbit_phi   = std::get<1>(pos);
        }
    }
}
