//
// Created by shiyin on 2023/12/19.
//
#pragma once

#ifndef SIMPLERENDERER_TRIANGLE_H
#define SIMPLERENDERER_TRIANGLE_H

#include "glm/glm.hpp"
#include "ray.h"
#include "bbox.h"
#include "intersection.h"

class Triangle{
public:

    Triangle() = default;
    Triangle(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, float _area);

    Intersection intersect(const Ray& ray);

    glm::vec3 v0,v1,v2;
    float area;
    glm::vec3 normal;
    BoundingBox3 boundingBox;
};

#endif //SIMPLERENDERER_TRIANGLE_H
