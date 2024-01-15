//
// Created by shiyin on 2023/12/8.
//
#pragma once

#ifndef SIMPLERENDERER_RENDERER_H
#define SIMPLERENDERER_RENDERER_H

#include <vector>
#include "pointLight.h"
#include "camera.h"
#include "model.h"
#include "scene.h"
#include "frameBuffer.h"
#include "ibl.h"
#include "mouseCastRay.h"
#include "memory"

// shadow (point light)
enum struct ShadowMethod{
    None = 0,
    ShadowMapping = 1,
    PCF = 2,
    PCSS = 3
};

enum struct POST_PROCESS{
    NONE = 0,
    GAUSSIAN_BLUR = 1,
    SHARPEN = 2
};

struct GaussianParameter{
    int mSize;
    float mSigma;
};

class Renderer{
public:

    explicit Renderer(const std::shared_ptr<Camera>& cameraPtr);
    ~Renderer();
    void addPointLight(const std::shared_ptr<PointLight>& light);
    void setLightsUniforms(const Shader& shader);
    void setLightsRotation();
    void setScene(const std::shared_ptr<Scene>& scenePtr);
    void initShaders();
    void drawSkyBox();
    void drawIblBrdfLutTexture();
    void renderToScreenSettings();
    void renderWithPostProcess();
    void finalRenderToScreen();
    void drawBoundingBox();
    void drawPointLights();
    void drawDepthMapOnly();
    void debugDepthMapToScreen();
    Model* selectModel(const Ray& castRay);
    void drawCastRay();
    void addCastedRay(const glm::vec3& endPoint);
    void drawOutlineOfSelectedModel();
    void blurOutlineColorBuffer();
    void finalColorToBuffer(bool bloom);
    void setViewMatrix(const Shader& shader);
    void setCameraPosUniform();
    void setProjectionMatrix(const Shader& shader);
    void addHdrScene(const std::string& path);
    void setIblUniforms();
    void bindIblTextures();
    void setSceneUniforms();
    void setShadowUniforms();
    void clear() const;
    void render();
    glm::vec3 getBackgroundColor()const;
    void setBackgroundColor(glm::vec3 color);
    std::shared_ptr<Camera> getCamera()const;
    std::vector<std::shared_ptr<PointLight>> getPointLights()const;
    std::shared_ptr<Scene> getScene()const;
    bool getEnableHdr()const;
    bool getEnableGamma()const;
    void setHdr();
    void setGamma();
    bool getEnableMsaa()const;
    void setMsaa();
    unsigned int getIblCubeMapId()const;
    unsigned int getIblPrefilterMapId()const;
    unsigned int getIblBrdfLutTexture2D()const;
    std::vector<unsigned int> getTextures2DOfEnvCubeMap()const;
    std::vector<unsigned int> getTextures2DOfIrradianceCubeMap() const;
    std::vector<std::vector<unsigned int>> getTextures2DOfPrefilteredCubeMap() const;
    void createFrameBuffers();
    void saveCurrentFrameBuffer();
    void setShadowMethod(const ShadowMethod& method);
    void setLightSpaceMatrix();
    ShadowMethod getShadowMethod() const;
    void setIBLIndex(int index);
    int getIBLIndex() const;
    void saveColorBufferToFile() const;

private:

    // IBL
    std::vector<std::unique_ptr<IBL>> ibl;
    int iblIndex;
    // default scene shader
    Shader sceneShader{};
    // skybox shader
    Shader skyBoxShader{};
    // framebuffer to render scene to
    std::unique_ptr<FrameBuffer> sceneMSAAFBO;
    // box
    Shader boundingBoxShader{};
    // quadShader
    Shader quadShader{};
    // line
    Shader castRayShader{};
    // outline
    Shader outlineShader{};
    // blur
    Shader blurShader{};
    // final
    Shader finalShader{};
    // point light
    Shader lightShader{};
    // depth
    Shader depthShader{};
    // depth debug
    Shader depthDebugShader{};
    // post process
    Shader postProcessShader{};
    // background color
    glm::vec3 backgroundColor;

    // camera and scene
    std::shared_ptr<Camera> camera;
    std::shared_ptr<Scene> scene;

    // shadow
    ShadowMethod shadowMethod;
    const int shadowWidth = 1024;
    const int shadowHeight = 1024;
    glm::vec3 defaultLightPos{};
    glm::mat4 lightProjection{};
    glm::mat4 lightView{};
    glm::mat4 lightSpaceMatrix{};

    bool enableMsaa{};
    bool hdrToneMapping{};
    bool gammaCorrection{};

public:

    // depthMap
    std::unique_ptr<FrameBuffer> depthMapFBO;

    // render the simpleModel such as quad, box and sphere
    SimpleModel simpleModel;

    // used to restore the resolved buffer from the multi-sampled FBO
    std::unique_ptr<FrameBuffer> blitSceneFBO;

    // used to blur the outlineFBO
    unsigned int pingPongFBO[2]{};
    unsigned int pingPongColorBuffers[2]{};

    // IBL
    bool drawEnvCubeMap;
    bool drawIrradianceCubeMap;
    bool drawPrefilterCubeMap;
    int prefilterCubeMapLevel;
    bool drawBrdfLutTexture2D;

    // boundingBox
    bool drawSceneBox;
    glm::vec3 boxLineColor;

    // selectModel
    bool enableSelect;
    Model* hitModel;
    bool showCastedRays;
    glm::vec3 castedRayColor;

    // pointLight
    std::vector<std::shared_ptr<PointLight>> pointLights;
    // enableRotation of the point light
    bool enableRotationPointLight;

    // store final color
    std::unique_ptr<FrameBuffer> sceneAndOutLineColorBufferFBO;

    // screenshot
    std::unique_ptr<FrameBuffer> screenShotColorFBO;

    // restore the last frame data
    std::unique_ptr<FrameBuffer> lastFrameBuffer;
    bool firstFrame;

    // post processing
    POST_PROCESS postProcess;
    // gaussian
    GaussianParameter gaussianParameter;
    // sharpen
    int sharpenKernelParameter;

};

#endif //SIMPLERENDERER_RENDERER_H
