#include "Head.h"
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>


#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include "glm/gtx/string_cast.hpp"
#include <spdlog/spdlog.h>

Head::Head()
{
	shader = new Shader(std::string("C:/dev/NIRS-Viz/data/Shaders/Phong.vert"), std::string("C:/dev/NIRS-Viz/data/Shaders/Phong.frag"));
	scalp_mesh = new Mesh("C:/dev/NIRS-Viz/data/example_scalp.obj");
	transform = new Transform();

	landmark_shader = new Shader(std::string("C:/dev/NIRS-Viz/data/Shaders/Landmark.vert"), std::string("C:/dev/NIRS-Viz/data/Shaders/Landmark.frag"));
	landmark_mesh = new Mesh("C:/dev/NIRS-Viz/data/landmark.obj");

	line_shader = new Shader(std::string("C:/dev/NIRS-Viz/data/Shaders/Line.vert"), std::string("C:/dev/NIRS-Viz/data/Shaders/Line.frag"));


	for (auto& lm : landmarks) {
		lm->transform->Scale(glm::vec3(5.0f, 5.0f, 5.0f));

	}
	float lm_dist = 100.0;
	naison.transform	->Translate(0, 0, -lm_dist);
	inion.transform		->Translate(0, 0, lm_dist);
	lpa.transform	->Translate(lm_dist, 0, 0);
	rpa.transform	->Translate(-lm_dist, 0, 0);

	// Create base lines
	naison_inion_line = new Line(naison.transform->position, inion.transform->position, glm::vec3(1, 1, 0), 2.0f, line_shader);
	ear_to_ear_line =	new Line(lpa.transform->position, rpa.transform->position, glm::vec3(0, 1, 1), 2.0f, line_shader);
	
};

Head::~Head()
{

};

void Head::Draw(glm::mat4 view, glm::mat4 proj, glm::vec3 view_pos)
{
	shader->Bind();
	shader->SetUniformMat4f("model", transform->GetMatrix());
	shader->SetUniformMat4f("view", view);
	shader->SetUniformMat4f("projection", proj);
		  
	shader->SetUniform3f("lightPos", view_pos + glm::vec3(0.0f, 50.f, 0.0f)); // glm::vec3(0.0f, 120.0f, -200.0f));   // adjust as needed
	shader->SetUniform3f("lightColor", glm::vec3(1.0f, 1.0f, 1.0f)); // white light
	shader->SetUniform3f("viewPos", view_pos); // you’ll need a getter in Camera
	shader->SetUniform3f("objectColor", glm::vec3(1.0f, 0.5f, 0.5f)); // pinkish brain
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBindVertexArray(scalp_mesh->VAO);
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(scalp_mesh->indices.size()), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	shader->Unbind(); // Disable shader



	if (render_landmarks) DrawLandmarks(view, proj, view_pos);
}


void Head::UpdateLandmark(LandmarkType type, const glm::vec3& position) {
	landmark_map[type]->transform->SetPosition(position);

	//Update lines if needed
	if(type == LandmarkType::NAISON || type == LandmarkType::INION) {
		naison_inion_line->p1 = naison.transform->position;
		naison_inion_line->p2 = inion.transform->position;
		naison_inion_line->dirty = true;
	}
	if (type == LandmarkType::LPA || type == LandmarkType::RPA) {
		ear_to_ear_line->p1 = lpa.transform-> position;
		ear_to_ear_line->p2 = rpa.transform->position;
		ear_to_ear_line->dirty = true;
	}
}

void Head::DrawLandmarks(glm::mat4 view, glm::mat4 proj, glm::vec3 veiw_pos)
{
	ImGui::Begin("Landmark Calibration");
	

	for (auto lm : landmarks) {

		glm::vec3 pos = lm->transform->position;
		if (ImGui::DragFloat3(landmark_labels[lm->type].c_str(), glm::value_ptr(pos), 0.1f)) {
			// position updated interactively
			UpdateLandmark(lm->type, pos);
		}

	}

	landmark_shader->Bind();

	landmark_shader->SetUniformMat4f("view", view);
	landmark_shader->SetUniformMat4f("projection", proj);

	glBindVertexArray(landmark_mesh->VAO);

	for (auto lm : landmarks) {
		landmark_shader->SetUniformMat4f("model", lm->transform->GetMatrix());
		landmark_shader->SetUniform3f("sphereColor", lm->color.x, lm->color.y, lm->color.z);

		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(landmark_mesh->indices.size()), GL_UNSIGNED_INT, 0);
	}

	landmark_shader->Unbind();
	glBindVertexArray(0);


	naison_inion_line->Draw(view, proj);
	ear_to_ear_line->Draw(view, proj);

	if (ImGui::Button("Generate Coordinates"))
	{
		GenerateCoordinateSystem();
	};
	ImGui::End();
}

std::unordered_map<LandmarkType, unsigned int> Head::LandmarksToClosestVertex()
{
	auto vertices = scalp_mesh->vertices;
	auto indices = scalp_mesh->indices;

	auto local = transform->GetMatrix();

	std::unordered_map<LandmarkType, unsigned int> lm_closest_vert_idx;

	for (auto lm : landmarks) {
		float min_dist = std::numeric_limits<float>::max();

		unsigned int closest_idx = 0;
		for (unsigned int i = 0; i < vertices.size(); i++) {
			auto v = vertices[i];

			glm::vec3 hs = glm::translate(local, v.position)[3]; // To head space
			float dist = glm::distance(lm->transform->position, hs);
			if (dist < min_dist) {
				min_dist = dist;
				closest_idx = i;
			}
		}

		lm_closest_vert_idx[lm->type] = closest_idx;
		auto hs = glm::translate(local, vertices[closest_idx].position)[3]; // To head space

		UpdateLandmark(lm->type, hs);
		//spdlog::info("Closest vertex to {} is idx {} at distance {}", landmark_labels[lm->type], closest_idx, min_dist);
	}

	return lm_closest_vert_idx;
}

void Head::ShortestPathOnScalp(unsigned int start_vertex, unsigned int end_vertex)
{
	// Find the normal between
	auto vertices = scalp_mesh->vertices;
	auto indices = scalp_mesh->indices;

	auto local = transform->GetMatrix();

	glm::vec3 p1 = glm::translate(local, vertices[start_vertex].position)[3]; // To head space
	glm::vec3 p2 = glm::translate(local, vertices[end_vertex].position)[3]; // To head space

	glm::vec3 dir = glm::normalize(p1 - p2);

	spdlog::info("Shortest path from {} to {} : ", start_vertex, end_vertex);
	spdlog::info(" ( {}, {}, {} ) to ( {}, {}, {} )", p1.x, p1.y, p1.z, p2.x, p2.y, p2.z);
	spdlog::info("Direction: ( {}, {}, {} )", dir.x, dir.y, dir.z);

	// Dijkstra's algorithm

}

void Head::GenerateCoordinateSystem()
{
	// Find the cloeset vertices to each landmark
	
	auto closest_indices_map = LandmarksToClosestVertex();

	// Find closest path from 
	// 
	// verticies[closest_indices[NAISON]] to verticies[closest_indices[INION]]
	ShortestPathOnScalp(closest_indices_map[NAISON], closest_indices_map[INION]);
	// verticies[closest_indices[LPA]] to verticies[closest_indices[RPA]]
	ShortestPathOnScalp(closest_indices_map[LPA], closest_indices_map[RPA]);


}
