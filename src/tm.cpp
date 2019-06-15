
#include <iostream>

using namespace std;

#include <fstream>

#include "tm.h"
#include "aabb.h"
#include <math.h>
#include "scene.h"

void TM::RotateAboutArbitraryAxis(V3 O, V3 a, float angled) {

	for (int vi = 0; vi < vertsN; vi++) {
		verts[vi] = verts[vi].RotatePointAboutAxis(a, O, angled);
	}

}


void TM::RenderPoints(PPC *ppc, FrameBuffer *fb) {

	for (int vi = 0; vi < vertsN; vi++) {
		fb->Draw3DPoint(verts[vi], colors[vi], ppc, 7);
	}

}

void TM::RenderWireframe(PPC *ppc, FrameBuffer *fb) {

	for (int tri = 0; tri < trisN; tri++) {
		for (int ei = 0; ei < 3; ei++) {
			int vi0 = tris[3 * tri + ei];
			int vi1 = tris[3 * tri + ((ei + 1) % 3)];
			fb->Draw3DSegment(verts[vi0], colors[vi0],
				verts[vi1], colors[vi1], ppc);
		}
	}

}

void TM::RenderFilled(PPC *ppc, FrameBuffer *fb) {
	for (int tri = 0; tri < trisN; tri++) {
		int vi0 = tris[3*tri];
		int vi1 = tris[3*tri+1];
		int vi2 = tris[3*tri+2];
		V3 v1 = verts[vi0];
		V3 c1 = colors[vi0];
		V3 v2 = verts[vi1];
		V3 c2 = colors[vi1];
		V3 v3 = verts[vi2];
		V3 c3 = colors[vi2];
		V3 pp1, pp2, pp3;
		if (!ppc->Project(v1, pp1))
			continue;
		if (!ppc->Project(v2, pp2))
			continue;
		if (!ppc->Project(v3, pp3))
			continue;
		M33 Q = fb->Q_Matrix(v1, v2, v3, ppc);
		V3 def = fb->DEF(Q);
		fb->Draw3DTriangle(pp1,pp2,pp3,c1,c2,c3,Q,def,ppc);
		
	}
}
void TM::RenderFilledAlt(PPC *ppc, FrameBuffer *fb) {

	V3 *pverts = new V3[vertsN];
	for (int vi = 0; vi < vertsN; vi++) {
		ppc->Project(verts[vi], pverts[vi]);
	}

	for (int tri = 0; tri < trisN; tri++) {
		unsigned int vis[3] = { tris[3 * tri + 0], tris[3 * tri + 1], tris[3 * tri + 2] };
		if (
			pverts[vis[0]][0] == FLT_MAX ||
			pverts[vis[1]][0] == FLT_MAX ||
			pverts[vis[2]][0] == FLT_MAX // implement clipping
			)
			continue;
		AABB aabb(pverts[vis[0]]);
		aabb.AddPoint(pverts[vis[1]]);
		aabb.AddPoint(pverts[vis[2]]);
		if (!aabb.Clip2D(0.0f, (float)fb->w, 0.0f, (float)fb->h))
			continue;
		int lefti = (int)(aabb.c0[0] + 0.5f);
		int righti = (int)(aabb.c1[0] - 0.5f);
		int topi = (int)(aabb.c0[1] + 0.5f);
		int bottomi = (int)(aabb.c1[1] - 0.5f);

		//setup edge equations
		M33 eeqs;
		for (int ei = 0; ei < 3; ei++) {
			V3 v0 = pverts[vis[ei]];
			V3 v1 = pverts[vis[(ei + 1) % 3]];
			V3 v2 = pverts[vis[(ei + 2) % 3]]; v2[2] = 1.0f;
			eeqs[ei][0] = v0[1] - v1[1];
			eeqs[ei][1] = v1[0] - v0[0];
			eeqs[ei][2] = -v0[0] * eeqs[ei][0] - v0[1] * eeqs[ei][1];
			if (eeqs[ei] * v2 < 0.0f)
				eeqs[ei] = eeqs[ei] * -1.0f;
		}

		M33 ssi;
		ssi[0] = pverts[vis[0]];
		ssi[1] = pverts[vis[1]];
		ssi[2] = pverts[vis[2]];
		V3 pvzs = ssi.GetColumn(2);
		ssi.SetColumn(2, V3(1.0f, 1.0f, 1.0f));
		ssi = ssi.Inverted();

		V3 zABC = ssi * pvzs;
		M33 colsm;
		colsm[0] = colors[vis[0]];
		colsm[1] = colors[vis[1]];
		colsm[2] = colors[vis[2]];
		M33 colsABC = ssi * colsm;
		colsABC = colsABC.Transposed();

		for (int v = topi; v <= bottomi; v++) {
			for (int u = lefti; u <= righti; u++) {
				V3 pc(.5f + (float)u, .5f + (float)v, 1.0f);
				V3 ss = eeqs * pc;
				if (ss[0] < 0.0f || ss[1] < 0.0f || ss[2] < 0.0f)
					continue;
				float currz = pc * zABC;
				if (!fb->Visible(u, v, currz))
					continue;
				V3 currCol = colsABC * pc;
				fb->Set(u, v, currCol.GetColor());
			}
		}
	}

}

