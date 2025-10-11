#pragma once
#include "Events/Event.h"


struct WindowProps
{
	std::string Title;
	uint32_t Width;
	uint32_t Height;

	WindowProps(const std::string& title = "Hazel Engine",
		uint32_t width = 1600,
		uint32_t height = 900)
		: Title(title), Width(width), Height(height)
	{
	}
};

struct WindowData
{
	std::string Title;
	unsigned int Width, Height;
	bool VSync;

	EventCallbackFn EventCallback;
};

class Window
{
public:
	WindowData data;
	GLFWwindow* window;

	Window(const WindowProps & props);
	virtual ~Window();

	virtual void Init(const WindowProps& props);
	virtual void Shutdown();

	virtual void OnUpdate(float dt);
	virtual unsigned int GetWidth() const  { return data.Width; }
	virtual unsigned int GetHeight() const { return data.Height; }

	virtual void SetEventCallback(const EventCallbackFn& callback) { data.EventCallback = callback; }
	virtual void SetVSync(bool enabled) ;

	virtual void* GetNativeWindow() const { return window; }



};