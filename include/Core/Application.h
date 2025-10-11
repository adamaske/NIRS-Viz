#pragma once

#include <string>

#include "Core/Base.h"
#include "Core/Window.h"
#include "Events/Event.h"
#include "Events/ApplicationEvent.h"
#include "Core/Layer.h"
#include "Core/LayerStack.h"
#include "GUI/ImGuiLayer.h"

struct ApplicationCommandLineArgs
{
	int count = 0;
	char** args = nullptr;

	const char* operator[](int index) const
	{
		return args[index];
	}
};

struct ApplicationSpecification
{
	std::string Name = "NIRS Viz";
	std::string WorkingDirectory;
	ApplicationCommandLineArgs CommandLineArgs;
};

class Application {
public:

	ApplicationSpecification specification;
	static Application* instance;
	Window* window;
	bool running = true;
	bool minimized = false;
	float last_frame_time = 0.0f;

	LayerStack layer_stack;
	ImGuiLayer* imgui_layer;

	Application(const ApplicationSpecification& spec);
	~Application();

	static Application& Get() { return *instance; }


	void OnEvent(Event& e);

	void Run();
	void Shutdown();

	bool OnWindowClose(WindowCloseEvent& e);
	bool OnWindowResize(WindowResizeEvent& e);
	
};