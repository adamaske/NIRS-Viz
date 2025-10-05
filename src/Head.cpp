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

	auto local_matrix = transform->GetMatrix();
	scalp_mesh_graph = CreateGraphFromTriangleMesh(*scalp_mesh, local_matrix);
	
	landmark_shader = new Shader(std::string("C:/dev/NIRS-Viz/data/Shaders/Landmark.vert"), std::string("C:/dev/NIRS-Viz/data/Shaders/Landmark.frag"));
	landmark_mesh = new Mesh("C:/dev/NIRS-Viz/data/landmark.obj");

	line_shader = new Shader(std::string("C:/dev/NIRS-Viz/data/Shaders/Line.vert"), std::string("C:/dev/NIRS-Viz/data/Shaders/Line.frag"));

	nz_iz_waypoint_renderer = new PointRenderer(15, {1, 0, 0, 1});
	lpa_rpa_waypoint_renderer = new PointRenderer(15, {0, 1, 0, 1});

	refpts_renderer = new PointRenderer(15, { 0.5, 0.9, 0.2, 1 });


	for (auto& lm : landmarks) {
		lm->transform->Scale(glm::vec3(5.0f, 5.0f, 5.0f));
	}
	float lm_dist = 120.0;
	naison.transform->Translate(0, 0, -lm_dist);
	inion.transform	->Translate(0, 0, lm_dist);
	lpa.transform	->Translate(-lm_dist, 0, 0);
	rpa.transform	->Translate(lm_dist, 0, 0);

	naison_inion_line = new Line(naison.transform->position, inion.transform->position, glm::vec3(1, 1, 0), 2.0f, line_shader);
	ear_to_ear_line =	new Line(lpa.transform->position, rpa.transform->position, glm::vec3(0, 1, 1), 2.0f, line_shader);

	hs_vertices.clear(); // Cahces head-space vertex positions
	for (const auto& v : scalp_mesh->vertices) {
		hs_vertices.push_back(glm::vec3(glm::translate(local_matrix, v.position)[3]));
	}
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

	ImGui::Checkbox("Draw Landmarks", &draw_landmarks);
	ImGui::Checkbox("Draw Landmark Lines", &draw_landmark_lines);
	ImGui::Checkbox("Draw Waypoints", &draw_waypoints);
	ImGui::Checkbox("Draw Rays", &draw_rays);
	ImGui::Checkbox("Draw Paths", &draw_paths);
	ImGui::Checkbox("Draw Reference Points", &draw_refpts);

	DrawScalp(view, proj, view_pos);
	if (draw_landmarks) DrawLandmarks(view, proj, view_pos);
	if (draw_waypoints) DrawWaypoints(view, proj);
	if (draw_landmark_lines) DrawLandmarkLines(view, proj, view_pos);
	if (draw_rays) DrawRays(view, proj, view_pos);
	if (draw_paths) DrawPaths(view, proj, view_pos);
	if (draw_refpts) refpts_renderer->Draw(view, proj);
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

	glEnable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glLineWidth(0.5f);

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
	for (auto& line : nz_iz_ray_lines) {
		line->Draw(view, proj);
	}
	for (auto& line : lpa_rpa_ray_lines) {
		line->Draw(view, proj);
	}
}
void Head::DrawPaths(glm::mat4 view, glm::mat4 proj, glm::vec3 veiw_pos)
{
	for (auto& line : nz_iz_path_lines) {
		line->Draw(view, proj);
	}
	for (auto& line : lpa_rpa_path_lines) {
		line->Draw(view, proj);
	}
	for(auto& line : horizontal_arc_path_lines) {
		line->Draw(view, proj);
	}
}

void Head::DrawWaypoints(const glm::mat4& view, const glm::mat4& proj)
{
	nz_iz_waypoint_renderer->Draw(view, proj);
	lpa_rpa_waypoint_renderer->Draw(view, proj);
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
	//Force symmetry between LPA and RPA 
	if (type == LPA) {
		//Set Rpa to be the mirrored position of LPA

		rpa.transform->SetPosition(glm::vec3(-position.x, position.y, position.z));
		ear_to_ear_line->dirty = true;
	}else if(type == RPA) {
		lpa.transform->SetPosition(glm::vec3(-position.x, position.y, position.z));
		ear_to_ear_line->dirty = true;
	}
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


	for (auto& line : nz_iz_ray_lines) delete line;
	for (auto& line : lpa_rpa_ray_lines) delete line;
	nz_iz_ray_lines.clear();
	lpa_rpa_ray_lines.clear();

	for (auto& ray : nz_iz_rays) {
		nz_iz_ray_lines.push_back(new Line(ray.origin, ray.endpoint, glm::vec3(0, 0, 1), 2, line_shader));
	}
	for (auto& ray : lpa_rpa_rays) {
		lpa_rpa_ray_lines.push_back(new Line(ray.origin, ray.endpoint, glm::vec3(0, 1, 0), 2, line_shader));
	}
}

