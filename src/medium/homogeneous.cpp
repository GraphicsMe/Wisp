#include "medium.h"
#include "sampler.h"

WISP_NAMESPACE_BEGIN
class HomogeneousMedium : public Medium
{
public:
    HomogeneousMedium(const ParamSet& paramSet)
    {
        m_sigmaS = paramSet.getColor("sigmaS");
        m_sigmaT = paramSet.getColor("sigmaA") + m_sigmaS;
        m_toWorld = paramSet.getTransform("toWorld", Transform());
        m_worldToMedium = paramSet.getTransform("toWorld", Transform()).inverse();
    }

    bool sampleDistance(const Ray& r, Sampler* sampler, float& t, Color3f& weight, float& albedo) const
    {
        Ray ray = m_worldToMedium * r;
        float t0, t1;
        if (!m_bound.rayIntersect(ray, t0, t1))
            return false;
        float samplingWeight = m_sigmaS.x / m_sigmaT.x;
        float rnd = sampler->next1D(), sampledDistance;
        if (rnd <= samplingWeight)
        {
            rnd /= samplingWeight; // reuse rand
            sampledDistance = -std::log(1.f-rnd) / m_sigmaT.x;
        }
        else
            sampledDistance = std::numeric_limits<float>::infinity();

        float range = t1 - t0;
        if (sampledDistance < range)
        {
            t = sampledDistance + t0;
            Point3f p = r(t);
            p = m_worldToMedium * Point4f(p.x, p.y, p.z, 1.0f);
            if (!m_bound.inside(p))
            {
                puts("out side media!");
                goto fail;
            }
            // pdf = transmittance * sigmaT * samplingWeight = sigmaS * transmittance
            // weight = sigmaS * transmittance / pdf = 1.0
            weight = Color3f(1.0f);
            albedo = samplingWeight;
            return true;
        }
        else
        {
            fail:
            sampledDistance = range;
            // pdf = samplingWeight * transmittance + (1 - samplingWeight)
            // weight = transmittance / pdf
            float transmittance = std::exp(-sampledDistance * m_sigmaT.x);
            float pdf = samplingWeight * transmittance + 1.f - samplingWeight;
            assert (pdf >= 0.f && pdf <= 1.f);
            weight = Color3f(transmittance / pdf);
            return false;
        }
    }

    Color3f evalTransmittance(const Ray& r, Sampler*) const
    {
        float t0, t1;
        Ray ray = m_worldToMedium * r;
        if (!m_bound.rayIntersect(ray, t0, t1))
            return Color3f(1.0f);

        float negLength = t0 - t1;
        Color3f transmittance;
        for (int i = 0; i < 3; ++i)
            transmittance[i] = std::exp(m_sigmaT[i] * negLength);

        return transmittance;
    }

    std::string toString() const {
        return std::string(formatString(
            "HomogeneousMedium[\n"
            "  sigmaS = [%f, %f, %f]\n"
            "  sigmaT = [%f, %f, %f],\n"
            "]", m_sigmaS.x, m_sigmaS.y, m_sigmaS.z
               , m_sigmaT.x, m_sigmaT.y, m_sigmaT.z));
    }
private:
    Color3f m_sigmaS;
    Color3f m_sigmaT;
    Transform m_toWorld;
    //Transform m_worldToMedium;
};

WISP_REGISTER_CLASS(HomogeneousMedium, "homogeneous")
WISP_NAMESPACE_END
