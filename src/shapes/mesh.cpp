#include "shape.h"
#include <fstream>
#include <sstream>
#include <unordered_map>

#include <boost/functional/hash.hpp>
WISP_NAMESPACE_BEGIN

class TriangleMesh : public Shape
{
public:
    friend class Triangle;

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
    Triangle(uint32_t index, TriangleMesh* pMesh)
        : m_pMesh(pMesh)
        , m_index(&pMesh->m_vertexIndex[3*index])
    {

    }

    ~Triangle()
    {

    }

    virtual void prepare()
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

    virtual void addChild(Object *pChild)
    {
        pChild;
    }

    virtual void samplePosition(const Point2f& sample, Point3f& p, Normal3f& n) const
    {
        Vector3f dir = uniformSphere(sample.x, sample.y);
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

        return true;
    }

    virtual void fillIntersectionRecord(const TRay& ray, Intersection& its) const
    {
        its.p = ray(its.t);
        its.uv = Point2f(1.0f, 1.0f);
        //its.color = m_diffuse;
        its.shape = this;
        //its.geoFrame = Frame(glm::normalize(its.p-m_center));
        //its.shFrame = its.geoFrame;
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
    std::shared_ptr<TriangleMesh> m_pMesh;
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
        fileName = "E:/Code/QT/Wisp/scenes/cbox/" + fileName;
        std::ifstream inputStream(fileName);
        if (inputStream.bad())
            throw WispException(formatString("Open object file %s error!", fileName.c_str()));

        std::cout << "Loading \"" << fileName << "\" beginning ..." << std::endl;
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

        std::cout << "Loading \"" << fileName << "\" finish!" << std::endl;
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

        for (size_t i = 0; i < m_triangleCount; ++i)
        {
            Triangle* pTri = new Triangle(i, this);
            m_bound.expand(pTri->getBoundingBox());
            m_triangles.push_back(TrianglePtr(pTri));
        }
    }

    virtual void prepare()
    {

    }

    virtual float area() const
    {
        return 1.0f;

    }

    virtual float pdf() const
    {
        return 1.0f / area();
    }

    virtual void addChild(Object *pChild)
    {
        pChild;
    }

    virtual void samplePosition(const Point2f& sample, Point3f& p, Normal3f& n) const
    {
        Vector3f dir = uniformSphere(sample.x, sample.y);
    }

    virtual bool rayIntersect(const TRay& ray, Intersection& its)
    {
        bool hitSomething = false;
        for (int i = 0; i < m_triangles.size(); ++i)
        {
            const TrianglePtr& tri = m_triangles[i];
            if (tri->rayIntersect(ray, its))
            {
                ray.maxt = its.t;
                its.color = m_diffuse;
                hitSomething = true;
            }
        }
        return hitSomething;
    }

    virtual void fillIntersectionRecord(const TRay& ray, Intersection& its) const
    {
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
    typedef std::shared_ptr<Triangle> TrianglePtr;
    std::vector<TrianglePtr> m_triangles;
	Color3f m_diffuse;
    BBox m_bound;
};



//WISP_REGISTER_CLASS(Triangle, "triangle")
WISP_REGISTER_CLASS(WavefrontOBJ, "obj")

WISP_NAMESPACE_END
