#include "bsdf.h"
#include "frame.h"

WISP_NAMESPACE_BEGIN

class Mirror : public BSDF
{
public:
    Mirror(const ParamSet&)
    {
    }

    Color3f sample_f(BSDFQueryRecord& bRec, const Point2f&) const
    {
        if (Frame::cosTheta(bRec.wo) <= 0.f)
            return Color3f(0.f);
        bRec.wi = Vector3f(-bRec.wo.x, -bRec.wo.y, bRec.wo.z);
        return Color3f(1.f);
    }

    virtual Color3f sample_f(BSDFQueryRecord& bRec, float& pdf, const Point2f&) const
    {
        if (Frame::cosTheta(bRec.wo) <= 0.f)
            return Color3f(0.f);
        bRec.wi = Vector3f(-bRec.wo.x, -bRec.wo.y, bRec.wo.z);
        pdf = 1.f;
        return Color3f(1.f);
    }

    Color3f eval(const BSDFQueryRecord&) const
    {
        return Color3f(0.f);
    }

    float pdf(const BSDFQueryRecord&) const
    {
        return 0.f;
    }

    std::string toString() const
    {
        return std::string("Mirror[]");
    }
};

WISP_REGISTER_CLASS(Mirror, "mirror")

WISP_NAMESPACE_END
