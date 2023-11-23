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

bool Object::isMeshWithinView(int meshIdx, Frustum& frustum, BoundVolumeEnum typeOfBound, bool toChangeColor) {
	int isWithinView = 0;

	switch (typeOfBound) {
	case RitterBoundingSphere:
		if (ritterBs[meshIdx].isInFrustum(frustum)) {
			isWithinView = 2;
		}
		else if (ritterBs[meshIdx].isOverlapFrustum(frustum)) {
			isWithinView = 1;
		}
		else {
			_boundingVolumeColor[meshIdx] = tre::colorF(Colors::Red);
			//addToQ = 1; //debug
		}

		break;

	case NaiveBoundingSphere:
		if (naiveBs[meshIdx].isInFrustum(frustum)) {
			isWithinView = 2;
		}
		else if (naiveBs[meshIdx].isOverlapFrustum(frustum)) {
			isWithinView = 1;
		}
		else {
			//addToQ = 1; //debug
		}

		break;

	case AABBBoundingBox:
		if (aabb[meshIdx].isInFrustum(frustum)) {
			isWithinView = 2;
		}
		else if (aabb[meshIdx].isOverlapFrustum(frustum)) {
			isWithinView = 1;
		}
		else {
			//addToQ = 1; //debug
		}

		break;
	}

	if (toChangeColor) {
		switch (isWithinView)
		{
		case 2:
			_boundingVolumeColor[meshIdx] = tre::colorF(Colors::LightGreen);
			break;
		case 1:
			_boundingVolumeColor[meshIdx] = tre::colorF(Colors::Blue);
			break;
		default:
			_boundingVolumeColor[meshIdx] = tre::colorF(Colors::Red);
			break;
		}
	}

	return isWithinView;
}

}