#ifndef LIGHT_H
#define LIGHT_H

#include "object.h"

WISP_NAMESPACE_BEGIN
class Light : public Object
{
public:
    virtual ~Light() {}

    virtual Color3f sample_f(const Point3f& p) const = 0;

    EClassType getClassType() const { return ELuminaire; }
};

class AreaLight : public Light
{
public:
    AreaLight(const ParamSet &paramSet);
    //~AreaLight();

    Color3f sample_f(const Point3f& p) const;
    void setShape(Shape* shape) { m_shape = shape; }
    void samplePosition(const Point2f& sample, Point3f& p, Normal3f& n) const;
    float pdf() const;
    Color3f radiance() const;

    std::string toString() const;

private:
    Color3f m_radiance;
    Shape* m_shape;
};

WISP_NAMESPACE_END

#endif // LIGHT_H
