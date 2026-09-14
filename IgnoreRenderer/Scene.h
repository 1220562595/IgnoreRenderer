#pragma once
#include "SceneObject.h"
#include "Camera.h"
#include "Light.h"
#include <map>

class Scene
{
public:
	virtual ~Scene();

	void SetCamera(const Camera& camera) { mCamera = camera; }
	const Camera& GetCamera()const { return mCamera; }

	SceneObject* Intersect(Ray ray, Intersection& isect)const;	

	SceneObject* CreateSceneObject(const Vector3f& position, const Vector3f& euler, float scale);

	template<typename T, typename... Args>
	T* CreateLight(Args&&... args)
	{
		T* light = new T(std::forward<Args>(args)...);
		mLights.push_back(light);
		return (T*)light;
	}

	template<typename T, typename... Args>
	T* CreateMaterial(const std::string& name, Args&&... args)
	{
		T* material = new T(std::forward<Args>(args)...);
		mMaterials.insert(std::make_pair(name, material));
		return (T*)material;
	}

	const std::vector<Light*>& GetLights() const { return mLights; }

private:
	Camera mCamera;
	std::vector<SceneObject*> mSceneObjects;
	std::vector<Light*> mLights;
	std::map<std::string, Material*> mMaterials;
};

