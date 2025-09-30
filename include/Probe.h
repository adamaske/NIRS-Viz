#pragma once

#include "Transform.h"
#include "Shader.h"
#include "Snirf.h"
class Probe {
public:
	Probe(SNIRF* _snirf);
	~Probe();

	SNIRF* snirf;
	Transform* transform;
	Shader* shader;

	void RegisterProbes();

	void Draw();
};