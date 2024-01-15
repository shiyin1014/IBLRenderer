//
// Created by shiyin on 2023/12/12.
//
#include <memory>

#include "../core/glfwwrapper.h"


bool GlfwWrapper::initGlfw(int width, int height) {
    //init
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_OPENGL_ANY_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE,GL_FALSE);
    glfwWindowHint(GLFW_SAMPLES,4);
    window = glfwCreateWindow(width, height, "IBLRenderer", nullptr, nullptr);
    if (window == nullptr) {
        std::cout << "GLFW window create failed." << std::endl;
        glfwTerminate();
        return false;
    }
    // 将实例指针关联到窗口的用户指针
    glfwSetWindowUserPointer(window, this);
    // 设置上下文
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);

    //load all opengl function pointers
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "initialize glad failed" << std::endl;
        return false;
    }
    return true;
}

GlfwWrapper::GlfwWrapper(int width, int height) : window(nullptr),userControl(new UserControl()){
    if (!initGlfw(width,height)){
        std::cout << "GLFW window create failed." << std::endl;
    }
    enableDepthTest();
    enableStencilTest();
    enableCubeMapSeamless();
}

void GlfwWrapper::framebufferSizeCallback(GLFWwindow *window, int width, int height) {
    auto* instance = static_cast<GlfwWrapper*>(glfwGetWindowUserPointer(window));
    instance->onFrameBufferSize(width,height);
}

void GlfwWrapper::mouseCallback(GLFWwindow *window, double xPosIn, double yPosIn) {
    auto* instance = static_cast<GlfwWrapper*>(glfwGetWindowUserPointer(window));
    instance->onCursorPos();
}

void GlfwWrapper::scrollCallback(GLFWwindow *window, double xOffset, double yOffset) {
    auto* instance = static_cast<GlfwWrapper*>(glfwGetWindowUserPointer(window));
    instance->onScroll(xOffset,yOffset);
}

void GlfwWrapper::mouseButtonCallback(GLFWwindow *window, int button, int action, int mods) {
    auto* instance = static_cast<GlfwWrapper*>(glfwGetWindowUserPointer(window));
    instance->onMouseButton(button,action,mods);
}

void GlfwWrapper::processInput(GLFWwindow *glfWindow) {
    if (glfwGetKey(glfWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(glfWindow, true);
    } else if (glfwGetKey(glfWindow, GLFW_KEY_SPACE) == GLFW_PRESS){
        renderer->getCamera()->restoreDefaultSetting();
    }
}

void GlfwWrapper::onFrameBufferSize(int width, int height) {
    glad_glViewport(0.0f,0.0f, width, height);
}

void GlfwWrapper::onScroll(double xOffset, double yOffset) {
    if (userControl->mouseScroll){
        renderer->getCamera()->zoom(static_cast<float>(yOffset) * 0.5f);
    }
}

void GlfwWrapper::onCursorPos() {
    glfwGetCursorPos(window, &userControl->currentX, &userControl->currentY);
    double delta_x = userControl->currentX - userControl->lastX;
    double delta_y = userControl->currentY - userControl->lastY;
    userControl->lastX = userControl->currentX;
    userControl->lastY = userControl->currentY;
    if (userControl->mouseLeftButtonDown) {//拖动时才移动视野
        if (delta_x!=0)renderer->getCamera()->rotateAzimuth(static_cast<float >(-delta_x) * 0.02f);
        if (delta_y!=0)renderer->getCamera()->rotatePolar(static_cast<float >(delta_y) * 0.02f);
    }else if (userControl->mouseMiddleButtonDown){
        if (delta_x!=0)renderer->getCamera()->moveHorizontal(static_cast<float>(-delta_x)*0.02f);
        if (delta_y!=0)renderer->getCamera()->moveVertical(static_cast<float >(delta_y)*0.02f);
    }
}

void GlfwWrapper::onMouseButton(int button, int action, int mods) {
    switch (button) {
        case GLFW_MOUSE_BUTTON_LEFT:{//鼠标左键被按下
            if (action == GLFW_PRESS) {
                userControl->mouseCastRayLeftButton = true;
                userControl->mouseLeftButtonDown = true;
                glfwGetCursorPos(window, &userControl->lastX, &userControl->lastY);
            } else if (action == GLFW_RELEASE) {
                userControl->mouseLeftButtonDown = false;
                userControl->mouseCastRayLeftButton = false;
            }
            break;
        }
        case GLFW_MOUSE_BUTTON_MIDDLE:{
            if (action == GLFW_PRESS) {
                userControl->mouseMiddleButtonDown = true;
                glfwGetCursorPos(window, &userControl->lastX, &userControl->lastY);
            } else if (action == GLFW_RELEASE) {
                userControl->mouseMiddleButtonDown = false;
            }
            break;
        }
        default:
            break;
    }
}

void GlfwWrapper::run() {

    // create framebuffer to render scene to
    renderer->createFrameBuffers();

    while(!glfwWindowShouldClose(window)){
        // ImGui
        imGuiControl->imGuiFunction(window);

        // process input
        processInput(window);

        // Draw Scene
        renderer->render();

        // Render ImGui Data
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Update window
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    imGuiControl->shutDown();
    destroyWindow();
    glfwTerminate();
}

void GlfwWrapper::enableDepthTest() {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
}

void GlfwWrapper::enableStencilTest() {
    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_KEEP,GL_KEEP,GL_REPLACE);
}

void GlfwWrapper::enableCubeMapSeamless() {
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}

void GlfwWrapper::createAndInitImGui() {
    imGuiControl = std::unique_ptr<ImGuiControl>(new ImGuiControl(userControl,renderer));
    imGuiControl->initImGui(window);
}

void GlfwWrapper::destroyWindow() {
    if (window){
        glfwDestroyWindow(window);
    }
}

void GlfwWrapper::setRenderer(const std::shared_ptr<Renderer> &sharedPtr) {
    this->renderer = sharedPtr;
    // initGui after setRenderer
    createAndInitImGui();
}

void GlfwWrapper::enableCullBackFace() {
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
}

GlfwWrapper::~GlfwWrapper() {
    if (window){
        glfwDestroyWindow(window);
        window = nullptr;
    }
}


