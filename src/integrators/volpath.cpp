#include "integrator.h"
#include "shape.h"
#include "scene.h"
#include "sampler.h"
#include "geometry.h"
#include "bsdf.h"
#include "medium.h"
#include "phase.h"

WISP_NAMESPACE_BEGIN
class VolumePathIntegrator : public Integrator
{
public:
    VolumePathIntegrator(const ParamSet& paramSet)
    {
        m_maxDepth = paramSet.getInteger("maxDepth", 10);
        m_multipleScattering = paramSet.getBoolean("multipleScattering", true);
        m_multipleScattering = true;
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

            float t;
            float albedo;
            Color3f weight;
            if (scene->sampleDistance(ray, sampler, t, weight, albedo))
            {
                pathThrough *= weight;

                Point3f scatterP = ray(t);
                assert (scene->getMedium()->inside(scatterP));

                const PhaseFunction* phase = scene->getMedium()->getPhaseFunction();
                assert (phase != NULL);

                // sample light
                LightSamplingRecord lRec;
                if (scene->sampleAttenuatedLight(scatterP, lRec, sampler->next2D(), its.t*Epsilon, sampler))
                    L += pathThrough * lRec.value * phase->eval(PhaseFunctionQueryRecord(-ray.d, -lRec.d));

                if (!m_multipleScattering)
                    break;

                // sample phase function
                PhaseFunctionQueryRecord pRec(-ray.d);
                float phaseVal = phase->sample(pRec, sampler->next2D());
                if (phaseVal == 0.f)
                    break;
                pathThrough *= phaseVal;

                ray = Ray(scatterP, pRec.wo, 0.f);

                if (depth > 5)
                {
                    if (sampler->next1D() > albedo)
                        break;
                    else
                        pathThrough /= albedo;
                }
            }
            else
            {
                pathThrough *= weight;

                const BSDF* bsdf = its.getBSDF();
                assert (bsdf != NULL);

                if (depth == 0 && its.shape->isLight())
                    L += pathThrough * its.Le(-ray.d);

                // sample light
                LightSamplingRecord lRec;
                if (scene->sampleAttenuatedLight(its.p, lRec, sampler->next2D(), its.t*Epsilon, sampler))
                {
                    Vector3f wi = -lRec.d;
                    BSDFQueryRecord bRec(its, its.toLocal(wi));
                    Color3f bsdfVal = bsdf->eval(bRec);
                    L += pathThrough * lRec.value * bsdfVal;
                }

                if (!m_multipleScattering)
                    break;

                // sample bsdf
                BSDFQueryRecord bRec(its);
                Color3f bsdfVal = bsdf->sample_f(bRec, sampler->next2D());
                if (isZero(bsdfVal))
                    break;

                Vector3f wi = its.toWorld(bRec.wi);
                ray = Ray(its.p, wi, its.t*Epsilon);

                pathThrough *= bsdfVal;

                if (depth > 5)
                {
                    float continueProbability = std::min(0.5f, vectorMax(pathThrough));
                    if (sampler->next1D() > continueProbability)
                        break;
                    pathThrough /= continueProbability;
                }
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
    bool m_multipleScattering;
};

WISP_REGISTER_CLASS(VolumePathIntegrator, "volpath")
WISP_NAMESPACE_END
