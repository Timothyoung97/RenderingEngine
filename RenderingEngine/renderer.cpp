#include "microprofile.h"

#include "renderer.h"
#include "window.h"
#include "mesh.h"
#include "device.h"
#include "dxdebug.h"
#include "maths.h"
#include "utility.h"
#include "scene.h"
#include "colors.h"

namespace tre {

Renderer::Renderer(ID3D11Device* _device, ID3D11DeviceContext* _context, HWND window) : _device(_device), _context(_context) {
	_factory.create();
	_swapchain.create(_factory.dxgiFactory6, _device, window);
	_blendstate.create(_device);
	_rasterizer.create(_device);
	_depthbuffer.create(_device, tre::SCREEN_WIDTH, tre::SCREEN_HEIGHT);
	_sampler.create(_device);
	_viewport.create(tre::SCREEN_WIDTH, tre::SCREEN_HEIGHT);
	
	std::wstring basePathWstr = tre::Utility::getBasePathWstr();
	_vertexShader.create(basePathWstr + L"shaders\\bin\\vertex_shader.bin", _device);
	_vertexShaderFullscreenQuad.create(basePathWstr + L"shaders\\bin\\vertex_shader_fullscreen.bin", _device);
	_vertexShaderInstanced.create(basePathWstr + L"shaders\\bin\\vertex_shader_instancedRendering.bin", _device);
	_inputLayout.create(_device, &_vertexShader);

	_forwardShader.create(basePathWstr + L"shaders\\bin\\pixel_shader_forward.bin", _device);
	_shadowCastShader.create(basePathWstr + L"shaders\\bin\\vertex_shader_csmShadowCast.bin", _device);
	_deferredShader.create(basePathWstr + L"shaders\\bin\\pixel_shader_deferred.bin", _device);
	_deferredShaderLightingEnv.create(basePathWstr + L"shaders\\bin\\pixel_shader_deferred_lighting_env.bin", _device);
	_deferredShaderLightingLocal.create(basePathWstr + L"shaders\\bin\\pixel_shader_deferred_lighting_local.bin", _device);
	_ssaoPixelShader.create(basePathWstr + L"shaders\\bin\\pixel_shader_ssao_rendering.bin", _device);
	_textureBlurPixelShader.create(basePathWstr + L"shaders\\bin\\pixel_shader_texture_blur.bin", _device);
	_hdrPixelShader.create(basePathWstr + L"shaders\\bin\\pixel_shader_hdr_rendering.bin", _device);
	_instancedPixelShader.create(basePathWstr + L"shaders\\bin\\pixel_shader_instanced_gbuffer.bin", _device);
	_debugPixelShader.create(basePathWstr + L"shaders\\bin\\pixel_shader_debug.bin", _device);

	_gBuffer.create(_device);
	_ssao.create(_device, _context);
	_hdrBuffer.create(_device, _context);
	_instanceBuffer.createBuffer(_device, _context);
}

void Renderer::reset() {
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

	this->clearSwapChainBuffer();
	this->clearShadowBuffer();
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
		_context->OMSetRenderTargets(1, _hdrBuffer.pRenderTargetViewHdrTexture.GetAddressOf(), _depthbuffer.pDepthStencilView.Get());
		break;		

	case tre::OPAQUE_M:
		_context->IASetInputLayout(_inputLayout.vertLayout.Get());
		_context->VSSetShader(_vertexShader.pShader.Get(), NULL, 0u);

		_context->RSSetViewports(1, &_viewport.defaultViewport);
		_context->RSSetState(_rasterizer.pRasterizerStateFCCW.Get());

		_context->PSSetShader(_forwardShader.pShader.Get(), NULL, 0u);
		_context->PSSetShaderResources(3, 1, _depthbuffer.pShadowShaderRescView.GetAddressOf()); // shadow
		_context->PSSetShaderResources(4, 1, nullSRV);

		_context->OMSetBlendState(_blendstate.opaque.Get(), NULL, 0xffffffff);
		_context->OMSetDepthStencilState(_depthbuffer.pDSStateWithDepthTWriteEnabled.Get(), 0);
		_context->OMSetRenderTargets(1, &currRenderTargetView, _depthbuffer.pDepthStencilView.Get());
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
		_context->VSSetShader(_shadowCastShader.pShader.Get(), NULL, 0u);

		// use setShadowBufferDrawSection to select draw section
		_context->RSSetState(_rasterizer.pShadowRasterizerState.Get());
		
		// unbind shadow buffer as a resource, so that we can write to it
		_context->PSSetShader(nullptr, NULL, 0u);
		_context->PSSetShaderResources(3, 1, nullSRV);
		
		_context->OMSetBlendState(_blendstate.opaque.Get(), NULL, 0xffffffff);
		_context->OMSetDepthStencilState(_depthbuffer.pDSStateWithDepthTWriteEnabled.Get(), 0);
		_context->OMSetRenderTargets(0, nullptr, _depthbuffer.pShadowDepthStencilView.Get());
		break;

	case tre::INSTANCED_SHADOW_M: // use normal draw func
		_context->IASetInputLayout(_inputLayout.vertLayout.Get());
		_context->VSSetShader(_vertexShaderInstanced.pShader.Get(), NULL, 0u);
		_context->VSSetShaderResources(0u, 1, _instanceBuffer.pInstanceBufferSRV.GetAddressOf());

		// use setShadowBufferDrawSection to select draw section
		_context->RSSetState(_rasterizer.pShadowRasterizerState.Get());
		
		// unbind shadow buffer as a resource, so that we can write to it
		_context->PSSetShader(nullptr, NULL, 0u);
		_context->PSSetShaderResources(3, 1, nullSRV);
		
		_context->OMSetBlendState(_blendstate.opaque.Get(), NULL, 0xffffffff);
		_context->OMSetDepthStencilState(_depthbuffer.pDSStateWithDepthTWriteEnabled.Get(), 0);
		_context->OMSetRenderTargets(0, nullptr, _depthbuffer.pShadowDepthStencilView.Get());
		break;

	case tre::DEFERRED_OPAQUE_M: // use normal draw func
		_context->IASetInputLayout(_inputLayout.vertLayout.Get());
		_context->VSSetShader(_vertexShader.pShader.Get(), NULL, 0u);

		_context->RSSetViewports(1, &_viewport.defaultViewport);
		_context->RSSetState(_rasterizer.pRasterizerStateFCCW.Get());
		
		// unbind depth buffer as a shader resource, so that we can write to it
		_context->OMSetRenderTargets(0, nullptr, nullptr);
		_context->PSSetShader(_deferredShader.pShader.Get(), NULL, 0u);
		_context->PSSetShaderResources(4, 1, nullSRV);
		
		_context->OMSetBlendState(_blendstate.opaque.Get(), NULL, 0xffffffff);
		_context->OMSetDepthStencilState(_depthbuffer.pDSStateWithDepthTWriteEnabled.Get(), 0);
		_context->OMSetRenderTargets(2, _gBuffer.rtvs, _depthbuffer.pDepthStencilView.Get());
		break;

	case tre::INSTANCED_DEFERRED_OPAQUE_M: // use normal draw func
		_context->IASetInputLayout(_inputLayout.vertLayout.Get());
		_context->VSSetShader(_vertexShaderInstanced.pShader.Get(), NULL, 0u);
		_context->VSSetShaderResources(0u, 1, _instanceBuffer.pInstanceBufferSRV.GetAddressOf());

		_context->RSSetViewports(1, &_viewport.defaultViewport);
		_context->RSSetState(_rasterizer.pRasterizerStateFCCW.Get());
		
		// unbind depth buffer as a shader resource, so that we can write to it
		_context->OMSetRenderTargets(0, nullptr, nullptr);
		_context->PSSetShader(_instancedPixelShader.pShader.Get(), NULL, 0u);
		_context->PSSetShaderResources(4, 1, nullSRV);
		
		_context->OMSetBlendState(_blendstate.opaque.Get(), NULL, 0xffffffff);
		_context->OMSetDepthStencilState(_depthbuffer.pDSStateWithDepthTWriteEnabled.Get(), 0);
		_context->OMSetRenderTargets(2, _gBuffer.rtvs, _depthbuffer.pDepthStencilView.Get());
		break;

	case tre::DEFERRED_OPAQUE_LIGHTING_ENV_M:
		_context->IASetInputLayout(nullptr);
		_context->VSSetShader(_vertexShaderFullscreenQuad.pShader.Get(), NULL, 0u);

		_context->RSSetViewports(1, &_viewport.defaultViewport);
		_context->RSSetState(_rasterizer.pRasterizerStateFCCW.Get());

		_context->OMSetRenderTargets(0, nullptr, nullptr);
		_context->PSSetShader(_deferredShaderLightingEnv.pShader.Get(), NULL, 0u);
		_context->PSSetShaderResources(0, 1, _gBuffer.pShaderResViewDeferredAlbedo.GetAddressOf()); // albedo
		_context->PSSetShaderResources(1, 1, _gBuffer.pShaderResViewDeferredNormal.GetAddressOf()); // normal
		_context->PSSetShaderResources(3, 1, _depthbuffer.pShadowShaderRescView.GetAddressOf()); // shadow
		_context->PSSetShaderResources(4, 1, _depthbuffer.pDepthStencilShaderRescView.GetAddressOf()); //depth
		_context->PSSetShaderResources(7, 1, _ssao.ssaoBlurredTexture2dSRV.GetAddressOf()); // ssao

		_context->OMSetRenderTargets(1, _hdrBuffer.pRenderTargetViewHdrTexture.GetAddressOf(), nullptr);
		_context->OMSetBlendState(_blendstate.opaque.Get(), NULL, 0xffffffff);
		_context->OMSetDepthStencilState(_depthbuffer.pDSStateWithDepthTWriteEnabled.Get(), 0);
		break;

	case tre::DEFERRED_LIGHTING_LOCAL_M:
		_context->IASetInputLayout(_inputLayout.vertLayout.Get());
		_context->VSSetShader(_vertexShader.pShader.Get(), NULL, 0u);

		_context->RSSetViewports(1, &_viewport.defaultViewport);
		_context->RSSetState(_rasterizer.pRasterizerStateFCCW.Get()); // by default: render only front face

		_context->OMSetRenderTargets(0, nullptr, nullptr);
		_context->PSSetShader(_deferredShaderLightingLocal.pShader.Get(), NULL, 0u);
		_context->PSSetShaderResources(0, 1, _gBuffer.pShaderResViewDeferredAlbedo.GetAddressOf()); // albedo
		_context->PSSetShaderResources(1, 1, _gBuffer.pShaderResViewDeferredNormal.GetAddressOf()); // normal
		_context->CopyResource(_depthbuffer.pDepthStencilReadOnlyTexture.Get(), _depthbuffer.pDepthStencilTexture.Get());
		_context->PSSetShaderResources(4, 1, _depthbuffer.pDepthStencilReadOnlyShaderRescView.GetAddressOf()); //depth

		_context->OMSetBlendState(_blendstate.lighting.Get(), NULL, 0xffffffff);
		_context->OMSetDepthStencilState(_depthbuffer.pDSStateWithDepthTWriteDisabled.Get(), 0); // by default: read only depth test
		_context->OMSetRenderTargets(1, _hdrBuffer.pRenderTargetViewHdrTexture.GetAddressOf(), _depthbuffer.pDepthStencilView.Get()); // draw to HDR floating point buffer
		break;

	case tre::SSAO_FULLSCREEN_PASS:
		_context->IASetInputLayout(nullptr);
		_context->VSSetShader(_vertexShaderFullscreenQuad.pShader.Get(), NULL, 0u);

		_context->RSSetViewports(1, &_viewport.defaultViewport);
		_context->RSSetState(_rasterizer.pRasterizerStateFCCW.Get());

		_context->OMSetRenderTargets(0, nullptr, nullptr);
		_context->PSSetShader(_ssaoPixelShader.pShader.Get(), NULL, 0u);
		_context->PSSetShaderResources(1, 1, _gBuffer.pShaderResViewDeferredNormal.GetAddressOf()); // normal
		_context->PSSetShaderResources(4, 1, _depthbuffer.pDepthStencilReadOnlyShaderRescView.GetAddressOf()); //depth
		_context->PSSetShaderResources(5, 1, _ssao.ssaoNoiseTexture2dSRV.GetAddressOf()); // noise texture

		_context->OMSetBlendState(_blendstate.opaque.Get(), NULL, 0xffffffff);
		_context->OMSetDepthStencilState(_depthbuffer.pDSStateWithDepthTWriteDisabled.Get(), 0); // by default: read only depth test
		_context->OMSetRenderTargets(1, _ssao.ssaoResultTexture2dRTV.GetAddressOf(), nullptr);
		break;

	case tre::SSAO_BLURRING_PASS:
		_context->IASetInputLayout(nullptr);
		_context->VSSetShader(_vertexShaderFullscreenQuad.pShader.Get(), NULL, 0u);

		_context->RSSetViewports(1, &_viewport.defaultViewport);
		_context->RSSetState(_rasterizer.pRasterizerStateFCCW.Get());

		_context->OMSetRenderTargets(0, nullptr, nullptr);
		_context->PSSetShader(_textureBlurPixelShader.pShader.Get(), NULL, 0u);
		_context->PSSetShaderResources(6, 1, _ssao.ssaoResultTexture2dSRV.GetAddressOf()); // normal

		_context->OMSetBlendState(_blendstate.opaque.Get(), NULL, 0xffffffff);
		_context->OMSetDepthStencilState(_depthbuffer.pDSStateWithDepthTWriteDisabled.Get(), 0); // by default: read only depth test
		_context->OMSetRenderTargets(1, _ssao.ssaoBlurredTexture2dRTV.GetAddressOf(), nullptr);
		break;

	case tre::TONE_MAPPING_PASS: 
		_context->IASetInputLayout(nullptr);
		_context->VSSetShader(_vertexShaderFullscreenQuad.pShader.Get(), NULL, 0u);

		_context->RSSetViewports(1, &_viewport.defaultViewport);
		_context->RSSetState(_rasterizer.pRasterizerStateFCCW.Get());

		_context->OMSetRenderTargets(0, nullptr, nullptr);
		_context->PSSetShader(_hdrPixelShader.pShader.Get(), NULL, 0u);
		_context->PSSetShaderResources(0u, 1u, _hdrBuffer.pShaderResViewHdrTexture.GetAddressOf()); // hdr texture
		_context->PSSetShaderResources(1u, 1u, _hdrBuffer.pLuminAvgSRV.GetAddressOf());

		_context->OMSetBlendState(_blendstate.opaque.Get(), NULL, 0xffffffff);
		_context->OMSetDepthStencilState(_depthbuffer.pDSStateWithDepthTWriteDisabled.Get(), 0); // by default: read only depth test
		_context->OMSetRenderTargets(1, &currRenderTargetView, nullptr);
		break;
	}
}

