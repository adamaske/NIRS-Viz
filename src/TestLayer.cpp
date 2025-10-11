#include "pch.h"
#include "TestLayer.h"

#include <imgui.h>

#include "Core/Application.h"
#include "Core/Input.h"

#include "Transform.h"
#include "Shader.h"
#include "Mesh.h"
#include "Renderer/OrbitCamera.h"
#include "Renderer/FreeRoamCamera.h"
#include "Cortex.h"
#include "Snirf.h"
#include "Probe.h"
#include "Head.h"
#include "PointRenderer.h"

TestLayer::TestLayer() : Layer("TestLayer")
{
}

TestLayer::~TestLayer()
{
}



void TestLayer::OnAttach()
{
    Application& app = Application::Get();
    auto screenWidth = app.window->GetWidth();
    auto screenHeight = app.window->GetHeight();

	orbit_camera = new OrbitCamera(0, 0, 600, nullptr); // 300 units backwards
    orbit_camera->SetOrbitPosition("Default");
	orbit_camera->aspect_ratio = static_cast<float>(screenWidth) / static_cast<float>(screenHeight);

	free_roam_camera = new FreeRoamCamera({0, 50, -500});
    free_roam_camera->aspect_ratio = static_cast<float>(screenWidth) / static_cast<float>(screenHeight);

    head = new Head();
    //head.transform->Scale(glm::vec3(100.0f, 100.0f, 100.0f));

    cortex = new Cortex();
    cortex->transform.Translate(glm::vec3(0, 0, -50.0f));

    orbit_camera->InsertTarget("Head", head->transform);
    orbit_camera->InsertTarget("Cortex", &cortex->transform);
    orbit_camera->SetCurrentTarget("Head");

}

void TestLayer::OnDetach()
{
}

void TestLayer::OnUpdate(float dt)
{


    Camera* camera = use_free_roam_camera ? free_roam_camera : (Camera*)orbit_camera;
    camera->OnUpdate(dt);

}

void TestLayer::OnRender()
{
    glm::vec4 clear_color = glm::vec4(0.45f, 0.55f, 0.60f, 1.00f);
    glClearColor(clear_color.r, clear_color.g, clear_color.b, clear_color.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_PROGRAM_POINT_SIZE); // <-- ADD THIS LINE

    Camera* camera = use_free_roam_camera ? free_roam_camera : (Camera*)orbit_camera;
    glm::mat4 view = camera->GetViewMatrix();
    glm::mat4 projection = camera->GetProjectionMatrix();
   

    head->Draw(view, projection, camera->position);
}

void TestLayer::OnImGuiRender()
{
    ImGui::Begin("Cortex & Camera Controller");
    if (ImGui::BeginCombo("Camera Method", use_free_roam_camera ? "Free Roam" : "Orbit Target")) {
        if (ImGui::Selectable("Free Roam")) use_free_roam_camera = true;
        if (ImGui::Selectable("Orbit Target")) use_free_roam_camera = false;
        ImGui::EndCombo();
    }

    if (!use_free_roam_camera) { // Handle Orbit Camera
        if (ImGui::BeginCombo("Orbit Target", orbit_camera->orbit_target_name.c_str())) // Select Orbit Target
        {
            for (auto& [name, transform] : orbit_camera->orbit_target_map)
            {
                bool is_selected = (orbit_camera->orbit_target_name == name);
                if (ImGui::Selectable(name.c_str(), is_selected))
                {
                    orbit_camera->SetCurrentTarget(name);
                }
    
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
       
        if (ImGui::BeginCombo("Camera Position", orbit_camera->current_orbit_position.c_str())) { // Predefined positions
            for (auto& [name, pos] : orbit_camera->orbit_positions) {

                bool is_selected = orbit_camera->current_orbit_position == name;
                if (ImGui::Selectable(name.c_str(), is_selected)) {
    
                    orbit_camera->orbit_theta = std::get<0>(pos);
                    orbit_camera->orbit_phi = std::get<1>(pos); 
                    orbit_camera->current_orbit_position = name;
                }

                if (is_selected)
                    ImGui::SetItemDefaultFocus();
    
            }
            ImGui::EndCombo();
        }

        ImGui::SliderFloat("Camera Distance", &orbit_camera->orbit_radius, 0.0f, 2000.0f, "%.1f units"); // Manual Controls
        ImGui::SliderFloat("Camera Theta", &orbit_camera->orbit_theta, -360.0f, 360.0f, "%.1f units");
        ImGui::SliderFloat("Camera Phi", &orbit_camera->orbit_phi, -89.9f, 89.9f, "%.1f units");
    }
    
    if (use_free_roam_camera) {

        ImGui::SliderFloat("Movement Speed", &free_roam_camera->movement_speed, 0.0f, 200.0f, "%.1f units"); // Manual Controls
        ImGui::SliderFloat("Rotation Speed", &free_roam_camera->rotation_speed, 0.01f, 10.0f, "%.1f units");
        ImGui::SliderFloat("Yaw", &free_roam_camera->yaw, -90.0f, 90.0f, "%.1f units");
        ImGui::SliderFloat("Pitch", &free_roam_camera->pitch, -90.0f, 90.0f, "%.1f units");

        ImGui::DragFloat3("Front", glm::value_ptr(free_roam_camera->front), 0.1f);
        ImGui::DragFloat3("Right", glm::value_ptr(free_roam_camera->right), 0.1f);
        ImGui::DragFloat3("Up", glm::value_ptr(free_roam_camera->up), 0.1f);
    }
    
    ImGuiIO& io = ImGui::GetIO();
    ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

	ImGui::End();


    ImGui::Begin("Coordinate System Generator");

    if (ImGui::Button("Generate Coordinates"))
    {
       head->GenerateCoordinateSystem();
    };

    ImGui::Checkbox("Draw Landmarks", &head->draw_landmarks);
    ImGui::Checkbox("Draw Landmark Lines", &head->draw_landmark_lines);
    for (auto lm : head->landmarks) {
        glm::vec3 pos = lm->transform->position;
        if (ImGui::DragFloat3(head->landmark_labels[lm->type].c_str(), glm::value_ptr(pos), 0.1f)) {
            // position updated interactively
            head->UpdateLandmark(lm->type, pos);
        }
    }

    ImGui::Checkbox("Draw Rays", &head->draw_rays);
    ImGui::Checkbox("Draw Waypoints", &head->draw_waypoints);
    ImGui::Checkbox("Draw Paths", &head->draw_paths);
    ImGui::Checkbox("Draw Reference Points", &head->draw_refpts);


    ImGui::Text("Raycast Parameters");
    ImGui::DragFloat(
        "Rotation Step (deg)", // Label displayed in the GUI
        &head->theta_step_size,      // Pointer to the float variable
        0.5f,                  // Speed/sensitivity of dragging
        0.1f,                  // Minimum allowed value
        90.0f,                 // Maximum allowed value
        "%.1f degrees"         // Display format
    );
    ImGui::DragFloat(
        "Ray Length (mm)",      // Label displayed in the GUI
        &head->ray_distance,          // Pointer to the float variable
        1.0f,                   // Speed/sensitivity of dragging
        10.0f,                  // Minimum allowed value (depends on mesh scale)
        1000.0f,                // Maximum allowed value (depends on mesh scale)
        "%.1f mm"               // Display format, assuming your mesh units are millimeters
    );
    
    ImGui::End();

}

void TestLayer::OnEvent(Event& event)
{
}
