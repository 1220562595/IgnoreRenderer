#include "Renderer.h"
#include <MiniFB.h>
#include <thread>
#include <vector>
#include "SceneObject.h"
#include "Material.h"
#include <chrono>
#include <ctime>

Renderer::Renderer(int w, int h, int minDepth, int maxDepth, int samplePerPixel)
    : mViewportWidth(w), mViewportHeight(h), mBuffer(nullptr),
    mCurrentPixelIndex(0), SamplePerPixel(samplePerPixel), mMaxDepth(maxDepth), mMinDepth(minDepth)
{
    mScene = new Scene();

    // ========== 相机设置（康奈尔盒标准视角）==========
    mCamera.Initialize(
        Vector3f(0, 1, 5.0f),      // 相机位置：在盒子前方偏上
        Vector3f(0, 1, 0),          // 目标点：盒子中心
        Vector3f(0, 1, 0),          // 上方向
        glm::radians(60.0f),        // FOV 60度
        0.1f,
        100.0f,
        w, h
    );
    mScene->SetCamera(mCamera);

    CreateCornellBox();

	//auto pMaterial = mScene->CreateMaterial<LambertMaterial>("Red", Color(1.0f, 0.0f, 0.0f));
	//SceneObject* obj1 = mScene->CreateSceneObject(Vector3f(0.5, 1.5, 1.5), Vector3f(0, 0, 0), 1.0f);
	//SceneObject* obj2 = mScene->CreateSceneObject(Vector3f(-0.5, 0.5, 1), Vector3f(0, 0, 0), 1.0f);
 //   /*obj1->CreatePrimitive<Sphere>(0.2f);
	//obj1->SetMaterial(mScene->CreateMaterial<LambertMaterial>("Red", Color(1.0f, 0.0f, 0.0f)));*/
 //   obj2->CreatePrimitive<Sphere>(0.6f);
	////obj2->SetMaterial(mScene->CreateMaterial<ConductorSpecularMaterial>("white", Color(1.5f, 1.5f, 1.5f), Color(3.0f, 3.0f, 3.0f), Color(1.0f, 1.0f, 1.0f)));
 //   obj2->SetMaterial(mScene->CreateMaterial<DielectricSpecularMaterial>("white",1.5f,Color(1.0f, 1.0f, 1.0f)));
 //   SceneObject* obj3 = mScene->CreateSceneObject(Vector3f(0.5, 0.5, 1), Vector3f(0, 0, 0), 1.0f);
 //   obj3->CreatePrimitive<Sphere>(0.3f);
	//obj3->SetEmission(Color(10.0f, 10.0f, 10.0f));
	//obj3->SetMaterial(mScene->CreateMaterial<LambertMaterial>("White", Color(1.0f, 1.0f, 1.0f)));

    std::vector<Material*> palette;
    palette.push_back(mScene->CreateMaterial<LambertMaterial>("BallA", Color(0.9f, 0.2f, 0.2f)));
    palette.push_back(mScene->CreateMaterial<LambertMaterial>("BallB", Color(0.2f, 0.9f, 0.2f)));
    palette.push_back(mScene->CreateMaterial<LambertMaterial>("BallC", Color(0.2f, 0.2f, 0.9f)));
    palette.push_back(mScene->CreateMaterial<LambertMaterial>("BallD", Color(0.85f, 0.85f, 0.85f)));
    palette.push_back(mScene->CreateMaterial<LambertMaterial>("BallE", Color(0.9f, 0.7f, 0.2f)));

    struct Ball { Vector3f c; float r; };
    std::vector<Ball> placed = {
        { Vector3f(-0.5f, 0.5f, 1.0f), 0.6f },
        { Vector3f(0.5f, 0.5f, 1.0f), 0.3f }
    };
    const int testBallCount = 500;              // ← 调这个数控制压力：100 / 500 / 2000
    int created = 0, attempts = 0;
    while (created < testBallCount && attempts < testBallCount * 50)
    {
        ++attempts;
        float r = Random(0.08f, 0.22f);         // 随机半径
        Vector3f c(Random(-1.7f, 1.7f), r, Random(-1.7f, 1.3f));  // 放在地面上（y=r）

        // 拒绝采样：和已有球保持距离
        bool ok = true;
        for (const Ball& b : placed)
        {
            if (glm::length(c - b.c) < r + b.r + 0.05f) { ok = false; break; }
        }
        if (!ok) continue;

        placed.push_back({ c, r });
        SceneObject* ball = mScene->CreateSceneObject(c, Vector3f(0, 0, 0), 1.0f);
        ball->CreatePrimitive<Sphere>(r);
        ball->SetMaterial(palette[created % palette.size()]);
        ++created;
    }
    std::cout << "Placed " << created << " test spheres ("
        << (10 + 2 + created) << " primitives total)." << std::endl;

    // ========== 随机圆盘（像撒硬币一样铺在地面/漂浮） ==========
    const int testDiskCount = 100;
    for (int i = 0; i < testDiskCount; ++i)
    {
        float radius = Random(0.15f, 0.35f);

        Vector3f pos, euler;
        if (i % 2 == 0)
        {
            // 一半：平躺在地面上（局部法线 +z → 绕X转-90° 朝向 +y）
            pos = Vector3f(Random(-1.7f, 1.7f), 0.01f, Random(-1.7f, 1.3f));
            euler = Vector3f(glm::radians(-90.0f), 0.0f, Random(0.0f, 2.0f * PI)); // 随机朝向
        }
        else
        {
            // 一半：随机方向漂浮在空中
            pos = Vector3f(Random(-1.7f, 1.7f), Random(0.5f, 3.5f), Random(-1.7f, 1.3f));
            euler = Vector3f(Random(0.0f, PI), Random(0.0f, PI), Random(0.0f, PI));
        }

        SceneObject* diskObj = mScene->CreateSceneObject(pos, euler, 1.0f);
        diskObj->CreatePrimitive<Disk>(radius);
        diskObj->SetMaterial(palette[i % palette.size()]);
    }

    // ========== 随机三角形（全部挂在一个对象上，集中压测图元数量） ==========
    const int testTriCount = 200;
    SceneObject* triHost = mScene->CreateSceneObject(Vector3f(0, 0, 0), Vector3f(0, 0, 0), 1.0f);
    triHost->SetMaterial(mScene->CreateMaterial<LambertMaterial>("BallF", Color(0.5f, 0.5f, 0.9f)));
    for (int i = 0; i < testTriCount; ++i)
    {
        // 在小立方体内随机取三个点构成三角形
        Vector3f center(Random(-1.7f, 1.7f), Random(0.3f, 3.7f), Random(-1.7f, 1.3f));
        float s = Random(0.1f, 0.3f);
        Vector3f v0 = center + Vector3f(Random(-s, s), Random(-s, s), Random(-s, s));
        Vector3f v1 = center + Vector3f(Random(-s, s), Random(-s, s), Random(-s, s));
        Vector3f v2 = center + Vector3f(Random(-s, s), Random(-s, s), Random(-s, s));

        triHost->CreatePrimitive<Triangle>(v0, v1, v2);
    }
    
    std::cout << "Placed " << created << " spheres, " << testDiskCount
        << " disks, " << testTriCount << " triangles ("
        << (10 + 2 + created + testDiskCount + testTriCount)
        << " primitives total)." << std::endl;

    mScene->BuildAccelerationStructure();
}

