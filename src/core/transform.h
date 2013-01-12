#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "common.h"

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

    inline void setIdentity()
    {
        m_transform = Matrix4f();
        m_inverse = Matrix4f();
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
        Matrix4f mat(1.0f);
        glm::rotate(mat, angleDegree, axis);
        return Transform(mat);
    }

private:
    Matrix4f m_transform;
    Matrix4f m_inverse;
};

WISP_NAMESPACE_END
#endif // TRANSFORM_H
