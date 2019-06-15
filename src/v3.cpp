#include "v3.h"

#include "m33.h"

V3::V3(float x, float y, float z) {

	xyz[0] = x;
	xyz[1] = y;
	xyz[2] = z;

}

float& V3::operator[](int i) {

	return xyz[i];

}

float V3::operator*(V3 v1) {

	V3 &v0 = *this;
	return v0[0] * v1[0] + v0[1] * v1[1] + v0[2] * v1[2];

}

V3 V3::operator^(V3 v1)
{
	V3 &v0 = *this;
	return V3((v0[1]*v1[2])-(v0[2]*v1[1]) ,(v0[2]*v1[0])-(v0[0]*v1[2]),(v0[0]*v1[1])-(v0[1]*v1[0]));
}

ostream& operator<<(ostream& ostr, V3 v) {

	return ostr << v[0] << " " << v[1] << " " << v[2];

}

istream & operator>>(istream & istr, V3 v)
{
	return istr >> v;
}

V3 V3::operator-(V3 v1) {

	V3 &v0 = *this;
	return V3(v0[0] - v1[0], v0[1] - v1[1], v0[2] - v1[2]);

}

V3 V3::operator+(V3 v1) {

	V3 &v0 = *this;
	return V3(v0[0] + v1[0], v0[1] + v1[1], v0[2] + v1[2]);

}

V3 V3::operator*(float scf) {

	V3 &v0 = *this;
	return V3(v0[0]*scf, v0[1]*scf, v0[2]*scf);

}

V3 V3::operator/(float scf)
{
	V3 &v0 = *this;
	return V3(float (v0[0] / scf), float (v0[1] / scf), float (v0[2] / scf));
}

V3 V3::RotatePointAboutAxis(V3 direction, V3 origin, float angle)
{   
	V3 &point = *this;
	//Create new coordinate system
	//pick axis that is more perpedicular to the direction
	V3 new_axis;
	M33 rotate_matrix;
	if (fabsf(direction[0]) > fabsf(direction[1])) {
		new_axis = V3(0.0f, 1.0f, 0.0f);
		rotate_matrix = M33().RotatePrincipal(angle, 1);
	}
	else {
		new_axis = V3(1.0f, 0.0f, 0.0f);
		rotate_matrix = M33().RotatePrincipal(angle, 0);
	}
	V3 b = (new_axis ^ direction).Normalized();
	V3 c = (direction^b).Normalized();
	M33 new_cs;//new coordinate system
	new_cs[0] = direction.Normalized();
	new_cs[1] = b;
	new_cs[2] = c;
	//Transform point(this) to new coordinate system
	V3 p_t = point - origin; //Translate to new origin
	V3 p_prime = new_cs * p_t; //Rotate old axes into new axes
	//p_prime is the old point in the new coordinate system
	//Rotate about direction
	V3 rotated_point = rotate_matrix * p_prime;
	V3 final_point = (new_cs.Inverted()*rotated_point) + origin;
	return final_point;
}


V3 V3::RotateVectorAboutDirection(V3 direction, float angle)
{
	V3 &vector = *this;
	V3 start = V3(0.0f, 0.0f, 0.0f); //consider this the "start" point of the given vector - the origin;
	V3 end = vector; //consider this the "end" point defined by the given vector
	return end.RotatePointAboutAxis(direction,start,angle) - start.RotatePointAboutAxis(direction, start, angle); //rotate both over direction then subtract the two point to get the vector
}

void V3::SetFromColor(unsigned int color) {

	unsigned char *rgb = (unsigned char*)&color;
	V3 &v = *this;
	v[0] = ((float)rgb[0]) / 255.0f;
	v[1] = ((float)rgb[1]) / 255.0f;
	v[2] = ((float)rgb[2]) / 255.0f;

}

unsigned int V3::GetColor() {

	V3 &v = *this;
	unsigned int ret;
	unsigned char *rgb = (unsigned char*) (&ret);
	for (int i = 0; i < 3; i++) {
		if (v[i] < 0.0f)
			rgb[i] = 0;
		else if (v[i] > 1.0f)
			rgb[i] = 255;
		else
			rgb[i] = (unsigned char) (255.0f*v[i]);
	}
	rgb[3] = 255;
	return ret;

}

float V3::Length() {

	V3 &v = *this;
	return sqrtf(v*v);

}

V3 V3::Normalized()
{
	V3 &v = *this;
	float length = v.Length();
	v[0] = float(v[0] / length);
	v[1] = float(v[1] / length);
	v[2] = float(v[2] / length);
	return V3(v[0], v[1], v[2]);
}
V3 V3::Reflect(V3 r) {

	V3 &n = *this;
	V3 rn = n * (r*n);
	V3 rr = rn * 2.0f - r;
	return rr;

}
V3 V3::Clamp() {

	V3 v = *this;
	for (int i = 0; i < 3; i++) {
		if (v[i] < 0.0f)
			v[i] = 0.0f;
		if (v[i] > 1.0f)
			v[i] = 1.0f;
	}
	return v;

}

V3 V3::UnitVector() {

	return (*this) * (1.0f / Length());

}