void Renderer::CreateCornellBox()
{
    // ========== 构建康奈尔盒（尺寸：宽4，高4，深4）==========
    // 盒子中心在 (0, 2, 0)，范围 x[-2,2], y[0,4], z[-2,2]

    float roomSize = 4.0f;
    float halfSize = roomSize * 0.5f;  // 2.0f

    // ---- 地面（y = 0）----
    SceneObject* floor = mScene->CreateSceneObject(
        Vector3f(0, 0, 0), Vector3f(0, 0, 0), 1.0f
    );
    floor->SetMaterial(mScene->CreateMaterial<LambertMaterial>("White", Color(1.0f, 1.0f, 1.0f)));
    // 三角形 1：左前 → 右前 → 右后（法线朝上）
    floor->CreatePrimitive<Triangle>(
        Vector3f(-halfSize, 0, halfSize),   // 左前
        Vector3f(halfSize, 0, halfSize),    // 右前
        Vector3f(halfSize, 0, -halfSize)    // 右后
    );

    // 三角形 2：左前 → 右后 → 左后（法线朝上）
    floor->CreatePrimitive<Triangle>(
        Vector3f(-halfSize, 0, halfSize),   // 左前
        Vector3f(halfSize, 0, -halfSize),   // 右后
        Vector3f(-halfSize, 0, -halfSize)   // 左后
    );

    // ---- 天花板（y = roomSize）----
    SceneObject* ceiling = mScene->CreateSceneObject(
        Vector3f(0, roomSize, 0), Vector3f(0, 0, 0), 1.0f
    );
    ceiling->SetMaterial(mScene->CreateMaterial<LambertMaterial>("White", Color(1.0f, 1.0f, 1.0f)));
    // 顶点在局部空间中 y=0（SceneObject 平移至 y=4），法线朝下
    ceiling->CreatePrimitive<Triangle>(
        Vector3f(-halfSize, 0, -halfSize),  // 左后
        Vector3f(halfSize, 0, -halfSize),    // 右后
        Vector3f(halfSize, 0, halfSize)      // 右前
    );
    ceiling->CreatePrimitive<Triangle>(
        Vector3f(-halfSize, 0, -halfSize),  // 左后
        Vector3f(halfSize, 0, halfSize),      // 右前
        Vector3f(-halfSize, 0, halfSize)      // 左前
    );

    // ---- 右墙（x = -halfSize，红色）----
    SceneObject* leftWall = mScene->CreateSceneObject(
        Vector3f(-halfSize, roomSize * 0.5f, 0), Vector3f(0, 0, 0), 1.0f
    );
	leftWall->SetMaterial(mScene->CreateMaterial<LambertMaterial>("Red", Color(1.0f, 0.0f, 0.0f)));
    // 顶点在局部空间中 x=0（SceneObject 平移至 x=-2），法线朝 +X
    leftWall->CreatePrimitive<Triangle>(
        Vector3f(0, -halfSize, halfSize),   // 下前
        Vector3f(0, -halfSize, -halfSize),  // 下后
        Vector3f(0, halfSize, -halfSize)  // 上后
    );
    leftWall->CreatePrimitive<Triangle>(
        Vector3f(0, -halfSize, halfSize),   // 下前
        Vector3f(0, halfSize, -halfSize),  // 上后
        Vector3f(0, halfSize, halfSize)    // 上前
    );

    // ---- 左墙（x = halfSize，绿色）----
    SceneObject* rightWall = mScene->CreateSceneObject(
        Vector3f(halfSize, roomSize * 0.5f, 0), Vector3f(0, 0, 0), 1.0f
    );
    rightWall->SetMaterial(mScene->CreateMaterial<LambertMaterial>("Blue", Color(0.0f, 0.0f, 1.0f)));
    // 顶点在局部空间中 x=0（SceneObject 平移至 x=2），法线朝 -X
    rightWall->CreatePrimitive<Triangle>(
        Vector3f(0, -halfSize, -halfSize),  // 下后
        Vector3f(0, -halfSize, halfSize),    // 下前
        Vector3f(0, halfSize, halfSize)  // 上前
    );
    rightWall->CreatePrimitive<Triangle>(
        Vector3f(0, -halfSize, -halfSize),  // 下后
        Vector3f(0, halfSize, halfSize),  // 上前
        Vector3f(0, halfSize, -halfSize)  // 上后
    );

    // ---- 后墙（z = -halfSize，白色）----
    SceneObject* backWall = mScene->CreateSceneObject(
        Vector3f(0, roomSize * 0.5f, -halfSize), Vector3f(0, 0, 0), 1.0f
    );
    backWall->SetMaterial(mScene->CreateMaterial<LambertMaterial>("White", Color(1.0f, 1.0f, 1.0f)));
    // 顶点在局部空间中 z=0（SceneObject 平移至 z=-2），法线朝 +Z（朝房间内部）
    backWall->CreatePrimitive<Triangle>(
        Vector3f(-halfSize, -halfSize, 0),  // 左下
        Vector3f(halfSize, halfSize, 0),  // 右上
        Vector3f(-halfSize, halfSize, 0)  // 左上
    );
    backWall->CreatePrimitive<Triangle>(
        Vector3f(-halfSize, -halfSize, 0),  // 左下
        Vector3f(halfSize, -halfSize, 0),   // 右下
        Vector3f(halfSize, halfSize, 0)  // 右上
    );

    // ---- 前面（z = halfSize）----
    // 通常省略或作为开口，让相机能看到里面
    // 如果要封闭，加上白色 Quad，法线朝内

    // ========== 天花板光源（区域光）==========
    //SceneObject* lightObj = mScene->CreateSceneObject(
    //    Vector3f(0, roomSize - 0.01f, 0), Vector3f(0, 0, 0), 1.0f
    //);
    //float lightSize = 0.5f;
    //// 顶点在局部空间中 y=0（SceneObject 平移至 y=3.99），法线朝下
    //// 三角形 1
    //lightObj->CreatePrimitive<Triangle>(
    //    Vector3f(-lightSize, 0, lightSize),  // 左前
    //    Vector3f(-lightSize, 0, -lightSize), // 左后
    //    Vector3f(lightSize, 0, -lightSize)   // 右后
    //);

    //// 三角形 2
    //lightObj->CreatePrimitive<Triangle>(
    //    Vector3f(-lightSize, 0, lightSize),  // 左前
    //    Vector3f(lightSize, 0, -lightSize),  // 右后
    //    Vector3f(lightSize, 0, lightSize)    // 右前
    //);

    // ========== 内部物体 ==========

    // ---- 大盒子（右侧，旋转）----
    //SceneObject* tallBox = mScene->CreateSceneObject(
    //    Vector3f(0.8f, 0.0f, 0.5f),           // 位置
    //    Vector3f(0, glm::radians(-15.0f), 0), // 旋转（绕Y轴-15度）
    //    1.0f
    //);
    //float box1W = 0.8f, box1H = 1.6f, box1D = 0.8f;
    //CreateBox(tallBox, box1W, box1H, box1D);

    //// ---- 小盒子（左侧，旋转）----
    //SceneObject* shortBox = mScene->CreateSceneObject(
    //    Vector3f(-0.8f, 0.0f, -0.5f),          // 位置
    //    Vector3f(0, glm::radians(15.0f), 0),   // 旋转（绕Y轴15度）
    //    1.0f
    //);
    /*float box2W = 0.8f, box2H = 0.8f, box2D = 0.8f;
    CreateBox(shortBox, box2W, box2H, box2D);

    SceneObject* pSceneObject = mScene->CreateSceneObject(
        Vector3f(0, 0, 0), Vector3f(0, 0, 0), 1.0f
    );
    pSceneObject->CreatePrimitive<Sphere>(0.3f);*/

    mScene->CreateLight<PointLight>(
        Vector3f(0, 3.5f, 0),      // 位置
        Color(1, 1, 1),             // 颜色
        Vector3f(0.1f, 0.1f, 0.1f) // 衰减
    );
}

