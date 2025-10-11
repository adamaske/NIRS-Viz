#pragma once
#include "Events/Event.h"


struct WindowSpecification
{
	std::string title;
	uint32_t width = 1280;
	uint32_t height = 720;
	bool resizeable = true;
	bool vsync = true;
};


struct WindowData
{
	std::string title;
	unsigned int width, height;
	bool vsync;

	EventCallbackFn EventCallback;
};

class Window
{
public:
	WindowSpecification specification;
	WindowData window_data;
	GLFWwindow* gl_window;

	Window(const WindowSpecification& spec);
	virtual ~Window();

	virtual void Init();
	virtual void Shutdown();

	virtual void OnUpdate(float dt);
	virtual unsigned int GetWidth() const  { return window_data.width; }
	virtual unsigned int GetHeight() const { return window_data.height; }

	virtual void SetEventCallback(const EventCallbackFn& callback) { window_data.EventCallback = callback; }
	virtual void SetVSync(bool enabled) ;

	GLFWwindow* GetHandle() const { return gl_window; }



};