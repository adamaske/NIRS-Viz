#include "pch.h"
#include "Core/Application.h"
#include "Core/Window.h"

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include "Transform.h"
#include "Shader.h"
#include "Mesh.h"
#include "Camera.h"
#include "Cortex.h"
#include "Snirf.h"
#include "Probe.h"
#include "Head.h"
#include "PointRenderer.h"

Application* Application::instance = nullptr;

Application::Application(const ApplicationSpecification& spec) : specification(spec)
{
	instance = this;


	//if (!specification.WorkingDirectory.empty())
	//	std::filesystem::current_path(specification.WorkingDirectory);

   window = new Window(WindowProps(specification.Name, 1280, 720));
   window->SetEventCallback(BIND_EVENT_FN(Application::OnEvent));
	
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
	//
	//for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it)
	//{
	//	if (e.Handled)
	//		break;
	//	(*it)->OnEvent(e);
	//}
}

void Application::Run()
{
    GLFWwindow* _window = window->gl_window;
	auto screenWidth = window->GetWidth();
	auto screenHeight = window->GetHeight();
    
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(_window, true);
    ImGui_ImplOpenGL3_Init("#version 460");
    
    glfwSwapInterval(1);
    glEnable(GL_DEPTH_TEST);


    fs::path resource_dir = fs::path("C:/dev/NIRS-Viz/data");

    fs::path vertex_path = resource_dir / "Shaders/Phong.vert";
    fs::path fragment_path = resource_dir / "Shaders/Phong.frag";
    fs::path mesh_path = resource_dir / "cortex.obj";

    std::unordered_map<std::string, Transform*> orbit_target_map = {};

    Camera camera(glm::vec3(0, 0, 300.0f)); // 300 units backwards
    camera.aspect_ratio = static_cast<float>(screenWidth) / static_cast<float>(screenHeight);


    Head head = Head();
    orbit_target_map["Head"] = head.transform;
    //head.transform->Scale(glm::vec3(100.0f, 100.0f, 100.0f));

    Cortex cortex = Cortex();
    cortex.transform.Translate(glm::vec3(0, 0, -50.0f));
    orbit_target_map["Cortex"] = &cortex.transform;

    //SNIRF snirf("C:/dev/NIRS-Viz/data/example.snirf");
    //Probe probe(&snirf);
    //orbit_target_map["Probe"] = probe.transform;

    static std::string current_target = "Head";
    camera.orbit_target = orbit_target_map[current_target];

    glm::vec4 clear_color = glm::vec4(0.45f, 0.55f, 0.60f, 1.00f);

    while (!glfwWindowShouldClose(_window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Cortex & Camera Controller");

        if (ImGui::BeginCombo("Orbit Target", current_target.c_str()))
        {
            for (auto& [name, transform] : orbit_target_map)
            {
                bool is_selected = (current_target == name);
                if (ImGui::Selectable(name.c_str(), is_selected))
                {
                    current_target = name;
                    camera.orbit_target = transform; // set the camera's orbit target
                }

                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        ImGui::SliderFloat("Camera Distance", &camera.orbit_radius, 0.0f, 2000.0f, "%.1f units");
        ImGui::SliderFloat("Camera Theta", &camera.orbit_theta, -360.0f, 360.0f, "%.1f units");
        ImGui::SliderFloat("Camera Phi", &camera.orbit_phi, -89.9f, 89.9f, "%.1f units");

        if (ImGui::Button("Reset Camera")) {
            camera.orbit_radius = 600.0f;
            camera.orbit_theta = 0.0f;
            camera.orbit_phi = 0.0f;
        }
        if (ImGui::Button("Anterior")) {
            //Anterior cortex view
            camera.orbit_theta = -90.0f;
            camera.orbit_phi = 0.0f;
        }
        if (ImGui::Button("Posteriour")) {
            //Anterior cortex view
            camera.orbit_theta = 90.0f;
            camera.orbit_phi = 0.0f;
        }
        if (ImGui::Button("Right"))
        {
            //Right cortex view
            camera.orbit_phi = 0.0f;
            camera.orbit_theta = 0.0f;
        }
        if (ImGui::Button("Left"))
        {
            camera.orbit_theta = 180.0f;
            camera.orbit_phi = 0.0f;
        }
        if (ImGui::Button("Superior"))
        {
            camera.orbit_theta = 90.0f;
            camera.orbit_phi = 90.0f;
        }
        if (ImGui::Button("Inferior"))
        {
            camera.orbit_theta = 90.0f;
            camera.orbit_phi = -90.0f;
        }

        ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        ImGui::End();

        glClearColor(clear_color.r, clear_color.g, clear_color.b, clear_color.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        camera.Update();

        auto view = camera.GetViewMatrix();
        auto projection = camera.GetProjectionMatrix();

        //cortex.Draw(view, projection, camera.position);
        //refpts.Draw(view, projection);
        head.Draw(view, projection, camera.position);


        //
        //
        //glm::mat4 refpts_T = glm::mat4(1.0f);
        //refpts_T = glm::translate(refpts_T, glm::vec3(refpts_x_position, 0.0f, 0.0));
        //refpts_T = glm::scale(refpts_T, glm::vec3(0.5f, 0.5f, 0.5f)); // make refpts larger
        //refpts_T = glm::rotate(refpts_T, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        //refpts.Draw(view, projection, refpts_T);
        //glEnable(GL_DEPTH_TEST);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(_window);
    }

    glfwDestroyWindow(_window);
    glfwTerminate();
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
