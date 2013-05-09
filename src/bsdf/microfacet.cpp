#include "bsdf.h"
#include "frame.h"

WISP_NAMESPACE_BEGIN

class Mirror : public BSDF
{
public:
    Mirror(const ParamSet&)
    {
    }

    ~Mirror()
    {
    }

    Color3f sample_f(BSDFQueryRecord& bRec, const Point2f&) const
    {
        if (Frame::cosTheta(bRec.wo) <= 0)
            return Color3f(0.0f);
        return Color3f(1.0f);
    }

    Color3f f(const BSDFQueryRecord& bRec) const
    {
        return Color3f(0.0f);
    }

    float pdf(const BSDFQueryRecord& bRec) const
    {
        assert (0);
        return 0.0f;
    }

    std::string toString() const
    {
        return std::string(formatString("Mirror[]"));
    }
};

WISP_REGISTER_CLASS(Mirror, "mirror")

WISP_NAMESPACE_END
