#include "Scene.h"

// initial BVH tree with objects in scene
void Scene::initBVH()
{
	this->bvhTree = new BVH(objects,1);
}

void Scene::sampleLight(Intersection& inter,float& pdf) const
{
	float sum = 0;
	//todo change?
	for (uint32_t i = 0; i < objects.size(); i++) {
		// find light sources in the scene, sum their area.
		if (objects[i]->isLightSource()) {
			sum += objects[i]->getArea();
		}
	}

	// get a random area from range [0, sum_area]
	// traverse all light sources, sum their area again, when sum > random area
	// render object with that light source
	float rad_A = get_random_float() * sum;
	sum = 0;

	//todo change?
	for (uint32_t i = 0; i < objects.size(); i++) {
		if (objects[i]->isLightSource()) {
			sum += objects[i]->getArea();
			if (rad_A <= sum) {
				objects[i]->Sample(inter, pdf);
				break;
			}
		}
	}
}

// traverse BVH find if rayintersect with object
Intersection Scene::Intersect(const Ray& ray) const
{
	return this->bvhTree->Intersect(ray);
}

bool Scene::Trace(const Ray& ray, const std::vector<Object*>& objects, 
	float& tNear, uint32_t& index, Object** hit_obj)
{
	bool isIntersect = false;
	*hit_obj = nullptr;

	for (auto object : objects) {
		float nearest = floatMax;
		uint32_t index_tri;
		Vector2f bbs_tri;

		if (object->Intersect(ray, nearest, index_tri) && nearest < tNear) {
			*hit_obj = object;
			tNear = nearest;
			index = index_tri;
			isIntersect = true;
		}
	} 

	return isIntersect;
}

Vector3f Scene::shade(Intersection& hit_obj, Vector3f wo) const
{
    if (hit_obj.m->isLightSource())
    {
        return hit_obj.m->getEmission();
    }
    const float epsilon = 0.0005f;
    // 直接光照贡献
    Vector3f Lo_dir;
    {
        float light_pdf;
        Intersection hit_light;
        sampleLight(hit_light, light_pdf);
        Vector3f obj2Light = hit_light.pos - hit_obj.pos;
        Vector3f obj2LightDir = obj2Light.normalized();

        // 检查光线是否被遮挡
        auto t = Intersect(Ray(hit_obj.pos, obj2LightDir));
        if (t.dist - obj2Light.norm() > -epsilon)
        {
            Vector3f f_r = hit_obj.m->brdf(obj2LightDir, wo, hit_obj.normal);
            float r2 = dotProduct(obj2Light, obj2Light);
            float cosA = std::max(.0f, dotProduct(hit_obj.normal, obj2LightDir));
            float cosB = std::max(.0f, dotProduct(hit_light.normal, -obj2LightDir));
            Lo_dir = hit_light.emit * f_r * cosA * cosB / r2 / light_pdf;
        }
    }

    // 间接光照贡献
    Vector3f Lo_indir;
    {
        if (get_random_float() < RussianRoulette)
        {
            Vector3f dir2NextObj = hit_obj.m->sample(wo, hit_obj.normal).normalized();
            float pdf = hit_obj.m->pdf(wo, dir2NextObj, hit_obj.normal);
            if (pdf > epsilon)
            {
                Intersection nextObj = Intersect(Ray(hit_obj.pos, dir2NextObj));
                if (nextObj.isMeet && !nextObj.m->isLightSource())
                {
                    Vector3f f_r = hit_obj.m->brdf(dir2NextObj, wo, hit_obj.normal); //BRDF
                    float cos = std::max(.0f, dotProduct(dir2NextObj, hit_obj.normal));
                    Lo_indir = shade(nextObj, -dir2NextObj) * f_r * cos / pdf / RussianRoulette;
                }
            }
        }
    }

    return Lo_dir + Lo_indir;
}


