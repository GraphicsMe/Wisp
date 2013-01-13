#include "filter.h"

WISP_NAMESPACE_BEGIN
class GaussianFilter : public Filter
{
public:
    GaussianFilter(const ParamSet& paramSet)
    {
        float radius = paramSet.getFloat("radius", 2.0f);
        m_size.x = m_size.y = radius;

        float stddev = paramSet.getFloat("stddev", 0.5f);
        m_alpha = 1.0f / (2.0f * stddev * stddev);

        m_expX = exp(-m_alpha * m_size.x * m_size.x);
        m_expY = exp(-m_alpha * m_size.y * m_size.y);
    }

    float evaluate(float x, float y) const
    {
        return gaussian(x, m_expX) * gaussian(y, m_expY);
    }

    std::string toString() const
    {
        return "GaussianFilter[]";
    }
protected:
    // f(x) = exp(-alpha*x*x) - exp(-alpha*w*w)
    float gaussian(float d, float expv) const
    {
        return std::max(0.f, std::exp(-m_alpha * d * d) - expv);
    }

    float m_alpha;
    float m_expX, m_expY;
};

WISP_REGISTER_CLASS(GaussianFilter, "gaussian")

WISP_NAMESPACE_END

