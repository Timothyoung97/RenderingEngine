#include "renderer.h"
#include "window.h"
#include "mesh.h"
#include "device.h"
#include "dxdebug.h"
#include "maths.h"
#include "utility.h"
#include "scene.h"

namespace tre {

Renderer::Renderer(ID3D11Device* _device, ID3D11DeviceContext* _context, HWND window) : _device(_device), _context(_context) {
	_factory.create();
	_swapchain.create(_factory.dxgiFactory2, _device, window);
	_blendstate.create(_device);
	_rasterizer.create(_device);
	_depthbuffer.create(_device, tre::SCREEN_WIDTH, tre::SCREEN_HEIGHT);
	_sampler.create(_device);
	_viewport.create(tre::SCREEN_WIDTH, tre::SCREEN_HEIGHT);
	
	std::wstring basePathWstr = tre::Utility::getBasePathWstr();
	_vertexShader.create(basePathWstr + L"shaders\\vertex_shader.bin", _device);
	_vertexShaderFullscreenQuad.create(basePathWstr + L"shaders\\vertex_shader_fullscreen.bin", _device);
	_inputLayout.create(_device, &_vertexShader);

	_forwardShader.create(basePathWstr + L"shaders\\pixel_shader_forward.bin", _device);
	_deferredShader.create(basePathWstr + L"shaders\\pixel_shader_deferred.bin", _device);
	_deferredShaderLighting.create(basePathWstr + L"shaders\\pixel_shader_deferred_lighting_env.bin", _device);
	_debugPixelShader.create(basePathWstr + L"shaders\\pixel_shader_debug.bin", _device);

	_gBuffer.create(_device);
}

void Renderer::reset() {
	_context->ClearState();
	_context->PSSetSamplers(0, 1, _sampler.pSamplerStateLinear.GetAddressOf());
	_context->PSSetSamplers(1, 1, _sampler.pSamplerStateMipPtWhiteBorder.GetAddressOf());
	_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Renderer::setShadowBufferDrawSection(int idx) {
	_viewport.shadowViewport.TopLeftX = _rasterizer.rectArr[idx].left;
	_viewport.shadowViewport.TopLeftY = _rasterizer.rectArr[idx].top;
	_context->RSSetViewports(1, &_viewport.shadowViewport);
	_context->RSSetScissorRects(1, &_rasterizer.rectArr[idx]);
}

void Renderer::clearSwapChainBuffer() {

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

void Renderer::clearShadowBuffer() {
	_context->ClearDepthStencilView(_depthbuffer.pShadowDepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void Renderer::configureStates(RENDER_MODE renderObjType) {

	ID3D11ShaderResourceView* nullSRV[1] = { nullptr };

	switch (renderObjType)
	{
	case tre::TRANSPARENT_M:
		_context->IASetInputLayout(_inputLayout.vertLayout.Get());
		_context->VSSetShader(_vertexShader.pShader.Get(), NULL, 0u);

		_context->RSSetViewports(1, &_viewport.defaultViewport);
		_context->RSSetState(_rasterizer.pRasterizerStateFCCW.Get());

		_context->PSSetShader(_forwardShader.pShader.Get(), NULL, 0u);
		_context->PSSetShaderResources(3, 1, _depthbuffer.pShadowShaderRescView.GetAddressOf()); // shadow
		_context->PSSetShaderResources(4, 1, nullSRV); 

		_context->OMSetBlendState(_blendstate.transparency.Get(), NULL, 0xffffffff);
		_context->OMSetDepthStencilState(_depthbuffer.pDSStateWithDepthTWriteDisabled.Get(), 0);
		_context->OMSetRenderTargets(1, &currRenderTargetView, _depthbuffer.pDepthStencilView.Get());
		break;		

	case tre::OPAQUE_M:
		_context->IASetInputLayout(_inputLayout.vertLayout.Get());
		_context->VSSetShader(_vertexShader.pShader.Get(), NULL, 0u);

		_context->RSSetViewports(1, &_viewport.defaultViewport);
		_context->RSSetState(_rasterizer.pRasterizerStateFCCW.Get());

		_context->PSSetShader(_forwardShader.pShader.Get(), NULL, 0u);
		_context->PSSetShaderResources(3, 1, _depthbuffer.pShadowShaderRescView.GetAddressOf()); // shadow
		_context->PSSetShaderResources(4, 1, _depthbuffer.pDepthStencilShaderRescView.GetAddressOf()); //depth

		_context->OMSetBlendState(_blendstate.opaque.Get(), NULL, 0xffffffff);
		_context->OMSetDepthStencilState(_depthbuffer.pDSStateWithDepthTWriteEnabled.Get(), 0);
		_context->OMSetRenderTargets(1, &currRenderTargetView, nullptr);
		break;			

	case tre::WIREFRAME_M:
		_context->VSSetShader(_vertexShader.pShader.Get(), NULL, 0u);
		_context->IASetInputLayout(_inputLayout.vertLayout.Get());

		_context->RSSetViewports(1, &_viewport.defaultViewport);
		_context->RSSetState(_rasterizer.pRasterizerStateWireFrame.Get());
		
		_context->PSSetShader(_debugPixelShader.pShader.Get(), NULL, 0u);

		_context->OMSetBlendState(_blendstate.transparency.Get(), NULL, 0xffffffff);
		_context->OMSetDepthStencilState(_depthbuffer.pDSStateWithoutDepthT.Get(), 0);
		_context->OMSetRenderTargets(1, &currRenderTargetView, nullptr);
		break;								

	case tre::SHADOW_M: // use normal draw func
		_context->IASetInputLayout(_inputLayout.vertLayout.Get());
		_context->VSSetShader(_vertexShader.pShader.Get(), NULL, 0u);

		// use setShadowBufferDrawSection to select draw section
		_context->RSSetState(_rasterizer.pShadowRasterizerState.Get());
		
		// unbind shadow buffer as a resource, so that we can write to it
		_context->PSSetShader(nullptr, NULL, 0u);
		_context->PSSetShaderResources(3, 1, nullSRV);
		
		_context->OMSetRenderTargets(0, nullptr, _depthbuffer.pShadowDepthStencilView.Get());
		_context->OMSetBlendState(_blendstate.opaque.Get(), NULL, 0xffffffff);
		_context->OMSetDepthStencilState(_depthbuffer.pDSStateWithDepthTWriteEnabled.Get(), 0);
		break;

	case tre::DEFERRED_OPAQUE_M: // use normal draw func
		_context->IASetInputLayout(_inputLayout.vertLayout.Get());
		_context->VSSetShader(_vertexShader.pShader.Get(), NULL, 0u);

		_context->RSSetViewports(1, &_viewport.defaultViewport);
		_context->RSSetState(_rasterizer.pRasterizerStateNoCull.Get());
		
		// unbind depth buffer as a shader resource, so that we can write to it
		_context->OMSetRenderTargets(0, nullptr, nullptr);
		_context->PSSetShader(_deferredShader.pShader.Get(), NULL, 0u);
		_context->PSSetShaderResources(4, 1, nullSRV);
		
		_context->ClearDepthStencilView(_depthbuffer.pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		_context->ClearRenderTargetView(_gBuffer.pRenderTargetViewDeferredAlbedo.Get(), tre::BACKGROUND_BLACK);
		_context->ClearRenderTargetView(_gBuffer.pRenderTargetViewDeferredNormal.Get(), tre::BACKGROUND_BLACK);
		_context->OMSetRenderTargets(2, _gBuffer.rtvs, _depthbuffer.pDepthStencilView.Get());
		_context->OMSetBlendState(_blendstate.opaque.Get(), NULL, 0xffffffff);
		_context->OMSetDepthStencilState(_depthbuffer.pDSStateWithDepthTWriteEnabled.Get(), 0);
		break;

	case tre::DEFERRED_OPAQUE_LIGHTING_ENV_M:
		_context->VSSetShader(_vertexShaderFullscreenQuad.pShader.Get(), NULL, 0u);
		_context->IASetInputLayout(nullptr);

		_context->RSSetViewports(1, &_viewport.defaultViewport);
		_context->RSSetState(_rasterizer.pRasterizerStateFCCW.Get());

		_context->OMSetRenderTargets(0, nullptr, nullptr);
		_context->PSSetShader(_deferredShaderLighting.pShader.Get(), NULL, 0u);
		_context->PSSetShaderResources(0, 1, _gBuffer.pShaderResViewDeferredAlbedo.GetAddressOf()); // albedo
		_context->PSSetShaderResources(1, 1, _gBuffer.pShaderResViewDeferredNormal.GetAddressOf()); // normal
		_context->PSSetShaderResources(3, 1, _depthbuffer.pShadowShaderRescView.GetAddressOf()); // shadow
		_context->PSSetShaderResources(4, 1, _depthbuffer.pDepthStencilShaderRescView.GetAddressOf()); //depth

		_context->OMSetRenderTargets(1, &currRenderTargetView, nullptr);
		_context->OMSetBlendState(_blendstate.opaque.Get(), NULL, 0xffffffff);
		_context->OMSetDepthStencilState(_depthbuffer.pDSStateWithDepthTWriteEnabled.Get(), 0);
		break;
	}
}

void Renderer::deferredLightingDraw() {
	configureStates(RENDER_MODE::DEFERRED_OPAQUE_LIGHTING_ENV_M);
	_context->Draw(6, 0);
}

void Renderer::draw(const std::vector<std::pair<Object*, Mesh*>> objQ, RENDER_MODE renderObjType) {

	configureStates(renderObjType);
	for (int i = 0; i < objQ.size(); i++) {

		UINT vertexStride = sizeof(Vertex);
		UINT offset = 0;

		//Set vertex buffer
		_context->IASetVertexBuffers(0, 1, objQ[i].second->pVertexBuffer.GetAddressOf(), &vertexStride, &offset);

		//Set index buffer
		_context->IASetIndexBuffer(objQ[i].second->pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		//set shader resc view and sampler
		bool hasTexture = 0;
		bool hasNormal = 0;
		if (objQ[i].second->material != nullptr) {
			if (objQ[i].second->material->objTexture != nullptr) {
				_context->PSSetShaderResources(0, 1, objQ[i].second->material->objTexture->pShaderResView.GetAddressOf());
				hasTexture = 1;
			}

			// set normal map
			if (objQ[i].second->material->objNormalMap != nullptr) {
				_context->PSSetShaderResources(1, 1, objQ[i].second->material->objNormalMap->pShaderResView.GetAddressOf());
				hasNormal = 1;
			}
		}

		//Config and set const buffer
		tre::ConstantBuffer::setObjConstBuffer(
			_device, _context,
			objQ[i].first->_transformationFinal,
			objQ[i].second->material->baseColor,
			hasTexture,
			hasNormal
		);

		_context->DrawIndexed(objQ[i].second->indexSize, 0, 0);
	}
}

void Renderer::debugDraw(const std::vector<std::pair<Object*, Mesh*>> objQ, Mesh& mesh, BoundVolumeEnum typeOfBound, RENDER_MODE renderObjType) {

	configureStates(renderObjType);
	for (int i = 0; i < objQ.size(); i++) {

		tre::Object* currObj = objQ[i].first;

		UINT vertexStride = sizeof(Vertex);
		UINT offset = 0;

		for (int j = 0; j < currObj->pObjMeshes.size(); j++) {
			//Set vertex buffer
			_context->IASetVertexBuffers(0, 1, mesh.pVertexBuffer.GetAddressOf(), &vertexStride, &offset);

			//Set index buffer
			_context->IASetIndexBuffer(mesh.pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

			//Update bounding volume
			XMMATRIX transformM;
			switch (typeOfBound) {
			case RitterBoundingSphere:
				transformM = tre::BoundingVolume::updateBoundingSphere(currObj->pObjMeshes[j]->ritterSphere, currObj->ritterBs[j], currObj->_transformationFinal);
				break;

			case NaiveBoundingSphere:
				transformM = tre::BoundingVolume::updateBoundingSphere(currObj->pObjMeshes[j]->naiveSphere, currObj->naiveBs[j], currObj->_transformationFinal);
				break;

			case AABBBoundingBox:
				transformM = tre::BoundingVolume::updateAABB(currObj->pObjMeshes[j]->aabb, currObj->aabb[j], currObj->_transformationFinal);
				break;
			}

			//Config and set const buffer
			tre::ConstantBuffer::setObjConstBuffer(
				_device, _context,
				transformM,
				currObj->_boundingVolumeColor[j],
				0, // bounding volume has no texture
				0 // bounding volume has no normal
			);

			_context->DrawIndexed(mesh.indexSize, 0, 0);
		}
	}
}



}