// 辅助函数：创建轴对齐盒子（6个面，每个面2个三角形）
void Renderer::CreateBox(SceneObject* obj, float w, float h, float d)
{
    float hw = w * 0.5f, hh = h * 0.5f, hd = d * 0.5f;

    // 底面（y = 0，面朝下）
    obj->CreatePrimitive<Triangle>(
        Vector3f(-hw, 0, -hd),   // 左后
        Vector3f(hw, 0, -hd),   // 右后
        Vector3f(hw, 0, hd)    // 右前
    );
    obj->CreatePrimitive<Triangle>(
        Vector3f(-hw, 0, -hd),   // 左后
        Vector3f(hw, 0, hd),   // 右前
        Vector3f(-hw, 0, hd)    // 左前
    );

    // 顶面（y = h，面朝上）
    obj->CreatePrimitive<Triangle>(
        Vector3f(-hw, h, hd),   // 左前
        Vector3f(hw, h, hd),   // 右前
        Vector3f(hw, h, -hd)    // 右后
    );
    obj->CreatePrimitive<Triangle>(
        Vector3f(-hw, h, hd),   // 左前
        Vector3f(hw, h, -hd),   // 右后
        Vector3f(-hw, h, -hd)    // 左后
    );

    // 前面（z = hd，面朝前）
    obj->CreatePrimitive<Triangle>(
        Vector3f(-hw, 0, hd),    // 左下
        Vector3f(hw, 0, hd),    // 右下
        Vector3f(hw, h, hd)     // 右上
    );
    obj->CreatePrimitive<Triangle>(
        Vector3f(-hw, 0, hd),    // 左下
        Vector3f(hw, h, hd),    // 右上
        Vector3f(-hw, h, hd)     // 左上
    );

    // 后面（z = -hd，面朝后）
    obj->CreatePrimitive<Triangle>(
        Vector3f(hw, 0, -hd),   // 右下
        Vector3f(-hw, 0, -hd),   // 左下
        Vector3f(-hw, h, -hd)    // 左上
    );
    obj->CreatePrimitive<Triangle>(
        Vector3f(hw, 0, -hd),   // 右下
        Vector3f(-hw, h, -hd),   // 左上
        Vector3f(hw, h, -hd)    // 右上
    );

    // 左面（x = -hw，面朝左）
    obj->CreatePrimitive<Triangle>(
        Vector3f(-hw, 0, -hd),   // 下后
        Vector3f(-hw, 0, hd),   // 下前
        Vector3f(-hw, h, hd)    // 上前
    );
    obj->CreatePrimitive<Triangle>(
        Vector3f(-hw, 0, -hd),   // 下后
        Vector3f(-hw, h, hd),   // 上前
        Vector3f(-hw, h, -hd)    // 上后
    );

    // 右面（x = hw，面朝右）
    obj->CreatePrimitive<Triangle>(
        Vector3f(hw, 0, hd),    // 下前
        Vector3f(hw, 0, -hd),    // 下后
        Vector3f(hw, h, -hd)     // 上后
    );
    obj->CreatePrimitive<Triangle>(
        Vector3f(hw, 0, hd),    // 下前
        Vector3f(hw, h, -hd),    // 上后
        Vector3f(hw, h, hd)     // 上前
    );
}

