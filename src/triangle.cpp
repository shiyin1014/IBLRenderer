//
// Created by shiyin on 2023/12/19.
//

#include "../core/triangle.h"

Triangle::Triangle(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, float _area): v0(p0), v1(p1), v2(p2), area(_area) {
    // calculate the bbox
    boundingBox = Union(Union(v0,v1),v2);
    // normal
    glm::vec3 edge1 = v1 - v0;
    glm::vec3 edge2 = v2 - v0;
    normal = glm::normalize(glm::cross(edge1,edge2));
}

Intersection Triangle::intersect(const Ray& ray) {
    Intersection intersection;
    if (glm::dot(ray.direction, normal) >= 0)
        return intersection;

    // MT Algorithm
    double u, v, t = 0;
    glm::vec3 edge1 = v1 - v0;  // e1
    glm::vec3 edge2 = v2 - v0;  // e2
    glm::vec3 pvec = glm::cross(ray.direction,edge2);   // s1
    double det = glm::dot(edge1, pvec);  // 1./dot(e1,s1)
    if (std::fabs(det) < 0.00001) return intersection;

    double invDet = 1.f / det;
    glm::vec3 tvec = ray.origin - v0;   // s
    u = glm::dot(tvec,pvec) * invDet;    // dot(s,s1)
    if (u < 0 || u > 1) return intersection;

    glm::vec3 qvec = glm::cross(tvec, edge1);
    v = glm::dot(ray.direction,qvec) * invDet;
    if (v < 0 || u + v > 1) return intersection;

    t = glm::dot(edge2, qvec) * invDet;
    if (t < 0) return intersection;
//    u *= invDet;
//    v *= invDet;

    intersection.happened = true;
    intersection.distance = t;

    return intersection;
}