// Implementation of Path Tracing
Vector3f Scene::castRay(const Ray& ray, int depth) const
{
	auto hitObj = Intersect(ray);
	if (!hitObj.isMeet) return {}; // if intersect with object
	return shade(hitObj, -ray.dir);
}

//Vector3f Scene::castRay(const Ray& ray, int depth) const
//{
//	if (depth > this->maxDepth) return Vector3f(0.0, 0.0, 0.0);
//
//	// get intersection (ray & objects)
//	Vector3f hitColor = this->backgroundColor;
//	Intersection inter = Scene::Intersect(ray);
//
//	Vector2f bbs;
//	uint32_t index = 0;
//
//	if (inter.isMeet) {
//		Vector3f hitPoint = inter.pos;
//		Vector3f N = inter.normal;
//		Vector2f uv;
//		inter.obj->getNormal(hitPoint, ray.dir, index, bbs, N, uv);
//
//		// detemine the way of bounce for light based on material 
//		switch (inter.m->getType())
//		{
//		case REFLECTION_AND_REFRACTION: {
//
//			// reflect and refract direction
//			Vector3f dir_reflection = normalize(reflect(ray.dir, N));
//			Vector3f dir_refraction = normalize(refract(ray.dir, N, inter.m->ior));
//
//
//			Vector3f origin_reflection = (dotProduct(dir_reflection, N) < 0) ?
//				hitPoint - N * EPSILON :
//				hitPoint + N * EPSILON;
//			Vector3f origin_refraction = (dotProduct(dir_refraction, N) < 0) ?
//				hitPoint - N * EPSILON :
//				hitPoint + N * EPSILON;
//
//			Vector3f hitColor_reflect = castRay(Ray(origin_reflection, dir_reflection), depth + 1);
//			Vector3f hitColor_refract = castRay(Ray(origin_refraction, dir_refraction), depth + 1);
//			float kr = fresnel(ray.dir, N, inter.m->ior);
//			hitColor = hitColor_reflect * kr + hitColor_refract * (1 - kr);
//			break;
//		}
//		case REFLECTION: {
//			float kr = fresnel(ray.dir, N, inter.m->ior);
//			Vector3f dir_reflection = normalize(reflect(ray.dir, N));
//			Vector3f origin_reflection = (dotProduct(dir_reflection, N) < 0) ?
//				hitPoint + N * EPSILON :
//				hitPoint - N * EPSILON;
//			hitColor = castRay(Ray(origin_reflection, dir_reflection), depth + 1) * kr;
//			break;
//		}
//		default:
//			// we use Phong shading in the default case (DIFFUSE_AND_GLOSSY)
//			// composed of a diffuse and a specular reflection component. no La light in this case
//			Vector3f amt = 0, spec = 0;
//			Vector3f origin_shadow = (dotProduct(ray.dir, N) < 0) ?
//				hitPoint + N * EPSILON :
//				hitPoint - N * EPSILON;
//
//			for (uint32_t i = 0; i < getLights().size(); ++i) {
//				Vector3f lightDir = getLights()[i]->position - hitPoint;
//
//				float distance_sqrt = dotProduct(lightDir, lightDir);
//				lightDir = normalize(lightDir);
//				float LdotN = std::max(0.f, dotProduct(lightDir, N));
//
//				bool isInShaodw;
//				isInShaodw = bvhTree->Intersect(Ray(origin_shadow, lightDir)).isMeet;
//				amt += isInShaodw ? 0 : getLights()[i]->intensity * LdotN;
//
//				Vector3f dir_reflect = normalize(reflect(-lightDir, N));
//
//				spec += powf(std::max(0.f, -dotProduct(dir_reflect, ray.dir)), inter.m->spw) * getLights()[i]->intensity;
//			}
//
//			hitColor = amt * inter.obj->getDiffuseColor(uv) * inter.m->Kd + spec * inter.m->Ks;
//			break;
//		}
//	}
//
//	return hitColor;
//}








