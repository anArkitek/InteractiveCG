#include "v3.h"

#include "m33.h"

V3::V3(float x, float y, float z) {

	xyz[0] = x;
	xyz[1] = y;
	xyz[2] = z;

}


V3::V3(float x)
{
	xyz[0] = x;
	xyz[1] = x;
	xyz[2] = x;
}

float& V3::operator[](int i) {

	return xyz[i];

}

float V3::operator*(V3 v1) {

	V3 &v0 = *this;
	return v0[0] * v1[0] + v0[1] * v1[1] + v0[2] * v1[2];

}

ostream& operator<<(ostream& ostr, V3 v) {

	return ostr << v[0] << " " << v[1] << " " << v[2] << endl;

}

istream& operator>>(istream& ist, V3 &v)
{
	return ist >> v[0] >> v[1] >> v[2];
}

V3 V3::operator-(V3 v1) {

	V3 &v0 = *this;
	return V3(v0[0] - v1[0], v0[1] - v1[1], v0[2] - v1[2]);

}

V3 V3::operator+(V3 v1) {

	V3 &v0 = *this;
	return V3(v0[0] + v1[0], v0[1] + v1[1], v0[2] + v1[2]);

}

V3 V3::operator*(float scf) {

	V3 &v0 = *this;
	return V3(v0[0]*scf, v0[1]*scf, v0[2]*scf);

}

V3 V3::operator/(float scf)
{
	V3 &v0 = *this;
	return v0 * (1.0f / scf);
}

void V3::SetFromColor(unsigned int color) {

	unsigned char *rgb = (unsigned char*)&color;
	V3 &v = *this;
	v[0] = ((float)rgb[0]) / 255.0f;
	v[1] = ((float)rgb[1]) / 255.0f;
	v[2] = ((float)rgb[2]) / 255.0f;

}

unsigned int V3::GetColor() {

	V3 &v = *this;
	unsigned int ret;
	unsigned char *rgb = (unsigned char*) (&ret);
	for (int i = 0; i < 3; i++) {
		if (v[i] < 0.0f)
			rgb[i] = 0;
		else if (v[i] > 1.0f)
			rgb[i] = 255;
		else
			rgb[i] = (unsigned char) (255.0f*v[i]);
	}
	rgb[3] = 255;
	return ret;
}


float V3::Length() {

	V3 &v = *this;
	return sqrtf(v*v);

}

V3 V3::operator^(V3 v2) {

	V3 &v1 = *this;
	V3 ret(v1[1] * v2[2] - v1[2] * v2[1],
		v1[2] * v2[0] - v1[0] * v2[2],
		v1[0] * v2[1] - v1[1] * v2[0]);
	return ret;
}

V3 V3::RotateThisPointAboutArbitraryAxis(V3 O, V3 a, float angled) {

	// find auxiliary axis
	V3 aux;
	if (fabsf(a[0]) > fabsf(a[1])) {
		aux = V3(0.0f, 1.0f, 0.0f);
	}
	else {
		aux = V3(1.0f, 0.0f, 0.0f);
	}

	V3 a0 = (aux ^ a).UnitVector();
	V3 a2 = (a0 ^ a).UnitVector();
	M33 lcs;
	lcs[0] = a0;
	lcs[1] = a;
	lcs[2] = a2;

	V3 &p = *this;
	// change to local coordinate system O, a0, a, a2
	V3 p1 = lcs * (p - O);
	// rotate about "Second axis" in local coordinate system;
	M33 mr; mr.SetRotationAboutY(angled);
	V3 p2 = mr * p1;
	V3 p3 = lcs.Inverted()*p2 + O;
	return p3;

}

V3 V3::RotateThisVectorAboutDirection(V3 a, float angled) {

	// find auxiliary axis
	V3 aux;
	if (fabsf(a[0]) > fabsf(a[1])) {
		aux = V3(0.0f, 1.0f, 0.0f);
	}
	else {
		aux = V3(1.0f, 0.0f, 0.0f);
	}

	V3 a0 = (aux ^ a).UnitVector();
	V3 a2 = (a0 ^ a).UnitVector();
	M33 lcs;
	lcs[0] = a0;
	lcs[1] = a;
	lcs[2] = a2;

	V3 &p = *this;
	// change to local coordinate system O, a0, a, a2
	V3 p1 = lcs * p;
	// rotate about "Second axis" in local coordinate system;
	M33 mr; mr.SetRotationAboutY(angled);
	V3 p2 = mr * p1;
	V3 p3 = lcs.Inverted()*p2;
	return p3;

}


V3 V3::UnitVector() {

	return (*this) * (1.0f / Length());

}


V3 V3::Reflect(V3 r)
{
	V3 n = *this;
	n = n.UnitVector();

	V3 rn = n * (r * n);
	V3 rr = rn * 2.0f - r;

	return rr;
}


V3 V3::Refract(V3 r)
{
	V3 n = *this;
	V3 unit_n = n.UnitVector();

	float rayLen = r.Length();

	V3 unit_r = r.UnitVector();

	float cos_in = unit_r * unit_n;
	float sin_in = sqrtf(1 - cos_in * cos_in);

	V3 para_n = unit_n * cos_in;
	V3 vert_n = unit_r - para_n;

	float sin_out = sin_in / 1.1;
	float cos_out = sqrtf(1 - sin_out * sin_out);
	float tan_out = sin_out / cos_out;

	V3 para_n_out = ( V3(0.0f, 0.0f, 0.0f) - para_n );
	V3 vert_n_out = ( V3(0.0f, 0.0f, 0.0f) - vert_n );

	para_n_out = para_n_out.UnitVector();
	vert_n_out = vert_n_out.UnitVector() * tan_out;

	V3 rr = (para_n_out + vert_n_out).UnitVector();
	
	return rr;
}




float V3::GetSum()
{
	return xyz[0] + xyz[1] + xyz[2];
}


V3 V3::Clamp()
{

	V3 v = *this;
	for (int i = 0; i < 3; ++i)
	{
		if (v[i] < 0.0f)
		{
			v[i] = 0.0f;
		}
		if (v[i] > 1.0f)
		{
			v[i] = 1.0f;
		}
	}

	return v;
}