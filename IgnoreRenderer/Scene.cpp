#include "Scene.h"

Scene::~Scene()
{
    for (SceneObject* pSceneObject : mSceneObjects)
        delete pSceneObject;

	for (Light* pLight : mLights)
		delete pLight;

	for (auto& pair : mMaterials)
		delete pair.second;
}

SceneObject* Scene::Intersect(Ray ray, Intersection& isect) const
{
	if (mBVHBuilt)
	{
		const Primitive* pHitPrim = nullptr;
		if (mBVH.Intersect(ray, isect, pHitPrim))
			return pHitPrim->GetSceneObject();
		return nullptr;
	}

	SceneObject* pHitObject = nullptr;
	for (const auto pSceneObject : mSceneObjects)
	{
		if (pSceneObject->Intersect(ray, isect))
		{
			ray.maxt = isect.t;
			pHitObject = pSceneObject;
		}
	}

	return pHitObject;
}

SceneObject* Scene::CreateSceneObject(const Vector3f& position, const Vector3f& euler, float scale)
{
    SceneObject* pSceneObject = new SceneObject(position, euler, scale);
    mSceneObjects.push_back(pSceneObject);

    return pSceneObject;
}

void Scene::BuildAccelerationStructure()
{
	std::vector<Primitive*> allPrims;
	for (SceneObject* obj : mSceneObjects)
		for (Primitive* prim : obj->GetPrimitives())   // 需要给 SceneObject 加个访问器
			allPrims.push_back(prim);

	mBVH.Build(allPrims);
	mBVHBuilt = true;
}

bool Scene::Occluded(const Ray& ray) const
{
	if (mBVHBuilt)
		return mBVH.Occluded(ray);

	// fallback：BVH 未建立时线性遍历
	Intersection isect;
	for (const SceneObject* obj : mSceneObjects)
	{
		if (obj->Intersect(ray, isect))
			return true;    // 命中任意一个即可，不需要最近点
	}
	return false;
}
