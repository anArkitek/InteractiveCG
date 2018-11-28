#include "scene.h"
#include "v3.h"
#include "m33.h"

#include <stdlib.h>
#include <algorithm>
#include <ctime>
#include <memory>
#include <string>

Scene *scene;

using namespace std;

#include <fstream>
#include <iostream>

Scene::Scene() {

	// cgi & soi
	cgi = 0;
	soi = 0;

	tms = 0;
	tmsN = 0;

	gui = new GUI();
	gui->show();

	int u0 = 20;
	int v0 = 20;
	int w = 800;
	int h = 600;

	fb = new FrameBuffer(u0, v0, w, h);
	fb->label("SW Framebuffer");
	fb->show();



	///////////////////////////////////
	std::vector<std::string> cubeImgPaths =
	{
		"./resources/cubemap/mytop.tiff",
		"./resources/cubemap/bottom.tiff",
		"./resources/cubemap/myfront.tiff",
		"./resources/cubemap/myback.tiff",
		"./resources/cubemap/myleft.tiff",
		"./resources/cubemap/myright.tiff",
	};

	cubemap = new CubeMap(cubeImgPaths);
	
	V3 testN(1.0f, 0.0f, 1.0f);
	V3 testOut = testN.Reflect(V3(1.0f, 1.0f, 1.0f));
	std::cout << testOut << std::endl;


	////////////////////////////////////


	fb3 = new FrameBuffer(u0 + fb->w + 30, v0, w, h);
	fb3->label("Billboard");
	//fb3->show();

	// Hardware Framebuffer
	hwfb = new FrameBuffer(u0 + fb->w + 600, v0, w, h);
	hwfb->label("HW FrameBuffer");
	hwfb->ishw = true;
	hwfb->show();

	// GPU FrameBuffer
	gpufb = new FrameBuffer(u0, v0, w, h);
	gpufb->label("GPU Framebuffer");
	gpufb->isgpu = true;
	gpufb->show();


	float hfov = 55.0f;
	ppc = new PPC(hfov, fb->w, fb->h);
	ppc3 = new PPC(hfov, fb3->w, fb3->h);

	gui->uiw->position(u0, v0 + fb->h + 60);

	// load $tmsN teapots 
	tmsN = 2;
	tms = new TM[tmsN];
	for (int i = 0; i < tmsN; ++i)
	{
		tms[i].LoadBin("geometry/teapot1K.bin");
	}
	
	// place two teapots
	tmCs = new V3[tmsN];
	tmCs[0] = ppc->C + ppc->GetVD()*200.0f;
	tmCs[1] = tmCs[0] + V3(-100.0f, -100.0f, -100.0f);
	float tmSize = 100.0f;
	tms[0].PositionAndSize(tmCs[0], tmSize);
	tms[1].PositionAndSize(tmCs[1], tmSize);
	
	//?
	ppc3->C = tmCs[0] + V3(50.0f, 50.0f, 50.0f);
	ppc3->PositionAndOrient(ppc3->C, tms[1].GetCenter(), V3(0.0f, 1.0f, 0.0f));
		
	ka = 0.2;

	Render(ppc3, fb3);
	
	Render();

}


void Scene::Render() {

	if (fb)
	{
		Render(ppc, fb);
	}

	if (fb3)
	{
		Render(ppc3, fb3);
		//if (cubemap)
		//{
			//cubemap->ppcs[0]->Visualize(ppc3, fb3, 10.0f);
			//cubemap->ppcs[1]->Visualize(ppc3, fb3, 10.0f);
			//cubemap->ppcs[2]->Visualize(ppc3, fb3, 10.0f);
			//cubemap->ppcs[3]->Visualize(ppc3, fb3, 10.0f);
			//cubemap->ppcs[4]->Visualize(ppc3, fb3, 10.0f);
			//cubemap->ppcs[5]->Visualize(ppc3, fb3, 10.0f);
		//}
			//fb3->Draw3DPoint(L, V3(1.0f, 1.0f, 0.3f), ppc3, 7);
	}
}


