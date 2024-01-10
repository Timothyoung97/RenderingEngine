#include "camera.h"
#include "window.h"

namespace tre {

Camera::Camera() {
	this->init(SCREEN_WIDTH, SCREEN_HEIGHT);
}

void Camera::init(float width, float height) {
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
	camProjection = XMMatrixPerspectiveFovLH(XMConvertToRadians(fovY), static_cast<float>(width) / height, .1f, 10000.f);

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
	camProjection = XMMatrixPerspectiveFovLH(XMConvertToRadians(fovY), static_cast<float>(tre::SCREEN_WIDTH) / tre::SCREEN_HEIGHT, .1f, 10000.f);
	camRightV = XMVector3Normalize(XMVector3Cross(directionV, defaultUpV));
	camUpV = XMVector3Normalize(XMVector3Cross(camRightV, directionV));
	camView = XMMatrixLookAtLH(camPositionV, camPositionV + directionV, camUpV);
	camViewProjection = XMMatrixMultiply(camView, camProjection);
	//{	// debug line
	//	XMMATRIX debugCamViewProjection = XMMatrixMultiply(camView, XMMatrixPerspectiveFovLH(XMConvertToRadians(10.f), static_cast<float>(1920.f) / 1080.f , .1f, 250.f)); 
	//	cameraFrustum = Maths::createFrustumFromViewProjectionMatrix(debugCamViewProjection); 
	//}
	cameraFrustum = Maths::createFrustumFromViewProjectionMatrix(camViewProjection);
}

}