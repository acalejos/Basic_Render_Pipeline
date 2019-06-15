//#define GEOM_SHADER

#include "CGInterface.h"
#include "v3.h"
#include "scene.h"

#include <iostream>

using namespace std;

CGInterface::CGInterface() {};

void CGInterface::PerSessionInit() {

	glEnable(GL_DEPTH_TEST);

	CGprofile latestVertexProfile = cgGLGetLatestProfile(CG_GL_VERTEX);
	CGprofile latestGeometryProfile = cgGLGetLatestProfile(CG_GL_GEOMETRY);
	CGprofile latestPixelProfile = cgGLGetLatestProfile(CG_GL_FRAGMENT);

#ifdef GEOM_SHADER
	if (latestGeometryProfile == CG_PROFILE_UNKNOWN) {
		cerr << "ERROR: geometry profile is not available" << endl;
		exit(0);
	}
#endif

	cgGLSetOptimalOptions(latestGeometryProfile);
	CGerror Error = cgGetError();
	if (Error) {
		cerr << "CG ERROR: " << cgGetErrorString(Error) << endl;
	}

	cout << "Info: Latest GP Profile Supported: " << cgGetProfileString(latestGeometryProfile) << endl;

	geometryCGprofile = latestGeometryProfile;

	cout << "Info: Latest VP Profile Supported: " << cgGetProfileString(latestVertexProfile) << endl;
	cout << "Info: Latest FP Profile Supported: " << cgGetProfileString(latestPixelProfile) << endl;

	vertexCGprofile = latestVertexProfile;
	pixelCGprofile = latestPixelProfile;
	cgContext = cgCreateContext();


}

bool ShaderOneInterface::PerSessionInit(CGInterface *cgi) {

#ifdef GEOM_SHADER
	geometryProgram = cgCreateProgramFromFile(cgi->cgContext, CG_SOURCE,
		"CG/shaderOne.cg", cgi->geometryCGprofile, "GeometryMain", NULL);
	if (geometryProgram == NULL) {
		CGerror Error = cgGetError();
		cerr << "Shader One Geometry Program COMPILE ERROR: " << cgGetErrorString(Error) << endl;
		cerr << cgGetLastListing(cgi->cgContext) << endl << endl;
		return false;
	}
#endif

	vertexProgram = cgCreateProgramFromFile(cgi->cgContext, CG_SOURCE,
		"CG/shaderOne.cg", cgi->vertexCGprofile, "VertexMain", NULL);
	if (vertexProgram == NULL) {
		CGerror Error = cgGetError();
		cerr << "Shader One Vertex Program COMPILE ERROR: " << cgGetErrorString(Error) << endl;
		cerr << cgGetLastListing(cgi->cgContext) << endl << endl;
		return false;
	}

	fragmentProgram = cgCreateProgramFromFile(cgi->cgContext, CG_SOURCE,
		"CG/shaderOne.cg", cgi->pixelCGprofile, "FragmentMain", NULL);
	if (fragmentProgram == NULL) {
		CGerror Error = cgGetError();
		cerr << "Shader One Fragment Program COMPILE ERROR: " << cgGetErrorString(Error) << endl;
		cerr << cgGetLastListing(cgi->cgContext) << endl << endl;
		return false;
	}

	// load programs
#ifdef GEOM_SHADER
	cgGLLoadProgram(geometryProgram);
#endif
	cgGLLoadProgram(vertexProgram);
	cgGLLoadProgram(fragmentProgram);

	// build some parameters by name such that we can set them later...
	vertexModelViewProj = cgGetNamedParameter(vertexProgram, "modelViewProj");
	geometryModelViewProj = cgGetNamedParameter(geometryProgram, "modelViewProj");
	fragmentKa = cgGetNamedParameter(fragmentProgram, "ka");
	fragmentC0 = cgGetNamedParameter(fragmentProgram, "C0");
	fragmentC1 = cgGetNamedParameter(fragmentProgram, "C1");
	vertexMorphRadius = cgGetNamedParameter(vertexProgram, "MR");
	vertexMorphCenter = cgGetNamedParameter(vertexProgram, "MC");
	vertexMorphFraction = cgGetNamedParameter(vertexProgram, "Mf");

	fragmentL = cgGetNamedParameter(fragmentProgram, "L");
	fragmentv0 = cgGetNamedParameter(fragmentProgram, "v0");
	fragmentv1 = cgGetNamedParameter(fragmentProgram, "v1");
	fragmentv2 = cgGetNamedParameter(fragmentProgram, "v2");
	fragmentv5 = cgGetNamedParameter(fragmentProgram, "v5");
	/*
	fragmentvv0 = cgGetNamedParameter(fragmentProgram, "vv0");
	fragmentvv1 = cgGetNamedParameter(fragmentProgram, "vv1");
	fragmentvv2 = cgGetNamedParameter(fragmentProgram, "vv2");
	fragmentvv5 = cgGetNamedParameter(fragmentProgram, "vv5");
	*/
	return true;

}

