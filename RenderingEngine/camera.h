#pragma once

#include "maths.h"
#include "window.h"

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

	XMMATRIX camViewProjection;

	Frustum cameraFrustum;

	float fovY = 45.0f; // default

	Camera();

	void init(float width, float height);
	void updateCamera();
	void moveCamera(XMVECTOR offset);
	void turnCamera(float yawOffset, float pitchOffset);
};
}