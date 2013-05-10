#include "shape.h"
#include "light.h"

WISP_NAMESPACE_BEGIN

const BSDF* Intersection::getBSDF() const
{
    return shape->getBSDF();
}

Color3f Intersection::Le(const Vector3f& dir) const
{
    return shape->getLight()->Le(ShapeSamplingRecord(*this), dir);
}

void Shape::refine(std::vector<ShapePtr>&) const
{
    throw WispException("Unimplemented Shape::refine() method called");
}

float Shape::sampleArea(ShapeSamplingRecord&, const Point2f&) const
{
    throw WispException("Unimplemented Shape::sampleArea() method called");
}

float Shape::sampleSolidAngle(ShapeSamplingRecord &sRec, const Point3f &from, const Point2f sample) const
{
    float pdfArea = this->sampleArea(sRec, sample);
    Vector3f lumToPoint = from - sRec.p;
    float distSqr = glm::dot(lumToPoint, lumToPoint);
    float dp = glm::dot(lumToPoint, sRec.n);
    if (dp > 0)
        return pdfArea * distSqr * std::sqrt(distSqr) / dp;
    else
        return 0.0f;
}

float Shape::pdfArea(const ShapeSamplingRecord &) const
{
    throw WispException("Unimplemented Shape::pdfArea() method called");
}

float Shape::pdfSolidAngle(const ShapeSamplingRecord &sRec, const Point3f &from) const
{
    //float pdfArea = this->sampleArea(sRec, sample);
    Vector3f lumToPoint = from - sRec.p;
    float distSqr = glm::dot(lumToPoint, lumToPoint);
    float dp = glm::dot(lumToPoint, sRec.n);
    if (dp > 0)
        return this->pdfArea(sRec) * distSqr * std::sqrt(distSqr) / dp;
    else
        return 0.0f;
}

/*void Shape::samplePosition(const Point2f& sample, Point3f& p, Normal3f& n) const
{
    throw WispException("Unimplemented Shape::samplePosition() method called");
}*/

bool Shape::rayIntersect(const TRay& ray)
{
    throw WispException("Unimplemented Shape::rayIntersect() method called");
}

bool Shape::rayIntersect(const TRay& ray, Intersection& its)
{
    throw WispException("Unimplemented Shape::rayIntersect() method called");
}

void Shape::fullyRefine(std::vector<ShapePtr> &refined) const
{
    std::vector<ShapePtr> todo;
    todo.push_back(ShapePtr(this));
    while (todo.size())
    {
        ShapePtr prim = todo.back();
        todo.pop_back();
        if (prim->canIntersect())
            refined.push_back(prim);
        else
            prim->refine(todo);
    }
}

WISP_NAMESPACE_END
