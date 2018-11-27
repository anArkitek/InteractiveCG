#pragma once

template<typename T>
bool isInclusiveBetween(T testVal, T lower, T upper)
{
	if (testVal >= lower && testVal <= upper)
	{
		return true;
	}
	else
	{
		return false;
	}
}
//
//V3 GetIntersectOfPlaneAndLine(V3 line, V3 plane, float d)
//{
//	float yoverx = line[1] / line[0];
//	float zoverx = line[2] / line[0];
//
//	float x = -d / (plane[0] + plane[1] * yoverx + plane[2] * zoverx);
//	float y = x * yoverx;
//	float z = x * zoverx;
//
//	return V3(x, y, z);
//}