#include "cubemap.h"


CubeMap::CubeMap(std::vector<std::string> imgs)
{
	if (imgs.size() != 6)
	{
		std::cerr << "the number of cubemap images are inconsistent" << std::endl;

		return;
	}

	imgPaths = imgs;

	LoadImages(imgPaths);
	
	imgW = fbs[0]->w;
	imgH = fbs[0]->h;

	for (int i = 0; i < 6; ++i)
	{
		ppcs.push_back(new PPC(90.0f, imgW,imgH));
	}

	V3 C(0.0f, 0.0f, 0.0f);

	// top
	ppcs[0]->PositionAndOrient(C, V3(0.0f, 1.0f, 0.0f), V3(0.0f, 0.0, 1.0f));
	// bottom
	ppcs[1]->PositionAndOrient(C, V3(0.0f, -1.0f, 0.0f), V3(0.0f, 0.0f, -1.0f));
	// front
	ppcs[2]->PositionAndOrient(C, V3(0.0f, 0.0f, -1.0f), V3(0.0f, 1.0f, 0.0f));
	// back
	ppcs[3]->PositionAndOrient(C, V3(0.0f, 0.0f, 1.0f), V3(0.0f, 1.0f, 0.0f));
	// left
	ppcs[4]->PositionAndOrient(C, V3(-1.0f, 0.0f, 0.0f), V3(0.0f, 1.0f, 0.0f));
	// right
	ppcs[5]->PositionAndOrient(C, V3(1.0f, 0.0f, 0.0f), V3(0.0f, 1.0f, 0.0f));


}


void CubeMap::LoadImages(std::vector<std::string> sfnames)
{

	for (auto sfname : sfnames)
	{
		FrameBuffer *fb = new FrameBuffer(0, 0, 512, 512);
		char* cfname = new char[sfname.length() + 1];
		strcpy(cfname, sfname.c_str());
		fb->LoadTiff(cfname);
		fbs.push_back(fb);

		delete cfname;
	}

}


V3 CubeMap::DirectionLookup(V3 direction)
{
	V3 color(1.0f, 0.0f, 1.0f);
	for (int i = 0; i < 6; ++i, currPPCNum = (currPPCNum + 1) % 6)
	{
		if (currPPCNum == -1)
		{
			currPPCNum = 0;
		}

		V3 P = direction.UnitVector();
		V3 pp(0.0f,0.0f,0.0f);

		// judge w>0
		if (!ppcs[currPPCNum]->Project(P, pp))
		{
			continue;
		}

		// judge in screen
		if (pp[0] < 0.0f || pp[0] >= (float)imgW || pp[1] < 0.0f || pp[1] >= (float)imgH)
		{
			continue;
		}

		float s = static_cast<float>(pp[0]) / imgW;
		float t = static_cast<float>(pp[1]) / imgH;

		V3 sta = V3(s, t, 1.0f);
		color = fbs[currPPCNum]->GetColorFromTexture(sta, fbs[currPPCNum]);
	}

	// std::cout << "Direction Lookup Failed!" << std::endl;
	return color;
}