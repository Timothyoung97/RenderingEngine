#include "camera.h"

namespace tre {

Camera::Camera(float width, float height) {
	defaultUpV = XMVectorSet(.0f, 1.0f, .0f, .0f); // always point up in the positive y axis

	// Init Values for Camera
	camPositionV = XMVectorSet(.0f, 5.0f, -10.0f, .0f);
	camUpV = XMVectorSet(.0f, 1.0f, .0f, .0f);
	camRightV = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);

	yaw = 90;
	pitch = 0;
	
	// Camera's front facing direction
	directionF.x = XMScalarCos(XMConvertToRadians(yaw)) * XMScalarCos(XMConvertToRadians(pitch));
	directionF.y = XMScalarSin(XMConvertToRadians(pitch));
	directionF.z = XMScalarSin(XMConvertToRadians(yaw)) * XMScalarCos(XMConvertToRadians(pitch));

	directionV = XMVector3Normalize(XMLoadFloat3(&directionF));
	
	//Speed setting
	cameraMoveSpeed = .05f;
	cameraRotateSpeed = .1f;

	// Camera View Matrix
	camView = XMMatrixLookAtLH(camPositionV, camPositionV + directionV, camUpV);

	// Projection Matrix
	camProjection = XMMatrixPerspectiveFovLH(XMConvertToRadians(45.0f), static_cast<float>(width) / height, 1.0f, 1000.0f);

	camViewProjection = XMMatrixMultiply(camView, camProjection);

	// create frustum
	updateCameraFrustum();
}

void Camera::moveCamera(XMVECTOR offset) {
	camPositionV = XMVectorAdd(camPositionV, offset * cameraMoveSpeed);
	updateCamera();
}

void Camera::turnCamera(float yawOffset, float pitchOffset) {
	yaw -= yawOffset * cameraRotateSpeed;
	pitch -= pitchOffset * cameraRotateSpeed;
	
	pitch = pitch > 90.f ? 89.9f : pitch < -90.f ? -89.9f : pitch;

	directionF.x = XMScalarCos(XMConvertToRadians(yaw)) * XMScalarCos(XMConvertToRadians(pitch));
	directionF.y = XMScalarSin(XMConvertToRadians(pitch));
	directionF.z = XMScalarSin(XMConvertToRadians(yaw)) * XMScalarCos(XMConvertToRadians(pitch));

	directionV = XMVector3Normalize(XMLoadFloat3(&directionF));
	updateCamera();
}

void Camera::updateCamera() {
	camRightV = XMVector3Normalize(XMVector3Cross(directionV, defaultUpV));
	camUpV = XMVector3Normalize(XMVector3Cross(camRightV, directionV));
	camView = XMMatrixLookAtLH(camPositionV, camPositionV + directionV, camUpV);
	camViewProjection = XMMatrixMultiply(camView, camProjection);
	updateCameraFrustum();
}

void Camera::updateCameraFrustum() {

	XMFLOAT4X4 viewProjectionF;
	XMStoreFloat4x4(&viewProjectionF, camViewProjection);

	// add 1st col to 4th col of viewProj
	cameraFrustum.leftF.eqn = XMFLOAT4(
		-(viewProjectionF._14 + viewProjectionF._11),
		-(viewProjectionF._24 + viewProjectionF._21),
		-(viewProjectionF._34 + viewProjectionF._31),
		viewProjectionF._44 + viewProjectionF._41
	);
	cameraFrustum.leftF.normalizePlane();

	// minus 1st col from 4th col of viewPr0j
	cameraFrustum.rightF.eqn = XMFLOAT4(
		-(viewProjectionF._14 - viewProjectionF._11),
		-(viewProjectionF._24 - viewProjectionF._21),
		-(viewProjectionF._34 - viewProjectionF._31),
		viewProjectionF._44 - viewProjectionF._41
	);
	cameraFrustum.rightF.normalizePlane();

	// minus 2nd col from 4th col 
	cameraFrustum.topF.eqn = XMFLOAT4(
		-(viewProjectionF._14 - viewProjectionF._12),
		-(viewProjectionF._24 - viewProjectionF._22),
		-(viewProjectionF._34 - viewProjectionF._32),
		viewProjectionF._44 - viewProjectionF._42
	);
	cameraFrustum.topF.normalizePlane();

	// add 2nd col to 4th col
	cameraFrustum.bottomF.eqn = XMFLOAT4(
		-(viewProjectionF._14 + viewProjectionF._12),
		-(viewProjectionF._24 + viewProjectionF._22),
		-(viewProjectionF._34 + viewProjectionF._32),
		viewProjectionF._44 + viewProjectionF._42
	);
	cameraFrustum.bottomF.normalizePlane();

	// 3rd col itself + 4th
	cameraFrustum.nearF.eqn = XMFLOAT4(
		-(viewProjectionF._13 + viewProjectionF._14),
		-(viewProjectionF._23 + viewProjectionF._24),
		-(viewProjectionF._33 + viewProjectionF._34),
		viewProjectionF._43 + viewProjectionF._44
	);
	cameraFrustum.nearF.normalizePlane();

	// minus 3rd col from 4th col
	cameraFrustum.farF.eqn = XMFLOAT4(
		-(viewProjectionF._14 - viewProjectionF._13),
		-(viewProjectionF._24 - viewProjectionF._23),
		-(viewProjectionF._34 - viewProjectionF._33),
		viewProjectionF._44 - viewProjectionF._43
	);
	cameraFrustum.farF.normalizePlane();
}
}