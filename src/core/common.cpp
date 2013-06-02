#include "common.h"
#include <stdarg.h>
#include <Windows.h>

WISP_NAMESPACE_BEGIN
std::string formatString(const char *fmt, ...)
{
    char tmp[512];
    va_list iterator;

#if defined(WIN32)
    va_start(iterator, fmt);
    size_t size = _vscprintf(fmt, iterator) + 1;

    if (size >= sizeof(tmp)) {
        char *dest = new char[size];
        vsnprintf_s(dest, size, size-1, fmt, iterator);
        va_end(iterator);
        std::string result(dest);
        delete[] dest;
        return result;
    }

    vsnprintf_s(tmp, size, size-1, fmt, iterator);
    va_end(iterator);
#else
    va_start(iterator, fmt);
    size_t size = vsnprintf(tmp, sizeof(tmp), fmt, iterator);
    va_end(iterator);

    if (size >= sizeof(tmp)) {
        /* Overflow! -- dynamically allocate memory */
        char *dest = new char[size+1];
        va_start(iterator, fmt);
        vsnprintf(dest, size+1, fmt, iterator);
        va_end(iterator);

        std::string result(dest);
        delete[] dest;
        return result;
    }
#endif

    return std::string(tmp);
}

Vector3f uniformSphere(float u1, float u2)
{
    float z = 1.f - 2.f * u1;
    float r = sqrtf(max(0.f, 1.f - z*z));
    float phi = 2.f * M_PI * u2;
    float x = r * cosf(phi);
    float y = r * sinf(phi);
    return Vector3f(x, y, z);
}

Vector3f cosineHemisphere(float u1, float u2)
{
    Point2f disk = concentricDisk(u1, u2);
    float z = std::sqrt(max(0.0f, 1.0f - disk.x * disk.x - disk.y * disk.y));
    return Vector3f(disk.x, disk.y, z);
}

Point2f concentricDisk(float u1, float u2)
{
    float r, theta;
    // Map uniform random numbers to $[-1,1]^2$
    float sx = 2 * u1 - 1;
    float sy = 2 * u2 - 1;

    // Map square to $(r,\theta)$

    // Handle degeneracy at the origin
    if (sx == 0.0 && sy == 0.0) {
        return Point2f(0.0f, 0.0f);
    }
    if (sx >= -sy) {
        if (sx > sy) {
            // Handle first region of disk
            r = sx;
            if (sy > 0.0) theta = sy/r;
            else          theta = 8.0f + sy/r;
        }
        else {
            // Handle second region of disk
            r = sy;
            theta = 2.0f - sx/r;
        }
    }
    else {
        if (sx <= sy) {
            // Handle third region of disk
            r = -sx;
            theta = 4.0f - sy/r;
        }
        else {
            // Handle fourth region of disk
            r = -sy;
            theta = 6.0f + sx/r;
        }
    }
    theta *= M_PI / 4.f;
    return r * Point2f(sinf(theta), cosf(theta));
}

Point2f uniformTriangle(float u1, float u2)
{
    float a = std::sqrt(1.0f - u1);
    return Point2f(1.0f - a, a * u2);
}

bool solveQuadratic(float A, float B, float C, float *t0, float *t1)
{
    // Find quadratic discriminant
    float discrim = B * B - 4.f * A * C;
    if (discrim <= 0.) return false;
    float rootDiscrim = sqrtf(discrim);

    // Compute quadratic _t_ values
    float q;
    if (B < 0) q = -.5f * (B - rootDiscrim);
    else       q = -.5f * (B + rootDiscrim);
    *t0 = q / A;
    *t1 = C / q;
    if (*t0 > *t1) std::swap(*t0, *t1);
    return true;
}

int getCoreCount()
{
    SYSTEM_INFO sys_info;
    GetSystemInfo(&sys_info);
    return sys_info.dwNumberOfProcessors;
}

WISP_NAMESPACE_END
