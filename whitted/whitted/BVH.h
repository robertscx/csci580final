#include "Object.h"
#include "Vector.h"
#include "Boundbox.h"

struct BVHNode;

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
    Material* m;
};

class BVH {
public:
	BVH(std::vector<Object*> p);

	~BVH();

	
};