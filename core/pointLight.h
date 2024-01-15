//
// Created by shiyin on 2023/12/8.
//

#pragma once

#ifndef SIMPLERENDERER_POINTLIGHT_H
#define SIMPLERENDERER_POINTLIGHT_H

#include "glm/glm.hpp"

class PointLight{
private:
    glm::vec3 lightPosition;
    glm::vec3 lightColor;

public:
    PointLight(const glm::vec3 &position, glm::vec3 color = glm::vec3(255.0f,255.0f,255.0f));
    ~PointLight() = default;
    glm::vec3 getLightPosition()const;
    glm::vec3 getLightColor()const;
    void setLightPosition(const glm::vec3& position);
    void setLightColor(const glm::vec3 &color);

};


#endif //SIMPLERENDERER_POINTLIGHT_H
