#include "aggregate.h"

WISP_NAMESPACE_BEGIN

struct KdAccelNode
{
    float SplitPos() const { return split; }
    size_t nPrimitives() const { return nPrims >> 2; }
    size_t SplitAxis() const { return flags & 3; }
    bool IsLeaf() const { return (flags & 3) == 3; }
    size_t AboveChild() const { return aboveChild >> 2; }

    void initLeaf(size_t* primNums, int np)
    {
        flags = 3;
        nPrims |= (np << 2);
        if (np == 0)
            onePrimitive = 0;
        else if (np == 1)
            onePrimitive = primNums[0];
        else
        {
            primitives = new size_t[np];
            for (int i = 0; i < np; ++i)
                primitives[i] = primNums[i];
        }
    }

    void initInterior(size_t axis, size_t ac, float s)
    {
        split = s;
        flags = axis;
        aboveChild |= (ac << 2);
    }

    union
    {
        float split;            // Interior
        size_t onePrimitive;    // leaf
        size_t* primitives;     // leaf
    };
private:
    union
    {
        size_t flags;       // both
        size_t nPrims;      // leaf
        size_t aboveChild;  // interior
    };
};

struct BoundEdge
{

    BoundEdge() {}
    BoundEdge(float tt, int pn, bool starting) : t(tt), primNum(pn)
    {
        type = starting ? START : END;
    }
    bool operator < (const BoundEdge& rhs) const
    {
        if (t == rhs.t)
        {
			if (primNum == rhs.primNum)
				return (int)type > (int)rhs.type;
			else
				return (int)type < (int)rhs.type;
		}
        return t < rhs.t;
    }

    float t;
    int primNum;
    enum { START = 1, END = 0 } type;
};

struct KdToDo {
    const KdAccelNode *node;
    float tmin, tmax;
};

const int MAX_TODO = 64;

class KdTreeAccel : public Aggregate
{
public:
    KdTreeAccel(const ParamSet& paramSet)
    {
        m_isectCost = paramSet.getInteger("intersectCost", 80);
        m_traversalCost = paramSet.getInteger("traversalCost", 1);
        m_emptyBonus = paramSet.getFloat("emptyBonus", 0.5f);
        m_maxPrims = paramSet.getInteger("maxPrims", 1);
        m_maxDepth = paramSet.getInteger("maxDepth", -1);
    }

	~KdTreeAccel()
	{
		delete[] m_nodes;
	}

    virtual void prepare()
    {
		std::vector<ShapePtr>allPrimitives;
		for (int i = 0; i < m_primitives.size(); ++i)
			m_primitives[i]->fullyRefine(allPrimitives);
		m_primitives.swap(allPrimitives);

        m_nextFreeNode = m_nAllocedNodes = 0;
        if (m_maxDepth <= 0)
            m_maxDepth = round2Int(8 + 1.3f * log2Int(float(m_primitives.size())));
        std::vector<BBox> primBounds;
        primBounds.reserve(m_primitives.size());
		for (size_t i = 0; i < m_primitives.size(); ++i)
        {
            BBox b = m_primitives[i]->getBoundingBox();
            m_bounds.expand(b);
            primBounds.push_back(b);
        }

        BoundEdge* edges[3];
        for (int i = 0; i < 3; ++i)
            edges[i] = new BoundEdge[2 * m_primitives.size()];
        size_t* prims0 = new size_t[m_primitives.size()];
        size_t* prims1 = new size_t[(m_maxDepth+1) * m_primitives.size()] ;

        size_t* primNums = new size_t[m_primitives.size()];
        for (size_t i = 0; i < m_primitives.size(); ++i)
            primNums[i] = i;
        buildTree(0, m_bounds, primBounds, primNums, m_primitives.size(),
                  m_maxDepth, edges, prims0, prims1);
        delete [] primNums;
        for (int i = 0; i < 3; ++i)
            delete [] edges[i];
        delete[] prims0;
        delete[] prims1;
    }

    virtual float area() const
    {
        return 1.0f;
    }

    virtual float pdf() const
    {
        return 0.0f;
    }

	bool canIntersect() const
	{
		return true;
	}

    virtual void addChild(Object *pChild)
    {
        Shape* shape = static_cast<Shape*>(pChild);
        assert (shape != NULL);
        m_primitives.push_back(ShapePtr(shape));
    }

    virtual void samplePosition(const Point2f& sample, Point3f& p, Normal3f& n) const
    {
		n;
        Vector3f dir = uniformSphere(sample.x, sample.y);
    }

