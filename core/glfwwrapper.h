//
// Created by shiyin on 2023/12/12.
//

#pragma once

#ifndef SIMPLERENDERER_GLFWWRAPPER_H
#define SIMPLERENDERER_GLFWWRAPPER_H

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "camera.h"
#include <iostream>
#include "usercontrol.h"
#include "renderer.h"
#include "memory"
#include "imguicontrol.h"

class GlfwWrapper{
public:

    GlfwWrapper(int width, int height);
    ~GlfwWrapper();
    bool initGlfw(int width, int height);
    static void onFrameBufferSize(int width, int height);
    void onScroll(double xOffset, double yOffset);
    void onCursorPos();
    void onMouseButton(int button, int action, int mods);
    void setRenderer(const std::shared_ptr<Renderer>& sharedPtr);
    void run();
    static void enableDepthTest();
    static void enableStencilTest();
    static void enableCubeMapSeamless();
    static void enableCullBackFace();
    void createAndInitImGui();
    void destroyWindow();

private:
    GLFWwindow* window;
    std::shared_ptr<UserControl> userControl;
    std::unique_ptr<ImGuiControl> imGuiControl;
    std::shared_ptr<Renderer> renderer;

    static void framebufferSizeCallback(GLFWwindow *window, int width, int height);

    static void mouseCallback(GLFWwindow *window, double xPosIn, double yPosIn);

    static void scrollCallback(GLFWwindow *window, double xOffset, double yOffset);

    static void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods);

    void processInput(GLFWwindow *glfWindow);
};

#endif //SIMPLERENDERER_GLFWWRAPPER_H
