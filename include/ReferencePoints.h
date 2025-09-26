#pragma once

#include <filesystem>
#include <glm/glm.hpp>
#include <vector>
#include "Shader.h"

namespace fs = std::filesystem;
class ReferencePoints {
public:

	unsigned int pointVBO;
	unsigned int pointVAO;
	std::vector<glm::vec3> points;
	std::vector<std::string> labels;
	Shader* shader;
	
	ReferencePoints(const fs::path& points_file, const fs::path& labels_file);
	
	void Draw(const glm::mat4& view, const glm::mat4& projection, const glm::mat4& transformation);
};