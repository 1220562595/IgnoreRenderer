#pragma once
#include "Ray.h"
#include "Primitive.h"
#include <vector>

class Material;

class SceneObject
{
public:
	SceneObject(const Vector3f& position, const Vector3f& euler, float scale)
		: mEmission(0.0f)
	{
		mObjectToWorld = MakeWorldTransform(position, euler, scale);
		mWorldToObject = glm::inverse(mObjectToWorld);
	}
	virtual ~SceneObject();

	bool Intersect(Ray ray, Intersection& isect) const;

	template<typename T, typename... Args>
	T* CreatePrimitive(Args&&... args)
	{
		T* primitive = new T(this, std::forward<Args>(args)...);
		mPrimitives.push_back(primitive);
		return primitive;
	}

	Matrix4x4 GetObjectToWorld() const { return mObjectToWorld; }
	Matrix4x4 GetWorldToObject() const { return mWorldToObject; }

	void SetMaterial(Material* pMaterial) { m_pMaterial = pMaterial; }
	Material* GetMaterial() const { return m_pMaterial; }

	void SetEmission(const Color& emission) { mEmission = emission; }
	Color GetEmission() const { return mEmission; }

	void Sample(Vector3f& p, Vector3f& normal, float& pdf) const;

private:
	Matrix4x4 mObjectToWorld;
	Matrix4x4 mWorldToObject;

	Material* m_pMaterial = nullptr; //场景对象的材质

	std::vector<Primitive*> mPrimitives; //场景对象包含的基本图元

	Color mEmission;
};

