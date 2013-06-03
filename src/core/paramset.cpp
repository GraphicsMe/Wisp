#include "paramset.h"
#include <assert.h>

using std::map;
using std::string;

WISP_NAMESPACE_BEGIN

#define PARAM_ACCESSOR(Type, TypeName, XmlName) \
    void ParamSet::set##TypeName(const std::string& strName, const Type& value) \
    { \
        if (m_paramSet.find(strName) != m_paramSet.end()) \
            throw WispException(formatString("Param name '%s' not found!", strName.c_str())); \
            m_paramSet[strName] = value; \
    } \
    \
    Type ParamSet::get##TypeName(const std::string& strName) const \
    { \
        ParamSetType::const_iterator it = m_paramSet.find(strName); \
        if (it == m_paramSet.end()) \
            throw WispException(formatString("Param name '%s' not found!", strName.c_str())); \
        const Type* result = boost::get<Type>(&it->second); \
        if (!result) \
            throw WispException(formatString("Get param '%s' error!", strName.c_str())); \
        return *result; \
    } \
    \
    Type ParamSet::get##TypeName(const std::string& strName, const Type& defaultValue) const \
    { \
        ParamSetType::const_iterator it = m_paramSet.find(strName); \
        if (it == m_paramSet.end()) \
            return defaultValue; \
        const Type* result = boost::get<Type>(&it->second); \
        if (!result) \
            return defaultValue; \
        return *result; \
    } \

PARAM_ACCESSOR(bool, Boolean, boolean)
PARAM_ACCESSOR(int, Integer, integer)
PARAM_ACCESSOR(float, Float, float)
PARAM_ACCESSOR(Color3f, Color, color)
PARAM_ACCESSOR(Point3f, Point, point)
PARAM_ACCESSOR(Vector3f, Vector, vector)
PARAM_ACCESSOR(std::string, String, string)
PARAM_ACCESSOR(Transform, Transform, transform)

bool ParamSet::hasProperty(const std::string& strName) const
{
    return m_paramSet.find(strName) != m_paramSet.end();
}

WISP_NAMESPACE_END
