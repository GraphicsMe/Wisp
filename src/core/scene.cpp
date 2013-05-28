#include "scene.h"
#include "camera.h"
#include "medium.h"
#include "sampler.h"
#include "integrator.h"

WISP_NAMESPACE_BEGIN

Scene::Scene(const ParamSet &)
    : m_camera(NULL)
    , m_medium(NULL)
    , m_sampler(NULL)
    , m_aggregate(NULL)
    , m_integrator(NULL)
    , m_rendering(false)
{

}

Scene::~Scene()
{
    if (m_camera) delete m_camera;
    if (m_medium) delete m_medium;
    if (m_sampler) delete m_sampler;
    if (m_aggregate) delete m_aggregate;
    if (m_integrator) delete m_integrator;
}

void Scene::prepare()
{
    if (m_aggregate == NULL)
        m_aggregate = static_cast<Shape*>(ObjectFactory::createInstance("kdtree", ParamSet()));

    for (size_t i = 0; i < m_shapes.size(); ++i)
        m_aggregate->addChild(m_shapes[i]);
    m_aggregate->prepare();

    for (size_t i = 0; i < m_lights.size(); ++i)
        m_lightPDf.append(1.f);
    m_lightPDf.normalize();

}

void Scene::addChild(Object *obj)
{
    switch(obj->getClassType())
    {
    case EShape:
    case EMesh:
        {
            Shape* shape = static_cast<Shape*>(obj);
            if (shape->isLight())
                m_lights.push_back(shape->getLight());
            m_shapes.push_back(shape);
            m_bound.expand(shape->getBoundingBox());
        }
        break;

    case EAccelerator:
        {
            m_aggregate = static_cast<Shape*>(obj);
        }
        break;

    case ESampler:
        {
            if (m_sampler)
                throw WispException("There can only be one sampler per scene!");
            m_sampler = static_cast<Sampler*>(obj);
        }
        break;

    case ECamera:
        {
            if (m_camera)
                throw WispException("There can only be one camera per scene!");
            m_camera = static_cast<Camera*>(obj);
        }
        break;

    case EIntegrator:
        {
            if (m_integrator)
                throw WispException("There can only be one intetrator per scene!");
            m_integrator = static_cast<Integrator*>(obj);
        }
        break;

    case EMedium:
        {
            if (m_medium)
                throw WispException("There can only be one medium per scene!");
            m_medium = static_cast<Medium*>(obj);
        }
        break;

    default:
        throw WispException(formatString("Scene::addChild(%s) is not supported!",
                            classTypeName(obj->getClassType())));
    }
}

bool Scene::rayIntersect(const Ray& ray) const
{
    return m_aggregate->rayIntersect(ray);
}

bool Scene::rayIntersect(const Ray& ray, Intersection& its) const
{
    return m_aggregate->rayIntersect(ray, its);
}

float Scene::pdfLight(const Point3f& p, LightSamplingRecord& lRec) const
{
    const Light* light = lRec.light;
    float fraction = m_lightPDf.getNormalization();
    return light->pdf(p, lRec) * fraction;
}

bool Scene::sampleLight(const Point3f& p, LightSamplingRecord& lRec, const Point2f& s, float epsilon) const
{
    Point2f sample(s);
    float lumPdf;
    size_t index = m_lightPDf.sampleReuse(sample.x, lumPdf);
    assert (lumPdf == 1.0f);
    Light* light = m_lights[index];
    if (!light)
        throw WispException(formatString("index: %d", index));
    light->sample_f(p, lRec, sample);
    if (lRec.pdf == 0.f)
        return false;

    Vector3f dir = lRec.sRec.p - p;
    float length = glm::length(dir);
    Ray ray(p, dir/length, epsilon, length*(1.0f - ShadowEpsilon));
    if (this->rayIntersect(ray))
        return false;

    Color3f temp = lRec.value;
    lRec.pdf *= lumPdf;
    lRec.value /= lRec.pdf;
    lRec.light = light;
    return true;
}

bool Scene::sampleAttenuatedLight(const Point3f& p, LightSamplingRecord& lRec,
            const Point2f& s, float epsilon, Sampler* sampler) const
{
    Point2f sample(s);
    float lumPdf;
    size_t index = m_lightPDf.sampleReuse(sample.x, lumPdf);
    assert (lumPdf == 1.0f);
    Light* light = m_lights[index];
    if (!light)
        throw WispException(formatString("index: %d", index));
    light->sample_f(p, lRec, sample);
    if (lRec.pdf == 0.f)
        return false;

    Vector3f dir = lRec.sRec.p - p;
    float length = glm::length(dir);
    Ray ray(p, dir/length, epsilon, length*(1.0f - ShadowEpsilon));
    if (this->rayIntersect(ray))
        return false;
    Color3f trans = evalTransmittance(ray, sampler);
    assert (trans.x <= 1.f && trans.y <=  1.f && trans.z <= 1.f);
    lRec.value *= trans;
    lRec.pdf *= lumPdf;
    lRec.value /= lRec.pdf;
    lRec.light = light;
    return true;
}

Color3f Scene::evalTransmittance(const Ray& ray, Sampler* sampler) const
{
    if (m_medium)
        return m_medium->evalTransmittance(ray, sampler);
    return Color3f(1.f);
}

bool Scene::sampleDistance(const Ray& ray, Sampler* sampler, float& t, Color3f& weight, float& albedo) const
{
    weight = Color3f(1.0f);
    if (m_medium)
        return m_medium->sampleDistance(ray, sampler, t, weight, albedo);
    return false;
}

std::string Scene::toString() const
{
    return std::string("Scene[]");
}

WISP_REGISTER_CLASS(Scene, "scene")

WISP_NAMESPACE_END
