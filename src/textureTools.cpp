//
// Created by shiyin on 2023/12/9.
//

#include "../core/textureTools.h"


unsigned int TextureTools::loadTexture2D(const std::string &path, bool gammaCorrection) {
    unsigned int textureId = -1;
    int width, height, numberComponents;
    unsigned char *data = stbi_load(path.c_str(),&width,&height,&numberComponents,0);
    if (data){
        GLenum format = 0;
        if (numberComponents == 1){
            format = GL_RED;
        }else if (numberComponents == 3){
            format = GL_RGB;
        }else if (numberComponents == 4){
            format = GL_RGBA;
        }
        glGenTextures(1,&textureId);
        glBindTexture(GL_TEXTURE_2D,textureId);
        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     static_cast<int>(format),
                     width,
                     height,
                     0,
                     format,
                     GL_UNSIGNED_BYTE,
                     data);
        glGenerateMipmap(GL_TEXTURE_2D);
        // set the texture wrapping parameters
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    }else{
        Logger::Log<TextureTools>(__FUNCTION__ ,"Texture failed to load at path: " + path);
    }
    stbi_image_free(data);

    glBindTexture(GL_TEXTURE_2D,0);
    return textureId;
}

unsigned int TextureTools::loadTextureCubeMap(const std::vector<std::string> &faces) {

    assert(faces.size() == 6);

    unsigned int cubeMapId = -1;
    glGenTextures(1,&cubeMapId);
    glBindTexture(GL_TEXTURE_CUBE_MAP,cubeMapId);

    int width, height, numberComponents;
    for (int i = 0; i < faces.size(); ++i) {
        unsigned char* data = stbi_load(faces[i].c_str(),&width,&height,&numberComponents,0);
        if (data){
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                         0,
                         GL_RGB,
                         width,
                         height,
                         0,
                         GL_RGB,
                         GL_UNSIGNED_BYTE,
                         data);
        }else{
            Logger::Log<TextureTools>(__FUNCTION__ ,"CubeMap texture failed to load at path: " + faces[i]);
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_CUBE_MAP,0);
    return cubeMapId;
}

unsigned int TextureTools::loadHdrTexture(const std::string &path) {
    stbi_set_flip_vertically_on_load(true);
    int width, height, numberComponents;
    float *data = stbi_loadf(path.c_str(),&width,&height,&numberComponents,0);
    unsigned int hdrTexture = -1;
    if (data){
        glGenTextures(1,&hdrTexture);
        glBindTexture(GL_TEXTURE_2D,hdrTexture);
        glTexImage2D(GL_TEXTURE_2D,0,GL_RGB16F,width,height,0,GL_RGB,GL_FLOAT,data);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }else{
        Logger::Log<TextureTools>(__FUNCTION__ ,"Fail to load HDR image at path : " + path);
    }

    stbi_image_free(data);
    return hdrTexture;
}

std::vector<unsigned int> TextureTools::loadTexture2DFromCubeMap(const unsigned int &cubeMapId,
                                                                 const GLenum& format,
                                                                 const GLenum& type,
                                                                 const int& level) {
    std::vector<unsigned int> textures;

    for (int i = 0; i < 6; ++i) {
        // bind cubeMap
        glBindTexture(GL_TEXTURE_CUBE_MAP,cubeMapId);
        GLint cubeMapSize;
        glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X,level,GL_TEXTURE_WIDTH,&cubeMapSize);
        GLint internalFormat;
        glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X,level,GL_TEXTURE_INTERNAL_FORMAT,&internalFormat);
        void* data;
        if (type == GL_UNSIGNED_BYTE) {
            data = new GLubyte[cubeMapSize * cubeMapSize * 4];
        } else {
            data = new GLfloat[cubeMapSize * cubeMapSize * 4];
        }
        glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,level,format,type,data);

        // create a 2D texture to save data of each face
        unsigned int texture;
        glGenTextures(1,&texture);
        glBindTexture(GL_TEXTURE_2D,texture);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D,0,format,cubeMapSize,cubeMapSize,0,format,type,data);
        glBindTexture(GL_TEXTURE_2D,0);
        // free
        if (type == GL_UNSIGNED_BYTE){
            delete[] reinterpret_cast<GLubyte*>(data);
        }else{
            delete[] reinterpret_cast<GLfloat*>(data);
        }
        // add
        textures.push_back(texture);
    }

    // unbind
    glBindTexture(GL_TEXTURE_CUBE_MAP,0);

    return textures;
}

void copyPixels(const std::vector<unsigned char>& sourcePixels,
                std::vector<unsigned char>& targetPixels,
                int width, int height, int startY, int endY){
    const int rowSize = width * 4;
    for (int y = startY; y < endY; ++y) {
        std::memcpy(targetPixels.data() + (height - y - 1) * rowSize,
                    sourcePixels.data() + y * rowSize,
                    rowSize);
    }
}

void parallelFlipPixels(const std::vector<unsigned char>& sourcePixels,
                        std::vector<unsigned char>& targetPixels,
                        int width, int height, int numThreads){

    const int numRowsPerThread = height / numThreads;
    std::vector<std::thread> threads;

    for (int i = 0; i < numThreads - 1; ++i) {
        int startY = i * numRowsPerThread;
        int endY = startY + numRowsPerThread;
        threads.emplace_back(copyPixels,std::ref(sourcePixels),std::ref(targetPixels),width,height,startY,endY);
    }

    int startY = (numThreads - 1) * numRowsPerThread;
    int endY = height;
    copyPixels(sourcePixels,targetPixels,width,height,startY,endY);

    // wait
    for (auto & thread : threads) {
        thread.join();
    }
}

void TextureTools::writeColorBufferToFile(const std::unique_ptr<FrameBuffer>& frameBuffer,
                                          bool enableMultiThreads) {

    int screenWidth = frameBuffer->getWidth();
    int screenHeight = frameBuffer->getHeight();

    std::vector<unsigned char> pixels(screenWidth * screenHeight * 4);

    glBindFramebuffer(GL_READ_FRAMEBUFFER,frameBuffer->getFrameBufferId());
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glReadPixels(0,0,screenWidth,screenHeight,
                 GL_RGBA,GL_UNSIGNED_BYTE,pixels.data());

    // flip y
    std::vector<unsigned char> flippedPixels(screenWidth * screenHeight * 4);
    // start time
    auto startTime = std::chrono::high_resolution_clock::now();
    if (!enableMultiThreads){
        //  single thread
        for (int y = 0; y < screenHeight; ++y) {
            memcpy(flippedPixels.data() + (screenHeight - y - 1) * screenWidth * 4,
                   pixels.data() + y * screenWidth * 4, screenWidth * 4);
        }
    }else{
        // multi threads
        int numThreads = 2;
        parallelFlipPixels(pixels,flippedPixels,screenWidth,screenHeight,numThreads);
    }
    // end time
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

    // save
    std::string name =  "../out/screenShot" + std::to_string(time(nullptr)) + ".png";
    stbi_write_png(name.c_str(),screenWidth,screenHeight,
                   4,flippedPixels.data(),screenWidth * 4);

    // log
    std::string msg = "Save image to : " + name + ", execution time : " + std::to_string(duration.count()) + " milliseconds";
    Logger::Log<TextureTools>(__FUNCTION__ ,msg);

    // unbind
    glBindFramebuffer(GL_READ_FRAMEBUFFER,0);
}