std::vector<unsigned int> Head::CastRays(std::vector<Ray> rays, PointRenderer* wp_render)
{
	const auto& indices = scalp_mesh->indices;
	const auto& vertices = scalp_mesh->vertices;
	const auto& local_matrix = transform->GetMatrix();

	std::vector<unsigned int> path;

	for (const auto& ray : rays) {

		glm::vec3 ray_origin = ray.origin;
		glm::vec3 ray_endpoint = ray.endpoint;
		glm::vec3 ray_direction = glm::normalize(ray_endpoint - ray_origin);

		RayHit best_hit;

		for (unsigned int i = 0; i < indices.size(); i += 3) {
			glm::vec3 v0 = hs_vertices[indices[i + 0]];
			glm::vec3 v1 = hs_vertices[indices[i + 1]];
			glm::vec3 v2 = hs_vertices[indices[i + 2]];

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
			path.push_back(closest_vertex_index);
		}
	}

	// Update Waypoint Renderer
	wp_render->Clear();
	for (unsigned int i = 0; i < path.size(); i++) {
		wp_render->InsertPoint(hs_vertices[path[i]]);
	}

	return path;
}

std::vector<unsigned int> Head::FindFinePath(std::vector<unsigned int> rough_path)
{
	std::vector<unsigned int> fine_path;

	for (size_t i = 0; i < rough_path.size() - 1; ++i) {
		auto segment = DjikstraShortestPath(scalp_mesh_graph, rough_path[i], rough_path[i + 1]);
		fine_path.insert(fine_path.end(), segment.begin(), segment.end());
	}
	
	auto new_end = std::unique(fine_path.begin(), fine_path.end());
	fine_path.erase(new_end, fine_path.end());


	return fine_path;
}

void Head::GenerateCoordinateSystem()
{
	LandmarksToClosestVertex();
	NZFirstMethod();
	LPASecondMethod();

	// Now we have The saggital plane : Nz to Iz path
	// And T3, C3, C4, T4.

	// I have 3D points for all these:
	std::unordered_map<std::string, unsigned int> closest_vert_ind_map;

	std::vector<std::string> keys = { "FpZ", "T3", "Oz", "T4"};
	for(unsigned int i = 0; i < keys.size(); i++) {
		auto point = point_label_map[keys[i]];
		
		int closest_vert_index = 0;
		float min_dist_sq = std::numeric_limits<float>::max();
		for(unsigned int j = 0; j < hs_vertices.size(); j++) {
			float dist_sq = glm::distance2(point, hs_vertices[j]);
			if(dist_sq < min_dist_sq) {
				min_dist_sq = dist_sq;
				closest_vert_index = j;
			}
		}
		closest_vert_ind_map[keys[i]] = closest_vert_index;
	}
	//
	horizontal_arc_rough_path_indices = { closest_vert_ind_map["FpZ"], closest_vert_ind_map["T3"], closest_vert_ind_map["Oz"], closest_vert_ind_map["T4"],   closest_vert_ind_map["FpZ"] };
	horizontal_arc_fine_path_indices = FindFinePath(horizontal_arc_rough_path_indices);

	UpdatePathLines(&horizontal_arc_path_lines, horizontal_arc_fine_path_indices, glm::vec3(1, 0, 0), 3.0f);

	auto labels = std::vector<std::string>{ "Fp1", "F7", "T5", "O1", "02", "T6", "F8", "Fp2"};
	auto percentages = std::vector<float>{ 0.05, 0.15, 0.35, 0.45, 0.55, 0.65, 0.85, 0.95};
	GetReferencePointsAlongPath(horizontal_arc_fine_path_indices, labels, percentages, refpts_renderer);
}

