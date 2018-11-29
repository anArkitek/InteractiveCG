#pragma once

#include <vector>
#include "gui.h"
#include "framebuffer.h"
#include "ppc.h"
#include "tm.h"
#include "cubemap.h"

#include "CGInterface.h"



class Scene {
public:

	Scene();
	void DBG();
	
	// select ppc in this function
	void Render();
	
	// @currPPC: current ppc
	// @currfb: current framebuffer
	// set BGR and clear Z-Buffer then render objects
	void Render(PPC *currPPC, FrameBuffer *currfb);

	// Render Hardware
	void RenderHW();

	// Render GPU
	void RenderGPU();
	void ReloadShaders();

	PPC *ppc, *ppc3;

	TM *tms;
	V3 *tmCs;
	int tmsN;
	V3 L; // point light source

	// Cubes
	TM *cube;
	V3 *cubeCs;
	int cubeN;

	// Ground
	TM *ground;
	V3 *groundCs;
	int groundN;

	// Textures
	std::vector<std::vector<Point>> quads;
	
	//FrameBuffer *text0, *text1, *text2, *text3, *text4;

	CubeMap *cubemap;

	CGInterface * cgi;
	ShaderOneInterface *soi;

	GUI *gui;

	// Framebuffers
	FrameBuffer *fb, *fb3;
	FrameBuffer *hwfb;
	FrameBuffer *gpufb;

	// Light parameters
	float kse;
	float ka;
	float mf; // morph fraction
};

extern Scene *scene;