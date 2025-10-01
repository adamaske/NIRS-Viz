#pragma once

#include <vector>
#include <string>
#include <filesystem>
#include <glm/glm.hpp>
#include <glad/glad.h>
#include "Vertex.h"



namespace fs = std::filesystem;
class Mesh {
public:
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;

	Mesh(const fs::path& obj_filepat);
	~Mesh();

	void Init();
	void Reset();

	bool LoadObj(const std::string& filename,
		std::vector<Vertex>& vertices,
		std::vector<unsigned int>& indices);//const fs::path& filepath);

	unsigned int VAO;
	unsigned int VBO;
	unsigned int EBO;
};