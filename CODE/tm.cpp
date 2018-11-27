
#include <iostream>

using namespace std;

#include <fstream>

#include "mathFunc.h"

#include "tm.h"
#include "aabb.h"
#include "scene.h"

void TM::RotateAboutArbitraryAxis(V3 O, V3 a, float angled) {

	for (int vi = 0; vi < vertsN; vi++) {
		verts[vi] = verts[vi].RotateThisPointAboutArbitraryAxis(O, a, angled);
	}

}


void TM::RenderPoints(PPC *ppc, FrameBuffer *fb) {

	for (int vi = 0; vi < vertsN; vi++) {
		fb->Draw3DPoint(verts[vi], colors[vi], ppc, 7);
	}

}


void TM::RenderWireframe(PPC *ppc, FrameBuffer *fb) {

	for (int tri = 0; tri < trisN; tri++) {
		for (int ei = 0; ei < 3; ei++) {
			int vi0 = tris[3 * tri + ei];
			int vi1 = tris[3 * tri + ((ei + 1) % 3)];
			fb->Draw3DSegment(verts[vi0], colors[vi0], 
				verts[vi1], colors[vi1], ppc);
		}
	}

}


void TM::RenderMesh(interpolation_t interpolation, PPC* ppc0, FrameBuffer* fb0, PPC *ppc1, FrameBuffer *fb1, CubeMap *cubemap, FrameBuffer *texture, std::vector<Point> *bb_pts)
{
	for (int tri = 0; tri < trisN; ++tri)
	{
		int vi0 = tris[tri * 3 + 0];
		int vi1 = tris[tri * 3 + 1];
		int vi2 = tris[tri * 3 + 2];

		Point p0 = Point(verts[vi0], colors[vi0], normals[vi0], V3(0.0f, 0.0f, 0.0f));
		Point p1 = Point(verts[vi1], colors[vi1], normals[vi1], V3(0.0f, 0.0f, 0.0f));
		Point p2 = Point(verts[vi2], colors[vi2], normals[vi2], V3(0.0f, 0.0f, 0.0f));
		
		//fb0->Draw3DTriangle(interpolation, verts[vi0], colors[vi0], verts[vi1], colors[vi1], verts[vi2], colors[vi2], ppc0, ppc1, fb1, texture);
		
		//std::cout << (*bb_pts)[0].pos << std::endl;

		fb0->Draw3DTriangleWithPoints(interpolation, p0, p1, p2, ppc0, ppc1, fb1, cubemap, texture, bb_pts);
		
	}
}

//void TM::RenderTexture(interpolation_t interpolation, PPC *ppc, FrameBuffer *fb)
//{
//	for (int tri = 0; tri < trisN; ++tri)
//	{
//		int vi0 = tris[tri * 3 + 0];
//		int vi1 = tris[tri * 3 + 1];
//		int vi2 = tris[tri * 3 + 2];
//
//		fb->Draw3DTriangle(interpolation, verts[vi0], colors[vi0], verts[vi1], colors[vi1], verts[vi2], colors[vi2], ppc);
//	}
//}


void TM::SetRectangle(V3 O, float rw, float rh) {

	vertsN = 4;
	trisN = 2;
	Allocate();

	verts[0] = O + V3(-rw / 2.0f, +rh / 2.0f, 0.0f);
	verts[1] = O + V3(-rw / 2.0f, -rh / 2.0f, 0.0f);
	verts[2] = O + V3(+rw / 2.0f, -rh / 2.0f, 0.0f);
	verts[3] = O + V3(+rw / 2.0f, +rh / 2.0f, 0.0f);

	int tri = 0;
	tris[3 * tri + 0] = 0;
	tris[3 * tri + 1] = 1;
	tris[3 * tri + 2] = 2;
	tri++;

	tris[3 * tri + 0] = 2;
	tris[3 * tri + 1] = 3;
	tris[3 * tri + 2] = 0;
	tri++;

	for (int vi = 0; vi < vertsN; vi++) {
		colors[vi] = V3(0.0f, 0.0, 0.0f);
	}

}

