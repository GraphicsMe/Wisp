#ifndef LIGHT_H
#define LIGHT_H

#include "object.h"
#include "shape.h"

WISP_NAMESPACE_BEGIN

struct LightSamplingRecord
{
    float pdf;
    Vector3f d; // pointing away from the light source
    Color3f value;
    const Light* light;
    ShapeSamplingRecord sRec;
    inline LightSamplingRecord() : light(NULL) {}
    inline LightSamplingRecord(const Intersection& its, const Vector3f& dir)
    {
        sRec.p = its.p;
        sRec.n = its.geoFrame.n;
        d = dir;
        light = its.shape->getLight();
    }
};

class Light : public Object
{
public:
    virtual ~Light() {}

    virtual Color3f Le(const Vector3f& dir) const { assert(0); return Color3f(0.f); }
    virtual Color3f Le(const ShapeSamplingRecord& sRec, const Vector3f& dir) const { assert(0); return Color3f(0.f); }
    virtual void sample_f(const Point3f& p, LightSamplingRecord& lRec, const Point2f& sample) const = 0;
    virtual float pdf(const Point3f& p, const LightSamplingRecord& lRec) const { assert(0); return 0.f; }

    EClassType getClassType() const { return ELuminaire; }
};

WISP_NAMESPACE_END

#endif // LIGHT_H
