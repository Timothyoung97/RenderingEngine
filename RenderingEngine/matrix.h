#pragma once

#include <DirectXMath.h>

using namespace DirectX;

namespace tre {
class Matrix {

public:
	static XMMATRIX createTransformationMatrix(XMFLOAT3 scale, XMFLOAT3 rotation, XMFLOAT3 position);
	static XMVECTOR getMatrixNormUp(XMMATRIX matrix);
	static XMVECTOR getMatrixNormRight(XMMATRIX matrix);
	static XMVECTOR getMatrixNormForward(XMMATRIX matrix);
};
}