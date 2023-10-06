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
	_context->PSSetSamplers(0, 1, _sampler.pSamplerStateLinear.GetAddressOf());
	_context->PSSetSamplers(1, 1, _sampler.pSamplerStateMipPtWhiteBorder.GetAddressOf());
	_viewport.create(tre::SCREEN_WIDTH, tre::SCREEN_HEIGHT);
	
	std::wstring basePathWstr = tre::Utility::getBasePathWstr();
	_vertexShader.create(basePathWstr + L"shaders\\vertex_shader.bin", _device);
	_inputLayout.create(_device, &_vertexShader);
	_context->VSSetShader(_vertexShader.pShader.Get(), NULL, 0u);
	_context->IASetInputLayout(_inputLayout.vertLayout.Get());
	_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	_forwardShader.create(basePathWstr + L"shaders\\pixel_shader_forward.bin", _device);
	_deferredShader.create(basePathWstr + L"shaders\\pixel_shader_deferred.bin", _device);
	_debugPixelShader.create(basePathWstr + L"shaders\\pixel_shader_debug.bin", _device);

	_gBuffer.create(_device);
}

void Renderer::configureShadawSetting() {
	ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
	_context->PSSetShaderResources(3, 1, nullSRV);

	_context->ClearDepthStencilView(_depthbuffer.pShadowDepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

	_context->OMSetRenderTargets(0, nullptr, _depthbuffer.pShadowDepthStencilView.Get());
}

void Renderer::setShadowBufferDrawSection(int idx) {
	_viewport.shadowViewport.TopLeftX = _rasterizer.rectArr[idx].left;
	_viewport.shadowViewport.TopLeftY = _rasterizer.rectArr[idx].top;
	_context->RSSetViewports(1, &_viewport.shadowViewport);
	_context->RSSetScissorRects(1, &_rasterizer.rectArr[idx]);
}

void Renderer::clearBufferToDraw() {

	// Alternating buffers
	int currBackBuffer = static_cast<int>(_swapchain.mainSwapchain->GetCurrentBackBufferIndex());

	ID3D11Texture2D* backBuffer = nullptr;

	CHECK_DX_ERROR(_swapchain.mainSwapchain->GetBuffer(
		currBackBuffer, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer
	));

	// Create render target view
	ID3D11RenderTargetView* renderTargetView = nullptr;

	CHECK_DX_ERROR(_device->CreateRenderTargetView(
		backBuffer, NULL, &renderTargetView
	));

	_context->ClearRenderTargetView(renderTargetView, tre::BACKGROUND_GREY);

	_context->ClearDepthStencilView(_depthbuffer.pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	_context->OMSetRenderTargets(1, &renderTargetView, _depthbuffer.pDepthStencilView.Get());

	// set shadowMap
	_context->PSSetShaderResources(3, 1, _depthbuffer.pShadowShaderRescView.GetAddressOf());

	//Set Viewport for color draw
	_context->RSSetViewports(1, &_viewport.defaultViewport);
}

void Renderer::configureStates(RENDER_OBJ_TYPE renderObjType) {
	switch (renderObjType)
	{
	case tre::TRANSPARENT_T:
		_context->OMSetBlendState(_blendstate.transparency.Get(), NULL, 0xffffffff);
		_context->RSSetState(_rasterizer.pRasterizerStateFCCW.Get());
		_context->OMSetDepthStencilState(_depthbuffer.pDSStateWithDepthTWriteDisabled.Get(), 0);
		_context->PSSetShader(_forwardShader.pShader.Get(), NULL, 0u);
		break;								
	case tre::OPAQUE_T:						
		_context->OMSetBlendState(_blendstate.opaque.Get(), NULL, 0xffffffff);
		_context->RSSetState(_rasterizer.pRasterizerStateFCCW.Get());
		_context->OMSetDepthStencilState(_depthbuffer.pDSStateWithDepthTWriteEnabled.Get(), 0);
		_context->PSSetShader(_forwardShader.pShader.Get(), NULL, 0u);
		break;								
	case tre::WIREFRAME_T:					
		_context->OMSetBlendState(_blendstate.transparency.Get(), NULL, 0xffffffff);
		_context->RSSetState(_rasterizer.pRasterizerStateWireFrame.Get());
		_context->OMSetDepthStencilState(_depthbuffer.pDSStateWithoutDepthT.Get(), 0);
		_context->PSSetShader(_debugPixelShader.pShader.Get(), NULL, 0u);
		break;								
	case tre::SHADOW_T:						
		_context->OMSetBlendState(_blendstate.opaque.Get(), NULL, 0xffffffff);
		_context->RSSetState(_rasterizer.pShadowRasterizerState.Get());
		_context->OMSetDepthStencilState(_depthbuffer.pDSStateWithDepthTWriteEnabled.Get(), 0);
		_context->PSSetShader(nullptr, NULL, 0u);
		break;
	case tre::DEFERRED_OPAQUE_T:
		_context->OMSetBlendState(_blendstate.opaque.Get(), NULL, 0xffffffff);
		_context->RSSetState(_rasterizer.pRasterizerStateFCCW.Get());
		_context->OMSetDepthStencilState(_depthbuffer.pDSStateWithDepthTWriteEnabled.Get(), 0);
		_context->PSSetShader(_deferredShader.pShader.Get(), NULL, 0u);
		break;
	}
}

void Renderer::draw(const std::vector<std::pair<Object*, Mesh*>> objQ, RENDER_OBJ_TYPE renderObjType) {

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

void Renderer::debugDraw(const std::vector<std::pair<Object*, Mesh*>> objQ, Mesh& mesh, BoundVolumeEnum typeOfBound, RENDER_OBJ_TYPE renderObjType) {

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

void Renderer::configureDeferredDraw() {

	_context->ClearRenderTargetView(_gBuffer.pRenderTargetViewDeferredAlbedo.Get(), tre::BACKGROUND_BLACK);
	_context->ClearRenderTargetView(_gBuffer.pRenderTargetViewDeferredNormal.Get(), tre::BACKGROUND_BLACK);
	_context->ClearDepthStencilView(_depthbuffer.pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	_context->OMSetRenderTargets(2, _gBuffer.rtvs, _depthbuffer.pDepthStencilView.Get());
	_context->RSSetViewports(1, &_viewport.defaultViewport);
}

}