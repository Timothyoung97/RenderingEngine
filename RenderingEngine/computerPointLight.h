#pragma once

#include "rendererBase.h"
#include "graphics.h"
#include "scene.h"
#include "microprofiler.h"

namespace tre {
class ComputerPointLight : public RendererBase {
public:
	ComputeShader computeShaderPtLightMovement;

	ComputerPointLight();

	void init();

	void compute(Graphics& graphics, const Scene& scene, const Camera& cam, MicroProfiler& profiler);
};

}