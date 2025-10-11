#include "pch.h"
#include "Core/Application.h"
#include "Core/Window.h"

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include "Renderer/Camera.h"
#include "Transform.h"
#include "Shader.h"
#include "Mesh.h"
#include "Cortex.h"
#include "Snirf.h"
#include "Probe.h"
#include "Head.h"
#include "PointRenderer.h"

#include "TestLayer.h"

#include "PointRenderer.h"

Application* Application::instance = nullptr;

Application::Application(const ApplicationSpecification& spec) : specification(spec)
{
	instance = this;

	if (!specification.WorkingDirectory.empty())
		std::filesystem::current_path(specification.WorkingDirectory);

    WindowSpecification window_spec;
	window_spec.title = specification.Name;
	window_spec.width = 1280;
	window_spec.height = 720;
    
    window = new Window(window_spec);
    window->SetEventCallback(BIND_EVENT_FN(Application::OnEvent));

	layer_stack = LayerStack();

	imgui_layer = new ImGuiLayer();
    layer_stack.PushOverlay(imgui_layer); // Calls onAttach

	layer_stack.PushLayer(new TestLayer());
}

Application::~Application()
{
	Shutdown();
}

void Application::OnEvent(Event& e)
{
	EventDispatcher dispatcher(e);
	dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(Application::OnWindowClose));
	dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(Application::OnWindowResize));
	
	for (auto it = layer_stack.rbegin(); it != layer_stack.rend(); ++it)
	{
		if (e.handled)
			break;
		(*it)->OnEvent(e);
	}
}

void Application::Run()
{
    while (running) {
        float time = (float)glfwGetTime();
        float delta_time = time - last_frame_time;
        last_frame_time = time;

        if (!minimized)
        {
            for (Layer* layer : layer_stack)
                layer->OnUpdate(delta_time);

            // Render Loop
			for (Layer* layer : layer_stack)
				layer->OnRender();
			 
            imgui_layer->Begin();
            for (Layer* layer : layer_stack)
                layer->OnImGuiRender();
            imgui_layer->End();

        }

		window->OnUpdate(delta_time);
    }

}

void Application::Shutdown()
{
}


bool Application::OnWindowClose(WindowCloseEvent& e)
{
	running = false;
	return true;
}

bool Application::OnWindowResize(WindowResizeEvent& e)
{

	if (e.GetWidth() == 0 || e.GetHeight() == 0)
	{
		minimized = true;
		return false;
	}

	minimized = false;
	//Renderer::OnWindowResize(e.GetWidth(), e.GetHeight());

	return false;
}
