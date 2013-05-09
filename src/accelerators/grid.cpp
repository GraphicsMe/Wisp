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

    virtual float area() const
    {
        return 1.0f;
    }

    virtual float pdf() const
    {
        return 0.0f;
    }

    virtual void addChild(Object *pChild)
    {
        Shape* shape = static_cast<Shape*>(pChild);
        assert (shape != NULL);
        m_primitives.push_back(ShapePtr(shape));
    }

    virtual bool rayIntersect(const TRay& ray, Intersection& its)
    {
        return false;
    }

    virtual void fillIntersectionRecord(const TRay& ray, Intersection& its) const
    {
    }

    virtual BBox getBoundingBox() const
    {
        return m_bound;
    }

    std::string toString() const
    {
        return std::string("GridAccel[]");
    }

protected:
    typedef std::shared_ptr<Shape> ShapePtr;
    std::vector<ShapePtr> m_primitives;
};

WISP_REGISTER_CLASS(GridAccel, "grid")

WISP_NAMESPACE_END
