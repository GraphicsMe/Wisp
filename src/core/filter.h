#ifndef RFILTER_H
#define RFILTER_H

#include "object.h"

WISP_NAMESPACE_BEGIN
class Filter : public Object
{
public:
    inline const Vector2f &getFilterSize() const { return m_size; }
    virtual float evaluate(float x, float y) const = 0;

    EClassType getClassType() const { return EFilter; }

protected:
    Vector2f m_size;
};

WISP_NAMESPACE_END

#endif // RFILTER_H
