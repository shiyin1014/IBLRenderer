//
// Created by shiyin on 2023/12/9.
//
#include "../core/frameBuffer.h"

FrameBuffer::FrameBuffer(int width, int height, int numColorTextures,bool enableDepth, bool enableStencil,
                         bool MSAA, bool enableRBO) :
        width(width), height(height), numColorAttachments(numColorTextures), enableMSAA(MSAA),
        frameBufferID(0), rboDepthId(0), rboStencilId(0), RBO(enableRBO), depthTextureId(0), stencilTextureId(0){

    // create a fbo
    glGenFramebuffers(1,&frameBufferID);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBufferID);

    if (RBO){ // RBO mode
        // colors info
        rboColorId.resize(numColorTextures);
        for (int i = 0; i < numColorTextures; ++i) {
            glGenRenderbuffers(1,&rboColorId[i]);
            glBindRenderbuffer(GL_RENDERBUFFER,rboColorId[i]);
            if (enableMSAA){
                glRenderbufferStorageMultisample(GL_RENDERBUFFER,4,GL_RGBA8,width,height);
            }else{
                glRenderbufferStorage(GL_RENDERBUFFER,GL_RGBA8,width,height);
            }
            glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0 + i,GL_RENDERBUFFER,rboColorId[i]);
        }

        if (enableDepth){
            // depth info
            glGenRenderbuffers(1,&rboDepthId);
            glBindRenderbuffer(GL_RENDERBUFFER, rboDepthId);
            if (enableMSAA){
                glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT, width, height);
            }else{
                glRenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH_COMPONENT,width,height);
            }
            // attach to FBO for depth test
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepthId);
        }

        if (enableStencil){
            // stencil info
            glGenRenderbuffers(1,&rboStencilId);
            glBindRenderbuffer(GL_RENDERBUFFER,rboStencilId);
            if (enableMSAA){
                glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4,GL_STENCIL_INDEX, width, height);
            }else{
                glRenderbufferStorage(GL_RENDERBUFFER,GL_STENCIL_INDEX,width,height);
            }
            // attach to fbo for stencil test
            glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_STENCIL_ATTACHMENT,GL_RENDERBUFFER,rboStencilId);
        }

    }else{  // texture mode

        // only create a depthBuffer to store depth information
        if (numColorTextures == 0){
            glGenTextures(1,&depthTextureId);
            glBindTexture(GL_TEXTURE_2D, depthTextureId);
            glTexImage2D(GL_TEXTURE_2D,0,GL_DEPTH_COMPONENT16,width,height,0,GL_DEPTH_COMPONENT,GL_FLOAT, nullptr);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_BORDER);
            GLfloat borderColor[] = {1.0,1.0,1.0,1.0};
            glTexParameterfv(GL_TEXTURE_2D,GL_TEXTURE_BORDER_COLOR,borderColor);
            glFramebufferTexture2D(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_TEXTURE_2D,depthTextureId,0);
            // 置读写缓冲为NULL
            glDrawBuffer(GL_NONE);
            glReadBuffer(GL_NONE);
        }else{
            // create colorTextureBuffers
            colorTextureId.resize(numColorTextures);
            for (int i = 0; i < numColorTextures; ++i) {
                glGenTextures(1,&colorTextureId[i]);
                glBindTexture(enableMSAA ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D, colorTextureId[i]);
                if (enableMSAA){
                    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE,4,GL_RGBA, width,height,GL_TRUE);
                    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                }else{
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                }
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i,
                                       enableMSAA ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D, colorTextureId[i],0);
            }

            // create depthStencil RBO, note that for the depth and stencil we use RBO to attach the FBO
            if (enableDepth || enableStencil){
                // create depthRBO
                glGenRenderbuffers(1,&depthTextureId);
                glBindRenderbuffer(GL_RENDERBUFFER,depthTextureId);
                if (enableMSAA){
                    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, width, height);
                }else{
                    glRenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH24_STENCIL8,width,height);
                }
                glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_STENCIL_ATTACHMENT,GL_RENDERBUFFER,depthTextureId);
            }
        }
    }

    // setup multi-color
    if (numColorAttachments > 1){
        auto* drawBuffers = new GLenum[numColorAttachments];
        for (int i = 0; i < numColorAttachments; ++i) {
            drawBuffers[i] = GL_COLOR_ATTACHMENT0 + i;
        }
        glDrawBuffers(numColorAttachments, drawBuffers);
        delete[] drawBuffers;
    }

    checkIsComplete();
    glBindFramebuffer(GL_FRAMEBUFFER,0);
}

void FrameBuffer::bind() const {
    glBindFramebuffer(GL_FRAMEBUFFER, frameBufferID);
}

void FrameBuffer::unbind() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBuffer::checkIsComplete() {
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        switch (status) {
            case GL_FRAMEBUFFER_UNDEFINED:
                Logger::Log<FrameBuffer>(__FUNCTION__, "Framebuffer is undefined", Logger::LogType::ERROR);
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
                Logger::Log<FrameBuffer>(__FUNCTION__, "Framebuffer has incomplete attachment(s)", Logger::LogType::ERROR);
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
                Logger::Log<FrameBuffer>(__FUNCTION__, "Framebuffer is missing attachment", Logger::LogType::ERROR);
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
                Logger::Log<FrameBuffer>(__FUNCTION__, "Framebuffer has incomplete draw buffer", Logger::LogType::ERROR);
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
                Logger::Log<FrameBuffer>(__FUNCTION__, "Framebuffer has incomplete read buffer", Logger::LogType::ERROR);
                break;
            case GL_FRAMEBUFFER_UNSUPPORTED:
                Logger::Log<FrameBuffer>(__FUNCTION__, "Framebuffer format is unsupported", Logger::LogType::ERROR);
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
                Logger::Log<FrameBuffer>(__FUNCTION__, "Framebuffer has incomplete multisample settings", Logger::LogType::ERROR);
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
                Logger::Log<FrameBuffer>(__FUNCTION__, "Framebuffer has incomplete layer targets", Logger::LogType::ERROR);
                break;
            default:
                Logger::Log<FrameBuffer>(__FUNCTION__, "Framebuffer is incomplete for unknown reasons", Logger::LogType::ERROR);
                break;
        }
    }
}

GLuint FrameBuffer::getDepthTextureId() const {
    return depthTextureId;
}

FrameBuffer::~FrameBuffer() {
    glDeleteFramebuffers(1,&frameBufferID);
    glDeleteRenderbuffers(1,&rboDepthId);
    glDeleteRenderbuffers(1,&rboStencilId);
    glDeleteTextures(numColorAttachments,rboColorId.data());
}

GLuint FrameBuffer::getFrameBufferId() const {
    return frameBufferID;
}

GLuint FrameBuffer::getColorTextureId(int index) const {
    return colorTextureId[index];
}

int FrameBuffer::getWidth() const {
    return width;
}

int FrameBuffer::getHeight() const {
    return height;
}

GLuint FrameBuffer::getStencilTextureId() const {
    return stencilTextureId;
}

