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

Color3f fresnelDiel(float cosi, float cost, const Color3f& etaI, const Color3f& etaT)
{
    Color3f rParl = ((etaT * cosi) - (etaI * cost)) /
                    ((etaT * cosi) + (etaI * cost));
    Color3f rPerp = ((etaI * cosi) - (etaT * cost)) /
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
        return fresnelDiel(std::abs(cosi), cost, Color3f(ei), Color3f(et));
    }
}


WISP_NAMESPACE_END
