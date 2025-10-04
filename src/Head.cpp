#include "Head.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <numeric>

#include <spdlog/spdlog.h>

#include <glm/glm.hpp>
#include <glm/geometric.hpp>
#include "glm/gtx/string_cast.hpp"

#include "Mesh.h"
#include "Transform.h"

#include "Raycaster.h"

Head::Head()
{
	scalp_shader = new Shader(std::string("C:/dev/NIRS-Viz/data/Shaders/Phong.vert"), std::string("C:/dev/NIRS-Viz/data/Shaders/Phong.frag"));
	scalp_mesh = new Mesh("C:/dev/NIRS-Viz/data/example_scalp_2.obj");
	transform = new Transform();

	scalp_mesh_graph = CreateGraphFromTriangleMesh(*scalp_mesh, transform->GetMatrix());

	landmark_shader = new Shader(std::string("C:/dev/NIRS-Viz/data/Shaders/Landmark.vert"), std::string("C:/dev/NIRS-Viz/data/Shaders/Landmark.frag"));
	landmark_mesh = new Mesh("C:/dev/NIRS-Viz/data/landmark.obj");

	line_shader = new Shader(std::string("C:/dev/NIRS-Viz/data/Shaders/Line.vert"), std::string("C:/dev/NIRS-Viz/data/Shaders/Line.frag"));

	waypoint_shader = landmark_shader;
	waypoint_mesh = landmark_mesh;

	for (auto& lm : landmarks) {
		lm->transform->Scale(glm::vec3(5.0f, 5.0f, 5.0f));

	}
	float lm_dist = 120.0;
	naison.transform->Translate(0, 0, -lm_dist);
	inion.transform	->Translate(0, 0, lm_dist);
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

	ImGui::Begin("Coordinate System Generator");

	if (ImGui::Button("Generate Coordinates"))
	{
		GenerateCoordinateSystem();
	};

	ImGui::Text("Raycast Parameters");
	ImGui::DragFloat(
		"Rotation Step (deg)", // Label displayed in the GUI
		&theta_step_size,      // Pointer to the float variable
		0.5f,                  // Speed/sensitivity of dragging
		0.1f,                  // Minimum allowed value
		90.0f,                 // Maximum allowed value
		"%.1f degrees"         // Display format
	);
	ImGui::DragFloat(
		"Ray Length (mm)",      // Label displayed in the GUI
		&ray_distance,          // Pointer to the float variable
		1.0f,                   // Speed/sensitivity of dragging
		10.0f,                  // Minimum allowed value (depends on mesh scale)
		1000.0f,                // Maximum allowed value (depends on mesh scale)
		"%.1f mm"               // Display format, assuming your mesh units are millimeters
	);

	ImGui::Checkbox("Draw Landmark Lines", &draw_landmark_lines);
	ImGui::Checkbox("Draw Waypoints", &draw_waypoints);
	ImGui::Checkbox("Draw Rays", &draw_rays);

	DrawScalp(view, proj, view_pos);
	DrawLandmarks(view, proj, view_pos);
	if(draw_waypoints) DrawWaypoints(view, proj, view_pos);
	if(draw_landmark_lines) DrawLandmarkLines(view, proj, view_pos);
	if (draw_rays) DrawRays(view, proj, view_pos);
	ImGui::End();
}


void Head::DrawScalp(glm::mat4 view, glm::mat4 proj, glm::vec3 view_pos)
{
	scalp_shader->Bind();
	scalp_shader->SetUniformMat4f("model", transform->GetMatrix());
	scalp_shader->SetUniformMat4f("view", view);
	scalp_shader->SetUniformMat4f("projection", proj);

	scalp_shader->SetUniform3f("lightPos", view_pos + glm::vec3(0.0f, 50.f, 0.0f)); // glm::vec3(0.0f, 120.0f, -200.0f));   // adjust as needed
	scalp_shader->SetUniform3f("lightColor", glm::vec3(1.0f, 1.0f, 1.0f)); // white light
	scalp_shader->SetUniform3f("viewPos", view_pos); // you�ll need a getter in Camera
	scalp_shader->SetUniform3f("objectColor", glm::vec3(1.0f, 0.5f, 0.5f)); // pinkish brain

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glLineWidth(0.7f);

	glBindVertexArray(scalp_mesh->VAO);
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(scalp_mesh->indices.size()), GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}
void Head::DrawLandmarkLines(glm::mat4 view, glm::mat4 proj, glm::vec3 veiw_pos)
{
	naison_inion_line->Draw(view, proj);
	ear_to_ear_line->Draw(view, proj);
}

