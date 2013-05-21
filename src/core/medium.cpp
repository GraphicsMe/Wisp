#include "medium.h"
#include "phase.h"

WISP_NAMESPACE_BEGIN
Medium::Medium() : m_phaseFunction(NULL), m_bound(Point3f(0.f), Point3f(1.f))
{
    //m_bound = BBox()
}

Medium::~Medium()
{
    if (m_phaseFunction)
        delete m_phaseFunction;
}

void Medium::prepare()
{
    if (!m_phaseFunction)
        m_phaseFunction = static_cast<PhaseFunction*>(
            ObjectFactory::createInstance("isotropic", ParamSet()));
}

void Medium::addChild(Object *pChild)
{
    switch (pChild->getClassType()) {
        case EPhaseFunction:
            if (m_phaseFunction)
                throw WispException("Medium: tried to register multiple phase function instances!");
            m_phaseFunction = static_cast<PhaseFunction *>(pChild);
            break;

        default:
            throw WispException(formatString("Medium::addChild(<%s>) is not supported!",
                classTypeName(pChild->getClassType())).c_str());
    }
}

bool Medium::inside(const Point3f& p) const
{
    Point3f pp = m_worldToMedium * Point4f(p.x, p.y, p.z, 1.0f);
    return m_bound.inside(pp);
}

WISP_NAMESPACE_END
