#include <iostream>
#include <spdlog/spdlog.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> 
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <filesystem>

#include "Camera.h"
#include "Shader.h"
#include "Mesh.h"
#include "ReferencePoints.h"
#include "Snirf.h"

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

    int screenWidth = 800;
    int screenHeight = 600;
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


    fs::path resource_dir = fs::path("C:/dev/NeuroVisualizer/data");

    fs::path vertex_path = resource_dir / "Shaders/Phong.vert";
    fs::path fragment_path = resource_dir / "Shaders/Phong.frag";
    fs::path mesh_path = resource_dir / "cortex.obj";
    
    Camera camera(glm::vec3(0, 0, 300.0f));
    Shader mesh_shader(vertex_path, fragment_path);
    Mesh mesh(mesh_path);
	ReferencePoints refpts(resource_dir / "Colin/anatomical/refpts.txt", resource_dir / "Colin/anatomical/refpts_labels.txt");
	SNIRF snirf("C:/dev/NeuroVisualizer/data/example.snirf");

	static float cortex_z_rotation = 0.0f;
    static float model_z_position = 0.0f;
    static float refpts_x_position = 0.0f;

    glm::vec4 clear_color = glm::vec4(0.45f, 0.55f, 0.60f, 1.00f);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // --- ImGui UI Definition ---
        ImGui::Begin("Cortex Controller");

        // Slider for Mesh's position (moving the object)
        ImGui::SliderFloat("Mesh Z Position", &model_z_position, -200.0f, 200.0f, "%.1f units");
        ImGui::SliderFloat("Mesh Z Rotation", &cortex_z_rotation, -180.0f, 180.0f, "%.1f units");
        ImGui::SliderFloat("refpts_x_position", &refpts_x_position, -180.0f, 180.0f, "%.1f units");

        // NEW SLIDER: Controls Camera distance (moving the camera)
        // Range 10.0f (close) to 500.0f (far)

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        ImGui::End();

        glClearColor(clear_color.r, clear_color.g, clear_color.b, clear_color.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
       
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, model_z_position));
		model = glm::rotate(model, glm::radians(cortex_z_rotation), glm::vec3(0.0f, 1.0f, 0.0f));

        auto view = camera.GetViewMatrix();
		auto projection = camera.GetProjectionMatrix();


        mesh_shader.Bind();

        mesh_shader.SetUniformMat4f("model", model);
        mesh_shader.SetUniformMat4f("view", view);
        mesh_shader.SetUniformMat4f("projection", projection);

        mesh_shader.SetUniform3f("lightPos", glm::vec3(0.0f, 120.0f, 50.0f));   // adjust as needed
        mesh_shader.SetUniform3f("lightColor", glm::vec3(1.0f, 1.0f, 1.0f)); // white light

        // Camera
        mesh_shader.SetUniform3f("viewPos", camera.position); // you’ll need a getter in Camera

        // Material (object base color)
        mesh_shader.SetUniform3f("objectColor", glm::vec3(1.0f, 0.5f, 0.5f)); // pinkish brain

        glBindVertexArray(mesh.VAO);
        glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        glDisable(GL_DEPTH_TEST);

        glm::mat4 refpts_T = glm::mat4(1.0f);
		refpts_T = glm::translate(refpts_T, glm::vec3(refpts_x_position, 0.0f, 0.0));
		refpts_T = glm::scale(refpts_T, glm::vec3(0.5f, 0.5f, 0.5f)); // make refpts larger
        refpts_T = glm::rotate(refpts_T, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		refpts.Draw(view, projection, refpts_T);
        glEnable(GL_DEPTH_TEST);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
