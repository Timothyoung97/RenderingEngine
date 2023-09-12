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

}