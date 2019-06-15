#include "cubeMap.h"


cubeMap::cubeMap(vector<string> &filenames, PPC *initPPC)
{
	prevFace = 0;
	int initH = initPPC->h;
	int initW = initPPC->w;
	//Populate PPCs
	pside1 = new PPC(90.0, initW, initH);
	pside2 = new PPC(90.0, initW, initH);
	pside3 = new PPC(90.0, initW, initH);
	pside4 = new PPC(90.0, initW, initH);
	pside5 = new PPC(90.0, initW, initH);
	pside6 = new PPC(90.0, initW, initH);
	PPCsides[0] = pside1; PPCsides[1] = pside2;
	PPCsides[2] = pside3; PPCsides[3] = pside4;
	PPCsides[4] = pside5; PPCsides[5] = pside6;
	V3 O = V3(0.0, 0.0, 0.0);
	V3 vpv = V3(0.0, 1.0, 0.0);
	/*
	for (int i = 0; i < 6; i++) {
		PPCsides[i]->PositionAndOrient(O, V3(1.0, 0.0, 0.0), vpv);
	}
	PPCsides[1]->Pan(90.0);
	PPCsides[2]->Pan(180.0);
	PPCsides[3]->Pan(-90.0);
	PPCsides[4]->Tilt(90.0);
	PPCsides[5]->Tilt(-90.0);
	*/
	
	PPCsides[0]->PositionAndOrient(O, V3(1.0, 0.0, 0.0), vpv); //front face
	PPCsides[1]->PositionAndOrient(O, V3(0.0, 0.0, 1.0), vpv); //right face
	PPCsides[2]->PositionAndOrient(O, V3(-1.0, 0.0, 0.0), vpv); // back face
	PPCsides[3]->PositionAndOrient(O, V3(0.0, 0.0, -1.0), vpv); //left face
	PPCsides[4]->PositionAndOrient(O, V3(0.0, 1.0, 0.0), V3(-1.0,0.0,0.0)); // top face
	PPCsides[5]->PositionAndOrient(O, V3(0.0, -1.0, 0.0), V3(1.0, 0.0, 0.0)); //bottom face
	
	//Populate FrameBuffers
	fside1 = new FrameBuffer(0, 0, initW, initH);
	fside2 = new FrameBuffer(0, 0, initW, initH);
	fside3 = new FrameBuffer(0, 0, initW, initH);
	fside4 = new FrameBuffer(0, 0, initW, initH);
	fside5 = new FrameBuffer(0, 0, initW, initH);
	fside6 = new FrameBuffer(0, 0, initW, initH);
	FBsides[0] = fside1; FBsides[1] = fside2;
	FBsides[2] = fside3; FBsides[3] = fside4;
	FBsides[4] = fside5; FBsides[5] = fside6;
	for (int i = 0; i < 6; i++) {
		std::string filename = filenames.at(i);
		std::vector<char> writable(filename.begin(), filename.end());
		writable.push_back('\0');
		FBsides[i]->LoadTiff(&writable[0]);
	}
}

V3 cubeMap::lookupDirection(V3 direction)
{
	V3 pps;
	int u, v;
	for (int i = 0; i < 6; i++) {
		if (!PPCsides[(i+prevFace)%6]->Project(direction, pps))
			continue;
		else {
			u = pps[0];
			v = pps[1];
			if (FBsides[(i + prevFace) % 6]->ClipToScreen(u, v, u, v)) {
				prevFace = (i + prevFace) % 6;
				break;
			}
		}
	}
	//}
	FrameBuffer * current = FBsides[prevFace];
	float s = pps[0];
	float t = pps[1];

	int fs, ft, cs, ct;
	fs = (s >= current->w - 1) ? abs(int(floor(s)) % current->w) : abs(int(floor(s)));
	cs = (s >= current->w - 1) ? abs(int(ceil(s)) % current->w) : abs(int(ceil(s)));
	ft = (t >= current->h - 1) ? abs(int(floor(t)) % current->h) : abs(int(floor(t)));
	ct = (t >= current->h - 1) ? abs(int(ceil(t)) % current->h) : abs(int(ceil(t)));


	//Get texture coordinate neighbors
	V3 topLeft = current->GetRGB(fs, ft);
	V3 topRight = current->GetRGB(cs, ft);
	V3 bottomLeft = current->GetRGB(fs, ct);	
	V3 bottomRight = current->GetRGB(cs, ct);
	float s_int, d_int;
	float ds = modf(s, &s_int);
	float dt = modf(t, &d_int);
	float currR = topLeft[0] * (1 - ds)*(1 - dt) + topRight[0] * ds*(1 - dt) + bottomLeft[0] * (1 - ds)*dt + bottomRight[0] * ds*dt;
	float currG = topLeft[1] * (1 - ds)*(1 - dt) + topRight[1] * ds*(1 - dt) + bottomLeft[1] * (1 - ds)*dt + bottomRight[1] * ds*dt;
	float currB = topLeft[2] * (1 - ds)*(1 - dt) + topRight[2] * ds*(1 - dt) + bottomLeft[2] * (1 - ds)*dt + bottomRight[2] * ds*dt;
	return V3(currR, currG, currB);
}



