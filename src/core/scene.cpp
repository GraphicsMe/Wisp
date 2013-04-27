#include "scene.h"
#include "camera.h"
#include "sampler.h"
#include "integrator.h"

WISP_NAMESPACE_BEGIN

Scene::Scene(const ParamSet &)
    : m_camera(NULL)
    , m_sampler(NULL)
    , m_integrator(NULL)
    , m_rendering(false)
    , m_aggregate(NULL)
{

}

Scene::~Scene()
{
    if (m_sampler)
        delete m_sampler;
    if (m_camera)
        delete m_camera;
    if (m_integrator)
        delete m_integrator;
}

void Scene::prepare()
{
    if (m_aggregate == NULL)
    {
        ParamSet param;
        Object* obj = ObjectFactory::createInstance("kdtree", param);
        m_aggregate = static_cast<Shape*>(obj);
    }
    for (size_t i = 0; i < m_shapes.size(); ++i)
        m_aggregate->addChild(m_shapes[i]);
    m_aggregate->prepare();
}

void Scene::addChild(Object *obj)
{
    switch(obj->getClassType())
    {
    case EShape:
    case EMesh:
        {
            Shape* shape = dynamic_cast<Shape*>(obj);
            m_shapes.push_back(shape);
            m_bound.expand(shape->getBoundingBox());
        }
        break;

    case EAccelerator:
        {
            m_aggregate = dynamic_cast<Shape*>(obj);
        }
        break;

    case ESampler:
        {
            if (m_sampler)
                throw WispException("There can only be one sampler per scene!");
            m_sampler = dynamic_cast<Sampler*>(obj);
        }
        break;

    case ECamera:
        {
            if (m_camera)
                throw WispException("There can only be one camera per scene!");
            m_camera = dynamic_cast<Camera*>(obj);
        }
        break;

    case EIntegrator:
        {
            if (m_integrator)
                throw WispException("There can only be one intetrator per scene!");
            m_integrator = dynamic_cast<Integrator*>(obj);
        }
        break;

    default:
        throw WispException(formatString("Scene::addChild(%s) is not supported!",
                            classTypeName(obj->getClassType())));
    }
}

bool Scene::rayIntersect(const TRay& ray) const
{
    return m_aggregate->rayIntersect(ray);
}

bool Scene::rayIntersect(const TRay& ray, Intersection& its) const
{
    return m_aggregate->rayIntersect(ray, its);
}

std::string Scene::toString() const
{
    return std::string("Scene[]");
}

WISP_REGISTER_CLASS(Scene, "scene")

WISP_NAMESPACE_END