void TM::RenderFilledWithShadows(PPC *ppc, FrameBuffer *fb, FrameBuffer *sm, PPC *light) {
	V3 *pverts = new V3[vertsN];
	for (int vi = 0; vi < vertsN; vi++) {
		ppc->Project(verts[vi], pverts[vi]);
	}

	for (int tri = 0; tri < trisN; tri++) {
		unsigned int vis[3] = { tris[3 * tri + 0], tris[3 * tri + 1], tris[3 * tri + 2] };
		if (
			pverts[vis[0]][0] == FLT_MAX ||
			pverts[vis[1]][0] == FLT_MAX ||
			pverts[vis[2]][0] == FLT_MAX // implement clipping
			)
			continue;
		AABB aabb(pverts[vis[0]]);
		aabb.AddPoint(pverts[vis[1]]);
		aabb.AddPoint(pverts[vis[2]]);
		if (!aabb.Clip2D(0.0f, (float)fb->w, 0.0f, (float)fb->h))
			continue;
		int lefti = (int)(aabb.c0[0] + 0.5f);
		int righti = (int)(aabb.c1[0] - 0.5f);
		int topi = (int)(aabb.c0[1] + 0.5f);
		int bottomi = (int)(aabb.c1[1] - 0.5f);

		//setup edge equations
		M33 eeqs;
		for (int ei = 0; ei < 3; ei++) {
			V3 v0 = pverts[vis[ei]];
			V3 v1 = pverts[vis[(ei + 1) % 3]];
			V3 v2 = pverts[vis[(ei + 2) % 3]]; v2[2] = 1.0f;
			eeqs[ei][0] = v0[1] - v1[1];
			eeqs[ei][1] = v1[0] - v0[0];
			eeqs[ei][2] = -v0[0] * eeqs[ei][0] - v0[1] * eeqs[ei][1];
			if (eeqs[ei] * v2 < 0.0f)
				eeqs[ei] = eeqs[ei] * -1.0f;
		}

		M33 ssi;
		ssi[0] = pverts[vis[0]];
		ssi[1] = pverts[vis[1]];
		ssi[2] = pverts[vis[2]];
		V3 pvzs = ssi.GetColumn(2);
		ssi.SetColumn(2, V3(1.0f, 1.0f, 1.0f));
		ssi = ssi.Inverted();

		V3 zABC = ssi * pvzs;
		M33 colsm;
		colsm[0] = colors[vis[0]];
		colsm[1] = colors[vis[1]];
		colsm[2] = colors[vis[2]];
		M33 colsABC = ssi * colsm;
		colsABC = colsABC.Transposed();

		for (int v = topi; v <= bottomi; v++) {
			for (int u = lefti; u <= righti; u++) {
				V3 pc(.5f + (float)u, .5f + (float)v, 1.0f);
				V3 ss = eeqs * pc;
				if (ss[0] < 0.0f || ss[1] < 0.0f || ss[2] < 0.0f)
					continue;
				float currz = pc * zABC;
				if (!fb->Visible(u, v, currz))
					continue;
				//Check shadow map
				V3 up(pc[0], pc[1], currz);
				//V3 unprojected = ppc->Unproject(up);
				V3 unprojected = ppc->UnprojectPixel(up[0], up[1], up[2]);
				V3 light_p;
				//V3 currCol;
				light->Project(unprojected, light_p);
				float df = 0.4;
				int uv = (sm->h - 1 - light_p[1])*sm->w + light_p[0];
				if (uv < 0 || uv >= sm->w*sm->h) {
					V3 currCol = colsABC * pc;
					fb->Set(u, v, currCol.GetColor());
				}
				else {
					if (!sm->VisibleSM(light_p[0], light_p[1], light_p[2]+df)) {
						V3 currCol = (colsABC * pc)*0.1;
						fb->Set(u, v, currCol.GetColor());
					}
					else {
						//Normal Color
						V3 currCol = colsABC * pc;
						fb->Set(u, v, currCol.GetColor());
					}
				}
			}
		}
	}

}

