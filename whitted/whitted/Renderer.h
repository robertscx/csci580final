#pragma once
#include "Scene.h"

struct hit_payload
{
    float tNear;
    uint32_t index;
    Vector2f bbs;
    Object* hit_obj;
};

class Renderer
{
public:
    void Render(const Scene& scene);

private:
};