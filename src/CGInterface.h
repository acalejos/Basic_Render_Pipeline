#pragma once

#include <Cg/cgGL.h>
#include <Cg/cg.h>

// two classes defining the interface between the CPU and GPU

// models part of the CPU-GPU interface that is independent of specific shaders
class CGInterface {

public:
	CGprofile   vertexCGprofile; // vertex shader profile of GPU
	CGprofile   geometryCGprofile; // geometry shader profile of GPU
	CGprofile   pixelCGprofile; // pixel shader profile of GPU
	CGcontext  cgContext;
	void PerSessionInit(); // per session initialization
	CGInterface(); // constructor
	void EnableProfiles(); // enable GPU rendering
	void DisableProfiles(); // disable GPU rendering
};


// models the part of the CPU-GPU interface for a specific shader
//        here there is a single shader "ShaderOne"
//  a shader consists of a vertex, a geometry, and a pixel (fragment) shader
// fragment == pixel; shader == program; e.g. pixel shader, pixel program, fragment shader, fragment program, vertex shader, etc.
class ShaderOneInterface {

	CGprogram geometryProgram; // the geometry shader to be used for the "ShaderOne"
	CGprogram vertexProgram;
	CGprogram fragmentProgram;
	// uniform parameters, i.e parameters that have the same value for all geometry rendered
	CGparameter vertexModelViewProj; // a matrix combining projection and modelview matrices
	CGparameter geometryModelViewProj; // geometry shader
	CGparameter fragmentKa; // ambient coefficient for fragment shader
	CGparameter fragmentC0, fragmentC1; // two corners of aabb
	CGparameter vertexMorphRadius, vertexMorphCenter, vertexMorphFraction; // for morphing mesh to sphere
	//CGparameter fragmentTopLeft, fragmentTopRight, fragmentBottomLeft, fragmentLightWidth, fragmentLightHeight; // For area light
	CGparameter fragmentL;
	CGparameter fragmentv0,fragmentv1,fragmentv2,fragmentv5;
	//CGparameter fragmentvv0, fragmentvv1, fragmentvv2, fragmentvv5;
	//CGparameter b1v0, b1v1, b1v2, b1v3;
	//CGparameter b2v0, b2v1, b2v2, b2v3;
	//CGparameter b3v0, b3v1, b3v2, b3v3;
	//CGparameter fragmentBoxTris1, fragmentBoxTris2, fragmentBoxTris3;
public:
	ShaderOneInterface() {};
	bool PerSessionInit(CGInterface *cgi); // per session initialization
	void BindPrograms(); // enable geometryProgram, vertexProgram, fragmentProgram
	void PerFrameInit(); // set uniform parameter values, etc.
	void PerFrameDisable(); // disable programs

};