void TM::RenderFilledWithReflections(PPC * ppc, FrameBuffer * fb, cubeMap cube){
	if (!scene)
	return;

	V3 *pverts = new V3[vertsN];
	for (int vi = 0; vi < vertsN; vi++) {
		ppc->Project(verts[vi], pverts[vi]);
	}

	for (int tri = 0; tri < trisN; tri++) {
		unsigned int vis[3] = { tris[3 * tri + 0], tris[3 * tri + 1], tris[3 * tri + 2] };
		if (
			pverts[vis[0]][0] == FLT_MAX ||
			pverts[vis[1]][0] == FLT_MAX ||
			pverts[vis[2]][0] == FLT_MAX // implement clipping
			)
			continue;
		AABB aabb(pverts[vis[0]]);
		aabb.AddPoint(pverts[vis[1]]);
		aabb.AddPoint(pverts[vis[2]]);
		if (!aabb.Clip2D(0.0f, (float)fb->w, 0.0f, (float)fb->h))
			continue;
		int lefti = (int)(aabb.c0[0] + 0.5f);
		int righti = (int)(aabb.c1[0] - 0.5f);
		int topi = (int)(aabb.c0[1] + 0.5f);
		int bottomi = (int)(aabb.c1[1] - 0.5f);

		//setup edge equations
		M33 eeqs;
		for (int ei = 0; ei < 3; ei++) {
			V3 v0 = pverts[vis[ei]];
			V3 v1 = pverts[vis[(ei + 1) % 3]];
			V3 v2 = pverts[vis[(ei + 2) % 3]]; v2[2] = 1.0f;
			eeqs[ei][0] = v0[1] - v1[1];
			eeqs[ei][1] = v1[0] - v0[0];
			eeqs[ei][2] = -v0[0] * eeqs[ei][0] - v0[1] * eeqs[ei][1];
			if (eeqs[ei] * v2 < 0.0f)
				eeqs[ei] = eeqs[ei] * -1.0f;
		}

		M33 ssi;
		ssi[0] = pverts[vis[0]];
		ssi[1] = pverts[vis[1]];
		ssi[2] = pverts[vis[2]];
		V3 pvzs = ssi.GetColumn(2);
		ssi.SetColumn(2, V3(1.0f, 1.0f, 1.0f));
		ssi = ssi.Inverted();

		V3 zABC = ssi * pvzs;
		M33 colsm;
		colsm[0] = colors[vis[0]];
		colsm[1] = colors[vis[1]];
		colsm[2] = colors[vis[2]];
		M33 colsABC = ssi * colsm;
		colsABC = colsABC.Transposed();

		M33 normsm;
		normsm[0] = normals[vis[0]];
		normsm[1] = normals[vis[1]];
		normsm[2] = normals[vis[2]];
		M33 normsABC = ssi * normsm;
		normsABC = normsABC.Transposed();


		for (int v = topi; v <= bottomi; v++) {
			for (int u = lefti; u <= righti; u++) {
				V3 pc(.5f + (float)u, .5f + (float)v, 1.0f);
				V3 ss = eeqs * pc;
				if (ss[0] < 0.0f || ss[1] < 0.0f || ss[2] < 0.0f)
					continue;
				float currz = pc * zABC;
				if (!fb->Visible(u, v, currz))
					continue;
				V3 P = ppc->Unproject(V3(pc[0], pc[1], currz));
				V3 eyeRay = ppc->C - P;
				V3 currNorm = normsABC * pc; currNorm = currNorm.UnitVector();
				V3 refRay = currNorm.Reflect(eyeRay);
				refRay = refRay.UnitVector();

				V3 currCol = cube.lookupDirection(refRay);

				fb->Set(u, v, currCol.GetColor());
			}
		}
	}
}

