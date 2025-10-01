#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> // For glm::value_ptr, needed to pass matrix to OpenGL
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

class Transform {

private:
    glm::mat4 model_matrix = glm::mat4(1.0f);

public:
    Transform() {
        SetToIdentity();
	}

    glm::vec3 position;
    glm::quat rotation;
    glm::vec3 scale;

    void SetToIdentity();

    glm::mat4 GetMatrix() const {
        glm::mat4 mat(1.0f);
        mat = glm::translate(mat, position);
        mat *= glm::toMat4(rotation);
        mat = glm::scale(mat, scale);
        return mat;
    }

    void Translate(float x, float y, float z);
    void Translate(glm::vec3 offset);

    void Rotate(float angle_degrees, const glm::vec3& axis);

    void Scale(float x, float y, float z);
    void Scale(glm::vec3 scale);

	void SetPosition(glm::vec3 _position);
	void SetScale(glm::vec3 _scale);
	void SetRotation(glm::quat _rotation);

};