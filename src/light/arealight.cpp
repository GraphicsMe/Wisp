#include "light.h"
#include "shape.h"

WISP_NAMESPACE_BEGIN

/*AreaLight::~AreaLight()
{
}*/

AreaLight::AreaLight(const ParamSet& paramSet)
{
    m_radiance = paramSet.getColor("radiance", Color3f(1.f));
}

Color3f AreaLight::sample_f(const Point3f &p) const
{
    return m_radiance;
}

std::string AreaLight::toString() const
{
    return formatString("AreaLight[radiance=%.3f]", m_radiance);
}

void AreaLight::samplePosition(const Point2f& sample, Point3f& p, Normal3f& n) const
{
    assert (m_shape != NULL);
    m_shape->samplePosition(sample, p, n);
}

float AreaLight::pdf(Point3f p, Point3f lp, Normal3f& n) const
{
    float pdfArea = m_shape->pdf();
    Vector3f dir = p - lp;
    float lenSqr = glm::dot(dir, dir);
    return pdfArea * lenSqr * std::sqrt(lenSqr) / std::max(0.f, glm::dot(n, dir));
}

Color3f AreaLight::radiance() const
{
    return m_radiance;
}

WISP_REGISTER_CLASS(AreaLight, "area")
WISP_NAMESPACE_END
