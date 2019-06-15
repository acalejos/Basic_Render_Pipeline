#pragma once
#include "v3.h"
#include "ppc.h"
#include "framebuffer.h"
#include "aabb.h"
#include <stdlib.h>
#include "cubeMap.h"

class TM {
public:
	V3 *verts, *colors, *normals;
	int vertsN;
	unsigned int *tris;
	float *tcs;
	V3 GetTCS(int vert); //returns V3(s,t,0) <- Z-value not used
	int trisN;
	TM() : verts(0), vertsN(0), colors(0), tris(0), trisN(0),normals(0),tcs(0) {};
	void ApplyTexture(PPC *ppc, FrameBuffer *fb, FrameBuffer * tmap);
	void ProjectiveTexture(PPC *ppc, FrameBuffer *fb, FrameBuffer *sm, FrameBuffer *tmap, PPC *light);
	//float * ComputeProjCoefficients(PPC * desired, PPC *reference,float w1);
	void SetRectangle(V3 O, float rw, float rh);
	void Allocate();
	void RenderPoints(PPC *ppc, FrameBuffer *fb);
	void RenderWireframe(PPC *ppc, FrameBuffer *fb);
	void RenderFilled(PPC * ppc, FrameBuffer * fb);
	void RenderFilledAlt(PPC *ppc, FrameBuffer *fb);
	void RenderFilledWithShadows(PPC * ppc, FrameBuffer * fb, FrameBuffer *sm, PPC * light);
	void RenderFilledWithReflections(PPC* ppc, FrameBuffer *fb, cubeMap cube);
	void RenderFilledWithRefraction(PPC* ppc, FrameBuffer *fb, cubeMap cube, float eta);
	void RotateAboutArbitraryAxis(V3 O, V3 a, float angled);
	void LoadBin(char *fname);
	AABB GetAABB();
	void PositionAndSize(V3 tmC, float tmSize);
	void Translate(V3 tv);
	void Scale(float scf);
	V3 GetCenter();
	void Light(V3 mc, V3 L);
	void RayTrace(PPC *ppc, FrameBuffer *fb);
	void RenderHW(int mode); //0 = wireframe | 1 = filled
	void GL_Texture(FrameBuffer * tmap);
	void makeBox(V3 O, float bw, float bh, float bd, V3 color);//origin,box width/height/depth
	void setGroundPlane(V3 O, float rw, float rd,V3 color);
};

