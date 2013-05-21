#ifndef MEDIUM_H
#define MEDIUM_H

#include "object.h"
#include "geometry.h"

WISP_NAMESPACE_BEGIN
class Medium : public Object
{
public:
    virtual ~Medium();

    inline const PhaseFunction* getPhaseFunction() const { return m_phaseFunction; }
    virtual Color3f evalTransmittance(const Ray& ray, Sampler* sampler) const = 0;
    virtual bool sampleDistance(const Ray& ray, Sampler* sampler, float& t, Color3f& weight, float& albedo) const = 0;

    virtual void prepare();
    virtual void addChild(Object *pChild);

    BBox getBound() const { return m_bound; }
    bool inside(const Point3f& p) const;

    EClassType getClassType() const { return EMedium; }

protected:
    Medium();

    BBox m_bound;
    Transform m_worldToMedium;
    PhaseFunction* m_phaseFunction;
};
WISP_NAMESPACE_END

#endif // MEDIUM_H
