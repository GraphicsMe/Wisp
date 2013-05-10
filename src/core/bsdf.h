#ifndef BSDF_H
#define BSDF_H

#include "object.h"
#include "shape.h"
WISP_NAMESPACE_BEGIN

struct BSDFQueryRecord
{
    Vector3f wi;    // local frame
    Vector3f wo;    // local frame
    const Intersection& its;
    inline BSDFQueryRecord(const Intersection& its)
        : its(its), wo(its.wo){}
    inline BSDFQueryRecord(const Intersection& its, const Vector3f& wi)
        : its(its), wi(wi), wo(its.wo) {}
    inline BSDFQueryRecord(const Intersection& its, const Vector3f& wi, const Vector3f& wo)
        : its(its), wi(wi), wo(wo) {}
};

class BSDF : public Object
{
public:
    virtual ~BSDF() {}

    virtual float pdf(const BSDFQueryRecord& bRec) const = 0;
    virtual Color3f eval(const BSDFQueryRecord& bRec) const = 0;
    virtual Color3f sample_f(BSDFQueryRecord& bRec, const Point2f& sample) const = 0;
    virtual Color3f sample_f(BSDFQueryRecord& bRec, float& pdf, const Point2f& sample) const = 0;

    EClassType getClassType() const { return EBSDF; }
};

WISP_NAMESPACE_END

#endif // SCENE_H
