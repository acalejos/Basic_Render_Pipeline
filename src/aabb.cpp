#include "aabb.h"

AABB::AABB(V3 firstPoint) : c0(firstPoint), c1(firstPoint) {};

void AABB::AddPoint(V3 p) {

	for (int i = 0; i < 3; i++) {
		if (p[i] < c0[i])
			c0[i] = p[i];
		if (p[i] > c1[i])
			c1[i] = p[i];
	}

}

float AABB::GetDiagonal() {

	return (c1 - c0).Length();

}

int AABB::Clip2D(float left, float right, float top, float bottom) {

	if (c0[0] > right || c1[0] < left || c0[1] > bottom || c1[1] < top)
		return 0;
	if (c0[0] < left)
		c0[0] = left;
	if (c1[0] > right)
		c1[0] = right;
	if (c0[1] < top)
		c0[1] = top;
	if (c1[1] > bottom)
		c1[1] = bottom;
	return 1;

}