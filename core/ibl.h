//
// Created by shiyin on 2023/12/10.
//

#pragma once

#ifndef SIMPLERENDERER_IBL_H
#define SIMPLERENDERER_IBL_H

#include <string>
#include "shader.h"
#include "textureTools.h"
#include "frameBuffer.h"
#include "simplemodel.h"
#include "GLFW/glfw3.h"

class IBL{
public:

    IBL() = default;

    explicit IBL(const std::string &path);
    ~IBL() = default;

    void loadHdrTexture();
    void hdrToCubeMap();
    void calculateIrradianceMap();
    void calculateBrdfLutTexture();
    void calculatePrefilterCubeMap();
    void initShaders();
    void initFbOAndRbo();
    void drawIbl();
    unsigned int getHdrCubeMapId()const;
    unsigned int getIrradianceCubeMapId()const;
    unsigned int getPrefilterCubeMapId()const;
    unsigned int getBrdfLutTextureId()const;
    std::vector<unsigned int> getTextures2DOfEnvCubeMap()const;
    std::vector<unsigned int> getTextures2DOfIrradianceCubeMap() const;
    std::vector<std::vector<unsigned int>> getTextures2DOfPrefilteredCubeMap()const;

private:
    std::string hdrFilePath;
    unsigned int hdrTextureId{};

    unsigned int envCubeMapId{};
    unsigned int irradianceCubeMapId{};
    unsigned int prefilterCubeMapId{};
    unsigned int brdfTextureId{};

    Shader hdrToCubeMapShader{};
    Shader irradianceShader{};
    Shader prefilterShader{};
    Shader brdfLutShader{};

    int cubeMapSizeWidth{};
    int cubeMapSizeHeight{};
    int irradianceSizeWidth{};
    int irradianceSizeHeight{};
    int prefilterCubeSizeWidthMip0{};
    int prefilterCubeSizeHeightMip0{};
    int brdfTextureSizeWidth{};
    int brdfTextureSizeHeight{};

    unsigned int iblFBO{};
    unsigned int iblRBO{};
    int depthBufferSize;
    SimpleModel simpleModel;

    glm::mat4 captureProjection{};
    glm::mat4 captureViews[6]{};

    std::vector<unsigned int> texture2DOfEnvCubeMap;
    std::vector<unsigned int> texture2DOfIrradianceCubeMap;
    std::vector<std::vector<unsigned int>> texture2DOfPrefilterCubeMap;

};

#endif //SIMPLERENDERER_IBL_H
