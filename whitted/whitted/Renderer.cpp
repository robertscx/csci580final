#include <fstream>
#include "Vector.h"
#include "Renderer.h"
#include "Scene.h"
#include <optional>


// convert degree to radius
inline float degree2Radius(const float &degree) {
	return degree * M_PI / 180;
}

// 计算反射光方向
Vector3f reflect(const Vector3f& I, const Vector3f& N) {
	return I - 2 * dotProduct(I, N) * N;
}

// [comment]
// computer refract direction with Snell`s law
// We need to handle with care the two possible situations:
//  - when ray inside obejct
//  - when ray outside object
// when inside, make cosi positive 
// when outside, invert refractive indices and negate N
// [/comment]
Vector3f refract(const Vector3f& I, const Vector3f& N, const float& ior) 
{
	float cosi = clamp(-1, 1, dotProduct(I, N));
	float incident_rate = 1, exit_rate = ior;
	Vector3f n = N;
	if (cosi < 0) {
		cosi = -cosi;
	}
	else
	{
		std::swap(incident_rate,exit_rate);
		n = -N;
	}

	float tmp = incident_rate / exit_rate;
	float k = 1 - tmp * tmp * (1 - cosi * cosi);
	return k < 0 ? 0 : tmp * I + (tmp * cosi - sqrtf(k)) * n;
}

// [comment]
// 菲涅尔项计算能量分布在反射和折射的比例，kr= 折射的能量 / 光能， 反射能量= 1- kr
// I incident view direction
// N normal at the hit point
// ior refractive rate of material
// [/comment]
float fresnel(const Vector3f& I, const Vector3f& N, const float& ior) {
	float cosi = clamp(-1, 1, dotProduct(I, N));
	float int_rate = 1, ext_rate = ior;
	if (cosi > 0) std::swap(int_rate, ext_rate);

	// calculate sint using Snell`s law
	float sint = (int_rate / ext_rate) * sqrtf(std::max(0.f, 1 - cosi * cosi));

	if (sint >= 1) return 1;
	else
	{
		float cost = sqrtf(std::max(0.f, 1 - sint * sint));
		cosi = fabsf(cosi);
		float Rs = ((ext_rate * cosi) - (int_rate * cost)) / ((ext_rate * cosi) + (int_rate * cost));
		float Rp = ((int_rate * cosi) - (ext_rate * cost)) / ((int_rate * cosi) + (ext_rate * cost));
		return (Rs * Rs + Rp * Rp) / 2;
	}
}
// [comment]
// Returns true if the ray intersects an object, false otherwise.
//
// \param orig is the ray origin
// \param dir is the ray direction
// \param objects is the list of objects the scene contains
// \param[out] tNear contains the distance to the cloesest intersected object.
// \param[out] index stores the index of the intersect triangle if the interesected object is a mesh.
// \param[out] bbs stores the b1 and b2 barycentric coordinates of the intersected point
// \param[out] *hitObject stores the pointer to the intersected object (used to retrieve material information, etc.)
// \param isShadowRay is it a shadow ray. We can return from the function sooner as soon as we have found a hit.
// [/comment]
std::optional<hit_payload> trace(
	const Vector3f& origin, const Vector3f &dir, 
	const std::vector<std::unique_ptr<Object>>& objects) 
{
	float tNear = MAXFLOAT;
	std::optional<hit_payload> payload;

	for (const auto& object : objects) {
		float tNear_ray = MAXFLOAT;
		uint32_t index_ray;
		Vector2f bbs_ray;

		// 找到最近相交的物体
		if (object->intersect(origin, dir, tNear_ray, index_ray, bbs_ray) && tNear_ray < tNear) {
			payload.emplace();
			payload->hit_obj = object.get();
			payload->tNear = tNear_ray;
			payload->index = index_ray;
			payload->bbs = bbs_ray;
			tNear = tNear_ray;
		}
	}

	return payload;
}

// [comment]
// Implementation of the Whitted-style light transport algorithm
// depth is the maxNum of light reflection or refraction. 最多折射或反射五次
// [/comment]

