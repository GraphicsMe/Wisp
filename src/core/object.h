#ifndef OBJECT_H
#define OBJECT_H

#include "common.h"
#include "paramset.h"
#include <boost/function.hpp>

WISP_NAMESPACE_BEGIN

class Object
{
public:
    enum EClassType {
        ETest = 0,
        EScene,
        EMesh,
        EShape,
        EBSDF,
        ELuminaire,
        ECamera,
        EMedium,
        ESampler,
        EIntegrator,
        EPhaseFunction,
        EFilter,
        EClassTypeCount
    };

    virtual ~Object() {}
    virtual void addChild(Object* pChild);
    virtual void setParent(Object* pParent);
    virtual void prepare();
    virtual EClassType getClassType() const = 0;
    virtual std::string toString() const = 0;

    inline static std::string classTypeName(EClassType eType)
    {
        switch(eType)
        {
        case EScene:      return "scene";
        case EMesh:       return "mesh";
        case EShape:      return "shape";
        case EBSDF:       return "bsdf";
        case ELuminaire:  return "luminaire";
        case ECamera:     return "camera";
        case EIntegrator: return "integrator";
        case ESampler:    return "sampler";
        case ETest:       return "test";
        case EFilter:     return "filter";
        default:          return "<unknow>";
        }
    }
};

class ObjectFactory
{
public:
    typedef boost::function<Object *(const ParamSet&)> Constructor;

    static void registerClass(const std::string& strName, const Constructor& constr);

    inline static Object* createInstance(const std::string& strName,
                                         const ParamSet& paramSet)
    {
        if (m_constructors->find(strName) == m_constructors->end())
            throw WispException(formatString("Object name '%s' not found!", strName.c_str()));

        return (*m_constructors)[strName](paramSet);
    }

private:
    static std::map<std::string, Constructor>* m_constructors;
};

#define WISP_REGISTER_CLASS(cls, name) \
    cls* cls##_create(const ParamSet& set) { \
        return new cls(set); \
    } \
    static struct cls##_ { \
        cls##_() { \
            ObjectFactory::registerClass(name, cls##_create); \
        } \
    }cls##__;

WISP_NAMESPACE_END

#endif // OBJECT_H
