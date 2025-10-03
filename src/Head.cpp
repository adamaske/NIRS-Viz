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

#include "RayIntersection.h"
#include "DjikstraSolver.h"


Head::Head()
{
	shader = new Shader(std::string("C:/dev/NIRS-Viz/data/Shaders/Phong.vert"), std::string("C:/dev/NIRS-Viz/data/Shaders/Phong.frag"));
	scalp_mesh = new Mesh("C:/dev/NIRS-Viz/data/example_scalp.obj");
	transform = new Transform();

	landmark_shader = new Shader(std::string("C:/dev/NIRS-Viz/data/Shaders/Landmark.vert"), std::string("C:/dev/NIRS-Viz/data/Shaders/Landmark.frag"));
	landmark_mesh = new Mesh("C:/dev/NIRS-Viz/data/landmark.obj");

	line_shader = new Shader(std::string("C:/dev/NIRS-Viz/data/Shaders/Line.vert"), std::string("C:/dev/NIRS-Viz/data/Shaders/Line.frag"));

	waypoint_shader = landmark_shader;
	waypoint_mesh = landmark_mesh;

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
	glLineWidth(0.7f);
	
	glBindVertexArray(scalp_mesh->VAO);
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(scalp_mesh->indices.size()), GL_UNSIGNED_INT, 0);
	
	glBindVertexArray(0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


	DrawLandmarks(view, proj, view_pos);
	DrawWaypoints(view, proj, view_pos);
	DrawLines(view, proj, view_pos);
}

void Head::DrawLines(glm::mat4 view, glm::mat4 proj, glm::vec3 veiw_pos)
{
	naison_inion_line->Draw(view, proj);
	ear_to_ear_line->Draw(view, proj);

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

	if (ImGui::Button("Generate Coordinates"))
	{
		GenerateCoordinateSystem();
	};

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

	
	ImGui::End();
}

std::unordered_map<LandmarkType, unsigned int> Head::LandmarksToClosestVertex()
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

		//spdlog::info("Landmark {} closest vertex index: {}, position: {}, distance: {}", 
		//	landmark_labels[lm->type], closest_idx, glm::to_string(hs_vertices[closest_idx]), sqrt(min_dist));
	}

	return lm_closest_vert_idx;
}

