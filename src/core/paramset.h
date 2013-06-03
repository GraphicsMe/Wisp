#ifndef PARAMSET_H
#define PARAMSET_H

#include "common.h"
#include "transform.h"
#include <boost/variant.hpp>

WISP_NAMESPACE_BEGIN
class ParamSet
{
public:
    ParamSet() {}

#define DEFINE_PARAM(Type, TypeName, XmlName) \
    void set##TypeName(const std::string& strName, const Type& value); \
    Type get##TypeName(const std::string& strName) const; \
    Type get##TypeName(const std::string& strName, const Type& defaultValue) const;

    /** example
     *
    void setBoolean(const std::string& strName, const bool& value);
    bool getBoolean(const std::string& strName) const;
    bool getBoolean(const std::string& strName, const bool& defaultValue) const;

    void setInteger(const std::string& strName, const int& value);
    int getInteger(const std::string& strName) const;
    int getInteger(const std::string& strName, const int& defaultValue) const;

    void setFloat(const std::string& strName, const float& value);
    float getFloat(const std::string& strName) const;
    float getFloat(const std::string& strName, const float& defaultValue) const;
    */

    DEFINE_PARAM(bool, Boolean, boolean)
    DEFINE_PARAM(int, Integer, integer)
    DEFINE_PARAM(float, Float, float)
    DEFINE_PARAM(Color3f, Color, color)
    DEFINE_PARAM(Point3f, Point, point)
    DEFINE_PARAM(Vector3f, Vector, vector)
    DEFINE_PARAM(std::string, String, string)
    DEFINE_PARAM(Transform, Transform, transform)

    bool hasProperty(const std::string& strName) const;

private:
    typedef boost::variant<bool, int, float, std::string,
            Color3f, Point3f, Transform> Param;
    typedef std::map<std::string, Param> ParamSetType;
    ParamSetType m_paramSet;
};
WISP_NAMESPACE_END

#endif // PARAMSET_H