Renderer::~Renderer()
{
    if (mScene)
        delete mScene;
}

void Renderer::Run()
{
    struct mfb_window* window = mfb_open_ex("Ignore Renderer", mViewportWidth, mViewportHeight, MFB_WF_RESIZABLE);
    if (window == NULL)
        return;

    //视口上每个像素点的颜色，格式为0xAARRGGBB
    mBuffer = (uint32_t*)malloc(mViewportWidth * mViewportHeight * 4);

    /*std::thread renderThread(&Renderer::RunRenderThread, this);
	renderThread.detach();*/

	int numThreads = std::thread::hardware_concurrency();
	std::vector<std::thread> renderThreads(numThreads - 1);
	std::cout << "Using " << numThreads << " threads for rendering." << std::endl;
    for(int i = 0; i < numThreads - 1; ++i)
    {
        renderThreads[i] = std::thread(&Renderer::RunRenderThread, this);
		renderThreads[i].detach();
	}

    mfb_update_state state;
    do {
        // TODO: add some fancy rendering to the buffer of size 800 * 600

        state = mfb_update_ex(window, mBuffer, mViewportWidth, mViewportHeight);

        if (state != MFB_STATE_OK)
            break;

    } while (mfb_wait_sync(window));

    free(mBuffer);
    mBuffer = NULL;
    window = NULL;

    return;
}

