#include "bsdf.h"
#include "frame.h"
#include "reflection.h"

WISP_NAMESPACE_BEGIN

class Microfacet : public BSDF
{
public:
    Microfacet(const ParamSet& paramSet)
    {
        m_alpha = paramSet.getFloat("alpha", 0.1f);
        m_intIOR = paramSet.getFloat("intIOR", 1.5046f); // glass
        m_extIOR = paramSet.getFloat("extIOR", 1.000277f); // air
        m_kd = paramSet.getColor("kd", Color3f(0.5f));
        m_ks = 1.f - vectorMax(m_kd);
        std::string strType = paramSet.getString("type", "phong");
        m_distribution = new MicrofacetDistribution(strType);
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

private:
    float m_alpha;
    float m_intIOR, m_extIOR;
    float m_ks;
    Color3f m_kd;
    MicrofacetDistribution* m_distribution;
};

WISP_REGISTER_CLASS(Microfacet, "microfacet")

WISP_NAMESPACE_END
