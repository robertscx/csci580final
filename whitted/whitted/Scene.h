#pragma once

#include <vector>
#include <memory>
#include "Vector.h"
#include "Object.h"
#include "LightRay.h"

class Scene
{
public:
	// 场景设置
	int width = 1280;
	int height = 960;
	double fov = 90;
	Vector3f backgroundColor = Vector3f(0.235294, 0.67451, 0.843137);
	int maxDepth = 5;
	float epsilon = 0.00001;

	//    float epsilon = 0.00001;

	Scene(int w, int h) : width(w), height(h)
	{}

	// 向场景中添加 object 和light
	void Add(std::unique_ptr<Object> object) { objects.push_back(std::move(object)); }
	void Add(std::unique_ptr<LightRay> light) { lights.push_back(std::move(light)); }

	// 获取场景中的objects和lights

	[[nodiscard]] const std::vector<std::unique_ptr<Object>>& getObjects() const { return objects; }
	[[nodiscard]] const std::vector<std::unique_ptr<LightRay>>& getLights() const { return lights; }

private:
	std::vector<std::unique_ptr<Object>> objects; // 场景内的物体
	std::vector<std::unique_ptr<LightRay>> lights; // 场景内的光源
};