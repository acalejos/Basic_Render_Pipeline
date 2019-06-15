#pragma once

#include "gui.h"
#include "framebuffer.h"
#include "ppc.h"
#include "tm.h"
#include "cubeMap.h"
#include <vector>
#include "CGInterface.h"


class Scene {
public:


	GUI *gui;
	FrameBuffer *fb,*fb3,*tmap, *tmap2, *tmap3, *sm,*hwfb, *gpufb;
	PPC *ppc,*ppc3,*lightsource;
	Scene();
	void DBG();
	void Render();
	void Render(PPC *currPPC, FrameBuffer *currfb);
	void RenderHW(int mode);
	void RenderGPU(int mode);
	TM *tms;
	int tmsN;
	V3 L; // point light source or center of area light
	//float * sm;
	void setSM(PPC * lightsource);
	PPC * setLightPPC(V3 light, PPC *ppc);
	float kse;
	float ka;
	float mf; // morph fraction
	CGInterface * cgi;
	ShaderOneInterface *soi;

};

extern Scene *scene;