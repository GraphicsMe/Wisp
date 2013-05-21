#include "light.h"
#include "shape.h"

WISP_NAMESPACE_BEGIN

class AreaLight : public Light
{
public:
    AreaLight(const ParamSet& paramSet) : m_shape(NULL)
    {
        m_radiance = paramSet.getColor("radiance", Color3f(1.f));
    }

    void sample_f(const Point3f& p, LightSamplingRecord& lRec, const Point2f& sample) const
    {
        lRec.pdf = m_shape->sampleSolidAngle(lRec.sRec, p, sample);
        if (lRec.pdf > 0)
        {
            lRec.d = p - lRec.sRec.p;
            assert (glm::dot(lRec.d, lRec.sRec.n) > 0.f);
            lRec.value = m_radiance;
            lRec.d = glm::normalize(lRec.d);
        }
        else
            lRec.pdf = 0;
    }

    float pdf(const Point3f& p, const LightSamplingRecord& lRec) const
    {
        return m_shape->pdfSolidAngle(lRec.sRec, p);
    }

    virtual Color3f Le(const ShapeSamplingRecord& sRec, const Vector3f& dir) const
    {
        if (glm::dot(dir, sRec.n) <= 0.f)
            return Color3f(0.f);
        return m_radiance;
    }

    void setParent(Object *pParent)
    {
        if (pParent->getClassType() == Object::EShape ||
            pParent->getClassType() == Object::EMesh)
        {
            if (m_shape)
                throw WispException("An area light source cannot be parent of multiple shapes");
            m_shape = static_cast<Shape*>(pParent);
        }
        else
            throw WispException("An area light source must be child of Shape instance");
    }

    std::string toString() const
    {
        return formatString("AreaLight[radiance=%.3f]", m_radiance);
    }

private:
    Shape* m_shape;
    Color3f m_radiance;
};

WISP_REGISTER_CLASS(AreaLight, "area")
WISP_NAMESPACE_END
