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

    virtual void prepare();
    virtual void addChild(Object *pChild);

    EClassType getClassType() const { return EMedium; }

protected:
    Medium();

    BBox m_bound;
    PhaseFunction* m_phaseFunction;
};
WISP_NAMESPACE_END

#endif // MEDIUM_H
