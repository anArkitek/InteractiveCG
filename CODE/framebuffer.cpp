#include <iostream>
#include <algorithm>
#include <iostream>
#include <cmath>

#include "scene.h"
#include "v3.h"
#include "math.h"

#include "tiffio.h"

#include "mathFunc.h"

using namespace std;



float max(float a, float b)
{
	return a > b ? a : b;
}


float min(float a, float b)
{
	return a < b ? a : b;
}

V3 ClampBetweenZeroAndOne(V3 sta)
{
	float a = max(sta[0] - static_cast<int>(sta[0]), 0.0f);
	float b = max(sta[1] - static_cast<int>(sta[1]), 0.0f);

	return V3(a, b, sta[2]);
}



float ClampBetweenZeroAndOne(float val)
{
	float temp = max(val, 0.0f);
	float res = min(temp, 1.0f);

	return res;
}




FrameBuffer::FrameBuffer(int u0, int v0,int _w, int _h) 
	: Fl_Gl_Window(u0, v0, _w, _h, 0) {

	// is hardware
	ishw = false;

	w = _w;
	h = _h;
	pix = new unsigned int[w*h];
	zb = new float[w*h];

}


void FrameBuffer::draw() {

	if (ishw)
	{
		scene->RenderHW();
	}
	else
	{
		glDrawPixels(w, h, GL_RGBA, GL_UNSIGNED_BYTE, pix);
	}


}


int FrameBuffer::handle(int event) {

	switch (event)
	{
	case FL_KEYBOARD: {
		KeyboardHandle();
		return 0;
	}
	default:
		break;
	}
	return 0;
}


void FrameBuffer::KeyboardHandle() {

	int key = Fl::event_key();
	switch (key) {
	case ',': {
		cerr << "INFO: pressed ," << endl;
		break;
	}
	default:
		cerr << "INFO: do not understand keypress" << endl;
	}
}


void FrameBuffer::SetGuarded(int u, int v, unsigned int color) {

	if (u < 0 || u > w - 1 || v < 0 || v > h - 1)
		return;

	Set(u, v, color);

}


void FrameBuffer::BlendGuarded(int u, int v, unsigned int color) {

	if (u < 0 || u > w - 1 || v < 0 || v > h - 1)
		return;

	pix[(h - 1 - v)*w + u] += color;

}


void FrameBuffer::Set(int u, int v, unsigned int color) 
{
	pix[(h - 1 - v)*w + u] = color;
}


unsigned int FrameBuffer::Get(int u, int v)
{
	if (u < 0 || u > w - 1 || v < 0 || v > h - 1)
	{
		std::cout << "INFO: Illegal Pixel" << std::endl;
		return 0;
	}
	return pix[(h - 1 - v) * w + u];
}


void FrameBuffer::SetBGR(unsigned int bgr) 
{
	for (int uv = 0; uv < w*h; uv++)
		pix[uv] = bgr;
}


// load a tiff image to pixel buffer
void FrameBuffer::LoadTiff(char* fname) 
{
	TIFF* in = TIFFOpen(fname, "r");
	if (in == NULL) {
		cerr << fname << " could not be opened" << endl;
		return;
	}

	int width, height;
	TIFFGetField(in, TIFFTAG_IMAGEWIDTH, &width);
	TIFFGetField(in, TIFFTAG_IMAGELENGTH, &height);
	if (w != width || h != height) {
		w = width;
		h = height;
		delete[] pix;
		pix = new unsigned int[w*h];
		size(w, h);
		glFlush();
		glFlush();
	}

	if (TIFFReadRGBAImage(in, w, h, pix, 0) == 0) {
		cerr << "failed to load " << fname << endl;
	}

	

	TIFFClose(in);
}


// save as tiff image
void FrameBuffer::SaveAsTiff(const char *fname) 
{
	TIFF* out = TIFFOpen(fname, "w");

	if (out == NULL) {
		cerr << fname << " could not be opened" << endl;
		return;
	}

	TIFFSetField(out, TIFFTAG_IMAGEWIDTH, w);
	TIFFSetField(out, TIFFTAG_IMAGELENGTH, h);
	TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, 4);
	TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, 8);
	TIFFSetField(out, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
	TIFFSetField(out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);

	for (uint32 row = 0; row < (unsigned int)h; row++) {
		TIFFWriteScanline(out, &pix[(h - row - 1) * w], row);
	}

	TIFFClose(out);
}


void FrameBuffer::DrawRectangle(int u0, int v0, int u1, int v1, unsigned int color) {

	if (!ClipToScreen(u0, v0, u1, v1))
		return;

	for (int u = u0; u <= u1; u++) {
		for (int v = v0; v <= v1; v++) {
			Set(u, v, color);
		}
	}

}


void FrameBuffer::DrawCircle(int u0, int v0, int r, unsigned int color) {

	int left = u0 - r;
	int right = u0 + r;
	int top = v0 - r;
	int bottom = v0 + r;
	if (!ClipToScreen(left, right, top, bottom))
		return;

	V3 cv(.5f + (float)u0, .5f + (float)v0, 0.0f);
	for (int u = left; u <= right; u++) {
		for (int v = top; v <= bottom; v++) {
			V3 pv(.5f + (float)u, .5f + (float)v, 0.0f);
			V3 dv = pv - cv;
			if (dv*dv > r*r)
				continue;
			Set(u, v, color);
		}
	}

}


