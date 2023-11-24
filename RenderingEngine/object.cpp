#include "object.h"
#include "colors.h"

namespace tre {

void ObjectUtility::updateBoundingVolumeTransformation(Object& obj, BoundVolumeEnum typeOfBound) {
	obj._boundingVolumeTransformation.clear();
	for (int i = 0; i < obj.pObjMeshes.size(); i++) {
		switch (typeOfBound) {
		case tre::AABBBoundingBox:
			obj._boundingVolumeTransformation.push_back(tre::BoundingVolume::updateAABB(obj.pObjMeshes[i]->aabb, obj.aabb[i], obj._transformationFinal));
			break;
		case tre::RitterBoundingSphere:
			obj._boundingVolumeTransformation.push_back(tre::BoundingVolume::updateBoundingSphere(obj.pObjMeshes[i]->ritterSphere, obj.ritterBs[i], obj._transformationFinal));
			break;
		case tre::NaiveBoundingSphere:
			obj._boundingVolumeTransformation.push_back(tre::BoundingVolume::updateBoundingSphere(obj.pObjMeshes[i]->naiveSphere, obj.naiveBs[i], obj._transformationFinal));
			break;
		}
	}

	// update for children
	for (Object& child : obj.children) {
		updateBoundingVolumeTransformation(child, typeOfBound);
	}
}

XMMATRIX ObjectUtility::makeLocalToWorldMatrix(Object& obj) {
	XMMATRIX localTransformation = Maths::createTransformationMatrix(obj.objScale, obj.objRotation, obj.objPos);

	if (obj.parent != nullptr) {
		XMMATRIX parentMatrix = makeLocalToWorldMatrix(*obj.parent);
		localTransformation = XMMatrixMultiply(parentMatrix, localTransformation);
	}

	return localTransformation;
}

bool ObjectUtility::isMeshWithinView(Object& obj, int meshIdx, Frustum& frustum, BoundVolumeEnum typeOfBound, bool isCameraView) {
	int isWithinView = 0;

	switch (typeOfBound) {
	case RitterBoundingSphere:
		if (obj.ritterBs[meshIdx].isInFrustum(frustum)) {
			isWithinView = 2;
		}
		else if (obj.ritterBs[meshIdx].isOverlapFrustum(frustum)) {
			isWithinView = 1;
		}
		else {
			//addToQ = 1; //debug
		}

		break;

	case NaiveBoundingSphere:
		if (obj.naiveBs[meshIdx].isInFrustum(frustum)) {
			isWithinView = 2;
		}
		else if (obj.naiveBs[meshIdx].isOverlapFrustum(frustum)) {
			isWithinView = 1;
		}
		else {
			//addToQ = 1; //debug
		}

		break;

	case AABBBoundingBox:
		if (obj.aabb[meshIdx].isInFrustum(frustum)) {
			isWithinView = 2;
		}
		else if (obj.aabb[meshIdx].isOverlapFrustum(frustum)) {
			isWithinView = 1;
		}
		else {
			//addToQ = 1; //debug
		}

		break;
	}

	if (isCameraView) {
		obj.isInView[meshIdx] = isWithinView != 0;
		switch (isWithinView)
		{
		case 2:
			obj._boundingVolumeColor[meshIdx] = tre::colorF(Colors::LightGreen);
			break;
		case 1:
			obj._boundingVolumeColor[meshIdx] = tre::colorF(Colors::Blue);
			break;
		default:
			obj._boundingVolumeColor[meshIdx] = tre::colorF(Colors::Red);
			break;
		}
	}

	return isWithinView > 0;
}

}