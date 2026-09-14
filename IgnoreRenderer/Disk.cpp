#include "Disk.h"
#include "SceneObject.h"

Disk::Disk(SceneObject* pSceneObject, float radius)
	: Primitive(pSceneObject), mRadius(radius)
{
}

bool Disk::Intersect(const Ray& ray, Intersection& isect) const
{
	//ray转到圆盘局部空间
	Ray r = m_pSceneObject->GetWorldToObject() * ray;

	//t
	if (fabs(r.d.z) < 1e-6f) //射线平行于圆盘所在平面
		return false;

	float t = -r.o.z / r.d.z; //圆盘所在平面z=0

	if (t < r.mint || t > r.maxt)
		return false;

	Vector3f p = r.o + t * r.d; //局部空间交点

	if (glm::dot(p, p) > mRadius * mRadius) //交点不在圆盘内
		return false;

	isect.position = Vector3f(m_pSceneObject->GetObjectToWorld() * Vector4f(p, 1.0f)); //交点位置（世界空间）
	isect.normal = glm::normalize(Vector3f(m_pSceneObject->GetObjectToWorld() * Vector4f(0.0f, 0.0f, 1.0f, 0.0f))); //交点法线（世界空间）
	isect.t = t;

	return true;
}

void Disk::Sample(Vector3f& p, Vector3f& normal, float& pdf) const
{
	Vector2f p2 = UniformSampleDisk(mRadius);
	p = Vector3f(p2.x, p2.y, 0.0f); //局部空间采样点
	p = Vector3f(m_pSceneObject->GetObjectToWorld() * Vector4f(p, 1.0f)); //世界空间采样点

	normal = glm::normalize(Vector3f(m_pSceneObject->GetObjectToWorld() * Vector4f(0.0f, 0.0f, 1.0f, 0.0f))); //世界空间法线
	pdf = 1.0f / (PI * mRadius * mRadius); //均匀采样圆盘的pdf
}
