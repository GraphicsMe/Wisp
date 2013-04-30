#include "integrator.h"
#include "shape.h"
#include "scene.h"
#include "sampler.h"
#include "geometry.h"

WISP_NAMESPACE_BEGIN
class PathIntegrator : public Integrator
{
public:
    PathIntegrator(const ParamSet& paramSet)
    {
        m_maxDepth = paramSet.getInteger("maxDepth", 10);
    }

    Color3f Li(const Scene *scene, Sampler *sampler, const TRay &r) const
    {
        AreaLight* pAreaLight = scene->getAreaLight();
        assert (pAreaLight != NULL);
        Color3f pathThrough(1.f), L(0.f);
        TRay ray(r);
        float maxt = ray.maxt;
        for (int depth = 0; depth < m_maxDepth; ++depth)
        {
            Intersection its;
            ray.maxt = maxt;
            if (!scene->rayIntersect(ray, its))
                break;
            Point3f p = ray.o + ray.d * its.t;
            const AreaLight* light = its.shape->getAreaLight();
            if (light)
            {
                L += pathThrough * light->sample_f(p);
                break;
            }

            const BSDF* bsdf = its.shape->getBSDF();
            assert (bsdf != NULL);

            Vector3f wo = -ray.d, wi;
            wo = its.toLocal(wo);
            Color3f f = bsdf->sample_f(wo, wi, sampler->next2D());
            wi = its.toWorld(wi);
            pathThrough *= f;

            Point3f lightPos;
            Normal3f lightN;
            pAreaLight->samplePosition(sampler->next2D(), lightPos, lightN);
            Vector3f dir = lightPos - p;
            float dis = glm::length(dir);
            dir /= dis;
            TRay shadowRay(p, dir, Epsilon, dis);
            if (!scene->rayIntersect(shadowRay))
            {
                float pdf = pAreaLight->pdf()*dis*dis/std::max(0.f, glm::dot(lightN, -wi));
                L += pathThrough * pAreaLight->radiance() / pdf;
            }

            if (depth > 3)
            {
                float continueProbability = std::min(0.5f, pathThrough.y);
                if (sampler->next1D() > continueProbability)
                    break;
                pathThrough /= continueProbability;
            }

            ray.o = p;
            ray.d = wi;
        }
        return L;
    }

    std::string toString() const
    {
        return formatString("PathIntegrator[]");
    }
private:
    int m_maxDepth;
};

WISP_REGISTER_CLASS(PathIntegrator, "mipath")
WISP_NAMESPACE_END
