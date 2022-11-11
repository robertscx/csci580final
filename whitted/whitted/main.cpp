#include "Scene.h"
#include "Sphere.h"
#include "Triangle.h"
#include "LightRay.h"
#include "Renderer.h"


int main()
{
    // scene setting
    Scene scene(1280,960);

    // add spheres
    auto sph1 = std::make_unique<Sphere>(Vector3f(-1, 0, -12), 2);
    sph1->materialType = DIFFUSE_AND_GLOSSY;
    sph1->diffuseColor = Vector3f(0.6, 0.7, 0.8);

    auto sph2 = std::make_unique<Sphere>(Vector3f(0.5, -0.5, -8), 1.5);
    sph2->ior = 1.5;
    sph2->materialType = REFLECTION_AND_REFRACTION;

    scene.Add(std::move(sph1));
    scene.Add(std::move(sph2));

    // add object with triangle mesh
    // - verts vertices position
    // - verIndex vertices index of triangle
    // - uv coordinate 
    Vector3f verts[4] = { {-5,-3,-6}, {5,-3,-6}, {5,-3,-16}, {-5,-3,-16} };
    uint32_t vertIndex[6] = { 0, 1, 3, 1, 2, 3 };
    Vector2f uv[4] = { {0, 0}, {1, 0}, {1, 1}, {0, 1} };
    auto mesh = std::make_unique<MeshTriangle>(verts, vertIndex, 2, uv);
    mesh->materialType = DIFFUSE_AND_GLOSSY;

    scene.Add(std::move(mesh));
    scene.Add(std::make_unique<LightRay>(Vector3f(-20, 70, 20), 0.5));
    scene.Add(std::make_unique<LightRay>(Vector3f(30, 50, -12), 0.5));

    Renderer r;
    r.Render(scene);

    return 0;
}

