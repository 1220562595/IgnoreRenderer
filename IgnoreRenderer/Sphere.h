#pragma once
#include "Primitive.h"
#include "SceneObject.h"

class Sphere : public Primitive
{
public:
	Sphere(SceneObject* pSceneObject, float R);
	~Sphere();

	virtual bool Intersect(const Ray& ray, Intersection& isect) const override;

	virtual void Sample(Vector3f& p, Vector3f& normal, float& pdf) const override;

    AABB GetWorldBounds() const override
    {
        Matrix4x4 o2w = m_pSceneObject->GetObjectToWorld();
        Vector3f center = Vector3f(o2w * Vector4f(0, 0, 0, 1));
        float scale = glm::length(Vector3f(o2w[0])); // X 轴基向量长度 = 缩放
        AABB box;
        box.Expand(center - mRadius * scale);
        box.Expand(center + mRadius * scale);
        return box;
    }

private:
	float mRadius;
};

