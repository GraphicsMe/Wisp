#ifndef MEDIUM_H
#define MEDIUM_H

#include "object.h"

WISP_NAMESPACE_BEGIN
class Medium : public Object
{
public:
    virtual ~Medium();


    inline const PhaseFunction* getPhaseFunction() const { return m_phaseFunction; }

    virtual void addChild(Object *pChild);
    virtual void prepare();
    EClassType getClassType() const { return EMedium; }

protected:
    Medium();

    PhaseFunction* m_phaseFunction;
};



WISP_NAMESPACE_END

#endif // MEDIUM_H
