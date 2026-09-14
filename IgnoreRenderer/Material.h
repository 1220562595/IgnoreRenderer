#pragma once
#include "Ray.h"

class Material
{
public:
	virtual Color BRDF(const Vector3f& wo, const Vector3f& wi) const = 0;
	virtual Color BTDF(const Vector3f& wo, const Vector3f& wi) const { return Color(0); }
	virtual bool IsSpecular() const { return false; }
	virtual bool SampleWt(const Vector3f& wo, Vector3f& wt) const { return false; }
	virtual Vector3f SampleWi(const Vector3f& wo, float& pdf) const { return Vector3f(0); }
};

class LambertMaterial : public Material
{
public:
	LambertMaterial(const Color& albedo) : mAlbedo(albedo) {}

	virtual Color BRDF(const Vector3f& wo, const Vector3f& wi) const override
	{
		return mAlbedo * INV_PI; // Lambert反射模型
	}

	virtual Vector3f SampleWi(const Vector3f& wo, float& pdf) const override
	{
		float e1 = Random01();
		float e2 = Random01();

		float cosTheta = sqrt(1.0f - e1);
		float sinTheta = sqrt(e1);
		float phi = 2.0f * PI * e2;

		Vector3f wi(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);

		pdf = cosTheta * INV_PI;

		return wi;
	}

private:
	Color mAlbedo;
};

// 导体的镜面材质
class ConductorSpecularMaterial : public Material
{
public:
	ConductorSpecularMaterial(const Color& eta, const Color& absorptionCoef, const Color& reflectionColor)
		: mEta(eta), mAbsorptionCoef(absorptionCoef), mReflectionColor(reflectionColor){ }

	virtual bool IsSpecular() const override { return true; }

	virtual Color BRDF(const Vector3f& wo, const Vector3f& wi) const override;

	virtual Vector3f SampleWi(const Vector3f& wo, float& pdf) const override;

private:
	Color mEta; //折射率
	Color mAbsorptionCoef; //消光系数
	Color mReflectionColor; 
};

// 绝缘体的镜面材质
class DielectricSpecularMaterial : public Material
{
public:
	DielectricSpecularMaterial(float eta, Color transmissionColor) :
		mEta(eta), mTransmissionColor(transmissionColor) {}

	virtual bool IsSpecular() const { return true; }
	virtual Color BRDF(const Vector3f& wo, const Vector3f& wi) const override;
	virtual Color BTDF(const Vector3f& wo, const Vector3f& wi) const override;
	virtual bool SampleWt(const Vector3f& wi, Vector3f& wt) const override;
	virtual Vector3f SampleWi(const Vector3f& wo, float& pdf) const override;

private:
	static float Fresnel(float eta_i, float eta_t, float cos_i, float cos_t);
	float mEta;
	Color mTransmissionColor;
};

