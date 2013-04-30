#include "integrator.h"
#include "shape.h"
#include "scene.h"
#include "sampler.h"
#include "geometry.h"

WISP_NAMESPACE_BEGIN
class AmbientOcclusion : public Integrator
{
public:
    AmbientOcclusion(const ParamSet& paramSet)
    {
        m_length = paramSet.getFloat("length", 1.0f);
    }

    Color3f Li(const Scene *scene, Sampler *sampler, const TRay &ray) const
    {
        Intersection its;
        if (!scene->rayIntersect(ray, its))
            return Color3f(0.0f);

        Point2f sample = sampler->next2D();
        Vector3f d = cosineHemisphere(sample.x, sample.y);

        d = its.toWorld(d);

        float length = m_length * glm::length(scene->getBoundingBox().getExtents());

        TRay shadowRay(its.p, d, 1e-4f*its.t, length);

        return Color3f(scene->rayIntersect(shadowRay) ? 0.0f : 1.0f);
    }

    std::string toString() const
    {
        return formatString("AmbientOcclusion[length=%.3f]", m_length);
    }

private:
    float m_length;
};

WISP_REGISTER_CLASS(AmbientOcclusion, "ao")
WISP_NAMESPACE_END
