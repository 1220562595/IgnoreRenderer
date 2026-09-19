#pragma once
#include "Ray.h"

class SceneObject;

struct AABB
{
	Vector3f minP = Vector3f(FLT_MAX);
	Vector3f maxP = Vector3f(-FLT_MAX);

	void Expand(const Vector3f& p)
	{
		minP = glm::min(minP, p);
		maxP = glm::max(maxP, p);
	}

    void Expand(const AABB& b) { Expand(b.minP); Expand(b.maxP); }
    Vector3f Centroid() const { return 0.5f * (minP + maxP); }

    // slab 法：射线与盒子求交，maxt 用于提前剔除
    bool Intersect(const Ray& ray, float maxt) const
    {
        float tEnter = ray.mint, tExit = maxt;
        for (int i = 0; i < 3; ++i)
        {
            float invD = 1.0f / ray.d[i];
            float t0 = (minP[i] - ray.o[i]) * invD;
            float t1 = (maxP[i] - ray.o[i]) * invD;
            if (invD < 0.0f) std::swap(t0, t1);
            tEnter = glm::max(tEnter, t0);
            tExit = glm::min(tExit, t1);
            if (tEnter > tExit) return false;
        }
        return true;
    }
};

class Primitive
{
public:
	Primitive(SceneObject* pSceneObject) : m_pSceneObject(pSceneObject) {}
	virtual ~Primitive() = default;
	virtual bool Intersect(const Ray& ray, Intersection& isect) const = 0;
	virtual void Sample(Vector3f& p, Vector3f& normal, float& pdf) const = 0;

    virtual AABB GetWorldBounds() const = 0;
    SceneObject* GetSceneObject() const { return m_pSceneObject; }

protected:
	SceneObject* m_pSceneObject = nullptr; //所属场景对象
};

