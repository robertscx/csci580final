#pragma once

#include <vector>
#include "Vector.h"
#include "Object.h"
#include "BVH.h"
#include "Ray.h"

class Scene
{
public:
    int width = 1280;
    int height = 960;
    double fov = 40;
    Vector3f backgroundColor = Vector3f(0.235294, 0.67451, 0.843137);
    int maxDepth = 1; // num of bounce of light
	float RussianRoulette = 0.8;

    // objects and lights in the scene
    std::vector<Object* > objects;
    std::vector<std::unique_ptr<Light> > lights;

	//BVH
	BVH* bvhTree;


    Scene(int w, int h) : width(w), height(h)
    {}

	// add light or object to scene
    void Add(Object* object) { objects.push_back(object); }
    void Add(std::unique_ptr<Light> light) { lights.push_back(std::move(light)); }

	// get lights and objects
	const std::vector<Object*>& getObjects() const { return objects; }
	const std::vector<std::unique_ptr<Light> >& getLights() const { return lights; }

	// intial BVH tree
	void initBVH();

	// if intersect with objects
	Intersection Intersect(const Ray& ray) const;

	Vector3f castRay(const Ray &ray,int depth) const;
	bool Trace(const Ray& ray, const std::vector<Object*>& objects, float& tNear, uint32_t& index, Object** hit_obj);



	void sampleLight(Intersection& inter, float& pdf) const;
	Vector3f shade(Intersection& hit_obj, Vector3f wo) const;
};