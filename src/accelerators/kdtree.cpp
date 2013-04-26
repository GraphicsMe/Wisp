#include "aggregate.h"

WISP_NAMESPACE_BEGIN

class KdTreeAccel : public Aggregate
{
public:
    KdTreeAccel(const ParamSet& paramSet)
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

    }

    virtual void samplePosition(const Point2f& sample, Point3f& p, Normal3f& n) const
    {
        Vector3f dir = uniformSphere(sample.x, sample.y);
    }

    virtual bool rayIntersect(const TRay &ray)
    {
        return false;
    }

    virtual bool rayIntersect(const TRay& ray, Intersection& its)
    {
        return false;
    }

    virtual void fillIntersectionRecord(const TRay& ray, Intersection& its) const
    {
        //its.p = ray(its.t);
        //its.uv = Point2f(1.0f, 1.0f);
        //its.color = m_diffuse;
        //its.shape = this;
        //its.geoFrame = Frame(glm::normalize(its.p-m_center));
        //its.shFrame = its.geoFrame;
    }

    virtual BBox getBoundingBox() const
    {
        return m_bound;
    }

    std::string toString() const
    {
        return std::string("KdTreeAccel[]");
    }

protected:
    typedef std::shared_ptr<Shape> ShapePtr;
    std::vector<ShapePtr> m_primitives;
};

WISP_REGISTER_CLASS(KdTreeAccel, "kdtree")

WISP_NAMESPACE_END
