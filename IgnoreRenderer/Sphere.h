#pragma once
#include "Primitive.h"

class Sphere : public Primitive
{
public:
	Sphere(SceneObject* pSceneObject, float R);
	~Sphere();

	virtual bool Intersect(const Ray& ray, Intersection& isect) const override;

	virtual void Sample(Vector3f& p, Vector3f& normal, float& pdf) const override;

private:
	float mRadius;
};

