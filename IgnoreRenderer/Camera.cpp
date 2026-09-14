#include "Camera.h"

void Camera::Initialize(const Vector3f& p, const Vector3f& target, const Vector3f& up, float fov, float n, float f, int W, int H)
{
	mPosition = p;

	//求观察矩阵
	/*Vector3f l = glm::normalize(target - p);
	Vector3f r = glm::normalize(glm::cross(up, l));
	Vector3f u = glm::cross(l, r);

	Matrix4x4 viewMatrix = glm::transpose(Matrix4x4(
		r.x, r.y, r.z, 0.0f,
		u.x, u.y, u.z, 0.0f,
		l.x, l.y, l.z, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	)) * MakeTranslation(-p);*/

	//左手坐标系
	Matrix4x4 viewMatrix = glm::lookAtLH(p, target, up);

	//求投影矩阵，z取值范围0-1
	Matrix4x4 projectionMatrix = glm::perspectiveFovLH_ZO(fov, (float)W, (float)H, n, f);

	//求视口矩阵
	Matrix4x4 viewportMatrix = Matrix4x4(
		W / 2.0f, 0.0f, 0.0f, 0.0f,
		0.0f, -H / 2.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		W / 2.0f, H / 2.0f, 0.0f, 1.0f
	);

	mCombinedMatrix = viewportMatrix * projectionMatrix * viewMatrix;
	mInvCombinedMatrix = glm::inverse(mCombinedMatrix);
}

Ray Camera::GetRay(float x, float y) const
{
	Ray ray;
	ray.o = mPosition;

	Vector4f p(x, y, 0.0f, 1.0f);
	Vector4f worldPos = mInvCombinedMatrix * p;

	worldPos /= worldPos.w;

	ray.d = glm::normalize(Vector3f(worldPos) - mPosition);

	return ray;
}
