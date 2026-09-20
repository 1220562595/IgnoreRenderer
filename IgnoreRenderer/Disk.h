#pragma once
#include "Primitive.h"
#include "SceneObject.h"

class Disk : public Primitive
{
public:
	Disk(SceneObject* pSceneObject, float radius);

	virtual bool Intersect(const Ray& ray, Intersection& isect) const override;

	virtual void Sample(Vector3f& p, Vector3f& normal, float& pdf) const override;

    AABB GetWorldBounds() const override
    {
        Matrix4x4 o2w = m_pSceneObject->GetObjectToWorld();
        AABB box;
        for (int i = 0; i < 4; ++i)
        {
            Vector3f local((i & 1) ? mRadius : -mRadius,
                (i & 2) ? mRadius : -mRadius, 0.0f);
            box.Expand(Vector3f(o2w * Vector4f(local, 1.0f)));
        }
        return box;
    }

private:
	float mRadius;

};