void TM::SetRectangleWithFourPoints(Point p0, Point p1, Point p2, Point p3) {

	vertsN = 4;
	trisN = 2;
	Allocate();

	verts[0] = p0.pos;
	verts[1] = p1.pos;
	verts[2] = p2.pos;
	verts[3] = p3.pos;

	colors[0] = p0.color;
	colors[1] = p1.color;
	colors[2] = p2.color;
	colors[3] = p3.color;

	normals[0] = p0.normal;
	normals[1] = p1.normal;
	normals[2] = p2.normal;
	normals[3] = p3.normal;


	sta[0] = p0.sta;
	sta[1] = p1.sta;
	sta[2] = p2.sta;
	sta[3] = p3.sta;

	// two triangles
	tris[0] = 0;
	tris[1] = 1;
	tris[2] = 2;
	tris[3] = 1;
	tris[4] = 2;
	tris[5] = 3;

}


void TM::Allocate() {

	verts = new V3[vertsN];
	colors = new V3[vertsN];
	tris = new unsigned int[3 * trisN];
	normals = new V3[vertsN];
	sta = new V3[vertsN];
}


void TM::LoadBin(char *fname) {

	ifstream ifs(fname, ios::binary);
	if (ifs.fail()) {
		cerr << "INFO: cannot open file: " << fname << endl;
		return;
	}

	// 0. vertsN
	ifs.read((char*)&vertsN, sizeof(int));
	char yn;
	ifs.read(&yn, 1); // always xyz
	if (yn != 'y') {
		cerr << "INTERNAL ERROR: there should always be vertex xyz data" << endl;
		return;
	}
	if (verts)
		delete verts;
	
	// 1. verts
	verts = new V3[vertsN];

	ifs.read(&yn, 1); // cols 3 floats
	
	// 2.colors
	if (colors)
		delete colors;
	colors = 0;
	if (yn == 'y') {
		colors = new V3[vertsN];
	}

	ifs.read(&yn, 1); // normals 3 floats
	
	// 3. normals
	if (normals)
		delete normals;
	normals = 0;
	if (yn == 'y') {
		normals = new V3[vertsN];
	}

	ifs.read(&yn, 1); // texture coordinates 2 floats
	float *tcs = 0; // don't have texture coordinates for now
	if (tcs)
		delete tcs;
	tcs = 0;
	if (yn == 'y') {
		tcs = new float[vertsN * 2];
	}

	ifs.read((char*)verts, vertsN * 3 * sizeof(float)); // load verts

	if (colors) {
		ifs.read((char*)colors, vertsN * 3 * sizeof(float)); // load cols
	}

	if (normals)
		ifs.read((char*)normals, vertsN * 3 * sizeof(float)); // load normals

	if (tcs)
		ifs.read((char*)tcs, vertsN * 2 * sizeof(float)); // load texture coordinates

	// 4. trisN
	ifs.read((char*)&trisN, sizeof(int));
	if (tris)
		delete tris;
	
	// 5. tris
	tris = new unsigned int[trisN * 3];
	ifs.read((char*)tris, trisN * 3 * sizeof(unsigned int)); // read tiangles

	ifs.close();

	cerr << "INFO: loaded " << vertsN << " verts, " << trisN << " tris from " << endl << "      " << fname << endl;
	cerr << "      xyz " << ((colors) ? "rgb " : "") << ((normals) ? "nxnynz " : "") << ((tcs) ? "tcstct " : "") << endl;

	delete[]tcs;

}

AABB TM::GetAABB() {

	if (!verts)
		return AABB(V3(0.0f, 0.0f, 0.0f));

	AABB aabb(verts[0]);
	for (int i = 1; i < vertsN; i++)
		aabb.AddPoint(verts[i]);

	return aabb;

}


V3 TM::GetCenter() {

	V3 tmc(0.0f, 0.0f, 0.0f);
	for (int vi = 0; vi < vertsN; vi++) {

		tmc = tmc + verts[vi];

	}

	tmc = tmc * (1.0f / (float)vertsN);

	return tmc;

}


void TM::PositionAndSize(V3 tmC, float tmSize) {

	AABB aabb = GetAABB();
	V3 oldC = aabb.GetCenter();
	float oldSize = aabb.GetDiagonal();

	Translate(V3(0.0f, 0.0f, 0.0f) - oldC);
	Scale(tmSize / oldSize);
	Translate(tmC);

}


void TM::Translate(V3 tv) {

	for (int vi = 0; vi < vertsN; vi++)
		verts[vi] = verts[vi] + tv;

}


void TM::Scale(float scf) {

	for (int vi = 0; vi < vertsN; vi++)
		verts[vi] = verts[vi]*scf;

}



