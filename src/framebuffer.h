#pragma once

#include <FL/Fl.H>
#include <FL/Fl_Gl_Window.H>
#include <GL/glut.h>

#include "v3.h"
#include "ppc.h"
#include "M33.h"
#include "mipmap.h"
#include <vector>
#include "cubeMap.h"
class cubeMap;
using namespace std;

class FrameBuffer : public Fl_Gl_Window {
public:
	int ishw, isgpu;
	int renderMode;
	unsigned int *pix;
	float *zb;
	int w, h;
	FrameBuffer(int u0, int v0, int _w, int _h);
	void draw();
	void KeyboardHandle();
	int handle(int guievent);
	void SetBGR(unsigned int bgr);
	void SetGuarded(int u, int v, unsigned int color);
	void Set(int u, int v, unsigned int color);
	unsigned int Get(int u, int v);
	V3 GetRGB(int u, int v);
	std::vector<FrameBuffer> GetMipMap();

	void DrawRectangle(int u0, int v0, int u1, int v1, unsigned int color);
	int ClipToScreen(int &u0, int &v0, int &u1, int &v1);
	void DrawCircle(int u0, int v0, int R, unsigned int color);
	void DrawSegment(V3 p0, V3 c0, V3 p1, V3 c1);
	void DrawTriangle(V3 a, V3 b, V3 c, unsigned int color);
	void Draw3DSegment(V3 p0, V3 c0, V3 p1, V3 c1, PPC *ppc);
	void Draw3DPoint(V3 p, V3 c, PPC *ppc, int psize);
	void Draw3DTriangle(V3 v1, V3 v2, V3 v3, V3 c1, V3 c2, V3 c3, M33 Q, V3 def, PPC* ppc);

	void ClearZB(float z0);
	int Visible(int u, int v, float currz);
	int VisibleSM(int u, int v, float currz);
	bool Safe(int u, int v);

	M33 Q_Matrix(V3 v1, V3 v2, V3 v3, PPC *ppc);
	V3 DEF(M33 Q_matrix);
	float ModelSpaceRastInterp(V3 verts_r, M33 Q_matrix, V3 DEF, int u, int v);

	void Visualize(PPC *ppc0, float currf, PPC *ppc1, FrameBuffer *fb1);
	void Visualize3D(PPC *ppc0, float currf, PPC *ppc1, FrameBuffer *fb1);

	void LoadTiff(char* fname);
	void SaveAsTiff(char* fname);

	void environmentMap(cubeMap cube, PPC *ppc);
};