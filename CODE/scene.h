#pragma once

#include <vector>
#include "gui.h"
#include "framebuffer.h"
#include "ppc.h"
#include "tm.h"
#include "cubemap.h"



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

	GUI *gui;
	FrameBuffer *fb, *fb3;
	
	PPC *ppc, *ppc3;

	TM *tms;
	V3 *tmCs;
	int tmsN;
	V3 L; // point light source
	
	// Textures
	std::vector<std::vector<Point>> quads;
	
	//FrameBuffer *text0, *text1, *text2, *text3, *text4;

	CubeMap *cubemap;

};

extern Scene *scene;