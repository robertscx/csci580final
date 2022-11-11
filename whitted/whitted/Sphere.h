#pragma once

#include "Object.h"
#include "Vector.h"

class Sphere : public Object
{
public:
	
	Sphere(const Vector3f& c, const float& r)
		: center(c)
		, radius(r)
		, radius_sqrt(r * r)
	{}

    bool intersect(const Vector3f& origin, const Vector3f& dir, float& tnear, uint32_t&, Vector2f&) const override
    {
		// �ж��Ƿ�������ཻ�������Դλ�ã�����tnear�Ƿ��̽�С���Ǹ��⣬���ǽ���������t
		Vector3f L = origin - center;
		float a = dotProduct(dir, dir);
		float b = 2 * dotProduct(dir, L);
		float c = dotProduct(L, L) - radius_sqrt;
		float t0, t1;
		if (!solveQuadratic(a, b, c, t0, t1))
			return false;
		if (t0 < 0)
			t0 = t1;
		if (t0 < 0)
			return false;
		tnear = t0;

        return true;
    }

	/*
	getSurfaceNormal���hit point �����norm����
	*/

	void getSurfaceNormal(const Vector3f& p, const Vector3f&, const uint32_t&, const Vector2f&, Vector3f& N,Vector2f&) const override
	{
		N = normalize(p -  center);
	}


	Vector3f center;
	float radius, radius_sqrt; // ���� rƽ�������ټ���
};