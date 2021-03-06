#include "phase.h"

WISP_NAMESPACE_BEGIN
class Isotropic : public PhaseFunction
{
public:
    Isotropic(const ParamSet&)
    {

    }

    float eval (const PhaseFunctionQueryRecord&) const
    {
        return INV_FOURPI;
    }

    float pdf(const PhaseFunctionQueryRecord&) const
    {
        return INV_FOURPI;
    }

    float sample(PhaseFunctionQueryRecord &rec, const Point2f &sample) const
    {
        rec.wo = uniformSphere(sample.x, sample.y);
        return 1.f;
    }

    virtual float sample(PhaseFunctionQueryRecord& rec, float& pdf, const Point2f& sample) const
    {
        rec.wo = uniformSphere(sample.x, sample.y);
        pdf = INV_FOURPI;
        return 1.f;
    }

    std::string toString() const
    {
        return std::string("Isotropic[]");
    }
};

WISP_REGISTER_CLASS(Isotropic, "isotropic")
WISP_NAMESPACE_END
