#pragma once


#include <glm/glm.hpp>

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

std::vector<unsigned int> DjikstraShortestPath(const Graph& graph, unsigned int start_index, unsigned int end_index);
Graph CreateGraphFromTriangleMesh(const Mesh& mesh, const glm::mat4 local_matrix);
bool ValidateGraph(const Graph& graph, int start_idx, int end_idx, int num_vertices);
