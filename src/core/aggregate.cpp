#include "aggregate.h"

WISP_NAMESPACE_BEGIN

Aggregate::Aggregate()
{
}

const BSDF* Aggregate::getBSDF() const
{
    throw WispException("Aggregate::getBSDF should never be called!");
}

WISP_NAMESPACE_END
