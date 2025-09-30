#include "Cortex.h"

#include <filesystem>
Cortex::Cortex() : 
	shader(	std::filesystem::path("C:/dev/NIRS-Viz/data/Shaders/Phong.vert"), 
			std::filesystem::path("C:/dev/NIRS-Viz/data/Shaders/Phong.frag")), 
	mesh("C:/dev/NIRS-Viz/data/cortex.obj")
{

	transform.SetToIdentity();

}

void Cortex::Draw(glm::mat4 view, glm::mat4 proj, glm::vec3 view_position)
{
	shader.Bind();
	shader.SetUniformMat4f("model", transform.model_matrix);
	shader.SetUniformMat4f("view", view);
	shader.SetUniformMat4f("projection", proj);

	shader.SetUniform3f("lightPos", view_position + glm::vec3(0.0f, 50.f, 0.0f)); // glm::vec3(0.0f, 120.0f, -200.0f));   // adjust as needed
	shader.SetUniform3f("lightColor", glm::vec3(1.0f, 1.0f, 1.0f)); // white light
	shader.SetUniform3f("viewPos", view_position); // you’ll need a getter in Camera
	shader.SetUniform3f("objectColor", glm::vec3(1.0f, 0.5f, 0.5f)); // pinkish brain

	glBindVertexArray(mesh.VAO);
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mesh.indices.size()), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	glUseProgram(0); // Disable shader
}