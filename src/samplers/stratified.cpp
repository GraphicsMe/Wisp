#include "sampler.h"
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

        m_resolution = i;
        m_sampleCount = m_resolution * m_resolution;

        if (m_sampleCount != desiredSampleCount)
            std::cout << "Sample count should be a perfect square -- rounding to "
                      << m_sampleCount << std::endl;

        m_depth = paramSet.getInteger("depth", 3);
        m_permutations1D = new size_t*[m_depth];
        m_permutations2D = new size_t*[m_depth];

        for (int i = 0; i < m_depth; ++i)
        {
            m_permutations1D[i] = new size_t[m_sampleCount];
            m_permutations2D[i] = new size_t[m_sampleCount];
        }

        m_invResolution = 1.0f / (float)m_resolution;
        m_invResolutionSquare = 1.0f / (float) m_sampleCount;
        m_random = new Random();
        m_sampleIndex = 0;
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
            cloned->m_permutations2D[i] = new size_t[m_sampleCount];
        }
        return cloned;
    }

    void generate()
    {
        for (int i = 0; i < m_depth; ++i)
        {
            for (size_t j = 0; j < m_sampleCount; ++j)
            {
                m_permutations1D[i][j] = j;
                m_permutations2D[i][j] = j;
            }
            m_random->shuffle(m_permutations1D[i], m_sampleCount);
            m_random->shuffle(m_permutations2D[i], m_sampleCount);
        }

        m_sampleIndex = 0;
        m_sampleDepth1D = m_sampleDepth2D = 0;
    }

    void advance()
    {
        m_sampleIndex++;
        m_sampleDepth1D = m_sampleDepth2D = 0;
    }

    float next1D()
    {
        assert (m_sampleIndex < m_sampleCount);
        if (m_sampleDepth1D < m_depth)
        {
            int k = m_permutations1D[m_sampleDepth1D++][m_sampleIndex];
            return (k + m_random->nextFloat()) * m_invResolutionSquare;
        }
        return m_random->nextFloat();
    }

    Point2f next2D()
    {
        assert (m_sampleIndex < m_sampleCount);
        if (m_sampleDepth2D < m_depth)
        {
            int k = m_permutations2D[m_sampleDepth2D++][m_sampleIndex];
            int x = k % m_resolution;
            int y = k / m_resolution;
            return Point2f(
                        (x + m_random->nextFloat()) * m_invResolution,
                        (y + m_random->nextFloat()) * m_invResolution);
        }
        return Point2f(m_random->nextFloat(), m_random->nextFloat());
    }

    std::string toString() const
    {
        std::ostringstream oss;
        oss << "StratifiedSampler[" << std::endl
            << "  resolution = " << m_resolution << "," << std::endl
            << "  sampleCount = " << m_sampleCount << "," << std::endl
            << "  sampleDepth = " << m_depth << "," << std::endl
            << "  sampleIndex = " << m_sampleIndex << "," << std::endl
            << "]";
        return oss.str();
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
