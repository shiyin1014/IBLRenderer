//
// Created by shiyin on 2023/12/8.
//

#pragma once

#ifndef SIMPLERENDERER_BBOX_H
#define SIMPLERENDERER_BBOX_H

#include <vector>
#include "glm/glm.hpp"
#include "ray.h"
#include "ostream"

class BoundingBox3{
public:

    glm::vec3 pMin, pMax;

    BoundingBox3();
    explicit BoundingBox3(const glm::vec3& p);
    BoundingBox3(const glm::vec3& p1, const glm::vec3& p2);

    glm::vec3 Diagonal() const;

    int maxExtent()const;

    double SurfaceArea() const;

    glm::vec3 Centroid() const;

    std::vector<glm::vec3> createCubeVertices() const;

    bool operator!=(const BoundingBox3& other) const{
        return !(this->pMin == other.pMin && this->pMax == other.pMax);
    }

    bool operator==(const BoundingBox3& other) const{
        return this->pMin == other.pMin && this->pMax == other.pMax;
    }

    bool intersect(const Ray& ray);

    friend std::ostream& operator<<(std::ostream& os, const BoundingBox3& box);

};

inline BoundingBox3 Union(const BoundingBox3& b1, const BoundingBox3& b2){
    BoundingBox3 res;
    res.pMin = glm::vec3 (std::min(b1.pMin.x,b2.pMin.x),
                          std::min(b1.pMin.y,b2.pMin.y),
                          std::min(b1.pMin.z,b2.pMin.z));

    res.pMax = glm::vec3 (std::max(b1.pMax.x,b2.pMax.x),
                          std::max(b1.pMax.y,b2.pMax.y),
                          std::max(b1.pMax.z,b2.pMax.z));
    return res;
}

inline BoundingBox3 Union(const BoundingBox3& b, const glm::vec3& p){
    BoundingBox3 res;
    res.pMin = glm::vec3 (std::min(b.pMin.x,p.x),
                          std::min(b.pMin.y,p.y),
                          std::min(b.pMin.z,p.z));

    res.pMax = glm::vec3 (std::max(b.pMax.x,p.x),
                          std::max(b.pMax.y,p.y),
                          std::max(b.pMax.z,p.z));
    return res;
}

inline BoundingBox3 Union(const glm::vec3& p1, const glm::vec3& p2){
    BoundingBox3 res;
    res.pMin = glm::vec3 (std::min(p1.x,p2.x),
                          std::min(p1.y,p2.y),
                          std::min(p1.z,p2.z));

    res.pMax = glm::vec3 (std::max(p1.x,p2.x),
                          std::max(p1.y,p2.y),
                          std::max(p1.z,p2.z));
    return res;
}

#endif //SIMPLERENDERER_BBOX_H
