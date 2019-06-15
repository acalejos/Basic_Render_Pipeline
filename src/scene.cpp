#include "scene.h"
#include "v3.h"
#include "m33.h"
#include "ppc.h"
#include "tm.h"
#include <stdlib.h>
#include <cmath>



Scene *scene;

using namespace std;

#include <fstream>

#include <iostream>

Scene::Scene() {

	L = V3(10.0f, 40.0f, -75.0f);

	tms = 0;
	tmsN = 0;

	gui = new GUI();
	gui->show();

	int u0 = 20;
	int v0 = 20;
	int w = 512;
	int h = 512;

	fb = new FrameBuffer(u0, v0, w, h);
	fb->label("SW Framebuffer");
	fb->show();


	tmap = new FrameBuffer(u0,v0,w,h);
	tmap2 = new FrameBuffer(u0, v0, w, h);
	tmap3 = new FrameBuffer(u0, v0, w, h);

	//sm = new FrameBuffer(u0, v0, w, h);
	//sm->label("Shadow Map");
	//sm->show();

	hwfb = new FrameBuffer(u0 + fb->w + 30, v0, w, h);
	hwfb->label("HW Framebuffer");
	hwfb->ishw = true;
	hwfb->show();

	gpufb = new FrameBuffer(u0, v0, w, h);
	gpufb->label("GPU Framebuffer");
	gpufb->isgpu = true;
	gpufb->show();


	//fb3 = new FrameBuffer(u0 + fb->w + 30, v0, w, h);
	//fb3->label("Third person");
	//fb3->show();

	float hfov = 55.0f;
	ppc = new PPC(hfov, fb->w, fb->h);
	//ppc3 = new PPC(30.0f, fb3->w, fb3->h);

	gui->uiw->position(u0, v0 + fb->h + 60);

	tmsN = 1;
	tms = new TM[tmsN];

	//Setup Box scene
	V3 origin1 = V3(0.0f, 0.0f, -100.0f);
	V3 color1 = V3(.75, 0.0, 0.0);
	V3 color2 = V3(0.0, .75, 0.0);
	V3 color3 = V3(0.0, 0.0, .75);
	V3 groundColor = V3(0.0, .8, .8);
	float bh = 20.0;
	float bw = 20.0;
	float bd = 20.0;
	//tms[0].makeBox(origin1, bh, bw, bd, color2);
	//tms[1].makeBox(origin1 + V3(25.0f, 0.0f, -25.0f), bh, bw, bd, color1);
	//tms[2].makeBox(origin1 + V3(-25.0f, 0.0f, -25.0f), bh, bw, bd, color3);
	//tms[3].setGroundPlane(origin1 + V3(0.0f, -10.0f, -50.0f), 120.0, 120.0, groundColor); //ground plane
	//tms[4].SetRectangle(bottomLeft, lightWidth, lightHeight);


	tms[0].LoadBin("geometry/teapot1K.bin");
	V3 tmC = ppc->C + ppc->GetVD()*100.0f;
	float tmSize = 160.0f;
	tms[0].PositionAndSize(tmC, tmSize);

	//ppc3->C = ppc3->C + V3(330.0f, 150.0f, 300.0f);
	//ppc3->PositionAndOrient(ppc3->C, tms[0].GetCenter(), V3(0.0f, 1.0f, 0.0f));

	tmap->LoadTiff("textures/stone_wall_blue.tiff");
	//tmap->LoadTiff("textures/pete2.tiff")
	//sm->LoadTiff("textures/complex_lighting.tiff"); 

	//lightsource = setLightPPC(L, ppc);
	//lightsource->PositionAndOrient(lightsource->C+V3(0.0,0.0,7.5) , tms[0].GetCenter(), V3(0.0, 1.0, 0.0));
	//setSM(lightsource);
	//ppc->PositionAndOrient(ppc->C + V3(70.0f, 20.0f, -45.0f), tms[0].GetCenter(), V3(0.0, 1.0, 0.0));
	ppc->PositionAndOrient(ppc->C + V3(0.0f, 65.0f, 50.0f), tms[0].GetCenter(), V3(0.0, 0.75, 0.0));
	ka = 0.2f;
	fb->Draw3DPoint(L, V3(0.0, 0.0, 255.0), ppc, 7);
	
	//Render();

}

