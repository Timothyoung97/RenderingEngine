#include "computerPointLight.h"

#include "utility.h"
#include "engine.h"

extern tre::Engine* pEngine;

namespace tre {

ComputerPointLight::ComputerPointLight() {
	std::wstring basePathWstr = tre::Utility::getBasePathWstr();
	computeShaderPtLightMovement.create(basePathWstr + L"shaders\\bin\\compute_shader_ptLight_movement.bin", pEngine->device->device.Get());
}

void ComputerPointLight::compute(Graphics& graphics, const Scene& scene, const Camera& cam) {
	if (!scene.lightResc.numOfLights) return;

	// set const buffer for global info
	ID3D11Buffer* constBufferGlobalInfo = tre::Buffer::createConstBuffer(pEngine->device->device.Get(), (UINT)sizeof(tre::GlobalInfoStruct));
	{
		// Create struct info and submit data to constant buffer
		tre::GlobalInfoStruct globalInfoStruct = tre::CommonStructUtility::createGlobalInfoStruct(cam.camPositionV, cam.camViewProjection, scene.viewProjs, graphics.setting.csmPlaneIntervalsF, scene.dirlight, scene.lightResc.numOfLights, XMFLOAT2(4096, 4096), graphics.setting.csmDebugSwitch, graphics.setting.ssaoSwitch);
		tre::Buffer::updateConstBufferData(pEngine->device->contextI.Get(), constBufferGlobalInfo, &globalInfoStruct, (UINT)sizeof(tre::GlobalInfoStruct));
	}

	// using compute shader update lights
	pEngine->device->contextI.Get()->CSSetShader(computeShaderPtLightMovement.pShader.Get(), NULL, 0u);
	pEngine->device->contextI.Get()->CSSetConstantBuffers(0u, 1u, &constBufferGlobalInfo);
	pEngine->device->contextI.Get()->CSSetUnorderedAccessViews(0, 1, scene.lightResc.pLightUnorderedAccessView.GetAddressOf(), nullptr);
	{
		PROFILE_GPU_SCOPED("Point Light Position Compute");
		pEngine->device->contextI.Get()->Dispatch(tre::Maths::divideAndRoundUp(scene.lightResc.numOfLights, 4u), 1u, 1u);
	}

	// Clean up
	{
		pEngine->device->contextI.Get()->CSSetUnorderedAccessViews(0, 1, graphics.nullUAV, nullptr);
		graphics.bufferQueue.push_back(constBufferGlobalInfo);
	}
}
}