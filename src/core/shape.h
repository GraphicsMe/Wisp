#ifndef SHAPE_H
#define SHAPE_H

#include "object.h"
#include "frame.h"
#include "geometry.h"
WISP_NAMESPACE_BEGIN

class Intersection
{
public:
    Point3f p;
    Vector3f wo; // local frame
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

    const BSDF* getBSDF() const;
    Color3f Le(const Vector3f& dir) const;
};

struct ShapeSamplingRecord
{
    Point3f p;
    Normal3f n;

    inline ShapeSamplingRecord() {}
    inline ShapeSamplingRecord(const Point3f& p, const Normal3f& n)
        : p(p), n(n) {}
    inline ShapeSamplingRecord(const Intersection& its)
        : p(its.p), n(its.geoFrame.n) {}
};

class Shape : public Object
{
public:
    typedef Shape* ShapePtr;
    
    Shape() : m_bsdf(NULL),m_light(NULL) {}
	virtual ~Shape() {}

    virtual float area() const = 0;
    virtual float pdf() const = 0;
    virtual bool canIntersect() const { return true; }
    virtual void refine(std::vector<ShapePtr>&) const;


    virtual float sampleArea(ShapeSamplingRecord& sRec, const Point2f& sample) const;
    virtual float sampleSolidAngle(ShapeSamplingRecord& sRec,const Point3f& x, const Point2f sample) const;    
    virtual float pdfArea(const ShapeSamplingRecord &sRec) const;
    virtual float pdfSolidAngle(const ShapeSamplingRecord &sRec, const Point3f &x) const;

    //virtual void samplePosition(const Point2f& sample, Point3f& p, Normal3f& n) const;
    virtual bool rayIntersect(const TRay& ray);
    virtual bool rayIntersect(const TRay& ray, Intersection& its);
    virtual void fillIntersectionRecord(const TRay& ray, Intersection& its) const = 0;

    virtual BBox getBoundingBox() const = 0;
    const BSDF* getBSDF() const { return m_bsdf; }
    inline bool isLight() const { return m_light != NULL; }
    Light* getLight() { return m_light; }
    const Light* getLight() const { return m_light; }
    const std::string& getName() const { return m_name; }

    EClassType getClassType() const { return EShape; }
    void fullyRefine(std::vector<ShapePtr> &refined) const;

protected:
    BSDF* m_bsdf;
    Light* m_light;
    std::string m_name;
};



WISP_NAMESPACE_END

#endif // SHAPE_H
