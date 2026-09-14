#include "SceneObject.h"

SceneObject::~SceneObject()
{
	for (auto& primitive : mPrimitives)
	{
		if (primitive)
			delete primitive;
	}
}

bool SceneObject::Intersect(Ray ray, Intersection& isect) const
{
	bool bHit = false;
	for(auto primitive : mPrimitives)
	{
		if (primitive->Intersect(ray, isect))
		{
			ray.maxt = isect.t; //更新射线的最大范围，避免后续的几何体遮挡
			bHit = true;
		}
	}

	return bHit;
}

void SceneObject::Sample(Vector3f& p, Vector3f& normal, float& pdf) const
{
	int index = rand() % mPrimitives.size();
	mPrimitives[index]->Sample(p, normal, pdf);
	pdf /= mPrimitives.size(); //平均采样每个基本图元
}