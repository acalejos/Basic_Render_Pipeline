#pragma once
#include "v3.h"
class mipmap{
public:
	unsigned char *tmap;
	int tmap_h;
	int tmap_w;
	V3 GetTmapRGB(int s, int t);
	mipmap() : tmap(0), tmap_h(0), tmap_w(0) {};
	void readBMPToTmap(char* filename);
};

