#pragma once

#include <unordered_map>

#include "Core/Layer.h"

#include "Transform.h"
#include "Renderer/FreeRoamCamera.h"
#include "Renderer/OrbitCamera.h"
#include "Head.h"
#include "Cortex.h"

class TestLayer : public Layer {
public:


	FreeRoamCamera* free_roam_camera = nullptr;
	OrbitCamera* orbit_camera = nullptr;

	bool use_free_roam_camera = true;

	Head* head = nullptr;
	Cortex* cortex = nullptr;

	TestLayer();
	~TestLayer();


	virtual void OnAttach() override;
	virtual void OnDetach() override;
	virtual void OnUpdate(float dt)  override;
	virtual void OnRender() override;
	virtual void OnImGuiRender() override;
	virtual void OnEvent(Event& event) override;
};