#include "filter.h"

WISP_NAMESPACE_BEGIN
class TentFilter : public Filter
{
public:
    TentFilter(const ParamSet& paramSet)
    {
        float radius = paramSet.getFloat("radius", 1.0f);
        m_size.x = m_size.y = radius;
    }

    float evaluate(float x, float y) const
    {
        return std::max(0.0f, std::abs(m_size.x - x)) *
                std::max(0.0f, std::abs(m_size.y - y));
    }

    std::string toString() const
    {
        return "TentFilter[]";
    }
};

WISP_REGISTER_CLASS(TentFilter, "tent")

WISP_NAMESPACE_END

