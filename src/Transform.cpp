#include "Transform.h"

Transform::Transform(glm::vec3 _pos, glm::quat _rot, glm::vec3 _scale)
{
	position = _pos;
	rotation = _rot;
	scale = _scale;
}

void Transform::SetToIdentity()
{
    position = glm::vec3(0.0f, 0.0f, 0.0f);
    rotation = glm::quat();
    scale = glm::vec3(1.0f, 1.0f, 1.0f);

	model_matrix = glm::mat4(1.0f);
}

void Transform::Translate(float x, float y, float z)
{
	position += glm::vec3(x, y, z);
}

void Transform::Translate(glm::vec3 offset)
{
	position += offset;
}

void Transform::Rotate(float angle_degrees, const glm::vec3& axis)
{
	rotation = glm::normalize(glm::rotate(rotation,
		glm::radians(angle_degrees),
		axis));
}

void Transform::Scale(float x, float y, float z)
{
	scale = glm::vec3(x, y, z);
}

void Transform::Scale(glm::vec3 _scale)
{
	scale = _scale;
}

void Transform::SetPosition(glm::vec3 _position)
{
	position = _position;
}

void Transform::SetScale(glm::vec3 _scale)
{
    scale = _scale;
}

void Transform::SetRotation(glm::quat _rotation)
{
	rotation = _rotation;
}