void Head::DrawRays(glm::mat4 view, glm::mat4 proj, glm::vec3 veiw_pos)
{
	for (auto& line : nz_iz_path) {
		line->Draw(view, proj);
	}
	for (auto& line : lpa_rpa_path) {
		line->Draw(view, proj);
	}
}

void Head::DrawWaypoints(glm::mat4 view, glm::mat4 proj, glm::vec3 veiw_pos)
{

	waypoint_shader->Bind();
	waypoint_shader->SetUniformMat4f("view", view);
	waypoint_shader->SetUniformMat4f("projection", proj);

	Transform t = Transform();
	t.SetScale({ 3, 3, 3 });
	glBindVertexArray(waypoint_mesh->VAO);
	for (auto& wp : nz_iz_waypoints) {

		t.SetPosition(wp->position);
		waypoint_shader->SetUniformMat4f("model", t.GetMatrix());
		waypoint_shader->SetUniform3f("sphereColor", 1, 0.2, 0);

		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(waypoint_mesh->indices.size()), GL_UNSIGNED_INT, 0);
	}
	for (auto& wp : lpa_rpa_waypoints) {

		t.SetPosition(wp->position);
		waypoint_shader->SetUniformMat4f("model", t.GetMatrix());
		waypoint_shader->SetUniform3f("sphereColor", 0.2, 1, 0);

		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(waypoint_mesh->indices.size()), GL_UNSIGNED_INT, 0);
	}
	glBindVertexArray(0);
}

void Head::DrawLandmarks(glm::mat4 view, glm::mat4 proj, glm::vec3 veiw_pos)
{
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

	for (auto& lm : landmarks) {
		landmark_shader->SetUniformMat4f("model", lm->transform->GetMatrix());
		landmark_shader->SetUniform3f("sphereColor", lm->color.x, lm->color.y, lm->color.z);

		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(landmark_mesh->indices.size()), GL_UNSIGNED_INT, 0);
	}

	glBindVertexArray(0);
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


void Head::LandmarksToClosestVertex()
{
	const auto& vertices = scalp_mesh->vertices;
	const glm::mat4& local_matrix = transform->GetMatrix();

	std::unordered_map<LandmarkType, unsigned int> lm_closest_vert_idx;
	lm_closest_vert_idx.reserve(landmarks.size()); 

	std::vector<glm::vec3> hs_vertices;
	hs_vertices.reserve(vertices.size()); // Vertices to head space
	for (const auto& v : vertices) {
		hs_vertices.push_back(glm::vec3(glm::translate(local_matrix, v.position)[3]));
	}
	for (auto lm : landmarks) {
		float min_dist = std::numeric_limits<float>::max();
		unsigned int closest_idx = 0;

		const glm::vec3& lm_pos = lm->transform->position;

		for (unsigned int i = 0; i < hs_vertices.size(); i++) {
			const glm::vec3& hs_v = hs_vertices[i];

			float dist_sq = glm::distance2(lm_pos, hs_v);
			
			if(dist_sq < min_dist) {
				min_dist = dist_sq;
				closest_idx = i;
			}
		}
		
		lm_closest_vert_idx[lm->type] = closest_idx;
		UpdateLandmark(lm->type, hs_vertices[closest_idx]);
	}

	lm_closest_vert_idx_map = lm_closest_vert_idx;
}

