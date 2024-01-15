//
// Created by shiyin on 2023/12/13.
//
#pragma once

#ifndef SIMPLERENDERER_USERCONTROL_H
#define SIMPLERENDERER_USERCONTROL_H

struct UserControl{
    double lastX;
    double lastY;
    double currentX;
    double currentY;
    bool mouseLeftButtonDown;
    bool mouseMiddleButtonDown;
    bool mouseScroll;
    bool mouseCastRayLeftButton;

    UserControl(){
        lastX = 0.0f;
        lastY = 0.0f;
        currentX = 0.0f;
        currentY = 0.0f;
        mouseLeftButtonDown = false;
        mouseMiddleButtonDown = false;
        mouseScroll = false;
        mouseCastRayLeftButton = false;
    }
};

#endif //SIMPLERENDERER_USERCONTROL_H
