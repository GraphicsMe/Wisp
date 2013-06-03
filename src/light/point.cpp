#include "light.h"
#include "shape.h"

WISP_NAMESPACE_BEGIN

class PointLight : public Light
{
public:
    PointLight(const ParamSet& paramSet)
    {
        m_intensity = paramSet.getColor("intensity", Color3f(1.f));
        m_lightToWorld = paramSet.getTransform("toWorld", Transform());
        if (paramSet.hasProperty("position"))
            m_lightToWorld = Transform::translate(paramSet.getPoint("position"));
        m_worldToLight = m_lightToWorld.inverse();
        m_position = m_lightToWorld * Point4f(0.f, 0.f, 0.f, 1.f);
    }

    void sample_f(const Point3f& p, LightSamplingRecord& lRec, const Point2f&) const
    {
        lRec.pdf = 1.f;
        lRec.sRec.p = m_position;
        Vector3f dir = p - m_position;
        float invDist = 1.f / glm::length(dir);
        lRec.d = dir * invDist;
        lRec.value = m_intensity * invDist * invDist;
    }

    float pdf(const Point3f&, const LightSamplingRecord& ) const
    {
        return 0.f;
    }

    std::string toString() const
    {
        return formatString("PointLight[intensity=%.3f %.3f %.3f]",
                            m_intensity.x, m_intensity.y, m_intensity.z);
    }

private:
    Point3f m_position;
    Color3f m_intensity;
};

WISP_REGISTER_CLASS(PointLight, "point")
WISP_NAMESPACE_END
