#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "common.h"

WISP_NAMESPACE_BEGIN
class Ray
{
public:
    Point3f o;
    Vector3f d;
    float mint;
    mutable float maxt;

    inline Ray()
        : mint(Epsilon)
        , maxt(Infinity)
    {}

    inline Ray(const Point3f& o, const Vector3f& d)
        : o(o)
        , d(d)
        , mint(Epsilon)
        , maxt(Infinity)
    {}

    inline Ray(const Point3f& o, const Vector3f& d, float mint)
        : o(o)
        , d(d)
        , mint(mint)
        , maxt(Infinity)
    {}

    inline Ray(const Point3f &o, const Vector3f &d, float mint, float maxt)
        : o(o)
        , d(d)
        , mint(mint)
        , maxt(maxt)
    {}

    inline Ray(const Ray& ray)
        : o(ray.o)
        , d(ray.d)
        , mint(ray.mint)
        , maxt(ray.maxt)
    {}

    inline Ray(const Ray& ray, float mint, float maxt)
        : o(ray.o)
        , d(ray.d)
        , mint(mint)
        , maxt(maxt)
    {}

    inline Point3f operator() (float t) const { return o+t*d; }

    inline Ray reverse() const
    {
        Ray result(*this);
        result.d = -this->d;
    }
};

class BBox
{
public:
    Point3f pMin, pMax;

    BBox()
        : pMin(Infinity, Infinity, Infinity)
        , pMax(-Infinity, -Infinity, -Infinity)
    {}

    BBox(const Point3f& p) : pMin(p), pMax(p)
    {}

    BBox(const Point3f& p1, const Point3f& p2)
    {
        pMin = Point3f(std::min(p1.x, p2.x), std::min(p1.y, p2.y), std::min(p1.z, p2.z));
        pMax = Point3f(std::max(p1.x, p2.x), std::max(p1.y, p2.y), std::max(p1.z, p2.z));
    }

    friend BBox merge(const BBox& b, const Point3f& p);
    friend BBox merge(const BBox& b1, const BBox& b2);

    bool rayIntersect(const Ray& ray, float& t0, float& t1) const;

    bool overLaps(const BBox& b) const
    {
        bool x = (pMax.x >= b.pMin.x) && (pMin.x <= b.pMax.x);
        bool y = (pMax.y >= b.pMin.y) && (pMin.y <= b.pMax.y);
        bool z = (pMax.z >= b.pMin.z) && (pMin.z <= b.pMax.z);
        return (x && y && z);
    }

    bool inside(const Point3f& pt) const
    {
        return (pt.x >= pMin.x && pt.x <= pMax.x &&
                pt.y >= pMin.y && pt.y <= pMax.y &&
                pt.z >= pMin.z && pt.z <= pMax.z);
    }

    void expand(float delta)
    {
        Vector3f temp = Vector3f(delta, delta, delta);
        pMin -= temp;
        pMax += temp;
    }

    void expand(const BBox& box)
    {
        pMin.x = std::min(pMin.x, box.pMin.x);
        pMin.y = std::min(pMin.y, box.pMin.y);
        pMin.z = std::min(pMin.z, box.pMin.z);

        pMax.x = std::max(pMax.x, box.pMax.x);
        pMax.y = std::max(pMax.y, box.pMax.y);
        pMax.z = std::max(pMax.z, box.pMax.z);
    }

    void expand(const Point3f& pt)
    {
        pMin.x = std::min(pMin.x, pt.x);
        pMin.y = std::min(pMin.y, pt.y);
        pMin.z = std::min(pMin.z, pt.z);

        pMax.x = std::max(pMax.x, pt.x);
        pMax.y = std::max(pMax.y, pt.y);
        pMax.z = std::max(pMax.z, pt.z);
    }

    float surfaceArea() const
    {
        Vector3f d = pMax - pMin;
        return 2.0f * (d.x*d.y + d.y*d.z + d.z*d.x);
    }

    float volume() const
    {
        Vector3f d = pMax - pMin;
        return d.x * d.y * d.z;
    }

    Vector3f offset(const Point3f& p) const
    {
        Vector3f d = pMax - pMin;
        Vector3f off = p - pMin;
        return off / d;
    }

    Vector3f getExtents() const
    {
        return pMax - pMin;
    }

    int maximumExtent() const
    {
        Vector3f d = pMax - pMin;
        if (d.x > d.y && d.x > d.z)
            return 0;
        else if (d.y > d.z)
            return 1;
        else
            return 2;
    }

    bool operator == (const BBox& b) const
    {
        return b.pMin == pMin && b.pMax == pMax;
    }

    bool operator != (const BBox& b) const
    {
        return !(operator==(b));
    }
};

WISP_NAMESPACE_END
#endif // GEOMETRY_H
