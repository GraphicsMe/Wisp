#ifndef BSDF_H
#define BSDF_H

#include "object.h"

WISP_NAMESPACE_BEGIN
class BSDF : public Object
{
public:
    virtual ~BSDF() {}

    virtual Color3f sample_f(const Vector3f& wo, Vector3f& wi, const Point2f& sample) const = 0;
    virtual Color3f f(const Vector3f& wo, const Vector3f& wi) const = 0;
    virtual float pdf(const Vector3f& wo, const Vector3f& wi) const = 0;

    EClassType getClassType() const { return EBSDF; }
};

WISP_NAMESPACE_END

#endif // SCENE_H
