#ifndef INTEGRATOR_H
#define INTEGRATOR_H

#include "object.h"

WISP_NAMESPACE_BEGIN
class Integrator : public Object
{
public:
    virtual ~Integrator() {}

    virtual Color3f Li(const Scene* scene, Sampler* sampler, const TRay& ray) const = 0;

    EClassType getClassType() const { return EIntegrator; }
};

WISP_NAMESPACE_END

#endif // INTEGRATOR_H
