//
// Created by shiyin on 2023/12/18.
//
#include "../core/ray.h"

Ray::Ray(const glm::vec3 &ori, const glm::vec3 &dir, const float _t) : origin(ori), direction(dir), t(_t) {
    tMin = 0.0f;
    tMax = std::numeric_limits<float>::max();
    invDir = glm::vec3(1.f/direction.x,1.f/direction.y,1.f/direction.z);
}