void Head::GenerateRays()
{

	const auto& vertices = scalp_mesh->vertices;
	const auto& local_matrix = transform->GetMatrix();

	const auto& nz = vertices[lm_closest_vert_idx_map[NAISON]].position;
	const auto& iz = vertices[lm_closest_vert_idx_map[INION]].position;
	const auto& lpa = vertices[lm_closest_vert_idx_map[LPA]].position;
	const auto& rpa = vertices[lm_closest_vert_idx_map[RPA]].position;

	// From vertex to head space -> Whats the solution to this??
	const auto& nz_iz_midpoint = glm::translate(local_matrix, glm::vec3((nz + iz) / 2.0f))[3];
	const auto& lpa_rpa_midpoint = glm::translate(local_matrix, glm::vec3((lpa + rpa) / 2.0f))[3];

	const auto& nz_iz_direction = glm::normalize(iz - nz);
	const auto& lpa_rpa_direction = glm::normalize(rpa - lpa);

	const auto& nz_iz_rotation_axis = glm::normalize(glm::cross(nz_iz_direction, glm::vec3(0, 1, 0)));
	const auto& lpa_rpa_rotation_axis = glm::normalize(glm::cross(lpa_rpa_direction, glm::vec3(0, 1, 0)));

	//Clear previous rays
	nz_iz_rays.clear();
	lpa_rpa_rays.clear();

	nz_iz_rays = GenerateSweepingArchRays(nz_iz_midpoint,
		nz_iz_direction,
		nz_iz_rotation_axis,
		ray_distance,
		theta_step_size,
		180.0f,
		theta_step_size);

	lpa_rpa_rays = GenerateSweepingArchRays(lpa_rpa_midpoint,
		lpa_rpa_direction,
		lpa_rpa_rotation_axis,
		ray_distance,
		theta_step_size,
		180.0f,
		theta_step_size);


	for (auto& line : nz_iz_path) delete line;
	for (auto& line : lpa_rpa_path) delete line;
	nz_iz_path.clear();
	lpa_rpa_path.clear();

	for (auto& ray : nz_iz_rays) {
		nz_iz_path.push_back(new Line(ray.origin, ray.endpoint, glm::vec3(0, 0, 1), 2, line_shader));
	}
	for (auto& ray : lpa_rpa_rays) {
		lpa_rpa_path.push_back(new Line(ray.origin, ray.endpoint, glm::vec3(0, 1, 0), 2, line_shader));
	}
}

