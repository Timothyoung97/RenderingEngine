#include "microprofile.h"

#include "graphics.h"
#include "window.h"
#include "mesh.h"
#include "device.h"
#include "dxdebug.h"
#include "maths.h"
#include "utility.h"
#include "scene.h"
#include "colors.h"

namespace tre {

Graphics::Graphics(ID3D11Device* _device, ID3D11DeviceContext* _context, HWND window) : _device(_device), _context(_context) {
	_factory.create();
	_swapchain.create(_factory.dxgiFactory6, _device, window);
	_blendstate.create(_device);
	_rasterizer.create(_device);
	_depthbuffer.create(_device, tre::SCREEN_WIDTH, tre::SCREEN_HEIGHT);
	_sampler.create(_device);
	_viewport.create(tre::SCREEN_WIDTH, tre::SCREEN_HEIGHT);
	
	std::wstring basePathWstr = tre::Utility::getBasePathWstr();
	_vertexShader.create(basePathWstr + L"shaders\\bin\\vertex_shader.bin", _device);
	_inputLayout.create(_device, &_vertexShader);

	_gBuffer.create(_device);
	_ssao.create(_device, _context);
	_hdrBuffer.create(_device, _context);
	_instanceBuffer.createBuffer(_device, _context);
}

void Graphics::clean() {
	MICROPROFILE_SCOPE_CSTR("Clean Up");

	while (!bufferQueue.empty()) {
		ID3D11Buffer* currBuffer = bufferQueue.back();
		bufferQueue.pop_back();
		currBuffer->Release();
	}

	_context->ClearState();
	_context->PSSetSamplers(0, 1, _sampler.pSamplerStateLinear.GetAddressOf());
	_context->PSSetSamplers(1, 1, _sampler.pSamplerStateMipPtWhiteBorder.GetAddressOf());
	_context->PSSetSamplers(2, 1, _sampler.pSamplerStateMipPtWrap.GetAddressOf());
	_context->PSSetSamplers(3, 1, _sampler.pSamplerStateMipPtClamp.GetAddressOf());
	_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	_context->ClearRenderTargetView(_ssao.ssaoBlurredTexture2dRTV.Get(), Colors::Transparent);
	_context->ClearRenderTargetView(_gBuffer.pRenderTargetViewDeferredAlbedo.Get(), tre::BACKGROUND_BLACK);
	_context->ClearRenderTargetView(_gBuffer.pRenderTargetViewDeferredNormal.Get(), tre::BACKGROUND_BLACK);
	_context->ClearRenderTargetView(_hdrBuffer.pRenderTargetViewHdrTexture.Get(), Colors::Transparent);
	_context->ClearDepthStencilView(_depthbuffer.pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	_context->ClearDepthStencilView(_depthbuffer.pShadowDepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

	this->clearSwapChainBuffer();
}

void Graphics::clearSwapChainBuffer() {

	// Alternating buffers
	int currBackBuffer = static_cast<int>(_swapchain.mainSwapchain->GetCurrentBackBufferIndex());

	ID3D11Texture2D* backBuffer = nullptr;

	CHECK_DX_ERROR(_swapchain.mainSwapchain->GetBuffer(
		currBackBuffer, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer
	));

	CHECK_DX_ERROR(_device->CreateRenderTargetView(
		backBuffer, NULL, &currRenderTargetView
	));
	
	// Set draw target to Screen
	_context->ClearRenderTargetView(currRenderTargetView, tre::BACKGROUND_GREY);
}

void Graphics::present() {
	MICROPROFILE_SCOPE_CSTR("Swap Chain Present");

	const UINT kSyncInterval = 0; // Need to sync CPU frame to this function if we want V-SYNC

	// When using sync interval 0, it is recommended to always pass the tearing flag when it is supported.
	const UINT presentFlags = (kSyncInterval == 0 && _swapchain.m_bTearingSupported) ? DXGI_PRESENT_ALLOW_TEARING : 0;

	CHECK_DX_ERROR(_swapchain.mainSwapchain->Present(kSyncInterval, presentFlags));
}

////////////////// Deprecated //////////////////

//void Graphics::configureStates(RENDER_MODE renderObjType) {
//
//	ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
//
//	switch (renderObjType)
//	{
//	case tre::OPAQUE_M:
//		_context->IASetInputLayout(_inputLayout.vertLayout.Get());
//		_context->VSSetShader(_vertexShader.pShader.Get(), NULL, 0u);
//
//		_context->RSSetViewports(1, &_viewport.defaultViewport);
//		_context->RSSetState(_rasterizer.pRasterizerStateFCCW.Get());
//
//		_context->PSSetShader(_forwardShader.pShader.Get(), NULL, 0u);
//		_context->PSSetShaderResources(3, 1, _depthbuffer.pShadowShaderRescView.GetAddressOf()); // shadow
//		_context->PSSetShaderResources(4, 1, nullSRV);
//
//		_context->OMSetBlendState(_blendstate.opaque.Get(), NULL, 0xffffffff);
//		_context->OMSetDepthStencilState(_depthbuffer.pDSStateWithDepthTWriteEnabled.Get(), 0);
//		_context->OMSetRenderTargets(1, &currRenderTargetView, _depthbuffer.pDepthStencilView.Get());
//		break;			
//
//	case tre::SHADOW_M: // use normal draw func
//		_context->IASetInputLayout(_inputLayout.vertLayout.Get());
//		_context->VSSetShader(_shadowCastShader.pShader.Get(), NULL, 0u);
//
//		// use setShadowBufferDrawSection to select draw section
//		_context->RSSetState(_rasterizer.pShadowRasterizerState.Get());
//		
//		// unbind shadow buffer as a resource, so that we can write to it
//		_context->PSSetShader(nullptr, NULL, 0u);
//		_context->PSSetShaderResources(3, 1, nullSRV);
//		
//		_context->OMSetBlendState(_blendstate.opaque.Get(), NULL, 0xffffffff);
//		_context->OMSetDepthStencilState(_depthbuffer.pDSStateWithDepthTWriteEnabled.Get(), 0);
//		_context->OMSetRenderTargets(0, nullptr, _depthbuffer.pShadowDepthStencilView.Get());
//		break;
//
//	case tre::DEFERRED_OPAQUE_M: // use normal draw func
//		_context->IASetInputLayout(_inputLayout.vertLayout.Get());
//		_context->VSSetShader(_vertexShader.pShader.Get(), NULL, 0u);
//
//		_context->RSSetViewports(1, &_viewport.defaultViewport);
//		_context->RSSetState(_rasterizer.pRasterizerStateFCCW.Get());
//		
//		// unbind depth buffer as a shader resource, so that we can write to it
//		_context->OMSetRenderTargets(0, nullptr, nullptr);
//		_context->PSSetShader(_deferredShader.pShader.Get(), NULL, 0u);
//		_context->PSSetShaderResources(4, 1, nullSRV);
//		
//		_context->OMSetBlendState(_blendstate.opaque.Get(), NULL, 0xffffffff);
//		_context->OMSetDepthStencilState(_depthbuffer.pDSStateWithDepthTWriteEnabled.Get(), 0);
//		_context->OMSetRenderTargets(2, _gBuffer.rtvs, _depthbuffer.pDepthStencilView.Get());
//		break;
//
//	}
//}
//
//void Graphics::fullscreenPass(tre::RENDER_MODE mode) {
//
//	const char* name = ToString(mode);
//	MICROPROFILE_SCOPE_CSTR(name);
//	PROFILE_GPU_SCOPED("Fullscreen Pass");
//
//	configureStates(mode);
//
//	_context->Draw(6, 0);
//}
//
//void Graphics::draw(const std::vector<std::pair<Object*, Mesh*>> objQ, RENDER_MODE renderObjType) {
//	if (objQ.size() == 0) return;
//
//	const char* name = ToString(renderObjType);
//	MICROPROFILE_SCOPE_CSTR(name);
//	PROFILE_GPU_SCOPED("Forward Draw");
//	
//	configureStates(renderObjType);
//
//	// Create empty const buffer and pre bind the constant buffer
//	ID3D11Buffer* constBufferModelInfo = tre::Buffer::createConstBuffer(_device, sizeof(tre::ModelInfoStruct));
//	_context->VSSetConstantBuffers(1u, 1u, &constBufferModelInfo);
//	_context->PSSetConstantBuffers(1u, 1u, &constBufferModelInfo);
//
//	for (int i = 0; i < objQ.size(); i++) {
//
//		UINT vertexStride = sizeof(Vertex);
//		UINT offset = 0;
//
//		//Set vertex buffer
//		_context->IASetVertexBuffers(0, 1, objQ[i].second->pVertexBuffer.GetAddressOf(), &vertexStride, &offset);
//
//		//Set index buffer
//		_context->IASetIndexBuffer(objQ[i].second->pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
//
//		//set shader resc view and sampler
//		bool hasTexture = 0;
//		bool hasNormal = 0;
//		if (objQ[i].second->pMaterial->objTexture != nullptr) {
//			_context->PSSetShaderResources(0, 1, objQ[i].second->pMaterial->objTexture->pShaderResView.GetAddressOf());
//			hasTexture = 1;
//		}
//
//		// set normal map
//		if (objQ[i].second->pMaterial->objNormalMap != nullptr) {
//			_context->PSSetShaderResources(1, 1, objQ[i].second->pMaterial->objNormalMap->pShaderResView.GetAddressOf());
//			hasNormal = 1;
//		}
//
//		// Submit each object's data to const buffer
//		{
//			tre::ModelInfoStruct modelInfoStruct = tre::CommonStructUtility::createModelInfoStruct(objQ[i].first->_transformationFinal, objQ[i].second->pMaterial->baseColor, hasTexture, hasNormal);
//			tre::Buffer::updateConstBufferData(_context, constBufferModelInfo, &modelInfoStruct, sizeof(tre::ModelInfoStruct));
//		}
//
//		_context->DrawIndexed(objQ[i].second->indexSize, 0, 0);
//	}
//
//	// clean up
//	{
//		constBufferModelInfo->Release();
//	}
//}
//
//
//void Graphics::instancedDraw(const std::vector<std::pair<Object*, Mesh*>>& objQ, RENDER_MODE renderMode) {
//	if (objQ.size() == 0) return;
//
//	ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
//
//	const char* name = ToString(renderMode);
//	MICROPROFILE_SCOPE_CSTR(name);
//	PROFILE_GPU_SCOPED("Instanced Draw");
//
//	_instanceBuffer.updateBuffer(objQ);
//
//	configureStates(renderMode);
//
//	UINT vertexStride = sizeof(Vertex);
//	UINT offset = 0;
//
//	// Create an empty const buffer 
//	ID3D11Buffer* constBufferBatchInfo = tre::Buffer::createConstBuffer(_device, (UINT)sizeof(tre::BatchInfoStruct));
//
//	for (int i = 0; i < _instanceBuffer.instanceBatchQueue.size(); i++) {
//		InstanceBatchInfo currBatchInfo = _instanceBuffer.instanceBatchQueue[i];
//
//		// update constant buffer for each instanced draw call
//		{
//			tre::BatchInfoStruct bInfo = tre::CommonStructUtility::createBatchInfoStruct(currBatchInfo.batchStartIdx);
//			tre::Buffer::updateConstBufferData(_context, constBufferBatchInfo, &bInfo, (UINT)sizeof(tre::BatchInfoStruct));
//
//			_context->VSSetConstantBuffers(1u, 1u, &constBufferBatchInfo);
//		}
//
//		// Update mesh vertex information
//		_context->IASetVertexBuffers(0, 1, currBatchInfo.pBatchMesh->pVertexBuffer.GetAddressOf(), &vertexStride, &offset);
//		_context->IASetIndexBuffer(currBatchInfo.pBatchMesh->pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
//
//		// Update texture information
//		_context->PSSetShaderResources(0u, 1u, nullSRV);
//		if (currBatchInfo.isWithTexture) {
//			_context->PSSetShaderResources(0u, 1u, currBatchInfo.pBatchTexture->pShaderResView.GetAddressOf());
//		}
//
//		_context->PSSetShaderResources(1u, 1u, nullSRV);
//		if (currBatchInfo.hasNormMap) {
//			_context->PSSetShaderResources(1u, 1u, currBatchInfo.pBatchNormalMap->pShaderResView.GetAddressOf());
//		}
//
//		// Draw call
//		_context->DrawIndexedInstanced(currBatchInfo.pBatchMesh->indexSize, currBatchInfo.quantity, 0u, 0u, 0u);
//	}
//
//	// clean up
//	{
//		constBufferBatchInfo->Release();
//	}
//}

}