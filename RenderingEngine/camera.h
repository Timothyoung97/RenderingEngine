#pragma once

#include "maths.h"

namespace tre {

class Camera {
public:

	XMVECTOR defaultUpV;

	XMVECTOR camPositionV;
	XMVECTOR camUpV;
	XMVECTOR camRightV;

	XMFLOAT3 directionF;
	XMVECTOR directionV;

	float yaw;
	float pitch;

	float cameraMoveSpeed;
	float cameraRotateSpeed;

	XMMATRIX camView;
	XMMATRIX camProjection;

	Frustum cameraFrustum;

	float aspect;
	float fovY;
	float zNear;
	float zFar;

	Camera(float width, float height, float fovY, float zNear, float zFar);

	void updateCamera();
	void moveCamera(XMVECTOR offset);
	void turnCamera(float yawOffset, float pitchOffset);
	void updateCameraFrustum();
};
}