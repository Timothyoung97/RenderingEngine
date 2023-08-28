#include "camera.h"

namespace tre {

Camera::Camera(float width, float height) {
	defaultUpV = XMVectorSet(.0f, 1.0f, .0f, .0f); // always point up in the positive y axis

	// Init Values for Camera
	camPositionV = XMVectorSet(.0f, .0f, -3.0f, .0f);
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
	cameraMoveSpeed = .001f;
	cameraRotateSpeed = .01f;

	// Camera View Matrix
	camView = XMMatrixLookAtLH(camPositionV, camPositionV + directionV, camUpV);

	// Projection Matrix
	camProjection = XMMatrixPerspectiveFovLH(XMConvertToRadians(45.0f), static_cast<float>(width / height), 1.0f, 1000.0f);
}

void Camera::moveCamera(XMVECTOR offset) {
	camPositionV = XMVectorAdd(camPositionV, offset * cameraMoveSpeed);
	updateCamera();
}

void Camera::turnCamera(float yawOffset, float pitchOffset) {
	yaw -= yawOffset * cameraRotateSpeed;
	pitch -= pitchOffset * cameraRotateSpeed;

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
}
}