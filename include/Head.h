#pragma once

#include "Transform.h"
#include "Mesh.h"
#include "Shader.h"
#include "Line.h"

enum LandmarkType {
	NAISON,
	INION,
	LPA,
	RPA
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


	// Landmarking
	Mesh* landmark_mesh;
	Shader* landmark_shader;
	bool render_landmarks = true;
	Landmark naison	= Landmark{LandmarkType::NAISON,	new Transform(),	glm::vec4(1, 0, 0, 1)};
	Landmark inion	= Landmark{LandmarkType::INION,		new Transform(),	glm::vec4(0, 0, 0, 1)};
	Landmark lpa	= Landmark{LandmarkType::LPA,		new Transform(),	glm::vec4(0, 1, 0, 1)};
	Landmark rpa	= Landmark{LandmarkType::RPA,		new Transform(),	glm::vec4(0, 0, 1, 1)};
																			  
	std::unordered_map<LandmarkType, Landmark*> landmark_map = {
		{ LandmarkType::NAISON, &naison	},
		{ LandmarkType::INION,	&inion	},
		{ LandmarkType::LPA,	&lpa	},
		{ LandmarkType::RPA,	&rpa	}
	};

	const std::vector<Landmark*> landmarks = { landmark_map[NAISON], landmark_map[INION], landmark_map[LPA], landmark_map[RPA] };
	std::unordered_map<LandmarkType, std::string> landmark_labels = { 
		{ LandmarkType::NAISON, "Naison" }, 
		{ LandmarkType::INION, "Inion" }, 
		{ LandmarkType::LPA, "RPA" }, 
		{ LandmarkType::RPA, "LPA" }, 
	};

	void UpdateLandmark(LandmarkType type, const glm::vec3& position);

	void DrawLandmarks(glm::mat4 view, glm::mat4 proj, glm::vec3 veiw_pos);

	std::unordered_map<LandmarkType, unsigned int> LandmarksToClosestVertex();
	void ShortestPathOnScalp(unsigned int start_vertex, unsigned int end_vertex);
	void GenerateCoordinateSystem();
};