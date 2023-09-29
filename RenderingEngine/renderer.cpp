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

	_pixelShader.create(basePathWstr + L"shaders\\pixel_shader.bin", _device);
	_debugPixelShader.create(basePathWstr + L"shaders\\light_pixel.bin", _device);	
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

	_context->ClearRenderTargetView(renderTargetView, tre::BACKGROUND_COLOR);

	_context->ClearDepthStencilView(_depthbuffer.pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	_context->OMSetRenderTargets(1, &renderTargetView, _depthbuffer.pDepthStencilView.Get());

	// set shadowMap
	_context->PSSetShaderResources(3, 1, _depthbuffer.pShadowShaderRescView.GetAddressOf());

	//Set Viewport for color draw
	_context->RSSetViewports(1, &_viewport.defaultViewport);
}

void Renderer::configureStates(RENDER_MODE renderMode) {
	switch (renderMode)
	{
	case tre::TRANSPARENT_M:
		_context->OMSetBlendState(_blendstate.transparency.Get(), NULL, 0xffffffff);
		_context->RSSetState(_rasterizer.pRasterizerStateFCCW.Get());
		_context->OMSetDepthStencilState(_depthbuffer.pDSStateWithDepthTWriteDisabled.Get(), 0);
		_context->PSSetShader(_pixelShader.pShader.Get(), NULL, 0u);
		break;								
	case tre::OPAQUE_M:						
		_context->OMSetBlendState(_blendstate.opaque.Get(), NULL, 0xffffffff);
		_context->RSSetState(_rasterizer.pRasterizerStateFCCW.Get());
		_context->OMSetDepthStencilState(_depthbuffer.pDSStateWithDepthTWriteEnabled.Get(), 0);
		_context->PSSetShader(_pixelShader.pShader.Get(), NULL, 0u);
		break;								
	case tre::WIREFRAME_M:					
		_context->OMSetBlendState(_blendstate.transparency.Get(), NULL, 0xffffffff);
		_context->RSSetState(_rasterizer.pRasterizerStateWireFrame.Get());
		_context->OMSetDepthStencilState(_depthbuffer.pDSStateWithoutDepthT.Get(), 0);
		_context->PSSetShader(_debugPixelShader.pShader.Get(), NULL, 0u);
		break;								
	case tre::SHADOW_M:						
		_context->OMSetBlendState(_blendstate.transparency.Get(), NULL, 0xffffffff);
		_context->RSSetState(_rasterizer.pShadowRasterizerState.Get());
		_context->OMSetDepthStencilState(_depthbuffer.pDSStateWithDepthTWriteEnabled.Get(), 0);
		_context->PSSetShader(nullptr, NULL, 0u);
		break;
	}
}

