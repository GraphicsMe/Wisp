#ifndef SAMPLER_H
#define SAMPLER_H

#include "common.h"
#include "object.h"

WISP_NAMESPACE_BEGIN

class Sampler : public Object
{
public:
    virtual ~Sampler() {}
    virtual Sampler* clone() = 0;
    virtual void generate() = 0;
    virtual void advance() = 0;
    virtual float next1D() = 0;
    virtual Point2f next2D() = 0;
    virtual inline size_t getSampleCount() const { return m_sampleCount; }
    EClassType getClassType() const { return ESampler; }

protected:
    size_t m_sampleCount;
    size_t m_sampleIndex;
};

WISP_NAMESPACE_END
#endif // SAMPLER_H
