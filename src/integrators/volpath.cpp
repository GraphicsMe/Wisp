#include "integrator.h"
#include "shape.h"
#include "scene.h"
#include "sampler.h"
#include "geometry.h"
#include "bsdf.h"

WISP_NAMESPACE_BEGIN
class VolumePathIntegrator : public Integrator
{
public:
    VolumePathIntegrator(const ParamSet& paramSet)
    {
        m_maxDepth = paramSet.getInteger("maxDepth", 10);
    }

    Color3f Li(const Scene *scene, Sampler *sampler, const Ray &r) const
    {
        Color3f pathThrough(1.f), L(0.f);
        Ray ray(r);
        for (int depth = 0; depth < m_maxDepth; ++depth)
        {
            Intersection its;
            if (!scene->rayIntersect(ray, its))
                break;

            const BSDF* bsdf = its.getBSDF();
            assert (bsdf != NULL);

            if (depth == 0 && its.shape->isLight())
                L += pathThrough * its.Le(-ray.d);

            // sample light
            LightSamplingRecord lRec;
            if (scene->sampleLight(its.p, lRec, sampler->next2D(), its.t*Epsilon))
            {
                Vector3f wi = -lRec.d;
                BSDFQueryRecord bRec(its, its.toLocal(wi));
                Color3f bsdfVal = bsdf->eval(bRec);
                L += pathThrough * lRec.value * bsdfVal;
            }

            // sample bsdf
            BSDFQueryRecord bRec(its);
            Color3f bsdfVal = bsdf->sample_f(bRec, sampler->next2D());
            if (isZero(bsdfVal))
                break;

            Vector3f wi = its.toWorld(bRec.wi);
            ray = Ray(its.p, wi, its.t*Epsilon);

            pathThrough *= bsdfVal;

            if (depth > 3)
            {
                float continueProbability = std::min(0.5f, vectorMax(pathThrough));
                if (sampler->next1D() > continueProbability)
                    break;
                pathThrough /= continueProbability;
            }
        }
        return L;
    }

    std::string toString() const
    {
        return formatString("VolumePathIntegrator[]");
    }
private:
    int m_maxDepth;
};

WISP_REGISTER_CLASS(VolumePathIntegrator, "volpath")
WISP_NAMESPACE_END