void Head::CastRays()
{
	const auto& vertices = scalp_mesh->vertices;
	const auto& indices = scalp_mesh->indices;
	const auto& local_matrix = transform->GetMatrix();
	
	std::vector<glm::vec3> hs_vertices;
	hs_vertices.reserve(vertices.size()); // Vertices to head space
	for (const auto& v : vertices) {
		hs_vertices.push_back(glm::vec3(glm::translate(local_matrix, v.position)[3]));

	}

	std::vector<unsigned int> nz_iz_rough_path_indices; 
	std::vector<unsigned int> lpa_rpa_rough_path_indices;

	spdlog::info("Casting {} rays for Nasion-Inion and {} rays for LPA-RPA", nz_iz_rays.size(), lpa_rpa_rays.size());
	for (const auto& ray : nz_iz_rays) {

		glm::vec3 ray_origin = ray.origin;
		glm::vec3 ray_endpoint = ray.endpoint;
		glm::vec3 ray_direction = glm::normalize(ray_endpoint - ray_origin);

		RayHit best_hit;

		// Iterate through every triangle in the mesh (Brute-Force)
		for (unsigned int i = 0; i < indices.size(); i += 3) {
			// Get the three vertices of the current triangle
			glm::vec3 v0 = glm::translate(local_matrix, vertices[indices[i + 0]].position)[3];
			glm::vec3 v1 = glm::translate(local_matrix, vertices[indices[i + 1]].position)[3];
			glm::vec3 v2 = glm::translate(local_matrix, vertices[indices[i + 2]].position)[3];

			float t; 

			if (RayIntersectsTriangle(ray_origin, ray_direction, v0, v1, v2, t)) {
				if (t < best_hit.t_distance) {
					best_hit.t_distance = t;
					best_hit.hit_v0 = indices[i + 0];
					best_hit.hit_v1 = indices[i + 1];
					best_hit.hit_v2 = indices[i + 2];
				}
			}
		}

		// --- Process the best hit for this ray ---
		if (best_hit.t_distance != std::numeric_limits<float>::max()) {

			glm::vec3 intersection_point = ray_origin + ray_direction * best_hit.t_distance;
			
			unsigned int closest_vertex_index = best_hit.hit_v0;
			float min_dist_sq = glm::distance2(intersection_point, hs_vertices[best_hit.hit_v0]);

			// Check v1
			float dist_sq_v1 = glm::distance2(intersection_point, hs_vertices[best_hit.hit_v1]);
			if (dist_sq_v1 < min_dist_sq) {
				min_dist_sq = dist_sq_v1;
				closest_vertex_index = best_hit.hit_v1;
			}

			// Check v2
			float dist_sq_v2 = glm::distance2(intersection_point, hs_vertices[best_hit.hit_v2]);
			if (dist_sq_v2 < min_dist_sq) {
				closest_vertex_index = best_hit.hit_v2;
			}

			// Add this closest vertex as a rough waypoint
			nz_iz_rough_path_indices.push_back(closest_vertex_index);
		}
	}
	for (const auto& ray : lpa_rpa_rays) {

		glm::vec3 ray_origin = ray.origin;
		glm::vec3 ray_endpoint = ray.endpoint;
		glm::vec3 ray_direction = glm::normalize(ray_endpoint - ray_origin);

		RayHit best_hit;

		// Iterate through every triangle in the mesh (Brute-Force)
		for (unsigned int i = 0; i < indices.size(); i += 3) {
			// Get the three vertices of the current triangle
			glm::vec3 v0 = glm::translate(local_matrix, vertices[indices[i + 0]].position)[3];
			glm::vec3 v1 = glm::translate(local_matrix, vertices[indices[i + 1]].position)[3];
			glm::vec3 v2 = glm::translate(local_matrix, vertices[indices[i + 2]].position)[3];
			// We need to turn it into head space


			float t; // Intersection distance

			if (RayIntersectsTriangle(ray_origin, ray_direction, v0, v1, v2, t)) {
				// Check if this intersection is the closest one so far
				if (t < best_hit.t_distance) {
					best_hit.t_distance = t;
					best_hit.hit_v0 = indices[i + 0];
					best_hit.hit_v1 = indices[i + 1];
					best_hit.hit_v2 = indices[i + 2];
				}
			}
		}

		// --- Process the best hit for this ray ---
		if (best_hit.t_distance != std::numeric_limits<float>::max()) {

			// Calculate the intersection point in world space
			glm::vec3 intersection_point = ray_origin + ray_direction * best_hit.t_distance;

			// Find the index of the vertex closest to the intersection point
			unsigned int closest_vertex_index = best_hit.hit_v0;
			float min_dist_sq = glm::distance2(intersection_point, hs_vertices[best_hit.hit_v0]);

			// Check v1
			float dist_sq_v1 = glm::distance2(intersection_point, hs_vertices[best_hit.hit_v1]);
			if (dist_sq_v1 < min_dist_sq) {
				min_dist_sq = dist_sq_v1;
				closest_vertex_index = best_hit.hit_v1;
			}

			// Check v2
			float dist_sq_v2 = glm::distance2(intersection_point, hs_vertices[best_hit.hit_v2]);
			if (dist_sq_v2 < min_dist_sq) {
				closest_vertex_index = best_hit.hit_v2;
			}

			// Add this closest vertex as a rough waypoint
			lpa_rpa_rough_path_indices.push_back(closest_vertex_index);
		}
	}
	// Delete previous waypoints
	spdlog::info("Nasion-Inion rough path waypoints: {}", nz_iz_rough_path_indices.size());
	spdlog::info("LPA-RPA rough path waypoints: {}", lpa_rpa_rough_path_indices.size());

	for (auto& wp : nz_iz_waypoints) delete wp;
	for (auto& wp : lpa_rpa_waypoints) delete wp;
	nz_iz_waypoints.clear();
	lpa_rpa_waypoints.clear();

	for (unsigned int i = 0; i < nz_iz_rough_path_indices.size(); i++) {
		auto& vertex_index = nz_iz_rough_path_indices[i];
		nz_iz_waypoints.push_back(new Waypoint{ i, vertex_index, hs_vertices[vertex_index]});
	}

	for (unsigned int i = 0; i < lpa_rpa_rough_path_indices.size(); i++) {;
		const auto& vertex_index = lpa_rpa_rough_path_indices[i];
		lpa_rpa_waypoints.push_back(new Waypoint{ i, vertex_index, hs_vertices[vertex_index]});
	}
}

