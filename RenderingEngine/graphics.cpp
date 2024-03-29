#include "graphics.h"

namespace tre {

Graphics::Graphics() {
	this->init();
}

void Graphics::init() {
	_factory.create();
	_swapchain.create(_factory.dxgiFactory6, pEngine->device->device, pEngine->window->hwnd);
	_blendstate.create(pEngine->device->device.Get());
	_rasterizer.create(pEngine->device->device.Get());
	_depthbuffer.create(pEngine->device->device.Get(), tre::SCREEN_WIDTH, tre::SCREEN_HEIGHT);
	_sampler.create(pEngine->device->device.Get());
	_viewport.create(tre::SCREEN_WIDTH, tre::SCREEN_HEIGHT);
	
	std::wstring basePathWstr = tre::Utility::getBasePathWstr();

	VertexShader _vertexShader;
	_vertexShader.create(basePathWstr + L"shaders\\bin\\vertex_shader.bin", pEngine->device->device.Get());
	_inputLayout.create(pEngine->device->device.Get(), &_vertexShader);

	_gBuffer.create(pEngine->device->device.Get());
	_ssao.create(pEngine->device->device.Get());
	_hdrBuffer.create(pEngine->device->device.Get());
	_bloomBuffer.create(pEngine->device->device.Get());
	_instanceBufferMainView.createBuffer(pEngine->device->device.Get());
	_instanceBufferPointlights.createBuffer(pEngine->device->device.Get());
	_instanceBufferWireframes.createBuffer(pEngine->device->device.Get());

	for (int i = 0; i < 4; i++) {
		_instanceBufferCSM[i].createBuffer(pEngine->device->device.Get());
	}
}

void Graphics::clean() {
	MICROPROFILE_SCOPE_CSTR("Clean Up");
	while (!bufferQueue.empty()) {
		ID3D11Buffer* currBuffer = bufferQueue.back();
		bufferQueue.pop_back();
		currBuffer->Release();
	}

	pEngine->device->contextI.Get()->ClearState();
	{
		ComPtr<ID3D11RenderTargetView> ssaoBlurredTexture2dRTV;
		D3D11_RENDER_TARGET_VIEW_DESC ssaoResultTexture2dRTVDesc;
		ZeroMemory(&ssaoResultTexture2dRTVDesc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
		ssaoResultTexture2dRTVDesc.Format = DXGI_FORMAT_R8_UNORM;
		ssaoResultTexture2dRTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		ssaoResultTexture2dRTVDesc.Texture2D.MipSlice = 0;

		CHECK_DX_ERROR(pEngine->device->device.Get()->CreateRenderTargetView(
			_ssao.ssaoBlurredTexture2d.Get(), &ssaoResultTexture2dRTVDesc, ssaoBlurredTexture2dRTV.GetAddressOf()
		));
		pEngine->device->contextI.Get()->ClearRenderTargetView(ssaoBlurredTexture2dRTV.Get(), Colors::Transparent);
	}

	{
		ComPtr<ID3D11DepthStencilView> shadowDepthStencilView;
		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
		ZeroMemory(&depthStencilViewDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
		depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;
		depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;

		CHECK_DX_ERROR(pEngine->device->device.Get()->CreateDepthStencilView(
			_depthbuffer.pShadowMapTexture.Get(), &depthStencilViewDesc, shadowDepthStencilView.GetAddressOf()
		));
		pEngine->device->contextI.Get()->ClearDepthStencilView(shadowDepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	}

	{
		ComPtr<ID3D11DepthStencilView> depthStencilView;
		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
		ZeroMemory(&depthStencilViewDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
		depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;

		CHECK_DX_ERROR(pEngine->device->device.Get()->CreateDepthStencilView(
			_depthbuffer.pDepthStencilTexture.Get(), &depthStencilViewDesc, depthStencilView.GetAddressOf()
		));
		pEngine->device->contextI.Get()->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	}

	{
		ComPtr<ID3D11RenderTargetView> hdrRTV;
		D3D11_RENDER_TARGET_VIEW_DESC hdrRTVDesc;
		ZeroMemory(&hdrRTVDesc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
		hdrRTVDesc.Format = DXGI_FORMAT_R11G11B10_FLOAT;
		hdrRTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		hdrRTVDesc.Texture2D.MipSlice = 0;

		CHECK_DX_ERROR(pEngine->device->device.Get()->CreateRenderTargetView(
			_hdrBuffer.pHdrBufferTexture.Get(), &hdrRTVDesc, hdrRTV.GetAddressOf()
		));
		pEngine->device->contextI.Get()->ClearRenderTargetView(hdrRTV.Get(), Colors::Transparent);
	}

	{
		ComPtr<ID3D11RenderTargetView> pRenderTargetViewDeferredAlbedo;
		D3D11_RENDER_TARGET_VIEW_DESC rtvd;
		ZeroMemory(&rtvd, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
		rtvd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		rtvd.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		rtvd.Texture2D.MipSlice = 0;

		CHECK_DX_ERROR(pEngine->device->device.Get()->CreateRenderTargetView(
			_gBuffer.pGBufferTextureAlbedo.Get(), &rtvd, pRenderTargetViewDeferredAlbedo.GetAddressOf()
		));
		pEngine->device->contextI.Get()->ClearRenderTargetView(pRenderTargetViewDeferredAlbedo.Get(), tre::BACKGROUND_BLACK);
	}

	{
		ComPtr<ID3D11RenderTargetView> pRenderTargetViewDeferredNormal;
		D3D11_RENDER_TARGET_VIEW_DESC rtvd;
		ZeroMemory(&rtvd, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
		rtvd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		rtvd.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		rtvd.Texture2D.MipSlice = 0;

		CHECK_DX_ERROR(pEngine->device->device.Get()->CreateRenderTargetView(
			_gBuffer.pGBufferTextureNormal.Get(), &rtvd, pRenderTargetViewDeferredNormal.GetAddressOf()
		));
		pEngine->device->contextI.Get()->ClearRenderTargetView(pRenderTargetViewDeferredNormal.Get(), tre::BACKGROUND_BLACK);
	}

	this->clearSwapChainBuffer();
}

void Graphics::clearSwapChainBuffer() {

	// Alternating buffers
	int currBackBuffer = static_cast<int>(_swapchain.mainSwapchain->GetCurrentBackBufferIndex());

	ComPtr<ID3D11Texture2D> backBuffer;

	CHECK_DX_ERROR(_swapchain.mainSwapchain->GetBuffer(
		currBackBuffer, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer
	));

	CHECK_DX_ERROR(pEngine->device->device.Get()->CreateRenderTargetView(
		backBuffer.Get(), NULL, currRenderTargetView.GetAddressOf()
	));
	
	// Set draw target to Screen
	pEngine->device->contextI.Get()->ClearRenderTargetView(currRenderTargetView.Get(), tre::BACKGROUND_GREY);
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