void TM::RenderFilledWithRefraction(PPC * ppc, FrameBuffer * fb, cubeMap cube, float eta) {
	if (!scene)
		return;

	V3 *pverts = new V3[vertsN];
	for (int vi = 0; vi < vertsN; vi++) {
		ppc->Project(verts[vi], pverts[vi]);
	}

	for (int tri = 0; tri < trisN; tri++) {
		unsigned int vis[3] = { tris[3 * tri + 0], tris[3 * tri + 1], tris[3 * tri + 2] };
		if (
			pverts[vis[0]][0] == FLT_MAX ||
			pverts[vis[1]][0] == FLT_MAX ||
			pverts[vis[2]][0] == FLT_MAX // implement clipping
			)
			continue;
		AABB aabb(pverts[vis[0]]);
		aabb.AddPoint(pverts[vis[1]]);
		aabb.AddPoint(pverts[vis[2]]);
		if (!aabb.Clip2D(0.0f, (float)fb->w, 0.0f, (float)fb->h))
			continue;
		int lefti = (int)(aabb.c0[0] + 0.5f);
		int righti = (int)(aabb.c1[0] - 0.5f);
		int topi = (int)(aabb.c0[1] + 0.5f);
		int bottomi = (int)(aabb.c1[1] - 0.5f);

		//setup edge equations
		M33 eeqs;
		for (int ei = 0; ei < 3; ei++) {
			V3 v0 = pverts[vis[ei]];
			V3 v1 = pverts[vis[(ei + 1) % 3]];
			V3 v2 = pverts[vis[(ei + 2) % 3]]; v2[2] = 1.0f;
			eeqs[ei][0] = v0[1] - v1[1];
			eeqs[ei][1] = v1[0] - v0[0];
			eeqs[ei][2] = -v0[0] * eeqs[ei][0] - v0[1] * eeqs[ei][1];
			if (eeqs[ei] * v2 < 0.0f)
				eeqs[ei] = eeqs[ei] * -1.0f;
		}

		M33 ssi;
		ssi[0] = pverts[vis[0]];
		ssi[1] = pverts[vis[1]];
		ssi[2] = pverts[vis[2]];
		V3 pvzs = ssi.GetColumn(2);
		ssi.SetColumn(2, V3(1.0f, 1.0f, 1.0f));
		ssi = ssi.Inverted();

		V3 zABC = ssi * pvzs;
		M33 colsm;
		colsm[0] = colors[vis[0]];
		colsm[1] = colors[vis[1]];
		colsm[2] = colors[vis[2]];
		M33 colsABC = ssi * colsm;
		colsABC = colsABC.Transposed();

		M33 normsm;
		normsm[0] = normals[vis[0]];
		normsm[1] = normals[vis[1]];
		normsm[2] = normals[vis[2]];
		M33 normsABC = ssi * normsm;
		normsABC = normsABC.Transposed();


		for (int v = topi; v <= bottomi; v++) {
			for (int u = lefti; u <= righti; u++) {
				V3 pc(.5f + (float)u, .5f + (float)v, 1.0f);
				V3 ss = eeqs * pc;
				if (ss[0] < 0.0f || ss[1] < 0.0f || ss[2] < 0.0f)
					continue;
				float currz = pc * zABC;
				if (!fb->Visible(u, v, currz))
					continue;
				V3 P = ppc->Unproject(V3(pc[0], pc[1], currz));
				V3 eyeRay = ppc->C - P;
				V3 currNorm = normsABC * pc; currNorm = currNorm.UnitVector();
				float NI = currNorm * eyeRay;
				float k = 1.0f - eta * eta*(1.0f - NI * NI);
				if (k < 0.0f) {
					V3 refRay = V3(0.0, 0.0, 0.0);
					refRay = refRay.UnitVector();
					V3 currCol = cube.lookupDirection(refRay);
					fb->Set(u, v, currCol.GetColor());
				}
				else {
					V3 refRay = (eyeRay*eta) - (currNorm*(eta*NI + sqrtf(k)));
					refRay = refRay.UnitVector();
					V3 currCol = cube.lookupDirection(refRay);
					fb->Set(u, v, currCol.GetColor());
				}
				

				
			}
		}
	}
}


V3 TM::GetTCS(int vert)
{
	return V3(tcs[2*vert],tcs[2*vert+1],0.0f);
}

