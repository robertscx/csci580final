#pragma once

#include "Lightray.h"
#include "Vector.h"
#include <limits>
#include <array>

class Boundbox 
{
	public:
		Vector3f pMin, pMax; // two points that define a bounding box

		Boundbox() {
			double minVal = std::numeric_limits<double>::lowest();
			double maxVal = std::numeric_limits<double>::max();
			pMax = Vector3f(minVal, minVal, minVal);
			pMin = Vector3f(maxVal, maxVal, maxVal);
		}

		Boundbox(const Vector3f p) : pMin(p), pMax(p) {}

		Boundbox(const Vector3f p1, const Vector3f p2) {
			pMin = Vector3f(fmin(p1.x, p2.x), fmin(p1.y, p2.y), fmin(p1.z, p2.z));
			pMax = Vector3f(fmax(p1.x, p2.x), fmax(p1.y, p2.y), fmax(p1.z, p2.z));
		}

		Vector3f Diagonal() const { return pMax - pMin; }

		int longestEdge() const {
			Vector3f diag = Diagonal();
			if (diag.x > diag.y && diag.x > diag.z) {
				return 0; // longest edge along x-axis
			}
			else if (diag.y > diag.z) {
				return 1; // longest edge along y-axis
			}
			else {
				return 2; // longest edge along z-axis
			}
		}

		double Centroid() const { return 0.5 * pMin + 0.5 * pMax; }

		inline const Vector3f& operator[](int i) const
		{
			return (i == 0) ? pMin : pMax;
		}

		inline bool IntersectP(const Ray& ray, const Vector3f& invDir,
			const std::array<int, 3>& dirisNeg) const;
};

inline Boundbox Union(const Bounds3& b1, const Bounds3& b2)
{
	Boundbox ret;
	ret.pMin = Vector3f(fmin(b1.pMin.x, b2.pMin.x), fmin(b1.pMin.y, b2.pMin.y), fmin(b1.pMin.z, b2.pMin.z));
	ret.pMax = Vector3f(fmin(b1.pMax.x, b2.pMax.x), fmin(b1.pMax.y, b2.pMax.y), fmin(b1.pMax.z, b2.pMax.z));
	return ret;
}

inline Boundbox Union(const Bounds3& b, const Vector3f& p)
{
	Bounds3 ret;
	ret.pMin = Vector3f(fmin(b1.pMin.x, p.x), fmin(b1.pMin.y, p.y), fmin(b1.pMin.z, p.z));
	ret.pMax = Vector3f(fmin(b1.pMax.x, p.x), fmin(b1.pMax.y, p.y), fmin(b1.pMax.z, p.z));
	return ret;
}

inline bool Boundbox::IntersectP(const Ray& ray, const Vector3f& invDir,
	const std::array<int, 3>& dirIsNeg) const
{
	// invDir: ray direction(x,y,z), invDir=(1.0/x,1.0/y,1.0/z), use this because Multiply is faster that Division
	// dirIsNeg: ray direction(x,y,z), dirIsNeg=[int(x>0),int(y>0),int(z>0)], use this to simplify your logic

	// t = (ray.xyz - origin.xyz) / dxyz
	double tmin(0.0), tmax(0.0);
	// x
	tmin = (pMin.x - ray.origin.x) * invDir.x;
	tmax = (pMax.x - ray.origin.x) * invDir.x;
	double tmin_x = dirIsNeg[0] > 0 ? tmin : tmax;
	double tmax_x = dirIsNeg[0] > 0 ? tmax : tmin;
	// y
	tmin = (pMin.y - ray.origin.y) * invDir.y;
	tmax = (pMax.y - ray.origin.y) * invDir.y;
	double tmin_y = dirIsNeg[1] > 0 ? tmin : tmax;
	double tmax_y = dirIsNeg[1] > 0 ? tmax : tmin;
	// z
	tmin = (pMin.z - ray.origin.z) * invDir.z;
	tmax = (pMax.z - ray.origin.z) * invDir.z;
	double tmin_z = dirIsNeg[2] > 0 ? tmin : tmax;
	double tmax_z = dirIsNeg[2] > 0 ? tmax : tmin;

	if (std::max(tmin_x, std::max(tmin_y, tmin_z)) < std::min(tmax_x, std::min(tmax_y, tmax_z)))
		return true;
	else
		return false;

}