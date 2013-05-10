#include "shape.h"

WISP_NAMESPACE_BEGIN

class Sphere : public Shape
{
public:
    Sphere(const ParamSet& paramSet)
    {
        m_radius = paramSet.getFloat("radius", 1.f);
        m_center = paramSet.getPoint("center", Point3f(0.f, 0.f, 0.f));
        m_diffuse = paramSet.getColor("albedo", Color3f(0.f, 0.f, 0.f));
    }

    ~Sphere()
    {

    }

    virtual float area() const
    {
        return M_PI * m_radius * m_radius;
    }

    virtual float pdf() const
    {
        return 1.0f / area();
    }

    /*virtual void samplePosition(const Point2f& sample, Point3f& p, Normal3f& n) const
    {
        Vector3f dir = uniformSphere(sample.x, sample.y);
        p = m_center + m_radius * dir;
        n = dir;
    }*/

    virtual bool rayIntersect(const TRay& ray)
    {
        Vector3f dir=ray.o-m_center;
        float a = glm::dot(ray.d, ray.d);
        float b = 2.0f * glm::dot(ray.d, dir);
        float c = glm::dot(dir, dir) - m_radius * m_radius;
        float t0, t1;
        if (!solveQuadratic(a, b, c, &t0, &t1))
            return false;

        if (t0 > ray.maxt || t1 < ray.mint)
            return false;

        float thit = t0;
        if (t0 < ray.mint)
        {
            thit = t1;
            if (thit > ray.maxt) return false;
        }
        return true;
    }

    virtual bool rayIntersect(const TRay& ray, Intersection& its)
    {
        Vector3f dir=ray.o-m_center;
        float a = glm::dot(ray.d, ray.d);
        float b = 2.0f * glm::dot(ray.d, dir);
        float c = glm::dot(dir, dir) - m_radius * m_radius;
        float t0, t1;
        if (!solveQuadratic(a, b, c, &t0, &t1))
            return false;

        if (t0 > ray.maxt || t1 < ray.mint)
            return false;

        float thit = t0;
        if (t0 < ray.mint)
        {
            thit = t1;
            if (thit > ray.maxt) return false;
        }

        its.t = thit;
        //intersection record
        its.p = ray(its.t);
        its.uv = Point2f(1.0f, 1.0f);
        its.color = m_diffuse;
        its.shape = this;
        its.geoFrame = Frame(glm::normalize(its.p-m_center));
        its.shFrame = its.geoFrame;
        its.wo = its.toLocal(-ray.d);

        return true;
    }

    virtual void fillIntersectionRecord(const TRay& ray, Intersection& its) const
    {
        its.p = ray(its.t);
        its.uv = Point2f(1.0f, 1.0f);
        its.color = m_diffuse;
        its.shape = this;
        its.geoFrame = Frame(glm::normalize(its.p-m_center));
        its.shFrame = its.geoFrame;
    }

    virtual BBox getBoundingBox() const
    {
        Vector3f off(m_radius, m_radius, m_radius);
        return BBox(m_center - off, m_center + off);
    }

    std::string toString() const
    {
        return std::string(formatString("Sphere[center=(%.3f, %.3f, %.3f), radius=%.3f]",
                                        m_center.x, m_center.y, m_center.z, m_radius));
    }

private:
    float m_radius;
    Point3f m_center;
    Color3f m_diffuse;
};

WISP_REGISTER_CLASS(Sphere, "sphere")

WISP_NAMESPACE_END