void ShaderOneInterface::PerFrameInit() {

	//set parameters
	cgGLSetStateMatrixParameter(vertexModelViewProj,
		CG_GL_MODELVIEW_PROJECTION_MATRIX, CG_GL_MATRIX_IDENTITY);

	cgGLSetStateMatrixParameter(
		geometryModelViewProj,
		CG_GL_MODELVIEW_PROJECTION_MATRIX, CG_GL_MATRIX_IDENTITY);

	cgSetParameter1f(fragmentKa, scene->ka);
	AABB aabb = scene->tms[0].GetAABB();
	// cerr << aabb.c0 << endl << aabb.c1 << endl;
	cgSetParameter3fv(fragmentC0, (float*)&aabb.c0);
	cgSetParameter3fv(fragmentC1, (float*)&aabb.c1);

	V3 C = (aabb.c0 + aabb.c1) * 0.5f;
	cgSetParameter3fv(vertexMorphCenter, (float*)&C);
	float mr = (aabb.c1 - aabb.c0).Length() / 3.0f;
	cgSetParameter1f(vertexMorphRadius, mr);
	cgSetParameter1f(vertexMorphFraction, scene->mf);

	//Parameters for Area Light
	V3 light = scene->L;
	V3 vert0 = scene->tms[0].verts[4];
	V3 vert1 = scene->tms[0].verts[5];
	V3 vert2 = scene->tms[0].verts[6];
	V3 vert5 = scene->tms[0].verts[1];

	V3 vvert0 = scene->tms[1].verts[4];
	V3 vvert1 = scene->tms[1].verts[5];
	V3 vvert2 = scene->tms[1].verts[6];
	V3 vvert5 = scene->tms[1].verts[1];

	cgSetParameter3fv(fragmentL, (float*)&light);
	cgSetParameter3fv(fragmentv0, (float*)&vert0);
	cgSetParameter3fv(fragmentv1, (float*)&vert1);
	cgSetParameter3fv(fragmentv2, (float*)&vert2);
	cgSetParameter3fv(fragmentv5, (float*)&vert5);
	/*
	cgSetParameter3fv(fragmentvv0, (float*)&vvert0);
	cgSetParameter3fv(fragmentvv1, (float*)&vvert1);
	cgSetParameter3fv(fragmentvv2, (float*)&vvert2);
	cgSetParameter3fv(fragmentvv5, (float*)&vvert5);
	*/
}

void ShaderOneInterface::PerFrameDisable() {
}


void ShaderOneInterface::BindPrograms() {

#ifdef GEOM_SHADER
	cgGLBindProgram(geometryProgram);
#endif
	cgGLBindProgram(vertexProgram);
	cgGLBindProgram(fragmentProgram);

}

void CGInterface::DisableProfiles() {

	cgGLDisableProfile(vertexCGprofile);
#ifdef GEOM_SHADER
	cgGLDisableProfile(geometryCGprofile);
#endif
	cgGLDisableProfile(pixelCGprofile);

}

void CGInterface::EnableProfiles() {

	cgGLEnableProfile(vertexCGprofile);
#ifdef GEOM_SHADER
	cgGLEnableProfile(geometryCGprofile);
#endif
	cgGLEnableProfile(pixelCGprofile);

}

