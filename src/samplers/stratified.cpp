/*#include "sampler.h"
#include "random.h"

WISP_NAMESPACE_BEGIN

class StratifiedSampler : public Sampler
{
public:
    StratifiedSampler(const ParamSet& paramSet)
    {
        size_t desiredSampleCount = (size_t) paramSet.getInteger("sampleCount", 1);

        size_t i = 1;
        while (i * i < desiredSampleCount)
            ++i;

        if (m_sampleCount != desiredSampleCount)
            std::cout << "Sample count should be a perfect square -- rounding to "
                      << m_sampleCount << std::endl;

        m_resolution = i;
        m_sampleCount = m_resolution * m_resolution;

        m_depth = paramSet.getInteger("depth", 3);
        m_permutations1D = new size_t*[m_depth];
        m_permutations2D = new size_t*[m_depth];

        for (int i = 0; i < m_depth; ++i)
        {
            m_permutations1D = new size_t[m_sampleCount];
            m_permutations2D = new size_t[m_sampleCount];
        }

        m_invResolution = 1.0f / (float)m_resolution;
        m_invResolutionSquare = 1.0f / (float) m_sampleCount;
        m_random = new Random();
    }

    virtual ~StratifiedSampler()
    {
        delete m_random;
        for (int i = 0; i < m_depth; ++i)
        {
            delete[] m_permutations1D[i];
            delete[] m_permutations2D[i];
        }
        delete[] m_permutations1D;
        delete[] m_permutations2D;
    }

    Sampler* clone()
    {
        StratifiedSampler* cloned = new StratifiedSampler();
        cloned->m_sampleCount = m_sampleCount;
        cloned->m_depth = m_depth;
        cloned->m_resolution = m_resolution;
        cloned->m_invResolution = m_invResolution;
        cloned->m_invResolutionSquare = m_invResolutionSquare;
        cloned->m_random = new Random(m_random);

        cloned->m_permutations1D = new size_t*[m_depth];
        cloned->m_permutations2D = new size_t*[m_depth];
        for (int i = 0; i < m_depth; ++i)
        {
            cloned->m_permutations1D[i] = new size_t[m_sampleCount];
            cloned->m_permutations1D[i] = new size_t[m_sampleCount];
        }
        return cloned;
    }

    void generate()
    {
        for (int i = 0; i < m_depth; ++i)
        {
            for (size_t j = 0; j < m_sampleCount; ++j)
                m_permutations1D[i][j] = j;
            m_random->shuffle()
        }
    }

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
        return std::string(formatString("StratifiedSampler[sampleCount=%d]", m_sampleCount));
    }

protected:
    StratifiedSampler() {}

protected:
    Random* m_random;
    int m_depth;
    int m_resolution;
    float m_invResolution, m_invResolutionSquare;
    int m_sampleDepth1D, m_sampleDepth2D;
    size_t **m_permutations1D, **m_permutations2D;
};

WISP_REGISTER_CLASS(StratifiedSampler, "stratified")

WISP_NAMESPACE_END
*/
