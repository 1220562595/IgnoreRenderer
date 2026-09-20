// BVH.cpp
#include "BVH.h"

void BVH::Build(const std::vector<Primitive*>& primitives)
{
    mPrimitives = primitives;
    size_t n = mPrimitives.size();
    mBounds.resize(n);
    mIndices.resize(n);
    for (size_t i = 0; i < n; ++i)
    {
        mBounds[i] = mPrimitives[i]->GetWorldBounds();
        mIndices[i] = (int)i;
    }
    mNodes.clear();
    if (n > 0)
        BuildNode(0, (int)n);
}

int BVH::BuildNode(int start, int end)
{
    int idx = (int)mNodes.size();
    mNodes.emplace_back();

    AABB centroidBounds;
    for (int i = start; i < end; ++i)
    {
        int p = mIndices[i];
        mNodes[idx].bounds.Expand(mBounds[p]);        // ← 不用 node. 引用
        centroidBounds.Expand(mBounds[p].Centroid());
    }

    int count = end - start;
    if (count <= 4 || /* 质心退化 */ false)
    {
        mNodes[idx].start = start;
        mNodes[idx].count = count;
        return idx;
    }

    // 选质心跨度最大的轴
    Vector3f ext = centroidBounds.maxP - centroidBounds.minP;
    int axis = ext.x > ext.y ? (ext.x > ext.z ? 0 : 2)
        : (ext.y > ext.z ? 1 : 2);
    if (ext[axis] < 1e-6f)          // 质心都挤在一起，直接成叶子
    {
        mNodes[idx].start = start;
        mNodes[idx].count = count;
        return idx;
    }

    int mid = start + count / 2;    // 中位切分
    std::nth_element(mIndices.begin() + start, mIndices.begin() + mid,
        mIndices.begin() + end,
        [&](int a, int b) {
            return mBounds[a].Centroid()[axis] < mBounds[b].Centroid()[axis];
        });

    int left = BuildNode(start, mid);
    int right = BuildNode(mid, end);
    mNodes[idx].left = left;
    mNodes[idx].right = right;
    return idx;
}

bool BVH::Intersect(const Ray& ray, Intersection& isect, const Primitive*& pHitPrim) const
{
    if (mNodes.empty()) return false;

    bool hit = false;
    float maxt = ray.maxt;
    int stack[64];
    int sp = 0;
    stack[sp++] = 0;

    while (sp > 0)
    {
        const Node& node = mNodes[stack[--sp]];
        if (!node.bounds.Intersect(ray, maxt))
            continue;

        if (node.IsLeaf())
        {
            for (int i = 0; i < node.count; ++i)
            {
                const Primitive* prim = mPrimitives[mIndices[node.start + i]];
                Intersection temp;
                if (prim->Intersect(ray, temp) && temp.t < maxt)
                {
                    maxt = temp.t;          // 命中后收紧 maxt，剪掉更远分支
                    isect = temp;
                    pHitPrim = prim;
                    hit = true;
                }
            }
        }
        else
        {
            stack[sp++] = node.left;
            stack[sp++] = node.right;
        }
    }
    return hit;
}

bool BVH::Occluded(const Ray& ray) const
{
    if (mNodes.empty()) return false;

    int stack[64];
    int sp = 0;
    stack[sp++] = 0;

    while (sp > 0)
    {
        const Node& node = mNodes[stack[--sp]];
        if (!node.bounds.Intersect(ray, ray.maxt))
            continue;

        if (node.IsLeaf())
        {
            for (int i = 0; i < node.count; ++i)
            {
                Intersection temp;
                if (mPrimitives[mIndices[node.start + i]]->Intersect(ray, temp))
                    return true;            // 阴影射线：随便命中一个就行
            }
        }
        else
        {
            stack[sp++] = node.left;
            stack[sp++] = node.right;
        }
    }
    return false;
}