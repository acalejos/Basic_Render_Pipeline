#include "ppc.h"
#include "m33.h"
#include "framebuffer.h"
#include "scene.h"


PPC::PPC(float hfov, int _w, int _h): a(1.0f, 0.0f, 0.0f), b(0.0f, -1.0f, 0.0f),
	C(0.0f, 0.0f, 0.0f), w(_w), h(_h) {

	this->hfov = hfov;
	float hfovr = hfov * 3.14159f / 180.0f;
	float f = -(float)w / (2.0f *tanf(hfovr / 2.0f));
	c = V3(-(float)w / 2.0f, (float)h / 2.0f, f);

}


int PPC::Project(V3 p, V3& pp) {

	M33 M; 
	M.SetColumn(0, a);
	M.SetColumn(1, b);
	M.SetColumn(2, c);
	V3 q = M.Inverted()*(p - C);
	if (q[2] < 0.0f)
		return 0;
	pp[2] = 1.0f/q[2];
	pp[0] = q[0] / q[2];
	pp[1] = q[1] / q[2];
	return 1;

}

V3 PPC::GetVD() {

	return (a ^ b).Normalized();

}

float PPC::GetFocalLength() {

	return GetVD()*c;

}

void PPC::ChangeFocalLength(float scf) {

	c = c + GetVD()*(GetFocalLength()*(scf - 1.0f));

}

void PPC::PositionAndOrient(V3 C1, V3 L1, V3 vpv) {

	V3 vd1 = (L1 - C1).Normalized();
	V3 a1 = (vd1 ^ vpv).Normalized() * a.Length();
	V3 b1 = (vd1 ^ a1).Normalized() * b.Length();
	V3 c1 = vd1 * GetFocalLength() - b1 * ((float)h / 2.0f) - a1 * ((float)w / 2.0f);

	C = C1;
	a = a1;
	b = b1;
	c = c1;

}


void PPC::Pan(float angled) {
	V3 dv = (b * -1.0f).Normalized();
	a = a.RotateVectorAboutDirection(dv, angled);
	c = c.RotateVectorAboutDirection(dv, angled);

}

void PPC::Tilt(float angled) {
	V3 dv = a.Normalized();
	b = b.RotateVectorAboutDirection(dv, angled);
	c = c.RotateVectorAboutDirection(dv, angled);
}

void PPC::Roll(float angled) {
	V3 dv = (a^b).Normalized();
	a = a.RotateVectorAboutDirection(dv, angled);
	b = b.RotateVectorAboutDirection(dv, angled);
	c = c.RotateVectorAboutDirection(dv, angled);

}



void PPC::Visualize(PPC *ppc3, FrameBuffer *fb3, float vf) {

	V3 colv(0.0f, 0.0f, 0.0f);
	fb3->Draw3DPoint(C, colv, ppc3, 7);

	float scf = vf / GetFocalLength();
	fb3->Draw3DSegment(C, colv, C + c * scf, colv, ppc3);
	fb3->Draw3DSegment(C + c * scf, colv, C + (c + a * (float)w)*scf, colv, ppc3);
	fb3->Draw3DSegment(C + (c + a * (float)w)*scf, colv,
		C + (c + a * (float)w + b * (float)h)*scf, colv, ppc3);
	fb3->Draw3DSegment(
		C + (c + a * (float)w + b * (float)h)*scf, colv,
		C + (c + b * (float)h)*scf, colv,
		ppc3);
	fb3->Draw3DSegment(
		C + (c + b * (float)h)*scf, colv,
		C + c * scf, colv,
		ppc3);

}

void PPC::TranslateLeftRight(float step)
{
	C = C + a.Normalized()*step;
}

void PPC::TranslateUpDown(float step)
{
	C = C - b.Normalized()*step;
}

void PPC::TranslateForwardBackward(float step)
{
	C = C + (a^b).Normalized()*step;
}

std::vector<PPC> PPC::InterpolateBetweenCameras(PPC *ppc2, int steps, V3 L1, V3 vpv)
{
	vector<PPC> intCams;
	for (int stepi = 0; stepi < steps; stepi++) {
		PPC* ret = new PPC(ppc2->hfov, ppc2->w, ppc2->h);
		V3 C_i = C + (ppc2->C - C)*(float)(stepi)/(float)(steps - 1);
		V3 vd_i = GetVD() + (ppc2->GetVD() - GetVD())*(float)(stepi) / (float)(steps - 1);
		V3 a_i = a + (ppc2->a - a)*(float)(stepi) / (float)(steps - 1);
		//V3 b_i = b + (ppc2->b - b)*(float)(stepi) / (float)(steps - 1);
		//V3 c_i = a + (ppc2->c - c)*(float)(stepi) / (float)(steps - 1);

		ret->C = C_i;
		ret->a = a_i;
		ret->PositionAndOrient(C_i, L1, vpv);
		//curr.PositionAndOrient(ppc2->C, L1, vpv);
		intCams.push_back(*ret);
		
	}
	return intCams;
}

