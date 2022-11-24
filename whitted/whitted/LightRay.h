#pragma once

#include "Vector.h"

class LightRay
{
public:
	LightRay(const Vector3f& orgin, const Vector3f& i) 
		: origin(orgin), intensity(i)
	{}

	
	virtual ~LightRay() = default;

	Vector3f origin;
	Vector3f intensity;
};

class Ray {
	Vector3f origin;
	Vector3f direction, direction_inv;
	double t;
	double t_min, t_max;

	Ray(const Vector3f& ori, const Vector3f& dir, const double _t = 0.0) :
		origin(ori), direction(dir), t(_t) 
	{
		direction_inv = Vector3f(1.0f / direction.x, 1.0f / direction.y, 1.0f / direction.z );
		t_min = 0.0;
		t_max = std::numeric_limits<double>::max();
	}

	Vector3f operator()(double t) const { return origin + direction * t; }

	friend std::ostream& operator<<(std::ostream& os, const Ray& r) {
		os << "[origin:=" << r.origin << ", direction=" << r.direction << ", time=" << r.t << "]\n";
		return os;
	}
};