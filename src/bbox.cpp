//
// Created by shiyin on 2023/12/8.
//

#include <iostream>
#include "../core/bbox.h"

BoundingBox3::BoundingBox3() : pMax(std::numeric_limits<double>::lowest(),
                                    std::numeric_limits<double>::lowest(),
                                    std::numeric_limits<double>::lowest()),
                               pMin(std::numeric_limits<double>::max(),
                                    std::numeric_limits<double>::max(),
                                    std::numeric_limits<double>::max()) {}


BoundingBox3::BoundingBox3(const glm::vec3 &p) : pMin(p), pMax(p) {}

BoundingBox3::BoundingBox3(const glm::vec3 &p1, const glm::vec3 &p2) :
        pMin(std::min(p1.x, p2.x), std::min(p1.y, p2.y), std::min(p1.z, p2.z)),
        pMax(std::max(p1.x, p2.x), std::max(p1.y, p2.y), std::max(p1.z, p2.z)) {}

glm::vec3 BoundingBox3::Diagonal() const {
    return pMax - pMin;
}

int BoundingBox3::maxExtent() const {
    glm::vec3 d = Diagonal();
    if (d.x > d.y && d.x > d.z) {
        return 0;
    } else if (d.y > d.z) {
        return 1;
    } else {
        return 2;
    }
}

double BoundingBox3::SurfaceArea() const {
    glm::vec3 d = Diagonal();
    return 2.0 * (d.x * d.y + d.x * d.z + d.y * d.z);
}

glm::vec3 BoundingBox3::Centroid() const {
    return pMin * 0.5f + pMax * 0.5f;
}

std::vector<glm::vec3> BoundingBox3::createCubeVertices() const {
    std::vector<glm::vec3> vertices;

    // front face
    vertices.emplace_back(pMin.x, pMin.y, pMax.z);
    vertices.emplace_back(pMax.x, pMin.y, pMax.z);

    vertices.emplace_back(pMax.x, pMin.y, pMax.z);
    vertices.emplace_back(pMax.x, pMax.y, pMax.z);

    vertices.emplace_back(pMax.x, pMax.y, pMax.z);
    vertices.emplace_back(pMin.x, pMax.y, pMax.z);

    vertices.emplace_back(pMin.x, pMax.y, pMax.z);
    vertices.emplace_back(pMin.x, pMin.y, pMax.z);

    // back face
    vertices.emplace_back(pMin.x, pMin.y, pMin.z);
    vertices.emplace_back(pMax.x, pMin.y, pMin.z);

    vertices.emplace_back(pMax.x, pMin.y, pMin.z);
    vertices.emplace_back(pMax.x, pMax.y, pMin.z);

    vertices.emplace_back(pMax.x, pMax.y, pMin.z);
    vertices.emplace_back(pMin.x, pMax.y, pMin.z);

    vertices.emplace_back(pMin.x, pMax.y, pMin.z);
    vertices.emplace_back(pMin.x, pMin.y, pMin.z);

    // middle
    vertices.emplace_back(pMin.x, pMin.y, pMax.z);
    vertices.emplace_back(pMin.x, pMin.y, pMin.z);

    vertices.emplace_back(pMax.x, pMin.y, pMax.z);
    vertices.emplace_back(pMax.x, pMin.y, pMin.z);

    vertices.emplace_back(pMax.x, pMax.y, pMax.z);
    vertices.emplace_back(pMax.x, pMax.y, pMin.z);

    vertices.emplace_back(pMin.x, pMax.y, pMax.z);
    vertices.emplace_back(pMin.x, pMax.y, pMin.z);

    return vertices;
}

bool BoundingBox3::intersect(const Ray &ray) {

    glm::vec3 t1 = (pMin - ray.origin) * ray.invDir;
    glm::vec3 t2 = (pMax - ray.origin) * ray.invDir;

    glm::vec3 tMin = glm::vec3(std::min(t1.x, t2.x), std::min(t1.y, t2.y), std::min(t1.z, t2.z));
    glm::vec3 tMax = glm::vec3(std::max(t1.x, t2.x), std::max(t1.y, t2.y), std::max(t1.z, t2.z));

    float tEnter = std::max(std::max(tMin.x, tMin.y), tMin.z);
    float tExit = std::min(std::min(tMax.x, tMax.y), tMax.z);

    if (tEnter < tExit && tExit > 0) {
        return true;
    }

    return false;
}

std::ostream &operator<<(std::ostream &os, const BoundingBox3 &box) {
    os << "BoundingBox3 (pMin: " << box.pMin.x << ", " << box.pMin.y << ", " << box.pMin.z
       << ", pMax: " << box.pMax.x << ", " << box.pMax.y << ", " << box.pMax.z << ")";
    return os;
}

