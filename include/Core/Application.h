#pragma once

#include "Core/Base.h"
#include "Core/Window.h"
#include "Events/Event.h"
#include "Events/ApplicationEvent.h"

#include <string>

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

	Scope<Window> window;
	bool running = true;
	bool minimized = false;
	float last_frame_time = 0.0f;


	Application(const ApplicationSpecification& spec);
	~Application();

	static Application& Get() { return *instance; }


	void OnEvent(Event& e);

	void Run();
	void Shutdown();

	bool OnWindowClose(WindowCloseEvent& e);
	bool OnWindowResize(WindowResizeEvent& e);
	
};