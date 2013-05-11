#include "bsdf.h"
#include "frame.h"

WISP_NAMESPACE_BEGIN

class Microfacet : public BSDF
{
public:
    Microfacet(const ParamSet&)
    {
    }

    Color3f sample_f(BSDFQueryRecord& bRec, const Point2f& sample) const
    {
        bRec;
        sample;
        assert (0);
        if (Frame::cosTheta(bRec.wo) <= 0)
            return Color3f(0.0f);
        return Color3f(1.0f);
    }

    Color3f sample_f(BSDFQueryRecord& bRec, float& pdf, const Point2f& sample) const
    {
        pdf;
        bRec;
        sample;
        assert (0);
        if (Frame::cosTheta(bRec.wo) <= 0)
            return Color3f(0.0f);
        return Color3f(1.0f);
    }

    Color3f eval(const BSDFQueryRecord& bRec) const
    {
        bRec;
        assert (0);
        return Color3f(0.0f);
    }

    float pdf(const BSDFQueryRecord& bRec) const
    {
        bRec;
        assert (0);
        return 0.0f;
    }

    std::string toString() const
    {
        return std::string(formatString("Microfacet[]"));
    }
};

WISP_REGISTER_CLASS(Microfacet, "microfacet")

WISP_NAMESPACE_END