    virtual void fillIntersectionRecord(const TRay& ray, Intersection& its) const
    {
		ray;
		its;
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
        return std::string("KdTreeAccel[]");
    }

private:
    void buildTree(int nodeNum, const BBox& nodeBounds,
                   const std::vector<BBox>& allPrimBounds,  size_t* primNums,
                   int nPrimitives, int depth, BoundEdge* edges[],
                   size_t* prims0, size_t* prims1, int badRefines = 0)
    {
        assert(nodeNum == m_nextFreeNode);
        if (m_nextFreeNode == m_nAllocedNodes)
        {
            int nAlloc = std::max(2 * m_nAllocedNodes, 512);
            KdAccelNode* nodes = new KdAccelNode[nAlloc];
            if (m_nAllocedNodes > 0)
            {
                memcpy(nodes, m_nodes, m_nAllocedNodes * sizeof(KdAccelNode));
                delete[] m_nodes;
            }
            m_nodes = nodes;
            m_nAllocedNodes = nAlloc;
        }
        ++m_nextFreeNode;

        if (nPrimitives <= m_maxPrims || depth == 0)
        {
            m_nodes[nodeNum].initLeaf(primNums, nPrimitives);
            return;
        }

        int bestAxis = -1, bestOffset = -1;
        float bestCost = Infinity;
        float oldCost = m_isectCost * (float)nPrimitives;
        float totalSA = nodeBounds.surfaceArea();
        float invTotalSA = 1.f / totalSA;
        Vector3f d = nodeBounds.getExtents();

        size_t axis = nodeBounds.maximumExtent();
        int retries = 0;

        retrySplit:
        for (int i = 0; i < nPrimitives; ++i)
        {
            int pn = primNums[i];
            const BBox& bbox = allPrimBounds[pn];
			assert(bbox.pMin[axis] <= bbox.pMax[axis]);
            edges[axis][2*i  ] = BoundEdge(bbox.pMin[axis], pn, true);
            edges[axis][2*i+1] = BoundEdge(bbox.pMax[axis], pn, false);
        }
        std::sort(&edges[axis][0], &edges[axis][2*nPrimitives]);

        int nBelow = 0, nAbove = nPrimitives;
        for (int i = 0; i < 2*nPrimitives; ++i)
        {
            if (edges[axis][i].type == BoundEdge::END)
                --nAbove;
            float edget = edges[axis][i].t;
            if (edget > nodeBounds.pMin[axis] && edget < nodeBounds.pMax[axis])
            {
                size_t otherAxis0 = (axis + 1) % 3;
                size_t otherAxis1 = (axis + 2) % 3;
                float belowSA = 2 * (d[otherAxis0] * d[otherAxis1] +
                                     (edget - nodeBounds.pMin[axis]) *
                                     (d[otherAxis0] + d[otherAxis1]));
                float aboveSA = 2 * (d[otherAxis0] * d[otherAxis1] +
                                     (nodeBounds.pMax[axis] - edget) *
                                     (d[otherAxis0] + d[otherAxis1]));
                float pBelow = belowSA * invTotalSA;
                float pAbove = aboveSA * invTotalSA;
                float eb = (nAbove == 0 || nBelow == 0) ? m_emptyBonus : 0.f;
                float cost = m_traversalCost + m_isectCost * (1.f - eb) * (pBelow*nBelow + pAbove*nAbove);

                if (cost < bestCost)
                {
                    bestCost = cost;
                    bestAxis = axis;
                    bestOffset = i;
                }
            }

            if (edges[axis][i].type == BoundEdge::START)
                ++nBelow;
        }
        assert (nBelow == nPrimitives && nAbove == 0);

        if (bestAxis == -1 && retries < 2)
        {
            ++retries;
            axis = (axis + 1) % 3;
            goto retrySplit;
        }

        if (bestCost > oldCost)
            ++badRefines;
        if ((bestCost > 4.f * oldCost && nPrimitives < 16) ||
            bestAxis == -1 || badRefines == 3)
        {
            m_nodes[nodeNum].initLeaf(primNums, nPrimitives);
            return;
        }

        int n0 = 0, n1 = 0;
        for (int i = 0; i < bestOffset; ++i)
            if (edges[bestAxis][i].type == BoundEdge::START)
                prims0[n0++] = edges[bestAxis][i].primNum;
        for (int i = bestOffset+1; i < 2*nPrimitives; ++i)
            if (edges[bestAxis][i].type == BoundEdge::END)
                prims1[n1++] = edges[bestAxis][i].primNum;
        float tsplit = edges[bestAxis][bestOffset].t;
        BBox bounds0 = nodeBounds, bounds1 = nodeBounds;
        bounds0.pMax[bestAxis] = bounds1.pMin[bestAxis] = tsplit;
        this->buildTree(nodeNum+1,  bounds0, allPrimBounds, prims0, n0, depth-1, edges,
                        prims0, prims1+nPrimitives, badRefines);
        size_t aboveChild = m_nextFreeNode;
		m_nodes[nodeNum].initInterior(bestAxis, aboveChild, tsplit);
        this->buildTree(aboveChild, bounds1, allPrimBounds, prims1, n1, depth-1, edges,
                        prims0, prims1+nPrimitives, badRefines);
    }

