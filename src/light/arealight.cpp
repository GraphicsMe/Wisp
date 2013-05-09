#include "light.h"
#include "shape.h"

WISP_NAMESPACE_BEGIN

class AreaLight
{
public:
    AreaLight(const ParamSet& paramSet)
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

    void samplePosition(const Point2f& sample, Point3f& p, Normal3f& n) const
    {
        assert (m_shape != NULL);
        m_shape->samplePosition(sample, p, n);
    }

    float pdf(Point3f p, Point3f lp, Normal3f& n) const
    {
        float pdfArea = m_shape->pdf();
        Vector3f dir = p - lp;
        float lenSqr = glm::dot(dir, dir);
        return pdfArea * lenSqr * std::sqrt(lenSqr) / std::max(0.f, glm::dot(n, dir));
    }

    Color3f radiance() const
    {
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
    Color3f m_radiance;
    Shape* m_shape;
};

WISP_REGISTER_CLASS(AreaLight, "area")
WISP_NAMESPACE_END
