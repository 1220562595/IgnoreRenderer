#include "Light.h"
#include "SceneObject.h"

Color DirectionalLight::GetRadiance(const Vector3f& p, Vector3f& sourcePos, float& pdf) const
{
	sourcePos = p - mDirection * 100000.0f; // 假设光源在无限远处，沿着方向向量反向延伸
	pdf = 1.0f;
	return mRadiance;
}

Color PointLight::GetRadiance(const Vector3f& p, Vector3f& sourcePos, float& pdf) const
{
	sourcePos = mPosition;
	float R = glm::length(mPosition - p); // 计算光源到点p的距离
	float attenuation = 1.0f / glm::max(1e-5f, mAttenuations.z + mAttenuations.y * R + mAttenuations.x * R * R);
	
	pdf = 1.0f;
	return mIntensity * attenuation;
}

Color SpotLight::GetRadiance(const Vector3f& p, Vector3f& sourcePos, float& pdf) const
{
	sourcePos = mPosition;
	// 计算光源到点p的距离，并根据距离计算衰减 k1
	float R = glm::length(mPosition - p); // 光源到点p的距离
	float k1 = 1.0f / glm::max(1e-5f, mAttenuations.z + mAttenuations.y * R + mAttenuations.x * R * R);

	// 计算光线与聚光灯方向的夹角，并根据夹角计算衰减 k2
	Vector3f L = glm::normalize(p - mPosition); // 光线方向
	float cosTheta = glm::dot(L, -mDirection); // 计算光线与聚光灯方向的夹角余弦值

	float k2 = (cosTheta - mCosOuterAngle) / (mCosInnerAngle - mCosOuterAngle); // 线性插值计算衰减

	pdf = 1.0f;
	return mIntensity * k1 * glm::clamp(k2, 0.0f, 1.0f); // 返回最终光照强度
}

Color AreaLight::GetRadiance(const Vector3f& p, Vector3f& sourcePos, float& pdf) const
{
	Vector3f A;
	Vector3f normalA;
	float pdfA;
	m_pSceneObject->Sample(A, normalA, pdfA);

	sourcePos = A;

	Vector3f d = p - A;
	float R = glm::length(d);
	d /= std::max(1e-5f, R);

	float cosThetaA = glm::dot(normalA, d);
	if (cosThetaA < 1e-5f)
	{
		pdf = 0.0f;
		return Color(0, 0, 0);
	}

	pdf = pdfA * (R * R) / cosThetaA;

	return m_pSceneObject->GetEmission();
}
