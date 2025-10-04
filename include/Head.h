#pragma once

#include "Transform.h"
#include "Mesh.h"
#include "Shader.h"
#include "Line.h"

#include "DjikstraSolver.h"
#include "Raycaster.h"
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

struct Waypoint {
	unsigned int waypoint_index;
	unsigned int vertex_index;
	glm::vec3 position;
};

class Head {
public:
	Head();
	~Head();

	void Draw(glm::mat4 view, glm::mat4 proj, glm::vec3 veiw_pos);

	Transform* transform;
	Shader* scalp_shader;
	Mesh* scalp_mesh;
	Graph scalp_mesh_graph;
	void DrawScalp(glm::mat4 view, glm::mat4 proj, glm::vec3 view_pos);

	Shader* line_shader;

	bool draw_landmark_lines = true;
	Line* naison_inion_line;
	Line* ear_to_ear_line;
	void DrawLandmarkLines(glm::mat4 view, glm::mat4 proj, glm::vec3 veiw_pos);


	float theta_step_size = 10.0f; // degrees
	float theta_deg = theta_step_size;
	float ray_distance = 250.0f;
	std::vector<Ray> nz_iz_rays;
	std::vector<Ray> lpa_rpa_rays;
	bool draw_rays = true;

	std::vector<Line*> nz_iz_path;
	std::vector<Line*> lpa_rpa_path;
	void DrawRays(glm::mat4 view, glm::mat4 proj, glm::vec3 veiw_pos);

	bool draw_waypoints;
	Mesh* waypoint_mesh;
	Shader* waypoint_shader;
	std::vector<Waypoint*> nz_iz_waypoints;
	std::vector<Waypoint*> lpa_rpa_waypoints;
	void DrawWaypoints(glm::mat4 view, glm::mat4 proj, glm::vec3 veiw_pos);

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

	std::unordered_map<LandmarkType, unsigned int> lm_closest_vert_idx_map;
	void DrawLandmarks(glm::mat4 view, glm::mat4 proj, glm::vec3 veiw_pos);
	void UpdateLandmark(LandmarkType type, const glm::vec3& position);
	void LandmarksToClosestVertex();

	void GenerateRays();
	void CastRays();

	std::vector<unsigned int> FindFinePath(std::vector<unsigned int> rough_path);

	void GenerateCoordinateSystem();
};