void TM::ApplyTexture(PPC *ppc, FrameBuffer *fb, FrameBuffer * tmap){
	V3 *pverts = new V3[vertsN];
	for (int vi = 0; vi < vertsN; vi++) {
		ppc->Project(verts[vi], pverts[vi]);
	}

	for (int tri = 0; tri < trisN; tri++) {
		unsigned int vis[3] = { tris[3 * tri + 0], tris[3 * tri + 1], tris[3 * tri + 2] };
		M33 textCoords;
		textCoords[0] = GetTCS(vis[0]);
		textCoords[1] = GetTCS(vis[1]);
		textCoords[2] = GetTCS(vis[2]);
		M33 Q = fb->Q_Matrix(verts[vis[0]], verts[vis[1]], verts[vis[2]], ppc);
		V3 def = fb->DEF(Q);
		if (
			pverts[vis[0]][0] == FLT_MAX ||
			pverts[vis[1]][0] == FLT_MAX ||
			pverts[vis[2]][0] == FLT_MAX // implement clipping
			)
			continue;
		AABB aabb(pverts[vis[0]]);
		aabb.AddPoint(pverts[vis[1]]);
		aabb.AddPoint(pverts[vis[2]]);
		if (!aabb.Clip2D(0.0f, (float)fb->w, 0.0f, (float)fb->h))
			continue;
		int lefti = (int)(aabb.c0[0] + 0.5f);
		int righti = (int)(aabb.c1[0] - 0.5f);
		int topi = (int)(aabb.c0[1] + 0.5f);
		int bottomi = (int)(aabb.c1[1] - 0.5f);

		//setup edge equations
		M33 eeqs;
		for (int ei = 0; ei < 3; ei++) {
			V3 v0 = pverts[vis[ei]];
			V3 v1 = pverts[vis[(ei + 1) % 3]];
			V3 v2 = pverts[vis[(ei + 2) % 3]]; v2[2] = 1.0f;
			eeqs[ei][0] = v0[1] - v1[1];
			eeqs[ei][1] = v1[0] - v0[0];
			eeqs[ei][2] = -v0[0] * eeqs[ei][0] - v0[1] * eeqs[ei][1];
			if (eeqs[ei] * v2 < 0.0f)
				eeqs[ei] = eeqs[ei] * -1.0f;
		}

		M33 ssi;
		ssi[0] = pverts[vis[0]];
		ssi[1] = pverts[vis[1]];
		ssi[2] = pverts[vis[2]];
		V3 pvzs = ssi.GetColumn(2);
		ssi.SetColumn(2, V3(1.0f, 1.0f, 1.0f));
		ssi = ssi.Inverted();

		V3 zABC = ssi * pvzs;

		for (int v = topi; v <= bottomi; v++) {
			for (int u = lefti; u <= righti; u++) {
				V3 pc(.5f + (float)u, .5f + (float)v, 1.0f);
				V3 ss = eeqs * pc;
				if (ss[0] < 0.0f || ss[1] < 0.0f || ss[2] < 0.0f)
					continue;
				float currz = pc * zABC;
				if (!fb->Visible(u, v, currz))
					continue;
				//Interpolate texture coordinates
				float s = fabs((tmap->ModelSpaceRastInterp(V3(textCoords[0][0], textCoords[1][0], textCoords[2][0]), Q, def, u, v)*tmap->w)+0.5f);
				float t = fabs((tmap->ModelSpaceRastInterp(V3(textCoords[0][1], textCoords[1][1], textCoords[2][1]), Q, def, u, v)*tmap->h)+0.5f);
				double fs, ft, cs, ct;
				fs = (s >= tmap->w - 1) ? abs(int(floor(s)) % tmap->w) : abs(int(floor(s)));
				cs = (s >= tmap->w - 1) ? abs(int(ceil(s)) % tmap->w) : abs(int(ceil(s)));
				ft = (t >= tmap->h - 1) ? abs(int(floor(t)) % tmap->h) : abs(int(floor(t)));
				ct = (t >= tmap->h - 1) ? abs(int(ceil(t)) % tmap->h) : abs(int(ceil(t)));
				//Get texture coordinate neighbors
				V3 topLeft = tmap->GetRGB(fs, ft);
				V3 topRight = tmap->GetRGB(cs, ft);
				V3 bottomLeft = tmap->GetRGB(fs, ct);
				V3 bottomRight = tmap->GetRGB(cs, ct);
				float s_int, d_int;
				float ds = modf(s,&s_int);
				float dt = modf(t,&d_int);
				float currR = topLeft[0] * (1 - ds)*(1 - dt) + topRight[0] * ds*(1 - dt) + bottomLeft[0] * (1 - ds)*dt + bottomRight[0] * ds*dt;
				float currG = topLeft[1] * (1 - ds)*(1 - dt) + topRight[1] * ds*(1 - dt) + bottomLeft[1] * (1 - ds)*dt + bottomRight[1] * ds*dt;
				float currB = topLeft[2] * (1 - ds)*(1 - dt) + topRight[2] * ds*(1 - dt) + bottomLeft[2] * (1 - ds)*dt + bottomRight[2] * ds*dt;
				fb->Set(u, v,V3(currR,currG,currB).GetColor());
			}
		}
	}

}

/*
float * TM::ComputeProjCoefficients(PPC * desired, PPC *reference, float w1) {
	float * ret = new float[12];
	M33 coords1;
	coords1.SetColumn(0, desired->a);
	coords1.SetColumn(1, desired->b);
	coords1.SetColumn(2, desired->c);
	M33 coords2;
	coords2.SetColumn(0, reference->a);
	coords2.SetColumn(1, reference->b);
	coords2.SetColumn(2, reference->c);
	V3 diff = desired->C - reference->C;
	V3 q1 = coords2.Inverted()*diff;
	M33 q2 = coords1.Inverted()*coords2;
	float A, B, C, D, E, F, G, H, I, J, K, L;
	A = q1[0] / w1; B = q2[0][0]; C = q2[0][1];
	D = q2[0][2]; E = q1[2] / w1; F = q2[2][0];
	G = q2[2][1]; H = q2[2][2]; I = q1[1] / w1;
	J = q2[1][0]; K = q2[1][1]; L = q2[1][2];
	ret[0] = A; ret[1] = B; ret[2] = C;
	ret[3] = D; ret[4] = D; ret[5] = F;
	ret[6] = G; ret[7] = H; ret[8] = I;
	ret[9] = J; ret[10] = K; ret[11] = L;
	return ret;
}*/

