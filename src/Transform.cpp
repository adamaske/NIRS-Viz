#include "Transform.h"
#include "Transform.h"
#include "Transform.h"
#include "Transform.h"

void Transform::SetToIdentity()
{
	model_matrix = glm::mat4(1.0f);
}

void Transform::Translate(float x, float y, float z)
{
	model_matrix = glm::translate(model_matrix, glm::vec3(x, y, z));
}

void Transform::Translate(glm::vec3 offset)
{
    model_matrix = glm::translate(model_matrix, offset);
}

void Transform::Rotate(float angle_degrees, float axis_x, float axis_y, float axis_z)
{
    model_matrix = glm::rotate(
        model_matrix,
        glm::radians(angle_degrees),
        glm::vec3(axis_x, axis_y, axis_z)
    );
}

void Transform::Scale(float x, float y, float z)
{
	model_matrix = glm::scale(model_matrix, glm::vec3(x, y, z));
}

void Transform::Scale(glm::vec3 scale)
{
	model_matrix = glm::scale(model_matrix, scale);
}

glm::vec3 Transform::GetPosition()
{
	// Extract translation from model matrix
    
    return model_matrix[3];
}

glm::quat Transform::GetRotation()
{
    return glm::quat();
}

glm::vec3 Transform::GetScale()
{
    float scaleX = glm::length(glm::vec3(model_matrix[0]));
    float scaleY = glm::length(glm::vec3(model_matrix[1]));
    float scaleZ = glm::length(glm::vec3(model_matrix[2]));
    return glm::vec3(scaleX, scaleY, scaleZ);
}
