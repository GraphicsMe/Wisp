#ifndef BSDF_H
#define BSDF_H

#include "object.h"

WISP_NAMESPACE_BEGIN

struct BSDFQueryRecord
{
    Vector3f wi;    // local frame
    Vector3f wo;    // local frame
    inline BSDFQueryRecord(const Vector3f& wo)
        : wo(wo) {}
    inline BSDFQueryRecord(const Vector3f &wo, const Vector3f& wi)
        : wo(wo), wi(wi) {}
};

class BSDF : public Object
{
public:
    virtual ~BSDF() {}

    virtual Color3f sample_f(BSDFQueryRecord& bRec, const Point2f& sample) const = 0;
    virtual Color3f f(const BSDFQueryRecord& bRec) const = 0;
    virtual float pdf(const BSDFQueryRecord& bRec) const = 0;

    EClassType getClassType() const { return EBSDF; }
};

WISP_NAMESPACE_END

#endif // SCENE_H