void TM::ProjectiveTexture(PPC *ppc, FrameBuffer *fb, FrameBuffer *sm, FrameBuffer *tmap, PPC *light) {

	V3 *pverts = new V3[vertsN];
	for (int vi = 0; vi < vertsN; vi++) {
		ppc->Project(verts[vi], pverts[vi]);
	}

	for (int tri = 0; tri < trisN; tri++) {
		unsigned int vis[3] = { tris[3 * tri + 0], tris[3 * tri + 1], tris[3 * tri + 2] };
		if (
			pverts[vis[0]][0] == FLT_MAX ||
			pverts[vis[1]][0] == FLT_MAX ||
			pverts[vis[2]][0] == FLT_MAX // implement clipping
			)
			continue;
		AABB aabb(pverts[vis[0]]);
		aabb.AddPoint(pverts[vis[1]]);
		aabb.AddPoint(pverts[vis[2]]);
		if (!aabb.Clip2D(0.0f, (float)fb->w, 0.0f, (float)fb->h))
			continue;
		int lefti = (int)(aabb.c0[0] + 0.5f);
		int righti = (int)(aabb.c1[0] - 0.5f);
		int topi = (int)(aabb.c0[1] + 0.5f);
		int bottomi = (int)(aabb.c1[1] - 0.5f);

		//setup edge equations
		M33 eeqs;
		for (int ei = 0; ei < 3; ei++) {
			V3 v0 = pverts[vis[ei]];
			V3 v1 = pverts[vis[(ei + 1) % 3]];
			V3 v2 = pverts[vis[(ei + 2) % 3]]; v2[2] = 1.0f;
			eeqs[ei][0] = v0[1] - v1[1];
			eeqs[ei][1] = v1[0] - v0[0];
			eeqs[ei][2] = -v0[0] * eeqs[ei][0] - v0[1] * eeqs[ei][1];
			if (eeqs[ei] * v2 < 0.0f)
				eeqs[ei] = eeqs[ei] * -1.0f;
		}

		M33 ssi;
		ssi[0] = pverts[vis[0]];
		ssi[1] = pverts[vis[1]];
		ssi[2] = pverts[vis[2]];
		V3 pvzs = ssi.GetColumn(2);
		ssi.SetColumn(2, V3(1.0f, 1.0f, 1.0f));
		ssi = ssi.Inverted();

		V3 zABC = ssi * pvzs;
		M33 colsm;
		colsm[0] = colors[vis[0]];
		colsm[1] = colors[vis[1]];
		colsm[2] = colors[vis[2]];
		M33 colsABC = ssi * colsm;
		colsABC = colsABC.Transposed();

		for (int v = topi; v <= bottomi; v++) {
			for (int u = lefti; u <= righti; u++) {
				V3 pc(.5f + (float)u, .5f + (float)v, 1.0f);
				V3 ss = eeqs * pc;
				if (ss[0] < 0.0f || ss[1] < 0.0f || ss[2] < 0.0f)
					continue;
				float currz = pc * zABC;
				if (!fb->Visible(u, v, currz))
					continue;
				//Check shadow map
				V3 up(pc[0], pc[1], currz);
				//V3 unprojected = ppc->Unproject(up);
				V3 unprojected = ppc->UnprojectPixel(up[0], up[1], up[2]);
				V3 light_p;
				//V3 currCol;
				light->Project(unprojected, light_p);
				float df = 0.4;
				int uv = (sm->h - 1 - light_p[1])*sm->w + light_p[0];
				if (uv < 0 || uv >= sm->w*sm->h) {
					V3 currCol = colsABC * pc;
					fb->Set(u, v, currCol.GetColor());
				}
				else {
					if (!sm->VisibleSM(light_p[0], light_p[1], light_p[2] + df)) {
						//Normal Color
						V3 currCol = colsABC * pc;
						fb->Set(u, v, currCol.GetColor());
					}
					else {
						//Projected Color
						V3 currCol = tmap->GetRGB(light_p[0], light_p[1]);
						fb->Set(u, v, currCol.GetColor());
					}
				}
			}
		}
	}

}


void TM::SetRectangle(V3 O, float rw, float rh) {

	vertsN = 4;
	trisN = 2;
	Allocate();

	verts[0] = O + V3(-rw / 2.0f, +rh / 2.0f, 0.0f);
	verts[1] = O + V3(-rw / 2.0f, -rh / 2.0f, 0.0f);
	verts[2] = O + V3(+rw / 2.0f, -rh / 2.0f, 0.0f);
	verts[3] = O + V3(+rw / 2.0f, +rh / 2.0f, 0.0f);

	int tri = 0;
	tris[3 * tri + 0] = 0;
	tris[3 * tri + 1] = 1;
	tris[3 * tri + 2] = 2;
	tri++;

	tris[3 * tri + 0] = 2;
	tris[3 * tri + 1] = 3;
	tris[3 * tri + 2] = 0;
	tri++;

	for (int vi = 0; vi < vertsN; vi++) {
		colors[vi] = V3(0.0f, 0.0, 0.0f);
	}

}

void TM::Allocate() {

	verts = new V3[vertsN];
	colors = new V3[vertsN];
	tris = new unsigned int[3 * trisN];

}


