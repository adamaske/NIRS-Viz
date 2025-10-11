#include "pch.h"
#include "Core/Window.h"

#include "Core/Input.h"

#include "Events/ApplicationEvent.h"
#include "Events/MouseEvent.h"
#include "Events/KeyEvent.h"

#include <glad/glad.h>

static int glfw_window_count = 0;

static void GLFWErrorCallback(int error, const char* description)
{
	spdlog::error("GLFW Error ({}): {}", error, description);
}

Window::Window(const WindowSpecification& spec) : specification(spec)
{
	Init();
}

Window::~Window()
{
}

void Window::Init()
{   
	// Populate window_data
	window_data.title = specification.title;
	window_data.width = specification.width;
	window_data.height = specification.height;

	// Initalize GLFW
	

	const auto init_glfw_glad = glfw_window_count == 0;
	if (init_glfw_glad) {
		if (!glfwInit())
			spdlog::error("Could not intialize GLFW!");

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	}

	gl_window = glfwCreateWindow(specification.width, specification.height, specification.title.c_str(), nullptr, nullptr);
	if (!gl_window)
		spdlog::error("Failed to create GLFW window");
	glfw_window_count++;

	glfwMakeContextCurrent(gl_window);

	if (init_glfw_glad) {
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
			spdlog::error("Failed to initialize GLAD");


		spdlog::info("OpenGL Info:");
		spdlog::info("  Version:  {}", (const char*)glad_glGetString(GL_VERSION));
		spdlog::info("  Renderer: {}", (const char*)glad_glGetString(GL_RENDERER));
		spdlog::info("  Vendor:   {}", (const char*)glad_glGetString(GL_VENDOR));

		glfwSetErrorCallback(GLFWErrorCallback);
	}
	
	glfwSetWindowUserPointer(gl_window, &window_data);
	SetVSync(true);


	glfwSetWindowSizeCallback(gl_window, [](GLFWwindow* _window, int _width, int _height)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(_window);
			data.width = _width;
			data.height = _height;

			WindowResizeEvent event(_width, _height);
			data.EventCallback(event);
		});

	glfwSetWindowCloseCallback(gl_window, [](GLFWwindow* _window)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(_window);
			WindowCloseEvent event;
			data.EventCallback(event);
		});

	glfwSetKeyCallback(gl_window, [](GLFWwindow* _window, int key, int scancode, int action, int mods)
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


	glfwSetCharCallback(gl_window, [](GLFWwindow* _window, unsigned int keycode)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(_window);

			KeyTypedEvent event(keycode);
			data.EventCallback(event);
		});

	glfwSetMouseButtonCallback(gl_window, [](GLFWwindow* _window, int button, int action, int mods)
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

	glfwSetScrollCallback(gl_window, [](GLFWwindow* _window, double xOffset, double yOffset)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(_window);

			MouseScrolledEvent event((float)xOffset, (float)yOffset);
			data.EventCallback(event);
		});

	glfwSetCursorPosCallback(gl_window, [](GLFWwindow* _window, double xPos, double yPos)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(_window);

			MouseMovedEvent event((float)xPos, (float)yPos);
			data.EventCallback(event);
		});
}

void Window::Shutdown()
{
	glfwDestroyWindow(gl_window);
	glfw_window_count--;
	if(glfw_window_count == 0)
		glfwTerminate();
}

void Window::OnUpdate(float dt)
{
	glfwPollEvents();
	glfwSwapBuffers(gl_window);
}

void Window::SetVSync(bool enabled)
{
	glfwSwapInterval(enabled);
	window_data.vsync = enabled;
}


