#include <iostream>
#include <filesystem>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> 

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <spdlog/spdlog.h>

#include "Transform.h"
#include "Shader.h"
#include "Mesh.h"
#include "Camera.h"
#include "Cortex.h"
#include "Snirf.h"
#include "ReferencePoints.h"

namespace fs = std::filesystem;
// Function to handle GLFW errors
void glfw_error_callback(int error, const char* description) {
    //fprintf(stderr, "GLFW Error %d: %s\n", error, description);
	spdlog::error("GLFW Error {}: {}", error, description);
}

// Function to handle window resizing
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// Main function
int main() {
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    int screenWidth = 1280;
    int screenHeight = 720;
    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "NIRS Visualizer", NULL, NULL);
    if (window == NULL) {
        spdlog::error("Failed to create GLFW window");
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        spdlog::error("Failed to initialize GLAD");
        glfwTerminate();
        return 1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 460");

    //glfwSwapInterval(1);
    glEnable(GL_DEPTH_TEST);


    fs::path resource_dir = fs::path("C:/dev/NIRS-Viz/data");

    fs::path vertex_path = resource_dir / "Shaders/Phong.vert";
    fs::path fragment_path = resource_dir / "Shaders/Phong.frag";
    fs::path mesh_path = resource_dir / "cortex.obj";
    
    Camera camera(glm::vec3(0, 0, 300.0f)); // 300 units backwards
	camera.aspect_ratio = static_cast<float>(screenWidth) / static_cast<float>(screenHeight);
    
    Cortex cortex = Cortex();
    cortex.transform.Translate(glm::vec3(0, 0, -50.0f));
	camera.orbit_target = &cortex.transform;

    ReferencePoints refpts(resource_dir / "Colin/anatomical/refpts.txt", resource_dir / "Colin/anatomical/refpts_labels.txt");
    SNIRF snirf("C:/dev/NIRS-Viz/data/example.snirf");

	static float cortex_z_rotation = 0.0f;
    static float model_z_position = 0.0f;
    static float refpts_x_position = 0.0f;

    glm::vec4 clear_color = glm::vec4(0.45f, 0.55f, 0.60f, 1.00f);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Cortex & Camera Controller");

		ImGui::SliderFloat("Camera Distance", &camera.orbit_radius, 0.0f, 2000.0f, "%.1f units");
        ImGui::SliderFloat("Camera Theta", &camera.orbit_theta, -360.0f, 360.0f, "%.1f units");
        ImGui::SliderFloat("Camera Phi", &camera.orbit_phi, -89.9f, 89.9f, "%.1f units");
		ImGui::Checkbox("Orbit Cortex", &camera.orbit_cortex);
        
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

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        ImGui::End();

        glClearColor(clear_color.r, clear_color.g, clear_color.b, clear_color.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
       
        camera.Update();

        auto view = camera.GetViewMatrix();
		auto projection = camera.GetProjectionMatrix();

        cortex.Draw(view, projection, camera.position);


        //glDisable(GL_DEPTH_TEST);
        //
        //glm::mat4 refpts_T = glm::mat4(1.0f);
		//refpts_T = glm::translate(refpts_T, glm::vec3(refpts_x_position, 0.0f, 0.0));
		//refpts_T = glm::scale(refpts_T, glm::vec3(0.5f, 0.5f, 0.5f)); // make refpts larger
        //refpts_T = glm::rotate(refpts_T, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		//refpts.Draw(view, projection, refpts_T);
        //glEnable(GL_DEPTH_TEST);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