int FrameBuffer::ClipToScreen(int &u0, int &v0, int &u1, int &v1)
{
	if (u1 < 0)
		return 0;
	if (u0 > w - 1)
		return 0;
	if (v1 < 0)
		return 0;
	if (v0 > h - 1)
		return 0;

	if (u0 < 0)
		u0 = 0;
	if (v0 < 0)
		v0 = 0;
	if (u1 > w - 1)
		u1 = w - 1;
	if (v1 > h - 1)
		v1 = h - 1;
	return 1;

}


void FrameBuffer::DrawSegment(V3 p0, V3 c0, V3 p1, V3 c1) {

	V3 p1p0 = p1 - p0;
	V3 c1c0 = c1 - c0;
	int pixN;
	if (fabsf(p1p0[0]) > fabsf(p1p0[1])) {
		pixN = (int)(fabsf(p1p0[0]) + 1);
	}
	else {
		pixN = (int)(fabsf(p1p0[1]) + 1);
	}
	for (int stepsi = 0; stepsi < pixN+1; stepsi++) {
		float fracf = (float) stepsi / (float) (pixN);
		V3 p = p0 + p1p0 * fracf;
		V3 c = c0 + c1c0 * fracf;
		int u = (int) p[0];
		int v = (int) p[1];
		if (u < 0 || u > w - 1 || v < 0 || v > h - 1)
			continue;
		if (!Visible(u, v, p[2]))
			continue;
		Set(u, v, c.GetColor());
	}

}


void FrameBuffer::ClearZB(float z0) 
{

	for (int i = 0; i < w*h; i++)
		zb[i] = z0;

}


int FrameBuffer::Visible(int u, int v, float currz) {
	// Check if in screen
	if (u < 0 || v < 0 || u >= w || v >= h)
		return 0;

	int uv = (h - 1 - v)*w + u;
	if (zb[uv] > currz)
		return 0;
	zb[uv] = currz;
	return 1;

}


void FrameBuffer::Draw3DPoint(V3 p, V3 c, PPC *ppc, int psize) {

	// get pp (Planar Point)
	V3 pp;
	if (!ppc->Project(p, pp))
		return;

	// test if in screen
	if (pp[0] < 0.0f || pp[0] >= (float)w || pp[1] < 0.0f || pp[1] >= (float)h)
		return;

	// test z-buffer
	if (!Visible(pp[0], pp[1], pp[2]))
		return;

	int u = (int)pp[0];
	int v = (int)pp[1];
	DrawRectangle(u - psize / 2, v - psize / 2, u + psize / 2, v + psize / 2, c.GetColor());

}


void FrameBuffer::Draw3DSegment(V3 p0, V3 c0, V3 p1, V3 c1, PPC *ppc) {

	V3 pp0, pp1;
	if (!ppc->Project(p0, pp0))
		return;
	if (!ppc->Project(p1, pp1))
		return;

	DrawSegment(pp0, c0, pp1, c1);

}


void FrameBuffer::Draw3DTriangle(interpolation_t interpolation, V3 p0, V3 c0, V3 p1, V3 c1, V3 p2, V3 c2, PPC *ppc0, PPC *ppc1, FrameBuffer *fb1, FrameBuffer *texture)
{
	V3 pp0, pp1, pp2;
	if (!ppc0->Project(p0, pp0))
		return;
	if (!ppc0->Project(p1, pp1))
		return;
	if (!ppc0->Project(p2, pp2))
		return;

	if (interpolation == SCREEN_SPACE_INTERPOLATION)
	{
		DrawTriangle(pp0, c0, pp1, c1, pp2, c2, ppc0, ppc1);
	}
	else
	{
		V3 V1_C = p0 - ppc0->C;
		V3 V2_C = p1 - ppc0->C;
		V3 V3_C = p2 - ppc0->C;

		M33 V_C33;
		V_C33.SetColumn(0, V1_C);
		V_C33.SetColumn(1, V2_C);
		V_C33.SetColumn(2, V3_C);

		M33 abc33;
		abc33.SetColumn(0, ppc0->a);
		abc33.SetColumn(1, ppc0->b);
		abc33.SetColumn(2, ppc0->c);

		M33 q33 = V_C33.Inverted()*abc33;

		if (texture == nullptr)
		{
			DrawTriangle(q33, pp0, c0, pp1, c1, pp2, c2, ppc0, ppc1, fb1);
		}
		else
		{
			DrawTriangle(q33, pp0, c0, pp1, c1, pp2, c2, ppc0, ppc1, fb1, texture);
			
		}
	}
}

