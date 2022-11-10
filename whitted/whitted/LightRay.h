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