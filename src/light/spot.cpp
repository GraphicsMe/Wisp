#include "light.h"
#include "shape.h"

WISP_NAMESPACE_BEGIN

class SpotLight : public Light
{
public:
    SpotLight(const ParamSet& paramSet) : Light(paramSet)
    {
        m_intensity = paramSet.getColor("intensity", Color3f(1.f));
        if (paramSet.hasProperty("position"))
        {
            m_lightToWorld = Transform::translate(paramSet.getPoint("position"));
            m_worldToLight = m_lightToWorld.inverse();
        }
        m_position = m_lightToWorld * Point4f(0.f, 0.f, 0.f, 1.f);
        m_cutoffAngle = paramSet.getFloat("cutoffAngle", 20);
        m_beamWidth = paramSet.getFloat("beamWidth", m_cutoffAngle * 3.f/4.f) ;

        m_cutoffAngle = degToRad(m_cutoffAngle);
        m_beamWidth = degToRad(m_beamWidth);

        assert (m_cutoffAngle >= m_beamWidth);
        m_cosBeamWidth = std::cos(m_beamWidth);
        m_cosCutoffAngle = std::cos(m_cutoffAngle);
        m_invTransitionWidth = 1.f / (m_cutoffAngle - m_beamWidth);
    }

    float falloff(const Vector3f& d) const
    {
        Vector3f localDir = m_worldToLight * d;
        float cosTheta = localDir.z;

        if (cosTheta <= m_cosCutoffAngle)
            return 0.f;

        if (cosTheta >= m_cosBeamWidth)
            return 1.f;

        //return (m_cutoffAngle - std::acos(cosTheta)) * m_invTransitionWidth;
        return (cosTheta - m_cosCutoffAngle) / (m_cosBeamWidth - m_cosCutoffAngle);
    }

    void sample_f(const Point3f& p, LightSamplingRecord& lRec, const Point2f&) const
    {
        lRec.pdf = 1.f;
        lRec.sRec.p = m_position;
        Vector3f dir = p - m_position;
        float invDist = 1.f / glm::length(dir);
        lRec.d = dir * invDist;
        lRec.value = m_intensity* falloff(lRec.d) * invDist * invDist;
    }

    float pdf(const Point3f&, const LightSamplingRecord& ) const
    {
        return 0.f;
    }

    std::string toString() const
    {
        return formatString("SpotLight[intensity=%.3f %.3f %.3f]",
                            m_intensity.x, m_intensity.y, m_intensity.z);
    }

private:
    Point3f m_position;
    Color3f m_intensity;
    float m_beamWidth, m_cutoffAngle;
    float m_cosBeamWidth, m_cosCutoffAngle, m_invTransitionWidth;
};

WISP_REGISTER_CLASS(SpotLight, "spot")
WISP_NAMESPACE_END