std::vector<unsigned int> Head::FindFinePath(std::vector<unsigned int> rough_path)
{
	std::vector<unsigned int> fine_path;
	
	size_t total_rough_segments = rough_path.size() > 0 ? rough_path.size() - 1 : 0;
	for (size_t i = 0; i < total_rough_segments; ++i) {
		auto segment = DjikstraShortestPath(scalp_mesh_graph, rough_path[i], rough_path[i + 1]);
		fine_path.insert(rough_path.end(), segment.begin(), segment.end());
	}
	
	auto new_end = std::unique(fine_path.begin(), fine_path.end());
	fine_path.erase(new_end, fine_path.end());


	return fine_path;
}

void Head::GenerateCoordinateSystem()
{
	// Find the cloeset vertices to each landmark
	LandmarksToClosestVertex();
	GenerateRays();
	CastRays();

	const auto& vertices = scalp_mesh->vertices;
	const auto& indices = scalp_mesh->indices;
	const auto& local_matrix = transform->GetMatrix();

	std::vector<glm::vec3> hs_vertices;
	hs_vertices.reserve(vertices.size()); // Vertices to head space
	for (const auto& v : vertices) {
		hs_vertices.push_back(glm::vec3(glm::translate(local_matrix, v.position)[3]));
	}

	std::vector<unsigned int> nz_iz_rough_path;
	nz_iz_rough_path.reserve(nz_iz_waypoints.size() + 2);
	nz_iz_rough_path.push_back(lm_closest_vert_idx_map[NAISON]);
	for (auto it = nz_iz_waypoints.rbegin(); it != nz_iz_waypoints.rend(); ++it) {
		nz_iz_rough_path.push_back((*it)->vertex_index);
	}
	nz_iz_rough_path.push_back(lm_closest_vert_idx_map[INION]);
	
	
	std::vector<unsigned int> lpa_rpa_rough_path;
	lpa_rpa_rough_path.reserve(lpa_rpa_waypoints.size() + 2);
	lpa_rpa_rough_path.push_back(lm_closest_vert_idx_map[LPA]);
	for (auto it = lpa_rpa_waypoints.rbegin(); it != lpa_rpa_waypoints.rend(); ++it) {
		lpa_rpa_rough_path.push_back((*it)->vertex_index);
	}
	lpa_rpa_rough_path.push_back(lm_closest_vert_idx_map[RPA]);
	
	std::vector<unsigned int> nz_iz_fine_path = FindFinePath(nz_iz_rough_path);
	std::vector<unsigned int> lpa_rpa_fine_path = FindFinePath(lpa_rpa_rough_path);
	
	std::for_each(nz_iz_path.begin(), nz_iz_path.end(), [](Line* line) {delete line; });
	nz_iz_path.clear();
	//nz_iz_path.reserve(nz_iz_fine_path.size() > 0 ? nz_iz_fine_path.size() - 1 : 0);
	for (size_t i = 0; i < nz_iz_fine_path.size() - 1; ++i) {
		// Use const auto& for safe and efficient access to vertices
		const auto& p1 = hs_vertices[nz_iz_fine_path[i]];
		const auto& p2 = hs_vertices[nz_iz_fine_path[i + 1]];
	
		// Create and store the new line object
		nz_iz_path.push_back(new Line(p1, p2, glm::vec3(1.0f, 0.0f, 0.0f), 3.0f, line_shader));
	}
	
	std::for_each(lpa_rpa_path.begin(), lpa_rpa_path.end(), [](Line* line) {delete line; });
	lpa_rpa_path.clear();
	//lpa_rpa_path.reserve(lpa_rpa_fine_path.size() > 0 ? lpa_rpa_fine_path.size() - 1 : 0);
	for (size_t i = 0; i < nz_iz_fine_path.size() - 1; ++i) {
		// Use const auto& for safe and efficient access to vertices
		const auto& p1 = hs_vertices[lpa_rpa_fine_path[i]];
		const auto& p2 = hs_vertices[lpa_rpa_fine_path[i + 1]];
	
		// Create and store the new line object
		lpa_rpa_path.push_back(new Line(p1, p2, glm::vec3(1.0f, 0.0f, 0.0f), 3.0f, line_shader));
	}

}
