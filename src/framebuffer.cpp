#include "framebuffer.h"
#include "math.h"
#include <iostream>
#include "scene.h"
#include "v3.h"
#include "m33.h"
#include "aabb.h"
#include <iostream>
#include <set>


#include "tiffio.h"

using namespace std;

FrameBuffer::FrameBuffer(int u0, int v0,int _w, int _h) 
	: Fl_Gl_Window(u0, v0, _w, _h, 0) {
	ishw = false;
	isgpu = false;
	renderMode = 1;

	w = _w;
	h = _h;
	pix = new unsigned int[w*h];
	zb = new float[w*h];

}


void FrameBuffer::draw() {

	if (ishw) {
		scene->RenderHW(renderMode);
	}
	else if (isgpu) {
		scene->RenderGPU(renderMode);
	}
	else {
		glDrawPixels(w, h, GL_RGBA, GL_UNSIGNED_BYTE, pix);
	}

}

int FrameBuffer::handle(int event) {

	switch (event)
	{
	case FL_KEYBOARD: {
		KeyboardHandle();
		return 0;
	}
	default:
		break;
	}
	return 0;
}

void FrameBuffer::KeyboardHandle() {

	int key = Fl::event_key();
	switch (key) {
	case ',': {
		cerr << "INFO: pressed ," << endl;
		break;
	}
	default:
		cerr << "INFO: do not understand keypress" << endl;
	}
}


void FrameBuffer::SetGuarded(int u, int v, unsigned int color) {

	if (u < 0 || u > w - 1 || v < 0 || v > h - 1)
		return;

	Set(u, v, color);

}


void FrameBuffer::Set(int u, int v, unsigned int color) {

	pix[(h - 1 - v)*w + u] = color;

}

void FrameBuffer::SetBGR(unsigned int bgr) {

	for (int uv = 0; uv < w*h; uv++)
		pix[uv] = bgr;

}


// load a tiff image to pixel buffer
void FrameBuffer::LoadTiff(char* fname) {
	TIFF* in = TIFFOpen(fname, "r");
	if (in == NULL) {
		cerr << fname << " could not be opened" << endl;
		return;
	}

	int width, height;
	TIFFGetField(in, TIFFTAG_IMAGEWIDTH, &width);
	TIFFGetField(in, TIFFTAG_IMAGELENGTH, &height);
	if (w != width || h != height) {
		w = width;
		h = height;
		delete[] pix;
		pix = new unsigned int[w*h];
		size(w, h);
		glFlush();
		glFlush();
	}

	if (TIFFReadRGBAImage(in, w, h, pix, 0) == 0) {
		cerr << "failed to load " << fname << endl;
	}

	TIFFClose(in);
}



// save as tiff image
void FrameBuffer::SaveAsTiff(char *fname) {

	TIFF* out = TIFFOpen(fname, "w");

	if (out == NULL) {
		cerr << fname << " could not be opened" << endl;
		return;
	}

	TIFFSetField(out, TIFFTAG_IMAGEWIDTH, w);
	TIFFSetField(out, TIFFTAG_IMAGELENGTH, h);
	TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, 4);
	TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, 8);
	TIFFSetField(out, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
	TIFFSetField(out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);

	for (uint32 row = 0; row < (unsigned int)h; row++) {
		TIFFWriteScanline(out, &pix[(h - row - 1) * w], row);
	}

	TIFFClose(out);
}

void FrameBuffer::environmentMap(cubeMap cube, PPC * ppc)
{
	for (int u = 0; u < w - 1; u++) {
		for (int v = 0; v < h - 1; v++) {
			V3 dir = ppc->UnprojectPixel(0.5f + (float)u, 0.5f + (float)v, 1.0);
			dir = dir - ppc->C;
			V3 col = cube.lookupDirection(dir);
			unsigned int color = col.GetColor();
			Set(u, v, color);
		}
	}
}


void FrameBuffer::DrawRectangle(int u0, int v0, int u1, int v1, unsigned int color) {

	if (!ClipToScreen(u0, v0, u1, v1))
		return;

	for (int u = u0; u <= u1; u++) {
		for (int v = v0; v <= v1; v++) {
			Set(u, v, color);
		}
	}

}

void FrameBuffer::DrawCircle(int u0, int v0, int r, unsigned int color) {

	int left = u0 - r;
	int right = u0 + r;
	int top = v0 - r;
	int bottom = v0 + r;
	if (!ClipToScreen(left,top,right,bottom))
		return;

	V3 cv(.5f + (float)u0, .5f + (float)v0, 0.0f);
	for (int u = left; u <= right; u++) {
		for (int v = top; v <= bottom; v++) {
			V3 pv(.5f + (float)u, .5f + (float)v, 0.0f);
			V3 dv = pv - cv;
			if (dv*dv > r*r)
				continue;
			Set(u, v, color);
		}
	}

}

