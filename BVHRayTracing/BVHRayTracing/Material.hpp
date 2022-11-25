//
// Created by LEI XU on 5/16/19.
//

#ifndef RAYTRACING_MATERIAL_H
#define RAYTRACING_MATERIAL_H

#include "Vector.hpp"

enum MaterialType { DIFFUSE};

class Material{
private:

    //todo: Add new reflect&refract&fresnel&toworld
    // Compute reflection direction
    Vector3f reflect(const Vector3f& I, const Vector3f& N) const
    {
        return I - 2 * dotProduct(I, N) * N;
    }

    Vector3f refract(const Vector3f& I, const Vector3f& N, const float& ior) const
    {
        float cosi = clamp(-1, 1, dotProduct(I, N));
        float etai = 1, etat = ior;
        Vector3f n = N;
        if (cosi < 0) { cosi = -cosi; }
        else { std::swap(etai, etat); n = -N; }
        float eta = etai / etat;
        float k = 1 - eta * eta * (1 - cosi * cosi);
        return k < 0 ? 0 : eta * I + (eta * cosi - sqrtf(k)) * n;
    }

    void fresnel(const Vector3f& I, const Vector3f& N, const float& ior, float& kr) const
    {
        float cosi = clamp(-1, 1, dotProduct(I, N));
        float etai = 1, etat = ior;
        if (cosi > 0) { std::swap(etai, etat); }
        // Compute sini using Snell's law
        float sint = etai / etat * sqrtf(std::max(0.f, 1 - cosi * cosi));
        // Total internal reflection
        if (sint >= 1) {
            kr = 1;
        }
        else {
            float cost = sqrtf(std::max(0.f, 1 - sint * sint));
            cosi = fabsf(cosi);
            float Rs = ((etat * cosi) - (etai * cost)) / ((etat * cosi) + (etai * cost));
            float Rp = ((etai * cosi) - (etat * cost)) / ((etai * cosi) + (etat * cost));
            kr = (Rs * Rs + Rp * Rp) / 2;
        }
        // As a consequence of the conservation of energy, transmittance is given by:
        // kt = 1 - kr;
    }

    // convert localRay where N  = (0,0,1) to world space
    Vector3f toWorld(const Vector3f& a,const Vector3f& N) {
        Vector3f fac1, fac2;

        // bool to avoid division with zero
        if (std::fabs(N.x) > std::fabs(N.y)) 
        { // weight in XZ coord and vertical to N
            float normalize_factor = 1.0f / std::sqrt(N.x * N.x + N.z * N.z);
            fac1 = Vector3f(N.z / normalize_factor, 0.0f, -N.x / normalize_factor);
        }
        else
        { // weight in YZ coord and vertical to N
            float normalize_factor = 1.0f / std::sqrt(N.y * N.y + N.z * N.z);
            fac1 = Vector3f(0.0f ,N.z / normalize_factor, -N.y / normalize_factor);
        }

        fac2 = crossProduct(fac1, N);
        return a.x * fac2 + a.y * fac1 + a.z * N;
    }


public:
    MaterialType m_type;
    Vector3f m_color;
    Vector3f m_emission;
    float ior;
    Vector3f Kd, Ks;
    float specularExponent;
    //Texture tex;

    inline Material(MaterialType t=DIFFUSE, Vector3f e=Vector3f(0,0,0));
    inline MaterialType getType();
    inline Vector3f getColor();
    inline Vector3f getColorAt(double u, double v);
    inline Vector3f getEmission();

    //todo: Add new hasEmission && sample && pdf && eval
    // if is light source
    inline bool hasEmission();

    // sample ray by Material properties
    inline Vector3f sample(const Vector3f& wi,const Vector3f& N);
    //
    inline float pdf(const Vector3f& wi, const Vector3f& wo, const Vector3f& N);
    // BRDF term of material.
    inline Vector3f eval(const Vector3f& wi, const Vector3f& wo, const Vector3f& N);
};

Material::Material(MaterialType t, Vector3f e){
    m_type = t;
    m_emission = e;
}

MaterialType Material::getType(){return m_type;}
Vector3f Material::getColor(){return m_color;}
Vector3f Material::getEmission() {return m_emission;}

// add New
bool Material::hasEmission() {
    if (m_emission.norm() > EPSILON) return true;
    else return false;
}

Vector3f Material::getColorAt(double u, double v) {
    return Vector3f();
}

Vector3f Material::sample(const Vector3f& wi,const Vector3f& N) {
    switch (m_type) {
        case DIFFUSE:
        {
            //uniform sampling on hemisphere 
            //getting an direction on hemisphere, converting to worldspace and return.
            float x1 = get_random_float(), x2 = get_random_float();
            float z = std::fabs(1.0f - 2.0f * x1);
            float r = std::sqrt(1.0f - z * z), phi = 2 * M_PI * x2;
            Vector3f localRay(r * std::cos(phi), r * std::sin(phi), z);
            return toWorld(localRay, N);
            
            break;
        }
    }
}

float Material::pdf(const Vector3f& wi, const Vector3f& wo, const Vector3f& N) {
    switch (m_type) {
        case DIFFUSE: 
        {
            if (dotProduct(wo, N) > 0.0f)
                return 0.5f / M_PI;
            else
                return 0.0f;
            break;
        }
    }
}

Vector3f Material::eval(const Vector3f& wi, const Vector3f& wo, const Vector3f& N) {
    switch (m_type) {
        case DIFFUSE: 
        {
            if (dotProduct(N, wo) > 0.0) { //only consider hemisphere 
                return Kd / M_PI; // why divided by PI? should be 2 * PI ????
            }
            else
                return Vector3f(0.0f);
            break;
        }
    }

    return Vector3f(0.0f);
}

#endif //RAYTRACING_MATERIAL_H
