#pragma once

#include "v3.h"
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
using namespace std;
class FrameBuffer;

class PPC {
public:
	V3 a, b, c, C;
	int w, h;
	float hfov;
	PPC(float hfov, int _w, int _h);
	int Project(V3 p, V3 &pp);
	V3 GetVD();
	float GetFocalLength();
	void ChangeFocalLength(float scf);
	void PositionAndOrient(V3 C1, V3 L1, V3 vpv); //C1 = new camera eye position, L = new look-at point, vpv = vertical plane vector 
	void Pan(float angled);
	void Tilt(float angled);
	void Roll(float angled);
	void Visualize(PPC *ppc3, FrameBuffer *fb3, float vf);
	void TranslateLeftRight(float step);
	void TranslateUpDown(float step);
	void TranslateForwardBackward(float step);
	std::vector<PPC> InterpolateBetweenCameras(PPC *ppc2, int steps, V3 L1, V3 vpv);
	void SavePPC(char * filename);
	PPC *LoadPPC(char * filename);
	V3 UnprojectPixel(float uf, float vf, float currf);
	V3 Unproject(V3 pP);
	void SetInterpolated(PPC *ppc0, PPC *ppc1, int stepi, int stepsN);
	void SetIntrinsicsHW();
	void SetExtrinsicsHW();
};