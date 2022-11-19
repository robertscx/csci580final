#include <algorithm>
#include <cassert>
#include "BVH.h"

BVH::BVH(std::vector<Object*> p, int maxPrimsInNode)
	: maxPrimsInNode(std::min(255, maxPrimsInNode)), primitives(std::move(p))
{
	if (primitives.empty()) {
		return;
	}

	root = recursiveBuild(std::vector<Object*> objects);
}

BVHNode* BVH::recursiveBuild(std::vector<Object*> objects) {
	BVHNode* node = new BVHNode();

	Boundbox bounds;

	for (int i = 0; i < objects.size(); i++) {
		bounds = Union(bounds, objects[i]->getBounds());
	}

	if (objects.size() == 1) {
		node->bounds = objects[0]->getBounds();
		node->object = objects[0];
		node->left = nullptr;
		node->right = nullptr;
		return node;
	}
	else if (objects.size() == 2) {
		node->left = recursiveBuild(std::vector{ objects[0] });
		node->right = recursiveBuild(std::vector{ objects[1] });
		node->bounds = Union(node->left->bounds, node->right->bounds);
		return node;
	}
	else {
		Boundbox centroidBounds;
		for (int i = 0; i < objects.size(); i++) {
			centroidBounds = Union(centroidBounds, objects[i]->getBounds().Centroid());
		}

		int axis = centroidBounds.longestEdge();
		switch (axis) {
		case 0:
			std::sort(objects.begin(), objects.end(), [](auto f1, auto f2) {
				return f1->getBounds().Centroid().x < f2->getBounds().Centroid().x;
				});
			break;
		case 1:
			std::sort(objects.begin(), objects.end(), [](auto f1, auto f2) {
				return f1->getBounds().Centroid().y < f2->getBounds().Centroid().y;
				});
			break;
		case 2:
			std::sort(objects.begin(), objects.end(), [](auto f1, auto f2) {
				return f1->getBounds().Centroid().z < f2->getBounds().Centroid().z;
				});
			break;
		}

		auto first = objects.begin();
		auto middle = objects.begin() + (objects.size() / 2);
		auto last = objects.end();

		auto leftObjects = std::vector<Object*>(first, middle);
		auto rightObjects = std::vector<Object*>(middle, last);

		assert(objects.size() == (leftObjects.size() + rightObjects.size()));

		node->left = recursiveBuild(leftObjects);
		node->right = recursiveBuild(rightObjects);

		node->bounds = Union(node->left->bounds, node->right->bounds);
	}

	return node;
}