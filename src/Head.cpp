#include "Head.h"
#include <glm/glm.hpp>
#include <glm/geometric.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <unordered_map>
#include <unordered_set>
#include <Queue>
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
	
	glBindVertexArray(scalp_mesh->VAO);
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(scalp_mesh->indices.size()), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	shader->Unbind(); 
	
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
	for (auto& line : nz_iz_path) {
		line->Draw(view, proj);
	}
	for (auto& line : lpa_rpa_path) {
		line->Draw(view, proj);
	}
	if (ImGui::Button("Generate Coordinates"))
	{
		GenerateCoordinateSystem();
	};
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

		spdlog::info("Landmark {} closest vertex index: {}, position: {}, distance: {}", 
			landmark_labels[lm->type], closest_idx, glm::to_string(hs_vertices[closest_idx]), sqrt(min_dist));
	}

	return lm_closest_vert_idx;
}

std::vector<unsigned int> Head::ShortestPathOnScalp(const Graph& graph, unsigned int start_index, unsigned int end_index)
{
	if(start_index >= graph.size() || end_index >= graph.size()) {
		spdlog::error("ShortestPath : Out of bounds start or end indecies. Graph size: {}, Start Index: {}, End Index: {}", graph.size(), start_index, end_index);
		return {};
	}

	unsigned int num_vertices = graph.size();
	spdlog::info("Graph size: {}, Vertices.size() : {}", num_vertices, scalp_mesh->vertices.size());
	// Distance array: stores the shortest distance from the start to every other node
	std::vector<float> distance(num_vertices, std::numeric_limits<float>::max());

	// Parent array: stores the predecessor node index for path reconstruction
	std::vector<unsigned int> parent(num_vertices, std::numeric_limits<unsigned int>::max());
	// Priority Queue: Acts as the 'unvisited set' ordered by distance (min-heap)
	// We use std::greater<DijkstraNode> to make it a min-heap based on distance.
	std::priority_queue<DijkstraNode, std::vector<DijkstraNode>, std::greater<DijkstraNode>> pq;


	distance[start_index] = 0.0f;
	pq.push({ 0.0f, start_index });


	while (!pq.empty()) {

		// 3. Select the current node (smallest distance)
		DijkstraNode current_node = pq.top();
		pq.pop();
		unsigned int u_idx = current_node.index;
		float dist_u = current_node.distance;

		if (dist_u > distance[u_idx]) {
			continue;
		}

		if (u_idx == end_index) {
			break;
		}

		// 4. For the current node, consider all of its unvisited neighbors
		for (const auto& edge : graph[u_idx]) {
			unsigned int v_idx = edge.destination_index;
			float weight = edge.weight;

			// Calculate the distance to neighbor v through current node u
			float new_dist = dist_u + weight;

			// Update distance if the new path is shorter
			if (new_dist < distance[v_idx]) {
				distance[v_idx] = new_dist;
				parent[v_idx] = u_idx;

				// Add/update the neighbor in the priority queue (acts as the 'unvisited' update)
				pq.push({ new_dist, v_idx });
			}
		}
	}

	// Check if the end node was reached (distance is finite)
	if (distance[end_index] == std::numeric_limits<float>::max()) {
		// Path not found
		spdlog::error("No path found between vertices {} and {}", start_index, end_index);
		return {};
	}

	// Reconstruct path by tracing back using the parent array
	std::vector<unsigned int> shortest_path;
	unsigned int current = end_index;

	while (current != start_index && current != std::numeric_limits<unsigned int>::max()) {
		shortest_path.push_back(current);
		current = parent[current];
	}

	// Add the start node
	shortest_path.push_back(start_index);

	// The path is currently end-to-start, reverse it to be start-to-end
	std::reverse(shortest_path.begin(), shortest_path.end());

	return shortest_path;
}

