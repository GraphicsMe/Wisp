#include "light.h"

WISP_NAMESPACE_BEGIN
Light::Light(const ParamSet& paramSet)
{
    m_lightToWorld = paramSet.getTransform("toWorld", Transform());
    m_worldToLight = m_lightToWorld.inverse();
    m_intersectable = false;
}

Color3f Light::Le(const Vector3f&) const
{
    throw WispException("Unimplemented Light::Le() method called");
    return Color3f(0.f);
}

Color3f Light::Le(const ShapeSamplingRecord&, const Vector3f&) const
{
    throw WispException("Unimplemented Light::Le() method called");
    return Color3f(0.f);
}

float Light::pdf(const Point3f&, const LightSamplingRecord&) const
{
    throw WispException("Unimplemented Light::pdf() method called");
    return 0.f;
}

void Light::sample_f(const Point3f&, LightSamplingRecord&, const Point2f&) const
{
    throw WispException("Unimplemented Light::sample_f() method called");
}



WISP_NAMESPACE_END