int FrameBuffer::ClipToScreen(int &u0, int &v0, int &u1, int &v1) {


	if (u1 < 0)
		return 0;
	if (u0 > w - 1)
		return 0;
	if (v1 < 0)
		return 0;
	if (v0 > h - 1)
		return 0;

	if (u0 < 0)
		u0 = 0;
	if (v0 < 0)
		v0 = 0;
	if (u1 > w - 1)
		u1 = w - 1;
	if (v1 > h - 1)
		v1 = h - 1;
	return 1;

}

void FrameBuffer::DrawSegment(V3 p0, V3 c0, V3 p1, V3 c1) {

	V3 p1p0 = p1 - p0;
	V3 c1c0 = c1 - c0;
	int pixN;
	if (fabsf(p1p0[0]) > fabsf(p1p0[1])) {
		pixN = (int)(fabsf(p1p0[0]) + 1);
	}
	else {
		pixN = (int)(fabsf(p1p0[1]) + 1);
	}
	for (int stepsi = 0; stepsi < pixN + 1; stepsi++) {
		float fracf = (float)stepsi / (float)(pixN);
		V3 p = p0 + p1p0 * fracf;
		V3 c = c0 + c1c0 * fracf;
		int u = (int)p[0];
		int v = (int)p[1];
		if (u < 0 || u > w - 1 || v < 0 || v > h - 1)
			continue;
		if (!Visible(u, v, p[2]))
			continue;
		Set(u, v, c.GetColor());
	}

}

int FrameBuffer::Visible(int u, int v, float currz) {

	int uv = (h - 1 - v)*w + u;
	if (zb[uv] > currz)
		return 0;
	zb[uv] = currz;
	return 1;
}

int FrameBuffer::VisibleSM(int u, int v, float currz) {

	int uv = (h - 1 - v)*w + u;
	if (zb[uv] > currz)
		return 0;
	else
		return 1;
}

void FrameBuffer::DrawTriangle(V3 v1, V3 v2, V3 v3, unsigned int color)
{
	//Input is three vectors representing the three vertices of the triangle
	float x[3], y[3];
	//Get x coordinates from x field in input vectors
	x[0] = v1[0]; x[1] = v2[0]; x[2] = v3[0];
	//Get y coordinates from y field in input vectors
	y[0] = v1[1]; y[1] = v2[1]; y[2] = v3[1];

	float a[3], b[3], c[3]; //a,b,c for the 3 edge expressions
	//establish the three edge equations
	//edge that goes through vertices 0 and 1
	int t1 = 0; //first vertex
	int t2 = 1; //second vertex
	int t3 = 2; //opposite vertex
	
	for (int i = 0; i < 3; i++) {
		a[i] = y[t2] - y[t1]; b[i] = -x[t2] + x[t1]; c[i] = (-x[t1] * y[t2]) + (y[t1] * x[t2]);
		float sidedness = 0; //temporary variable used to establish correct sidedness
		sidedness = (a[i] * x[t3]) + (b[i] * y[t3]) + c[i];
		if (sidedness < 0) {
			a[i] = -a[i]; b[i] = -b[i]; c[i] = -c[i];
		}
		if (i == 0) {
			t2 = 2;
			t3 = 1;
		}
		else if (i == 1) {
			t1 = 1;
			t3 = 0;
		}
	}
	
	//compute screen axes-aligned bounding box for triangle
	AABB box = AABB(v1);
	box.AddPoint(v2);
	box.AddPoint(v3);

	int left = (int)(box.c0[0] + 0.5f), right = (int)(box.c1[0] - 0.5f);
	int top = (int)(box.c0[1] + 0.5f), bottom = (int)(box.c1[1] - 0.5f);

	int currPixX, currPixY;//current pixel considered
	float currEELS[3], currEE[3];//edge expression values for line starts and within line
	std::set<pair<int, int>> outside; //used to collect points outside of triangle
	for (int i = 0; i < 3; i++) { //loop once for each edge
		for (currPixY = top, currEELS[i] = a[i] * (left + 0.5f) + b[i] * (top + 0.5f) + c[i];
			currPixY <= bottom;
			currPixY++, currEELS[i] += b[i])
			for (currPixX = left, currEE[i] = currEELS[i];
				currPixX <= right; currPixX++, currEE[i] += a[i]) {
			if (outside.find(pair<int,int>(currPixX, currPixY)) != outside.end()) continue; //skip if pair is already outside
			if (currEE[i] < 0){
				outside.insert(make_pair(currPixX, currPixY));//add to set of points outside of triangle
			    continue;}//outside triangle
			}

	}
	//color all point inside of triangle
	for (int u = left; u <= right; u++) {
		for (int v = top; v <= bottom; v++) {
			if (outside.find(pair<int, int>(u, v)) == outside.end()) {
				Set(u, v, color);
			}
		}
	}

}