void TM::Light(V3 mc, V3 L, FrameBuffer *shadowMap) {

	for (int vi = 0; vi < vertsN; vi++) {
		float ka = 0.5f;
		float kd = (L - verts[vi]).UnitVector() * normals[vi].UnitVector();
		kd = (kd < 0.0f) ? 0.0f : kd;
		colors[vi] = mc * (ka + (1.0f - ka)*kd);
	}

}



void TM::RenderQuad(interpolation_t interpolation, PPC * ppc, FrameBuffer * fb, FrameBuffer * texture)
{
	for (int tri = 0; tri < trisN; ++tri)
	{
		int vi0 = tris[tri * 3 + 0];
		int vi1 = tris[tri * 3 + 1];
		int vi2 = tris[tri * 3 + 2];

		Point p0(verts[vi0], colors[vi0], normals[vi0], sta[vi0]);
		Point p1(verts[vi0], colors[vi0], normals[vi0], sta[vi0]);
		Point p2(verts[vi0], colors[vi0], normals[vi0], sta[vi0]);

		fb->Draw3DTriangleWithTexture(interpolation, p0, p1, p2, ppc, texture);
	}
}




void TM::RenderFilled(PPC *ppc, FrameBuffer *fb) {

	if (!scene)
		return;

	V3 *pverts = new V3[vertsN];
	for (int vi = 0; vi < vertsN; vi++) {
		ppc->Project(verts[vi], pverts[vi]);
	}

	for (int tri = 0; tri < trisN; tri++) {
		unsigned int vis[3] = { tris[3 * tri + 0], tris[3 * tri + 1], tris[3 * tri + 2] };
		if (
			pverts[vis[0]][0] == FLT_MAX ||
			pverts[vis[1]][0] == FLT_MAX ||
			pverts[vis[2]][0] == FLT_MAX // implement clipping
			)
			continue;
		AABB aabb(pverts[vis[0]]);
		aabb.AddPoint(pverts[vis[1]]);
		aabb.AddPoint(pverts[vis[2]]);
		if (!aabb.Clip2D(0.0f, (float)fb->w, 0.0f, (float)fb->h))
			continue;
		int lefti = (int)(aabb.c0[0] + 0.5f);
		int righti = (int)(aabb.c1[0] - 0.5f);
		int topi = (int)(aabb.c0[1] + 0.5f);
		int bottomi = (int)(aabb.c1[1] - 0.5f);

		//setup edge equations
		M33 eeqs;
		for (int ei = 0; ei < 3; ei++) {
			V3 v0 = pverts[vis[ei]];
			V3 v1 = pverts[vis[(ei + 1) % 3]];
			V3 v2 = pverts[vis[(ei + 2) % 3]]; v2[2] = 1.0f;
			eeqs[ei][0] = v0[1] - v1[1];
			eeqs[ei][1] = v1[0] - v0[0];
			eeqs[ei][2] = -v0[0] * eeqs[ei][0] - v0[1] * eeqs[ei][1];
			if (eeqs[ei] * v2 < 0.0f)
				eeqs[ei] = eeqs[ei] * -1.0f;
		}

		M33 ssi;
		ssi[0] = pverts[vis[0]];
		ssi[1] = pverts[vis[1]];
		ssi[2] = pverts[vis[2]];
		V3 pvzs = ssi.GetColumn(2);
		ssi.SetColumn(2, V3(1.0f, 1.0f, 1.0f));
		ssi = ssi.Inverted();

		V3 zABC = ssi * pvzs;
		M33 colsm;
		colsm[0] = colors[vis[0]];
		colsm[1] = colors[vis[1]];
		colsm[2] = colors[vis[2]];
		M33 colsABC = ssi * colsm;
		colsABC = colsABC.Transpose();

		M33 normsm;
		normsm[0] = normals[vis[0]];
		normsm[1] = normals[vis[1]];
		normsm[2] = normals[vis[2]];
		M33 normsABC = ssi * normsm;
		normsABC = normsABC.Transpose();


		for (int v = topi; v <= bottomi; v++) {
			for (int u = lefti; u <= righti; u++) {
				V3 pc(.5f + (float)u, .5f + (float)v, 1.0f);
				V3 ss = eeqs * pc;
				if (ss[0] < 0.0f || ss[1] < 0.0f || ss[2] < 0.0f)
					continue;
				float currz = pc * zABC;
				if (!fb->Visible(u, v, currz))
					continue;
				V3 P = ppc->Unproject(V3(pc[0], pc[1], currz));
				V3 eyeRay = ppc->C - P;
				V3 currNorm = normsABC * pc; currNorm = currNorm.UnitVector();
				V3 refRay = currNorm.Reflect(eyeRay);
				V3 currCol = colsABC * pc;
				refRay = refRay.UnitVector();

				float ka = 0.2f;
				V3 lv = (scene->L - P).UnitVector();
				float kd = currNorm * lv;
				kd = (kd < 0.0f) ? 0.0f : kd;
				currCol = currCol * (ka + (1.0f - ka)*kd);

				float ks = lv * refRay;
				if (ks < 0.0f) ks = 0.0f;
				float kse = 200.0f;
				ks = pow(ks, kse);
				currCol = (currCol + V3(1.0f, 1.0f, 1.0f)*ks).Clamp();


				fb->Set(u, v, currCol.GetColor());
			}
		}
	}

}


