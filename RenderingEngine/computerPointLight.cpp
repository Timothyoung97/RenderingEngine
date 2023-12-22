#include "computerPointLight.h"

#include "utility.h"
#include "engine.h"

extern tre::Engine* pEngine;

namespace tre {

ComputerPointLight::ComputerPointLight() {
	this->init();
}

void ComputerPointLight::init() {
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
		tre::Buffer::updateConstBufferData(contextD.Get(), constBufferGlobalInfo, &globalInfoStruct, (UINT)sizeof(tre::GlobalInfoStruct));
	}

	//Create View

	ComPtr<ID3D11UnorderedAccessView> lightUnorderedAccessView;
	{
		// update GPU on buffer
		D3D11_BUFFER_UAV lightBufferUAV;
		lightBufferUAV.NumElements = scene.lightResc.numOfLights;
		lightBufferUAV.FirstElement = 0;
		lightBufferUAV.Flags = 0u;

		D3D11_UNORDERED_ACCESS_VIEW_DESC lightUnorderAccessViewDesc;
		lightUnorderAccessViewDesc.Format = DXGI_FORMAT_UNKNOWN;
		lightUnorderAccessViewDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		lightUnorderAccessViewDesc.Buffer = lightBufferUAV;

		CHECK_DX_ERROR(pEngine->device->device.Get()->CreateUnorderedAccessView(
			scene.lightResc.pLightBufferGPU.Get(), &lightUnorderAccessViewDesc, lightUnorderedAccessView.GetAddressOf()
		));
	}

	// using compute shader update lights
	contextD.Get()->CSSetShader(computeShaderPtLightMovement.pShader.Get(), NULL, 0u);
	contextD.Get()->CSSetConstantBuffers(0u, 1u, &constBufferGlobalInfo);
	contextD.Get()->CSSetUnorderedAccessViews(0, 1, lightUnorderedAccessView.GetAddressOf(), nullptr);
	{
		//PROFILE_GPU_SCOPED("Point Light Position Compute");
		contextD.Get()->Dispatch(tre::Maths::divideAndRoundUp(scene.lightResc.numOfLights, 4u), 1u, 1u);
	}

	// Clean up
	{
		contextD.Get()->CSSetUnorderedAccessViews(0, 1, graphics.nullUAV, nullptr);
		std::lock_guard<std::mutex> lock(graphics.bufferQueueMutex);
		graphics.bufferQueue.push_back(constBufferGlobalInfo);
	}

	{
		CHECK_DX_ERROR(contextD->FinishCommandList(
			false, &commandList
		));
	}
}
}