void FrameBuffer::Draw3DPoint(V3 p, V3 c, PPC *ppc, int psize) {

	V3 pp;
	if (!ppc->Project(p, pp))
		return;

	if (pp[0] < 0.0f || pp[0] >= (float)w || pp[1] < 0.0f || pp[1] >= (float)h)
		return;
	int u = (int)pp[0];
	int v = (int)pp[1];
	DrawRectangle(u - psize / 2, v - psize / 2, u + psize / 2, v + psize / 2, c.GetColor());

}

void FrameBuffer::ClearZB(float z0) {

	for (int i = 0; i < w*h; i++)
		zb[i] = z0;

}

void FrameBuffer::Draw3DSegment(V3 p0, V3 c0, V3 p1, V3 c1, PPC *ppc) {

	V3 pp0, pp1;
	if (!ppc->Project(p0, pp0))
		return;
	if (!ppc->Project(p1, pp1))
		return;

	DrawSegment(pp0, c0, pp1, c1);

}

float FrameBuffer::ModelSpaceRastInterp(V3 verts_r, M33 Q_matrix, V3 DEF, int u, int v)
{

	float A = verts_r * Q_matrix.GetColumn(0);
	float B = verts_r * Q_matrix.GetColumn(1);
	float C_vert = verts_r * Q_matrix.GetColumn(2);

	return (A*u + B * v + C_vert) / (DEF[0] * u + DEF[1] * v + DEF[2]);
}

M33 FrameBuffer::Q_Matrix(V3 v1, V3 v2, V3 v3, PPC * ppc)
{
	M33 Q_1;
	V3 C = ppc->C;
	Q_1.SetColumn(0,v1 - C);
	Q_1.SetColumn(1,v2 - C);
	Q_1.SetColumn(2,v3 - C);
	M33 coords;
	coords.SetColumn(0,ppc->a);
	coords.SetColumn(1,ppc->b);
	coords.SetColumn(2,ppc->c);
	return Q_1.Inverted()*coords;
}

V3 FrameBuffer::DEF(M33 Q_matrix)
{
	float D = Q_matrix[0][0] + Q_matrix[1][0] + Q_matrix[2][0];
	float E = Q_matrix[0][1] + Q_matrix[1][1] + Q_matrix[2][1];
	float F = Q_matrix[0][2] + Q_matrix[1][2] + Q_matrix[2][2];
	return V3(D, E, F);
}


