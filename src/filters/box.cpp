#include "filter.h"

WISP_NAMESPACE_BEGIN
class BoxFilter : public Filter
{
public:
    BoxFilter(const ParamSet& paramSet)
    {
        float radius = paramSet.getFloat("radius", 0.5f);
        m_size.x = m_size.y = radius;
    }

    float evaluate(float, float) const
    {
        return 1.0f;
    }

    std::string toString() const
    {
        return "BoxFilter[]";
    }
};

WISP_REGISTER_CLASS(BoxFilter, "box")

WISP_NAMESPACE_END
