#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include "Transform.h"

// Defines the directions used in the world coordinate system (Y is Up)
const glm::vec3 WORLD_UP = glm::vec3(0.0f, 1.0f, 0.0f);
const glm::vec3 WORLD_RIGHT = glm::vec3(1.0f, 0.0f, 0.0f);
const glm::vec3 WORLD_FORWARD = glm::vec3(0.0f, 0.0f, -1.0f); // Looking down -Z

class Camera {
public:
	glm::vec3 position;
	float yaw;  
	float pitch;

	glm::vec3 front;
	glm::vec3 up;   
	glm::vec3 right;

	bool fixed_aspect_ratio = false;
	float fov = 45.0f;       // A more standard starting FOV
	float aspect_ratio = 4.0f / 3.0f;
	float near_plane = 0.1f;
	float far_plane = 1000.0f;

	float orbit_theta = 0.0f;
	float orbit_phi = 0.0f;
	float orbit_radius = 600.0f;
	Transform* orbit_target = nullptr;

	Camera(glm::vec3 _position = glm::vec3(0.0f, 0.0f, 3.0f), float _yaw = -90.0f, float _pitch = 0.0f);

	glm::mat4 GetViewMatrix() const;
	glm::mat4 GetProjectionMatrix() const;

	void UpdateCameraVectors();
	void UpdateAspectRatio(float new_aspect_ratio);

	void Update();

private:
	void Init(float _yaw, float _pitch);

};