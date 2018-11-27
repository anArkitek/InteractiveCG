#include "ppc.h"

#include "m33.h"

#include "framebuffer.h"

PPC::PPC(float hfov, int _w, int _h): a(1.0f, 0.0f, 0.0f), b(0.0f, -1.0f, 0.0f),
	C(0.0f, 0.0f, 0.0f), w(_w), h(_h) {

	float hfovr = hfov * 3.14159f / 180.0f;
	float f = -(float)w / (2.0f *tanf(hfovr / 2.0f));
	c = V3(-(float)w / 2.0f, (float)h / 2.0f, f);

}


void PPC::Translate(V3 vec)
{
	C = C + vec;
}

int PPC::Project(V3 p, V3& pp) {

	M33 M; 
	M.SetColumn(0, a);
	M.SetColumn(1, b);
	M.SetColumn(2, c);
	V3 q = M.Inverted()*(p - C);
	if (q[2] < 0.0f)
		return 0;
	pp[2] = 1.0f / q[2];
	pp[0] = q[0] / q[2];
	pp[1] = q[1] / q[2];
	return 1;

}

V3 PPC::GetVD() {

	return (a ^ b).UnitVector();

}

float PPC::GetFocalLength() {

	return GetVD()*c;

}

void PPC::ChangeFocalLength(float scf) {

	c = c + GetVD()*(GetFocalLength()*(scf - 1.0f));

}


void PPC::PositionAndOrient(V3 C1, V3 L1, V3 vpv) {

	V3 vd1 = (L1 - C1).UnitVector();
	V3 a1 = (vd1 ^ vpv).UnitVector() * a.Length();
	V3 b1 = (vd1 ^ a1).UnitVector() * b.Length();
	V3 c1 = vd1 * GetFocalLength() - b1 * ((float)h / 2.0f) - a1 * ((float)w / 2.0f);

	C = C1;
	a = a1;
	b = b1;
	c = c1;

}


void PPC::Pan(float angled) 
{
	V3 dv = (b * -1.0f).UnitVector();
	a = a.RotateThisVectorAboutDirection(dv, angled);
	c = c.RotateThisVectorAboutDirection(dv, angled);
}

void PPC::Tilt(float angled)
{
	V3 dv = a.UnitVector();
	b = b.RotateThisVectorAboutDirection(dv, angled);
	c = c.RotateThisVectorAboutDirection(dv, angled);
}

void PPC::Roll(float angled)
{
	V3 dv = GetVD();
	a = a.RotateThisVectorAboutDirection(dv, angled);
	b = b.RotateThisVectorAboutDirection(dv, angled);
	c = c.RotateThisVectorAboutDirection(dv, angled);
}

void PPC::InterpolateTwoCameras(PPC* ppc0, PPC* ppc1, int stepsN, int step)
{
	if (step < 0 || step >= stepsN)
	{
		std::cerr << "step cannnot exceed stepsN!" << std::endl;
		return; 
	}

	float fract = static_cast<float>(step) / static_cast<float>(stepsN-1);
	auto ppc0F = ppc0->GetFocalLength();
	auto ppc1F = ppc1->GetFocalLength();
	auto ppc0VD = ppc0->GetVD();
	auto ppc1VD = ppc1->GetVD();
	float fl = ppc0F + (ppc1F - ppc0F) * fract;
	C = ppc0->C + (ppc1->C - ppc0->C) * fract;
	V3 vd = ppc0VD + (ppc1VD - ppc0VD) * fract;
	a = ppc0->a + (ppc1->a - ppc0->a) * fract;
	b = ppc0->b + (ppc1->b - ppc0->b) * fract;
	
	c = vd * fl - a * (static_cast<float>(w) / 2.0f) - b * (static_cast<float>(h) / 2.0f);

}

void PPC::InterpolateTwoCameras(PPC * ppc0, PPC * ppc1, float scf)
{
	V3 ppcV = ppc1->C - ppc0->C;
	V3 ppc0vd = (ppc0->a ^ ppc0->b).UnitVector();
	V3 ppc1vd = (ppc1->a ^ ppc1->b).UnitVector();
	V3 vd = ppc0vd + (ppc1vd - ppc0vd)*scf;
	float ppc0F = ppc0->GetFocalLength();
	float ppc1F = ppc1->GetFocalLength();
	float f = ppc0->GetFocalLength() + (ppc1->GetFocalLength() - ppc0->GetFocalLength())*scf;

	C = ppc0->C + ppcV * scf;
	a = ppc0->a + (ppc1->a - ppc0->a)*scf;
	b = b = ppc0->b + (ppc1->b - ppc0->b) * scf;
	c = vd * f - a * (static_cast<float>(w) / 2.0f) - b * (static_cast<float>(h) / 2.0f);
}


void PPC::Visualize(PPC *ppc3, FrameBuffer *fb3, float vf) {

	V3 colv(0.0f, 0.0f, 0.0f);
	fb3->Draw3DPoint(C, colv, ppc3, 7);

	float scf = vf / GetFocalLength();
	fb3->Draw3DSegment(C, colv, C + c * scf, colv, ppc3);
	fb3->Draw3DSegment(C + c * scf, colv, C + (c + a * (float)w)*scf, colv, ppc3);
	fb3->Draw3DSegment(C + (c + a * (float)w)*scf, colv,
		C + (c + a * (float)w + b * (float)h)*scf, colv, ppc3);
	fb3->Draw3DSegment(
		C + (c + a * (float)w + b * (float)h)*scf, colv,
		C + (c +  b * (float)h)*scf, colv,
		ppc3);
	fb3->Draw3DSegment(
		C + (c + b * (float)h)*scf, colv,
		C+c*scf, colv,
		ppc3);

}




void PPC::WriteTxt(std::string fname)
{
	std::ofstream outfile;
	outfile.open(fname);
	if (outfile.is_open())
	{
		outfile << C << "\n";
		outfile << a << "\n";
		outfile << b << "\n";
		outfile << c << "\n";
	}
	else
	{
		std::cerr << "Unable to open " << fname << std::endl;
	}
	outfile.close();
}


void PPC::ReadTxt(std::string fname)
{
	std::string line;
	ifstream infile(fname);
	if (infile.is_open())
	{
		infile >> C;
		infile >> a;
		infile >> b;
		infile >> c;
	}
	else
	{
		std::cerr << "Unable to open " << fname << std::endl;
	}
	infile.close();
}


V3 PPC::UnprojectPixel(float uf, float vf, float curf)
{
	return C + (a * uf + b * vf + c) * curf * (1.0 / GetFocalLength());
}

V3 PPC::Unproject(V3 pP) {

	return C + (a*pP[0] + b * pP[1] + c) * (1.0f / pP[2]);

}

V3 PPC::GetRay(int u, int v)
{
	return a * ((float)u + 0.5f) + b * ((float)v + 0.5f) + c;
}
