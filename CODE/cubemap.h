#pragma once

#include <vector>
#include <cstring>
#include <iostream>
#include <map>
#include <iterator>

#include "ppc.h"

#include "tiffio.h"
#include "framebuffer.h"


struct CubeImg
{
	std::vector<unsigned int> pix;
	int w;
	int h;
};


class CubeMap
{
public:
	CubeMap(std::vector<std::string> images);

	void LoadImages(std::vector<std::string> sfnames);

	V3 DirectionLookup(V3 direction);

	std::vector<std::string> imgPaths;
	std::vector<CubeImg> cubeimages;

	std::vector<PPC *> ppcs;
	std::vector<FrameBuffer *> fbs;

	// image width & height
	int imgW;
	int imgH;

	int currPPCNum = -1;
};