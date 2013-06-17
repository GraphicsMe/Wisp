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
        printf ("kd: %f %f %f\tks: %f\n", m_kd.x, m_kd.y, m_kd.z, m_ks);
        m_fresnel = new FresnelDielectric(m_intIOR, m_extIOR);
        m_distribution = new MicrofacetDistribution(paramSet.getString("type", "beckmann"));
        m_specularSamplingWeight = 0.5f;
        m_hasDiffuse = true;
        m_hasSpecular = true;
    }

    ~Microfacet()
    {
        delete m_fresnel;
        delete m_distribution;
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
        float probSpecular = m_specularSamplingWeight;
        bool choseSpeuclar = m_hasSpecular;
        if (m_hasSpecular && m_hasDiffuse)
        {
            if (sample.y <= probSpecular)
                sample.y /= probSpecular;
            else
            {
                sample.y = (sample.y - probSpecular) / (1.f - probSpecular);
                choseSpeuclar = false;
            }
        }

        if (choseSpeuclar)
        {
            float alphaT = m_distribution->unifyRoughness(m_alpha);
            Normal3f h = m_distribution->sample(sample, alphaT);
            bRec.wi = reflect(bRec.wo, h);
            bRec.sampledType = EGlossyReflection;
            if (Frame::cosTheta(bRec.wi) <= 0.f)
                return Color3f(0.f);
        }
        else
        {
            bRec.wi = cosineHemisphere(sample.x, sample.y);
            bRec.sampledType = EDiffuseReflection;
        }

        pdf = this->pdf(bRec);
        if (pdf == 0.f)
            return Color3f(0.f);

        return this->eval(bRec) / pdf;
    }

    Color3f eval(const BSDFQueryRecord& bRec) const
    {
        if (Frame::cosTheta(bRec.wi) <= 0.f ||
            Frame::cosTheta(bRec.wo) <= 0.f)
            return Color3f(0.f);

        float alphaT = m_distribution->unifyRoughness(m_alpha);

        Color3f result(0.f);

        // diffuse
        if (m_hasDiffuse)
            result += m_kd * INV_PI * Frame::cosTheta(bRec.wi);

        // specular
        if (m_hasSpecular)
        {
            Vector3f H = glm::normalize(bRec.wi + bRec.wo);
            float D = m_distribution->eval(H, alphaT);
            Color3f F = m_fresnel->eval(glm::dot(bRec.wi, H));
            float G = m_distribution->G(bRec.wi, bRec.wo, H, alphaT);
            result += m_ks * D * F * G / (4.f * Frame::cosTheta(bRec.wo));
        }

        assert (isValid(result));
        return result;
    }

    float pdf(const BSDFQueryRecord& bRec) const
    {
        if (Frame::cosTheta(bRec.wi) <= 0.f ||
            Frame::cosTheta(bRec.wo) <= 0.f)
            return 0.f;

        float alphaT = m_distribution->unifyRoughness(m_alpha);
        Vector3f H = glm::normalize(bRec.wi + bRec.wo);

        float probSpecular = m_specularSamplingWeight;
        float probDiffuse = 1.f - probSpecular;
        if (!m_hasSpecular || !m_hasDiffuse)
            probDiffuse = probSpecular = 1.f;

        float result = 0.f;

        // specular
        if (m_hasSpecular)
        {
            float dhdo = 1.f / (4.f * glm::dot(bRec.wi, H));
            float pdfH = m_distribution->pdf(H, alphaT);
            result += pdfH * dhdo * probSpecular;
        }

        // diffuse
        if (m_hasDiffuse)
            result += Frame::cosTheta(bRec.wi) * INV_PI * probDiffuse;

        assert (isValid(result));
        return result;
    }

    std::string toString() const
    {
        return std::string(formatString("Microfacet[]"));
    }

private:
    bool m_hasDiffuse, m_hasSpecular;
    float m_alpha;
    float m_intIOR, m_extIOR;
    float m_ks;
    float m_specularSamplingWeight;
    Color3f m_kd;
    Fresnel* m_fresnel;
    MicrofacetDistribution* m_distribution;
};

WISP_REGISTER_CLASS(Microfacet, "microfacet")

WISP_NAMESPACE_END