void FrameBuffer::Draw3DTriangleWithPoints(interpolation_t interpolation, Point p0, Point p1, Point p2, PPC * ppc0, PPC *ppc1, FrameBuffer *fb1, CubeMap *cubemap, FrameBuffer *texture, std::vector<Point> *bb_pts)
{
	V3 pp0, pp1, pp2;
	if (!ppc0->Project(p0.pos, pp0))
		return;
	if (!ppc0->Project(p1.pos, pp1))
		return;
	if (!ppc0->Project(p2.pos, pp2))
		return;

	if (interpolation == SCREEN_SPACE_INTERPOLATION)
	{
		//DrawTriangle(pp0, p0.color, pp1, p1.color, pp2, p2.color, ppc0, ppc1);
	}
	else
	{
		V3 V1_C = p0.pos - ppc0->C;
		V3 V2_C = p1.pos - ppc0->C;
		V3 V3_C = p2.pos - ppc0->C;

		M33 V_C33;
		V_C33.SetColumn(0, V1_C);
		V_C33.SetColumn(1, V2_C);
		V_C33.SetColumn(2, V3_C);

		M33 abc33;
		abc33.SetColumn(0, ppc0->a);
		abc33.SetColumn(1, ppc0->b);
		abc33.SetColumn(2, ppc0->c);

		M33 q33 = V_C33.Inverted()*abc33;

		if (texture == nullptr)
		{
			//DrawTriangle(q33, pp0, p0.color, pp1, p1.color, pp2, p2.color, ppc0, ppc1, fb1);
			Draw2DTriangleWithPoints(q33, pp0, pp1, pp2, p0, p1, p2, ppc0, ppc1, fb1, cubemap, texture, bb_pts);
		}
		else
		{
			//DrawTriangle(q33, pp0, p0.color, pp1, p1.color, pp2, p2.color, ppc0, ppc1, fb1, texture);
			
		}
	}
}


void FrameBuffer::DrawTriangle(M33 q33, V3 pp0, V3 c0, V3 pp1, V3 c1, V3 pp2, V3 c2, PPC *ppc0, PPC *ppc1, FrameBuffer *fb1)
{
	AABB tribb(pp0);
	tribb.AddPoint(pp1);
	tribb.AddPoint(pp2);

	int left = static_cast<int>(tribb.c0[0]);
	int top = static_cast<int>(tribb.c0[1]);
	int right = static_cast<int>(tribb.c1[0]);
	int bottom = static_cast<int>(tribb.c1[1]);

	ClipToScreen(left, top, right, bottom);

	V3 r_channel(c0[0], c1[0], c2[0]);
	V3 g_channel(c0[1], c1[1], c2[1]);
	V3 b_channel(c0[2], c1[2], c2[2]);
	V3 z_channel(pp0[2], pp1[2], pp2[2]);

	float d_coef = q33.GetColumn(0).GetSum();
	float e_coef = q33.GetColumn(1).GetSum();
	float f_coef = q33.GetColumn(2).GetSum();

	V3 def = V3(d_coef, e_coef, f_coef);

	for (int v = top; v <= bottom; ++v)
	{
		for (int u = left; u <= right; ++u)
		{
			V3 pp_coords(u + 0.5f, v + 0.5f, 1.0f);
			if (IsInsideTriangle(pp_coords, pp0, pp1, pp2) && IsInsideTriangle(pp_coords, pp1, pp2, pp0) && IsInsideTriangle(pp_coords, pp2, pp0, pp1))
			{
				M33 q33_trans = q33.Transpose();
				V3 uv1 = V3(u, v, 1.0f);

				V3 qt_z = q33_trans * z_channel;
				float z_val = qt_z * uv1 / (def * uv1); 
				
				lightNum = 1;

				if (Visible(u, v, z_val))
				{
					if (ppc0 == ppc1)
					{
						V3 qt_r = q33_trans * r_channel;
						V3 qt_g = q33_trans * g_channel;
						V3 qt_b = q33_trans * b_channel;

						float r_val = qt_r * uv1 / (def * uv1);
						float g_val = qt_g * uv1 / (def * uv1);
						float b_val = qt_b * uv1 / (def * uv1);

						V3 color = V3(r_val, g_val, b_val);
						SetGuarded(u, v, color.GetColor());
					}

					// transparnt pixel
					else
					{
						for (int i = 0; i < lightNum; ++i)
						{
							V3 uvw(u, v, z_val);
							V3 uvw1 = ProjMapping(uvw, ppc0, ppc1);
							int u1 = static_cast<int>(uvw1[0]);
							int v1 = static_cast<int>(uvw1[1]);
							float z1 = uvw1[2];

							int u1v1 = (ppc1->h - 1 - v1) * ppc1->w + u1;
							if (u1v1 < ppc1->w * ppc1->h && u1v1 >= 0 && !IsClose(z1, fb1->zb[u1v1]))
							{
								SetGuarded(u, v, 0xFF000000);
							}
							else
							{
								V3 qt_r = q33_trans * r_channel;
								V3 qt_g = q33_trans * g_channel;
								V3 qt_b = q33_trans * b_channel;

								float r_val = qt_r * uv1 / (def * uv1);
								float g_val = qt_g * uv1 / (def * uv1);
								float b_val = qt_b * uv1 / (def * uv1);

								V3 color = V3(r_val, g_val, b_val);
								SetGuarded(u, v, color.GetColor());
							}
						}
					}
					
				}
			}
		}
	}
}


