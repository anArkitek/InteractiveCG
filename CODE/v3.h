#pragma once

#include <iostream>

using namespace std;


class V3 {
public:
	float xyz[3];
	float& operator[](int i);
	float operator*(V3 v1);
	V3 operator-(V3 v1);
	V3 operator+(V3 v1);
	V3 operator*(float scf);
	V3 operator/(float scf);
	V3 operator^(V3 v2);
	friend ostream& operator<<(ostream& ostr, V3 v);
	friend istream& operator>>(istream& ist, V3 &v);
	void SetFromColor(unsigned int color);
	unsigned int GetColor();
	float Length();
	V3() {};
	V3(float x, float y, float z);
	V3(float x);
	V3 RotateThisPointAboutArbitraryAxis(V3 O, V3 a, float angled);
	V3 RotateThisVectorAboutDirection(V3 a, float angled);
	V3 UnitVector();
	
	// the object itself should be a normal, take a input ray and reflect
	// @ r: input ray
	// @ return: the reflected ray
	V3 Reflect(V3 r);
	
	V3 Refract(V3 r);


	// return the sum of the elements in this vector
	float GetSum();

	V3 Clamp();
};