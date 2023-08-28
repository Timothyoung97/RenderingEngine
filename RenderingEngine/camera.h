#pragma once

#include <DirectXMath.h>

using namespace DirectX;

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

	Camera(float width, float height);

	void updateCamera();
	void moveCamera(float offset);
	void turnCamera(float yawOffset, float pitchOffset);
};
}