void Head::CastRays()
{
	auto closest_indices_map = LandmarksToClosestVertex();

	const auto& vertices = scalp_mesh->vertices;
	const auto& indices = scalp_mesh->indices;
	const auto& local_matrix = transform->GetMatrix();

	auto nz  = vertices[closest_indices_map[NAISON]].position;
	auto iz  = vertices[closest_indices_map[INION]].position;
	auto lpa = vertices[closest_indices_map[LPA]].position;
	auto rpa = vertices[closest_indices_map[RPA]].position;

	auto nz_iz_direction = glm::normalize(iz - nz);
	auto lpa_rpa_direction = glm::normalize(rpa - lpa);

	std::vector<unsigned int> nz_iz_rough_path_indices; // Rough path 
	std::vector<unsigned int> lpa_rpa_rough_path_indices; // Rough path 

	auto nz_iz_midpoint = (nz + iz) / 2.0f;
	auto lpa_rpa_midpoint = (lpa + rpa) / 2.0f;

	nz_iz_midpoint = glm::translate(transform->GetMatrix(), nz_iz_midpoint)[3];
	lpa_rpa_midpoint = glm::translate(transform->GetMatrix(), lpa_rpa_midpoint)[3];

	auto nz_iz_rotation_axis = glm::normalize(glm::cross(nz_iz_direction, glm::vec3(0, 1, 0)));
	auto lpa_rpa_rotation_axis = glm::normalize(glm::cross(lpa_rpa_direction, glm::vec3(0, 1, 0)));

	std::vector<std::tuple<glm::vec3, glm::vec3>> nz_iz_rays; // {ray_origin, ray_endpoint}
	std::vector<std::tuple<glm::vec3, glm::vec3>> lpa_rpa_rays;

	for (theta_deg = theta_step_size; theta_deg < 180.0f; theta_deg += theta_step_size) {
		//Cast NZ-IZ
		auto nz_rotation_quat = glm::angleAxis(glm::radians(theta_deg), nz_iz_rotation_axis);
		auto lpa_rotation_quat = glm::angleAxis(glm::radians(theta_deg), lpa_rpa_rotation_axis);

		auto nz_ray_direction = nz_rotation_quat * nz_iz_direction;
		auto lpa_ray_direction = lpa_rotation_quat * lpa_rpa_direction;

		auto nz_ray_endpoint = nz_iz_midpoint + nz_ray_direction * ray_distance;
		auto lpa_ray_endpoint = lpa_rpa_midpoint + lpa_ray_direction * ray_distance;

		nz_iz_rays.push_back(std::make_tuple(nz_iz_midpoint, nz_ray_endpoint));
		lpa_rpa_rays.push_back(std::make_tuple(lpa_rpa_midpoint, lpa_ray_endpoint));
	}


	for (auto& line : nz_iz_path) delete line;
	for (auto& line : lpa_rpa_path) delete line;
	nz_iz_path.clear();
	lpa_rpa_path.clear();

	for (auto& ray : nz_iz_rays) {
		auto& [ray_start, ray_end] = ray;
		nz_iz_path.push_back(new Line(ray_start, ray_end, glm::vec3(0, 0, 1), 2, line_shader));
	}
	for (auto& ray : lpa_rpa_rays) {
		auto& [ray_start, ray_end] = ray;
		lpa_rpa_path.push_back(new Line(ray_start, ray_end, glm::vec3(0, 1, 0), 2, line_shader));
	}


	spdlog::info("Casting {} rays for Nasion-Inion and {} rays for LPA-RPA", nz_iz_rays.size(), lpa_rpa_rays.size());
	for (const auto& ray_tuple : nz_iz_rays) {

		glm::vec3 ray_origin = std::get<0>(ray_tuple);
		glm::vec3 ray_endpoint = std::get<1>(ray_tuple);
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
			float min_dist_sq = glm::distance2(intersection_point, vertices[best_hit.hit_v0].position);

			// Check v1
			float dist_sq_v1 = glm::distance2(intersection_point, vertices[best_hit.hit_v1].position);
			if (dist_sq_v1 < min_dist_sq) {
				min_dist_sq = dist_sq_v1;
				closest_vertex_index = best_hit.hit_v1;
			}

			// Check v2
			float dist_sq_v2 = glm::distance2(intersection_point, vertices[best_hit.hit_v2].position);
			if (dist_sq_v2 < min_dist_sq) {
				closest_vertex_index = best_hit.hit_v2;
			}

			// Add this closest vertex as a rough waypoint
			nz_iz_rough_path_indices.push_back(closest_vertex_index);
		}
	}
	for (const auto& ray_tuple : lpa_rpa_rays) {

		glm::vec3 ray_origin = std::get<0>(ray_tuple);
		glm::vec3 ray_endpoint = std::get<1>(ray_tuple);
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
			float min_dist_sq = glm::distance2(intersection_point, vertices[best_hit.hit_v0].position);

			// Check v1
			float dist_sq_v1 = glm::distance2(intersection_point, vertices[best_hit.hit_v1].position);
			if (dist_sq_v1 < min_dist_sq) {
				min_dist_sq = dist_sq_v1;
				closest_vertex_index = best_hit.hit_v1;
			}

			// Check v2
			float dist_sq_v2 = glm::distance2(intersection_point, vertices[best_hit.hit_v2].position);
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
		unsigned int waypoint_index = i;
		auto& vertex_index = nz_iz_rough_path_indices[i];
		glm::vec3 hs_position = glm::translate(local_matrix, vertices[vertex_index].position)[3];
		nz_iz_waypoints.push_back(new Waypoint{ waypoint_index, vertex_index, hs_position });
	}

	for (unsigned int i = 0; i < lpa_rpa_rough_path_indices.size(); i++) {
		unsigned int waypoint_index = i;
		auto& vertex_index = lpa_rpa_rough_path_indices[i];
		glm::vec3 hs_position = glm::translate(local_matrix, vertices[vertex_index].position)[3];
		lpa_rpa_waypoints.push_back(new Waypoint{ waypoint_index, vertex_index, hs_position });
	}
}

