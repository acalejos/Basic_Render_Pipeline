#pragma once

#include "v3.h"

class AABB {
public:
	V3 c0, c1;
	AABB(V3 firstPoint);
	void AddPoint(V3 p);
	float GetDiagonal();
	V3 GetCenter() { return (c0 + c1) * 0.5f; };
	int Clip2D(float left, float right, float top, float bottom);

};