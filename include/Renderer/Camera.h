#pragma once
#include <glm/glm.hpp>
#include "Events/Event.h"

// Defines the directions used in the world coordinate system (Y is Up)
const glm::vec3 WORLD_UP = glm::vec3(0.0f, 1.0f, 0.0f);
const glm::vec3 WORLD_RIGHT = glm::vec3(1.0f, 0.0f, 0.0f);
const glm::vec3 WORLD_FORWARD = glm::vec3(0.0f, 0.0f, -1.0f); // Looking down -Z

class Camera {
public:
	float fov = 45.0f;       // A more standard starting FOV
	float aspect_ratio = 4.0f / 3.0f;
	float near_plane = 0.1f;
	float far_plane = 1000.0f;
	glm::vec3 position = { 0.0f, 0.0f, 0.0f };

	glm::vec3 front;
	glm::vec3 right;
	glm::vec3 up;
	glm::vec3 focal_point = { 0.0f, 0.0f, 0.0f };
	Camera();

	virtual void OnUpdate(float dt) = 0;
	virtual void OnEvent(Event& e) = 0;
	const glm::mat4& GetViewMatrix() { return view_matrix; };
	const glm::mat4& GetProjectionMatrix() { return projection_matrix; };

protected:
	glm::mat4 view_matrix = glm::mat4(1.0f);
	glm::mat4 projection_matrix = glm::mat4(1.0f);




};