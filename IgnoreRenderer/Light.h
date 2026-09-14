#pragma once
#include "Ray.h"

class SceneObject;

class Light
{
public:
	//对于一点p，计算光照强度
	virtual Color GetRadiance(const Vector3f& p, Vector3f& sourcePos, float& pdf) const = 0;
};

//平行光
class DirectionalLight : public Light
{
public:
	DirectionalLight(const Vector3f& direction, const Color& color)
		: mDirection(glm::normalize(direction)), mRadiance(color) {}

	virtual Color GetRadiance(const Vector3f& p, Vector3f& sourcePos, float& pdf) const override;

private:
	Vector3f mDirection; // 光线方向，单位向量
	Color mRadiance; // 光照强度，RGB颜色
};

//点光源
class PointLight : public Light
{
public:
	PointLight(const Vector3f& position, const Color& intensity, const Vector3f& attenuations)
		: mPosition(position), mIntensity(intensity), mAttenuations(attenuations) {}

	virtual Color GetRadiance(const Vector3f& p, Vector3f& sourcePos, float& pdf) const override;

private:
	Vector3f mPosition; // 光源位置
	Color mIntensity; // 光照强度，RGB颜色
	Vector3f mAttenuations; // 衰减系数，分别对应二次项、一次项和常数项
};

//聚光灯
class SpotLight : public Light
{
public:
	SpotLight(const Vector3f& position, const Vector3f& direction, const Color& intensity, float innerAngle, float outerAngle, const Vector3f& attenuations)
		: mPosition(position), mDirection(glm::normalize(direction)), mIntensity(intensity), mCosInnerAngle(glm::cos(innerAngle)), mCosOuterAngle(glm::cos(outerAngle)), mAttenuations(attenuations) {}
	
	virtual Color GetRadiance(const Vector3f& p, Vector3f& sourcePos, float& pdf) const override;

private:
	Vector3f mPosition; // 光源位置
	Vector3f mDirection; // 光线方向，单位向量
	Color mIntensity; // 光照强度，RGB颜色

	float mCosInnerAngle; // 内角范围
	float mCosOuterAngle; // 外角范围

	Vector3f mAttenuations; // 衰减系数，分别对应二次项、一次项和常数项
};

//面光源
class AreaLight : public Light
{
public:
	AreaLight(SceneObject* pSceneObject)
		: m_pSceneObject(pSceneObject) {}

	virtual Color GetRadiance(const Vector3f& p, Vector3f& sourcePos, float& pdf) const override;

private:
	SceneObject* m_pSceneObject;
};
