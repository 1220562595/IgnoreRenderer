#pragma once
#include "Common.h"
#include <atomic>
#include "Camera.h"
#include "Sphere.h"
#include "Disk.h"
#include "Triangle.h"
#include <vector>
#include "Scene.h"

class Renderer
{
public:
	Renderer(int w, int h, int minDepth, int maxDepth, int samplePerPixel);
	void CreateCornellBox();
	virtual ~Renderer();

	void CreateBox(SceneObject* obj, float w, float h, float d);

	void Run();

private:
	Color RenderPixel(int x, int y);
	void RunRenderThread();
	Color RenderSubPixel(float x, float y);
	Color GetIrradiance(const Ray& ray);
	Color GetRadiance(const Ray& ray, int depth, bool bPrevIsSpecular);

	int mViewportWidth = 800;
	int mViewportHeight = 600;
	//屏幕上每个像素的采样次数
	int SamplePerPixel = 4;
	int mMaxDepth = 10;
	int mMinDepth = 3;

	uint32_t* mBuffer = nullptr;
	std::atomic<int> mCurrentPixelIndex = 0;	

	Camera mCamera;

	//std::vector<Primitive*> mPrimitives; //场景中所有几何体
	//SceneObject* mTestSceneObject = nullptr; //场景对象

	Scene* mScene = nullptr;
};