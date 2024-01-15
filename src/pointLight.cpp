//
// Created by shiyin on 2023/12/8.
//

#include "../core/pointLight.h"

PointLight::PointLight(const glm::vec3 &position, glm::vec3 color) : lightColor(color), lightPosition(position){
}

glm::vec3 PointLight::getLightPosition() const {
    return lightPosition;
}

glm::vec3 PointLight::getLightColor() const {
    return lightColor;
}

void PointLight::setLightPosition(const glm::vec3& position) {
    this->lightPosition = position;
}

void PointLight::setLightColor(const glm::vec3 &color) {
    this->lightColor = color;
}