void FrameBuffer::Draw2DTriangleWithPoints(M33 q33, V3 pp0, V3 pp1, V3 pp2, Point p0, Point p1, Point p2, PPC * ppc0, PPC * ppc1, FrameBuffer * fb1, CubeMap *cubemap, FrameBuffer *texture, std::vector<Point> *bb_pts)
{
	AABB tribb(pp0);
	tribb.AddPoint(pp1);
	tribb.AddPoint(pp2);

	int left = static_cast<int>(tribb.c0[0]);
	int top = static_cast<int>(tribb.c0[1]);
	int right = static_cast<int>(tribb.c1[0]);
	int bottom = static_cast<int>(tribb.c1[1]);

	ClipToScreen(left, top, right, bottom);

	V3 r_channel(p0.color[0], p1.color[0], p2.color[0]);
	V3 g_channel(p0.color[1], p1.color[1], p2.color[1]);
	V3 b_channel(p0.color[2], p1.color[2], p2.color[2]);
	V3 z_channel(pp0[2], pp1[2], pp2[2]);

	V3 nx_channel( p0.normal[0], p1.normal[0], p2.normal[0] );
	V3 ny_channel( p0.normal[1], p1.normal[1], p2.normal[1] );
	V3 nz_channel( p0.normal[2], p1.normal[2], p2.normal[2] );

	V3 px_channel(p0.pos[0], p1.pos[0], p2.pos[0]);
	V3 py_channel(p0.pos[1], p1.pos[1], p2.pos[1]);
	V3 pz_channel(p0.pos[2], p1.pos[2], p2.pos[2]);

	float d_coef = q33.GetColumn(0).GetSum();
	float e_coef = q33.GetColumn(1).GetSum();
	float f_coef = q33.GetColumn(2).GetSum();

	V3 def = V3(d_coef, e_coef, f_coef);

	for (int v = top; v <= bottom; ++v)
	{
		for (int u = left; u <= right; ++u)
		{
			V3 pp_coords(u + 0.5f, v + 0.5f, 1.0f);
			if (IsInsideTriangle(pp_coords, pp0, pp1, pp2) && 
				IsInsideTriangle(pp_coords, pp1, pp2, pp0) && 
				IsInsideTriangle(pp_coords, pp2, pp0, pp1))
			{
				M33 q33_trans = q33.Transpose();
				V3 uv1 = V3(u, v, 1.0f);

				V3 qt_z = q33_trans * z_channel;
				float z_val = qt_z * uv1 / (def * uv1);

				lightNum = 1;

				if (Visible(u, v, z_val))
				{
					// They are the same camera
					//if (ppc0 == ppc1)
					if (true)
					{
						V3 qt_r = q33_trans * r_channel;
						V3 qt_g = q33_trans * g_channel;
						V3 qt_b = q33_trans * b_channel;

						V3 qt_nx = q33_trans * nx_channel;
						V3 qt_ny = q33_trans * ny_channel;
						V3 qt_nz = q33_trans * nz_channel;

						// billboard
						V3 qt_px = q33_trans * px_channel;
						V3 qt_py = q33_trans * py_channel;
						V3 qt_pz = q33_trans * pz_channel;


						float r_val = qt_r * uv1 / (def * uv1);
						float g_val = qt_g * uv1 / (def * uv1);
						float b_val = qt_b * uv1 / (def * uv1);

						float nx_val = qt_nx * uv1 / (def * uv1);
						float ny_val = qt_ny * uv1 / (def * uv1);
						float nz_val = qt_nz * uv1 / (def * uv1);

						// billboard
						float px_val = qt_px * uv1 / (def * uv1);
						float py_val = qt_py * uv1 / (def * uv1);
						float pz_val = qt_pz * uv1 / (def * uv1);

						V3 color = V3(r_val, g_val, b_val);
						V3 normal = V3(nx_val, ny_val, nz_val);
						V3 pos = V3(px_val, py_val, pz_val);

						V3 inciRay = V3(0.0f, 0.0f, 0.0f) -(ppc0->c + ppc0->a * u + ppc0->b * v);
						V3 reflRay = normal.Reflect(inciRay);
						//V3 reflRay = normal.Refract(inciRay);

						// billboard
						M33 M;
						M.SetColumn(0, (*bb_pts)[0].pos);
						M.SetColumn(1, (*bb_pts)[1].pos);
						M.SetColumn(2, (*bb_pts)[2].pos);

						M = M.Inverted();

						V3 q2 = M * pos;
						V3 q3 = M * reflRay;

						float w = (1 - q2[0] - q2[1] - q2[2]) / (q3[0] + q3[1] + q3[2]);

						V3 abc = q2 + q3 * w;

						V3 bb_pos = (*bb_pts)[0].pos * abc[0] +
							(*bb_pts)[1].pos * abc[1] +
							(*bb_pts)[2].pos * abc[2];

						//if (IsInsideTriangle(bb_pos, (*bb_pts)[0].pos, ))

						float bb_s = (bb_pos[0] - (*bb_pts)[0].pos[0]) / ((*bb_pts)[1].pos[0] - (*bb_pts)[0].pos[0]);
						float bb_t = (bb_pos[1] - (*bb_pts)[0].pos[1]) / ((*bb_pts)[1].pos[1] - (*bb_pts)[0].pos[1]);

						V3 refColor;

						if (bb_s >= 0 && bb_s <= 1 && bb_t >= 0 && bb_t <= 1)
						{
							int u = static_cast<int>(bb_s * fb1->w);
							int v = static_cast<int>(bb_t * fb1->h);

							if (fb1->zb[(h - 1 - v)*fb1->w + u] != 0)
							{
								refColor = GetColorFromTexture(V3(bb_s, bb_t, 1.0f), fb1);
							}
							else
							{
								refColor = cubemap->DirectionLookup(reflRay);
							}
						}
						else
						{
							refColor = cubemap->DirectionLookup(reflRay);
						}

						SetGuarded(u, v, refColor.GetColor());
					}


					else
					{
						return;

						for (int i = 0; i < lightNum; ++i)
						{
							V3 uvw(u, v, z_val);
							V3 uvw1 = ProjMapping(uvw, ppc0, ppc1);
							int u1 = static_cast<int>(uvw1[0]);
							int v1 = static_cast<int>(uvw1[1]);
							float z1 = uvw1[2];

							int u1v1 = (ppc1->h - 1 - v1) * ppc1->w + u1;


							if (u1v1 < ppc1->w * ppc1->h && u1v1 >= 0 && 
								!IsClose(z1, fb1->zb[u1v1]))
							{
								V3 qt_r = q33_trans * r_channel;
								V3 qt_g = q33_trans * g_channel;
								V3 qt_b = q33_trans * b_channel;

								V3 qt_nx = q33_trans * nx_channel;
								V3 qt_ny = q33_trans * ny_channel;
								V3 qt_nz = q33_trans * nz_channel;

								float r_val = qt_r * uv1 / (def * uv1);
								float g_val = qt_g * uv1 / (def * uv1);
								float b_val = qt_b * uv1 / (def * uv1);

								float nx_val = qt_nx * uv1 / (def * uv1);
								float ny_val = qt_ny * uv1 / (def * uv1);
								float nz_val = qt_nz * uv1 / (def * uv1);

								V3 color = V3(r_val, g_val, b_val);
								V3 normal = V3(nx_val, ny_val, nz_val);


								SetGuarded(u, v, color.GetColor());
							}
							else
							{
								//SetGuarded(u, v, V3(1.0f, 0.0f, 1.0f).GetColor());

								V3 qt_r = q33_trans * r_channel;
								V3 qt_g = q33_trans * g_channel;
								V3 qt_b = q33_trans * b_channel;

								float r_val = qt_r * uv1 / (def * uv1);
								float g_val = qt_g * uv1 / (def * uv1);
								float b_val = qt_b * uv1 / (def * uv1);

								// the original color
								V3 color0 = V3(r_val, g_val, b_val);

								// Get texture color 
								float s = static_cast<float>(u1) / fb1->w;
								float t = static_cast<float>(v1) / fb1->h;
								V3 sta(s, t, 1.0f);
								V3 color = GetColorFromTexture(sta, texture);
								float alpha = GetAlphaFromTexture(sta, texture);
								color = color * alpha + color0 * (1 - alpha);
								SetGuarded(u, v, color.GetColor());

							}
						}
					}
				}
			}
		}
	}
}



