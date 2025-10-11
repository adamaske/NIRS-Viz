#pragma once

#include "Camera.h"

#include "Transform.h"
#include <map>
class OrbitCamera : public Camera {
public:
	float orbit_theta = -100.0f;
	float orbit_phi = 22.0f;
	float orbit_radius = 460;
	Transform* orbit_target = nullptr;

	std::unordered_map<std::string, Transform*> orbit_target_map = {};
	std::string orbit_target_name = "";

	std::string current_orbit_position = "Default";
	std::vector<std::tuple<std::string, std::tuple<float, float>>> orbit_positions = {
		{"Default",			{-100.0f, 22.0f}},
		{"Anterior",        {-90.0f , 0.0f}},
		{"Posterior",       {90.0f  , 0.0f}},
		{"Left",            {180.0f , 0.0f}},
		{"Right",           {0.0f   , 0.0f}},
		{"Superior",        {90.0f  , 90.0f}},
		{"Inferior",        {90.0f  , -90.0f}}
	};

	OrbitCamera(float _theta, float _phi, float _radius, Transform* _target)
		: Camera(), orbit_theta(_theta), orbit_phi(_phi), orbit_radius(_radius), orbit_target(_target) {};

	void OnUpdate(float dt) override;
	void OnEvent(Event& e) override;


	void InsertTarget(const std::string& name, Transform* target);
	void SetCurrentTarget(const std::string& name);

	void SetOrbitPosition(float theta, float phi, float distance);
	void SetOrbitPosition(const std::string& name);
};