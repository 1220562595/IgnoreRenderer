#pragma once
#include "Primitive.h"

class Disk : public Primitive
{
public:
	Disk(SceneObject* pSceneObject, float radius);

	virtual bool Intersect(const Ray& ray, Intersection& isect) const override;

	virtual void Sample(Vector3f& p, Vector3f& normal, float& pdf) const override;

private:
	float mRadius;

};

