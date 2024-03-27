#pragma once

#include "rendererBase.h"
#include "graphics.h"
#include "scene.h"
#include "microprofiler.h"
#include "utility.h"
#include "engine.h"

extern std::shared_ptr<tre::Engine> pEngine;

namespace tre {
class ComputerPointLight : public RendererBase {
public:
	ComputeShader computeShaderPtLightMovement;

	ComputerPointLight();

	void init();

	void compute(Graphics& graphics, const Scene& scene, const Camera& cam, MicroProfiler& profiler);
};

}