#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> // For glm::value_ptr, needed to pass matrix to OpenGL


class Transform {
public:
    glm::mat4 model_matrix = glm::mat4(1.0f);

    void SetToIdentity();

    void Translate(float x, float y, float z);
    void Translate(glm::vec3 offset);

    void Rotate(float angle_degrees, float axis_x, float axis_y, float axis_z);

    void Scale(float x, float y, float z);
    void Scale(glm::vec3 scale);

    glm::vec3 GetPosition();
    glm::quat GetRotation();
    glm::vec3 GetScale();
};