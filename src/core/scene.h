#ifndef SCENE_H
#define SCENE_H

#include "common.h"
#include "object.h"
#include "shape.h"
#include "geometry.h"
#include "light.h"
#include "distribution1D.h"

WISP_NAMESPACE_BEGIN
class Scene : public Object
{
public:
    Scene(const ParamSet&);
    virtual ~Scene();

    inline bool isRendering() const { return m_rendering; }
    inline void stop() { m_rendering = false; }
    inline void start() { m_rendering = true; }
    inline const BBox& getBoundingBox() const { return m_bound; }
    inline const Camera* getCamera() const { return m_camera; }
    inline Sampler* getSampler() { return m_sampler; }
    inline const Sampler* getSampler() const { return m_sampler; }
    inline const Integrator* getIntegrator() const { return m_integrator; }
    inline const std::vector<Shape* >& getShapes() const { return m_shapes; }
    inline const Medium* getMedium() const { m_medium; }

    void prepare();
    void addChild(Object *obj);
    bool rayIntersect(const Ray& ray) const;
    bool rayIntersect(const Ray& ray, Intersection& its) const;

    // light related
    float pdfLight(const Point3f& p, LightSamplingRecord& lRec) const;
    bool sampleLight(Point3f& p, LightSamplingRecord& lRec, const Point2f& sample, float epsilon) const;

    // medium related
    Color3f evalTransmittance(const Ray& ray, Sampler* sampler) const;
    bool sampleDistance(const Ray& ray, Sampler* sampler, float& t, Color3f& weight) const;

    EClassType getClassType() const { return EScene; }
    std::string toString() const;

private:
    bool m_rendering;
    BBox m_bound;
    Camera* m_camera;
    Medium* m_medium;
    Sampler* m_sampler;
    Shape* m_aggregate;
    Integrator* m_integrator;
    std::vector<Shape*> m_shapes;
    Distribution1D m_lightPDf;
    std::vector<Light*> m_lights;
};

WISP_NAMESPACE_END

#endif // SCENE_H
