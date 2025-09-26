#include "ReferencePoints.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <spdlog/spdlog.h>
#include <string>
#include <glad/glad.h>


ReferencePoints::ReferencePoints(const fs::path& points_file, const fs::path& labels_file)
{
	labels.clear();
	points.clear();

	// Points
	std::ifstream points_stream(points_file);
	if (!points_stream.is_open()) {
		spdlog::error("Could not open points file: {}", points_file.string());
		return;
	}

	std::string points_line;
	while (std::getline(points_stream, points_line)) {
		if (points_line.empty()) continue; // skip empty lines

		std::istringstream ss(points_line);
		float x, y, z;

		ss >> x >> y >> z;
		if (ss.fail()) {
			spdlog::error("Failed to parse line: ", points_line);
			continue;
		}

		points.emplace_back(x, y, z);
	}

	// Labels 
	// for each line in labels file, read line and push to labels vector
	std::ifstream labels_stream(labels_file);
	if (!labels_stream.is_open()) {
		spdlog::error("Could not open labels file: {}", labels_file.string());
		return;
	}
	std::string label_line;
	while (std::getline(labels_stream, label_line)) {
		labels.push_back(label_line);
	}
	labels_stream.close();

	spdlog::info("Loaded {} reference points and {} labels.", points.size(), labels.size());
	//auto n = 20;
	//for(int i = 0; i < n; i++ )
	//{
	//	spdlog::info("{} : {}, {}, {}", labels[i], points[i].x, points[i].y, points[i].z);
	//}
	shader = new Shader("C:/dev/NeuroVisualizer/data/Shaders/refpoints.vert", "C:/dev/NeuroVisualizer/data/Shaders/refpoints.frag");

	glGenVertexArrays(1, &pointVAO);
	glGenBuffers(1, &pointVBO);

	glBindVertexArray(pointVAO);
	glBindBuffer(GL_ARRAY_BUFFER, pointVBO);
	glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(glm::vec3), points.data(), GL_STATIC_DRAW);

	// position attribute
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

	glBindVertexArray(0);
}

void ReferencePoints::Draw(const glm::mat4& view, const glm::mat4& projection, const glm::mat4& transformation)
{
	if (!shader) return;
	
	shader->Bind(); 
	shader->SetUniformMat4f("model", transformation);
	shader->SetUniformMat4f("view", view);
	shader->SetUniformMat4f("projection", projection);

	glBindVertexArray(pointVAO);
	glPointSize(8.0f); 
	glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(points.size()));
	glBindVertexArray(0);
}
