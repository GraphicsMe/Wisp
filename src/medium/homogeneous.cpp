#include "medium.h"

WISP_NAMESPACE_BEGIN
class HomogeneousMedium : public Medium
{
public:
    HomogeneousMedium(const ParamSet& paramSet)
    {
        m_sigmaS = paramSet.getColor("sigmaS");
        m_sigmaT = paramSet.getColor("sigmaA") + m_sigmaS;
        m_worldToMedium = paramSet.getTransform("toWorld", Transform()).inverse();
    }

    bool sampleDistance(const Ray &ray, Sampler *sampler, float &t, Color3f &weight) const
    {
        return false;
    }

    Color3f evalTransmittance(const Ray &ray, Sampler *sampler) const
    {
        return Color3f(0.f);
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
    Transform m_worldToMedium;
};

WISP_REGISTER_CLASS(HomogeneousMedium, "homogeneous")
WISP_NAMESPACE_END
