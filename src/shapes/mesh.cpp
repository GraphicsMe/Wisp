#include "shape.h"
#include "scene.h"
#include "bsdf.h"
#include "distribution1D.h"
#include <fstream>
#include <sstream>
#include <unordered_map>

#include <boost/functional/hash.hpp>
WISP_NAMESPACE_BEGIN

class TriangleMesh : public Shape
{
public:
    TriangleMesh() :
        m_vertexIndex(NULL),
        m_vertexPositions(NULL),
        m_vertexNormals(NULL),
        m_vertexTexCoords(NULL)
    {
    }

    friend class Triangle;

    virtual bool canIntersect() const { return false; }
    EClassType getClassType() const { return EMesh; }

protected:
    uint32_t m_vertexCount;
    uint32_t m_triangleCount;
    uint32_t* m_vertexIndex;
    Point3f*  m_vertexPositions;
    Vector3f* m_vertexNormals;
    Point2f*  m_vertexTexCoords;
};

class Triangle : public Shape
{
public:
    friend class TriangleMesh;
    friend class WavefrontOBJ;

    Triangle(uint32_t index, TriangleMesh* pMesh, BSDF* pBSDF, Light* pLight)
        : m_pMesh(pMesh)
        , m_index(&pMesh->m_vertexIndex[3*index])
    {
        m_bsdf = pBSDF;
        m_light = pLight;
    }

    ~Triangle()
    {

    }

    virtual float area() const
    {
        Point3f p1 = m_pMesh->m_vertexPositions[m_index[0]];
        Point3f p2 = m_pMesh->m_vertexPositions[m_index[1]];
        Point3f p3 = m_pMesh->m_vertexPositions[m_index[2]];
        Vector3f e1 = p2 - p1;
        Vector3f e2 = p3 - p1;
        return glm::length(glm::cross(e1, e2)) * 0.5f;

    }

    virtual float pdf() const
    {
        return 1.0f / area();
    }

    virtual bool rayIntersect(const TRay& ray)
    {
        Point3f p1 = m_pMesh->m_vertexPositions[m_index[0]];
        Point3f p2 = m_pMesh->m_vertexPositions[m_index[1]];
        Point3f p3 = m_pMesh->m_vertexPositions[m_index[2]];
        Vector3f e1 = p2 - p1;
        Vector3f e2 = p3 - p1;
        Vector3f s1 = glm::cross(ray.d, e2);
        float divisor = glm::dot(s1, e1);

        if (divisor == 0.)
            return false;
        float invDivisor = 1.f / divisor;

        // Compute first barycentric coordinate
        Vector3f d = ray.o - p1;
        float b1 = glm::dot(d, s1) * invDivisor;
        if (b1 < 0. || b1 > 1.)
            return false;

        // Compute second barycentric coordinate
        Vector3f s2 = glm::cross(d, e1);
        float b2 = glm::dot(ray.d, s2) * invDivisor;
        if (b2 < 0. || b1 + b2 > 1.)
            return false;

        // Compute _t_ to intersection point
        float t = glm::dot(e2, s2) * invDivisor;
        if (t < ray.mint || t > ray.maxt)
            return false;

        return true;
    }

