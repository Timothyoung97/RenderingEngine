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
	static float distBetweentObjToCam(XMFLOAT3 objPosF, XMVECTOR camPosV);
	static XMFLOAT3 getRotatePosition(XMFLOAT3 objOrigin, float stackAngle, float sectorAngle, float radius);
	static float XMFLOAT3DotProduct(XMFLOAT3 pt1, XMFLOAT3 pt2);
	static XMFLOAT3 XMFLOAT3Addition(XMFLOAT3 a, XMFLOAT3 b);
	static XMFLOAT3 XMFLOAT3Minus(XMFLOAT3 a, XMFLOAT3 b);
	static XMFLOAT3 XMFLOAT3ScalarMultiply(XMFLOAT3 a, float x);
};
}