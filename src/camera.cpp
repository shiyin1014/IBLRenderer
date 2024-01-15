//
// Created by shiyin on 2023/12/8.
//
#include "../core/Camera.h"

Camera::Camera(const glm::vec3 &center, const glm::vec3 &upDir, float radius, float azimuthAngle,
               float polarAngle, int screenWidth, int screenHeight) : mCenter(center), mUpVector(upDir),
                                   mRadius(radius), mAzimuthAngle(azimuthAngle), mPolarAngle(polarAngle),
                                   mMinRadius(4.0f), mMaxRadius(50.0f),
                                   mMinAzimuthAngle(0.0f), mMaxAzimuthAngle(359.999f),
                                   mMinPolarAngle(-89.0f), mMaxPolarAngle(89.f),
                                   defaultCenter(glm::vec3(0,0,0)),defaultUpVector (glm::vec3(0,1,0)),
                                   width(screenWidth), height(screenHeight), near(0.1f), far(100.0f),
                                   projection(glm::perspective(glm::radians(45.0f),
                                                               static_cast<float >((float )width) / static_cast<float>(height),
                                                               near,far)){
    defaultRadius = 10.0f;
    defaultPolarAngle = glm::radians(0.0f);
    defaultAzimuthAngle = 0.0f;
}

void Camera::rotateAzimuth(const float radians) {
    mAzimuthAngle += radians;
    //将方位角限定在0-2Pi之内
    const auto fullCircle = 2.0f * glm::pi<float>();
    mAzimuthAngle = fmodf(mAzimuthAngle, (float )fullCircle);
    if(mAzimuthAngle < 0.0f){
        mAzimuthAngle = (float )fullCircle + mAzimuthAngle;
    }
}

void Camera::rotatePolar(const float radians) {
    mPolarAngle+=radians;
    //将极角限定在0-Pi/2内
    const auto polarCap = glm::pi<float>() / 2.0f - 0.001f;
    if (mPolarAngle > polarCap){
        mPolarAngle = (float )polarCap;
    }
    if (mPolarAngle < mMinPolarAngle){
        mPolarAngle = mMinPolarAngle;
    }
}

void Camera::zoom(const float by) {
    mRadius -= by;
    if (mRadius <= mMinRadius){
        mRadius = mMinRadius;
    }
    if (mRadius >= mMaxRadius){
        mRadius = mMaxRadius;
    }
}

void Camera::moveHorizontal(const float distance) {
    auto position = getEye();//获取当前坐标位置
    glm::vec3 view_vector = getNormalizedViewVector();
    glm::vec3 strafe_vector = glm::normalize(glm::cross(view_vector, mUpVector));
    mCenter += strafe_vector * distance;
}

void Camera::moveVertical(const float distance) {
    mCenter += mUpVector * distance;
}

glm::mat4 Camera::getViewMatrix() const {
    const auto eye = getEye();
    return glm::lookAt(eye, eye + getNormalizedViewVector(), mUpVector);
}

glm::vec3 Camera::getEye() const {
    const auto sineAzimuth = std::sin(mAzimuthAngle);
    const auto cosineAzimuth = std::cos(mAzimuthAngle);
    const auto sinePolar = std::sin(mPolarAngle);
    const auto cosinePolar = std::cos(mPolarAngle);
    //将球体坐标转变为三维坐标
    const auto x = mCenter.x + mRadius * cosinePolar * sineAzimuth;
    const auto y = mCenter.y + mRadius * sinePolar;
    const auto z = mCenter.z + mRadius * cosinePolar * cosineAzimuth;
    return {x,y,z};
}

glm::vec3 Camera::getViewPoint() const {
    return mCenter;
}

glm::vec3 Camera::getUpDir() const {
    return mUpVector;
}

glm::vec3 Camera::getNormalizedViewVector() const  {
    return glm::normalize(getViewPoint() - getEye());
}

float Camera::getAzimuthAngle() const {
    return mAzimuthAngle;
}

float Camera::getPolarAngle() const {
    return mPolarAngle;
}

float Camera::getRadius() const {
    return mRadius;
}

void Camera::restoreDefaultSetting() {
    mCenter = defaultCenter;
    mUpVector = defaultUpVector;
    mRadius = defaultRadius;
    mPolarAngle = defaultPolarAngle;
    mAzimuthAngle = defaultAzimuthAngle;
}

void Camera::setRadius(float radius) {
    this->mRadius = radius;
}

void Camera::setPolar(float polarAngle) {
    this->mPolarAngle = polarAngle;
}

void Camera::setAzimuth(float azimuthAngle) {
    this->mAzimuthAngle = azimuthAngle;
}

float Camera::getMinRadius() const {
    return mMinRadius;
}

float Camera::getMaxRadius() const {
    return mMaxRadius;
}

float Camera::getMinPolarAngle() const {
    return mMinPolarAngle;
}

float Camera::getMaxPolarAngle() const {
    return mMaxPolarAngle;
}

float Camera::getMinAzimuthAngle() const {
    return mMinAzimuthAngle;
}

float Camera::getMaxAzimuthAngle() const {
    return mMaxAzimuthAngle;
}

glm::mat4 Camera::getProjectionMatrix() {
    return projection;
}

glm::mat4 Camera::getInverseProjectionMatrix() {
    return glm::inverse(projection);
}

glm::mat4 Camera::getInverseViewMatrix() {
    return glm::inverse(getViewMatrix());
}

float Camera::getNear() const {
    return near;
}

float Camera::getFar() const {
    return far;
}

int Camera::getWidth() const {
    return width;
}

int Camera::getHeight() const {
    return height;
}

glm::vec3 Camera::getCenter() const {
    return mCenter;
}