    virtual bool rayIntersect(const TRay& ray)
    {
        float tmin, tmax;
        if (!m_bounds.rayIntersect(ray, tmin, tmax))
            return false;

        Vector3f invDir = ray.dRcp;
        KdToDo todo[MAX_TODO];
        int todoPos = 0;
        bool hit = false;
        const KdAccelNode* node = &m_nodes[0];
        while (node != NULL)
        {
            if (!node->IsLeaf())
            {
                int axis = node->SplitAxis();
                float tplane = (node->SplitPos() - ray.o[axis]) * invDir[axis];

                const KdAccelNode *firstChild, *secondChild;
                bool belowFirst = (ray.o[axis] < node->SplitPos()) ||
                        (ray.o[axis] == node->SplitPos() && ray.d[axis] <= 0);
                if (belowFirst)
                {
                    firstChild = node + 1;
                    secondChild = &m_nodes[node->AboveChild()];
                }
                else
                {
                    firstChild = &m_nodes[node->AboveChild()];
                    secondChild = node + 1;
                }

                if (tplane > tmax || tplane <= 0)
                    node = firstChild;
                else if (tplane < tmin)
                    node = secondChild;
                else
                {
                    todo[todoPos].node = secondChild;
                    todo[todoPos].tmin = tplane;
                    todo[todoPos].tmax = tmax;
                    ++todoPos;
                    node = firstChild;
                    tmax = tplane;
                }
            }
            else
            {
                size_t nPrimitives = node->nPrimitives();
                if (nPrimitives == 1)
                {
                    ShapePtr prim = m_primitives[node->onePrimitive];
                    if (prim->rayIntersect(ray))
                        return true;
                }
                else
                {
                    size_t* prims = node->primitives;
                    for (size_t i = 0; i < nPrimitives; ++i)
                    {
                        ShapePtr prim = m_primitives[prims[i]];
                        if (prim->rayIntersect(ray))
                            return true;
                    }
                }

                if (todoPos > 0)
                {
                    --todoPos;
                    node = todo[todoPos].node;
                    tmin = todo[todoPos].tmin;
                    tmax = todo[todoPos].tmax;
                }
                else
                     break;
            }
        }
        return hit;
    }

    virtual bool rayIntersect(const TRay& ray, Intersection& its)
    {
        float tmin, tmax;
        if (!m_bounds.rayIntersect(ray, tmin, tmax))
            return false;

        //Vector3f invDir = ray.dRcp;
		Vector3f invDir(1.f/ray.d.x, 1.f/ray.d.y, 1.f/ray.d.z);
        KdToDo todo[MAX_TODO];
        int todoPos = 0;
        bool hit = false;
        const KdAccelNode* node = &m_nodes[0];
        while (node != NULL)
        {
            if (ray.maxt < tmin)
                break;

			assert (node != NULL);
            if (!node->IsLeaf())
            {
                int axis = node->SplitAxis();
                float tplane = (node->SplitPos() - ray.o[axis]) * invDir[axis];

                const KdAccelNode *firstChild, *secondChild;
                bool belowFirst = (ray.o[axis] < node->SplitPos()) ||
                        (ray.o[axis] == node->SplitPos() && ray.d[axis] <= 0);
                if (belowFirst)
                {
                    firstChild = node + 1;
                    secondChild = &m_nodes[node->AboveChild()];
                }
                else
                {
                    firstChild = &m_nodes[node->AboveChild()];
                    secondChild = node + 1;
                }

                if (tplane > tmax || tplane <= 0)
                    node = firstChild;
                else if (tplane < tmin)
                    node = secondChild;
                else
                {
                    todo[todoPos].node = secondChild;
                    todo[todoPos].tmin = tplane;
                    todo[todoPos].tmax = tmax;
                    ++todoPos;
                    node = firstChild;
                    tmax = tplane;
                }
            }
            else
            {
                size_t nPrimitives = node->nPrimitives();
                if (nPrimitives == 1)
                {
                    ShapePtr prim = m_primitives[node->onePrimitive];
                    if (prim->rayIntersect(ray, its))
                    {
						ray.maxt = its.t;
                        hit = true;
                    }
                }
                else
                {
                    size_t* prims = node->primitives;
                    for (size_t i = 0; i < nPrimitives; ++i)
                    {
                        ShapePtr prim = m_primitives[prims[i]];
                        if (prim->rayIntersect(ray, its))
                        {
							assert(ray.maxt >= its.t);
                            ray.maxt = its.t;
                            hit = true;
                        }
                    }
                }

                if (todoPos > 0)
                {
                    --todoPos;
                    node = todo[todoPos].node;
                    tmin = todo[todoPos].tmin;
                    tmax = todo[todoPos].tmax;
                }
                else
                     break;
            }
        }
        return hit;
    }

protected:
    int m_isectCost, m_traversalCost, m_maxPrims, m_maxDepth;
    float m_emptyBonus;
    std::vector<ShapePtr> m_primitives;
    KdAccelNode* m_nodes;
    int m_nAllocedNodes, m_nextFreeNode;
    BBox m_bounds;
};

WISP_REGISTER_CLASS(KdTreeAccel, "kdtree")

WISP_NAMESPACE_END
