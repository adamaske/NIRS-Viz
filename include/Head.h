#pragma once

#include "Transform.h"
#include "Mesh.h"
#include "Shader.h"
#include "Line.h"
#include "PointRenderer.h"

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
	std::vector<glm::vec3> hs_vertices; // Vertices in head space
	void DrawScalp(glm::mat4 view, glm::mat4 proj, glm::vec3 view_pos);

	Shader* line_shader;
	bool draw_landmark_lines = true;
	Line* naison_inion_line;
	Line* ear_to_ear_line;
	void DrawLandmarkLines(glm::mat4 view, glm::mat4 proj, glm::vec3 veiw_pos);

	// Raycasts
	bool draw_rays = true;
	float theta_step_size = 10.0f; // degrees
	float theta_deg = theta_step_size;
	float ray_distance = 250.0f;
	std::vector<Ray> nz_iz_rays;
	std::vector<Ray> lpa_rpa_rays;
	std::vector<Line*> nz_iz_ray_lines;
	std::vector<Line*> lpa_rpa_ray_lines;
	void DrawRays(glm::mat4 view, glm::mat4 proj, glm::vec3 veiw_pos);

	// Landmark-To-Landmark Paths
	bool draw_paths = true;
	std::vector<Line*> nz_iz_path_lines;
	std::vector<Line*> lpa_rpa_path_lines;
	std::vector<Line*> horizontal_arc_path_lines;

	// Rough path from raycasts
	std::vector<unsigned int> nz_iz_rough_path_indices;
	std::vector<unsigned int> nz_iz_fine_path_indices;

	std::vector<unsigned int> lpa_rpa_rough_path_indices;
	std::vector<unsigned int> lpa_rpa_fine_path_indices;

	std::vector<unsigned int> horizontal_arc_rough_path_indices;
	std::vector<unsigned int> horizontal_arc_fine_path_indices;
	void DrawPaths(glm::mat4 view, glm::mat4 proj, glm::vec3 veiw_pos);

	bool draw_waypoints;
	PointRenderer* nz_iz_waypoint_renderer;
	PointRenderer* lpa_rpa_waypoint_renderer;
	PointRenderer* horizontal_arc_waypoint_renderer;
	void DrawWaypoints(const glm::mat4& view, const glm::mat4& proj);

	// Landmarking
	bool draw_landmarks = true;
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
		{ LandmarkType::LPA, "LPA" }, 
		{ LandmarkType::RPA, "RPA" }, 
	};

	std::unordered_map<LandmarkType, unsigned int> lm_closest_vert_idx_map;
	void DrawLandmarks(glm::mat4 view, glm::mat4 proj, glm::vec3 veiw_pos);
	void UpdateLandmark(LandmarkType type, const glm::vec3& position);
	void LandmarksToClosestVertex();

	void GenerateRays();
	std::vector<unsigned int> CastRays(std::vector<Ray> rays, PointRenderer* wp_render);
	std::vector<unsigned int> FindFinePath(std::vector<unsigned int> rough_path);
	void GenerateCoordinateSystem();

	void UpdatePathLines(std::vector<Line*>* old_lines,
						 const std::vector<unsigned int>& new_indcies, 
						 const glm::vec3 color, 
						 const float& thickness);


	void GetReferencePointsAlongPath(
		const std::vector<unsigned int>& fine_path_indices,
		const std::vector<std::string>& labels,
		const std::vector<float>& percentages,
		PointRenderer* renderer
	);

	std::unordered_map<std::string, glm::vec3> point_label_map;
	bool draw_refpts = true;
	PointRenderer* refpts_renderer;

	void NZFirstMethod();
	void LPASecondMethod();
};