void Scene::RenderHW()
{
	glEnable(GL_DEPTH_TEST);

	glClearColor(0.0f, 0.0f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// Set camera parameters
	ppc->SetIntrinsicsHW();
	ppc->SetExtrinsicsHW();

	for (int tmi = 0; tmi < tmsN; tmi++) {
		tms[tmi].RenderHW();
	}
}


void Scene::RenderGPU()
{
	// if the first time, call per session initialization
	if (cgi == NULL) {
		cgi = new CGInterface();
		cgi->PerSessionInit();
		soi = new ShaderOneInterface();
		soi->PerSessionInit(cgi);
	}

	// clear the framebuffer
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.0, 0.0f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// set intrinsics
	ppc->SetIntrinsicsHW();
	// set extrinsics
	ppc->SetExtrinsicsHW();

	// per frame initialization
	cgi->EnableProfiles();
	soi->PerFrameInit();
	soi->BindPrograms();

	// render geometry
	for (int tmi = 0; tmi < tmsN; tmi++) {
		tms[tmi].RenderHW();
	}

	soi->PerFrameDisable();
	cgi->DisableProfiles();
}



void Scene::Render(PPC *currppc, FrameBuffer *currfb)
{
	currfb->SetBGR(0x003F3F3F);
	currfb->ClearZB(0.0f);

	// Billboard
	std::vector<Point> bb_pts = tms[1].GenBillboard(ppc3->C);

	for (int i = 0; i < currfb->h; ++i)
	{
		for (int j = 0; j < currfb->w; ++j)
		{
			V3 direction = currppc->GetRay(j,i);
			currfb->SetGuarded(j,i,cubemap->DirectionLookup(direction).GetColor());
		}
	}

	for (int tmi = 0; tmi < tmsN; tmi++) {
		tms[tmi].RenderMesh(MODEL_SPACE_INTERPOLATION, currppc, currfb, ppc3, fb3, cubemap, nullptr, &bb_pts);
		// tms[tmi].RenderFilled(currppc, currfb);		
	}
	currfb->redraw();
}





void Scene::DBG() 
{

	int stepN = 360;
	{
		for (int stepi = 0; stepi < stepN; ++stepi)
		{
			V3 rotateCenter = tms[0].GetCenter();


			ppc->Translate(V3(1.0f, 1.0f, 1.0f));
			ppc->PositionAndOrient(ppc->C, tms[0].GetCenter(), V3(0.0f, 1.0f, 0.0f));
			Render();
			std::string tiffName = "images//hw_05" + to_string(stepi) + ".tiff";
			fb->SaveAsTiff(tiffName.c_str());
			Fl::check();

		}
		return;
	}


	{
		V3 lightPos = ppc3->C;
		for (int i = 0; i < 360; i++) 
		{
			lightPos = lightPos.RotateThisPointAboutArbitraryAxis(tms[0].GetCenter() , V3(0.0f, 1.0f, 0.0f), (float)(i * 2));
			tms[0].Light(V3(1.0f, 0.0f, 0.0f), L);
			Render();
			Fl::check();
		}
		return;
	}

	for (int stepi = 0; stepi < stepN; ++stepi)
	{
		ppc->Translate(V3(0.0f, 0.0f, 1.0f));

		Render();
		std::string tiffName = "images//hw_03" + to_string(stepi) + ".tiff";
		fb->SaveAsTiff(tiffName.c_str());
		Fl::check();
	}
	return;
	


	{

		V3 pp = V3(-1000.0f, 1000.0f, -10.0f);
		for (int i = 0; i < 100; ++i)
		{
			pp[3] += -10.0f;
			fb->Draw3DPoint(pp, V3(1.0f, 0.0f, 0.0f), ppc, 7);
			Render();
			Fl::check();
		}
		return;
	}

	{
		V3 tc = tms[0].GetCenter();
		for (int i = 0; i < 360; i++) {
			L = tc + V3(40.0f, 0.0f, 0.0f);
			L = L.RotateThisPointAboutArbitraryAxis(tc, V3(0.0f, 1.0f, 0.0f), (float)(i * 2));
			tms[0].Light(V3(1.0f, 0.0f, 0.0f), L);
			Render();
			Fl::check();
		}
		return;
	}


	//{
	//	cerr << ppc->C << "\t" << ppc->a << "\t" << ppc->b << "\t" << ppc->c << "\n";
	//	std::string outputFile = "mydbg/ppc.out";
	//	ppc->WriteText(outputFile);

	//	PPC ppc1(*ppc);

	//	ppc->C = V3(20.0f, 100.0f, 200.0f);
	//	ppc->PositionAndOrient(ppc->C, tms[0].GetCenter(), V3(0.0f, 1.0f, 0.0f));

	//	for (int i = 0; i < 1000; i++) {
	//		ppc1.Pan(.5f);
	//		Render();
	//		ppc1.Visualize(ppc, fb, 20.0f);
	//		Fl::check();
	//	}

	//	// test save and load
	//	ppc->ReadText(outputFile);
	//	cerr << ppc->C << "\t" << ppc->a << "\t" << ppc->b << "\t" << ppc->c << "\n";

	//	Render();
	//	return;
	//}



	{
		// Random Axes
		srand(static_cast<unsigned int>(time(0)));
		std::vector<V3> axis;
		axis.reserve(5);

		for(int triI = 0; triI < tmsN;++triI)
		{
			float x = static_cast<float>(rand() % 10)-5;
			float y = static_cast<float>(rand() % 10)-5;
			float z = static_cast<float>(rand() % 10)-5;
			axis.push_back(V3(x, y, z));
		}

		int fovf = 55.0f;
		PPC* ppc1 = new PPC(fovf, fb->w, fb->h);
		PPC* ppc2 = new PPC(fovf,fb->w, fb->h);
		ppc1->PositionAndOrient(V3(0.0f, 0.0f, 0.0f), tms[0].GetCenter(), V3(0.0f, 1.0, 0.0f));
		ppc2->C = ppc2->C.RotateThisPointAboutArbitraryAxis(tms[0].GetCenter(), V3(0.0f, 1.0, 0.0f), -90.0f);
		ppc2->PositionAndOrient(ppc2->C, tms[0].GetCenter(), V3(0.0f, 1.0, 0.0f));

		int stepN = 150;

		// First 5s
		for (int stepi = 0; stepi < stepN; ++stepi)
		{
			int count = 0;
			for (int triI = 0; triI < tmsN; ++triI)
			{
				tms[triI].RotateAboutArbitraryAxis(tms[triI].GetCenter(), axis[count++], static_cast<float>(rand() % 10));
			}

			Render();
			//std::string tiffName = "images//hw_02" + to_string(stepi) + ".tiff";
			//fb->SaveAsTiff(tiffName.c_str());
			Fl::check();
		}
		

		// Second 5s
		for (int stepi = 0; stepi < stepN; ++stepi)
		{
			ppc->InterpolateTwoCameras(ppc1, ppc2, stepN, stepi);

			int count = 0;
			for (int triI = 0; triI < tmsN; ++triI)
			{
				tms[triI].RotateAboutArbitraryAxis(tms[triI].GetCenter(), axis[count++], static_cast<float>(rand() % 10));
			}

			Render();
			//std::string tiffName = "images//hw_02" + to_string(stepi+150) + ".tiff";
			//fb->SaveAsTiff(tiffName.c_str());
			Fl::check();
		}


		cerr << "Images Finished\n";
		delete ppc1, ppc2;

		return;
	}




	{

		for (int i = 0; i < 30; i++) {
			ppc->Pan(.5f);
			Render();
			Fl::check();
		}
		return;


	}


	{

		V3 tmc = tms[0].GetCenter();
		V3 C1 = ppc->C + V3(120.0f, 80.0f, 0.0f);
		ppc->PositionAndOrient(C1, tmc, V3(0.0f, 1.0f, 0.0));
		Render();
		return;

	}


	{
		TM tm;
		tm.LoadBin("geometry/teapot1K.bin");
		V3 tmC = ppc->C + ppc->GetVD()*100.0f;
		float tmSize = 50.0f;
		tm.PositionAndSize(tmC, tmSize);
		fb->SetBGR(0xFFFFFFFF);
		fb->ClearZB(0.0f);
		tm.RenderWireframe(ppc, fb);
		TM tm2;
//		tm2.SetRectangle(tmC, 50.0f, 50.0f);
//		tm2.RenderWireframe(ppc, fb);
		fb->redraw();

		for (int i = 0; i < 100; i++) {
			fb->SetBGR(0xFFFFFFFF);
			fb->ClearZB(0.0f);
			ppc->ChangeFocalLength(1.05f);
			tm.RenderWireframe(ppc, fb);
			fb->redraw();
			Fl::check();
		}

		return;
	}

	{

		V3 p(20.0f, -20.0f, -80.0f);
		V3 Oa(0.0f, -20.0f, -100.0f), a(0.0f, 1.0f, 0.0f);
		fb->Draw3DSegment(Oa, V3(0.0f, 0.0f, 0.0f), Oa + a * 30.0f, V3(0.0f, 1.0f, 0.0f), ppc);
		for (int i = 0; i < 270; i++) {
			p = p.RotateThisPointAboutArbitraryAxis(Oa, a, 1.0f);
			fb->Draw3DPoint(p, V3(1.0f, 0.0f, 0.0f), ppc, 7);
			fb->redraw();
			Fl::check();
		}
		return;

	}



	{
		TM tm;
		tm.SetRectangle(V3(0.0f, 0.0f, -100.0f), 45.0f, 30.0f);
//		tm.RenderPoints(ppc, fb);
		int stepsN = 360;
		for (int stepsi = 0; stepsi < stepsN; stepsi++) {
			fb->SetBGR(0xFFFFFFFF);
			tm.RotateAboutArbitraryAxis(V3(0.0f, 0.0f, -100.0f), V3(0.0f, 1.0f, 0.0f),
				1.0f);
			tm.RenderWireframe(ppc, fb);
			fb->redraw();
			Fl::check();
		}
		return;

	}

	{

		V3 p0(-20.0f, 0.0f, -100.0f), pp;
		V3 p1(-20.0f, 0.0f, -1000.0f);
		int stepsN = 5;
		for (int stepi = 0; stepi < stepsN; stepi++) {
//			fb->SetBGR(0xFFFFFFFF);
			V3 p = p0 + (p1 - p0) * ((float)stepi / (float)(stepsN - 1));
			fb->Draw3DPoint(p, V3(1.0f, 0.0f, 0.0f), ppc, 7);
			fb->redraw();
			Fl::check();
		}
		return;

	}


	{

		V3 p0(20.1f, 40.0f, 0.0f);
		V3 p1(50.9f, 240.0f, 0.0f);
		V3 c0(1.0f, 0.0f, 0.0f);
		V3 c1(0.0f, 1.0f, 0.0f);
		fb->DrawSegment(p0, c0, p1, c1);
		fb->redraw();
		return;
	}


	{
		V3 v(1.0f, 0.0f, 0.0f);
		for (int i = 0; i < 1000; i++) {
			float scf = (float)i / 999.0f;
			v[1] = scf;
			unsigned int color = v.GetColor();
			V3 v1; v1.SetFromColor(color);
			cerr << v1 - v << "          \r";
			fb->SetBGR(color);
			fb->redraw();
			Fl::check();
		}
		cerr << endl;
		return;
	}

	{
		int u0 = 100;
		int v0 = 50;
		int u1 = 300;
		int v1 = 400;
		int r = 30;
		int stepsN = 100;
		for (int stepi = 0; stepi < stepsN; stepi++) {
			fb->SetBGR(0xFFFFFFFF);
//			fb->DrawRectangle(u0 + stepi, v0, u1 + stepi, v1, 0xFF0000FF);
			fb->DrawCircle(u0 + stepi, v0, r, 0xFF0000FF);
			fb->redraw();
			Fl::check();
		}
		return;
	}

	{
		M33 m;
		m[0] = V3(1.0f, 0.0f, 0.0f);
		m[1] = V3(0.0f, 1.0f, 0.0f);
		m[2] = V3(0.0f, 0.0f, 1.0f);
		M33 m1;
		m1[0] = V3(-1.0f, 2.0f, 3.0f);
		m1[1] = V3(-2.0f, 2.0f, 3.0f);
		m1[2] = V3(-3.0f, -2.0f, 5.0f);

		cerr << m1 << endl << m * m1 << endl << m1 * m << endl;
		return;

		cerr << m << endl;
		V3 v0(1.0f, -2.0f, 0.5f);
		cerr << m * v0;
		cerr << v0;

		return;

	}

	{
		V3 v0(1.0f, -2.0f, 0.5f);
		V3 v1(-1.0f, -4.0f, 3.0f);
		cerr << v0 << v1;
		cerr << "v0v1 = " << v0 * v1 << endl;
		return;
	}

	V3 v(1.0f, 3.0f, -1.0f);
	cerr << v[1] << endl;
	v[1] = 2.0f;
	cerr << v[1] << endl;
	return;

	int u0 = 100;
	int v0 = 50;
	int u1 = 300;
	int v1 = 400;

	fb->DrawRectangle(u0, v0, u1, v1, 0xFF0000FF);
	fb->redraw();

	fb->SaveAsTiff("mydbg/rr.tif");
	
	FrameBuffer *fb1 = new FrameBuffer(30 + fb->w, 30, fb->w, fb->h);
	fb1->LoadTiff("mydbg/rr.tif");
	fb1->label("Loaded image");
	fb1->show();

	return;

	for (int u = 0; u < fb->w; u++) {
		fb->Set(u, fb->h / 2, 0xFF000000);
	}
	fb->redraw();
	cerr << "INFO: pressed DBG" << endl;

}