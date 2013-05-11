#include "geometry.h"

WISP_NAMESPACE_BEGIN
BBox merge(const BBox& b, const Point3f& p)
{
    BBox ret = b;
    ret.pMin.x = std::min(b.pMin.x, p.x);
    ret.pMin.y = std::min(b.pMin.y, p.y);
    ret.pMin.z = std::min(b.pMin.z, p.z);

    ret.pMax.x = std::max(b.pMax.x, p.x);
    ret.pMax.y = std::max(b.pMax.y, p.y);
    ret.pMax.z = std::max(b.pMax.z, p.z);
    return ret;
}

BBox merge(const BBox& b1, const BBox& b2)
{
    BBox ret;
    ret.pMin.x = std::min(b1.pMin.x, b2.pMin.x);
    ret.pMin.y = std::min(b1.pMin.y, b2.pMin.y);
    ret.pMin.z = std::min(b1.pMin.z, b2.pMin.z);

    ret.pMax.x = std::max(b1.pMax.x, b2.pMax.x);
    ret.pMax.y = std::max(b1.pMax.y, b2.pMax.y);
    ret.pMax.z = std::max(b1.pMax.z, b2.pMax.z);
    return ret;
}

bool BBox::rayIntersect(const Ray& ray, float& tt0, float& tt1)
{
    float t0 = ray.mint, t1 = ray.maxt;

    for (int i = 0; i < 3; ++i)
    {
        float invRayDir = 1.f / ray.d[i];
        float tNear = (pMin[i] - ray.o[i]) * invRayDir;
        float tFar  = (pMax[i] - ray.o[i]) * invRayDir;
        if (tNear > tFar)
            std::swap(tNear, tFar);
        t0 = (tNear > t0 ? tNear : t0);
        t1 = (tFar  < t1 ? tFar  : t1);
        if (t0 > t1)
            return false;
    }

    tt0 = t0;
    tt1 = t1;
    return true;
}

WISP_NAMESPACE_END
