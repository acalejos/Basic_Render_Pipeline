#pragma once
#include "ppc.h"
#include "framebuffer.h"
#include <vector>

using namespace std;
class cubeMap{
public:
	cubeMap(vector<string> &filenames, PPC *initPPC);
	int prevFace;
	FrameBuffer *FBsides[6];
	PPC *PPCsides[6];
	FrameBuffer *fside1, *fside2, *fside3, *fside4, *fside5, *fside6;
	PPC *pside1, *pside2, *pside3, *pside4, *pside5, *pside6;
	V3 lookupDirection(V3 direction);
};

