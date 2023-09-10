#include "boundingvolume.h"

#include "utility.h"

namespace tre {

/*
	Reference: Real-Time Collision Detection
*/

// Compute indices to the two most separated points of the (up to) six points
// defining the AABB encompassing the point set. Return these as min and max.
void mostSeparatedPointsOnAABB(int& min, int& max, const std::vector<XMFLOAT3>& uniquePoint) {

	// First find most extreme points along principal axes
	int minX = 0, maxX = 0, minY = 0, maxY = 0, minZ = 0, maxZ = 0;
	for (int i = 1; i < uniquePoint.size(); i++) {
		if (uniquePoint[i].x < uniquePoint[minX].x) minX = i;
		if (uniquePoint[i].x > uniquePoint[maxX].x) maxX = i;
		if (uniquePoint[i].y < uniquePoint[minY].y) minY = i;
		if (uniquePoint[i].y > uniquePoint[maxY].y) maxY = i;
		if (uniquePoint[i].z < uniquePoint[minZ].z) minZ = i;
		if (uniquePoint[i].z > uniquePoint[maxZ].z) maxZ = i;
	}
	
	// Compute the squared distances for the three pairs of points
	float dist2X = tre::Utility::XMFLOAT3DotProduct(tre::Utility::XMFLOAT3Minus(uniquePoint[maxX], uniquePoint[minX]), tre::Utility::XMFLOAT3Minus(uniquePoint[maxX], uniquePoint[minX]));
	float dist2Y = tre::Utility::XMFLOAT3DotProduct(tre::Utility::XMFLOAT3Minus(uniquePoint[maxY], uniquePoint[minY]), tre::Utility::XMFLOAT3Minus(uniquePoint[maxY], uniquePoint[minY]));
	float dist2Z = tre::Utility::XMFLOAT3DotProduct(tre::Utility::XMFLOAT3Minus(uniquePoint[maxZ], uniquePoint[minZ]), tre::Utility::XMFLOAT3Minus(uniquePoint[maxZ], uniquePoint[minZ]));

	// Pick the pair (min,max) of points most distant
	min = minX;
	max = maxX;

	if (dist2Y > dist2X && dist2Y > dist2Z) {
		max = maxY;
		min = minY;
	}
	if (dist2Z > dist2X && dist2Z > dist2Y) {
		max = maxZ;
		min = minZ;
	}
};

void sphereFromDistantPoints(XMFLOAT3& sphereCenter, float& radius, const std::vector<XMFLOAT3>& uniquePoint) {

	// Find the most separated point pair defining the encompassing AABB
	int min, max;
	mostSeparatedPointsOnAABB(min, max, uniquePoint);

	// Set up sphere to just encompass these two points
	sphereCenter = tre::Utility::XMFLOAT3ScalarMultiply(tre::Utility::XMFLOAT3Addition(uniquePoint[min], uniquePoint[max]), .5f);
	radius = tre::Utility::XMFLOAT3DotProduct(tre::Utility::XMFLOAT3Minus(uniquePoint[max], sphereCenter), tre::Utility::XMFLOAT3Minus(uniquePoint[max], sphereCenter));
	radius = sqrtf(radius);
};

// Given Sphere s and Point p, update s (if needed) to just encompass p
void sphereOfSphereAndPt(XMFLOAT3& sphereCenter, float& radius, const XMFLOAT3& point) {
	// Compute squared distance between point and sphere center
	XMFLOAT3 d = tre::Utility::XMFLOAT3Minus(point, sphereCenter);

	float dist2 = tre::Utility::XMFLOAT3DotProduct(d, d);

	if (dist2 > radius * radius) {
		float dist = sqrtf(dist2);
		float newRadius = (radius + dist) * 0.5f;
		float k = (newRadius - radius) / dist;
		radius = newRadius;
		sphereCenter = tre::Utility::XMFLOAT3Addition(sphereCenter, tre::Utility::XMFLOAT3ScalarMultiply(d, k));
	}
};

RitterBS::RitterBS(const std::vector<XMFLOAT3>& uniquePoint) {

	// Get sphere encompassing two approximately most distant points
	sphereFromDistantPoints(sphereCenter, radius, uniquePoint);

	for (int i = 0; i < uniquePoint.size(); i++) {
		sphereOfSphereAndPt(sphereCenter, radius, uniquePoint[i]);
	}
};

RitterBS::RitterBS(const std::vector<XMFLOAT3>& uniquePoint, bool isNew) {
	
	double dx, dy, dz;
	double rad2, xSpan, ySpan, zSpan, maxSpan;
	double oldToP, oldToP2, oldToNew;

	XMFLOAT3 xMax, xMin, yMax, yMin, zMax, zMin, dia1, dia2;

	xMin.x = yMin.y = zMin.z = INT_MAX;
	xMax.x = yMax.y = zMax.z = INT_MIN;

	// FIRST PASS: Find 6 minima/maxima points
	for (int i = 0; i < uniquePoint.size(); i++) {
		if (uniquePoint[i].x < xMin.x) xMin = uniquePoint[i];
		if (uniquePoint[i].x > xMax.x) xMax = uniquePoint[i];
		if (uniquePoint[i].y < yMin.y) yMin = uniquePoint[i];
		if (uniquePoint[i].y > yMax.y) yMax = uniquePoint[i];
		if (uniquePoint[i].z < zMax.z) zMin = uniquePoint[i];
		if (uniquePoint[i].z > zMax.z) zMax = uniquePoint[i];
	}

	// Set xspan = distance between 2 points xMin and xMax (squared)
	dx = xMax.x - xMin.x;
	dy = xMax.y - xMin.y;
	dz = xMax.z - xMin.z;
	xSpan = dx * dx + dy * dy + dz * dz;

	// Same for y & z spans
	dx = yMax.x - yMin.x;
	dy = yMax.y - yMin.y;
	dz = yMax.z - yMin.z;
	ySpan = dx * dx + dy * dy + dz * dz;	
	
	dx = zMax.x - zMin.x;
	dy = zMax.y - zMin.y;
	dz = zMax.z - zMin.z;
	zSpan = dx * dx + dy * dy + dz * dz;

	// set points dia1 & dia2 to the maximally separated pair
	dia1 = xMin;
	dia2 = xMax;
	maxSpan = xSpan;

	if (ySpan > maxSpan) {
		maxSpan = ySpan;
		dia1 = yMin; 
		dia2 = yMax;
	}

	if (zSpan > maxSpan) {
		dia1 = zMin; 
		dia2 = zMax;
	}

	// dia1, dia2 is a diameter of initial sphere
	sphereCenter.x = (dia1.x + dia2.x) / 2;
	sphereCenter.y = (dia1.y + dia2.y) / 2;
	sphereCenter.z = (dia1.z + dia2.z) / 2;

	// cal initial radius
	dx = dia2.x - sphereCenter.x;
	dy = dia2.y - sphereCenter.y;
	dz = dia2.z - sphereCenter.z;
	rad2 = dx * dx + dy * dy + dz * dz;
	radius = sqrtf(rad2);

	// 2nd pass: increment curr sphere
	for (int i = 0; i < uniquePoint.size(); i++) {
		dx = uniquePoint[i].x - sphereCenter.x;
		dy = uniquePoint[i].y - sphereCenter.y;
		dz = uniquePoint[i].z - sphereCenter.z;

		oldToP2 = dx * dx + dy * dy + dz * dz;

		if (oldToP2 > rad2) {
			oldToP = sqrtf(oldToP2);
			radius = (radius + oldToP) / 2;
			rad2 = radius * radius;
			oldToNew = oldToP - radius;

			sphereCenter.x = (radius * sphereCenter.x + oldToNew * uniquePoint[i].x) / oldToP;
			sphereCenter.y = (radius * sphereCenter.y + oldToNew * uniquePoint[i].y) / oldToP;
			sphereCenter.z = (radius * sphereCenter.z + oldToNew * uniquePoint[i].z) / oldToP;
		}
	}
}

NaiveBS::NaiveBS(const std::vector<XMFLOAT3>& uniquePoint) {
	XMVECTOR sphereCenterV = XMLoadFloat3(&sphereCenter);

	for (int i = 0; i < uniquePoint.size(); i++) {
		XMVECTOR pointV = XMLoadFloat3(&uniquePoint[0]);
		XMVECTOR dirV = pointV - sphereCenterV;

		XMFLOAT3 dirVLength;
		XMStoreFloat3(&dirVLength, XMVector3Length(dirV));

		float dist = dirVLength.x;
		
		if (dist > radius) radius = dist;
	}
}

}