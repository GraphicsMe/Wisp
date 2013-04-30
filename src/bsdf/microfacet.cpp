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

    Color3f sample_f(const Vector3f& wo, Vector3f& wi, const Point2f&) const
    {
        if (Frame::cosTheta(wo) <= 0)
            return Color3f(0.0f);
        wi = Color3f(-wo.x, -wo.y, wo.z);
        return Color3f(1.0f);
    }

    Color3f f(const Vector3f& wo, const Vector3f& wi) const
    {
        return Color3f(0.0f);
    }

    float pdf(const Vector3f& wo, const Vector3f& wi) const
    {
        return 0.0f;
    }

    std::string toString() const
    {
        return std::string(formatString("Mirror[]"));
    }
};

WISP_REGISTER_CLASS(Mirror, "mirror")

WISP_NAMESPACE_END
