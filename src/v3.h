#pragma once

#include <iostream>

using namespace std;


class V3 {
public:
	float xyz[3];
	float& operator[](int i);
	float operator*(V3 v1);
	V3 operator^(V3 v1);
	V3 operator-(V3 v1);
	V3 operator+(V3 v1);
	V3 operator*(float scf);
	V3 operator/(float scf);
	friend ostream& operator<<(ostream& ostr, V3 v);
	friend istream& operator>>(istream& istr, V3 v);
	V3 RotatePointAboutAxis(V3 direction, V3 origin, float angle);
	V3 RotateVectorAboutDirection(V3 direction, float angle);
	void SetFromColor(unsigned int color);
	unsigned int GetColor();
	float Length();
	V3 Normalized();
	V3() {};
	V3(float x, float y, float z);
	V3 Reflect(V3 r);
	V3 Clamp();
	V3 UnitVector();
};