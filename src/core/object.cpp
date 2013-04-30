#include "object.h"

WISP_NAMESPACE_BEGIN

void Object::addChild(Object *pChild)
{
    throw WispException(formatString("Object::addChild is not implemented for object of type %s",
                                     classTypeName(pChild->getClassType()).c_str()));
}

void Object::prepare() {}

void Object::setParent(Object*) {}

std::map<std::string, ObjectFactory::Constructor>* ObjectFactory::m_constructors = NULL;

void ObjectFactory::registerClass(const std::string &strName, const Constructor &constr)
{
    if (!m_constructors)
        m_constructors = new std::map<std::string, ObjectFactory::Constructor>();
    (*m_constructors)[strName] = constr;
}

WISP_NAMESPACE_END
