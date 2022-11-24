#pragma once

#include "Vector.h"
#include "global.h"
#include "Boundbox.h"
#include "LightRay.h"

class Object
{
public:
	Object()
		: materialType(DIFFUSE_AND_GLOSSY)
		, ior(1.3)
		, kd(0.8)
		, ks(0.2)
		, diffuseColor(0.2)
		, spew(25)
	{}
	
	virtual ~Object() = default;
	
 	virtual bool intersect(const Ray& ray, float&, uint32_t&, Vector2f&) const = 0; // ���麯��

	virtual Intersection getIntersection(Ray _ray) = 0;

	virtual void getSurfaceNormal(const Vector3f&, const Vector3f&, const uint32_t&, const Vector2f&, Vector3f&,
		Vector2f&) const = 0; // ��ȡ����ķ���

	virtual Vector3f getDiffuseColor(const Vector2f&) const
	{
		return diffuseColor;
	}

	MaterialType materialType;
	float kd, ks;
	Vector3f diffuseColor;
	float spew;
	float ior; // ������

	virtual Boundbox getBounds() = 0;
};