void Renderer::fullscreenPass(tre::RENDER_MODE mode) {

	const char* name = ToString(mode);
	MICROPROFILE_SCOPE_CSTR(name);
	PROFILE_GPU_SCOPED("Fullscreen Pass");

	configureStates(mode);

	_context->Draw(6, 0);
}

void Renderer::deferredLightingLocalDraw(const std::vector<std::pair<Object*, Mesh*>> objQ, XMVECTOR cameraPos) {
	if (objQ.size() == 0) return;

	const char* name = ToString(DEFERRED_LIGHTING_LOCAL_M);
	MICROPROFILE_SCOPE_CSTR(name);
	PROFILE_GPU_SCOPED("Deferred Local Light Draw");

	configureStates(RENDER_MODE::DEFERRED_LIGHTING_LOCAL_M);

	UINT vertexStride = sizeof(Vertex);
	UINT offset = 0;

	//Set vertex buffer
	_context->IASetVertexBuffers(0, 1, objQ[0].second->pVertexBuffer.GetAddressOf(), &vertexStride, &offset);

	//Set index buffer
	_context->IASetIndexBuffer(objQ[0].second->pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	for (int i = 0; i < objQ.size(); i++) {

		//Config and set const buffer
		tre::ConstantBuffer::setObjConstBuffer(
			_device, _context,
			objQ[i].first->_transformationFinal,
			objQ[i].second->pMaterial->baseColor,
			0,
			0
		);

		tre::ConstantBuffer::setLightingVolumeConstBuffer(_device, _context, i);

		float distFromObjToCam = tre::Maths::distBetweentObjToCam(objQ[i].first->objPos, cameraPos);
		
		// if the camera is inside the light sphere
		if (distFromObjToCam < objQ[i].first->objScale.x) {
			_context->RSSetState(_rasterizer.pRasterizerStateFCW.Get()); // render only back face
			_context->OMSetDepthStencilState(_depthbuffer.pDSStateWithoutDepthT.Get(), 0); // all depth test pass to render the sphere
		}

		_context->DrawIndexed(objQ[i].second->indexSize, 0, 0);
	}
}


void Renderer::draw(const std::vector<std::pair<Object*, Mesh*>> objQ, RENDER_MODE renderObjType) {
	if (objQ.size() == 0) return;

	const char* name = ToString(renderObjType);
	MICROPROFILE_SCOPE_CSTR(name);
	PROFILE_GPU_SCOPED("Forward Draw");
	
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
		if (objQ[i].second->pMaterial != nullptr) {
			if (objQ[i].second->pMaterial->objTexture != nullptr) {
				_context->PSSetShaderResources(0, 1, objQ[i].second->pMaterial->objTexture->pShaderResView.GetAddressOf());
				hasTexture = 1;
			}

			// set normal map
			if (objQ[i].second->pMaterial->objNormalMap != nullptr) {
				_context->PSSetShaderResources(1, 1, objQ[i].second->pMaterial->objNormalMap->pShaderResView.GetAddressOf());
				hasNormal = 1;
			}
		}

		//Config and set const buffer
		tre::ConstantBuffer::setObjConstBuffer(
			_device, _context,
			objQ[i].first->_transformationFinal,
			objQ[i].second->pMaterial->baseColor,
			hasTexture,
			hasNormal
		);

		_context->DrawIndexed(objQ[i].second->indexSize, 0, 0);
	}
}

