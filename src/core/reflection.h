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
    MicrofacetDistribution(std::string strType);

    inline EType getType() const { return m_type; }
    bool isAnisotropic() const { return m_type == EAnisotropic; }
    float unifyRoughness(float value) const; // beckmann --> phong
    float eval(const Normal3f& h, float alphaU, float alphaV) const;
    float pdf(const Normal3f& h, float alphaU, float alphaV) const;

    Normal3f sample(const Point2f &sample, float alphaU, float alphaV) const;
    float G(const Vector3f& wi, const Vector3f& wo,
                   const Vector3f& wh, float alphaU, float alphaV);

    inline float eval(const Normal3f& h, float alpha) const
    {
        return eval(h, alpha, alpha);
    }

    inline float pdf(const Normal3f& n, float alpha) const
    {
        return pdf(n, alpha, alpha);
    }

    inline Normal3f sample(const Point2f& s, float alpha) const
    {
        return sample(s, alpha, alpha);
    }

    inline float G(const Vector3f& wi, const Vector3f& wo,
                   const Vector3f& n, float alpha)
    {
        return G(wi, wo, n, alpha, alpha);
    }

private:
    float smithG1(const Vector3f& v, const Vector3f& m, float alpha) const;
    void sampleFirstQuadrant(float alphaU, float alphaV, float u1, float u2,
            float& phi, float& cosTheta) const;

private:
    EType m_type;
};

WISP_NAMESPACE_END
