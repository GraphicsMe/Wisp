#include "aggregate.h"

WISP_NAMESPACE_BEGIN

class GridAccel : public Aggregate
{
public:
    GridAccel(const ParamSet&)
    {

    }

    virtual void prepare()
    {

    }

    virtual void addChild(Object *pChild)
    {
        Shape* shape = static_cast<Shape*>(pChild);
        assert (shape != NULL);
        m_primitives.push_back(ShapePtr(shape));
    }

    virtual bool rayIntersect(const TRay& ray, Intersection& its)
    {
        ray;
        its;
        assert (0);
        return false;
    }

    virtual BBox getBoundingBox() const
    {
        return m_bound;
    }

    std::string toString() const
    {
        return std::string("GridAccel[]");
    }

private:
    std::vector<ShapePtr> m_primitives;
};

WISP_REGISTER_CLASS(GridAccel, "grid")

WISP_NAMESPACE_END
