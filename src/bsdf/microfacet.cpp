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
        m_fresnel = new FresnelDielectric(m_intIOR, m_extIOR);
        m_distribution = new MicrofacetDistribution(paramSet.getString("type", "beckmann"));
    }

    inline Vector3f reflect(const Vector3f& wi, const Normal3f& n) const
    {
        return 2.f * glm::dot(wi, n) * n - wi;
    }

    Color3f sample_f(BSDFQueryRecord& bRec, const Point2f& sample) const
    {
        float pdf;
        return this->sample_f(bRec, pdf, sample);
    }

    Color3f sample_f(BSDFQueryRecord& bRec, float& pdf, const Point2f& _sample) const
    {
        if (Frame::cosTheta(bRec.wo) <= 0)
            return Color3f(0.0f);

        Point2f sample(_sample);
        float probSpecular = 0.5f;
        bool choseSpeuclar = true;
        if (sample.y <= probSpecular)
            sample.y /= probSpecular;
        else
        {
            sample.y = (sample.y - probSpecular) / (1.f - probSpecular);
            choseSpeuclar = false;
        }

        if (choseSpeuclar)
        {
            float alphaT = m_distribution->unifyRoughness(m_alpha);
            Normal3f n = m_distribution->sample(sample, alphaT);
            bRec.wi = reflect(bRec.wo, n);

            if (Frame::cosTheta(bRec.wi) <= 0.f)
                return Color3f(0.f);
        }
        else
        {
            bRec.wi = cosineHemisphere(sample.x, sample.y);
        }

        pdf = this->pdf(bRec);
        return this->eval(bRec) / pdf;
    }

    Color3f eval(const BSDFQueryRecord& bRec) const
    {
        if (Frame::cosTheta(bRec.wi) <= 0.f ||
            Frame::cosTheta(bRec.wo) <= 0.f)
            return Color3f(0.f);

        float alphaT = m_distribution->unifyRoughness(m_alpha);

        Color3f diffuse = m_kd * INV_PI;

        //specular
        Vector3f H = glm::normalize(bRec.wi + bRec.wo);
        float D = m_distribution->eval(H, alphaT);
        Color3f F = m_fresnel->eval(glm::dot(bRec.wi, H));
        float G = m_distribution->G(bRec.wi, bRec.wo, H, alphaT);
        Color3f specular = m_ks * D * F * G / (4.f * Frame::cosTheta(bRec.wo));
        return diffuse + specular;
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
    Fresnel* m_fresnel;
    MicrofacetDistribution* m_distribution;
};

WISP_REGISTER_CLASS(Microfacet, "microfacet")

WISP_NAMESPACE_END
