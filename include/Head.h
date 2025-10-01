#pragma once

#include "Transform.h"
#include "Mesh.h"
#include "Shader.h"
#include "Line.h"

enum class LandmarkType {
	NAISON,
	INION,
	LEFT_EAR,
	RIGHT_EAR
};
struct Landmark {
	LandmarkType type;
	Transform* transform;
	glm::vec4 color;
};

class Head {
public:


	Shader* shader;
	Mesh* scalp_mesh;
	Transform* transform;

	Head();
	~Head();
	void Draw(glm::mat4 view, glm::mat4 proj, glm::vec3 veiw_pos);

	Shader* line_shader;
	Line* naison_inion_line;
	Line* ear_to_ear_line;
	std::vector<Line*> lines;

	Mesh* landmark_mesh;
	Shader* landmark_shader;
	bool render_landmarks = true;
	Landmark naison    = Landmark{ LandmarkType::NAISON, new Transform(), glm::vec4(1, 0, 0, 1) };
	Landmark inion     = Landmark{ LandmarkType::INION, new Transform(), glm::vec4(0, 0, 0, 1) };
	Landmark left_ear  = Landmark{ LandmarkType::LEFT_EAR, new Transform(), glm::vec4(0, 1, 0, 1) };
	Landmark right_ear = Landmark{ LandmarkType::RIGHT_EAR, new Transform(), glm::vec4(0, 0, 1, 1) };

	const std::vector<Landmark*> landmarks = { &naison, &inion, &left_ear, &right_ear };

	std::unordered_map<LandmarkType, Landmark*> landmark_map = {
		{ LandmarkType::NAISON, &naison },
		{ LandmarkType::INION, &inion },
		{ LandmarkType::LEFT_EAR, &left_ear },
		{ LandmarkType::RIGHT_EAR, &right_ear },
	};

	std::unordered_map<LandmarkType, std::string> landmark_labels = { 
		{ LandmarkType::NAISON, "Naison" }, 
		{ LandmarkType::INION, "Inion" }, 
		{ LandmarkType::LEFT_EAR, "Left Ear" }, 
		{ LandmarkType::RIGHT_EAR, "Right Ear" }, 
	};

	void UpdateLandmark(LandmarkType type, const glm::vec3& position);

	void DrawLandmarks(glm::mat4 view, glm::mat4 proj, glm::vec3 veiw_pos);

};