    virtual bool rayIntersect(const TRay& ray, Intersection& its)
    {
        Point3f p1 = m_pMesh->m_vertexPositions[m_index[0]];
        Point3f p2 = m_pMesh->m_vertexPositions[m_index[1]];
        Point3f p3 = m_pMesh->m_vertexPositions[m_index[2]];
        Vector3f e1 = p2 - p1;
        Vector3f e2 = p3 - p1;
        Vector3f s1 = glm::cross(ray.d, e2);
        float divisor = glm::dot(s1, e1);

        if (divisor == 0.)
            return false;
        float invDivisor = 1.f / divisor;

        // Compute first barycentric coordinate
        Vector3f d = ray.o - p1;
        float b1 = glm::dot(d, s1) * invDivisor;
        if (b1 < 0. || b1 > 1.)
            return false;

        // Compute second barycentric coordinate
        Vector3f s2 = glm::cross(d, e1);
        float b2 = glm::dot(ray.d, s2) * invDivisor;
        if (b2 < 0. || b1 + b2 > 1.)
            return false;

        // Compute _t_ to intersection point
        float t = glm::dot(e2, s2) * invDivisor;
        if (t < ray.mint || t > ray.maxt)
            return false;

        //intersection record
        its.t = t;
        its.p = ray(its.t);
        its.uv = Point2f(1.0f, 1.0f);
        //its.color = m_diffuse;
        its.shape = this;
        its.geoFrame = Frame(glm::normalize(glm::cross(e1, e2)));
        its.shFrame = its.geoFrame;
        its.wo = its.toLocal(-ray.d);

        return true;
    }

    Point3f sample(Normal3f& normal, const Point2f& sample) const
    {
        Point3f p1 = m_pMesh->m_vertexPositions[m_index[0]];
        Point3f p2 = m_pMesh->m_vertexPositions[m_index[1]];
        Point3f p3 = m_pMesh->m_vertexPositions[m_index[2]];
        Vector3f e1 = p2 - p1;
        Vector3f e2 = p3 - p1;

        Point2f b = uniformTriangle(sample.x, sample.y);
        Point3f p = p1 + e1 * b.x + e2 * b.y;

        if (m_pMesh->m_vertexNormals)
        {
            Normal3f n0 = m_pMesh->m_vertexNormals[m_index[0]];
            Normal3f n1 = m_pMesh->m_vertexNormals[m_index[1]];
            Normal3f n2 = m_pMesh->m_vertexNormals[m_index[2]];
            normal = glm::normalize(n0 * (1.0f - b.x - b.y) + n1 * b.x + n2 * b.y);
        }
        else
            normal = glm::normalize(glm::cross(e1, e2));
        return p;
    }

    virtual void fillIntersectionRecord(const TRay& ray, Intersection& its) const
    {
        Point3f p1 = m_pMesh->m_vertexPositions[m_index[0]];
        Point3f p2 = m_pMesh->m_vertexPositions[m_index[1]];
        Point3f p3 = m_pMesh->m_vertexPositions[m_index[2]];
        Vector3f e1 = p2 - p1;
        Vector3f e2 = p3 - p1;
        its.p = ray(its.t);
        its.uv = Point2f(1.0f, 1.0f);
        //its.color = m_diffuse;
        its.shape = this;
        its.geoFrame = Frame(glm::normalize(glm::cross(e1, e2)));
        its.shFrame = its.geoFrame;
    }

    virtual BBox getBoundingBox() const
    {
        Point3f p1 = m_pMesh->m_vertexPositions[m_index[0]];
        Point3f p2 = m_pMesh->m_vertexPositions[m_index[1]];
        Point3f p3 = m_pMesh->m_vertexPositions[m_index[2]];
        BBox box(p1, p2);
        box.expand(p3);
        return box;
    }

    std::string toString() const
    {
        return std::string("Triangle[]");
    }

private:
    uint32_t* m_index;
	TriangleMesh* m_pMesh;
    //Point3f p1, p2, p3;
    //Color3f m_diffuse;
};

