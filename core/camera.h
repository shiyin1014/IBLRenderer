//
// Created by shiyin on 2023/12/8.
//

#pragma once

#ifndef SIMPLERENDERER_CAMERA_H
#define SIMPLERENDERER_CAMERA_H

#include <cmath>
#include "glm/glm.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/mat4x4.hpp"
#include <glm/gtc/type_ptr.hpp>

class Camera {
public:
    Camera(const glm::vec3 &center, const glm::vec3 &upDir, float radius,
           float azimuthAngle, float polarAngle,
           int screenWidth, int screenHeight);

    void rotateAzimuth(float radians);
    void rotatePolar(float radians);
    void zoom(float by);
    void setRadius(float radius);
    void setPolar(float polarAngle);
    void setAzimuth(float azimuthAngle);
    void moveHorizontal(float distance);
    void moveVertical(float distance);

    glm::mat4 getViewMatrix()const;
    glm::vec3 getEye()const;
    glm::vec3 getViewPoint()const;
    glm::vec3 getUpDir()const;
    glm::vec3 getNormalizedViewVector() const;
    float getAzimuthAngle()const;
    float getPolarAngle()const;
    float getRadius()const;
    void restoreDefaultSetting();
    float getMinRadius()const;
    float getMaxRadius()const;
    float getMinPolarAngle() const;
    float getMaxPolarAngle() const;
    float getMinAzimuthAngle() const;
    float getMaxAzimuthAngle() const;

    glm::mat4 getProjectionMatrix();
    glm::mat4 getInverseProjectionMatrix();
    glm::mat4 getInverseViewMatrix();

    float getNear()const;
    float getFar()const;
    int getWidth() const;
    int getHeight() const;
    glm::vec3  getCenter() const;

private:
    glm::vec3 mCenter;
    glm::vec3 mUpVector;

    glm::vec3 defaultCenter;
    glm::vec3 defaultUpVector;

    float mRadius;
    float mMinRadius;
    float mMaxRadius;
    float defaultRadius;

    // azimth angle and polar angle are used to define the location of camera in sphere coordinate
    float mAzimuthAngle;
    float mMinAzimuthAngle;
    float mMaxAzimuthAngle;
    float defaultAzimuthAngle;

    float mPolarAngle;
    float mMinPolarAngle;
    float mMaxPolarAngle;
    float defaultPolarAngle;

    // projection
    float near,far;
    int width;
    int height;
    glm::mat4 projection;
};


#endif //SIMPLERENDERER_CAMERA_H