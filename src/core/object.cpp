#include "object.h"

WISP_NAMESPACE_BEGIN

void Object::addChild(Object *pChild)
{
    pChild;
    assert (0 && "addChild not implemented!");
}

void Object::prepare() {}

void Object::setParent(Object *pParent)
{
    pParent;
}

std::map<std::string, ObjectFactory::Constructor>* ObjectFactory::m_constructors = NULL;

void ObjectFactory::registerClass(const std::string &strName, const Constructor &constr)
{
    if (!m_constructors)
        m_constructors = new std::map<std::string, ObjectFactory::Constructor>();
    (*m_constructors)[strName] = constr;
}

WISP_NAMESPACE_END
