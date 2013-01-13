#include "filter.h"

WISP_NAMESPACE_BEGIN
class MitchellFilter : public Filter
{
public:
    MitchellFilter(const ParamSet& paramSet)
    {
        float radius = paramSet.getFloat("radius", 2.0f);
        m_size = Vector2f(radius, radius);
        m_invSize = 1.0f / m_size;

        m_B = paramSet.getFloat("B", 1.0f / 3.0f);
        m_C = paramSet.getFloat("C", 1.0f / 3.0f);
    }

    float evaluate(float x, float y) const
    {
        return mitchell1D(x * m_invSize.x) * mitchell1D(y * m_invSize.y);
    }

    std::string toString() const
    {
        return "MitchellFilter[]";
    }

protected:
    float mitchell1D(float x) const
    {
        x = std::abs(2.0f * x);
        float x2 = x * x;
        float x3 = x * x2;

        if (x < 1.f)
            return ((12 - 9*m_B - 6*m_C) * x3 +
                    (-18 + 12*m_B + 6*m_C) * x2 +
                    (6 - 2*m_B)) * (1.f/6.f);
        else if (x < 2.f)
            return ((-m_B - 6*m_C) * x3 + (6*m_B + 30*m_C) * x2 +
                    (-12*m_B - 48*m_C) * x + (8*m_B + 24*m_C)) * (1.f/6.f);
        else
            return 0.0f;
    }

    float m_B, m_C;
    Vector2f m_invSize;
};

WISP_REGISTER_CLASS(MitchellFilter, "mitchell")

WISP_NAMESPACE_END


