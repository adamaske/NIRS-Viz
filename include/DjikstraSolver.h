#pragma once

#include <unordered_map>
#include <unordered_set>
#include <queue>

#include <spdlog/spdlog.h>
#include <glm/glm.hpp>
#include <glm/geometric.hpp>
#include "glm/gtx/string_cast.hpp"

#include "Mesh.h"

struct Edge {
	unsigned int destination_index;
	float weight;
};

struct PairHash {
	template <class T1, class T2>
	std::size_t operator () (const std::pair<T1, T2>& p) const {
		auto h1 = std::hash<T1>{}(p.first);
		auto h2 = std::hash<T2>{}(p.second);
		return h1 ^ (h2 << 1);
	}
};

using AdjacencyList = std::vector<Edge>;
using Graph = std::vector<AdjacencyList>;
using EdgeKey = std::pair<unsigned int, unsigned int>;

struct DijkstraNode {
	float distance;
	unsigned int index;

	bool operator>(const DijkstraNode& other) const {
		return distance > other.distance;
	}
};

std::vector<unsigned int> DjikstraShortestPath(const Graph& graph, unsigned int start_index, unsigned int end_index) {

	if (start_index >= graph.size() || end_index >= graph.size()) {
		spdlog::error("ShortestPath : Out of bounds start or end indecies. Graph size: {}, Start Index: {}, End Index: {}", graph.size(), start_index, end_index);
		return {};
	}

	unsigned int num_vertices = graph.size();
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

Graph CreateGraphFromTriangleMesh(const Mesh& mesh, const glm::mat4 local_matrix) {

	const auto& vertices = mesh.vertices;
	const auto& indices = mesh.indices;

	unsigned int num_vertices = vertices.size();
	Graph graph(num_vertices);

	auto calculate_distance = [&](unsigned int idx1, unsigned int idx2) -> float {
		const glm::vec3& p1 = vertices[idx1].position;
		const glm::vec3& p2 = vertices[idx2].position;
		return glm::distance(p1, p2);
		};

	auto calculate_weighted_cost = [&](unsigned int idx1, unsigned int idx2) -> float {
		// Vertices is assumed to be available via capture [&]
		const glm::vec3& p1 = vertices[idx1].position;
		const glm::vec3& p2 = vertices[idx2].position;

		// The cost is the direct Euclidean distance between the two connected vertices.
		float actual_distance = glm::distance(p1, p2);

		// No penalty or constraint is applied.
		float weighted_cost = actual_distance;

		return weighted_cost;
	};


	std::unordered_set<EdgeKey, PairHash> processed_edges;

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
				graph[u].push_back({ v, weight });

				// v -> u
				graph[v].push_back({ u, weight });
			}
		}
	}

	return graph;
}

bool ValidateGraph(const Graph& graph, int start_idx, int end_idx, int num_vertices)
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