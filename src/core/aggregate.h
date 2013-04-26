#ifndef AGGREGATE_H
#define AGGREGATE_H

#include "shape.h"
WISP_NAMESPACE_BEGIN

class Aggregate : public Shape
{
public:
    Aggregate();

    const BSDF* getBSDF() const;
    EClassType getClassType() const { return EAccelerator; }

protected:
    BBox m_bound;
};

WISP_NAMESPACE_END

#endif // AGGREGATE_H
