//
// Created by LEI XU on 5/16/19.
//

#ifndef RAYTRACING_INTERSECTION_H
#define RAYTRACING_INTERSECTION_H
#include "Vector.hpp"
#include "Material.hpp"
class Object;
class Sphere;

struct Intersection
{
    Intersection(){
        happened=false;
        coords=Vector3f();
        normal=Vector3f();
        distance= std::numeric_limits<double>::max();
        obj =nullptr;
        m=nullptr;
    }
    bool happened; // if intersected
    Vector3f coords; // the coord of intersecion
    Vector3f normal; // intersection suface normal
    Vector3f emit; //todo: add new: emission term
    double distance; // light distance from origin to the intersection 
    Object* obj; // which obj intersected
    Material* m; // material of intersection surface
};
#endif //RAYTRACING_INTERSECTION_H