class WavefrontOBJ : public TriangleMesh
{
public:
    WavefrontOBJ(const ParamSet& paramSet)
    {
        m_diffuse = paramSet.getColor("albedo", Color3f(1.0f, 0.0f, 0.0f));
        Transform toWorld = paramSet.getTransform("toWorld", Transform());
        std::string fileName = paramSet.getString("filename");
        fileName = g_sceneDirectory + fileName;
        std::ifstream inputStream(fileName);
        if (inputStream.bad())
            throw WispException(formatString("Open object file %s error!", fileName.c_str()));

        std::cout << "Loading \"" << paramSet.getString("filename") << "\" beginning ... ";
        std::string line;
        std::vector<Point3f>    positions;
        std::vector<Point2f>    texcoords;
        std::vector<Vector3f>   normals;
        std::vector<int>        indices;
        std::vector<OBJVertex>  vertices;
        VertexMap               vertexMap;

        while (getline(inputStream, line))
        {
            std::string prefix;
            std::istringstream lineStream(line);

            lineStream >> prefix;
            if (prefix == "v")
            {
                Point3f p;
                lineStream >> p.x >> p.y >> p.z;
                p = toWorld * Point4f(p.x, p.y, p.z, 1.0f);
                positions.push_back(p);
            }
            else if (prefix == "vt")
            {
                Point2f vt;
                lineStream >> vt.x >> vt.y;
                texcoords.push_back(vt);
            }
            else if (prefix == "vn")
            {
                Vector3f vn;
                lineStream >> vn.x >> vn.y >> vn.z;
                normals.push_back(vn);
            }
            else if (prefix == "f")
            {
                std::string str1, str2, str3;
                lineStream >> str1 >> str2;
                OBJVertex v[3] = {str1, str2};
                while (lineStream >> str3)
                {
                    v[2] = OBJVertex(str3);
                    for (size_t i = 0; i < 3; ++i)
                    {
                        VertexMap::const_iterator it = vertexMap.find(v[i]);
                        if (it == vertexMap.end())
                        {
                            vertexMap[v[i]] = vertices.size();
                            indices.push_back(vertices.size());
                            vertices.push_back(v[i]);
                        }
                        else
                        {
                            indices.push_back(it->second);
                        }
                    }
                    v[1] = v[2];
                }
            }
        }

        m_triangleCount = indices.size() / 3;
        m_vertexCount = vertices.size();

        std::cout << "finish!" << std::endl;
        std::cout << "\t Read " << m_triangleCount << " triangles and " << m_vertexCount << " vertices." << std::endl;

        m_vertexIndex = new uint32_t[indices.size()];
        for (size_t i = 0; i < indices.size(); ++i)
            m_vertexIndex[i] = indices[i];

        m_vertexPositions = new Point3f[m_vertexCount];
        for (size_t i = 0; i < m_vertexCount; ++i)
            m_vertexPositions[i] = positions[vertices[i].p];

        if (!normals.empty())
        {
            m_vertexNormals = new Vector3f[m_vertexCount];
            for (size_t i = 0; i < m_vertexCount; ++i)
                m_vertexNormals[i] = normals[vertices[i].n];
        }

        if (!texcoords.empty())
        {
            m_vertexTexCoords = new Point2f[m_vertexCount];
            for (size_t i = 0; i < m_vertexCount; ++i)
                m_vertexTexCoords[i] = texcoords[vertices[i].uv];
        }
    }

    virtual float area() const
    {
        return m_distr.getSum();
    }

    virtual float pdf() const
    {
        return m_distr.getNormalization();
    }

    float pdfArea(const ShapeSamplingRecord &) const
    {
        return m_distr.getNormalization();
    }

    virtual void refine(std::vector<ShapePtr>& refined) const
    {
        for (size_t i = 0; i < m_triangles.size(); ++i)
        {
            refined.push_back(m_triangles[i]);
        }
    }

    virtual float sampleArea(ShapeSamplingRecord &sRec, const Point2f &_sample) const
    {
        Point2f sample(_sample);
        int index = m_distr.sampleReuse(sample.y);
        sRec.p = m_triangles[index]->sample(sRec.n, sample);
        return m_distr.getNormalization();
    }