void FrameBuffer::DrawTriangle(V3 pp0, V3 c0, V3 pp1, V3 c1, V3 pp2, V3 c2, PPC *ppc0, PPC *ppc1)
{
	AABB tribb(pp0);
	tribb.AddPoint(pp1);
	tribb.AddPoint(pp2);

	int left = static_cast<int>(tribb.c0[0]);
	int top = static_cast<int>(tribb.c0[1]);
	int right = static_cast<int>(tribb.c1[0]);
	int bottom = static_cast<int>(tribb.c1[1]);

	ClipToScreen(left, top, right, bottom);

	M33 m;
	m[0] = V3(pp0[0], pp0[1], 1.0f);
	m[1] = V3(pp1[0], pp1[1], 1.0f);
	m[2] = V3(pp2[0], pp2[1], 1.0f);

	M33 mInver = m.Inverted();

	V3 r_channel(c0[0], c1[0], c2[0]);
	V3 g_channel(c0[1], c1[1], c2[1]);
	V3 b_channel(c0[2], c1[2], c2[2]);
	V3 z_channel(pp0[2], pp1[2], pp2[2]);

	V3 r_coef = mInver * r_channel;
	V3 g_coef = mInver * g_channel;
	V3 b_coef = mInver * b_channel;
	V3 z_coef = mInver * z_channel;

	for (int v = top; v <= bottom; ++v)
	{
		for (int u = left; u <= right; ++u)
		{
			V3 pp_coords(u + 0.5f, v + 0.5f, 1.0f);
			if (IsInsideTriangle(pp_coords, pp0, pp1, pp2) && IsInsideTriangle(pp_coords, pp1, pp2, pp0) && IsInsideTriangle(pp_coords, pp2, pp0, pp1))
			{
				float z_val = z_coef * pp_coords;
				if (Visible(u, v, z_val))
				{
					float r_val = r_coef * pp_coords;
					float g_val = g_coef * pp_coords;
					float b_val = b_coef * pp_coords;

					V3 color = V3(r_val, g_val, b_val);
					SetGuarded(u, v, color.GetColor());

				}
			}
		}
	}
}


