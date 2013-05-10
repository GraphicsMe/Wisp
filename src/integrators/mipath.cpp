#include "integrator.h"
#include "shape.h"
#include "scene.h"
#include "sampler.h"
#include "geometry.h"
#include "bsdf.h"

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
        Color3f pathThrough(1.f), L(0.f);
        TRay ray(r);
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
                float bsdfPdf = bsdf->pdf(bRec);
                float weight = powerHeuristic(1, lRec.pdf, 1, bsdfPdf);
                L += pathThrough * lRec.value * bsdfVal * weight;
            }

            // sample bsdf
            BSDFQueryRecord bRec(its);
            float bsdfPdf;
            Color3f bsdfVal = bsdf->sample_f(bRec, bsdfPdf, sampler->next2D());
            if (isZero(bsdfVal))
                break;

            Vector3f wi = its.toWorld(bRec.wi);
            ray = TRay(its.p, wi, its.t*Epsilon);

            if (scene->rayIntersect(ray, its) && its.shape->isLight())
            {
                lRec = LightSamplingRecord(its, -ray.d);
                lRec.value = its.Le(-ray.d);

                float lightPdf = scene->pdfLight(ray.o, lRec);
                float weight = powerHeuristic(1, bsdfPdf, 1, lightPdf);
                L += pathThrough * lRec.value * bsdfVal * weight;
            }

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
        return formatString("MIPathIntegrator[]");
    }
private:
    int m_maxDepth;
};

WISP_REGISTER_CLASS(MIPathIntegrator, "mipath")
WISP_NAMESPACE_END
