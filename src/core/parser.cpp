#include "parser.h"
#include "paramset.h"
#include "transform.h"
#include <QtXml>
#include <QtXmlPatterns>
#include <QApplication>
#include <map>
#include <string>
#include <vector>

WISP_NAMESPACE_BEGIN
using std::string;
class Parser : public QXmlDefaultHandler
{
public:
    enum ETag {
        ETest           = Object::ETest,
        EScene          = Object::EScene,
        EMesh           = Object::EMesh,
        EShape          = Object::EShape,
        EAccelerator    = Object::EAccelerator,
        EBSDF           = Object::EBSDF,
        ELuminaire      = Object::ELuminaire,
        ECamera         = Object::ECamera,
        EMedium         = Object::EMedium,
        ESampler        = Object::ESampler,
        EIntegrator     = Object::EIntegrator,
        EPhaseFunction  = Object::EPhaseFunction,
        EFilter         = Object::EFilter,

        EBoolean = Object::EClassTypeCount,
        EInteger,
        EFloat,
        EString,
        EPoint,
        EVector,
        EColor,
        ETransform,
        ETranslate,
        ERotate,
        EScale,
        ELookAt
    };

    struct ParserContext
    {
        QXmlAttributes attr;
        ParamSet paramSet;
        std::vector<Object*> children;

        inline ParserContext(const QXmlAttributes& _attr) : attr(_attr) {}
    };

    Parser()
    {
        m_tags["test"]       = ETest;
        m_tags["scene"]      = EScene;
        m_tags["mesh"]       = EMesh;
        m_tags["shape"]      = EShape;
        m_tags["accelerator"]= EAccelerator;
        m_tags["bsdf"]       = EBSDF;
        m_tags["luminaire"]  = ELuminaire;
        m_tags["camera"]     = ECamera;
        m_tags["medium"]     = EMedium;
        m_tags["sampler"]    = ESampler;
        m_tags["integrator"] = EIntegrator;
        m_tags["phase"]      = EPhaseFunction;
        m_tags["rfilter"]    = EFilter;

        m_tags["boolean"]    = EBoolean;
        m_tags["integer"]    = EInteger;
        m_tags["float"]      = EFloat;
        m_tags["string"]     = EString;
        m_tags["point"]      = EPoint;
        m_tags["vector"]     = EVector;
        m_tags["color"]      = EColor;
        m_tags["transform"]  = ETransform;
        m_tags["translate"]  = ETranslate;
        m_tags["rotate"]     = ERotate;
        m_tags["scale"]      = EScale;
        m_tags["lookat"]     = ELookAt;
    }

    inline Object* getRoot() const
    {
        return m_root;
    }

private:
    float parseFloat(const QString &str) const
    {
        bool success;
        float result = str.toFloat(&success);
        if (!success)
            throw WispException(formatString("Unable to parse floating point value '%s'!", str.toStdString().c_str()));
        return result;
    }

    Vector3f parseVector(const QString &str) const
    {
        QRegExp re("[\\s,]+");
        QStringList list = str.split(re);
        if (list.size() != 3)
            throw WispException(formatString("Cannot parse 3-vector '%s'!", str.toStdString().c_str()));

        Vector3f result;
        for (int i=0; i<3; ++i)
            result[i] = parseFloat(list[i]);
        return result;
    }

    bool startElement(const QString&, const QString &localName, const QString &, const QXmlAttributes &atts)
    {
        //qDebug() << "startElement" << localName;

        ParserContext ctx(atts);
        if (localName == "transform")
        {
            m_transform.setIdentity();
        }
        else if (localName == "scene")
        {
            ctx.attr.append("type", "", "type", "scene");
        }

        m_context.push_back(ctx);
        return true;
    }

