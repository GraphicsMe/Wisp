#ifndef CAMERA_H
#define CAMERA_H

#include "object.h"

WISP_NAMESPACE_BEGIN
class Camera : public Object
{
public:
    virtual float generateRay(const Point2f& sample,
                              const Point2f& lensSample,
                              Ray& ray) const = 0;
    inline const Vector2i& getOutputSize() const { return m_outputSize; }
    inline const Filter* getFilter() const { return m_filter; }

    EClassType getClassType() const { return ECamera; }

protected:
    Vector2i m_outputSize;
    Filter* m_filter;
};
WISP_NAMESPACE_END
#endif // CAMERA_H