    /*virtual void samplePosition(const Point2f& _sample, Point3f& p, Normal3f& n) const
    {
        Point2f sample(_sample);
        size_t index = m_distr.sampleReuse(sample.x);

        const Triangle* pTriangle = static_cast<const Triangle*>(m_triangles[index]);
        Point3f p0 = m_vertexPositions[pTriangle->m_index[0]];
        Point3f p1 = m_vertexPositions[pTriangle->m_index[1]];
        Point3f p2 = m_vertexPositions[pTriangle->m_index[2]];
        Point2f b = uniformTriangle(sample.x, sample.y);
        p = p0 * (1.0f - b.x - b.y) + p1 * b.x + p2 * b.y;

        if (m_vertexNormals)
        {
            Normal3f n0 = m_vertexNormals[pTriangle->m_index[0]];
            Normal3f n1 = m_vertexNormals[pTriangle->m_index[1]];
            Normal3f n2 = m_vertexNormals[pTriangle->m_index[2]];
            n = glm::normalize(n0 * (1.0f - b.x - b.y) + n1 * b.x + n2 * b.y);
        }
        else
            n = glm::normalize(glm::cross(p1-p0, p2-p0));
    }*/

    virtual void fillIntersectionRecord(const TRay& ray, Intersection& its) const
    {
        its.shape->fillIntersectionRecord(ray, its);
        //its.p = ray(its.t);
        //its.uv = Point2f(1.0f, 1.0f);
        //its.color = m_diffuse;
        //its.shape = this;
        //its.geoFrame = Frame(glm::normalize(its.p-m_center));
        //its.shFrame = its.geoFrame;
    }

    virtual BBox getBoundingBox() const
    {
        return m_bound;
    }

    std::string toString() const
    {
        return std::string("WavefrontOBJ[]");
    }

    virtual void addChild(Object* pChild)
    {
        switch(pChild->getClassType())
        {
        case EBSDF:
            if (m_bsdf)
            {
                std::cout << m_bsdf->toString() << std::endl;
                throw WispException("Shape: try to register multiple BSDF intances!");
            }
            m_bsdf = static_cast<BSDF*>(pChild);
            break;
        case ELuminaire:
            m_light = static_cast<Light*>(pChild);
            break;

        default:
            throw WispException(formatString("Shape::addChild(%s) is not supported!",
                                             pChild->getClassType()).c_str());
        }
    }

    void prepare()
    {
        if (!m_bsdf)
            m_bsdf = static_cast<BSDF*>(ObjectFactory::createInstance("diffuse", ParamSet()));
        for (size_t i = 0; i < m_triangleCount; ++i)
        {
            Triangle* pTri = new Triangle(i, this, m_bsdf, m_light);
            m_bound.expand(pTri->getBoundingBox());
            m_triangles.push_back(pTri);
        }

        m_distr.clear();
        m_distr.reserve(m_triangleCount);
        for (size_t i = 0; i < m_triangleCount; ++i)
            m_distr.append(m_triangles[i]->area());
        m_distr.normalize();
    }

protected:
    struct OBJVertex
    {
        int p, n, uv;
        OBJVertex()
        {}
        OBJVertex(const std::string& indices)
        {
            p = n = uv = 0;
            int ret = sscanf (indices.c_str(), "%d/%d/%d", &p, &uv, &n);
            assert (ret == 3 || ret == 1);
            --p;
            --uv;
            --n;
        }

        inline bool operator==(const OBJVertex& v) const
        {
            return (v.p == p && v.n == n && v.uv == uv);
        }
    };

    struct OBJVertexHash : std::unary_function<OBJVertex, size_t>
    {
        std::size_t operator()(const OBJVertex &v) const
        {
            size_t hash = 0;
            hash_combine(hash, v.p);
            hash_combine(hash, v.n);
            hash_combine(hash, v.uv);
            return hash;
        }
    };

    typedef std::unordered_map<OBJVertex, int, OBJVertexHash> VertexMap;
    //typedef std::shared_ptr<Triangle> TrianglePtr;
    std::vector<Triangle*> m_triangles;
	Color3f m_diffuse;
    BBox m_bound;
    Distribution1D m_distr;
};

//WISP_REGISTER_CLASS(Triangle, "triangle")
WISP_REGISTER_CLASS(WavefrontOBJ, "obj")

WISP_NAMESPACE_END
