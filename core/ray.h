//
// Created by shiyin on 2023/12/18.
//

#pragma once

#ifndef SIMPLERENDERER_RAY_H
#define SIMPLERENDERER_RAY_H

#include "glm/glm.hpp"

class Ray{
public:
    glm::vec3 origin;
    glm::vec3 direction;
    glm::vec3 invDir{};

    float t;
    float tMin, tMax;

    Ray(const glm::vec3& ori, const glm::vec3& dir,float _t = 0.0f);

    glm::vec3 operator()(float _t)const{
        return origin + direction * _t;
    }

};

#endif //SIMPLERENDERER_RAY_H
