#pragma once

#include <glm/glm.hpp>

struct Ray {
    glm::vec3 origin;
    glm::vec3 endpoint;
};

struct RayHit {
    float t_distance = std::numeric_limits<float>::max();
    unsigned int hit_v0 = 0, hit_v1 = 0, hit_v2 = 0; // Vertices of the hit triangle
};

std::vector<Ray> GenerateSweepingArchRays(const glm::vec3& origin,
                                          const glm::vec3& direction,
                                          const glm::vec3& rotation_axis,
                                          const float& distance = 250.0f,
                                          const float& theta_min = 0,
                                          const float& theta_max = 180,
                                          const float& theta_stepsize = 10.0f);

bool RayIntersectsTriangle(const glm::vec3& origin,
                           const glm::vec3& direction,
                           const glm::vec3& v0,
                           const glm::vec3& v1,
                           const glm::vec3& v2,
                           float& t);