Color Renderer::RenderPixel(int x, int y)
{
	//SSAA 抗锯齿采样，随机采样4次，取平均值
    Color resultColor(0, 0, 0);

    for (int i = 0; i < SamplePerPixel; i++)
    {
        //(x,y) - (x+1,y+1)范围内随机采样
        float px = x + glm::linearRand(0.0f, 1.0f);
        float py = y + glm::linearRand(0.0f, 1.0f);

		resultColor += RenderSubPixel(px, py) / (float)SamplePerPixel;
    }

	return resultColor; //返回平均颜色
}

Color Renderer::RenderSubPixel(float x, float y)
{
    Ray ray = mScene->GetCamera().GetRay(x, y);
    Color color = GetRadiance(ray, 0, false);

    return color;
}

//Color Renderer::GetIrradiance(const Ray& ray)
//{
//	Intersection isect;
//	if (!mScene->Intersect(ray, isect))
//		return Color(0, 0, 0); //如果没有交点，返回黑色
//
//    //E(p) = ∫ L(p, ω) * cosθ dω
//	Color E(0, 0, 0);
//    for(Light* pLight : mScene->GetLights())
//    {
//		Vector3f sourcePos;
//		Color L = pLight->GetRadiance(isect.position, sourcePos);
//
//        //求shadowRay
//		Ray shadowRay;
//		shadowRay.o = isect.position;
//		shadowRay.d = glm::normalize(sourcePos - isect.position);
//		shadowRay.mint = 1e-4f; //避免自相交
//		shadowRay.maxt = glm::length(sourcePos - isect.position);
//
//		Intersection shadow_isect;
//        if(mScene->Intersect(shadowRay, shadow_isect)) //如果有遮挡，说明该点在阴影中
//			continue;
//
//        float cosTheta = glm::dot(isect.normal, shadowRay.d);
//
//		E += L * glm::max(cosTheta, 0.0f); //只考虑正向光照
//	}
//
//    return E;
//}