bool FrameBuffer::IsInsideTriangle(V3 p, V3 v1, V3 v2, V3 v3)
{
	float x[3], y[3];
	x[1] = v1[0];
	x[2] = v2[0];
	y[1] = v1[1];
	y[2] = v2[1];

	float xCo = y[2] - y[1];
	float yCo = x[2] - x[1];
	float x1y2 = x[1] * y[2];
	float y1x2 = y[1] * x[2];
	float res1 = p[0] * xCo - p[1] * yCo - x1y2 + y1x2;
	float res2 = v3[0] * xCo - v3[1] * yCo - x1y2 + y1x2;

	return res1 * res2 >= 0.0f ? true : false;
}


void FrameBuffer::Draw3DTriangleWithTexture(interpolation_t interpolation, Point p0, Point p1, Point p2, PPC *ppc, FrameBuffer *texture)
{
	V3 pp0, pp1, pp2;
	if (!ppc->Project(p0.pos, pp0))
		return;
	if (!ppc->Project(p1.pos, pp1))
		return;
	if (!ppc->Project(p2.pos, pp2))
		return;

	if (interpolation == SCREEN_SPACE_INTERPOLATION)
	{
		//DrawTriangle(pp0, c0, pp1, c1, pp2, c2, u, v, texture);
		// ignore them for now
	}
	else
	{
		// MODEL_SPACE_INTERPOLATION
		V3 V1_C = p0.pos - ppc->C;
		V3 V2_C = p1.pos - ppc->C;
		V3 V3_C = p2.pos - ppc->C;

		M33 V_C33;
		V_C33.SetColumn(0, V1_C);
		V_C33.SetColumn(1, V2_C);
		V_C33.SetColumn(2, V3_C);

		M33 abc33;
		abc33.SetColumn(0, ppc->a);
		abc33.SetColumn(1, ppc->b);
		abc33.SetColumn(2, ppc->c);

		M33 q33 = V_C33.Inverted()*abc33;


		// DRAW TRIANGLE ON SCREEN
		AABB tribb(pp0);
		tribb.AddPoint(pp1);
		tribb.AddPoint(pp2);

		int left = static_cast<int>(tribb.c0[0]);
		int top = static_cast<int>(tribb.c0[1]);
		int right = static_cast<int>(tribb.c1[0]);
		int bottom = static_cast<int>(tribb.c1[1]);

		ClipToScreen(left, top, right, bottom);

		// rgb
		V3 r_channel(p0.color[0], p1.color[0], p2.color[0]);
		V3 g_channel(p0.color[1], p1.color[1], p2.color[1]);
		V3 b_channel(p0.color[2], p1.color[2], p2.color[2]);

		float d_coef = q33.GetColumn(0).GetSum();
		float e_coef = q33.GetColumn(1).GetSum();
		float f_coef = q33.GetColumn(2).GetSum();

		V3 def = V3(d_coef, e_coef, f_coef);

		for (int v = top; v <= bottom; ++v)
		{
			for (int u = left; u <= right; ++u)
			{
				V3 pp_coords(u + 0.5f, v + 0.5f, 1.0f);
				if (IsInsideTriangle(pp_coords, pp0, pp1, pp2) && IsInsideTriangle(pp_coords, pp1, pp2, pp0) && IsInsideTriangle(pp_coords, pp2, pp0, pp1))
				{
					M33 q33_trans = q33.Transpose();
					//V3 uv1 = V3(u, v, 1.0f);

					V3 z_channel(pp0[2], pp1[2], pp2[2]);
					V3 qt_z = q33_trans * z_channel;
					//float z_val = qt_z * uv1 / (def * uv1);
					float z_val = qt_z * pp_coords / (def * pp_coords);

					if (Visible(u, v, z_val))
					{
						if (texture == nullptr)
						{
							// COLOR INTERPOLATION, WE DONT CARE ABOUT TEXTURE
							V3 qt_r = q33_trans * r_channel;
							V3 qt_g = q33_trans * g_channel;
							V3 qt_b = q33_trans * b_channel;

							float r_val = qt_r * pp_coords / (def * pp_coords);
							float g_val = qt_g * pp_coords / (def * pp_coords);
							float b_val = qt_b * pp_coords / (def * pp_coords);

							V3 color = V3(r_val, g_val, b_val);
							BlendGuarded(u, v, color.GetColor());
						}
						else
						{
							// TEXTURE ST CALCULATION, WE DONT CARE ABOUT COLOR
							float k = q33[1] * V3(u, v, 1.0f) / (q33.GetColumn(0).GetSum() * u + q33.GetColumn(1).GetSum() * v + q33.GetColumn(2).GetSum());
							float l = q33[2] * V3(u, v, 1.0f) / (q33.GetColumn(0).GetSum() * u + q33.GetColumn(1).GetSum() * v + q33.GetColumn(2).GetSum());
							float dist = q33.GetColumn(0).GetSum() * u + q33.GetColumn(1).GetSum() * v + q33.GetColumn(2).GetSum();

							V3 sta = p0.sta * (1 - k - l) + p1.sta * k + p2.sta * l;
							V3 normal = p0.normal * (1 - k - l) + p1.normal * k + p2.normal * l;

							sta = ClampBetweenZeroAndOne(sta);

							V3 color = GetColorFromTexture(sta, texture);
							float alpha = GetAlphaFromTexture(sta, texture);
							BlendGuarded(u, v, color.GetColor());
						}
					}
				}
			}
		}
	}
}


