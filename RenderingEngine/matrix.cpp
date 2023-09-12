#include "matrix.h"

namespace tre {

XMMATRIX Matrix::createTransformationMatrix(XMFLOAT3 scale, XMFLOAT3 rotation, XMFLOAT3 position) {
	return XMMatrixMultiply(
		XMMatrixScaling(scale.x, scale.y, scale.z),
		XMMatrixMultiply(
			XMMatrixRotationRollPitchYaw(XMConvertToRadians(rotation.x), XMConvertToRadians(rotation.y), XMConvertToRadians(rotation.z)),
			XMMatrixTranslation(
				position.x,
				position.y,
				position.z)
		)
	);
}

XMVECTOR Matrix::getMatrixNormUp(XMMATRIX matrix) {
	XMFLOAT4X4 transformationF;
	XMStoreFloat4x4(&transformationF, matrix);

	return XMVector3Normalize(XMVECTOR{ transformationF._21, transformationF._22, transformationF._23 });
};

XMVECTOR Matrix::getMatrixNormRight(XMMATRIX matrix) {
	XMFLOAT4X4 transformationF;
	XMStoreFloat4x4(&transformationF, matrix);

	return XMVector3Normalize(XMVECTOR{ transformationF._11, transformationF._12, transformationF._13 });
};

XMVECTOR Matrix::getMatrixNormForward(XMMATRIX matrix) {
	XMFLOAT4X4 transformationF;
	XMStoreFloat4x4(&transformationF, matrix);

	return XMVector3Normalize(XMVECTOR{ transformationF._31, transformationF._32, transformationF._33 });
};

float Matrix::distBetweentObjToCam(XMFLOAT3 objPosF, XMVECTOR camPosV) {

	XMVECTOR objPosV{ objPosF.x, objPosF.y, objPosF.z };

	XMVECTOR distFromCamV = XMVector3Length(objPosV - camPosV); // length of vector is replicated in all components 

	XMFLOAT4 distFromCamF;
	XMStoreFloat4(&distFromCamF, distFromCamV);

	return distFromCamF.x;
}

XMFLOAT3 Matrix::getRotatePosition(XMFLOAT3 objOrigin, float stackAngle, float sectorAngle, float radius) {
	XMFLOAT3 rotatedPosition;
	rotatedPosition.x = objOrigin.x + radius * XMScalarCos(XMConvertToRadians(sectorAngle)) * XMScalarCos(XMConvertToRadians(stackAngle));
	rotatedPosition.y = objOrigin.y + radius * XMScalarSin(XMConvertToRadians(stackAngle));
	rotatedPosition.z = objOrigin.z + radius * XMScalarSin(XMConvertToRadians(sectorAngle)) * XMScalarCos(XMConvertToRadians(stackAngle));
	return rotatedPosition;
}

XMFLOAT3 Matrix::XMFLOAT3Addition(XMFLOAT3 a, XMFLOAT3 b) {
	return XMFLOAT3(a.x + b.x, a.y + b.y, a.z + b.z);
}

XMFLOAT3 Matrix::XMFLOAT3Minus(XMFLOAT3 a, XMFLOAT3 b) {
	return XMFLOAT3(a.x - b.x, a.y - b.y, a.z - b.z);
}

XMFLOAT3 Matrix::XMFLOAT3ScalarMultiply(XMFLOAT3 a, float x) {
	return XMFLOAT3(a.x * x, a.y * x, a.z * x);
}

float Matrix::XMFLOAT3DotProduct(XMFLOAT3 pt1, XMFLOAT3 pt2) {
	XMVECTOR pV1 = XMLoadFloat3(&pt1), pV2 = XMLoadFloat3(&pt2);

	XMFLOAT3 ans;
	XMStoreFloat3(&ans, XMVector3Dot(pV1, pV2));
	return ans.x;
}
}