void Renderer::instancedDraw(const std::vector<std::pair<Object*, Mesh*>>& objQ, RENDER_MODE renderMode) {
	if (objQ.size() == 0) return;

	const char* name = ToString(renderMode);
	MICROPROFILE_SCOPE_CSTR(name);
	PROFILE_GPU_SCOPED("Instanced Draw");

	_instanceBuffer.updateBuffer(objQ);

	configureStates(renderMode);

	UINT vertexStride = sizeof(Vertex);
	UINT offset = 0;

	//Set vertex buffer
	_context->IASetVertexBuffers(0, 1, objQ[0].second->pVertexBuffer.GetAddressOf(), &vertexStride, &offset);

	//Set index buffer
	_context->IASetIndexBuffer(objQ[0].second->pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	Mesh* pCurrMesh = objQ[0].second;
	Texture* pCurrTexture = nullptr;
	Texture* pCurrNormTexture = nullptr;

	if (objQ[0].second->pMaterial != nullptr) {
		if (objQ[0].second->pMaterial->objTexture != nullptr) {
			pCurrTexture = objQ[0].second->pMaterial->objTexture;
			_context->PSSetShaderResources(0u, 1u, objQ[0].second->pMaterial->objTexture->pShaderResView.GetAddressOf());
		}

		// set normal map
		if (objQ[0].second->pMaterial->objNormalMap != nullptr) {
			pCurrNormTexture = objQ[0].second->pMaterial->objNormalMap;
			_context->PSSetShaderResources(1u, 1u, objQ[0].second->pMaterial->objNormalMap->pShaderResView.GetAddressOf());
		}
	}

	int batchStartIdx = 0;

	for (int i = 0; i < objQ.size(); i++) {
		if (pCurrMesh != objQ[i].second && pCurrTexture != objQ[i].second->pMaterial->objTexture) {
			tre::ConstantBuffer::setBatchInfoConstBuffer(_device, _context, batchStartIdx);

			_context->DrawIndexedInstanced(pCurrMesh->indexSize, i - batchStartIdx, 0u, 0u, 0u);

			// update new batch info
			batchStartIdx = i;
			pCurrMesh = objQ[i].second;

			if (objQ[i].second->pMaterial != nullptr) {
				if (objQ[i].second->pMaterial->objTexture != nullptr) {
					pCurrTexture = objQ[i].second->pMaterial->objTexture;
					_context->PSSetShaderResources(0u, 1u, objQ[i].second->pMaterial->objTexture->pShaderResView.GetAddressOf());
				}

				// set normal map
				if (objQ[i].second->pMaterial->objNormalMap != nullptr) {
					pCurrNormTexture = objQ[i].second->pMaterial->objNormalMap;
					_context->PSSetShaderResources(1u, 1u, objQ[i].second->pMaterial->objNormalMap->pShaderResView.GetAddressOf());
				}
			}
		}
	}
}

void Renderer::debugDraw(const std::vector<std::pair<Object*, Mesh*>> objQ, Mesh& mesh, BoundVolumeEnum typeOfBound, RENDER_MODE renderObjType) {
	if (objQ.size() == 0) return;

	const char* name = ToString(renderObjType);
	MICROPROFILE_SCOPE_CSTR(name);
	PROFILE_GPU_SCOPED("Debug Draw");

	configureStates(renderObjType);

	UINT vertexStride = sizeof(Vertex);
	UINT offset = 0;
	
	//Set vertex buffer
	_context->IASetVertexBuffers(0, 1, mesh.pVertexBuffer.GetAddressOf(), &vertexStride, &offset);

	//Set index buffer
	_context->IASetIndexBuffer(mesh.pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	for (int i = 0; i < objQ.size(); i++) {

		tre::Object* currObj = objQ[i].first;

		for (int j = 0; j < currObj->pObjMeshes.size(); j++) {

			//Config and set const buffer
			tre::ConstantBuffer::setObjConstBuffer(
				_device, _context,
				currObj->_boundingVolumeTransformation,
				currObj->_boundingVolumeColor[j],
				0, // bounding volume has no texture
				0 // bounding volume has no normal
			);

			_context->DrawIndexed(mesh.indexSize, 0, 0);
		}
	}
}
}