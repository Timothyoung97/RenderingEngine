#pragma once

#include "graphics.h"
#include "scene.h"

namespace tre {
class ComputerPointLight {
public:
	ID3D11Device* _device;
	ID3D11DeviceContext* _context;

	ComputeShader computeShaderPtLightMovement;

	ComputerPointLight(ID3D11Device* device, ID3D11DeviceContext* context);

	void compute(Graphics& graphics, const Scene& scene, const Camera& cam);
};

}