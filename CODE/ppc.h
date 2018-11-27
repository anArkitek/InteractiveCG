#pragma once

#include "v3.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

enum interpolation_t
{
	SCREEN_SPACE_INTERPOLATION,
	MODEL_SPACE_INTERPOLATION
};


class Point
{
public:
	// @pos: position
	// @color: color
	// @normal: normal vector
	// @sta: s, t, alpha
	Point(V3 pos, V3 color, V3 normal, V3 sta) : pos(pos), color(color), normal(normal), sta(sta) {};

	V3 pos;		// 3d coordinates or u, v, z_value
	V3 color;	// color
	V3 normal;	// normal
	V3 sta;		// s, t, alpha
};

// A Declaration
class FrameBuffer;


// @PPC: Planar Pinhole Camera
class PPC {
public:
	V3 a, b, c, C;
	int w, h;

	// hw2-1-a
	// @hfov: horizontal field of view
	// @w: width of ppc
	// @h: height of ppc
	PPC(float hfov, int _w, int _h);
	
	// hw2-1-b
	void Translate(V3 vec);

	V3 GetVD();
	float GetFocalLength();

	// hw2-1-c
	void Pan(float angled);
	void Tilt(float angled);
	void Roll(float angled);

	// hw2-1-d
	void ChangeFocalLength(float scf);

	void PositionAndOrient(V3 C1, V3 L1, V3 vpv);

	// return 0 if w < 0, else return 1
	// @p: real-world coordinates
	// @pp: image coordinates
	int Project(V3 p, V3 &pp);

	// hw2-1-f
	// @ppc0: start ppc
	// @ppc1: end ppc
	// @stepsN: total number of interpolation
	// @step: the interpolation position, it is in range [0, stepsN]
	void InterpolateTwoCameras(PPC* ppc0, PPC* ppc1, int stepsN, int step);
	void InterpolateTwoCameras(PPC* ppc0, PPC* ppc1, float scf);


	void Visualize(PPC *ppc3, FrameBuffer *fb3, float vf);

	// hw2-1-g
	void ReadTxt(std::string fname);
	void WriteTxt(std::string fname);

	V3 UnprojectPixel(float uf, float vf, float curf);
	V3 Unproject(V3 pP);
	V3 GetRay(int u, int v);
};