void TM::LoadBin(char *fname) {

	ifstream ifs(fname, ios::binary);
	if (ifs.fail()) {
		cerr << "INFO: cannot open file: " << fname << endl;
		return;
	}

	ifs.read((char*)&vertsN, sizeof(int));
	char yn;
	ifs.read(&yn, 1); // always xyz
	if (yn != 'y') {
		cerr << "INTERNAL ERROR: there should always be vertex xyz data" << endl;
		return;
	}
	if (verts)
		delete verts;
	verts = new V3[vertsN];

	ifs.read(&yn, 1); // cols 3 floats
	if (colors)
		delete colors;
	colors = 0;
	if (yn == 'y') {
		colors = new V3[vertsN];
	}

	ifs.read(&yn, 1); // normals 3 floats
	if (normals)
		delete normals;
	normals = 0;
	if (yn == 'y') {
		normals = new V3[vertsN];
	}

	ifs.read(&yn, 1); // texture coordinates 2 floats
	//float *tcs = 0; // don't have texture coordinates for now
	if (tcs)
		delete tcs;
	tcs = 0;
	if (yn == 'y') {
		tcs = new float[vertsN * 2];
	}

	ifs.read((char*)verts, vertsN * 3 * sizeof(float)); // load verts

	if (colors) {
		ifs.read((char*)colors, vertsN * 3 * sizeof(float)); // load cols
	}

	if (normals)
		ifs.read((char*)normals, vertsN * 3 * sizeof(float)); // load normals

	if (tcs)
		ifs.read((char*)tcs, vertsN * 2 * sizeof(float)); // load texture coordinates

	ifs.read((char*)&trisN, sizeof(int));
	if (tris)
		delete tris;
	tris = new unsigned int[trisN * 3];
	ifs.read((char*)tris, trisN * 3 * sizeof(unsigned int)); // read tiangles

	ifs.close();

	cerr << "INFO: loaded " << vertsN << " verts, " << trisN << " tris from " << endl << "      " << fname << endl;
	cerr << "      xyz " << ((colors) ? "rgb " : "") << ((normals) ? "nxnynz " : "") << ((tcs) ? "tcstct " : "") << endl;

	//delete[]tcs;

}

AABB TM::GetAABB() {

	if (!verts)
		return AABB(V3(0.0f, 0.0f, 0.0f));

	AABB aabb(verts[0]);
	for (int i = 1; i < vertsN; i++)
		aabb.AddPoint(verts[i]);

	return aabb;

}

V3 TM::GetCenter() {

	V3 tmc(0.0f, 0.0f, 0.0f);
	for (int vi = 0; vi < vertsN; vi++) {
		tmc = tmc + verts[vi];
	}

	tmc = tmc * (1.0f / (float)vertsN);

	return tmc;
}

void TM::PositionAndSize(V3 tmC, float tmSize) {

	AABB aabb = GetAABB();
	V3 oldC = aabb.GetCenter();
	float oldSize = aabb.GetDiagonal();

	Translate(V3(0.0f, 0.0f, 0.0f) - oldC);
	Scale(tmSize / oldSize);
	Translate(tmC);

}

void TM::Translate(V3 tv) {

	for (int vi = 0; vi < vertsN; vi++)
		verts[vi] = verts[vi] + tv;

}

void TM::Scale(float scf) {

	for (int vi = 0; vi < vertsN; vi++)
		verts[vi] = verts[vi] * scf;

}
void TM::Light(V3 mc, V3 L) {

	for (int vi = 0; vi < vertsN; vi++) {
		float ka = 0.5f;
		float kd = (L - verts[vi]).Normalized() * normals[vi].Normalized();
		kd = (kd < 0.0f) ? 0.0f : kd;
		colors[vi] = mc * (ka + (1.0f - ka)*kd);
	}

}

void TM::RayTrace(PPC *ppc, FrameBuffer *fb) {

	for (int v = 0; v < fb->h; v++) {
		fb->DrawRectangle(0, v, fb->w - 1, v, 0xFFFFFFFF);
		for (int u = 0; u < fb->w; u++) {
			V3 ray = ppc->c + ppc->a*((float)u + .5f) +
				ppc->b*((float)v + 0.5f);
			V3 O = ppc->C;
			for (int tri = 0; tri < trisN; tri++) {
				M33 M;
				M.SetColumn(0, verts[tris[3 * tri + 0]]);
				M.SetColumn(1, verts[tris[3 * tri + 1]]);
				M.SetColumn(2, verts[tris[3 * tri + 2]]);
				M = M.Inverted();
				V3 q2 = M * O;
				V3 q3 = M * ray;
				float w = (1 - q2[0] - q2[1] - q2[2]) / (q3[0] + q3[1] + q3[2]);
				V3 abc = q2 + q3 * w;
				if (abc[0] < 0.0f || abc[1] < 0.0f || abc[2] < 0.0f)
					continue;
				if (!fb->Visible(u, v, 1.0f / w))
					continue;
				V3 currCol =
					colors[tris[3 * tri + 0]] * abc[0] +
					colors[tris[3 * tri + 1]] * abc[1] +
					colors[tris[3 * tri + 2]] * abc[2];
				fb->Set(u, v, currCol.GetColor());
			}
		}
		fb->DrawRectangle(0, v + 1, fb->w - 1, v + 1, 0xFF0000FF);
		fb->redraw();
		Fl::check();
	}

}

