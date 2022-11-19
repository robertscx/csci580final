#pragma once

#include "Object.h"
#include "Vector.h"
#include "Boundbox.h"

struct BVHNode {
    Boundbox bounds;
    BVHNode* left;
    BVHNode* right;
    Object* object;

public:
    int splitAxis = 0, firstPrimOffset = 0, nPrimitives = 0;
    BVHNode() {
        bounds = Boundbox();
        left = nullptr;
        right = nullptr;
        object = nullptr;
    }
};

struct Intersection
{
    Intersection() {
        happened = false;
        coords = Vector3f();
        normal = Vector3f();
        distance = std::numeric_limits<double>::max();
        obj = nullptr;
        m = nullptr;
    }
    bool happened;
    Vector3f coords;
    Vector3f normal;
    double distance;
    Object* obj;
    //Material* m;
};

class BVH {
public:
    const int maxPrimsInNode;
    std::vector<Object*> primitives;

    BVHNode* root;

	BVH(std::vector<Object*> p, int maxPrimsInNode = 1);

	~BVH();

    BVHNode* recursiveBuild(std::vector<Object*> objects);
};