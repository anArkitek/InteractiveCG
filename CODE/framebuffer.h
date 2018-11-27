#pragma once

#include <FL/Fl.H>
#include <FL/Fl_Gl_Window.H>
#include <GL/glut.h>

#include "v3.h"
#include "ppc.h"
#include "m33.h"
//#include "cubemap.h"

class CubeMap;

class FrameBuffer : public Fl_Gl_Window {
public:

	// is hardware
	int ishw;

	unsigned int *pix;
	float *zb;
	int w, h;
	
	
	FrameBuffer(int u0, int v0, int _w, int _h);
	void draw();
	void KeyboardHandle();
	int handle(int guievent);
	void SetBGR(unsigned int bgr);

	/*
	 * @u, v: pixel coordinates
	 * @color: color value
	 * if (u, v) is in screen, set the pixel to be the color
	 */
	void SetGuarded(int u, int v, unsigned int color);

	/*
	* @u, v: pixel coordinates
	* @color: color value
	* set the pixel to be the color
	*/
	void Set(int u, int v, unsigned int color);

	/*
	* @u, v: pixel coordinates
	* get color of the pixel
	*/
	unsigned int Get(int u, int v);


	void DrawRectangle(int u0, int v0, int u1, int v1, unsigned int color);
	int ClipToScreen(int &u0, int &v0, int &u1, int &v1);

	void DrawCircle(int u0, int v0, int R, unsigned int color);
	void DrawSegment(V3 p0, V3 c0, V3 p1, V3 c1);
	void Draw3DSegment(V3 p0, V3 c0, V3 p1, V3 c1, PPC *ppc);
	
	// @p: point position
	// @c: point color
	// @ppc: planar pinhole camera
	// @psize: point size
	void Draw3DPoint(V3 p, V3 c, PPC *ppc, int psize);
	
	// screen space interpolation
	void DrawTriangle(V3 pp0, V3 c0, V3 pp1, V3 c1, V3 pp2, V3 c2, PPC *ppc0, PPC *ppc1);
	// model space interpolation
	void DrawTriangle(M33 q33, V3 pp0, V3 c0, V3 pp1, V3 c1, V3 pp2, V3 c2, PPC *ppc0, PPC *ppc1, FrameBuffer *fb1);

	void Draw2DTriangleWithPoints(M33 q33, V3 pp0, V3 pp1, V3 pp2, Point p0, Point p1, Point p2, PPC *ppc0, PPC *ppc1, FrameBuffer *fb1, CubeMap *cubemap, FrameBuffer *texture = nullptr, std::vector<Point> *bb_pts = nullptr);

	// @p0: point 0
	// @c0: color 0
	// @p1: point 1
	// @c1: color 1
	// @p2: point 2
	// @c2: color 2
	// @fb1: 3rd person framebuffer
	void Draw3DTriangle(interpolation_t interpolation, V3 p0, V3 c0, V3 p1, V3 c1, V3 p2, V3 c2, PPC *ppc0, PPC *ppc1, FrameBuffer *fb1, FrameBuffer *texture=nullptr);
	
	void Draw3DTriangleWithPoints(interpolation_t interpolation, Point p0, Point p1, Point p2, PPC *ppc0, PPC *ppc1, FrameBuffer *fb1, CubeMap *cubemap, FrameBuffer *texture=nullptr, std::vector<Point> *bb_pts = nullptr);

	bool IsInsideTriangle(V3 p, V3 p0, V3 p1, V3 p2);

	void ClearZB(float z0);

	/*
	 * @u, v: pixel coordinates
	 * @currz: current z value
	 * Compare current z-value and the stored z-value, return 0 if currz is smaller
	 */
	int Visible(int u, int v, float currz);

	void LoadTiff(char* fname);
	void SaveAsTiff(const char* fname);

	// textures
	void Draw3DTriangleWithTexture(interpolation_t, Point p0, Point p1, Point p2, PPC *ppc, FrameBuffer *texture);
	// @sta: s, t, alpha 
	// @text: texture
	V3 GetColorFromTexture(V3 sta, FrameBuffer* text);

	// ALPHA BLENDING
	void BlendGuarded(int u, int v, unsigned int color);
	// LIGHT
	V3 Lighting(Point p, V3 lightPos, PPC *ppc);


	// ShadowMap
	int lightNum;

	// input & output w are z-buffer values
	// @uvw: u, v, w
	// @ppc0: ppc with known uv
	// @ppc1: ppc with target uv
	V3 ProjMapping(V3 uvw, PPC *ppc0, PPC *ppc1);
	bool IsClose(float a, float b);

	// @ppc0: Viewer's Camera
	// @ppc1: 3rd person Camera
	// @fb1:  3rd person Camera

	void DrawTriangle(M33 q33, V3 pp0, V3 c0, V3 pp1, V3 c1, V3 pp2, V3 c2, PPC *ppc0, PPC *ppc1, FrameBuffer *fb1, FrameBuffer *texture);
	float GetAlphaFromTexture(V3 sta, FrameBuffer *text);
	


};