    bool endElement(const QString&, const QString &name, const QString&)
    {
        //qDebug() << "endElement" << name;
        std::map<string, ETag>::const_iterator it = m_tags.find(name.toStdString());
        if (it == m_tags.end())
            throw WispException(formatString("Encountered an unknown tag '%s'!", name.toAscii()));
        ParserContext& context = m_context.back();

        int tag = it->second;
        if (tag < Object::EClassTypeCount)
        {
            Object* obj = ObjectFactory::createInstance(context.attr.value("type").toStdString(), context.paramSet);
            if (obj->getClassType() != tag)
            {
                throw WispException(formatString("Unexpectedly constructed an object of type <%s> (expected type <%s>): %s",
                                Object::classTypeName(obj->getClassType()).c_str(),
                                Object::classTypeName(Object::EClassType(tag)).c_str(),
                                obj->toString().c_str()));
            }

            for (size_t i = 0; i < context.children.size(); ++i)
            {
                obj->addChild(context.children[i]);
                context.children[i]->setParent(obj);
            }

            obj->prepare();

            if (m_context.size() >= 2)
                m_context[m_context.size()-2].children.push_back(obj);
            else
                m_root = obj;
        }
        else
        {
            ParamSet& paramSet = m_context[m_context.size()-2].paramSet;
            bool success;
            switch(tag)
            {
            case EInteger:
                {
                    QString value = context.attr.value("value");
                    int result = value.toInt(&success);
                    if (!success)
                        throw WispException(formatString("Unable to parse integer value '%s'", value));
                    paramSet.setInteger(context.attr.value("name").toStdString(), result);
                }
                break;
            case EFloat:
                {
                    QString value = context.attr.value("value");
                    float result = value.toFloat(&success);
                    if (!success)
                        throw WispException(formatString("Unable to parse float value '%s'", value));
                    paramSet.setFloat(context.attr.value("name").toStdString(), result);
                }
                break;
            case EBoolean:
                {
                    QString value = context.attr.value("value").toLower();
                    if (value != "true" || value != "false")
                        throw WispException(formatString("Unable to parse boolean value '%s'", value));
                    bool result = value == QString("true");
                    paramSet.setBoolean(context.attr.value("name").toStdString(), result);
                }
                break;
             case EString:
                {
                    paramSet.setString(context.attr.value("name").toStdString(),
                                       context.attr.value("value").toStdString());

                }
                break;
            case EPoint:
            case EColor:
                {
                    Vector3f v = parseVector(context.attr.value("value"));
                    paramSet.setVector(context.attr.value("name").toStdString(), v);
                }
                break;
            case ETransform:
                paramSet.setTransform(context.attr.value("name").toStdString(), m_transform);
                break;
            case ETranslate:
                {
                    Vector3f v = parseVector(context.attr.value("value"));
                    m_transform = Transform::translate(v.x, v.y, v.z) * m_transform;
                }
                break;
            case EScale:
                {
                    Vector3f v = parseVector(context.attr.value("value"));
                    m_transform = Transform::scale(v.x, v.y, v.z) * m_transform;
                }
                break;
            case ERotate:
                {
                    float angle = parseFloat(context.attr.value("angle"));
                    Vector3f axis = glm::normalize(parseVector(context.attr.value("axis")));
                    m_transform = Transform::rotate(angle, axis) * m_transform;
                }
                break;
            case ELookAt:
                {
                    Point3f origin = parseVector(context.attr.value("origin"));
                    Point3f target = parseVector(context.attr.value("target"));
                    Vector3f up = glm::normalize(parseVector(context.attr.value("up")));

                    Vector3f dir = glm::normalize(target - origin);
                    Vector3f left = glm::cross(up, dir);
                    Vector3f newUp = glm::cross(dir, left);


                    Matrix4f toWorldMat(left.x, left.y, left.z, 0.f,
                                        newUp.x, newUp.y, newUp.z, 0.f,
                                        dir.x, dir.y, dir.z, 0.f,
                                        origin.x, origin.y, origin.z, 1.f);
                    Transform toWorld(toWorldMat);
                    m_transform = toWorld * m_transform;
                }
                break;
            }
        }
        m_context.pop_back();
        return true;
    }

private:
    Object* m_root;
    Transform m_transform;
    std::map<string, ETag> m_tags;
    std::vector<ParserContext> m_context;
};

class WispMessageHandler : public QAbstractMessageHandler
{
    void handleMessage(QtMsgType type, const QString &description,
                       const QUrl &, const QSourceLocation &sourceLocation)
    {
        const char* typeName;
        switch(type)
        {
        case QtDebugMsg: typeName = "Debug"; break;
        case QtWarningMsg: typeName = "Warning"; break;
        case QtCriticalMsg: typeName = "Critical"; break;
        case QtFatalMsg:
        default: typeName = "Fatal"; break;
        }

        QXmlStreamReader xml(description);
        QString text;
        while (!xml.atEnd())
            if (xml.readNext() == QXmlStreamReader::Characters)
                text += xml.text();

        std::cerr << typeName << ": " << qPrintable(text);
        if (!sourceLocation.isNull())
            std::cerr << " (line " << sourceLocation.line()
                 << ", col " << sourceLocation.column() << ")" << std::endl;
    }
};

Object* loadScene(const string& strFileName)
{
    //qDebug() << QDir::currentPath();
    qDebug() << QString::fromStdString(strFileName);
    QFile schemaFile(":/Schema/Schema/schema.xsd");
    QXmlSchema schema;

    WispMessageHandler handler;
    schema.setMessageHandler(&handler);
    if (!schemaFile.open(QIODevice::ReadOnly))
        throw WispException(formatString("Unable to open the XML schema!"));
    if (!schema.load(schemaFile.readAll()))
        throw WispException(formatString("Unable to parse the XML schema!"));

    QXmlSchemaValidator validator(schema);
    QFile file(QString::fromStdString(strFileName));
    if (!file.open(QIODevice::ReadOnly))
        throw WispException(formatString("Unable to open the file \"%s\"", strFileName.c_str()));
    if (!validator.validate(&file))
        throw WispException(formatString("Unable to validate the file \"%s\"", strFileName.c_str()));

    file.seek(0);
    Parser parser;
    QXmlSimpleReader reader;
    reader.setContentHandler(&parser);
    QXmlInputSource source(&file);
    if (!reader.parse(source))
        throw WispException(formatString("Unable to parse the file \"%s\"", strFileName.c_str()));

    return parser.getRoot();
}

WISP_NAMESPACE_END
