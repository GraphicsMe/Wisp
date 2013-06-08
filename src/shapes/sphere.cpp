#include "shape.h"

WISP_NAMESPACE_BEGIN

class Sphere : public Shape
{
public:
    Sphere(const ParamSet& paramSet)
    {
        m_objectToWorld = Transform::translate(paramSet.getPoint("center", Point3f(0.f)));
        m_radius = paramSet.getFloat("radius", 1.f);
        if (paramSet.hasProperty("toWorld"))
        {
            Transform toWorld = paramSet.getTransform("toWorld");
            float scale = glm::length(toWorld * Vector3f(1.f, 0.f, 0.f));
            m_objectToWorld = toWorld * Transform::scale(Vector3f(1.f/scale)) * m_objectToWorld;
            m_radius *= scale;
        }

        m_flipNormals = paramSet.getBoolean("flipNormals", false);
        m_center = m_objectToWorld * Point4f(0.f, 0.f, 0.f, 1.f);
        m_invSurfaceArea = 1.f / (4.f * M_PI * m_radius * m_radius);
    }

    ~Sphere()
    {

    }

    virtual float area() const
    {
        return 4.f * M_PI * m_radius * m_radius;
    }

    virtual float pdfArea() const
    {
        return m_invSurfaceArea;
    }

    virtual float sampleArea(ShapeSamplingRecord &sRec, const Point2f &sample) const
    {
        Vector3f v = uniformSphere(sample.x, sample.y);
        sRec.n = Normal3f(v);
        sRec.p = m_center + m_radius * v;
        return m_invSurfaceArea;
    }

    /*
    virtual float pdfSolidAngle(const ShapeSamplingRecord &sRec, const Point3f &x) const
    {
        return 0.f;
    }

    virtual float sampleSolidAngle(ShapeSamplingRecord& sRec,const Point3f& x, const Point2f sample) const
    {
        return 0.f;
    }
    */

    virtual bool rayIntersect(const Ray& ray)
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

    virtual bool rayIntersect(const Ray& ray, Intersection& its)
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

        //intersection record
        its.t = thit;
        its.p = ray(its.t);
        its.uv = Point2f(1.0f, 1.0f);
        its.shape = this;
        its.geoFrame = Frame(glm::normalize(its.p-m_center));
        if (m_flipNormals)
            its.geoFrame.n *= -1;
        its.shFrame = its.geoFrame;
        its.wo = its.toLocal(-ray.d);

        return true;
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
    bool m_flipNormals;
    float m_invSurfaceArea;
    Transform m_objectToWorld;
};

WISP_REGISTER_CLASS(Sphere, "sphere")

WISP_NAMESPACE_END