void TM::RenderHW(int mode) {
	if (mode == 0) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);

	glVertexPointer(3, GL_FLOAT, 0, (float*)verts);
	glColorPointer(3, GL_FLOAT, 0, (float*)colors);
	glNormalPointer(GL_FLOAT, 0, (float*)normals);


	glDrawElements(GL_TRIANGLES, 3 * trisN, GL_UNSIGNED_INT, tris);

	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

}

void TM::GL_Texture(FrameBuffer * tmap) {
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnable(GL_TEXTURE_2D);

	glTexCoordPointer(2, GL_FLOAT, 0, (float*)tcs);
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	if (tmap->pix) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tmap->w, tmap->h, 0, GL_RGBA,GL_UNSIGNED_BYTE, tmap->pix);
		//return texture;
	}
	else {
		std::cout << "Failed to load texture" << std::endl;
		return;
	}
	glDrawElements(GL_TRIANGLES, 3 * trisN, GL_UNSIGNED_INT, tris);

	glDisable(GL_TEXTURE_2D);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

}

void TM::makeBox(V3 O, float bw, float bh, float bd, V3 color) {
	vertsN = 8;
	trisN = 12;
	Allocate();

	verts[0] = O + V3(-bw / 2.0f, +bh / 2.0f, 0.0f);
	verts[1] = O + V3(-bw / 2.0f, -bh / 2.0f, 0.0f);
	verts[2] = O + V3(+bw / 2.0f, -bh / 2.0f, 0.0f);
	verts[3] = O + V3(+bw / 2.0f, +bh / 2.0f, 0.0f);
	verts[4] = O + V3(-bw / 2.0f, +bh / 2.0f, +bd );
	verts[5] = O + V3(-bw / 2.0f, -bh / 2.0f, +bd );
	verts[6] = O + V3(+bw / 2.0f, -bh / 2.0f, +bd );
	verts[7] = O + V3(+bw / 2.0f, +bh / 2.0f, +bd );

	//Front Face
	int tri = 0;
	tris[3 * tri + 0] = 0;
	tris[3 * tri + 1] = 1;
	tris[3 * tri + 2] = 2;
	tri++;

	tris[3 * tri + 0] = 2;
	tris[3 * tri + 1] = 3;
	tris[3 * tri + 2] = 0;
	tri++;

	//Back Face
	tris[3 * tri + 0] = 4;
	tris[3 * tri + 1] = 5;
	tris[3 * tri + 2] = 6;
	tri++;

	tris[3 * tri + 0] = 6;
	tris[3 * tri + 1] = 7;
	tris[3 * tri + 2] = 4;
	tri++;

	//Top Face
	tris[3 * tri + 0] = 1;
	tris[3 * tri + 1] = 5;
	tris[3 * tri + 2] = 6;
	tri++;

	tris[3 * tri + 0] = 6;
	tris[3 * tri + 1] = 2;
	tris[3 * tri + 2] = 1;
	tri++;

	//Bottom Face
	tris[3 * tri + 0] = 0;
	tris[3 * tri + 1] = 4;
	tris[3 * tri + 2] = 7;
	tri++;

	tris[3 * tri + 0] = 7;
	tris[3 * tri + 1] = 3;
	tris[3 * tri + 2] = 0;
	tri++;

	//Left Face
	tris[3 * tri + 0] = 0;
	tris[3 * tri + 1] = 1;
	tris[3 * tri + 2] = 5;
	tri++;

	tris[3 * tri + 0] = 5;
	tris[3 * tri + 1] = 4;
	tris[3 * tri + 2] = 0;
	tri++;

	//Right Face
	tris[3 * tri + 0] = 3;
	tris[3 * tri + 1] = 2;
	tris[3 * tri + 2] = 6;
	tri++;

	tris[3 * tri + 0] = 6;
	tris[3 * tri + 1] = 7;
	tris[3 * tri + 2] = 3;
	tri++;

	for (int vi = 0; vi < vertsN; vi++) {
		colors[vi] = color;
	}

}

void TM::setGroundPlane(V3 O, float rw, float rd, V3 color) {
	vertsN = 4;
	trisN = 2;
	Allocate();
	//047 730
	verts[0] = O + V3(-rw / 2.0f, +0.0f, 0.0f);
	verts[1] = O + V3(+rw / 2.0f, +0.0f, 0.0f);
	verts[2] = O + V3(-rw / 2.0f, +0.0f, +rd);
	verts[3] = O + V3(+rw / 2.0f, +0.0f, +rd);

	int tri = 0;
	tris[3 * tri + 0] = 0;
	tris[3 * tri + 1] = 2;
	tris[3 * tri + 2] = 3;
	tri++;

	tris[3 * tri + 0] = 3;
	tris[3 * tri + 1] = 1;
	tris[3 * tri + 2] = 0;
	tri++;

	for (int vi = 0; vi < vertsN; vi++) {
		colors[vi] = color;
	}
}