#include "boundingvolume.h"

#include "utility.h"
#include "mesh.h"

namespace tre {

BoundingSphere BoundingVolume::createRitterBS(const std::vector<XMFLOAT3>& uniquePoint) {
	
	BoundingSphere bs;

	float dx, dy, dz;
	float rad2;
	float oldToP, oldToP2, oldToNew;

	XMFLOAT3 xMax, xMin, yMax, yMin, zMax, zMin, dia1, dia2;

	xMin.x = yMin.y = zMin.z = INT_MAX;
	xMax.x = yMax.y = zMax.z = INT_MIN;

	// FIRST PASS: Find 6 minima/maxima points
	for (int i = 0; i < uniquePoint.size(); i++) {
		if (uniquePoint[i].x < xMin.x) xMin = uniquePoint[i];
		if (uniquePoint[i].x > xMax.x) xMax = uniquePoint[i];
		if (uniquePoint[i].y < yMin.y) yMin = uniquePoint[i];
		if (uniquePoint[i].y > yMax.y) yMax = uniquePoint[i];
		if (uniquePoint[i].z < zMin.z) zMin = uniquePoint[i];
		if (uniquePoint[i].z > zMax.z) zMax = uniquePoint[i];
	}

	// Set xspan = distance between 2 points xMin and xMax (squared)
	dx = xMax.x - xMin.x;
	dy = xMax.y - xMin.y;
	dz = xMax.z - xMin.z;

	// Same for y & z spans
	dx = yMax.x - yMin.x;
	dy = yMax.y - yMin.y;
	dz = yMax.z - yMin.z;
	
	dx = zMax.x - zMin.x;
	dy = zMax.y - zMin.y;
	dz = zMax.z - zMin.z;

	// set points dia1 & dia2 to the maximally separated pair
	dia1 = xMin;
	dia2 = xMax;
	bs.center.x = (dia1.x + dia2.x) / 2;

	dia1 = yMin; 
	dia2 = yMax;
	bs.center.y = (dia1.y + dia2.y) / 2;

	dia1 = zMin; 
	dia2 = zMax;
	bs.center.z = (dia1.z + dia2.z) / 2;

	// cal initial radius
	dx = dia2.x - bs.center.x;
	dy = dia2.y - bs.center.y;
	dz = dia2.z - bs.center.z;
	rad2 = dx * dx + dy * dy + dz * dz;
	bs.radius = sqrtf(rad2);

	// 2nd pass: increment curr sphere
	for (int i = 0; i < uniquePoint.size(); i++) {
		dx = uniquePoint[i].x - bs.center.x;
		dy = uniquePoint[i].y - bs.center.y;
		dz = uniquePoint[i].z - bs.center.z;

		oldToP2 = dx * dx + dy * dy + dz * dz;

		if (oldToP2 > rad2) {
			oldToP = sqrtf(oldToP2);
			bs.radius = (bs.radius + oldToP) / 2;
			rad2 = bs.radius * bs.radius;
			oldToNew = oldToP - bs.radius;

			bs.center.x = (bs.radius * bs.center.x + oldToNew * uniquePoint[i].x) / oldToP;
			bs.center.y = (bs.radius * bs.center.y + oldToNew * uniquePoint[i].y) / oldToP;
			bs.center.z = (bs.radius * bs.center.z + oldToNew * uniquePoint[i].z) / oldToP;
		}
	}

	return bs;
}

BoundingSphere BoundingVolume::createNaiveBS(const std::vector<XMFLOAT3>& uniquePoint) {

	BoundingSphere bs;

	for (int i = 0; i < uniquePoint.size(); i++) {
		XMFLOAT3 diff;
		diff.x = uniquePoint[i].x - bs.center.x;
		diff.y = uniquePoint[i].y - bs.center.y;
		diff.z = uniquePoint[i].z - bs.center.z;

		float dist = sqrtf(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);
		
		if (dist > bs.radius) bs.radius = dist;
	}

	return bs;
}

AABB BoundingVolume::createAABB(const std::vector<XMFLOAT3>& uniquePoint) {

	AABB bv;
	double dx, dy, dz;
	double oldToP, oldToP2, oldToNew;

	XMFLOAT3 xMax, xMin, yMax, yMin, zMax, zMin;

	xMin.x = yMin.y = zMin.z = INT_MAX;
	xMax.x = yMax.y = zMax.z = INT_MIN;

	// FIRST PASS: Find 6 minima/maxima points
	for (int i = 0; i < uniquePoint.size(); i++) {
		if (uniquePoint[i].x < xMin.x) xMin = uniquePoint[i];
		if (uniquePoint[i].x > xMax.x) xMax = uniquePoint[i];
		if (uniquePoint[i].y < yMin.y) yMin = uniquePoint[i];
		if (uniquePoint[i].y > yMax.y) yMax = uniquePoint[i];
		if (uniquePoint[i].z < zMin.z) zMin = uniquePoint[i];
		if (uniquePoint[i].z > zMax.z) zMax = uniquePoint[i];
	}

	// set points dia1 & dia2 to the maximally separated pair
	bv.center.x = (xMin.x + xMax.x) / 2;
	bv.halfExtent.x = (xMax.x - xMin.x) / 2.0f;

	bv.center.y = (yMin.y + yMax.y) / 2;
	bv.halfExtent.y = (yMax.y - yMin.y) / 2.0f;

	bv.center.z = (zMin.z + zMax.z) / 2;
	bv.halfExtent.z = (zMax.z - zMin.z) / 2.0f;

	return bv;
}	

XMFLOAT3 updateCenter(XMFLOAT3 center, XMMATRIX transformation) {
	// update center
	XMVECTOR localCenterV = { center.x, center.y, center.z, 1 };

	localCenterV = XMVector4Transform(localCenterV, transformation);

	// store new center
	XMFLOAT3 newCenter;
	XMStoreFloat3(&newCenter, localCenterV);

	return newCenter;
}

XMMATRIX BoundingVolume::updateBoundingSphere(BoundingSphere& meshSphere, BoundingSphere& objSphere, XMMATRIX transformation) {

	// update center
	XMFLOAT3 newCenter = updateCenter(meshSphere.center, transformation);

	XMVECTOR scale, rotationQ, translation;
	XMMatrixDecompose(&scale, &rotationQ, &translation, transformation);

	XMFLOAT3 scaleF;
	XMStoreFloat3(&scaleF, scale);

	// store in ObjSphere
	objSphere.center = newCenter;
	objSphere.radius = scaleF.x * meshSphere.radius;

	return tre::Maths::createTransformationMatrix(
		XMFLOAT3(objSphere.radius / unitLength, objSphere.radius / unitLength, objSphere.radius / unitLength),
		XMFLOAT3(.0f, .0f, .0f),
		newCenter
	);
}

XMMATRIX BoundingVolume::updateAABB(AABB& meshAABB, AABB& objAABB, XMMATRIX transformation) {

	// update center
	XMFLOAT3 newCenter = updateCenter(meshAABB.center, transformation);

	// obtain original up, right and forward
	XMVECTOR transformRight = tre::Maths::getMatrixNormRight(transformation);
	XMVECTOR transformUp = tre::Maths::getMatrixNormUp(transformation);
	XMVECTOR transformForward = tre::Maths::getMatrixNormForward(transformation);

	// insert code
	XMVECTOR right = transformRight * meshAABB.halfExtent.x;
	XMVECTOR up = transformUp * meshAABB.halfExtent.y;
	XMVECTOR forward = transformForward * meshAABB.halfExtent.z;

	XMVECTOR x = XMVECTOR{ 1.f, .0f, .0f, 0 }, y = XMVECTOR{ .0f, 1.f, .0f, 0 }, z = XMVECTOR{ .0f, .0f, 1.f, 0 };

	XMVECTOR Ii = XMVectorAbs(XMVector3Dot(x, right)) + XMVectorAbs(XMVector3Dot(x, up)) + XMVectorAbs(XMVector3Dot(x, forward));
	XMVECTOR Ij = XMVectorAbs(XMVector3Dot(y, right)) + XMVectorAbs(XMVector3Dot(y, up)) + XMVectorAbs(XMVector3Dot(y, forward));
	XMVECTOR Ik = XMVectorAbs(XMVector3Dot(z, right)) + XMVectorAbs(XMVector3Dot(z, up)) + XMVectorAbs(XMVector3Dot(z, forward));

	XMFLOAT3 newIi, newIj, newIk;
	XMStoreFloat3(&newIi, Ii);
	XMStoreFloat3(&newIj, Ij);
	XMStoreFloat3(&newIk, Ik);

	XMVECTOR scale, rotationQ, translation;
	XMMatrixDecompose(&scale, &rotationQ, &translation, transformation);

	XMFLOAT3 scaleF;
	XMStoreFloat3(&scaleF, scale);

	// store in objAABB
	objAABB.center = newCenter;
	objAABB.halfExtent = XMFLOAT3((scaleF.x * newIi.x / unitLength) / 2, (scaleF.y * newIj.y / unitLength) / 2, (scaleF.z * newIk.z / unitLength) / 2);

	return tre::Maths::createTransformationMatrix(
		XMFLOAT3(scaleF.x * newIi.x / unitLength, scaleF.y * newIj.y / unitLength, scaleF.z * newIk.z / unitLength),
		XMFLOAT3(.0f, .0f, .0f),
		newCenter
	);
}

bool AABB::testAABB(AABB& other) {
	bool intersectX1 = this->center.x + this->halfExtent.x >= other.center.x - other.halfExtent.x;
	bool intersectX2 = other.center.x + other.halfExtent.x >= this->center.x - this->halfExtent.x;
	bool intersectY1 = this->center.y + this->halfExtent.y >= other.center.y - other.halfExtent.y;
	bool intersectY2 = other.center.y + other.halfExtent.y >= this->center.y - this->halfExtent.y;
	bool intersectZ1 = this->center.z + this->halfExtent.z >= other.center.z - other.halfExtent.z;
	bool intersectZ2 = other.center.z + other.halfExtent.z >= this->center.z - this->halfExtent.z;

	return intersectX1 && intersectX2 && intersectY1 && intersectY2 && intersectZ1 && intersectZ2;
}

bool AABB::overlapAABB(AABB& other) {
	float thisXmax = this->center.x + this->halfExtent.x, 
		thisXmin = this->center.x - this->halfExtent.x,
		thisYmax = this->center.y + this->halfExtent.y, 
		thisYmin = this->center.y - this->halfExtent.y, 
		thisZmax = this->center.z + this->halfExtent.z, 
		thisZmin = this->center.z - this->halfExtent.z;
	float otherXmax = other.center.x + other.halfExtent.x,
		otherXmin = other.center.x - other.halfExtent.x,
		otherYmax = other.center.y + other.halfExtent.y,
		otherYmin = other.center.y - other.halfExtent.y,
		otherZmax = other.center.z + other.halfExtent.z,
		otherZmin = other.center.z - other.halfExtent.z;

	bool thisOverlapOtherX = thisXmax >= otherXmax && thisXmin <= otherXmin;
	bool thisOverlapOtherY = thisYmax >= otherYmax && thisYmin <= otherYmin;
	bool thisOverlapOtherZ = thisZmax >= otherZmax && thisZmin <= otherZmin;	
	
	bool otherOverlapThisY = otherYmax >= thisYmax && otherYmin <= thisYmin;
	bool otherOverlapThisZ = otherZmax >= thisZmax && otherZmin <= thisZmin;
	bool otherOverlapThisX = otherXmax >= thisXmax && otherXmin <= thisXmin;

	return thisOverlapOtherX && thisOverlapOtherY && thisOverlapOtherZ
		|| otherOverlapThisX && otherOverlapThisY && otherOverlapThisZ;
}

bool BoundingSphere::testBoundingSphere(BoundingSphere& other) {
	XMVECTOR thisToOther = XMVECTOR{ this->center.x, this->center.y, this->center.z } - XMVECTOR{ other.center.x, other.center.y, other.center.z };
	XMVECTOR dist = XMVector3Length(thisToOther);
	
	XMFLOAT3 distF;
	XMStoreFloat3(&distF, dist);
	
	return distF.x <= this->radius + other.radius;
}

bool BoundingSphere::overlapBoundingSphere(BoundingSphere& other) {
	float larger = this->radius >= other.radius ? this->radius : other.radius;
	float smaller = this->radius >= other.radius ? other.radius : this->radius;

	XMVECTOR thisToOther = XMVECTOR{ this->center.x, this->center.y, this->center.z } - XMVECTOR{ other.center.x, other.center.y, other.center.z };
	XMVECTOR dist = XMVector3Length(thisToOther);

	XMFLOAT3 distF;
	XMStoreFloat3(&distF, dist);

	return larger >= distF.x + smaller;
}

bool BoundingSphere::isForwardPlane(Plane& plane) {
	float dist = plane.getSignedDistanceToPlane(this->center); 
	return dist > this->radius;
}

bool BoundingSphere::isInFrustum(Frustum& frustum) {
	bool b = isForwardPlane(frustum.bottomF);
	bool t = isForwardPlane(frustum.topF);
	bool l = isForwardPlane(frustum.leftF);
	bool r = isForwardPlane(frustum.rightF);
	bool n = isForwardPlane(frustum.nearF);
	return !b && !t && !l && !r && !n;
}

bool BoundingSphere::isOnPlane(Plane& plane) {
	float dist = plane.getSignedDistanceToPlane(this->center);
	return dist > .0f;
}

bool BoundingSphere::isOverlapFrustum(Frustum& frustum) {
	bool b = isOnPlane(frustum.bottomF);
	bool t = isOnPlane(frustum.topF);
	bool l = isOnPlane(frustum.leftF);
	bool r = isOnPlane(frustum.rightF);
	bool n = isOnPlane(frustum.nearF);
	return !b && !t && !l && !r && !n;
}

bool AABB::isOnPlane(Plane& plane) {
	float r = halfExtent.x * fabs(plane.eqn.x) + halfExtent.y * fabs(plane.eqn.y) + halfExtent.z * fabs(plane.eqn.z);

	return -r <= plane.getSignedDistanceToPlane(this->center);
}

bool AABB::isOverlapFrustum(Frustum& frustum) {
	bool b = isOnPlane(frustum.bottomF);
	bool t = isOnPlane(frustum.topF);
	bool l = isOnPlane(frustum.leftF);
	bool r = isOnPlane(frustum.rightF);
	bool n = isOnPlane(frustum.nearF);
	return !b && !t && !l && !r && !n;
}

bool AABB::isForwardPlane(Plane& plane) {
	XMFLOAT3 points[8] = {
		XMFLOAT3(this->center.x + halfExtent.x, this->center.y + halfExtent.y, this->center.z + halfExtent.z),
		XMFLOAT3(this->center.x + halfExtent.x, this->center.y + halfExtent.y, this->center.z - halfExtent.z),
		XMFLOAT3(this->center.x + halfExtent.x, this->center.y - halfExtent.y, this->center.z + halfExtent.z),
		XMFLOAT3(this->center.x + halfExtent.x, this->center.y - halfExtent.y, this->center.z - halfExtent.z),
		XMFLOAT3(this->center.x - halfExtent.x, this->center.y + halfExtent.y, this->center.z + halfExtent.z),
		XMFLOAT3(this->center.x - halfExtent.x, this->center.y + halfExtent.y, this->center.z - halfExtent.z),
		XMFLOAT3(this->center.x - halfExtent.x, this->center.y - halfExtent.y, this->center.z + halfExtent.z),
		XMFLOAT3(this->center.x - halfExtent.x, this->center.y - halfExtent.y, this->center.z - halfExtent.z),
	};
	
	float minDist = plane.getSignedDistanceToPlane(points[0]);
	for (int i = 1; i < 8; i++) {
		minDist = std::min(minDist, plane.getSignedDistanceToPlane(points[i]));
	}

	return minDist > .0f;
}

bool AABB::isInFrustum(Frustum& frustum) {
	bool b = isForwardPlane(frustum.bottomF);
	bool t = isForwardPlane(frustum.topF);
	bool l = isForwardPlane(frustum.leftF);
	bool r = isForwardPlane(frustum.rightF);
	bool n = isForwardPlane(frustum.nearF);
	return !b && !t && !l && !r && !n;
}

}