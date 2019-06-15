#include "mipmap.h"


V3 mipmap::GetTmapRGB(int s, int t)
{
	return V3(tmap[3 * (s*tmap_w + t)], tmap[3 * (s*tmap_w + t) + 1], tmap[3 * (s*tmap_w + t) + 2]);
}

void mipmap::readBMPToTmap(char* filename)
{
	int i;
	FILE* f = fopen(filename, "rb");
	unsigned char info[54];
	fread(info, sizeof(unsigned char), 54, f); // read the 54-byte header

	// extract image height and width from header
	int width = *(int*)&info[18];
	int height = *(int*)&info[22];
	tmap_w = width;
	tmap_h = height;

	int size = 3 * width * height;
	tmap = new unsigned char[size]; // allocate 3 bytes per pixel
	fread(tmap, sizeof(unsigned char), size, f); // read the rest of the data at once
	fclose(f);

	for (i = 0; i < size; i += 3)
	{
		unsigned char tmp = tmap[i];
		tmap[i] = tmap[i + 2];
		tmap[i + 2] = tmp;
	}

}

