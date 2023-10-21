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
	camProjection = XMMatrixPerspectiveFovLH(XMConvertToRadians(fovY), static_cast<float>(width) / height, .1f, 100.f);

	camViewProjection = XMMatrixMultiply(camView, camProjection);

	// create frustum
	cameraFrustum = Maths::createFrustumFromViewProjectionMatrix(camViewProjection);
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
	cameraFrustum = Maths::createFrustumFromViewProjectionMatrix(camViewProjection);
}

}