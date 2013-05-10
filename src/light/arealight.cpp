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

    Color3f sample_f(const Point3f &p) const
    {
        return m_radiance;
    }

    std::string toString() const
    {
        return formatString("AreaLight[radiance=%.3f]", m_radiance);
    }

    void sample_f(const Point3f& p, LightSamplingRecord& lRec, const Point2f& sample) const
    {
        lRec.pdf = m_shape->sampleSolidAngle(lRec.sRec, p, sample);
        lRec.d = p - lRec.sRec.p;
        if (lRec.pdf > 0 && glm::dot(lRec.d, lRec.sRec.n))
        {
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

    /*void samplePosition(const Point2f& sample, Point3f& p, Normal3f& n) const
    {
        assert (m_shape != NULL);
        m_shape->samplePosition(sample, p, n);
    }*/

    float pdf(Point3f p, Point3f lp, Normal3f& n) const
    {
        float pdfArea = m_shape->pdf();
        Vector3f dir = p - lp;
        float lenSqr = glm::dot(dir, dir);
        return pdfArea * lenSqr * std::sqrt(lenSqr) / std::max(0.f, glm::dot(n, dir));
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
private:
    Shape* m_shape;
    Color3f m_radiance;
};

WISP_REGISTER_CLASS(AreaLight, "area")
WISP_NAMESPACE_END