void Head::UpdatePathLines(std::vector<Line*>* old_lines, const std::vector<unsigned int>& new_indcies, const glm::vec3 color, const float& thickness) {
	std::for_each(old_lines->begin(), old_lines->end(), [](Line* line) {delete line; });
	old_lines->clear();
	for (size_t i = 0; i < new_indcies.size() - 1; ++i) {
		const auto& p1 = hs_vertices[new_indcies[i]];
		const auto& p2 = hs_vertices[new_indcies[i + 1]];
		old_lines->push_back(new Line(p1, p2, glm::vec3(1.0f, 0.0f, 0.0f), 3.0f, line_shader));
	}
}

void Head::GetReferencePointsAlongPath( const std::vector<unsigned int>& fine_path_indices, 
										const std::vector<std::string>& labels, 
										const std::vector<float>& percentages, 
										PointRenderer* renderer)
{
	std::vector<float> cumulative_distances;
	float total_distance = 0.0f;
	cumulative_distances.push_back(0.0f);

	for (size_t i = 0; i < fine_path_indices.size() - 1; i++) {
		float segment_dist = glm::distance(hs_vertices[fine_path_indices[i]], hs_vertices[fine_path_indices[i + 1]]);
		total_distance += segment_dist;
		cumulative_distances.push_back(total_distance);
	}

	auto get_distance_by_percentage = [total_distance](float percentage) -> float {
		if (percentage < 0.0f) percentage = 0.0f;
		if (percentage > 1.0f) percentage = 1.0f;

		return total_distance * percentage;
	};


	for (size_t i = 0; i < labels.size(); i++)
	{
		float target_distance = get_distance_by_percentage(percentages[i]);
		glm::vec3 point(0, 0, 0);

		if (percentages[i] == 0.0f) {

			point = hs_vertices[fine_path_indices.front()];

			renderer->InsertPoint(point);
			point_label_map[labels[i]] = point;
			continue;
		}

		if (percentages[i] == 1.0f) {
			point = hs_vertices[fine_path_indices.back()];

			renderer->InsertPoint(point);
			point_label_map[labels[i]] = point;
			continue;
		}

		for (size_t j = 0; j < cumulative_distances.size() - 1; j++) {
			if (target_distance >= cumulative_distances[j] && target_distance <= cumulative_distances[j + 1]) {

				float start_dist = cumulative_distances[j];
				float segment_length = cumulative_distances[j + 1] - cumulative_distances[j];
				float remaining_distance = target_distance - start_dist;
				float ratio = remaining_distance / segment_length;

				glm::vec3 v_start = hs_vertices[fine_path_indices[j]];
				glm::vec3 v_end = hs_vertices[fine_path_indices[j + 1]];

				point = glm::mix(v_start, v_end, ratio);
				point_label_map[labels[i]] = point;
				renderer->InsertPoint(point);
				break;
			}
		}
	}
}


void Head::NZFirstMethod()
{
	const auto& vertices = scalp_mesh->vertices;
	const auto& indices = scalp_mesh->indices;
	const auto& local_matrix = transform->GetMatrix();

	const auto& nz = vertices[lm_closest_vert_idx_map[NAISON]].position;
	const auto& iz = vertices[lm_closest_vert_idx_map[INION]].position;

	// Generate Rays
	const auto& nz_iz_midpoint = glm::translate(local_matrix, glm::vec3((nz + iz) / 2.0f))[3];
	const auto& nz_iz_direction = glm::normalize(iz - nz);
	const auto& nz_iz_rotation_axis = glm::normalize(glm::cross(nz_iz_direction, glm::vec3(0, 1, 0)));

	nz_iz_rays.clear();
	nz_iz_rays = GenerateSweepingArchRays(nz_iz_midpoint,
		nz_iz_direction,
		nz_iz_rotation_axis,
		ray_distance,
		theta_step_size,
		180.0f,
		theta_step_size);

	// Update Rays
	for (auto& line : nz_iz_ray_lines) delete line;
	nz_iz_ray_lines.clear();
	for (auto& ray : nz_iz_rays) {
		nz_iz_ray_lines.push_back(new Line(ray.origin, ray.endpoint, glm::vec3(0, 0, 1), 2, line_shader));
	}

	// Cast Rays
	nz_iz_rough_path_indices = CastRays(nz_iz_rays, nz_iz_waypoint_renderer);

	// Find Fine Path
	std::vector<unsigned int> nz_iz_rough_path;
	nz_iz_rough_path.reserve(nz_iz_rough_path_indices.size() + 2);
	nz_iz_rough_path.push_back(lm_closest_vert_idx_map[NAISON]);
	for (auto it = nz_iz_rough_path_indices.rbegin(); it != nz_iz_rough_path_indices.rend(); ++it) {
		nz_iz_rough_path.push_back((*it));
	}
	nz_iz_rough_path.push_back(lm_closest_vert_idx_map[INION]);
	nz_iz_fine_path_indices = FindFinePath(nz_iz_rough_path);

	// Update Path Lines
	UpdatePathLines(&nz_iz_path_lines, nz_iz_fine_path_indices, glm::vec3(1.0f, 0.0f, 0.0f), 3.0f);

	// Reset refpts
	refpts_renderer->Clear();
	point_label_map.clear();

	std::vector<std::string> labels = { "Nz", "FpZ", "Fz", "Cz", "Pz", "Oz", "Iz" };
	std::vector<float> percentages = { 0.0f, 0.10f, 0.30f, 0.50f, 0.70f, 0.90f, 1.0f };
	GetReferencePointsAlongPath(nz_iz_fine_path_indices, labels, percentages, refpts_renderer);
}

