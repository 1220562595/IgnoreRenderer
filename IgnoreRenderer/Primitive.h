#pragma once
#include "Ray.h"

class SceneObject;

class Primitive
{
public:
	Primitive(SceneObject* pSceneObject) : m_pSceneObject(pSceneObject) {}
	virtual ~Primitive() = default;
	virtual bool Intersect(const Ray& ray, Intersection& isect) const = 0;
	virtual void Sample(Vector3f& p, Vector3f& normal, float& pdf) const = 0;

protected:
	SceneObject* m_pSceneObject = nullptr; //所属场景对象
};