std::vector<Point> TM::GenBillboard(V3 src)
{
	AABB aabb = this->GetAABB();

	float x_len = aabb.c1[0] - aabb.c0[0];
	float y_len = aabb.c1[1] - aabb.c0[1];
	float z_len = aabb.c1[2] - aabb.c0[2];

	V3 center =  this->GetCenter();

	V3 src_to_bb = center - src;

	src_to_bb = src_to_bb.UnitVector();

	V3 bb_a = src_to_bb ^ V3(0.0f, 1.0f, 0.0f);
	V3 bb_b = src_to_bb ^ bb_a;

	bb_a = bb_a.UnitVector();
	bb_b = bb_b.UnitVector();

	// top left
	V3 corner0 = center - bb_a * x_len * 0.5 - bb_b * y_len * 0.5;
	// top right
	V3 corner1 = center + bb_a * x_len * 0.5 - bb_b * y_len * 0.5;
	// bottom left
	V3 corner2 = center - bb_a * x_len * 0.5 + bb_b * y_len * 0.5;
	// bottom right
	V3 corner3 = center + bb_a * x_len * 0.5 + bb_b * y_len * 0.5;

	V3 zeroVec(0.0f, 0.0f, 0.0f);

	std::vector<Point> points;

	Point p0(corner0, zeroVec, zeroVec, zeroVec);
	Point p1(corner1, zeroVec, zeroVec, zeroVec);
	Point p2(corner2, zeroVec, zeroVec, zeroVec);
	Point p3(corner3, zeroVec, zeroVec, zeroVec);

	points.push_back(p0);
	points.push_back(p1);
	points.push_back(p2);
	points.push_back(p3);

	return points;
	//std::cout << points[3].pos << std::endl;
}


void TM::RayTrace(PPC *ppc, FrameBuffer *fb) {

	for (int v = 0; v < fb->h; v++) {
		fb->DrawRectangle(0, v, fb->w - 1, v, 0xFFFFFFFF);
		for (int u = 0; u < fb->w; u++) {
			V3 ray = ppc->c + ppc->a*((float)u + .5f) +
				ppc->b*((float)v + 0.5f);
			V3 O = ppc->C;
			for (int tri = 0; tri < trisN; tri++) {
				M33 M;
				M.SetColumn(0, verts[tris[3 * tri + 0]]);
				M.SetColumn(1, verts[tris[3 * tri + 1]]);
				M.SetColumn(2, verts[tris[3 * tri + 2]]);
				M = M.Inverted();
				V3 q2 = M * O;
				V3 q3 = M * ray;
				float w = (1 - q2[0] - q2[1] - q2[2]) / (q3[0] + q3[1] + q3[2]);
				V3 abc = q2 + q3 * w;
				if (abc[0] < 0.0f || abc[1] < 0.0f || abc[2] < 0.0f)
					continue;
				if (!fb->Visible(u, v, 1.0f / w))
					continue;
				V3 currCol =
					colors[tris[3 * tri + 0]] * abc[0] +
					colors[tris[3 * tri + 1]] * abc[1] +
					colors[tris[3 * tri + 2]] * abc[2];
				fb->Set(u, v, currCol.GetColor());
			}
		}
		fb->DrawRectangle(0, v + 1, fb->w - 1, v + 1, 0xFF0000FF);
		fb->redraw();
		Fl::check();
	}

}


void TM::RenderHW()
{
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	glVertexPointer(3, GL_FLOAT, 0, (float*)verts);
	glColorPointer(3, GL_FLOAT, 0, (float*)colors);

	glDrawElements(GL_TRIANGLES, 3 * trisN, GL_UNSIGNED_INT, tris);

	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
}

