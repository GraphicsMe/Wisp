#ifndef COMMON_H
#define COMMON_H

# pragma warning (disable:4819)

#define WISP_NAMESPACE_BEGIN namespace Wisp {
#define WISP_NAMESPACE_END }

#include <iostream>
#include <algorithm>
#include <map>
#include <vector>
#include <string>
#include <stdint.h>
#include <thread>
#include <mutex>
#include <memory>
//#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif
#define INV_PI (1.0f/M_PI)
#define INV_TWOPI (0.5f/M_PI)
#define Epsilon 1e-3f
const float Infinity = std::numeric_limits<float>::infinity();

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/type_precision.hpp>
#include <glm/gtc/matrix_transform.hpp>

WISP_NAMESPACE_BEGIN
typedef glm::mediump_vec2 Vector2f;
typedef glm::mediump_vec3 Vector3f;
typedef glm::mediump_vec4 Vector4f;
typedef glm::highp_vec2 Vector2d;
typedef glm::highp_vec3 Vector3d;
typedef glm::highp_vec4 Vector4d;
typedef glm::i32vec1 Vector1i;
typedef glm::i32vec2 Vector2i;
typedef glm::i32vec3 Vector3i;
typedef glm::i32vec4 Vector4i;
typedef glm::mediump_vec2 Point2f;
typedef glm::mediump_vec3 Point3f;
typedef glm::mediump_vec4 Point4f;
typedef glm::highp_vec2 Point2d;
typedef glm::highp_vec3 Point3d;
typedef glm::highp_vec4 Point4d;
typedef glm::i32vec1 Point1i;
typedef glm::i32vec2 Point2i;
typedef glm::i32vec3 Point3i;
typedef glm::i32vec4 Point4i;
typedef Vector3f Color3f;
typedef Vector4f Color4f;
typedef Vector3f Normal3f;

typedef glm::mediump_mat3x3 Matrix3f;
typedef glm::highp_mat3x3 Matrix3d;
typedef glm::mediump_mat4x4 Matrix4f;
typedef glm::highp_mat4x4 Matrix4d;

class Object;
class ObjectFactory;
class Mesh;
class BSDF;
class Bitmap;
class BlockGenerator;
class ImageBlock;
class Camera;
class Integrator;
class Sampler;
class KDTree;
class Scene;
class Filter;
class PhaseFunction;
class Medium;
class Shape;
class Sphere;
class TRay;
class Transform;


class WispException
{
public:
    WispException(const std::string& reason) : m_reason(reason) {}
    inline const std::string& getReason() const { return m_reason; }
private:
    std::string m_reason;
};

/// Measures associated with probability distributions
enum EMeasure {
    EUnknownMeasure = 0,
    ESolidAngle,
    EDiscrete
};

//// Convert radians to degrees
inline float radToDeg(float value) { return value * (180.0f / M_PI); }

/// Convert degrees to radians
inline float degToRad(float value) { return value * (M_PI / 180.0f); }

/// Simple floating point clamping function
inline float clamp(float value, float min, float max) {
    if (value < min)
        return min;
    else if (value > max)
        return max;
    else return value;
}

/// Simple integer clamping function
inline int clamp(int value, int min, int max) {
    if (value < min)
        return min;
    else if (value > max)
        return max;
    else return value;
}

/// Linearly interpolate between two values
inline float lerp(float t, float v1, float v2) {
    return ((float) 1 - t) * v1 + t * v2;
}

template <class T>
inline void hash_combine(std::size_t& seed, const T& v)
{
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
}

inline bool isPowerOf2(int v)
{
    return v && (v & (v - 1));
}

inline size_t roundUpPow2(size_t v)
{
    --v;
    v |= v >> 1;    v |= v >> 2;
    v |= v >> 4;    v |= v >> 8;
    v |= v >> 16;
    return v+1;
}

inline float log2(float x)
{
    static float invLog2 = 1.f / logf(2.f);
    return logf(x) * invLog2;
}

inline int floor2Int(float val)
{
    return (int) floorf(val);
}

inline int log2Int(float v)
{
    return floor2Int(log2(v));
}

inline int ceil2Int(float val)
{
    return (int) ceilf(val);
}

inline int round2Int(float val)
{
    return floor2Int(val + 0.5f);
}

inline int float2Int(float val)
{
    return (int) val;
}

extern std::string formatString(const char *fmt, ...);

extern Vector3f uniformSphere(float u1, float u2);
extern Vector3f cosineHemisphere(float u1, float u2);
extern Point2f concentricDisk(float u1, float u2);

extern bool solveQuadratic(float A, float B, float C, float *t0, float *t1);
extern int getCoreCount();

WISP_NAMESPACE_END
#endif // COMMON_H
