//
// Created by shiyin on 2023/12/10.
//
#include "../core/ibl.h"

IBL::IBL(const std::string &path) :
    cubeMapSizeWidth(1024), cubeMapSizeHeight(1024),
    brdfTextureSizeWidth(1024), brdfTextureSizeHeight(1024),
    prefilterCubeSizeWidthMip0(128), prefilterCubeSizeHeightMip0(128),
    irradianceSizeWidth(32),irradianceSizeHeight(32),
    captureProjection(glm::perspective(glm::radians(90.0f),1.0f,0.1f,10.0f)),
    captureViews{glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
                 glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
                 glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
                 glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
                 glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
                 glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))},
    simpleModel(),
    depthBufferSize(1024){
    hdrFilePath = path;
    loadHdrTexture();
    initFbOAndRbo();
    drawIbl();
}

void IBL::loadHdrTexture() {
    hdrTextureId = TextureTools::loadHdrTexture(hdrFilePath);
}

void IBL::hdrToCubeMap() {
    glGenTextures(1,&envCubeMapId);
    glBindTexture(GL_TEXTURE_CUBE_MAP,envCubeMapId);
    for (int i = 0; i < 6; ++i) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                     0,
                     GL_RGB16F,
                     cubeMapSizeWidth,cubeMapSizeHeight,0,GL_RGB,GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // enable pre-filter mipmap sampling (combatting visible dots artifact)
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    hdrToCubeMapShader.use();
    hdrToCubeMapShader.setMat4("projection",captureProjection);
    // transform hdr texture to shader
    hdrToCubeMapShader.setInt("hdrMap",0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D,hdrTextureId);

    glViewport(0,0,cubeMapSizeWidth,cubeMapSizeHeight);
    glBindFramebuffer(GL_FRAMEBUFFER,iblFBO);
    for (int i = 0; i < 6; ++i) {
        hdrToCubeMapShader.setMat4("view",captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,envCubeMapId,0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        simpleModel.renderCube();
    }
    glBindFramebuffer(GL_FRAMEBUFFER,0);

    glBindTexture(GL_TEXTURE_CUBE_MAP,envCubeMapId);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    // resolve each face data to texture_2D
    texture2DOfEnvCubeMap = TextureTools::loadTexture2DFromCubeMap(envCubeMapId,GL_RGB,GL_FLOAT);
}

void IBL::calculateIrradianceMap() {
    glGenTextures(1,&irradianceCubeMapId);
    glBindTexture(GL_TEXTURE_CUBE_MAP,irradianceCubeMapId);
    for (int i = 0; i < 6; ++i) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F,
                     irradianceSizeWidth,irradianceSizeHeight,0,GL_RGB,GL_FLOAT,
                     nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindFramebuffer(GL_FRAMEBUFFER,iblFBO);
    glBindRenderbuffer(GL_RENDERBUFFER,iblRBO);
    glRenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH_COMPONENT24,
                          irradianceSizeWidth,irradianceSizeHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_RENDERBUFFER,
                              iblRBO);

    irradianceShader.use();
    irradianceShader.setMat4("projection",captureProjection);
    irradianceShader.setInt("envCubeMap",0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP,envCubeMapId);

    glViewport(0,0,irradianceSizeWidth,irradianceSizeHeight);
    glBindFramebuffer(GL_FRAMEBUFFER,iblFBO);
    for (int i = 0; i < 6; ++i) {
        irradianceShader.setMat4("view",captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_CUBE_MAP_POSITIVE_X+i,irradianceCubeMapId,0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        simpleModel.renderCube();
    }
    glBindFramebuffer(GL_FRAMEBUFFER,0);

    // resolve each face data to texture_2D
    texture2DOfIrradianceCubeMap = TextureTools::loadTexture2DFromCubeMap(irradianceCubeMapId,GL_RGB,GL_FLOAT);
}

void IBL::calculateBrdfLutTexture() {
    glGenTextures(1,&brdfTextureId);
    glBindTexture(GL_TEXTURE_2D,brdfTextureId);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RG16F,
                 brdfTextureSizeWidth,brdfTextureSizeHeight,0,GL_RG,GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindFramebuffer(GL_FRAMEBUFFER,iblFBO);
    glBindRenderbuffer(GL_RENDERBUFFER,iblRBO);
    glRenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH_COMPONENT24,brdfTextureSizeWidth,brdfTextureSizeHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_RENDERBUFFER,iblRBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,brdfTextureId,0);

    glViewport(0,0,brdfTextureSizeWidth,brdfTextureSizeHeight);
    brdfLutShader.use();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    simpleModel.renderQuad();

    glBindFramebuffer(GL_FRAMEBUFFER,0);
}

