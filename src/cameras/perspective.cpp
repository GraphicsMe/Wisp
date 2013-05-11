#include "camera.h"
#include "filter.h"
#include "transform.h"
#include "geometry.h"

WISP_NAMESPACE_BEGIN
class PerspectiveCamera : public Camera
{
public:
    PerspectiveCamera(const ParamSet& paramSet)
    {
        m_outputSize.x = paramSet.getInteger("width", 512);
        m_outputSize.y = paramSet.getInteger("height", 512);
        m_invOutputSize = Vector2f(1.0f / m_outputSize.x, 1.0f / m_outputSize.y);
        m_cameraToWorld = paramSet.getTransform("toWorld", Transform());
        m_fov = paramSet.getFloat("fov", 30.0f);
        m_lensRadius = paramSet.getFloat("apertureRadius", 0.0f);
        m_nearClip = paramSet.getFloat("nearClip", 1e-4f);
        m_farClip = paramSet.getFloat("farClip", 1e4f);
        m_focusDistance = paramSet.getFloat("focusDistance", m_farClip);
        m_filter = NULL;
    }

    void prepare()
    {
        float aspect = (float)m_outputSize.x / (float)m_outputSize.y;
        float recip = 1.0f / (m_farClip - m_nearClip);
        float cot = 1.0f / tanf(degToRad(0.5f * m_fov));
        Matrix4f perspective(cot, 0.f, 0.f, 0.f,
                             0.f, cot, 0.f, 0.f,
                             0.f, 0.f, m_farClip*recip, 1.f,
                             0.f, 0.f, -m_nearClip*m_farClip*recip, 0.f);
        Transform cameraToSample = Transform::scale(0.5f/aspect, -0.5f, 1.f) *
                Transform::translate(aspect, -1.f, 0.f) * perspective;
        m_sampleToCamera = cameraToSample.inverse();
        if (!m_filter)
            m_filter = dynamic_cast<Filter*>(ObjectFactory::createInstance("box", ParamSet()));
    }

    float generateRay(const Point2f &sample, const Point2f &lensSample, Ray &ray) const
    {
        Point2f diskP = concentricDisk(lensSample.x, lensSample.y) * m_lensRadius;
        Point3f lensP(diskP.x, diskP.y, 0.f);

        Point4f samplePoint(sample.x * m_invOutputSize.x, sample.y * m_invOutputSize.y, 0.0f, 1.0f);
        Point3f nearP = m_sampleToCamera * samplePoint;
        Point3f focusP = nearP * (m_focusDistance / nearP.z);
        Vector3f d = glm::normalize(focusP - lensP);
        float invZ = 1.0f / d.z;

        ray.o = m_cameraToWorld * Point4f(lensP.x, lensP.y, lensP.z, 1.0f);
        ray.d = m_cameraToWorld * d;
        ray.mint = std::max(ray.mint, m_nearClip * invZ);
        ray.maxt = std::min(ray.maxt, m_farClip * invZ);

        return 1.0f;
    }

    void addChild(Object *pChild)
    {
        switch(pChild->getClassType())
        {
        case EFilter:
            {
                if (m_filter)
                    throw WispException("Camera: there can only be one filter per camera!");
                m_filter = dynamic_cast<Filter*>(pChild);
            }
            break;

        default:
            throw WispException(formatString("Camera::addChild(%s) is not supported!",
                                             classTypeName(pChild->getClassType()).c_str()));
            break;
        }
    }

    std::string toString() const
    {
        return std::string("PerspectiveCamera[]");
    }

private:
    Vector2f m_invOutputSize;
    Transform m_sampleToCamera;
    Transform m_cameraToWorld;
    float m_fov;
    float m_lensRadius;
    float m_focusDistance;
    float m_nearClip;
    float m_farClip;
};

WISP_REGISTER_CLASS(PerspectiveCamera, "perspective")

WISP_NAMESPACE_END