// Bilinear Interpolation
V3 FrameBuffer::GetColorFromTexture(V3 sta, FrameBuffer *text)
{
	sta[0] = sta[0] - static_cast<int>(sta[0]);
	sta[1] = sta[1] - static_cast<int>(sta[1]);

	float u = sta[0] * (text->w - 1);
	float v = sta[1] * (text->h - 1);
	float alpha = sta[2];

	// get the four pixels to implement bilinear interpolation
	int lefti = std::clamp(static_cast<int>(u - 0.5), 0, text->w - 1);
	int topi = std::clamp(static_cast<int>(v - 0.5), 0, text->w - 1);

	int righti = std::clamp(static_cast<int>(u + 0.5), 0, text->w - 1);
	int bottomi = std::clamp(static_cast<int>(v + 0.5), 0, text->h - 1);

	int a = (text->h - 1 - topi) * text->w + lefti;
	unsigned int color_tl = text->pix[a];
	
	int b = (text->h - 1 - topi) * text->w + righti;
	unsigned int color_tr = text->pix[b];
	
	int c = (text->h - 1 - bottomi) * text->w + lefti;
	unsigned int color_bl = text->pix[c];
	
	int d = (text->h - 1 - bottomi) * text->w + righti;
	unsigned int color_br = text->pix[d];

	V3 c0, c1, c2, c3;
	c0.SetFromColor(color_tl);
	c1.SetFromColor(color_tr);
	c2.SetFromColor(color_bl);
	c3.SetFromColor(color_br);

	unsigned char *alpha_tl = (unsigned char*)&color_tl;
	unsigned char *alpha_tr = (unsigned char*)&color_tr;
	unsigned char *alpha_bl = (unsigned char*)&color_bl;
	unsigned char *alpha_br = (unsigned char*)&color_br;
	
	float alpha0= ((float)alpha_tl[3]) / 255.0f;
	float alpha1= ((float)alpha_tr[3]) / 255.0f;
	float alpha2 = ((float)alpha_bl[3]) / 255.0f;
	float alpha3 = ((float)alpha_bl[3]) / 255.0f;

	float leftf = lefti+ 0.5;
	float rightf = righti + 0.5;
	float topf = topi + 0.5;
	float bottomf = bottomi + 0.5;

	float delta_s = ClampBetweenZeroAndOne(u - leftf);
	float delta_t = ClampBetweenZeroAndOne(v - topf);

	V3 color = c0 * (1 - delta_s) * (1 - delta_t) + c1 * delta_s * (1 - delta_t) + 
		c2 * (1 - delta_s) * delta_t + c3* delta_s * delta_t;
	return color;
}


float FrameBuffer::GetAlphaFromTexture(V3 sta, FrameBuffer *text)
{

	float s = sta[0] * (text->w - 1);
	float t = sta[1] * (text->h - 1);

	// get the four pixels to implement bilinear interpolation
	int lefti = max(static_cast<int>(s - 0.5), 0);
	int topi = max(static_cast<int>(t - 0.5), 0);

	int righti = min(static_cast<int>(s + 0.5), text->w - 1);
	int bottomi = min(static_cast<int>(t + 0.5), text->h - 1);

	int a = (text->h - 1 - topi) * text->w + lefti;
	unsigned int color_tl = text->pix[a];

	int b = (text->h - 1 - topi) * text->w + righti;
	unsigned int color_tr = text->pix[b];

	int c = (text->h - 1 - bottomi) * text->w + lefti;
	unsigned int color_bl = text->pix[c];

	int d = (text->h - 1 - bottomi) * text->w + righti;
	unsigned int color_br = text->pix[d];


	unsigned char *alpha_tl = (unsigned char*)&color_tl;
	unsigned char *alpha_tr = (unsigned char*)&color_tr;
	unsigned char *alpha_bl = (unsigned char*)&color_bl;
	unsigned char *alpha_br = (unsigned char*)&color_br;

	float alpha0 = ((float)alpha_tl[3]) / 255.0f;
	float alpha1 = ((float)alpha_tr[3]) / 255.0f;
	float alpha2 = ((float)alpha_bl[3]) / 255.0f;
	float alpha3 = ((float)alpha_bl[3]) / 255.0f;

	float leftf = lefti + 0.5;
	float rightf = righti + 0.5;
	float topf = topi + 0.5;
	float bottomf = bottomi + 0.5;

	float delta_s = ClampBetweenZeroAndOne(s - leftf);
	float delta_t = ClampBetweenZeroAndOne(t - topf);

	float alpha = alpha0 * (1 - delta_s) * (1 - delta_t) + alpha1 * delta_s * (1 - delta_t) +
		alpha2 * (1 - delta_s) * delta_t + alpha3 * delta_s * delta_t;
	return alpha;
}