void IBL::calculatePrefilterCubeMap() {
    glGenTextures(1,&prefilterCubeMapId);
    glBindTexture(GL_TEXTURE_CUBE_MAP,prefilterCubeMapId);
    for (int i = 0; i < 6; ++i) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,GL_RGB16F,
                     prefilterCubeSizeWidthMip0,prefilterCubeSizeHeightMip0,
                     0,GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    prefilterShader.use();
    prefilterShader.setMat4("projection",captureProjection);
    prefilterShader.setInt("environmentMap",0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP,envCubeMapId);

    glBindFramebuffer(GL_FRAMEBUFFER,iblFBO);
    unsigned int maxMipLevels = 5;
    for (int mip = 0; mip < maxMipLevels; ++mip) {
        auto mipWidth = static_cast<int>(prefilterCubeSizeWidthMip0 * std::pow(0.5,mip));
        auto mipHeight = static_cast<int>(prefilterCubeSizeHeightMip0 * std::pow(0.5,mip));
        glBindRenderbuffer(GL_RENDERBUFFER,iblRBO);
        glRenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH_COMPONENT24,mipWidth,mipHeight);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,
                                  GL_RENDERBUFFER,iblRBO);
        glViewport(0,0,mipWidth,mipHeight);

        float roughness = static_cast<float>(mip) / static_cast<float>(maxMipLevels-1);
        prefilterShader.setFloat("roughness",roughness);
        for (unsigned int i = 0; i < 6; ++i)
        {
            prefilterShader.setMat4("view", captureViews[i]);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                   GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilterCubeMapId, mip);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            simpleModel.renderCube();
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER,0);

    // resolve each face data to texture_2D
    for (int i = 0; i < maxMipLevels; ++i) {
        std::vector<unsigned int> textureId = TextureTools::loadTexture2DFromCubeMap(prefilterCubeMapId,GL_RGB,GL_FLOAT,i);
        texture2DOfPrefilterCubeMap.push_back(textureId);
    }
}

void IBL::initShaders() {
    hdrToCubeMapShader = Shader("../glsl/ibl/cubemap.vert","../glsl/ibl/hdr_to_cubeMap.frag");
    irradianceShader = Shader("../glsl/ibl/cubemap.vert","../glsl/ibl/irradiance.frag");
    prefilterShader = Shader("../glsl/ibl/cubemap.vert", "../glsl/ibl/prefilter.frag");
    brdfLutShader = Shader("../glsl/ibl/brdf.vert","../glsl/ibl/brdf.frag");
}

void IBL::drawIbl() {
    initShaders();
    hdrToCubeMap();
    calculateIrradianceMap();
    calculatePrefilterCubeMap();
    calculateBrdfLutTexture();
}

unsigned int IBL::getHdrCubeMapId() const {
    return envCubeMapId;
}

unsigned int IBL::getIrradianceCubeMapId() const {
    return irradianceCubeMapId;
}

unsigned int IBL::getPrefilterCubeMapId() const {
    return prefilterCubeMapId;
}

unsigned int IBL::getBrdfLutTextureId() const {
    return brdfTextureId;
}

void IBL::initFbOAndRbo() {
    glGenFramebuffers(1,&iblFBO);
    glGenRenderbuffers(1,&iblRBO);

    glBindFramebuffer(GL_FRAMEBUFFER,iblFBO);
    glBindRenderbuffer(GL_RENDERBUFFER,iblRBO);
    glRenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH_COMPONENT24,depthBufferSize,depthBufferSize);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_RENDERBUFFER,iblRBO);
}

std::vector<unsigned int> IBL::getTextures2DOfEnvCubeMap() const {
    return texture2DOfEnvCubeMap;
}

std::vector<unsigned int> IBL::getTextures2DOfIrradianceCubeMap() const {
    return texture2DOfIrradianceCubeMap;
}

std::vector<std::vector<unsigned int>> IBL::getTextures2DOfPrefilteredCubeMap() const {
    return texture2DOfPrefilterCubeMap;
}