#include "integrator.h"
#include "shape.h"
#include "scene.h"
#include "sampler.h"
#include "geometry.h"

WISP_NAMESPACE_BEGIN
class MIPathIntegrator : public Integrator
{
public:
    MIPathIntegrator(const ParamSet& paramSet)
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
            //ray.maxt = maxt;
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

            Vector3f wo = -ray.d;
            BSDFQueryRecord bRec(its.toLocal(wo));
            Color3f f = bsdf->sample_f(bRec, sampler->next2D());
            Vector3f wi = its.toWorld(bRec.wi);
            pathThrough *= f;

            Normal3f lightN;
            Point3f lightPos;
            pAreaLight->samplePosition(sampler->next2D(), lightPos, lightN);
            Vector3f dir = lightPos - p;
            float dis = glm::length(dir);
            dir /= dis;
            TRay shadowRay(p, dir, its.t*Epsilon, dis*(1.f-Epsilon));
            if (!scene->rayIntersect(shadowRay))
            {
                float lightPdf = pAreaLight->pdf(p, lightPos, lightN);
                L += pathThrough * pAreaLight->radiance() / lightPdf;
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
            ray.mint = its.t * Epsilon + Epsilon;
            ray.maxt = maxt;
        }
        return L;
    }

    std::string toString() const
    {
        return formatString("MIPathIntegrator[]");
    }
private:
    int m_maxDepth;
};

WISP_REGISTER_CLASS(MIPathIntegrator, "mipath")
WISP_NAMESPACE_END