V3 FrameBuffer::ProjMapping(V3 uvw, PPC *ppc0, PPC *ppc1)
{
	M33 abc0;
	abc0.SetColumn(0, ppc0->a);
	abc0.SetColumn(1, ppc0->b);
	abc0.SetColumn(2, ppc0->c);

	M33 abc1;
	abc1.SetColumn(0, ppc1->a);
	abc1.SetColumn(1, ppc1->b);
	abc1.SetColumn(2, ppc1->c);
	M33 abc1Inv = abc1.Inverted();

	V3 q31 = abc1Inv * (ppc0->C - ppc1->C);
	M33 q33 = abc1Inv * abc0;

	float w0 = 1.0f / uvw[2];
	V3 uv1_w = V3(uvw[0], uvw[1], 1.0f) * w0;
	float w1 = 1.0f / (q31[2] + q33[2] * uv1_w);
	float u1 = (q31[0] + q33[0] * uv1_w) * w1;
	float v1 = (q31[1] + q33[1] * uv1_w) * w1;

	return V3(u1, v1, w1);
}


bool FrameBuffer::IsClose(float a, float b)
{
	float epsilon = 1e-2;
	return a + epsilon > b  ? 1 : 0;
}


void FrameBuffer::DrawTriangle(M33 q33, V3 pp0, V3 c0, V3 pp1, V3 c1, V3 pp2, V3 c2, PPC *ppc0, PPC *ppc1, FrameBuffer *fb1, FrameBuffer *texture)
{
	AABB tribb(pp0);
	tribb.AddPoint(pp1);
	tribb.AddPoint(pp2);

	int left = static_cast<int>(tribb.c0[0]);
	int top = static_cast<int>(tribb.c0[1]);
	int right = static_cast<int>(tribb.c1[0]);
	int bottom = static_cast<int>(tribb.c1[1]);

	ClipToScreen(left, top, right, bottom);

	V3 r_channel(c0[0], c1[0], c2[0]);
	V3 g_channel(c0[1], c1[1], c2[1]);
	V3 b_channel(c0[2], c1[2], c2[2]);
	V3 z_channel(pp0[2], pp1[2], pp2[2]);

	float d_coef = q33.GetColumn(0).GetSum();
	float e_coef = q33.GetColumn(1).GetSum();
	float f_coef = q33.GetColumn(2).GetSum();

	V3 def = V3(d_coef, e_coef, f_coef);

	for (int v = top; v <= bottom; ++v)
	{
		for (int u = left; u <= right; ++u)
		{
			V3 pp_coords(u + 0.5f, v + 0.5f, 1.0f);
			if (IsInsideTriangle(pp_coords, pp0, pp1, pp2) && IsInsideTriangle(pp_coords, pp1, pp2, pp0) && IsInsideTriangle(pp_coords, pp2, pp0, pp1))
			{
				M33 q33_trans = q33.Transpose();
				V3 uv1 = V3(u, v, 1.0f);

				V3 qt_z = q33_trans * z_channel;
				float z_val = qt_z * uv1 / (def * uv1);

				lightNum = 1;

				if (Visible(u, v, z_val))
				{
					// They are the same camera
					if (ppc0 == ppc1)
					{
						V3 qt_r = q33_trans * r_channel;
						V3 qt_g = q33_trans * g_channel;
						V3 qt_b = q33_trans * b_channel;

						float r_val = qt_r * uv1 / (def * uv1);
						float g_val = qt_g * uv1 / (def * uv1);
						float b_val = qt_b * uv1 / (def * uv1);

						V3 color = V3(r_val, g_val, b_val);
						SetGuarded(u, v, color.GetColor());
					}

					
					else
					{
						for (int i = 0; i < lightNum; ++i)
						{
							V3 uvw(u, v, z_val);
							V3 uvw1 = ProjMapping(uvw, ppc0, ppc1);
							int u1 = static_cast<int>(uvw1[0]);
							int v1 = static_cast<int>(uvw1[1]);
							float z1 = uvw1[2];

							int u1v1 = (ppc1->h - 1 - v1) * ppc1->w + u1;
							
							if (u1v1 < ppc1->w * ppc1->h && u1v1 >= 0 && !IsClose(z1, fb1->zb[u1v1]))
							{
								V3 qt_r = q33_trans * r_channel;
								V3 qt_g = q33_trans * g_channel;
								V3 qt_b = q33_trans * b_channel;

								float r_val = qt_r * uv1 / (def * uv1);
								float g_val = qt_g * uv1 / (def * uv1);
								float b_val = qt_b * uv1 / (def * uv1);

								V3 color = V3(r_val, g_val, b_val);
								SetGuarded(u, v, color.GetColor());
							}
							else
							{
								V3 qt_r = q33_trans * r_channel;
								V3 qt_g = q33_trans * g_channel;
								V3 qt_b = q33_trans * b_channel;

								float r_val = qt_r * uv1 / (def * uv1);
								float g_val = qt_g * uv1 / (def * uv1);
								float b_val = qt_b * uv1 / (def * uv1);

								// the original color
								V3 color0 = V3(r_val, g_val, b_val);

								// Get texture color 
								//float s = static_cast<float>(u1) / fb1->w;
								//float t = static_cast<float>(v1) / fb1->h;
								//V3 sta(s, t, 1.0f);
								//V3 color = GetColorFromTexture(sta, texture);
								//float alpha = GetAlphaFromTexture(sta, texture);
								//color = color * alpha + color0 * (1 - alpha);
								//SetGuarded(u, v, color.GetColor());

								//
								
							}
						}
					}

				}
			}
		}
	}
}