Graph Head::CreateGraph(unsigned int start_index, unsigned int end_index)
{

	const auto& vertices = scalp_mesh->vertices;
	const auto& indices = scalp_mesh->indices;
	const glm::mat4& local_matrix = transform->GetMatrix();

	unsigned int num_vertices = vertices.size();
	Graph mesh_graph;
	mesh_graph.resize(num_vertices);

	const glm::vec3& start_pos = vertices[start_index].position;
	const glm::vec3& end_pos = vertices[end_index].position;

	// 2. Calculate the IDEAL vector (the shortcut) and normalize it
	const glm::vec3 ideal_direction = glm::normalize(end_pos - start_pos);
	spdlog::info("Start Position: {}", glm::to_string(start_pos));
	spdlog::info("End Position: {}", glm::to_string(end_pos));
	spdlog::info("Ideal Direction: {}", glm::to_string(ideal_direction));
	// A. Euclidian distance
	auto calculate_distance = [&](unsigned int idx1, unsigned int idx2) -> float {
		const glm::vec3& p1 = vertices[idx1].position;
		const glm::vec3& p2 = vertices[idx2].position;
		return glm::distance(p1, p2);
	};
	auto calculate_weighted_cost = [&](unsigned int idx1, unsigned int idx2) -> float {
		const glm::vec3& p1 = vertices[idx1].position;
		const glm::vec3& p2 = vertices[idx2].position;

		// NOTE: IDEAL_DIRECTION must be calculated once outside this function:
		// const glm::vec3 IDEAL_DIRECTION = glm::normalize(end_pos - start_pos);

		// --- Step 1: Calculate Actual Edge Weight (Length) ---
		float actual_distance = glm::distance(p1, p2);

		// --- Step 2: Calculate Alignment ---
		glm::vec3 edge_vector = glm::normalize(p2 - p1);

		// Alignment is between +1 (perfectly toward end) and -1 (directly away from end).
		float alignment = glm::dot(edge_vector, ideal_direction);

		// --- Step 3: Calculate the Directional Penalty ---

		// Penalty Factor: 1 - Alignment
		// Ranges from 0 (perfectly aligned) to 2 (perfectly opposite)
		float penalty_factor = 1.0f - alignment;

		// Define the strength of the penalty (PUNISHMENT_STRENGTH >= 0)
		// You will need to TUNE this value (start high, e.g., 5.0 to 10.0).
		const float PUNISHMENT_STRENGTH = 8.0f;

		// Directional Cost = Actual Length * Strength * Penalty Factor
		float directional_cost = actual_distance * PUNISHMENT_STRENGTH * penalty_factor;

		// --- Step 4: Final Weighted Cost ---
		// Total Cost = Actual Length + Directional Cost

		// If the edge moves toward the goal (alignment=1), cost is just actual_distance.
		// If the edge moves away from the goal (alignment=-1), cost is much higher.
		float weighted_cost = actual_distance + directional_cost;

		return weighted_cost;
		};
	std::unordered_set<EdgeKey, PairHash> processed_edges;

	// Iterate over each triangle in the mesh
	for (size_t i = 0; i < indices.size(); i += 3) {
		unsigned int v0 = indices[i];
		unsigned int v1 = indices[i + 1];
		unsigned int v2 = indices[i + 2];

		std::vector<std::pair<unsigned int, unsigned int>> triangle_edges = {
			{v0, v1}, {v1, v2}, {v2, v0}
		};

		for (auto& edge_pair : triangle_edges) {
			unsigned int u = edge_pair.first;
			unsigned int v = edge_pair.second;

			//Normalize the pair for set loopkup, i.e (5,1) and (1,5) are the same edge
			unsigned int a = std::min(u, v);
			unsigned int b = std::max(u, v);

			// Is it already processed? 
			if (processed_edges.find({ a, b }) == processed_edges.end()) {
				processed_edges.insert({ a, b });

				float weight = calculate_weighted_cost(u, v);
				// Here we want to add a punishment to straying from a straight line in my desired direciton
				// Lets say the points v0 and v1 are going from (0, 0, -50), to (0, 0, 50), then i want it 
				// To heavily favor staying in this (0, 0, -1/1) direction

				// u -> v
				mesh_graph[u].push_back({ v, weight });

				// v -> u
				mesh_graph[v].push_back({ u, weight });
			}
		}
	}

	return mesh_graph;
}
bool Head::ValidateGraph(const Graph& graph, int start_idx, int end_idx, int num_vertices)
{

	std::vector<bool> visited(num_vertices, false);
	std::queue<unsigned int> q;

	q.push(start_idx);
	visited[start_idx] = true;

	bool is_connected = false;

	while (!q.empty()) {
		unsigned int u = q.front();
		q.pop();

		if (u == end_idx) {
			is_connected = true;
			break;
		}

		for (const auto& edge : graph[u]) {
			if (!visited[edge.destination_index]) {
				visited[edge.destination_index] = true;
				q.push(edge.destination_index);
			}
		}
	}
	
	return is_connected;
}