Color Renderer::GetRadiance(const Ray& ray, int depth, bool bPrevIsSpecular)
{
    if (depth > mMaxDepth)
        return Color(0, 0, 0);

    //俄罗斯轮盘
    static const float SurvivalProbability = 0.8f;
    float RewardFactor = 1.0f;
    if (depth >= mMinDepth)
    {
        float K = Random01();
        if (K > SurvivalProbability)
        {
            return Color(0, 0, 0);
        }
        RewardFactor = 1.0f / SurvivalProbability;
    }

    Intersection isect;
    SceneObject* pSceneObject = mScene->Intersect(ray, isect);
    if (!pSceneObject)
        return Color(0, 0, 0); //如果没有交点，返回黑色

	Material* pMaterial = pSceneObject->GetMaterial();

    Color Lo(0, 0, 0);

	Matrix3x3 localToWorld = MakeCoordinateSystem(isect.normal);
	Matrix3x3 worldToLocal = glm::transpose(localToWorld);

	Vector3f wo = worldToLocal * (-ray.d); //出射方向，局部空间

    //自发光
	Color Le = pSceneObject->GetEmission();
    if (bPrevIsSpecular || depth == 0)
    {
	    Lo += Le;
    }

	//计算直接光照贡献
    if (!pMaterial->IsSpecular())
    {
        for (Light* pLight : mScene->GetLights())
        {
            Vector3f sourcePos;
            float pdf;
            Color L = pLight->GetRadiance(isect.position, sourcePos, pdf);

            //求shadowRay
            Ray shadowRay;
            shadowRay.o = isect.position;
            shadowRay.d = glm::normalize(sourcePos - isect.position);
            shadowRay.mint = 1e-4f; //避免自相交
            shadowRay.maxt = glm::length(sourcePos - isect.position) - 1e-3f;

            //Intersection shadow_isect;
            //if (mScene->Intersect(shadowRay, shadow_isect)) //如果有遮挡，说明该点在阴影中
            //    continue;

            if (mScene->Occluded(shadowRay)) //如果有遮挡，说明该点在阴影中
                continue;

            Vector3f wi = worldToLocal * shadowRay.d; //入射方向，局部空间
            float cosTheta = glm::dot(isect.normal, shadowRay.d);
            Color brdf(1, 1, 1);
            brdf = pMaterial->BRDF(wo, wi);
            Lo += brdf * L * glm::max(cosTheta, 0.0f) / std::max(pdf, 1e-5f); //只考虑正向光照
        }
    }
    
	//计算间接光照贡献（递归）
    //反射
    {
        float pdf = 0.0f;
        Vector3f wi = pMaterial->SampleWi(wo, pdf);
        Color brdf = pMaterial->BRDF(wo, wi);
        Ray r;
        r.d = localToWorld * wi;
        r.o = isect.position;
        r.mint = 1e-3f;
        Color Li = GetRadiance(r, depth + 1, pMaterial->IsSpecular());
        Lo += brdf * Li * fabs(wi.z) / std::max(pdf, 1e-5f);
    }
    //折射
    {
        Vector3f wt;
        if (pMaterial->SampleWt(wo, wt))
        {
            Color btdf = pMaterial->BTDF(wo, wt);
            Ray r;
            r.d = localToWorld * wt;
            r.o = isect.position;
            r.mint = 1e-3f;

            Color Li = GetRadiance(r, depth + 1, pMaterial->IsSpecular());
            Lo += btdf * Li * fabs(wt.z);
        }
    }

    return Lo * RewardFactor;
}

//渲染线程入口函数
void Renderer::RunRenderThread()
{
    //读取当前屏幕的下一个像素
    auto start = std::chrono::steady_clock::now();
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    char buf[26];  // ctime_s 的缓冲区至少 26 字节
    ctime_s(buf, sizeof(buf), &t);
    std::cout << buf;   // buf 自带换行
    int pixelIndex = 0;
    while (pixelIndex < mViewportWidth * mViewportHeight)
    {
		pixelIndex = mCurrentPixelIndex.fetch_add(1);
        if(pixelIndex >= mViewportWidth * mViewportHeight)
            break;

		int x = pixelIndex % mViewportWidth;
		int y = pixelIndex / mViewportWidth;

        Color color = RenderPixel(x, y);
        uint32_t r = glm::clamp((uint32_t)std::round(color.r * 255.0f), 0u, 255u);
        uint32_t g = glm::clamp((uint32_t)std::round(color.g * 255.0f), 0u, 255u);
        uint32_t b = glm::clamp((uint32_t)std::round(color.b * 255.0f), 0u, 255u);
		mBuffer[y * mViewportWidth + x] = (r << 16) | (g << 8) | b;
    }
    auto end = std::chrono::steady_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Render time: " << ms.count() << " ms" << std::endl;
}

