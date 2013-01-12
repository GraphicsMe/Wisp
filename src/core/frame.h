#ifndef FRAME_H
#define FRAME_H

#include "common.h"

WISP_NAMESPACE_BEGIN
struct Frame
{
    Vector3f t, b;
    Normal3f n;

    inline Frame() {}
    inline Frame(const Vector3f& t, const Vector3f& b, const Normal3f& n)
        : t(t), b(b), n(n) {}

    inline Frame(const Vector3f& n) : n(n)
    {
        coordinateSystem(n, t, b);
    }

    inline Vector3f toLocal(const Vector3f& v) const
    {
        return Vector3f(glm::dot(v, t), glm::dot(v, b), glm::dot(v, n));
    }

    inline Vector3f toWorld(const Vector3f& v) const
    {
        return t * v.x + b * v.y + n * v.z;
    }

    inline static float cosTheta(const Vector3f& v)
    {
        return v.z;
    }

    inline static float sinTheta(const Vector3f& v)
    {
        float temp = sinTheta2(v);
        if (temp <= 0.0f)
            return 0.0f;
        return std::sqrt(temp);
    }

    inline static float tanTheta(const Vector3f& v)
    {
        return sinTheta(v) / v.z;
    }

    inline static float sinTheta2(const Vector3f& v)
    {
        return 1.0f - v.z * v.z;
    }

    inline static float sinPhi(const Vector3f& v)
    {
        float sinTheta = Frame::sinTheta(v);
        if (sinTheta == 0.0f)
            return 1.0f;
        return clamp(v.y / sinTheta, -1.0f, 1.0f);
    }

    inline static float cosPhi(const Vector3f& v)
    {
        float sinTheta = Frame::sinTheta(v);
        if (sinTheta == 0.0f)
            return 1.0f;
        return clamp(v.x / sinTheta, -1.0f, 1.0f);
    }

    inline static float sinPhi2(const Vector3f& v)
    {
        return clamp(v.y*v.y / sinTheta2(v), 0.0f, 1.0f);
    }

    inline static float cosPhi2(const Vector3f& v)
    {
        return clamp(v.x*v.x / sinTheta2(v), 0.0f, 1.0f);
    }

    inline bool operator == (const Frame& other) const
    {
        return other.t == t && other.b == b && other.n == n;
    }

    inline bool operator != (const Frame& other) const
    {
        return !(operator==(other));
    }

    inline std::string toString() const
    {
        return std::string(formatString("Frame["
                                        "t=[%.3f, %.3f, %.3f]"
                                        "b=[%.3f, %.3f, %.3f]"
                                        "n=[%.3f, %.3f, %.3f]"
                                        "]",
                                        t.x, t.y, t.z,
                                        b.x, b.y, b.z,
                                        n.x, n.y, n.z));
    }

private:
    void coordinateSystem(const Vector3f &a, Vector3f &b, Vector3f &c)
    {
        if (std::abs(a.x) > std::abs(a.y)) {
            float invLen = 1.0f / std::sqrt(a.x * a.x + a.z * a.z);
            c = Vector3f(a.z * invLen, 0.0f, -a.x * invLen);
        } else {
            float invLen = 1.0f / std::sqrt(a.y * a.y + a.z * a.z);
            c = Vector3f(0.0f, a.z * invLen, -a.y * invLen);
        }
        b = glm::cross(c, a);
    }
};

WISP_NAMESPACE_END
#endif // FRAME_H
