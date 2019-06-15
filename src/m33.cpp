#include "m33.h"

V3& M33::operator[](int i) {

	return rows[i];

}

ostream& operator<<(ostream& ostr, M33 m) {

	return ostr << m[0] << endl << m[1] << endl << m[2];
}

istream & operator>>(istream& istr, M33 m)
{
	return istr >> m[0]>>m[1]>>m[2];
}

V3 M33::operator*(V3 v) {

	M33 &m = *this;
	return V3(m[0] * v, m[1] * v, m[2] * v);

}

M33 M33::operator*(M33 m1) {

	M33 &m0 = *this;
	M33 ret;
	ret.SetColumn(0, m0*m1.GetColumn(0));
	ret.SetColumn(1, m0*m1.GetColumn(1));
	ret.SetColumn(2, m0*m1.GetColumn(2));
	return ret;

}

V3 M33::GetColumn(int i) {

	M33 &m = *this;
	return V3(m[0][i], m[1][i], m[2][i]);

}

void M33::SetColumn(int i, V3 c) {

	M33 &m = *this;
	m[0][i] = c[0];
	m[1][i] = c[1];
	m[2][i] = c[2];

}

M33 M33::RotatePrincipal(float theta, int axis)
{
	M33 ret;
	float deg = theta * (3.14159625f / 180.0f);
	switch (axis) {
	case 0: //x-axis
		ret[0] = V3(1.0f,0.0f,0.0f);
		ret[1] = V3(0.0f, cosf(deg), -sinf(deg));
		ret[2] = V3(0.0f,sinf(deg),cosf(deg));
		return ret;
	case 1: //y-axis
		ret[0] = V3(cosf(deg),0.0f,sinf(deg));
		ret[1] = V3(0.0f,1.0f,0.0f);
		ret[2] = V3(-sinf(deg),0.0f,cosf(deg));	
		return ret;
	case 2: //z-axis
		ret[0] = V3(cosf(deg),-sinf(deg),0.0f);
		ret[1] = V3(sinf(deg),cosf(deg),0.0f);
		ret[2] = V3(0.0f,0.0f,1.0f);
		return ret;
	}
	return ret;
}

M33 M33::Inverted() {

	M33 ret;
	V3 a = GetColumn(0), b = GetColumn(1), c = GetColumn(2);
	V3 _a = b ^ c; _a = _a / (a * _a);
	V3 _b = c ^ a; _b = _b / (b * _b);
	V3 _c = a ^ b; _c = _c / (c * _c);
	ret[0] = _a;
	ret[1] = _b;
	ret[2] = _c;

	return ret;

}

M33 M33::Transposed()
{
	M33 &m = *this;
	M33 ret;
	V3 c1 = V3(m[0][0],m[0][1],m[0][2]);
	V3 c2 = V3(m[1][0], m[1][1], m[1][2]);
	V3 c3 = V3(m[2][0], m[2][1], m[2][2]);
	ret.SetColumn(0,c1);
	ret.SetColumn(1, c2);
	ret.SetColumn(2, c3);
	return ret;
}

