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

float AreaLight::pdf() const
{
    return m_shape->pdf();
}

Color3f AreaLight::radiance() const
{
    return m_radiance;
}

WISP_REGISTER_CLASS(AreaLight, "area")
WISP_NAMESPACE_END