void Head::GenerateCoordinateSystem()
{
	// Find the cloeset vertices to each landmark
	CastRays();

	return;


	//std::vector<unsigned int> lpa_rpa_rough_path_indices = { closest_indices_map[LPA] }; // Rough path 

	// For example if nz_iz_direction is (0, 0, 1) then we want to rotate around the x axis and shoot rays
	// For example if lpa_rpa_direction is (1, 0, 0) then we want to rotate around the z axis and shoot rays




	//std::vector<unsigned int> nz_iz_fine_path_indices;
	//std::vector<unsigned int> lpa_rpa_fine_path_indices; // Rough path 


	//for (unsigned int i = 0; i < nz_iz_rough_path_indices.size(); i++) {
	//	// Find the path between these rough waypoints
	//	auto path_segment = DjikstraShortestPath(graph, nz_iz_rough_path_indices[i], nz_iz_rough_path_indices[i+1]);

	//	for(auto& idx : path_segment) {
	//		nz_iz_fine_path_indices.push_back(idx);
	//	}
	//}

	//for(unsigned int i = 0; i < lpa_rpa_rough_path_indices.size(); i++) {
	//	// Find the path between these rough waypoints
	//	auto path_segment = DjikstraShortestPath(graph, lpa_rpa_rough_path_indices[i], lpa_rpa_rough_path_indices[i + 1]);

	//	for(auto& idx : path_segment) {
	//		lpa_rpa_fine_path_indices.push_back(idx);
	//	}
	//}


	//// Check for duplicates
	//for(unsigned int i = 0; i < nz_iz_fine_path_indices.size() - 1; i++) {
	//	if(nz_iz_fine_path_indices[i] == nz_iz_fine_path_indices[i+1]) {
	//		spdlog::warn("Duplicate index found in Naison-Inion path at position {}: {}", i, nz_iz_fine_path_indices[i]);
	//	}
	//}
	//for (unsigned int i = 0; i < lpa_rpa_fine_path_indices.size() - 1; i++) {
	//	if (lpa_rpa_fine_path_indices[i] == lpa_rpa_fine_path_indices[i + 1]) {
	//		spdlog::warn("Duplicate index found in Naison-Inion path at position {}: {}", i, nz_iz_fine_path_indices[i]);
	//	}
	//}
	//return;
	//auto nz_iz = DjikstraShortestPath(graph, closest_indices_map[NAISON], closest_indices_map[INION]);
	//auto lpa_rpa = DjikstraShortestPath(graph, closest_indices_map[LPA], closest_indices_map[RPA]);
	//
	//spdlog::info("Naison to Inion path length: {}", nz_iz.size());
	//spdlog::info("LPA to RPA path length: {}", lpa_rpa.size());
	//
	//const auto& vertices = scalp_mesh->vertices;
	//// Generate Lines for the tracks
	//
	//nz_iz_path.clear();
	//for (unsigned int i = 0; i < nz_iz.size() - 1; i++) {
	//	const glm::vec3& p1 = vertices[nz_iz[i]].position;
	//	const glm::vec3& p2 = vertices[nz_iz[i + 1]].position;
	//
	//	// to head space
	//	auto& p1_hs = glm::vec3(glm::translate(transform->GetMatrix(), p1)[3]);
	//	auto& p2_hs = glm::vec3(glm::translate(transform->GetMatrix(), p2)[3]);
	//
	//	Line* segment = new Line(p1_hs, p2_hs, glm::vec3(1, 0, 0), 3.0f, line_shader);
	//	nz_iz_path.push_back(segment);
	//}
	//
	//
	//lpa_rpa_path.clear();
	//for (unsigned int i = 0; i < lpa_rpa.size() - 1; i++) {
	//	const glm::vec3& p1 = vertices[lpa_rpa[i]].position;
	//	const glm::vec3& p2 = vertices[lpa_rpa[i + 1]].position;
	//	// to head space
	//	auto& p1_hs = glm::vec3(glm::translate(transform->GetMatrix(), p1)[3]);
	//	auto& p2_hs = glm::vec3(glm::translate(transform->GetMatrix(), p2)[3]);
	//	Line* segment = new Line(p1_hs, p2_hs, glm::vec3(0, 1, 0), 3.0f, line_shader);
	//	lpa_rpa_path.push_back(segment);
	//}
}
