#pragma once

#include "Renderer/Camera.h"

#include "Events/MouseEvent.h"
#include "Events/ApplicationEvent.h"

class FreeRoamCamera : public Camera {
public:

	float movement_speed = 10.0f;
	float rotation_speed = 0.8f;

	float pitch = 0.0f;
	float yaw = 90.0f;

	double original_cursor_x = 0;
	double original_cursor_y = 0;
	
	bool m_IsRMBDown = false;
	glm::vec2 inital_mouse_position;

	FreeRoamCamera(const glm::vec3& start_pos);

	void OnUpdate(float dt) override;
	void OnEvent(Event& e) override;

	void UpdateCameraVectors();
	void UpdateViewMatrix();
};