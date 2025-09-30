#include "Probe.h"

Probe::Probe(SNIRF* _snirf)
{
	// Get Probe from sNIRF file
	snirf = _snirf;
	transform = new Transform();
	//shader = new Shader("C:/dev/NIRS-Viz/data/Shaders/probe.vert", "C:/dev/NIRS-Viz/data/Shaders/probe.frag");
}

Probe::~Probe()
{
}

void Probe::RegisterProbes()
{
}

void Probe::Draw()
{

	// Render Spheres in 3D space

	//std::vector<glm::vec3> probe_positions = {}
	//std::vector<int> types = {}
	//
}
