#include "computerPointLight.h"

#include "utility.h"

namespace tre {

ComputerPointLight::ComputerPointLight(ID3D11Device* pDevice, ID3D11DeviceContext* pContext) : _device(pDevice), _context(pContext) {
	std::wstring basePathWstr = tre::Utility::getBasePathWstr();
	computeShaderPtLightMovement.create(basePathWstr + L"shaders\\bin\\compute_shader_ptLight_movement.bin", _device);
}

void ComputerPointLight::compute(Graphics& graphics, const Scene& scene, const Camera& cam) {
	if (!scene.lightResc.numOfLights) return;

	// set const buffer for global info
	ID3D11Buffer* constBufferGlobalInfo = tre::Buffer::createConstBuffer(_device, (UINT)sizeof(tre::GlobalInfoStruct));
	{
		// Create struct info and submit data to constant buffer
		tre::GlobalInfoStruct globalInfoStruct = tre::CommonStructUtility::createGlobalInfoStruct(cam.camPositionV, cam.camViewProjection, scene.viewProjs, graphics.setting.csmPlaneIntervalsF, scene.dirlight, scene.lightResc.numOfLights, XMFLOAT2(4096, 4096), graphics.setting.csmDebugSwitch, graphics.setting.ssaoSwitch);
		tre::Buffer::updateConstBufferData(_context, constBufferGlobalInfo, &globalInfoStruct, (UINT)sizeof(tre::GlobalInfoStruct));
	}

	// using compute shader update lights
	_context->CSSetShader(computeShaderPtLightMovement.pShader.Get(), NULL, 0u);
	_context->CSSetConstantBuffers(0u, 1u, &constBufferGlobalInfo);
	_context->CSSetUnorderedAccessViews(0, 1, scene.lightResc.pLightUnorderedAccessView.GetAddressOf(), nullptr);
	{
		PROFILE_GPU_SCOPED("Point Light Position Compute");
		_context->Dispatch(tre::Maths::divideAndRoundUp(scene.lightResc.numOfLights, 4u), 1u, 1u);
	}

	// Clean up
	{
		_context->CSSetUnorderedAccessViews(0, 1, graphics.nullUAV, nullptr);
		graphics.bufferQueue.push_back(constBufferGlobalInfo);
	}
}
}