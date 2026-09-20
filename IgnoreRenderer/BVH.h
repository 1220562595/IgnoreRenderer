// BVH.h
#pragma once
#include "Primitive.h"
#include <vector>
#include <algorithm>
#include <cmath>

class BVH
{
public:
    void Build(const std::vector<Primitive*>& primitives);
    bool Intersect(const Ray& ray, Intersection& isect, const Primitive*& pHitPrim) const;
    bool Occluded(const Ray& ray) const;   // 阴影射线专用：命中即返回

private:
    struct Node
    {
        AABB bounds;
        int left = -1, right = -1;  // 内部节点
        int start = 0, count = 0;   // 叶子节点
        bool IsLeaf() const { return count > 0; }
    };

    int BuildNode(int start, int end);

    std::vector<Primitive*> mPrimitives;  // 建树时会被重排
    std::vector<AABB> mBounds;            // 与 mPrimitives 一一对应
    std::vector<int> mIndices;
    std::vector<Node> mNodes;
};
