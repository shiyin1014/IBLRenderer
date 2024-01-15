//
// Created by shiyin on 2023/12/9.
//

#pragma once

#ifndef SIMPLERENDERER_TEXTURETOOLS_H
#define SIMPLERENDERER_TEXTURETOOLS_H

#include "stb_image.h"
#include "stbi_image_write.h"
#include <string>
#include <vector>
#include <glad/glad.h>
#include <memory>
#include "logger.h"
#include "frameBuffer.h"
#include <iostream>
#include <cassert>
#include <cstring>
#include <thread>


class TextureTools{
public:

    static unsigned int loadTexture2D(const std::string& path, bool gammaCorrection = false);

    static unsigned int loadTextureCubeMap(const std::vector<std::string>& faces);

    static unsigned int loadHdrTexture(const std::string& path);

    static std::vector<unsigned int> loadTexture2DFromCubeMap(const unsigned int& cubeMapId,
                                                              const GLenum& format,
                                                              const GLenum& type,
                                                              const int& level = 0);

    static void writeColorBufferToFile(const std::unique_ptr<FrameBuffer>& frameBuffer,
                                       bool enableMultiThreads = false);

};

#endif //SIMPLERENDERER_TEXTURETOOLS_H
