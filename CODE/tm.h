#pragma once
#include "v3.h"
#include "ppc.h"
#include "framebuffer.h"
#include "aabb.h"
#include "cubemap.h"



class TM {
public:
	V3 *verts, *colors, *normals, *sta;
	int vertsN;

	// all the indices of vertices [# = 3*N]
	unsigned int *tris;
	
	// number of triangles
	int trisN;

	TM() : verts(0), vertsN(0), colors(0), normals(0), tris(0), trisN(0) {};
	
	// @O: center point
	// @rw: width
	// @rh: height
	void SetRectangle(V3 O, float rw, float rh);
	void SetRectangleWithFourPoints(Point p0, Point p1, Point p2, Point p3);
	void Allocate();
	void RenderPoints(PPC *ppc, FrameBuffer *fb);

	// hw-2-f
	void RenderWireframe(PPC *ppc, FrameBuffer *fb);

	// hw-2-g
	// @ppc0: current ppc
	// @fb0: current framebuffer
	// @ppc1: 3rd view ppc
	// @fb1: 3rd view framebuffer
	void RenderMesh(interpolation_t interpolation, PPC *ppc0, FrameBuffer *fb0, PPC *ppc1, FrameBuffer *fb1, CubeMap* cubemap, FrameBuffer *texture=nullptr, std::vector<Point> *bb_pts = nullptr);

	//void RenderTexture(interpolation_t interpolation, PPC *ppc, FrameBuffer *fb);

	void RotateAboutArbitraryAxis(V3 O, V3 a, float angled);
	
	// hw-2-a
	// @fname: file name
	// load from bin file
	void LoadBin(char *fname);

	// hw-2-b
	AABB GetAABB();

	// hw-2-e
	void PositionAndSize(V3 tmC, float tmSize);

	// hw-2-c
	void Translate(V3 tv);

	// hw-2-d
	void Scale(float scf);
	V3 GetCenter();

	// @mc: material color
	// @lightPos: point light position
	void Light(V3 mc, V3 lightPos, FrameBuffer *shadowMap = nullptr);

	void RenderQuad(interpolation_t interpolation, PPC *ppc, FrameBuffer *fb, FrameBuffer *texture);

	void RenderFilled(PPC *ppc, FrameBuffer * fb);

	std::vector<Point> GenBillboard(V3 src);
};

