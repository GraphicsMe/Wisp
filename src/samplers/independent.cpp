#include "sampler.h"
#include "random.h"

WISP_NAMESPACE_BEGIN

class IndependentSampler : public Sampler
{
public:
    IndependentSampler(const ParamSet& paramSet)
    {
        m_sampleCount = (size_t) paramSet.getInteger("sampleCount", 1);
        m_random = new Random();
    }

    virtual ~IndependentSampler()
    {
        delete m_random;
    }

    Sampler* clone()
    {
        IndependentSampler* cloned = new IndependentSampler();
        cloned->m_sampleCount = m_sampleCount;
        cloned->m_random = new Random();
        cloned->m_random->seed(m_random);
        return cloned;
    }

    void generate() {}
    void advance() {}

    float next1D()
    {
        return m_random->nextFloat();
    }

    Point2f next2D()
    {
        return Point2f(m_random->nextFloat(), m_random->nextFloat());
    }

    std::string toString() const
    {
        return std::string(formatString("Independent[sampleCount=%d]", m_sampleCount));
    }

protected:
    IndependentSampler() {}

protected:
    Random* m_random;

};

WISP_REGISTER_CLASS(IndependentSampler, "independent")

WISP_NAMESPACE_END
