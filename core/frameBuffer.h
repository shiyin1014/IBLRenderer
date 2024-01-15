//
// Created by shiyin on 2023/12/9.
//
#pragma once

#ifndef SIMPLERENDERER_FRAMEBUFFER_H
#define SIMPLERENDERER_FRAMEBUFFER_H

#include "glad/glad.h"
#include <iostream>
#include <vector>
#include "logger.h"

class FrameBuffer{
public:

    FrameBuffer(int width, int height, int numColorTextures = 1,
                bool enableDepth = false, bool enableStencil = false,
                bool enableMSAA = false, bool enableRBO = false);

    void bind() const;
    void unbind();

    static void checkIsComplete() ;

    GLuint getColorTextureId(int index = 0) const;
    GLuint getDepthTextureId() const;
    GLuint getStencilTextureId() const;

    GLuint getFrameBufferId() const;

    int getWidth() const;
    int getHeight() const;

    ~FrameBuffer();

private:

    GLuint frameBufferID;

    // texture mode
    std::vector<GLuint> colorTextureId;
    GLuint depthTextureId;
    GLuint stencilTextureId;

    // rbo mode
    std::vector<GLuint> rboColorId;
    GLuint rboDepthId;
    GLuint rboStencilId;

    int width, height;
    int numColorAttachments;
    bool enableMSAA;
    bool RBO;
};

inline void BlitFBOToAnotherFBO(const FrameBuffer* src, const FrameBuffer* dst, GLbitfield mask,
                                GLenum attachmentColorReadIndex = GL_COLOR_ATTACHMENT0,
                                GLenum attachmentColorDrawIndex = GL_COLOR_ATTACHMENT0, GLenum filter = GL_NEAREST){
    glBindFramebuffer(GL_READ_FRAMEBUFFER,src->getFrameBufferId());
    glReadBuffer(attachmentColorReadIndex);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dst->getFrameBufferId());
    glDrawBuffer(attachmentColorDrawIndex);
    glBlitFramebuffer(0,0,src->getWidth(),src->getHeight(),
                      0,0,dst->getWidth(),dst->getHeight(),
                      mask,filter);
    glBindFramebuffer(GL_FRAMEBUFFER,0);
}

#endif //SIMPLERENDERER_FRAMEBUFFER_H
