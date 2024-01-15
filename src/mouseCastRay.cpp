//
// Created by shiyin on 2023/12/18.
//
#include "../core/mouseCastRay.h"

MouseCastRay::MouseCastRay(const std::shared_ptr<Camera> &cameraPtr,
                           unsigned int start_x, unsigned int start_y,
                           unsigned int width, unsigned int height) :
        camera(cameraPtr), screenWidth(width), screenHeight(height), startX(start_x), startY(start_y){
}

Ray MouseCastRay::getRay(const glm::vec3& worldPos) {
    glm::vec3 direction = glm::normalize(worldPos - camera->getEye());
    return {camera->getEye(),direction};
}

glm::vec3 MouseCastRay::getWorldPosFromScreenCoords(const glm::vec3 &screenPos) const {
    float x = (2.0f * (screenPos.x - (float )startX)) / (float )screenWidth - 1.0f;
    float y = 1.0f - (2.0f * (screenPos.y - float (startY))) / (float )screenHeight;
    float z = screenPos.z * 2.0f - 1.0f;

    if (z < 1.0f && z > 0.0f){
        float w = 2.0f * camera->getNear() * camera->getFar() /
                (camera->getNear() + camera->getFar() + (camera->getNear() - camera->getFar()) * z);
        glm::vec4 worldPosition = camera->getInverseViewMatrix() * camera->getInverseProjectionMatrix() *
                w * glm::vec4 (x,y,z,1);
        return {worldPosition.x,worldPosition.y,worldPosition.z};
    }else{
//        Logger::Log<MouseCastRay>(__FUNCTION__ ,
//                                  "z = " + std::to_string(z) + " : The ray dit not hit any model in the scene.");
    }
    return {};
}
