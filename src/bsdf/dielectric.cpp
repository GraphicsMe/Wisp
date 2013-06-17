#include "bsdf.h"
#include "frame.h"
#include "reflection.h"

WISP_NAMESPACE_BEGIN

class Dielectric : public BSDF
{
public:
    Dielectric(const ParamSet& paramSet)
    {
        m_intIOR = paramSet.getFloat("intIOR", 1.5046f); // glass
        m_extIOR = paramSet.getFloat("extIOR", 1.000277f); // air
        m_kr = paramSet.getColor("kr", Color3f(1.f));
        m_kt = paramSet.getColor("kt", Color3f(1.f));
        m_fresnel = new FresnelDielectric(m_intIOR, m_extIOR);
    }

    ~Dielectric()
    {
        delete m_fresnel;
    }

    // local coordinates
    inline Vector3f reflect(const Vector3f& wi) const
    {
        return Vector3f(-wi.x, -wi.y, wi.z);
    }

    // local coordinates
    inline Vector3f refract(const Vector3f& wi, float eta, float cost) const
    {
        return Vector3f(eta * -wi.x, eta * -wi.y, cost);
    }

    // local coordinates
    inline Vector3f refract(const Vector3f& wi) const
    {
        float ei = m_intIOR, et = m_extIOR;
        bool entering = Frame::cosTheta(wi) > 0.f;
        if (!entering)
            std::swap(ei, et);

        float sini2 = Frame::sinTheta2(wi);
        float eta = ei / et;
        float sint2 = eta * eta * sini2;

        if (sint2 >= 1.f)
            return Vector3f(0.f);
        else
        {
            float cost = std::sqrt(std::max(0.f, 1.f - sint2));
            return refract(wi, eta, entering ? -cost : cost);
        }
    }

    Color3f sample_f(BSDFQueryRecord& bRec, const Point2f& sample) const
    {
        float pdf;
        return this->sample_f(bRec, pdf, sample);
    }

    Color3f sample_f(BSDFQueryRecord& bRec, float& pdf, const Point2f& sample) const
    {
        float ei = m_intIOR, et = m_extIOR;
        bool entering = Frame::cosTheta(bRec.wo) > 0.f;
        if (!entering)
            std::swap(ei, et);

        float sini2 = Frame::sinTheta2(bRec.wo);
        float eta = ei / et;
        float sint2 = eta * eta * sini2;

        Color3f Fr;
        float cost;
        if (sint2 >= 1.f)
            Fr = Color3f(1.f);
        else
        {
            cost = std::sqrt(std::max(0.f, 1.f - sint2));
            Fr = m_fresnel->eval(Frame::cosTheta(bRec.wo));
            if (entering)
                cost = -cost;
        }

        // sample
        if (sample.x <= Fr.x) // reflection
        {
            bRec.wi = reflect(bRec.wo);
            pdf = Fr.x;
            bRec.sampledType = EDeltaReflection;
            return m_kr;
        }
        else                // transmission
        {
            bRec.wi = refract(bRec.wo, eta, cost);
            pdf = 1.f - Fr.x;
            bRec.sampledType = EDeltaTransmission;
            return m_kt * eta * eta;
        }
    }

    Color3f eval(const BSDFQueryRecord& bRec) const
    {
        bool reflection = std::abs(1.f - glm::dot(reflect(bRec.wo), bRec.wi)) < Epsilon;
        bool refraction = std::abs(1.f - glm::dot(refract(bRec.wo), bRec.wi)) < Epsilon;
        if (!reflection && !refraction)
            return Color3f(0.f);
        assert (!reflection || !refraction);

        Color3f Fr = m_fresnel->eval(Frame::cosTheta(bRec.wo));
        if (reflection)
            return m_kr * Fr;
        else
        {
            float ei = m_intIOR, et = m_extIOR;
            bool entering = Frame::cosTheta(bRec.wo) > 0.f;
            if (!entering)
                std::swap(ei, et);
            float eta = ei / et;
            return m_kt * eta * eta * (1.f - Fr);
        }
    }

    float pdf(const BSDFQueryRecord& bRec) const
    {
        bool reflection = std::abs(1.f - glm::dot(reflect(bRec.wo), bRec.wi)) < Epsilon;
        bool refraction = std::abs(1.f - glm::dot(refract(bRec.wo), bRec.wi)) < Epsilon;
        if (!reflection && !refraction)
            return 0.f;
        assert (!reflection || !refraction);

        Color3f Fr = m_fresnel->eval(Frame::cosTheta(bRec.wo));
        if (reflection)
            return Fr.x;
        else
            return 1.f - Fr.x;
    }

    std::string toString() const
    {
        return std::string(formatString("Dielectric[]"));
    }

private:
    Fresnel* m_fresnel;
    Color3f m_kr, m_kt;
    float m_intIOR, m_extIOR;
};

WISP_REGISTER_CLASS(Dielectric, "dielectric")
WISP_NAMESPACE_END
