#include "bsdf.h"
#include "frame.h"

WISP_NAMESPACE_BEGIN

class Diffuse : public BSDF
{
public:
    Diffuse(const ParamSet& paramSet)
    {
        m_albedo = paramSet.getColor("albedo", Color3f(0.5f));
    }

    ~Diffuse()
    {
    }

    Color3f sample_f(BSDFQueryRecord& bRec, const Point2f& sample) const
    {
        bRec.wi = cosineHemisphere(sample.x, sample.y);
        return m_albedo;
    }

    Color3f f(const BSDFQueryRecord& bRec) const
    {
        return m_albedo * INV_PI;
    }

    float pdf(const BSDFQueryRecord& bRec) const
    {
        if (Frame::cosTheta(bRec.wi) <= 0 || Frame::cosTheta(bRec.wo) <= 0)
            return 0.0f;
        return Frame::cosTheta(bRec.wi) * INV_PI;
    }

    std::string toString() const
    {
        return std::string(formatString("Diffuse[Albedo=(%.3f, %.3f, %.3f)]",
                                        m_albedo.x, m_albedo.y, m_albedo.z));
    }

private:
    Color3f m_albedo;
};

WISP_REGISTER_CLASS(Diffuse, "diffuse")

WISP_NAMESPACE_END
