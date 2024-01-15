//
// Created by shiyin on 2023/12/18.
//

#pragma once

#ifndef SIMPLERENDERER_MOUSECASTRAY_H
#define SIMPLERENDERER_MOUSECASTRAY_H

#include "memory"
#include <iostream>
#include "camera.h"
#include "logger.h"
#include "ray.h"

class MouseCastRay{
public:

    MouseCastRay(const std::shared_ptr<Camera>& cameraPtr,
                 unsigned int start_x, unsigned int start_y,
                 unsigned int width, unsigned int height);
    Ray getRay(const glm::vec3& worldPos);

    glm::vec3 getWorldPosFromScreenCoords(const glm::vec3& screenPos) const;

private:
    std::shared_ptr<Camera> camera;
    unsigned int startX;
    unsigned int startY;
    unsigned int screenWidth;
    unsigned int screenHeight;

};

#endif //SIMPLERENDERER_MOUSECASTRAY_H
