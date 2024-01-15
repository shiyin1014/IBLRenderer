//
// Created by shiyin on 2023/12/13.
//

#pragma once

#ifndef SIMPLERENDERER_IMGUICONTROL_H
#define SIMPLERENDERER_IMGUICONTROL_H

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "memory"
#include "camera.h"
#include "usercontrol.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "renderer.h"
#include <utility>
#include <iostream>
#include "glm/gtc/type_ptr.hpp"
#include "mouseCastRay.h"

class ImGuiControl{
public:

    ImGuiControl(std::shared_ptr<UserControl> sharedPtrUserControl,
                 std::shared_ptr<Renderer> sharedPtrRenderer);
    static void initImGui(GLFWwindow* window);
    void imGuiFunction(GLFWwindow *window);
    void imGuiPanel(GLFWwindow *window);
    void displayBvhTree(const BVHNode* root);
    static void shutDown();
    static void setNewFont(const std::string &str);

    ~ImGuiControl() = default;

private:

    std::shared_ptr<UserControl> userControl;
    std::shared_ptr<Renderer> renderer;
};

#endif //SIMPLERENDERER_IMGUICONTROL_H