void Scene::RenderHW(int mode) { //Modes: 0 = WF | 1 = Filed | 2 = Texture Map

	// clear the frame buffers
	glEnable(GL_DEPTH_TEST);

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// set camera instrinsics
	ppc->SetIntrinsicsHW();
	// set camera extrinsics
	ppc->SetExtrinsicsHW();

	// render geometry
	for (int tmi = 0; tmi < tmsN; tmi++) {
		if (mode == 3) {
			tms[tmi].GL_Texture(tmap);
		}
		else {
			if (tmi == 4)
				tms[tmi].RenderHW(0);
			else
				tms[tmi].RenderHW(mode);
		}
	}

}



void Scene::RenderGPU(int mode) {

	// if the first time, call per session initialization
	if (cgi == NULL) {
		cgi = new CGInterface();
		cgi->PerSessionInit();
		soi = new ShaderOneInterface();
		soi->PerSessionInit(cgi);
	}

	// clear the framebuffer
	glEnable(GL_DEPTH_TEST);
	glClearColor(1.0, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// set intrinsics
	ppc->SetIntrinsicsHW();
	// set extrinsics
	ppc->SetExtrinsicsHW();

	// per frame initialization
	cgi->EnableProfiles();
	soi->PerFrameInit();
	soi->BindPrograms();

	// render geometry

	for (int tmi = 0; tmi < tmsN; tmi++) {
		if (mode == 3) {
			tms[tmi].GL_Texture(tmap);
		}
		else {
			tms[tmi].RenderHW(mode);
		}
	}
	


	soi->PerFrameDisable();
	cgi->DisableProfiles();



}


void Scene::Render() {

	if (fb) {
		
		Render(ppc, fb);
	}

	float currf = 40.0f;
	if (fb3) {
		fb3->SetBGR(0xFFFFFFFF);
		fb3->ClearZB(0.0f);
		Render(ppc3, fb3);
		fb3->Draw3DPoint(L, V3(1.0f, 1.0f, 0.3f), ppc3, 7);
		//		ppc->Visualize(ppc3, fb3, currf);
		//		fb->Visualize(ppc, currf, ppc3, fb3);
		//		fb->Visualize3D(ppc, currf, ppc3, fb3);
	}

}

void Scene::Render(PPC *currppc, FrameBuffer *currfb) {

	currfb->SetBGR(0xFFFFFFFF);
	currfb->ClearZB(0.0f);
	fb->Draw3DPoint(L, V3(0.0, 0.0, 0255.0), ppc, 7);
	//fb->Draw3DPoint(tms[0].verts[0], V3(0.0, 0.0, 255.0), ppc, 7); //back top left
	fb->Draw3DPoint(tms[0].verts[1], V3(0.0, 0.0, 255.0), ppc, 7); //back bottom left
	//fb->Draw3DPoint(tms[0].verts[2], V3(0.0, 0.0, 255.0), ppc, 7); //back bottom right
	//fb->Draw3DPoint(tms[0].verts[3], V3(0.0, 0.0, 255.0), ppc, 7); // back top right
	fb->Draw3DPoint(tms[0].verts[4], V3(0.0, 0.0, 255.0), ppc, 7); // front top left
	fb->Draw3DPoint(tms[0].verts[5], V3(0.0, 0.0, 255.0), ppc, 7); //front bottom left
	fb->Draw3DPoint(tms[0].verts[6], V3(0.0, 0.0, 255.0), ppc, 7);  // front bottom right
	//fb->Draw3DPoint(tms[0].verts[7], V3(0.0, 0.0, 255.0), ppc, 7); // front top right
	for (int tmi = 0; tmi < tmsN; tmi++) {
		tms[tmi].RenderWireframe(currppc, currfb);
		//		tms[tmi].RenderWireframe(currppc, currfb);
		//		tms[tmi].RenderPoints(currppc, currfb);
	}
	currfb->redraw();

}

void Scene::setSM(PPC *lightsource)
{
	//PPC * lightSource = setLightPPC(L, ppc);
	for (int tmi = 0; tmi < tmsN; tmi++) {
		//tms[tmi].SetSM(lightSource, fb, sm);
		tms[tmi].RenderFilledAlt(lightsource, sm);
	}
	//fb->redraw();
}

PPC * Scene::setLightPPC(V3 light, PPC *ppc)
{
	PPC *lightSource = new PPC(ppc->hfov, ppc->w, ppc->h);
	lightSource->C = light;
	lightSource->a = ppc->a;
	lightSource->b = ppc->b;
	lightSource->c = ppc->c;
	return lightSource;
}

void Scene::DBG() {
	
	{

		
		for (int i = 0; i < 3600; i++) {
			tms[0].RotateAboutArbitraryAxis(V3(0.0f, 0.0f, -100.0f), V3(0.0f, 1.0f, 0.0f), 0.1f);
			//ppc->C = ppc->C + ppc->a.UnitVector()*0.1f;
			hwfb->renderMode = 1;
			gpufb->renderMode = 3;
			hwfb->redraw();
			gpufb->redraw();
			Fl::check();
		}
		return;

		

		//fb->hide();
		int framesN = 3600;
		for (int i = 0; i < framesN; i++) {
			//char filename[100];
			//char filename2[100];
			for (int j = 0; j < tmsN; j++) {
				if (j < 3)
					tms[j].RotateAboutArbitraryAxis(V3(0.0f, 0.0f, -100.0f), V3(0.0f, 1.0f, 0.0f), 0.1f);
				//L = L + V3(0.01, 0.01, 0);
				V3 tc = ppc->C + ppc->GetVD()*100.0f;
				L = tc + V3(40.0f, 30.0f, 0.0f);
				L = L.RotatePointAboutAxis(V3(0.0f, 1.0f, 0.0f), tc, (float)(-(i/10)));
			}
			Render();
			hwfb->redraw();
			gpufb->redraw();
			Fl::check();
			//snprintf(filename, sizeof(filename), "CS535_HW6_Light_frame_%d.tiff", i);
			//snprintf(filename2, sizeof(filename2), "CS535_HW6_Shadows_frame_%d.tiff", i);
			//fb->SaveAsTiff(filename);
			//gpufb->SaveAsTiff(filename2);
		}
		return;
	}
	
	{
		for (int i = 0; i < 100; i++) {
			ppc->C = ppc->C + ppc->a.UnitVector()*0.1f;
			//Render(ppc, fb);
			//RenderGPU();
			hwfb->renderMode = 3;
			gpufb->renderMode = 3;
			RenderHW(hwfb->renderMode);
			hwfb->redraw();
			gpufb->redraw();
			Fl::check();
		}
		return;

	}
	
	{
		PPC ppc0 = *ppc;
		PPC ppc1 = *ppc;
		ppc1.C = ppc1.C + V3(30.0f, 60.0f, 0.0f);
		ppc1.PositionAndOrient(ppc1.C, tms[0].GetCenter(), V3(0.0f, 1.0f, 0.0f));
		ppc1 = *ppc;
		fb->hide();
		int framesN = 1000;
		for (int i = 0; i < framesN; i++) {
			ka = (float)i / (float)framesN;
			mf = (float)i / (float)framesN;
			ppc->SetInterpolated(&ppc0, &ppc1, i, framesN);
			//			Render(ppc, fb);
			hwfb->redraw();
			gpufb->redraw();
			Fl::check();
		}
		*ppc = ppc0;
		return;
	}

	{
		{
			std::vector<std::string> filenames = { "textures/front_face.tiff","textures/right_face.tiff" ,"textures/back_face.tiff" ,"textures/left_face.tiff" ,"textures/top_face.tiff" ,"textures/bottom_face.tiff" };
			cubeMap cube = cubeMap(filenames, ppc);
			fb->environmentMap(cube, ppc);
			float eta = 1.5f;
			
			for (int i = 0; i < 600; i++) {
				//char filename[100];
				fb->SetBGR(0xFFFFFFFF);
				fb->environmentMap(cube, ppc);
				fb->ClearZB(0.0f);
				V3 tc = ppc->C + ppc->GetVD()*100.0f;
				tms[0].RenderFilledWithReflections(ppc, fb, cube);
				//tms[0].RenderFilledWithRefraction(ppc, fb, cube,eta);
				if (i < 300) {
					ppc->C = ppc->C.RotatePointAboutAxis(V3(0.0f, 1.0f, 0.0f), tc, 0.5f);
					ppc->PositionAndOrient(ppc->C, tms[0].GetCenter(), V3(0.0, 1.0, 0.0));
				}
				else {
					ppc->C = ppc->C.RotatePointAboutAxis(V3(1.0f, 0.0f, 0.0f), tc, -0.5f);
					ppc->PositionAndOrient(ppc->C, tms[0].GetCenter(), V3(0.0, 1.0, 0.0));
				}
				fb->redraw();
				Fl::check();
				//snprintf(filename, sizeof(filename), "CS535_HW5_frame_%d.tiff", i);
				//fb->SaveAsTiff(filename);
			}
			/*
			cube.FBsides[0]->label("Front Face");
			cube.FBsides[0]->show();
			cube.FBsides[1]->label("Right Face");
			cube.FBsides[1]->show();
			cube.FBsides[2]->label("Back Face");
			cube.FBsides[2]->show();
			cube.FBsides[3]->label("Left Face");
			cube.FBsides[3]->show();
			cube.FBsides[4]->label("Top Face");
			cube.FBsides[4]->show();
			cube.FBsides[5]->label("Bottom Face");
			cube.FBsides[5]->show();
			*/
			return;
		}
	}

	{
		for (int i = 0; i < 300; i++) {
			//char filename[100];
			//V3 tc = ppc->C + ppc->GetVD()*100.0f;
			//L = tc + V3(40.0f, 0.0f, 0.0f);
			//L = L.RotatePointAboutAxis(V3(0.0f, 1.0f, 0.0f), tc, (float)(i * 2));
			//lightsource->PositionAndOrient(L, tms[0].GetCenter(), V3(0.0, 1.0, 0.0));
			Render(ppc, fb);
			fb->redraw();
			Fl::check();
			//snprintf(filename, sizeof(filename), "CS535_HW4_frame_%d.tiff", i+299);
			//fb->SaveAsTiff(filename);
			//ppc->Pan(0.1f);
		}
		return;
	}
	/*
	{
		//PPC * lightsource = setLightPPC(L,ppc);
		//setSM();
		ppc->PositionAndOrient(ppc->C - V3(0.0f, 0.0f, 40.0f), tms[0].GetCenter(), V3(0.0f, 1.0f, 1.0f));
		ppc->Pan(90.0);
		tms[0].RenderFilledWithShadows(ppc, fb, sm, lightsource);
		fb->redraw();
		Fl::check();
		return;

	}
	*/
	/*
	{
		tmap->LoadTiff("textures/stone_wall_blue.tiff"); //From the web and tiled
		//tmap->LoadTiff("textures/wood_grain.tiff"); 
		tmap2->LoadTiff("textures/red_placemat.tiff"); //From my camera with a reflection
		tmap3->LoadTiff("textures/complex_lighting.tiff");//Texture with complex lighting
		TM rect;
		rect.SetRectangle(V3(0.0f, 0.0f, -100.0f), 20.0f, 20.0f);
		rect.tcs = new float[2 * 4];
		rect.tcs[0] = 0.0f;
		rect.tcs[1] = 0.0f;
		rect.tcs[2] = 0.0f;
		rect.tcs[3] = 4.5f;
		rect.tcs[4] = 4.5f;
		rect.tcs[5] = 4.5f;
		rect.tcs[6] = 4.5f;
		rect.tcs[7] = 0.0f;
		TM rect2;
		rect2.SetRectangle(V3(-30.0f, 0.0f, -100.0f), 20.0f, 20.0f);
		rect2.tcs = new float[2 * 4];
		rect2.tcs[0] = 0.0f;
		rect2.tcs[1] = 0.0f;
		rect2.tcs[2] = 0.0f;
		rect2.tcs[3] = 1.0f;
		rect2.tcs[4] = 1.0f;
		rect2.tcs[5] = 1.0f;
		rect2.tcs[6] = 1.0f;
		rect2.tcs[7] = 0.0f;
		TM rect3;
		rect3.SetRectangle(V3(30.0f, 0.0f, -100.0f), 20.0f, 20.0f);
		rect3.tcs = new float[2 * 4];
		rect3.tcs[0] = 0.0f;
		rect3.tcs[1] = 0.0f;
		rect3.tcs[2] = 0.5f;
		rect3.tcs[3] = 0.0f;
		rect3.tcs[4] = 0.0f;
		rect3.tcs[5] = 0.5f;
		rect3.tcs[6] = 0.5f;
		rect3.tcs[7] = 0.5f;
		V3 O_a(0.0f, -20.0f, -100.0f), a(0.0f, 1.0f, 0.0f);
		V3 L1 = rect.GetCenter();
		V3 vpv = V3(0.0f, 1.0f, 0.0f);
		PPC* ret = new PPC(ppc->hfov, ppc->w, ppc->h);
		ret->C = V3(-20.0f, -5.0f, -20.0f);
		ppc->PositionAndOrient(V3(100.0f, 300.0f, 150.0f), rect.GetCenter(), V3(0.0f, 1.0f, 0.0f));
		std::vector<PPC> int_cams = ret->InterpolateBetweenCameras(ppc, 250,L1 , vpv);
		int stepsN = 300;
		//char filename[100];
		for (int stepsi = 0; stepsi < stepsN; stepsi++) {
			if (stepsi <= 250) {
				PPC *ppc3 = &int_cams.at(abs(250 - stepsi-1));
				ppc->PositionAndOrient(ppc3->C, L1, vpv);
			}
			if (stepsi > 250) {
				ppc->Pan(.15f);
				ppc->TranslateLeftRight(.15f);
			}
			fb->SetBGR(0xFFFFFFFF);
			fb->ClearZB(0.0f);
			rect.RotateAboutArbitraryAxis(V3(0.0f, 0.0f, -100.0f), V3(0.0f, 1.0f, 0.0f),
				1.0f);
			rect.ApplyTexture(ppc, fb, tmap);
			rect2.RotateAboutArbitraryAxis(V3(0.0f, 0.0f, -100.0f), V3(0.0f, 1.0f, 0.0f),
				1.0f);
			rect2.ApplyTexture(ppc, fb, tmap2);
			rect3.RotateAboutArbitraryAxis(V3(0.0f, 0.0f, -100.0f), V3(0.0f, 1.0f, 0.0f),
				1.0f);
			rect3.ApplyTexture(ppc, fb, tmap3);
			fb->redraw();
			Fl::check();
			//snprintf(filename, sizeof(filename), "CS535_HW3_frame_%d.tiff", stepsi);
			//fb->SaveAsTiff(filename);
		}
		//fb->redraw();
		//Fl::check();
		return;

	}
	
	*/
	{
		tmap->LoadTiff("textures/stone_wall_blue.tiff");
		//tmap->LoadTiff("textures/wood_grain.tiff");
		//tmap->LoadTiff("textures/teapotbump.tiff");
		ppc->PositionAndOrient(ppc->C - V3(0.0f, 0.0f, 40.0f), tms[0].GetCenter(), V3(0.0f, 1.0f, 1.0f));
		Fl::check();
		int stepsN = 360;
		V3 tc = tms[0].GetCenter();
		for (int stepsi = 0; stepsi < stepsN; stepsi++) {
			fb->SetBGR(0xFFFFFFFF);
			fb->ClearZB(0.0f);
			//L = tc + V3(40.0f, 0.0f, 0.0f);
			//L = L.RotatePointAboutAxis(V3(0.0f, 1.0f, 0.0f), tc, (float)(stepsi * 2));
			tms[0].RotateAboutArbitraryAxis(V3(0.0f, 0.0f, -100.0f), V3(0.0f, 1.0f, 0.0f),
				1.0f);
			//tms[0].Light(V3(1.0f, 0.0f, 0.0f), L);
			tms[0].ApplyTexture(ppc, fb,tmap);
			//tms[0].RenderFilled(ppc, fb);
			//ret->PositionAndOrient(ppc3->C, L1, vpv);	
			fb->redraw();
			Fl::check();
		}
		return;
	}
	
	/*
	{

		PPC ppc1(*ppc);

		ppc->C = V3(20.0f, 100.0f, 200.0f);
		ppc->PositionAndOrient(ppc->C, tms[0].GetCenter(), V3(0.0f, 1.0f, 0.0f));

		for (int i = 0; i < 1000; i++) {
			ppc1.Pan(.5f);
			Render();
			ppc1.Visualize(ppc, fb, 20.0f);
			Fl::check();
		}
		return;
	}
	*/
/*
{
	TM tm;
	tm.SetRectangle(V3(0.0f, 0.0f, -100.0f), 45.0f, 30.0f);
	//		tm.RenderPoints(ppc, fb);
	int stepsN = 360;
	for (int stepsi = 0; stepsi < stepsN; stepsi++) {
		fb->SetBGR(0xFFFFFFFF);
		tm.RotateAboutArbitraryAxis(V3(0.0f, 0.0f, -100.0f), V3(0.0f, 1.0f, 0.0f),
			1.0f);
		tm.RenderWireframe(ppc, fb);
		fb->redraw();
		Fl::check();
	}
	return;

}
*/
	
	{
		PPC* ret = new PPC(ppc->hfov, ppc->w, ppc->h);
		ret->PositionAndOrient(V3(90.0f, 40.0f, 15.0f), tms[0].GetCenter(), V3(0.0f, 1.0f, 0.0f));
		ppc->PositionAndOrient(ppc->C-V3(0.0f,0.0f,30.0f), tms[0].GetCenter(), V3(0.0f, 1.0f, 1.0f));
		//ret->C = V3(0.0f, 0.0f, 0.0f);
		//PPC *ppc4(ppc);
		//ppc->PositionAndOrient(V3(90.0f, 40.0f, 15.0f), tms[0].GetCenter(), V3(0.0f, 1.0f, 0.0f));
		//ppc4->C = V3(30.0f, 0.0f, 30.0f);
		V3 p_1(20.0f, -20.0f, -80.0f);
		V3 O_a(0.0f, -20.0f, -100.0f), a(0.0f, 1.0f, 0.0f);
		V3 L1 = tms[0].GetCenter();
		V3 vpv = V3(0.0f, 1.0f, 0.0f);
		//std::vector<PPC> int_cams = ret->InterpolateBetweenCameras(ppc, 150,L1 , vpv);
		//ret->SavePPC("testPPCsave.txt");
		//ppc->SavePPC("testPPCsave.txt");
		//ppc = ppc->LoadPPC("testPPCsave.txt");
		//ppc->SavePPC("testPPCsave2.txt");
		//char filename[100];
		for (int i = 0; i < 500; i++) {
			//if (i >= 150) {
				//PPC *ppc3 = &int_cams.at(abs(150 - i));
				//ret->PositionAndOrient(ppc3->C, L1, vpv);				
			//}
			tms[0].RotateAboutArbitraryAxis(O_a, a, 1.0f);
			//tms[1].RotateAboutArbitraryAxis(O_a, a, 1.0f);
			//tms[2].RotateAboutArbitraryAxis(O_a, a, 1.0f);
			//tms[3].RotateAboutArbitraryAxis(O_a, a, 1.0f);
			//tms[4].RotateAboutArbitraryAxis(O_a, a, 1.0f);
			Render();
			Fl::check();
			//snprintf(filename, sizeof(filename), "CS535_HW2_frame_%d.tiff", i);
			//fb->SaveAsTiff(filename);
		}
		return;


	}
	
	/*
	{

		V3 tmc = tms[0].GetCenter();
		V3 C1 = ppc->C + V3(120.0f, 80.0f, 0.0f);
		ppc->PositionAndOrient(C1, tmc, V3(0.0f, 1.0f, 0.0));
		Render();
		return;

	}
	*/
	/*
	{
		TM tm;
		tm.LoadBin("geometry/teapot1K.bin");
		V3 tmC = ppc->C + ppc->GetVD()*100.0f;
		float tmSize = 50.0f;
		tm.PositionAndSize(tmC, tmSize);
		fb->SetBGR(0xFFFFFFFF);
		fb->ClearZB(0.0f);
		tm.RenderFilled(ppc, fb);
		TM tm2;
		//		tm2.SetRectangle(tmC, 50.0f, 50.0f);
		//		tm2.RenderWireframe(ppc, fb);
		fb->redraw();

		for (int i = 0; i < 100; i++) {
			fb->SetBGR(0xFFFFFFFF);
			fb->ClearZB(0.0f);
			ppc->ChangeFocalLength(1.05f);
			tm.RenderWireframe(ppc, fb);
			fb->redraw();
			Fl::check();
		}

		return;
	}

	{

		V3 p(20.0f, -20.0f, -80.0f);
		V3 Oa(0.0f, -20.0f, -100.0f), a(0.0f, 1.0f, 0.0f);
		fb->Draw3DSegment(Oa, V3(0.0f, 0.0f, 0.0f), Oa + a * 30.0f, V3(0.0f, 1.0f, 0.0f), ppc);
		for (int i = 0; i < 270; i++) {
			p = p.RotatePointAboutAxis(a, Oa, 1.0f);
			fb->Draw3DPoint(p, V3(1.0f, 0.0f, 0.0f), ppc, 7);
			fb->redraw();
			Fl::check();
		}
		return;

	}



	{
		TM tm;
		tm.SetRectangle(V3(0.0f, 0.0f, -100.0f), 45.0f, 30.0f);
		//		tm.RenderPoints(ppc, fb);
		int stepsN = 360;
		for (int stepsi = 0; stepsi < stepsN; stepsi++) {
			fb->SetBGR(0xFFFFFFFF);
			tm.RotateAboutArbitraryAxis(V3(0.0f, 0.0f, -100.0f), V3(0.0f, 1.0f, 0.0f),
				1.0f);
			tm.RenderWireframe(ppc, fb);
			fb->redraw();
			Fl::check();
		}
		return;

	}

	{

		V3 p0(-20.0f, 0.0f, -100.0f), pp;
		V3 p1(-20.0f, 0.0f, -1000.0f);
		int stepsN = 5;
		for (int stepi = 0; stepi < stepsN; stepi++) {
			//			fb->SetBGR(0xFFFFFFFF);
			V3 p = p0 + (p1 - p0) * ((float)stepi / (float)(stepsN - 1));
			fb->Draw3DPoint(p, V3(1.0f, 0.0f, 0.0f), ppc, 7);
			fb->redraw();
			Fl::check();
		}
		return;

	}


	{

		V3 p0(20.1f, 40.0f, 0.0f);
		V3 p1(50.9f, 240.0f, 0.0f);
		V3 c0(1.0f, 0.0f, 0.0f);
		V3 c1(0.0f, 1.0f, 0.0f);
		fb->DrawSegment(p0, c0, p1, c1);
		fb->redraw();
		return;
	}


	{
		V3 v(1.0f, 0.0f, 0.0f);
		for (int i = 0; i < 1000; i++) {
			float scf = (float)i / 999.0f;
			v[1] = scf;
			unsigned int color = v.GetColor();
			V3 v1; v1.SetFromColor(color);
			cerr << v1 - v << "          \r";
			fb->SetBGR(color);
			fb->redraw();
			Fl::check();
		}
		cerr << endl;
		return;
	}

	{
		int u0 = 100;
		int v0 = 50;
		int u1 = 300;
		int v1 = 400;
		int r = 30;
		int stepsN = 100;
		for (int stepi = 0; stepi < stepsN; stepi++) {
			fb->SetBGR(0xFFFFFFFF);
			//			fb->DrawRectangle(u0 + stepi, v0, u1 + stepi, v1, 0xFF0000FF);
			fb->DrawCircle(u0 + stepi, v0, r, 0xFF0000FF);
			fb->redraw();
			Fl::check();
		}
		return;
	}

	{
		M33 m;
		m[0] = V3(1.0f, 0.0f, 0.0f);
		m[1] = V3(0.0f, 1.0f, 0.0f);
		m[2] = V3(0.0f, 0.0f, 1.0f);
		M33 m1;
		m1[0] = V3(-1.0f, 2.0f, 3.0f);
		m1[1] = V3(-2.0f, 2.0f, 3.0f);
		m1[2] = V3(-3.0f, -2.0f, 5.0f);

		cerr << m1 << endl << m * m1 << endl << m1 * m << endl;
		return;

		cerr << m << endl;
		V3 v0(1.0f, -2.0f, 0.5f);
		cerr << m * v0;
		cerr << v0;

		return;

	}

	{
		V3 v0(1.0f, -2.0f, 0.5f);
		V3 v1(-1.0f, -4.0f, 3.0f);
		cerr << v0 << v1;
		cerr << "v0v1 = " << v0 * v1 << endl;
		return;
	}

	V3 v(1.0f, 3.0f, -1.0f);
	cerr << v[1] << endl;
	v[1] = 2.0f;
	cerr << v[1] << endl;
	return;

	int u0 = 100;
	int v0 = 50;
	int u1 = 300;
	int v1 = 400;

	fb->DrawRectangle(u0, v0, u1, v1, 0xFF0000FF);
	fb->redraw();

	fb->SaveAsTiff("mydbg/rr.tif");

	FrameBuffer *fb1 = new FrameBuffer(30 + fb->w, 30, fb->w, fb->h);
	fb1->LoadTiff("mydbg/rr.tif");
	fb1->label("Loaded image");
	fb1->show();

	return;

	for (int u = 0; u < fb->w; u++) {
		fb->Set(u, fb->h / 2, 0xFF000000);
	}
	fb->redraw();
	cerr << "INFO: pressed DBG" << endl;
	*/
}