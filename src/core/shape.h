#ifndef SHAPE_H
#define SHAPE_H

#include "object.h"
#include "frame.h"
#include "geometry.h"
#include "bsdf.h"
#include "light.h"

WISP_NAMESPACE_BEGIN

struct Intersection
{
    Point3f p;
    float t;
    Color3f color;
    Point2f uv;
    Frame shFrame;
    Frame geoFrame;
    const Shape* shape;

    inline Intersection() : shape(NULL) {}
    inline Vector3f toLocal(const Vector3f& d) const
    {
        return shFrame.toLocal(d);
    }

    inline Vector3f toWorld(const Vector3f& d) const
    {
        return shFrame.toWorld(d);
    }
};

class AreaLight;
class Shape : public Object
{
public:
    typedef Shape* ShapePtr;
    
    Shape() : m_bsdf(NULL),m_areaLight(NULL) {}
	virtual ~Shape() {}

    virtual float area() const = 0;
    virtual float pdf() const = 0;
    virtual bool canIntersect() const { return true; }
    virtual void refine(std::vector<ShapePtr>&) const { throw WispException("Unimplemented Shape::refine() method called"); }

    virtual void samplePosition(const Point2f& sample, Point3f& p, Normal3f& n) const = 0;
    virtual bool rayIntersect(const TRay& ray) { throw WispException("Unimplemented Shape::rayIntersect() method called"); }
    virtual bool rayIntersect(const TRay& ray, Intersection& its) { throw WispException("Unimplemented Shape::rayIntersect() method called"); }
    virtual void fillIntersectionRecord(const TRay& ray, Intersection& its) const = 0;

    virtual BBox getBoundingBox() const = 0;
    const BSDF* getBSDF() const { return m_bsdf; }
    const AreaLight* getAreaLight() const { return m_areaLight; }
    const std::string& getName() const { return m_name; }

    EClassType getClassType() const { return EShape; }

    void fullyRefine(std::vector<ShapePtr> &refined) const
	{
        std::vector<ShapePtr> todo;
        todo.push_back(ShapePtr(this));
        while (todo.size())
        {
            ShapePtr prim = todo.back();
			todo.pop_back();
			if (prim->canIntersect())
				refined.push_back(prim);
			else
				prim->refine(todo);
		}
	}

protected:
    BSDF* m_bsdf;
    AreaLight* m_areaLight;
    std::string m_name;
};



WISP_NAMESPACE_END

#endif // SHAPE_H