void FrameBuffer::Draw3DTriangle(V3 v1, V3 v2, V3 v3, V3 c1, V3 c2, V3 c3,M33 Q,V3 def, PPC* ppc)
{

	//Input is three vectors representing the three vertices of the triangle
	float x[3], y[3];
	//Get x coordinates from x field in input vectors
	x[0] = v1[0]; x[1] = v2[0]; x[2] = v3[0];
	//Get y coordinates from y field in input vectors
	y[0] = v1[1]; y[1] = v2[1]; y[2] = v3[1];

	float a[3], b[3], c[3]; //a,b,c for the 3 edge expressions
	//establish the three edge equations
	//edge that goes through vertices 0 and 1
	int t1 = 0; //first vertex
	int t2 = 1; //second vertex
	int t3 = 2; //opposite vertex

	for (int i = 0; i < 3; i++) {
		a[i] = y[t2] - y[t1]; b[i] = -x[t2] + x[t1]; c[i] = (-x[t1] * y[t2]) + (y[t1] * x[t2]);
		float sidedness = 0; //temporary variable used to establish correct sidedness
		sidedness = (a[i] * x[t3]) + (b[i] * y[t3]) + c[i];
		if (sidedness < 0) {
			a[i] = -a[i]; b[i] = -b[i]; c[i] = -c[i];
		}
		if (i == 0) {
			t2 = 2;
			t3 = 1;
		}
		else if (i == 1) {
			t1 = 1;
			t3 = 0;
		}
	}

	//compute screen axes-aligned bounding box for triangle
	AABB box = AABB(v1);
	box.AddPoint(v2);
	box.AddPoint(v3);

	int left = (int)(box.c0[0] + 0.5f), right = (int)(box.c1[0] - 0.5f);
	int top = (int)(box.c0[1] + 0.5f), bottom = (int)(box.c1[1] - 0.5f);

	int currPixX, currPixY;//current pixel considered
	float currEELS[3], currEE[3];//edge expression values for line starts and within line
	std::set<pair<int, int>> outside; //used to collect points outside of triangle
	for (int i = 0; i < 3; i++) { //loop once for each edge
		for (currPixY = top, currEELS[i] = a[i] * (left + 0.5f) + b[i] * (top + 0.5f) + c[i];
			currPixY <= bottom;
			currPixY++, currEELS[i] += b[i])
			for (currPixX = left, currEE[i] = currEELS[i];
				currPixX <= right; currPixX++, currEE[i] += a[i]) {
			if (outside.find(pair<int, int>(currPixX, currPixY)) != outside.end()) continue; //skip if pair is already outside
			if (currEE[i] < 0) {
				outside.insert(make_pair(currPixX, currPixY));//add to set of points outside of triangle
				continue;
			}//outside triangle
		}

	}
	//Get rasterization parameters
	float u_0 = v1[0];
	float v_0 = v1[1];
	float u_1 = v2[0];
	float v_1 = v2[1];
	float u_2 = v3[0];
	float v_2 = v3[1];
	V3 red_params = V3(c1[0], c2[0], c3[0]);
	V3 green_params = V3(c1[1], c2[1], c3[1]);
	V3 blue_params = V3(c1[2], c2[2], c3[2]);
	V3 z_params = V3(v1[2], v2[2], v3[2]);
	//M33 trans;
	//trans[0] = V3(u_0, v_0, 1.0f);
	//trans[1] = V3(u_1, v_1, 1.0f);
	//trans[2] = V3(u_2, v_2, 1.0f);
	//V3 r_fac = trans.Inverted()*red_params;
	//V3 g_fac = trans.Inverted()*green_params;
	//V3 b_fac = trans.Inverted()*blue_params;
	//V3 z_fac = trans.Inverted()*z_params;
	//color all point inside of triangle
	
	for (int u = left; u <= right; u++) {
		for (int v = top; v <= bottom; v++) {
			if (outside.find(pair<int, int>(u, v)) == outside.end()) {
				//float r_1 = r_fac[0] * u + r_fac[1] * v + r_fac[2];
				//float g_1 = g_fac[0] * u + g_fac[1] * v + g_fac[2];
				//float b_1 = b_fac[0] * u + b_fac[1] * v + b_fac[2];
				//float z0 = z_fac[0] * u + z_fac[1] * v + z_fac[2];
				//float r, g, b;
				//unsigned int curr_color = V3(r,g,b).GetColor();
				float r_0 = ModelSpaceRastInterp(red_params, Q, def, u, v);
				float g_0 = ModelSpaceRastInterp(green_params, Q, def, u, v);
				float b_0 = ModelSpaceRastInterp(blue_params, Q, def, u, v);
				float z1 = ModelSpaceRastInterp(z_params, Q, def, u, v);
				unsigned int curr_color = V3(r_0, g_0, b_0).GetColor();
				if (!Safe(u, v))
					continue;
				if (!Visible(u, v, z1))
					continue;
				Set(u, v, curr_color);
			}
		}
	}

}

bool FrameBuffer::Safe(int  u, int v) {
	if (u < 0 || u > w - 1 || v < 0 || v > h - 1)
		return false;
	else return true;
}



unsigned int FrameBuffer::Get(int u, int v) {


	return pix[(h - 1 - v)*w + u];

}

V3 FrameBuffer::GetRGB(int u, int v) {
	V3 ret;
	ret.SetFromColor(Get(u, v));
	return ret;
}

std::vector<FrameBuffer> FrameBuffer::GetMipMap()
{
	return std::vector<FrameBuffer>();
}

void FrameBuffer::Visualize3D(PPC *ppc0, float currf, PPC *ppc1, FrameBuffer *fb1) {

	for (int v = 0; v < h; v++) {
		for (int u = 0; u < w; u++) {
			if (zb[(h - 1 - v)*w + u] == 0.0f)
				continue;
			V3 pP(.5f + (float)u, .5f + (float)v, zb[(h - 1 - v)*w + u]);
			V3 pixP = ppc0->Unproject(pP);
			V3 cv; cv.SetFromColor(Get(u, v));
			fb1->Draw3DPoint(pixP, cv, ppc1, 1);
		}
	}

}

void FrameBuffer::Visualize(PPC *ppc0, float currf, PPC *ppc1, FrameBuffer *fb1) {

	for (int v = 0; v < h; v++) {
		for (int u = 0; u < w; u++) {
			if (zb[(h - 1 - v)*w + u] == 0.0f)
				continue;
			V3 pixP = ppc0->UnprojectPixel(.5f + (float)u, .5f + (float)v, currf);
			V3 cv; cv.SetFromColor(Get(u, v));
			fb1->Draw3DPoint(pixP, cv, ppc1, 1);
		}
	}

}