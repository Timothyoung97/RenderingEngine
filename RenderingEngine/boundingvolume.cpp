#include "boundingvolume.h"

#include "utility.h"

namespace tre {

RitterBS::RitterBS(const std::vector<XMFLOAT3>& uniquePoint) {
	
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

	for (int i = 0; i < uniquePoint.size(); i++) {
		XMFLOAT3 diff;
		diff.x = uniquePoint[i].x - sphereCenter.x;
		diff.y = uniquePoint[i].y - sphereCenter.y;
		diff.z = uniquePoint[i].z - sphereCenter.z;

		float dist = sqrtf(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);
		
		if (dist > radius) radius = dist;
	}
}

}