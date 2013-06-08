#include "reflection.h"

WISP_NAMESPACE_BEGIN

Color3f fresnelCond(float cosi, const Color3f& eta, const Color3f& k)
{
    float c2 = cosi * cosi;
    Color3f tmpF = eta*eta + k*k;
    Color3f tmp = tmpF * c2;
    Color3f ec = 2.f * eta * cosi;

    Color3f rParl2 = (tmp - ec + Color3f(1.f)) / (tmp + ec + Color3f(1.f));
    Color3f rPerp2 = (tmpF - ec + c2) / (tmpF + ec + c2);
    return (rParl2 + rPerp2) / 2.f;
}

float fresnelDiel(float cosi, float cost, float etaI, float etaT)
{
    float rParl = ((etaT * cosi) - (etaI * cost)) /
                    ((etaT * cosi) + (etaI * cost));
    float rPerp = ((etaI * cosi) - (etaT * cost)) /
                    ((etaI * cosi) + (etaT * cost));
    return (rParl*rParl + rPerp*rPerp) / 2.f;
}

Color3f FresnelConductor::eval(float cosi) const
{
    return fresnelCond(cosi, m_eta, m_k);
}

Color3f FresnelDielectric::eval(float cosi) const
{
    cosi = clamp(cosi, -1.f, 1.f);

    float ei = m_etaI, et = m_etaT;
    bool entering = (cosi > 0.f);
    if (!entering)
        std::swap(ei, et);

    float sint = ei/et * std::sqrt(std::max(0.f, 1.f - cosi*cosi));
    if (sint >= 1.f)
        return Color3f(1.f); // total internal reflection
    else
    {
        float cost = std::sqrt(std::max(0.f, 1.f - sint*sint));
        float fr = fresnelDiel(std::abs(cosi), cost, ei, et);
        return Color3f(fr);
    }
}

MicrofacetDistribution::MicrofacetDistribution(std::string strType)
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

float MicrofacetDistribution::unifyRoughness(float value) const
{
    value = std::max(value, 1e-5f);
    if (m_type == EPhong || m_type == EAnisotropic)
        value = std::max(2.f / (value*value) - 2.f, 0.1f);
    return value;
}

float MicrofacetDistribution::eval(const Normal3f& h, float alphaU, float alphaV) const
{
    if (Frame::cosTheta(h) <= 0)
        return 0.0f;
    float result;
    switch(m_type)
    {
    case EBeckmann:
        {
            float ex = Frame::tanTheta(h) / alphaU;
            result = std::exp(-(ex*ex)) / (M_PI * alphaU*alphaU *
                    std::pow(Frame::cosTheta(h), 4.0f));
        }
        break;
    case EGGX:
        {
            float tanTheta = Frame::tanTheta(h), cosTheta = Frame::cosTheta(h);
            float root = alphaU / (cosTheta*cosTheta *
                                   (alphaU*alphaU + tanTheta*tanTheta));
            result = INV_PI * (root * root);
        }
        break;
    case EPhong:
        result = (alphaU + 2.f) * INV_TWOPI * std::pow(Frame::cosTheta(h), alphaU);
        break;
    case EAnisotropic:
        {
            float cosTheta = Frame::cosTheta(h);
            float ds = 1 - cosTheta*cosTheta;
            if (ds < 0.f)
                return 0.f;
            float exponent = (alphaU * h.x * h.x + alphaV * h.y * h.y) / ds;
            result = std::sqrt((alphaU + 2.f) * (alphaV + 2.f))
                    * INV_TWOPI * std::pow(cosTheta, exponent);
        }
        break;
    default:
        std::cerr << "Invalid distribution function!" << std::endl;
        return 0.f;
    }

    if (result < 1e-20f)
        result = 0.f;
    return result;
}

float MicrofacetDistribution::pdf(const Normal3f& h, float alphaU, float alphaV) const
{
    // D(h) * cos(theta_H) except anisotropic
    if (m_type != EAnisotropic)
        return eval(h, alphaU, alphaV) * Frame::cosTheta(h);

    float cosTheta = Frame::cosTheta(h);
    float ds = 1 - cosTheta * cosTheta;
    if (ds < 0)
        return 0.0f;
    float exponent = (alphaU * h.x * h.x + alphaV * h.y * h.y) / ds;
    float result = std::sqrt((alphaU + 1) * (alphaV + 1))
        * INV_TWOPI * std::pow(cosTheta, exponent);

    /* Prevent potential numerical issues in other stages of the model */
    if (result < 1e-20f)
        result = 0;

    return result;
}