V3 PPC::UnprojectPixel(float uf, float vf, float currf) {

	return C + (a*uf + b * vf + c)*currf * (1.0f / GetFocalLength());

}


V3 PPC::Unproject(V3 pP) {

	return C + (a*pP[0] + b * pP[1] + c) * (1.0f / pP[2]);

}

void PPC::SavePPC(char * filename)
{
	PPC &curr = *this;
	ofstream file_obj;
	file_obj.open(filename, ios::app);
	file_obj << C << endl;
	file_obj << a << endl;
	file_obj << b << endl;
	file_obj << c << endl;
	file_obj << h << endl;
	file_obj << w << endl;
	file_obj << hfov << endl;
	file_obj.close();
}

PPC* PPC::LoadPPC(char * filename)
{
	string line;
	ifstream myfile;
	myfile.open(filename);

	if (!myfile.is_open()) {
		perror("Error open");
		exit(EXIT_FAILURE);
	}
	V3 C_0, a_0, b_0, c_0;
//	int h_0, w_0;
	//float hfov_0;
	int count = 0;
	
	while (std::getline(myfile, line)) {
		std::istringstream(line);
		string temp, buff;
		line >> temp;
		float v[3];
		int i = 0;
		switch (count){
		case 0:
			while (getline(line, buff, ' ')) {
				cerr << buff << endl;
				v[i] = strtof(buff.c_str(),0);
				i++;
			}
			C_0 = V3(v[0], v[1], v[2]);
			//cerr << C_0 << endl;
			count++;
		case 1:
			while (getline(line, buff, ' ')) {
				cerr << buff << endl;
				v[i] = strtof(buff.c_str(), 0);
				i++;
			}
			a_0 = V3(v[0], v[1], v[2]);
			//cerr << a_0 << endl;
			count++;
		case 2:
			while (getline(line, buff, ' ')) {
				cerr << buff << endl;
				v[i] = strtof(buff.c_str(), 0);
				i++;
			}
			b_0 = V3(v[0], v[1], v[2]);
			//cerr << b_0 << endl;
			count++;
		case 3:
			while (getline(line, buff, ' ')) {
				cerr << buff << endl;
				v[i] = strtof(buff.c_str(), 0);
				i++;
			}
			c_0 = V3(v[0], v[1], v[2]);
			//cerr << c_0 << endl;
			count++;
		case 4:
			h = atoi(temp.c_str());
			count++;
		case 5:
			w = atoi(temp.c_str());
			count++;
		case 6:
			hfov = strtof(temp.c_str(),0);
			count = 0;
		}
	}
	PPC* ret = new PPC(hfov, w, h);
	ret->C = C_0;
	ret->a = a_0;
	ret->b = b_0;
	ret->c = c_0;
	return ret;
}

void PPC::SetIntrinsicsHW() {

	glViewport(0, 0, w, h);

	float zNear = 1.0f;
	float zFar = 1000.0f;
	float scf = zNear / GetFocalLength();
	float left = -a.Length()*(float)w / 2.0f*scf;
	float right = a.Length()*(float)w / 2.0f*scf;
	float top = b.Length()*(float)h / 2.0f*scf;
	float bottom = -b.Length()*(float)h / 2.0f*scf;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(left, right, bottom, top, zNear, zFar);

	glMatrixMode(GL_MODELVIEW);

}
void PPC::SetExtrinsicsHW() {

	V3 L = C + GetVD()*100.0f;
	glLoadIdentity();
	gluLookAt(C[0], C[1], C[2], L[0], L[1], L[2], -b[0], -b[1], -b[2]);

}


void PPC::SetInterpolated(PPC *ppc0, PPC *ppc1, int stepi, int stepsN) {


	(*this) = *ppc0;

	float f = (float)stepi / (float)(stepsN - 1);
	V3 Ci = ppc0->C + (ppc1->C - ppc0->C) * f;

	V3 L0 = ppc0->C + ppc0->GetVD()*100.0f;
	V3 L1 = ppc1->C + ppc1->GetVD()*100.0f;
	V3 Li = L0 + (L1 - L0) * f;

	V3 vpv0 = ppc0->b*-1.0f;
	V3 vpv1 = ppc1->b*-1.0f;
	V3 vpvi = vpv0 + (vpv1 - vpv0)*f;

	PositionAndOrient(Ci, Li, vpvi);

}

