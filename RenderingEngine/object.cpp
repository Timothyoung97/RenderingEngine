#include "object.h"
#include "colors.h"

namespace tre {
	
XMMATRIX Object::makeLocalToWorldMatrix() {
	XMMATRIX localTransformation = Maths::createTransformationMatrix(this->objScale, this->objRotation, this->objPos);

	if (this->parent != nullptr) {
		XMMATRIX parentMatrix = this->parent->makeLocalToWorldMatrix();
		localTransformation = XMMatrixMultiply(parentMatrix, localTransformation);
	}

	return localTransformation;
}

bool Object::isMeshWithinView(int meshIdx, Frustum& frustum, BoundVolumeEnum typeOfBound) {
	bool isWithinView = false;

	switch (typeOfBound) {
	case RitterBoundingSphere:
		if (ritterBs[meshIdx].isInFrustum(frustum)) {
			_boundingVolumeColor[meshIdx] = tre::colorF(Colors::LightGreen);
			isWithinView = true;
		}
		else if (ritterBs[meshIdx].isOverlapFrustum(frustum)) {
			_boundingVolumeColor[meshIdx] = tre::colorF(Colors::Blue);
			isWithinView = true;
		}
		else {
			_boundingVolumeColor[meshIdx] = tre::colorF(Colors::Red);
			//addToQ = 1; //debug
		}

		break;

	case NaiveBoundingSphere:
		if (naiveBs[meshIdx].isInFrustum(frustum)) {
			_boundingVolumeColor[meshIdx] = tre::colorF(Colors::LightGreen);
			isWithinView = true;
		}
		else if (naiveBs[meshIdx].isOverlapFrustum(frustum)) {
			_boundingVolumeColor[meshIdx] = tre::colorF(Colors::Blue);
			isWithinView = true;
		}
		else {
			_boundingVolumeColor[meshIdx] = tre::colorF(Colors::Red);
			//addToQ = 1; //debug
		}

		break;

	case AABBBoundingBox:
		if (aabb[meshIdx].isInFrustum(frustum)) {
			_boundingVolumeColor[meshIdx] = tre::colorF(Colors::LightGreen);
			isWithinView = true;
		}
		else if (aabb[meshIdx].isOverlapFrustum(frustum)) {
			_boundingVolumeColor[meshIdx] = tre::colorF(Colors::Blue);
			isWithinView = true;
		}
		else {
			_boundingVolumeColor[meshIdx] = tre::colorF(Colors::Red);
			//addToQ = 1; //debug
		}

		break;
	}

	return isWithinView;
}

}