void MicrofacetDistribution::sampleFirstQuadrant(float alphaU, float alphaV, float u1, float u2,
        float& phi, float& cosTheta) const
{
    if (alphaU == alphaV)
        phi = M_PI * u1 * 0.5f;
    else
        phi = std::atan(
            std::sqrt((alphaU + 1.0f) / (alphaV + 1.0f)) *
            std::tan(M_PI * u1 * 0.5f));
    float cosPhi = std::cos(phi), sinPhi = std::sin(phi);
    cosTheta = std::pow(u2, 1.0f /
        (alphaU * cosPhi * cosPhi + alphaV * sinPhi * sinPhi + 1.0f));
}

Normal3f MicrofacetDistribution::sample(const Point2f &sample, float alphaU, float alphaV) const
{
    float cosThetaH = 0.f, phiH = 2.f * M_PI * sample.y;
    switch(m_type)
    {
    case EBeckmann:
        {
            float tanThetaMSqr = -alphaU*alphaU * std::log(1.0f - sample.x);
            cosThetaH = 1.0f / std::sqrt(1 + tanThetaMSqr);
        }
        break;
    case EGGX:
        {
            float tanThetaMSqr = alphaU*alphaU * sample.x / (1.f - sample.x);
            cosThetaH = 1.f / std::sqrt(1 + tanThetaMSqr);
        }
        break;
    case EPhong:
            cosThetaH = std::pow(sample.x, 1.f / (alphaU + 2.f));
        break;
    case EAnisotropic:
        {
            if (sample.x < 0.25f) {
                sampleFirstQuadrant(alphaU, alphaV,
                    4 * sample.x, sample.y, phiH, cosThetaH);
            } else if (sample.x < 0.5f) {
                sampleFirstQuadrant(alphaU, alphaV,
                    4 * (0.5f - sample.x), sample.y, phiH, cosThetaH);
                phiH = M_PI - phiH;
            } else if (sample.x < 0.75f) {
                sampleFirstQuadrant(alphaU, alphaV,
                    4 * (sample.x - 0.5f), sample.y, phiH, cosThetaH);
                phiH += M_PI;
            } else {
                sampleFirstQuadrant(alphaU, alphaV,
                    4 * (1 - sample.x), sample.y, phiH, cosThetaH);
                phiH = 2 * M_PI - phiH;
            }
        }
        break;
    default:
        std::cerr << "Invalid distribution function!" << std::endl;
    }

    float sinThetaH = std::sqrt(std::max(0.f, 1.f - cosThetaH*cosThetaH));
    return Normal3f(sinThetaH * std::cos(phiH),
                    sinThetaH * std::sin(phiH),
                    cosThetaH);
}

float MicrofacetDistribution::smithG1(const Vector3f& v, const Vector3f& m, float alpha) const
{
    const float tanTheta = std::abs(Frame::tanTheta(v));

    /* perpendicular incidence -- no shadowing/masking */
    if (tanTheta == 0.0f)
        return 1.0f;

    /* Can't see the back side from the front and vice versa */
    if (glm::dot(v, m) * Frame::cosTheta(v) <= 0)
        return 0.0f;

    switch (m_type)
    {
        case EPhong:
        case EBeckmann:
            {
                float a;
                /* Approximation recommended by Bruce Walter: Use
                   the Beckmann shadowing-masking function with
                   specially chosen roughness value */
                if (m_type != EBeckmann)
                    a = std::sqrt(0.5f * alpha + 1) / tanTheta;
                else
                    a = 1.0f / (alpha * tanTheta);

                if (a >= 1.6f)
                    return 1.0f;

                /* Use a fast and accurate (<0.35% rel. error) rational
                   approximation to the shadowing-masking function */
                const float aSqr = a * a;
                return (3.535f * a + 2.181f * aSqr)
                     / (1.0f + 2.276f * a + 2.577f * aSqr);
            }
            break;

        case EGGX:
            {
                const float root = alpha * tanTheta;
                return 2.0f / (1.0f + std::sqrt(1.0f + root*root));
            }
            break;

        default:
            std::cerr << "Invalid distribution function!" << std::endl;
            return 0.0f;
    }
}


float MicrofacetDistribution::G(const Vector3f& wi, const Vector3f& wo,
               const Vector3f& wh, float alphaU, float alphaV)
{
    if (m_type != EAnisotropic)
        return smithG1(wi, wh, alphaU) * smithG1(wo, wh, alphaV);

    if (glm::dot(wi, wh) * Frame::cosTheta(wi) <= 0.f ||
        glm::dot(wo, wh) * Frame::cosTheta(wo) <= 0.f)
        return 0.f;
    float NdotWh = Frame::cosTheta(wh);
    float NdotWo = Frame::cosTheta(wo);
    float NdotWi = Frame::cosTheta(wi);
    float WOdotWh = glm::dot(wo, wh);
    //float WIdotWh = glm::dot(wi, wh);
    return std::min(1.f,
        std::min(std::abs(2.f * NdotWh * NdotWo / WOdotWh),
                 std::abs(2.f * NdotWh * NdotWi / WOdotWh)));
}

WISP_NAMESPACE_END
