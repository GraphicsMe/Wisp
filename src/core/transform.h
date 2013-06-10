#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "common.h"
#include "geometry.h"

WISP_NAMESPACE_BEGIN

class Transform
{
public:
    Transform()
        : m_transform(Matrix4f())
        , m_inverse(Matrix4f())
    {
    }

    Transform(const Matrix4f& trans)
        : m_transform(trans)
        , m_inverse(glm::inverse(trans))
    {
    }

    Transform(const Matrix4f& trans, const Matrix4f& inv)
        : m_transform(trans), m_inverse(inv) {}

    inline const Matrix4f& getMatrix() const { return m_transform; }
    inline const Matrix4f& getInverseMatrix() const { return m_inverse; }

    Transform inverse() const
    {
        return Transform(m_inverse, m_transform);
    }

    Transform operator*(const Transform& t) const
    {
        return Transform(m_transform*t.m_transform, t.m_inverse*m_inverse);
    }

    inline Vector3f operator*(const Vector3f& v) const
    {
        Matrix3f mat3(m_transform);
        return mat3*v;
    }

    inline Point3f operator*(const Point4f& p) const
    {
        Vector4f result = m_transform * p;
        Point3f ret(result.x, result.y, result.z);
        return ret/result.w;
    }

    inline Ray operator*(const Ray& r) const
    {
        Ray ret(r);
        ret.d = (*this)*r.d;
        ret.o = (*this)*Point4f(r.o.x, r.o.y, r.o.z, 1.0f);
        return ret;
    }

    inline void setIdentity()
    {
        m_transform = Matrix4f();
        m_inverse = Matrix4f();
    }

    inline static Transform scale(Vector3f v)
    {
        Matrix4f m(v.x, 0.f, 0.f, 0.f,
                   0.f, v.y, 0.f, 0.f,
                   0.f, 0.f, v.z, 0.f,
                   0.f, 0.f, 0.f, 1.f);
        Matrix4f minv(1.f/v.x, 0.f, 0.f, 0.f,
                   0.f, 1.f/v.y, 0.f, 0.f,
                   0.f, 0.f, 1.f/v.z, 0.f,
                   0.f, 0.f, 0.f, 1.f);
        return Transform(m, minv);
    }


    static Transform scale(float x, float y, float z)
    {
        Matrix4f m(x, 0.f, 0.f, 0.f,
                   0.f, y, 0.f, 0.f,
                   0.f, 0.f, z, 0.f,
                   0.f, 0.f, 0.f, 1.f);
        Matrix4f minv(1.f/x, 0.f, 0.f, 0.f,
                   0.f, 1.f/y, 0.f, 0.f,
                   0.f, 0.f, 1.f/z, 0.f,
                   0.f, 0.f, 0.f, 1.f);
        return Transform(m, minv);
    }

    static Transform translate(const Vector3f& v)
    {
        Matrix4f m(1.f, 0.f, 0.f, 0.f,
                   0.f, 1.f, 0.f, 0.f,
                   0.f, 0.f, 1.f, 0.f,
                   v.x, v.y, v.z, 1.f);
        Matrix4f minv(1.f, 0.f, 0.f, 0.f,
                   0.f, 1.f, 0.f, 0.f,
                   0.f, 0.f, 1.f, 0.f,
                   -v.x, -v.y, -v.z, 1.f);
        return Transform(m, minv);
    }

    static Transform translate(float x, float y, float z)
    {
        Matrix4f m(1.f, 0.f, 0.f, 0.f,
                   0.f, 1.f, 0.f, 0.f,
                   0.f, 0.f, 1.f, 0.f,
                   x, y, z, 1.f);
        Matrix4f minv(1.f, 0.f, 0.f, 0.f,
                   0.f, 1.f, 0.f, 0.f,
                   0.f, 0.f, 1.f, 0.f,
                   -x, -y, -z, 1.f);
        return Transform(m, minv);
    }

    static Transform rotate(float angleDegree, Vector3f axis)
    {
        Matrix4f mat(1.f, 0.f, 0.f, 0.f,
                   0.f, 1.f, 0.f, 0.f,
                   0.f, 0.f, 1.f, 0.f,
                   0.f, 0.f, 0.f, 1.f);
        mat = glm::rotate(mat, angleDegree, axis);
        return Transform(mat);
    }

    std::string toString()
    {
        return formatString(
                    "%.3f %.3f %.3f %.3f\n"
                    "%.3f %.3f %.3f %.3f\n"
                    "%.3f %.3f %.3f %.3f\n"
                    "%.3f %.3f %.3f %.3f\n",
                    m_transform[0][0], m_transform[0][1], m_transform[0][2], m_transform[0][3],
                    m_transform[1][0], m_transform[1][1], m_transform[1][2], m_transform[1][3],
                    m_transform[2][0], m_transform[2][1], m_transform[2][2], m_transform[2][3],
                    m_transform[3][0], m_transform[3][1], m_transform[3][2], m_transform[3][3]);
    }

private:
    Matrix4f m_transform;
    Matrix4f m_inverse;
};

WISP_NAMESPACE_END
#endif // TRANSFORM_H