void Head::GenerateCoordinateSystem()
{
	// Find the cloeset vertices to each landmark
	
	auto closest_indices_map = LandmarksToClosestVertex();

	const auto& vertices = scalp_mesh->vertices;
	unsigned int num_vertices = vertices.size();

	Graph nz_graph = CreateGraph(closest_indices_map[NAISON], closest_indices_map[INION]);
	Graph lpa_graph = CreateGraph(closest_indices_map[LPA], closest_indices_map[RPA]);

	//if (!ValidateGraph(nz_graph, closest_indices_map[NAISON], closest_indices_map[INION], num_vertices)) {
	//	spdlog::error("Graph validation failed between Naison and Inion");
	//}
	//if(!ValidateGraph(lpa_graph, closest_indices_map[LPA], closest_indices_map[RPA], num_vertices)) {
	//	spdlog::error("Graph validation failed between LPA and RPA");
	//}

	// verticies[closest_indices[NAISON]] to verticies[closest_indices[INION]]
	auto nz_iz = ShortestPathOnScalp(nz_graph, closest_indices_map[NAISON], closest_indices_map[INION]);
	// verticies[closest_indices[LPA]] to verticies[closest_indices[RPA]]
	auto lpa_rpa =  ShortestPathOnScalp(lpa_graph, closest_indices_map[LPA], closest_indices_map[RPA]);

	spdlog::info("Naison to Inion path length: {}", nz_iz.size());
	spdlog::info("LPA to RPA path length: {}", lpa_rpa.size());

	// Generate Lines for the tracks

	nz_iz_path.clear();
	for (unsigned int i = 0; i < nz_iz.size() - 1; i++) {
		const glm::vec3& p1 = vertices[nz_iz[i]].position;
		const glm::vec3& p2 = vertices[nz_iz[i + 1]].position;

		// to head space
		auto& p1_hs = glm::vec3(glm::translate(transform->GetMatrix(), p1)[3]);
		auto& p2_hs = glm::vec3(glm::translate(transform->GetMatrix(), p2)[3]);

		Line* segment = new Line(p1_hs, p2_hs, glm::vec3(1, 0, 0), 3.0f, line_shader);
		nz_iz_path.push_back(segment);
	}


	lpa_rpa_path.clear();
	for (unsigned int i = 0; i < lpa_rpa.size() - 1; i++) {
		const glm::vec3& p1 = vertices[lpa_rpa[i]].position;
		const glm::vec3& p2 = vertices[lpa_rpa[i + 1]].position;
		// to head space
		auto& p1_hs = glm::vec3(glm::translate(transform->GetMatrix(), p1)[3]);
		auto& p2_hs = glm::vec3(glm::translate(transform->GetMatrix(), p2)[3]);
		Line* segment = new Line(p1_hs, p2_hs, glm::vec3(0, 1, 0), 3.0f, line_shader);
		lpa_rpa_path.push_back(segment);
	}
}
