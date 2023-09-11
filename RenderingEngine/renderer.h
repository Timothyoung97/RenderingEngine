#pragma once

#include <d3d11.h>

#include <vector>

#include "device.h"
#include "object.h"
#include "constbuffer.h"

namespace tre {
class Renderer {
public:
	void draw(ID3D11Device* device, ID3D11DeviceContext* context, ID3D11RasterizerState* rasterizerState, tre::ConstantBuffer& cb, const std::vector<Object>& objQ);
	void debugDraw(ID3D11Device* device, ID3D11DeviceContext* context, ID3D11RasterizerState* rasterizerState, tre::ConstantBuffer& cb, const std::vector<Object>& objQ, const Mesh& sphere, int typeOfBound);
};
}