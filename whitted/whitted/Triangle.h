#pragma once

#include "Object.h"

#include <cstring>

bool rayTriangleIntersect(const Vector3f& v0, const Vector3f& v1, const Vector3f& v2,
	const Vector3f& orgin, const Vector3f& dir, float& tnear, float& b1, float& b2 )
{
	// 使用Moller Trumbore Algorithm计算 b1和b2，如果b1>0 b2 >0, tnear > 0 则返回true
	// 为 {b1,b2}赋值，b1和b2还要用来插值

	Vector3f S, S1, S2, E1, E2;
	E1 = v1 - v0;
	E2 = v2 - v0;
	S = orgin - v0;
	S1 = crossProduct(dir, E2);
	S2 = crossProduct(S, E1);
	float tmp = 1.0f / dotProduct(S1, E1);
	tnear = tmp * dotProduct(S2, E2);
	b1 = tmp * dotProduct(S1, S);
	b2 = tmp * dotProduct(S2, dir);

	if (tnear > 0 && b1 > 0 && b2 > 0 && 1 - b1 - b2 > 0) return true;

	return false;
}


class MeshTriangle : public Object
{
public:
	std::unique_ptr<Vector3f[]> vertices; // object所有的顶点
	uint32_t numTriangles; // object mesh中triangle的数量
	std::unique_ptr<uint32_t[]> vertexIndex; // 哪些顶点属于一个三角形
	std::unique_ptr<Vector2f[]> uvCoordinates;

	MeshTriangle(const Vector3f* verts, const uint32_t* vertsIndex, const uint32_t& numTris, const Vector2f* uv) 
	{

		// 取得顶点的数量
		uint32_t numVertex = 0; 
		for (uint32_t i = 0; i < numTris * 3; i++) {
			if (vertsIndex[i] > numVertex)
				numVertex = vertsIndex[i];
		}
		numVertex += 1;

		// 初始化所有数组并赋值
		numTriangles = numTris;
		vertices = std::unique_ptr<Vector3f[]>(new Vector3f[numVertex]);
		vertexIndex = std::unique_ptr<uint32_t[]>(new uint32_t[numTris * 3]);
		uvCoordinates = std::unique_ptr<Vector2f[]>(new Vector2f[numVertex]);

		memcpy(vertices.get(), verts, sizeof(Vector3f) * numVertex);
		memcpy(vertexIndex.get(), vertsIndex, sizeof(uint32_t) * numTris * 3);
		memcpy(uvCoordinates.get(), uv, sizeof(Vector2f) * numVertex);
	}

	bool intersect(const Vector3f& orgin, const Vector3f& dir, float& tnear, uint32_t& index , Vector2f& bbs) const override
	{
		bool isInter = false;
		for (uint32_t i = 0; i < numTriangles; i++) 
		{
			const Vector3f& v0 = vertices[vertexIndex[i * 3]];
			const Vector3f& v1 = vertices[vertexIndex[i * 3 + 1]];
			const Vector3f& v2 = vertices[vertexIndex[i * 3 + 2]];
			float t, b1, b2;
			if (rayTriangleIntersect(v0, v1, v2, orgin, dir, t, b1, b2) && t < tnear) {
				tnear = t;
				bbs.x = b1;
				bbs.y = b2;
				index = i;    // 记录相交的三角形
				isInter |= true;
			}
		}

		return isInter;
	}

	void getSurfaceNormal(const Vector3f&, const Vector3f&, const uint32_t& index, const Vector2f& bbs, Vector3f& N,
		Vector2f& uv) const override
	{
		// 计算三角形表面法向量，插值计算当前的pixel的uv
		const Vector3f& v0 = vertices[vertexIndex[index * 3]];
		const Vector3f& v1 = vertices[vertexIndex[index * 3 + 1]];
		const Vector3f& v2 = vertices[vertexIndex[index * 3 + 2]];
		Vector3f e0 = normalize(v1 - v0);
		Vector3f e1 = normalize(v2 - v1);
		N = normalize(crossProduct(e0, e1));
		const Vector2f& uv0 = uvCoordinates[vertexIndex[index * 3]];
		const Vector2f& uv1 = uvCoordinates[vertexIndex[index * 3 + 1]];
		const Vector2f& uv2 = uvCoordinates[vertexIndex[index * 3 + 2]];
		uv = uv0 * (1 - bbs.x - bbs.y) + uv1 * bbs.x + uv2 * bbs.y;

	}

	// procedure texture chessboard
	Vector3f getDiffuseColor(const Vector2f& uv) const override {
		float scale = 5;
		float pattern = (fmodf(uv.x * scale,1) > 0.5 ^ (fmodf(uv.y * scale, 1) > 0.5));
		return lerp(Vector3f(0.815, 0.235, 0.031), Vector3f(0.937, 0.937, 0.231), pattern);
	}

};