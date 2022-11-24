#pragma once

#include "Object.h"
#include "Vector.h"
#include "BVH.h"

class Sphere : public Object
{
public:
	
	Sphere(const Vector3f& c, const float& r)
		: center(c)
		, radius(r)
		, radius_sqrt(r * r)
	{}

    bool intersect(const Ray& ray, float& tnear, uint32_t&, Vector2f&) const override
    {
		// 判断是否与光线相交，传入光源位置，方向，tnear是方程较小的那个解，就是进入表面的是t
		Vector3f L = ray.origin - center;
		float a = dotProduct(ray.direction, ray.direction);
		float b = 2 * dotProduct(ray.direction, L);
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

	Intersection getIntersection(Ray ray) {
		Intersection result;
		result.happened = false;
		Vector3f L = ray.origin - center;
		float a = dotProduct(ray.direction, ray.direction);
		float b = 2 * dotProduct(ray.direction, L);
		float c = dotProduct(L, L) - radius2;
		float t0, t1;
		if (!solveQuadratic(a, b, c, t0, t1)) return result;
		if (t0 < 0) t0 = t1;
		if (t0 < 0) return result;
		result.happened = true;

		result.coords = Vector3f(ray.origin + ray.direction * t0);
		result.normal = normalize(Vector3f(result.coords - center));
		result.m = this->m;
		result.obj = this;
		result.distance = t0;
		return result;
	}

	/*
	getSurfaceNormal获得hit point 表面的norm方向
	*/

	void getSurfaceNormal(const Vector3f& p, const Vector3f&, const uint32_t&, const Vector2f&, Vector3f& N,Vector2f&) const override
	{
		N = normalize(p -  center);
	}

	Boundbox getBounds() override {
		return Boundbox(Vector3f(center.x - radius, center.y - radius, center.z - radius),
						Vector3f(center.x + radius, center.y + radius, center.z - radius));
	}


	Vector3f center;
	float radius, radius_sqrt; // 保存 r平方，减少计算
};