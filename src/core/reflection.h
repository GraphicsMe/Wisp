#include "common.h"
#include "frame.h"

WISP_NAMESPACE_BEGIN

Color3f fresnelCond(float cosi, const Color3f& eta, const Color3f& k);
Color3f fresnelDiel(float cosi, float cost, const Color3f& etaI, const Color3f& etaT);

class Fresnel
{
public:
    virtual ~Fresnel() {}
    virtual Color3f eval(float cosi) const = 0;
};

class FresnelConductor : public Fresnel
{
public:
    FresnelConductor(const Color3f& e, const Color3f& kk)
        : m_eta(e), m_k(kk) {}
    ~FresnelConductor() {}

    Color3f eval(float cosi) const;

private:
    Color3f m_eta;
    Color3f m_k; // absortion coefficient
};

class FresnelDielectric : public Fresnel
{
public:
    FresnelDielectric(float ei, float et) : m_etaI(ei), m_etaT(et) {}
    ~FresnelDielectric() {}

    Color3f eval(float cosi) const;

private:
    float m_etaI, m_etaT;
};

class MicrofacetDistribution
{
public:
    enum EType
    {
        EBeckmann       = 0,
        EGGX            = 1,
        EPhong          = 2,
        EAnisotropic    = 3
    };

    MicrofacetDistribution(EType type) : m_type(type) {}

    MicrofacetDistribution(std::string strType)
    {
        if (strType == "beckmann")
            m_type = EBeckmann;
        else if (strType == "phong")
            m_type = EPhong;
        else if (strType == "ggx")
            m_type = EGGX;
        else if (strType == "anisotropic")
            m_type = EAnisotropic;
        else
            throw WispException("Specified an invalid distribution");
    }

    inline EType getType() const { return m_type; }
    bool isAnisotropic() const { return m_type == EAnisotropic; }

    // convert to beckmann style
    float unifyRoughness(float value) const {
        value = std::max(value, 1e-5f);
        if (m_type == EPhong || m_type == EAnisotropic)
            value = std::max(2.f / (value*value) - 2.f, 0.1f);
        return value;
    }

    inline float eval(const Normal3f& h, float alpha) const
    {
        return eval(h, alpha, alpha);
    }

    float eval(const Normal3f& h, float alphaU, float alphaV) const
    {
        if (Frame::cosTheta(h) <= 0)
            return 0.0f;
        float result;
        switch(m_type)
        {
        case EBeckmann:{
            float ex = Frame::tanTheta(h) / alphaU;
            result = std::exp(-(ex*ex)) / (M_PI * alphaU*alphaU *
                    std::pow(Frame::cosTheta(h), 4.0f));
            }
            break;
        case EGGX:{
            assert (0);
            }
            break;
        case EPhong:{
            result = (alphaU + 2.f) * INV_TWOPI
                    * std::pow(Frame::cosTheta(h), alphaU);
            }
            break;
        case EAnisotropic:{
            assert (0);
            }
            break;
        }

        if (result < 1e-20f)
            result = 0.f;
        return result;
    }

    inline float pdf(const Normal3f& n, float alpha) const
    {
        return pdf(n, alpha, alpha);
    }

    float pdf(const Normal3f& h, float alphaU, float alphaV) const
    {
        return eval(h, alphaU, alphaV) * Frame::cosTheta(h);
    }

    inline Normal3f sample(const Point2f& s, float alpha) const
    {
        return sample(s, alpha, alpha);
    }

    Normal3f sample(const Point2f &sample, float alphaU, float alphaV) const
    {
        float cosThetaH = 0.f, phiH = 2.f * M_PI / sample.y;
        switch(m_type)
        {
        case EBeckmann:
            float tanThetaMSqr = -alphaU*alphaU * std::log(1.0f - sample.x);
            cosThetaH = 1.0f / std::sqrt(1 + tanThetaMSqr);
            break;
        }

        float sinThetaH = std::sqrt(std::max(0.f, 1.f - cosThetaH*cosThetaH));

        return Normal3f(sinThetaH * std::cos(phiH),
                        sinThetaH * std::sin(phiH),
                        cosThetaH);
    }

    inline float G(const Vector3f& wi, const Vector3f& wo,
                   const Vector3f& n, float alpha)
    {
        return G(wi, wo, n, alpha, alpha);
    }

    inline float G(const Vector3f& wi, const Vector3f& wo,
                   const Vector3f& wh, float, float)
    {
        if (glm::dot(wi, wh) * Frame::cosTheta(wi) <= 0.f ||
            glm::dot(wo, wh) * Frame::cosTheta(wo) <= 0.f)
            return 0.f;
        float NdotWh = Frame::cosTheta(wh);
        float NdotWo = Frame::cosTheta(wo);
        float NdotWi = Frame::cosTheta(wi);
        float WOdotWh = glm::dot(wo, wh);
        return glm::min(1.f, glm::min((2.f * NdotWh * NdotWo / WOdotWh),
                         (2.f * NdotWh * NdotWi / WOdotWh)));
        return 0.f;
    }

private:
    EType m_type;
};

WISP_NAMESPACE_END
