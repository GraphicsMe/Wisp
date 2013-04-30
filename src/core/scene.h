#ifndef SCENE_H
#define SCENE_H

#include "common.h"
#include "object.h"
#include "shape.h"
#include "geometry.h"
#include "light.h"

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

    void prepare();
    void addChild(Object *obj);
    bool rayIntersect(const TRay& ray) const;
    bool rayIntersect(const TRay& ray, Intersection& its) const;

    AreaLight* getAreaLight() const { return m_areaLight; }
    void addAreaLight(AreaLight* pAreaLight) { m_areaLight = pAreaLight; }

    EClassType getClassType() const { return EScene; }
    std::string toString() const;

private:
    bool m_rendering;
    BBox m_bound;
    Camera* m_camera;
    Sampler* m_sampler;
    Integrator* m_integrator;
    std::vector<Shape*> m_shapes;
    Shape* m_aggregate;
    AreaLight* m_areaLight;
};

WISP_NAMESPACE_END

#endif // SCENE_H
