#ifndef PHASE_H
#define PHASE_H

#include "object.h"

WISP_NAMESPACE_BEGIN

struct PhaseFunctionQueryRecord
{
    Vector3f wi;
    Vector3f wo;

    inline PhaseFunctionQueryRecord(const Vector3f& wi)
        : wi(wi) {}
    inline PhaseFunctionQueryRecord(const Vector3f &wi,
        const Vector3f& wo) : wi(wi), wo(wo) {}
};

class PhaseFunction : public Object
{
public:
    virtual float pdf(const PhaseFunctionQueryRecord& rec) const = 0;
    virtual float eval(const PhaseFunctionQueryRecord& rec) const = 0;
    virtual float sample(PhaseFunctionQueryRecord& rec, const Point2f& sample) const = 0;
    virtual float sample(PhaseFunctionQueryRecord& rec, float& pdf, const Point2f& sample) const = 0;

    EClassType getClassType() const { return EPhaseFunction; }
};

WISP_NAMESPACE_END
#endif //PHASE_H
