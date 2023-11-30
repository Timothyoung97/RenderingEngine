#pragma once

#include "graphics.h"
#include "scene.h"

namespace tre {
class ComputerPointLight {
public:
	ComputeShader computeShaderPtLightMovement;

	ComputerPointLight();

	void init();

	void compute(Graphics& graphics, const Scene& scene, const Camera& cam);
};

}