void Renderer::draw(const std::vector<Object>& objQ, RENDER_MODE renderMode) {

	configureStates(renderMode);
	
	for (int i = 0; i < objQ.size(); i++) {

		const tre::Object& currObj = objQ[i];

		//Set vertex buffer
		UINT vertexStride = sizeof(Vertex);
		UINT offset = 0;
		_context->IASetVertexBuffers(0, 1, currObj.pObjMesh->pVertexBuffer.GetAddressOf(), &vertexStride, &offset);

		//Set index buffer
		_context->IASetIndexBuffer(currObj.pObjMesh->pIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

		//set shader resc view and sampler
		if (currObj.pObjMesh->material->objTexture->pShaderResView.Get() != nullptr) {
			_context->PSSetShaderResources(0, 1, currObj.pObjMesh->material->objTexture->pShaderResView.GetAddressOf());
		}

		// set normal map
		if (currObj.pObjMesh->material->objNormalMap->pShaderResView.Get() != nullptr) {
			_context->PSSetShaderResources(1, 1, currObj.pObjMesh->material->objNormalMap->pShaderResView.GetAddressOf());
		}

		//Config and set const buffer
		tre::ConstantBuffer::setObjConstBuffer(
			_device, _context,
			tre::Maths::createTransformationMatrix(currObj.objScale, currObj.objRotation, currObj.objPos),
			currObj.pObjMesh->material->baseColor,
			currObj.pObjMesh->material->objTexture->pShaderResView.Get() != nullptr ? 1 : 0,
			currObj.pObjMesh->material->objNormalMap->pShaderResView.Get() != nullptr ? 1 : 0
		);


		_context->DrawIndexed(currObj.pObjMesh->indexSize, 0, 0);
	}
}

void Renderer::draw(Object* obj, RENDER_MODE renderMode, XMMATRIX matrix) {

	configureStates(renderMode);

	if (obj->pObjMesh != nullptr) {
		//Set vertex buffer
		UINT vertexStride = sizeof(Vertex);
		UINT offset = 0;
		_context->IASetVertexBuffers(0, 1, obj->pObjMesh->pVertexBuffer.GetAddressOf(), &vertexStride, &offset);

		//Set index buffer
		_context->IASetIndexBuffer(obj->pObjMesh->pIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

		//set shader resc view and sampler
		bool hasTexture = 0;
		if (obj->pObjMesh->material->objTexture != nullptr) {
			_context->PSSetShaderResources(0, 1, obj->pObjMesh->material->objTexture->pShaderResView.GetAddressOf());
			hasTexture = 1;
		}

		// set normal map
		bool hasNormal = 0;
		if (obj->pObjMesh->material->objNormalMap != nullptr) {
			_context->PSSetShaderResources(1, 1, obj->pObjMesh->material->objNormalMap->pShaderResView.GetAddressOf());
			hasNormal = 1;
		}

		//Config and set const buffer
		tre::ConstantBuffer::setObjConstBuffer(
			_device, _context,
			matrix,
			obj->pObjMesh->material->baseColor,
			hasTexture,
			hasNormal
		);

		_context->DrawIndexed(obj->pObjMesh->indexSize, 0, 0);
	}

}

void Renderer::debugDraw(std::vector<Object>& objQ, Mesh& mesh, BoundVolumeEnum typeOfBound, RENDER_MODE renderMode) {

	configureStates(renderMode);

	for (int i = 0; i < objQ.size(); i++) {

		tre::Object& currObj = objQ[i];

		//Set vertex buffer
		UINT vertexStride = sizeof(Vertex);
		UINT offset = 0;
		_context->IASetVertexBuffers(0, 1, mesh.pVertexBuffer.GetAddressOf(), &vertexStride, &offset);

		//Set index buffer
		_context->IASetIndexBuffer(mesh.pIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

		//Update bounding volume
		XMMATRIX transformM;
		switch (typeOfBound) {
		case RitterBoundingSphere:
			transformM = tre::BoundingVolume::updateBoundingSphere(currObj.pObjMesh->ritterSphere, currObj.ritterBs, currObj.objScale, currObj.objRotation, currObj.objPos);
			break;

		case NaiveBoundingSphere:
			transformM = tre::BoundingVolume::updateBoundingSphere(currObj.pObjMesh->naiveSphere, currObj.naiveBs, currObj.objScale, currObj.objRotation, currObj.objPos);
			break;

		case AABBBoundingBox:
			transformM = tre::BoundingVolume::updateAABB(currObj.pObjMesh->aabb, currObj.aabb, currObj.objScale, currObj.objRotation, currObj.objPos);
			break;
		}

		//set shader resc view and sampler
		if (currObj.pObjMesh->material->objTexture->pShaderResView.Get() != nullptr) {
			_context->PSSetShaderResources(0, 1, currObj.pObjMesh->material->objTexture->pShaderResView.GetAddressOf());
		}

		// set normal map
		if (currObj.pObjMesh->material->objNormalMap->pShaderResView.Get() != nullptr) {
			_context->PSSetShaderResources(1, 1, currObj.pObjMesh->material->objNormalMap->pShaderResView.GetAddressOf());
		}

		//Config and set const buffer
		tre::ConstantBuffer::setObjConstBuffer(
			_device, _context,
			transformM,
			currObj._boundingVolumeColor,
			currObj.pObjMesh->material->objTexture->pShaderResView.Get() != nullptr ? 1 : 0,
			currObj.pObjMesh->material->objNormalMap->pShaderResView.Get() != nullptr ? 1 : 0
		);

		_context->DrawIndexed(mesh.indexSize, 0, 0);
	}
}

}