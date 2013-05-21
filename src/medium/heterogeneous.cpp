#include "medium.h"
WISP_NAMESPACE_BEGIN

class HeterogeneousMedium : public Medium
{
public:
    HeterogeneousMedium(const ParamSet& paramSet)
    {
        m_albedo = paramSet.getColor("albedo");
        m_filename = paramSet.getString("filename");
        m_densityMultiplier = paramSet.getFloat("densityMultiplier", 1.0f);
        m_worldToMedium = paramSet.getTransform("toWorld", Transform()).inverse();
    }

    virtual ~HeterogeneousMedium()
    {
    }

    float lookupSigmaT(const Point3f&) const
    {
        return 0.f;
    }

    bool sampleDistance(const Ray&, Sampler*, float&, Color3f&, float&) const
    {
        return false;
    }

    Color3f evalTransmittance(const Ray&, Sampler*) const
    {
        return Color3f(0.f);
    }

    std::string toString() const
    {
        return std::string(formatString(
            "HeterogeneousMedium[\n"
            "  filename = \"%s\",\n"
            "  densityMultiplier = %f,\n"
            "  albedo = [%f %f %f]\n"
            "]",
            m_filename.c_str(), m_densityMultiplier,
            m_albedo.x, m_albedo.y, m_albedo.z));
    }

private:
    std::string m_filename;

    Color3f m_albedo;
    Vector3i m_resolution;
    float m_densityMultiplier;
    Transform m_worldToMedium;
};

WISP_REGISTER_CLASS(HeterogeneousMedium, "heterogeneous")
WISP_NAMESPACE_END