void Head::LPASecondMethod()
{
	const auto& vertices = scalp_mesh->vertices;
	const auto& indices = scalp_mesh->indices;
	const auto& local_matrix = transform->GetMatrix();
	const auto& lpa = vertices[lm_closest_vert_idx_map[LPA]].position;
	const auto& rpa = vertices[lm_closest_vert_idx_map[RPA]].position;


	const auto& cz_position = point_label_map["Cz"];
	const glm::vec3& lpa_rpa_midpoint = glm::vec3((lpa + rpa) / 2.0f);
	const glm::vec3& lpa_rpa_direction = glm::normalize(rpa - lpa);
	const glm::vec3& cz = glm::vec3(lpa_rpa_midpoint.x, cz_position.y, cz_position.z);
	const glm::vec3& up_vector = glm::normalize(cz - lpa_rpa_midpoint);
	const glm::vec3& rotation_axis = glm::normalize(glm::cross(lpa_rpa_direction, up_vector));
	const glm::vec3& new_direction = glm::normalize(glm::cross(rotation_axis, -up_vector));
	
	lpa_rpa_rays.clear();
	lpa_rpa_rays = GenerateSweepingArchRays(lpa_rpa_midpoint,
		new_direction,
		rotation_axis,
		ray_distance,
		theta_step_size,
		180.0f,
		theta_step_size);


	for (auto& line : lpa_rpa_ray_lines) delete line;
	lpa_rpa_ray_lines.clear();

	for (auto& ray : lpa_rpa_rays) {
		lpa_rpa_ray_lines.push_back(new Line(ray.origin, ray.endpoint, glm::vec3(0, 1, 0), 2, line_shader));
	}

	// Cast Rays
	lpa_rpa_rough_path_indices = CastRays(lpa_rpa_rays, lpa_rpa_waypoint_renderer);

	// Find Fine Path
	std::vector<unsigned int> lpa_rpa_rough_path;
	lpa_rpa_rough_path.reserve(lpa_rpa_rough_path_indices.size() + 2);
	lpa_rpa_rough_path.push_back(lm_closest_vert_idx_map[LPA]);
	for (auto it = lpa_rpa_rough_path_indices.rbegin(); it != lpa_rpa_rough_path_indices.rend(); ++it) {
		lpa_rpa_rough_path.push_back((*it));
	}
	lpa_rpa_rough_path.push_back(lm_closest_vert_idx_map[RPA]);
	lpa_rpa_fine_path_indices = FindFinePath(lpa_rpa_rough_path);

	// Update Path Lines
	UpdatePathLines(&lpa_rpa_path_lines, lpa_rpa_fine_path_indices, glm::vec3(1.0f, 0.0f, 0.0f), 3.0f);

	std::vector<std::string> labels = { "LPA", "T3", "C3", "C4", "T4", "RPA" };
	std::vector<float> percentages = { 0.0f, 0.10f, 0.30f, 0.70f, 0.90f, 1.0f };
	GetReferencePointsAlongPath(lpa_rpa_fine_path_indices, labels, percentages, refpts_renderer);
}
