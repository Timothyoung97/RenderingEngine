#pragma once

#include <d3d11.h>

#include <vector>

#include "device.h"
#include "object.h"
#include "constbuffer.h"
#include "blendstate.h"

namespace tre {

enum RENDER_MODE {
	TRANSPARENT_M,
	OPAQUE_M,
	WIREFRAME_M,
	SHADOW_M
};

class Renderer {
public:

	ID3D11Device* _device;
	ID3D11DeviceContext* _context;

	BlendState _blendstate;

	Renderer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

	void configureContext(RENDER_MODE renderMode);
	void draw(ID3D11RasterizerState* rasterizerState, const std::vector<Object>& objQ, RENDER_MODE renderMode);
	void debugDraw(ID3D11RasterizerState* rasterizerState, std::vector<Object>& objQ, Mesh& mesh, BoundVolumeEnum typeOfBound, RENDER_MODE renderMode);
};
}