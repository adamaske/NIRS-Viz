#include "pch.h"
#include "Core/Window.h"

#include "Core/Input.h"

#include "Events/ApplicationEvent.h"
#include "Events/MouseEvent.h"
#include "Events/KeyEvent.h"

#include <glad/glad.h>

static uint8_t glfw_window_count = 0;

static void GLFWErrorCallback(int error, const char* description)
{
	spdlog::error("GLFW Error ({}): {}", error, description);
}

Window::Window(const WindowProps& props)
{
	Init(props);
}

Window::~Window()
{
}

void Window::Init(const WindowProps& props)
{
	data.Title = props.Title;
	data.Width = props.Width;
	data.Height = props.Height;


	spdlog::info("Creating window {} ({}, {})", props.Title, props.Width, props.Height);

	if(glfw_window_count == 0)
	{
		int success = glfwInit();
		if (!success)
		{
			spdlog::error("Could not initialize GLFW!");
		}
		glfwSetErrorCallback(GLFWErrorCallback);
	}

	window = glfwCreateWindow((int)props.Width, (int)props.Height, data.Title.c_str(), nullptr, nullptr);
	++glfw_window_count;

	glfwMakeContextCurrent(window);
	int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

	if (!status) {
		spdlog::error("Failed to initialize GLAD");
	}

	auto version = glad_glGetString(GL_VERSION);
	auto renderer = glad_glGetString(GL_RENDERER);
	auto vendor = glad_glGetString(GL_VENDOR);

	spdlog::info("OpenGL Info:");
	spdlog::info("  Version:  {}", (const char*)version);
	spdlog::info("  Renderer: {}", (const char*)renderer);
	spdlog::info("  Vendor:   {}", (const char*)vendor);

	glfwSetWindowUserPointer(window, &data);
	SetVSync(true);


	glfwSetWindowSizeCallback(window, [](GLFWwindow* _window, int width, int height)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(_window);
			data.Width = width;
			data.Height = height;

			WindowResizeEvent event(width, height);
			data.EventCallback(event);
		});

	glfwSetWindowCloseCallback(window, [](GLFWwindow* _window)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(_window);
			WindowCloseEvent event;
			data.EventCallback(event);
		});

	glfwSetKeyCallback(window, [](GLFWwindow* _window, int key, int scancode, int action, int mods)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(_window);

			switch (action)
			{
			case GLFW_PRESS:
			{
				KeyPressedEvent event(key, 0);
				data.EventCallback(event);
				break;
			}
			case GLFW_RELEASE:
			{
				KeyReleasedEvent event(key);
				data.EventCallback(event);
				break;
			}
			case GLFW_REPEAT:
			{
				KeyPressedEvent event(key, true);
				data.EventCallback(event);
				break;
			}
			}
		});


	glfwSetCharCallback(window, [](GLFWwindow* _window, unsigned int keycode)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(_window);

			KeyTypedEvent event(keycode);
			data.EventCallback(event);
		});

	glfwSetMouseButtonCallback(window, [](GLFWwindow* _window, int button, int action, int mods)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(_window);

			switch (action)
			{
			case GLFW_PRESS:
			{
				MouseButtonPressedEvent event(button);
				data.EventCallback(event);
				break;
			}
			case GLFW_RELEASE:
			{
				MouseButtonReleasedEvent event(button);
				data.EventCallback(event);
				break;
			}
			}
		});

	glfwSetScrollCallback(window, [](GLFWwindow* _window, double xOffset, double yOffset)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(_window);

			MouseScrolledEvent event((float)xOffset, (float)yOffset);
			data.EventCallback(event);
		});

	glfwSetCursorPosCallback(window, [](GLFWwindow* _window, double xPos, double yPos)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(_window);

			MouseMovedEvent event((float)xPos, (float)yPos);
			data.EventCallback(event);
		});
}

void Window::Shutdown()
{
	glfwDestroyWindow(window);
	--glfw_window_count;
	if (glfw_window_count == 0)
	{
		glfwTerminate();
	}
}

void Window::OnUpdate(float dt)
{
	glfwPollEvents();
	glfwSwapBuffers(window);
}

void Window::SetVSync(bool enabled)
{
	if (enabled)
		glfwSwapInterval(1);
	else
		glfwSwapInterval(0);

	data.VSync = enabled;

}