Vector3f castRay(const Vector3f& origin,const Vector3f& dir,const Scene& scene, int depth)
{
	if (depth > scene.maxDepth) return Vector3f(0.0, 0.0, 0.0);

	Vector3f hitColor = scene.backgroundColor;
	if (auto payload = trace(origin, dir, scene.getObjects()); payload) {
		Vector3f hitPoint = origin + dir * payload->tNear;
		Vector3f N;
		Vector2f uv;
		payload->hit_obj->getSurfaceNormal(hitPoint, dir, payload->index, payload->bbs, N, uv);
		switch (payload->hit_obj->materialType)
		{
		case REFLECTION_AND_REFRACTION: {
			Vector3f dir_reflection = normalize(reflect(dir, N));
			Vector3f dir_refraction = normalize(refract(dir, N,payload->hit_obj->ior));

			// 对 origin 做出一定的偏移，因为迭代要计算下一次与物体相交，不能把光的起始点放在hitpoint上。
			Vector3f origin_reflection = (dotProduct(dir_reflection, N) < 0) ?
				hitPoint - N * scene.epsilon :
				hitPoint + N * scene.epsilon;
			Vector3f origin_refraction = (dotProduct(dir_refraction, N) < 0) ?
				hitPoint - N * scene.epsilon :
				hitPoint + N * scene.epsilon;
			
			Vector3f hitColor_reflect = castRay(origin_reflection, dir_reflection, scene, depth + 1);
			Vector3f hitColor_refract = castRay(origin_refraction, dir_refraction, scene, depth + 1);
			float kr = fresnel(dir, N, payload->hit_obj->ior);
			hitColor = hitColor_reflect * kr + hitColor_refract * (1 - kr);
			break;
		}
		case REFLECTION: {
			float kr = fresnel(dir, N, payload->hit_obj->ior);
			Vector3f dir_reflection = normalize(reflect(dir, N));
			Vector3f origin_reflection = (dotProduct(dir_reflection, N) < 0) ?
				hitPoint + N * scene.epsilon :
				hitPoint - N * scene.epsilon;
			hitColor = castRay(origin_reflection, dir_reflection, scene, depth + 1) * kr;
			break;
		}
		default:
			// we use Phong shading in the default case (DIFFUSE_AND_GLOSSY)
			// composed of a diffuse and a specular reflection component. no La light in this case
			Vector3f amt = 0, spec = 0;
			Vector3f origin_shadow = (dotProduct(dir, N) < 0) ?
				hitPoint + N * scene.epsilon :
				hitPoint - N * scene.epsilon;

			for (auto& light : scene.getLights()) {
				Vector3f lightDir = light->origin - hitPoint; // 从当前点射向光源

				float distance_sqrt = dotProduct(lightDir, lightDir);
				lightDir = normalize(lightDir);
				float LdotN = std::max(0.f, dotProduct(lightDir, N)); // light intensity随入射角度增大衰减

				// check if point in shadow, lightDir is normalized, so tnear == distance
				auto shadow_res = trace(origin_shadow, lightDir, scene.getObjects());
				// 如果 light dir上有物体，会在小于distance的距离碰到物体并得到tnear
				bool inShadow = shadow_res && (shadow_res->tNear * shadow_res->tNear < distance_sqrt);

				amt += inShadow ? 0 : light->intensity * LdotN;
				Vector3f dir_reflect = normalize(reflect(-lightDir, N));
				
				spec += powf(std::max(0.f, -dotProduct(dir_reflect, dir)), payload->hit_obj->spew) * light->intensity;
			}

			hitColor = amt * payload->hit_obj->getDiffuseColor(uv) * payload->hit_obj->kd + spec * payload->hit_obj->ks;
			break;
		}
	}

	return hitColor;
}

void Renderer::Render(const Scene& scene) {

	std::vector<Vector3f> framebuffer(scene.width * scene.height);

	// transform and perspective
	float scale = std::tan(degree2Radius(scene.fov * 0.5f));
	float aspect = scene.width / (float)scene.height;

	// camera position, where ray starts
	Vector3f camera_pos(0);

	int k = 0;
	for (int j = 0; j < scene.height; j++) {
		for (int i = 0; i < scene.width; i++) {
			float x;
			float y;

			// i,j 是屏幕空间中的pixel坐标，转换成image space中的坐标 (x,y),从camera_pos与(x,y)的连线就是primary ray
			x = (2 * (i + 0.5f) / (float)(scene.width - 1) - 1) * aspect * scale;
			y = (1 - 2 * (j + 0.5) / scene.height ) * scale ;

			Vector3f dir = normalize(Vector3f(x, y, -1));
			framebuffer[k++] = castRay(camera_pos, dir, scene, 0);
		}
		UpdateProgress(j / (float)scene.height);
	}


	
	// save framebuffer to file
	FILE* fp = fopen("binary.ppm", "wb");
	(void)fprintf(fp, "P6\n%d %d\n255\n", scene.width, scene.height);
	for (auto i = 0; i < scene.height * scene.width; ++i) {
		static unsigned char color[3];
		color[0] = (char)(255 * clamp(0, 1, framebuffer[i].x));
		color[1] = (char)(255 * clamp(0, 1, framebuffer[i].y));
		color[2] = (char)(255 * clamp(0, 1, framebuffer[i].z));
		fwrite(color, 1, 3, fp);
	}
	fclose(fp);
}

