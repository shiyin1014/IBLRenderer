//
// Created by shiyin on 2023/12/8.
//

#include "../core/renderer.h"

void Renderer::render() {
    if (drawEnvCubeMap) {

        // shadow
        if (shadowMethod != ShadowMethod::None) drawDepthMapOnly(); // first pass : draw depthMap
        // debugDepthMapToScreen(); // draw depthMap to screen if needed
        sceneMSAAFBO->bind();   // draw scene
            glViewport(0,0,camera->getWidth(),camera->getHeight());
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
            // draw
            setSceneUniforms(); // uniforms of scene
            setShadowUniforms(); // uniforms of shadow
            bindIblTextures();  // iblTextures
            drawPointLights();  // draw point lights
            scene->drawScene(sceneShader, false);  // render scene
            drawBoundingBox();  // drawBbox
            drawSkyBox();   // draw skybox
            if (hitModel) drawOutlineOfSelectedModel();    // draw outline if needed
            if (showCastedRays) drawCastRay();   // draw castRay if needed
        // unbind
        sceneMSAAFBO->unbind();

        // blitSceneFBO used to store the colorBuffer(scene(attachment0) and outline(attachment1)) and depthBuffer
        // sceneColorBuffer and depthBuffer
        BlitFBOToAnotherFBO(sceneMSAAFBO.get(),
                            blitSceneFBO.get(),
                            GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // outlineColorBuffer, used to blur, if rayHitModel
        if (hitModel){
            BlitFBOToAnotherFBO(sceneMSAAFBO.get(),
                                blitSceneFBO.get(),
                                GL_COLOR_BUFFER_BIT,
                                GL_COLOR_ATTACHMENT1,
                                GL_COLOR_ATTACHMENT1);
            blurOutlineColorBuffer();         // blur the outlineFBO
        }

        // sceneColor merge with blur outline
        finalColorToBuffer(hitModel);      // sceneColor + blurColor

        // gaussian blur, sharpen and others if needed, render to screenshot fbo
        renderWithPostProcess();

        // final render to screen
        finalRenderToScreen();

    } else if (drawIrradianceCubeMap || drawPrefilterCubeMap) {
        drawSkyBox();
    } else if (drawBrdfLutTexture2D) {
        drawIblBrdfLutTexture();
    }

    // store current frameBuffer and display it in the imGui panel if needed
    saveCurrentFrameBuffer();
}

Renderer::Renderer(const std::shared_ptr<Camera> &cameraPtr)
        : camera(cameraPtr),
          backgroundColor(0, 0, 0), shadowMethod(ShadowMethod::None), iblIndex(0),
          hdrToneMapping(true), gammaCorrection(true), simpleModel(), enableMsaa(true),
          drawEnvCubeMap(true), drawIrradianceCubeMap(false), drawPrefilterCubeMap(false), prefilterCubeMapLevel(0),
          drawBrdfLutTexture2D(false), drawSceneBox(false), boxLineColor(1.0f, 1.0f, 1.0f),
          enableSelect(false), showCastedRays(false), sceneMSAAFBO(nullptr), hitModel(nullptr),
          castedRayColor(1.0f, 1.0f, 1.0f), firstFrame(true), enableRotationPointLight(false),
          postProcess(POST_PROCESS::NONE),gaussianParameter{11,7.0f},sharpenKernelParameter(5)
          {
    pointLights.clear();
    initShaders();
    setProjectionMatrix(sceneShader);
}

void Renderer::clear() const {
    glClearColor(backgroundColor.r, backgroundColor.g, backgroundColor.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void Renderer::setScene(const std::shared_ptr<Scene> &scenePtr) {
    this->scene = scenePtr;
    if (scene->hasHdr()) {
        ibl.reserve(scene->hdrPaths.size());
        for (int i = 0; i < scene->hdrPaths.size(); ++i) {
            addHdrScene(scene->getHdrPath(i));
        }
        setIblUniforms();
    }
}

void Renderer::initShaders() {
    sceneShader = Shader("../glsl/scene/default.vert", "../glsl/scene/default.frag");
    skyBoxShader = Shader("../glsl/skybox/skybox.vert", "../glsl/skybox/skybox.frag");
    quadShader = Shader("../glsl/quad/quad.vert", "../glsl/quad/quad.frag");
    boundingBoxShader = Shader("../glsl/bbox/bbox.vert", "../glsl/bbox/bbox.frag");
    castRayShader = Shader("../glsl/ray/castRay.vert","../glsl/ray/castRay.frag");
    outlineShader = Shader("../glsl/scene/default.vert","../glsl/scene/outline.frag");
    blurShader = Shader("../glsl/scene/bluroutline.vert","../glsl/scene/bluroutline.frag");
    finalShader = Shader("../glsl/scene/bluroutline.vert","../glsl/scene/final.frag");
    lightShader = Shader("../glsl/pointlight/light.vert","../glsl/pointlight/light.frag");
    depthShader = Shader("../glsl/shadow/depthmap.vert","../glsl/shadow/depthmap.frag");
    depthDebugShader = Shader("../glsl/shadow/debug.vert","../glsl/shadow/debug.frag");
    postProcessShader = Shader("../glsl/quad/quad.vert", "../glsl/scene/postprocess.frag");
}

void Renderer::addPointLight(const std::shared_ptr<PointLight> &light) {
    if (pointLights.empty()){
        defaultLightPos = light->getLightPosition();
    }
    pointLights.push_back(light);
}

void Renderer::drawSkyBox() {

    if (drawIrradianceCubeMap || drawPrefilterCubeMap){ // render to screen directly
        // render to screen
        renderToScreenSettings();
    }

    skyBoxShader.use();
    skyBoxShader.setInt("cubeMap", 0);
    setViewMatrix(skyBoxShader);
    setProjectionMatrix(skyBoxShader);
    glActiveTexture(GL_TEXTURE0);
    if (drawEnvCubeMap) {
        skyBoxShader.setFloat("level", 0.0f);
        glBindTexture(GL_TEXTURE_CUBE_MAP, ibl[iblIndex]->getHdrCubeMapId());
    } else if (drawIrradianceCubeMap) {
        skyBoxShader.setFloat("level", 0.0f);
        glBindTexture(GL_TEXTURE_CUBE_MAP, ibl[iblIndex]->getIrradianceCubeMapId());
    } else if (drawPrefilterCubeMap) {
        skyBoxShader.setFloat("level", static_cast<float>(prefilterCubeMapLevel));
        glBindTexture(GL_TEXTURE_CUBE_MAP, ibl[iblIndex]->getPrefilterCubeMapId());
    }
    simpleModel.renderCube();
}

Model* Renderer::selectModel(const Ray& castRay) {
    Model* model = scene->intersect(castRay);
    return model;
}

void Renderer::drawOutlineOfSelectedModel() {

    // Object Outlining
    if (hitModel){
        glStencilFunc(GL_NOTEQUAL, 1, 0xFF);    // 重新定义模板缓冲的通过规则，只有不相等时才通过模板测试
        glStencilMask(0x00);    // 禁用模板缓冲的写入操作
//        glDisable(GL_DEPTH_TEST);

        // 绘制边框
        outlineShader.use();
        glm::mat4 scale = hitModel->getScale();
        glm::mat4 rotate = hitModel->getRotate();
        glm::mat4 translation = hitModel->getTranslate();
        glm::mat4 scaleMatrix = glm::scale(scale,glm::vec3 (1.01f,1.01f,1.01f));
        glm::mat4 model = translation * rotate * scaleMatrix;
        outlineShader.setMat4("model",model);
        setViewMatrix(outlineShader);
        setProjectionMatrix(outlineShader);
        hitModel->draw(outlineShader);

        // 恢复模板设置
        glStencilFunc(GL_ALWAYS, 0, 0xFF);
        glStencilMask(0xFF);
//        glEnable(GL_DEPTH_TEST);
    }
}

void Renderer::setViewMatrix(const Shader &shader) {
    shader.use();
    shader.setMat4("view", camera->getViewMatrix());
}

void Renderer::setCameraPosUniform() {
    sceneShader.use();
    sceneShader.setVec3("camPos", camera->getEye());
}

void Renderer::setProjectionMatrix(const Shader &shader) {
    shader.use();
    shader.setMat4("projection", camera->getProjectionMatrix());
}

void Renderer::addHdrScene(const std::string &path) {
    ibl.push_back(std::unique_ptr<IBL>(new IBL(path)));
}

void Renderer::setIblUniforms() {
    sceneShader.use();
    sceneShader.setInt("irradianceMap", 6);
    sceneShader.setInt("prefilterMap", 7);
    sceneShader.setInt("brdfLUTMap", 8);
}

void Renderer::bindIblTextures() {
    sceneShader.use();
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_CUBE_MAP, ibl[iblIndex]->getIrradianceCubeMapId());
    glActiveTexture(GL_TEXTURE7);
    glBindTexture(GL_TEXTURE_CUBE_MAP, ibl[iblIndex]->getPrefilterCubeMapId());
    glActiveTexture(GL_TEXTURE8);
    glBindTexture(GL_TEXTURE_2D, ibl[iblIndex]->getBrdfLutTextureId());
}

void Renderer::setLightsUniforms(const Shader& shader) {
    shader.use();
    if (enableRotationPointLight) setLightsRotation();
    for (int i = 0; i < pointLights.size(); ++i) {
        shader.setVec3("lightPositions[" + std::to_string(i) + "]", pointLights[i]->getLightPosition());
        shader.setVec3("lightColors[" + std::to_string(i) + "]", pointLights[i]->getLightColor());
    }
}

void Renderer::drawIblBrdfLutTexture() {
    // render to screen
    renderToScreenSettings();
    quadShader.use();
    quadShader.setInt("quadTexture", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ibl[iblIndex]->getBrdfLutTextureId());
    simpleModel.renderQuad();
}

void Renderer::setSceneUniforms() {
    sceneShader.use();
    setViewMatrix(sceneShader);
    setCameraPosUniform();
    setLightsUniforms(sceneShader);
    sceneShader.setFloat("near",camera->getNear());
    sceneShader.setFloat("far",camera->getFar());
    sceneShader.setInt("HDRToneMapping", hdrToneMapping);
    sceneShader.setInt("gammaCorrection", gammaCorrection);
}

glm::vec3 Renderer::getBackgroundColor() const {
    return backgroundColor;
}

void Renderer::setBackgroundColor(glm::vec3 color) {
    this->backgroundColor = color;
}

std::shared_ptr<Camera> Renderer::getCamera() const {
    return camera;
}

void Renderer::setHdr() {
    hdrToneMapping = !hdrToneMapping;
    sceneShader.use();
    sceneShader.setInt("HDRToneMapping", hdrToneMapping);
}

void Renderer::setGamma() {
    gammaCorrection = !gammaCorrection;
    sceneShader.use();
    sceneShader.setInt("gammaCorrection", gammaCorrection);
}

bool Renderer::getEnableHdr() const {
    return hdrToneMapping;
}

bool Renderer::getEnableGamma() const {
    return gammaCorrection;
}

std::vector<std::shared_ptr<PointLight>> Renderer::getPointLights() const {
    return pointLights;
}

std::shared_ptr<Scene> Renderer::getScene() const {
    return scene;
}

bool Renderer::getEnableMsaa() const {
    return enableMsaa;
}

void Renderer::setMsaa() {
    enableMsaa = !enableMsaa;
    if (enableMsaa) {
        glEnable(GL_MULTISAMPLE);
    } else {
        glDisable(GL_MULTISAMPLE);
    }
}

unsigned int Renderer::getIblCubeMapId() const {
    return ibl[iblIndex]->getHdrCubeMapId();
}

unsigned int Renderer::getIblPrefilterMapId() const {
    return ibl[iblIndex]->getPrefilterCubeMapId();
}

unsigned int Renderer::getIblBrdfLutTexture2D() const {
    return ibl[iblIndex]->getBrdfLutTextureId();
}

std::vector<unsigned int> Renderer::getTextures2DOfEnvCubeMap() const {
    return ibl[iblIndex]->getTextures2DOfEnvCubeMap();
}

std::vector<unsigned int> Renderer::getTextures2DOfIrradianceCubeMap() const {
    return ibl[iblIndex]->getTextures2DOfIrradianceCubeMap();
}

std::vector<std::vector<unsigned int>> Renderer::getTextures2DOfPrefilteredCubeMap() const {
    return ibl[iblIndex]->getTextures2DOfPrefilteredCubeMap();
}

void Renderer::drawBoundingBox() {
    boundingBoxShader.use();
    boundingBoxShader.setMat4("model",glm::mat4 (1.0f));
    setViewMatrix(boundingBoxShader);
    setProjectionMatrix(boundingBoxShader);
    boundingBoxShader.setVec3("lineColor", boxLineColor);
    if (drawSceneBox) {
        std::vector<BoundingBox3> sceneBox = scene->getSceneBVHBoundingBox();
        if (sceneBox.empty()) return;
        for (const auto & i : sceneBox) {
            simpleModel.renderBbox(i);
        }
    }else{
        for (auto & model : scene->models)
            model->drawBox(simpleModel);
    }
}

void Renderer::addCastedRay(const glm::vec3& endPoint){
    simpleModel.addRayVertices(camera->getEye(),endPoint);
}

void Renderer::drawCastRay() {
    castRayShader.use();
    setViewMatrix(castRayShader);
    setProjectionMatrix(castRayShader);
    castRayShader.setVec3("color",castedRayColor);
    simpleModel.renderRayLines();
}

void Renderer::createFrameBuffers() {
    // 纹理1保存场景颜色，纹理2保存选中的模型边框颜色，纹理3保存场景的深度的可视化图
    // MSAA的纹理需要降采样为非MSAA的纹理才能够进行采样使用
    sceneMSAAFBO = std::unique_ptr<FrameBuffer>(
            new FrameBuffer(camera->getWidth(),camera->getHeight(),3,true, true, true));
    // 纹理1保存场景的颜色（降采样后的）用于合成最后结果，纹理2保存边框纹理颜色值（降采样后的）用于模糊处理
    blitSceneFBO = std::unique_ptr<FrameBuffer>(
            new FrameBuffer(camera->getWidth(),camera->getHeight(),2, true));
    // 纹理1保存当前帧渲染完成后的场景，纹理2保存当前帧的深度可视化（降采样后的），纹理3保存选中的模型的边框颜色（降采样后的）
    lastFrameBuffer = std::unique_ptr<FrameBuffer>(
            new FrameBuffer(camera->getWidth(),camera->getHeight(),3));
    // 仅仅需要一个深度缓冲即可（用作阴影），在绘制场景的时候顺便将深度保存在这里
    depthMapFBO = std::unique_ptr<FrameBuffer>(
            new FrameBuffer(shadowWidth,shadowHeight,0, true, false));
    // 将场景颜色和边框模糊后的颜色融合在一起之后保存下来，供后处理（高斯模糊、锐化等）使用
    sceneAndOutLineColorBufferFBO = std::unique_ptr<FrameBuffer>(
            new FrameBuffer(camera->getWidth(), camera->getHeight(),1));
    // 为了防止截屏的时候被ImGui遮挡住，将上述后处理之后的颜色缓冲保存下来以供截屏时从该缓冲中读取像素数据
    screenShotColorFBO = std::unique_ptr<FrameBuffer>(
            new FrameBuffer(camera->getWidth(), camera->getHeight(),1));


    // 对选中物体的边框进行模糊处理
    glGenFramebuffers(2,pingPongFBO);
    glGenTextures(2,pingPongColorBuffers);
    for (int i = 0; i < 2; ++i) {
        glBindFramebuffer(GL_FRAMEBUFFER,pingPongFBO[i]);
        glBindTexture(GL_TEXTURE_2D,pingPongColorBuffers[i]);
        glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA16F,camera->getWidth(),camera->getHeight(),0,GL_RGBA,GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,pingPongColorBuffers[i],0);
        // also check if framebuffers are complete (no need for depth buffer)
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            Logger::Log<Renderer>(__FUNCTION__, "Framebuffer is not complete!", Logger::LogType::ERROR);
        glBindFramebuffer(GL_FRAMEBUFFER,0);
    }
}

void Renderer::blurOutlineColorBuffer() {
    // gaussian blur
    bool horizontal = true;
    bool first_iteration = true;
    unsigned int amount = 4;
    blurShader.use();
    for (unsigned int i = 0; i < amount; ++i) {
        // horizontal
        glBindFramebuffer(GL_FRAMEBUFFER,pingPongFBO[horizontal]);
        blurShader.setBool("horizontal",horizontal);
        glBindTexture(GL_TEXTURE_2D, first_iteration ? blitSceneFBO->getColorTextureId(1) : pingPongColorBuffers[!horizontal]);
        simpleModel.renderQuad();
        // vertical
        horizontal = !horizontal;
        if (first_iteration)first_iteration = false;
    }
    glBindFramebuffer(GL_FRAMEBUFFER,0);
}

void Renderer::finalColorToBuffer(bool bloom) {

    sceneAndOutLineColorBufferFBO->bind();

    // render the quad
    finalShader.use();
    finalShader.setInt("sceneColor", 0);
    if (bloom){
        finalShader.setBool("bloom", true);
        finalShader.setInt("blurColor",1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, pingPongColorBuffers[0]);          // blur
    }else{
        finalShader.setBool("bloom", false);
    }
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, blitSceneFBO->getColorTextureId());    // scene

    simpleModel.renderQuad();

    sceneAndOutLineColorBufferFBO->unbind();
}

void Renderer::renderToScreenSettings() {
    // render to screen
    glBindFramebuffer(GL_FRAMEBUFFER,0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    // update the viewport
    glViewport(0,0,camera->getWidth(),camera->getHeight());
}

void Renderer::renderWithPostProcess() {

    screenShotColorFBO->bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glViewport(0,0,camera->getWidth(),camera->getHeight());

    postProcessShader.use();
    // process choice
    postProcessShader.setInt("process",static_cast<int>(postProcess));
    switch (postProcess) {

        case POST_PROCESS::NONE:
            break;
        case POST_PROCESS::GAUSSIAN_BLUR:
            postProcessShader.setInt("gaussian_size",gaussianParameter.mSize);
            postProcessShader.setFloat("gaussian_sigma",gaussianParameter.mSigma);
            break;
        case POST_PROCESS::SHARPEN:
            postProcessShader.setInt("sharpen_kernel",sharpenKernelParameter);
            break;
    }
    // texture
    postProcessShader.setInt("color",0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sceneAndOutLineColorBufferFBO->getColorTextureId());
    simpleModel.renderQuad();

    screenShotColorFBO->bind();
}

void Renderer::finalRenderToScreen() {
    renderToScreenSettings();

    quadShader.use();
    quadShader.setInt("quadTexture", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, screenShotColorFBO->getColorTextureId());
    simpleModel.renderQuad();

}

void Renderer::drawPointLights() {
    lightShader.use();
    for (auto & pointLight : pointLights) {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model,pointLight->getLightPosition());
        model = glm::scale(model, glm::vec3(0.2f,0.2f,0.2f));
        lightShader.setMat4("model",model);
        setViewMatrix(lightShader);
        setProjectionMatrix(lightShader);
        lightShader.setVec3("lightColor",pointLight->getLightColor() / 255.0f);
        simpleModel.renderSphere();
    }
}

void Renderer::saveCurrentFrameBuffer() {
    if (drawEnvCubeMap){
        firstFrame = false;
        // 将场景的颜色通道（MSAA）降采样并保存
        BlitFBOToAnotherFBO(sceneMSAAFBO.get(),
                            lastFrameBuffer.get(),
                            GL_COLOR_BUFFER_BIT);

        // 将场景的深度通道复制并保存(处理过的线性深度值并可视化)
        BlitFBOToAnotherFBO(sceneMSAAFBO.get(),
                            lastFrameBuffer.get(),
                            GL_COLOR_BUFFER_BIT,
                            GL_COLOR_ATTACHMENT2,GL_COLOR_ATTACHMENT1);

        // 将选中的模型的边框显示出来
        BlitFBOToAnotherFBO(sceneMSAAFBO.get(),
                            lastFrameBuffer.get(),
                            GL_COLOR_BUFFER_BIT,
                            GL_COLOR_ATTACHMENT1,GL_COLOR_ATTACHMENT2);
    }
}

Renderer::~Renderer() {
    if (hitModel){
//        delete hitModel;      // 防止~Scene中对model的重复释放，这里不delete
        hitModel = nullptr;
    }
}

void Renderer::setShadowMethod(const ShadowMethod &method) {
    this->shadowMethod = method;
}

void Renderer::setLightSpaceMatrix() {
    lightProjection = glm::ortho(-50.f,50.f,-50.f,50.f,camera->getNear(),camera->getFar());
    lightView = glm::lookAt(pointLights[0]->getLightPosition(),camera->getCenter(),camera->getUpDir());
    lightSpaceMatrix = lightProjection * lightView;
    depthShader.setMat4("lightSpaceMatrix",lightSpaceMatrix);
}

void Renderer::setLightsRotation() {
    for (auto & pointLight : pointLights) {
        glm::mat4 rotateMatrix = glm::rotate(glm::mat4 (1.0f), (float )sin(glfwGetTime()) * (float )M_PI,glm::vec3(0.0f, 1.0f, 0.0f));
        glm::vec3 newPos = glm::vec3 (rotateMatrix * glm::vec4 (defaultLightPos,1.0f));
        pointLight->setLightPosition(newPos);
    }
}

void Renderer::setShadowUniforms() {
    sceneShader.use();
    if (shadowMethod != ShadowMethod::None){
        sceneShader.setMat4("lightSpaceMatrix",lightSpaceMatrix);
        sceneShader.setInt("depthMap",9);
        glActiveTexture(GL_TEXTURE9);
        glBindTexture(GL_TEXTURE_2D, depthMapFBO->getDepthTextureId());
    }
    sceneShader.setInt("shadowMethod", static_cast<int>(shadowMethod));
}

void Renderer::drawDepthMapOnly() {
    glViewport(0,0,shadowWidth,shadowHeight);
    glBindFramebuffer(GL_FRAMEBUFFER,depthMapFBO->getFrameBufferId());
    glClear(GL_DEPTH_BUFFER_BIT);
    // lightMatrix uniform
    depthShader.use();
    if (enableRotationPointLight) setLightsRotation(); // rotate the light
    setLightSpaceMatrix();  // set light space matrix
    // render scene
    scene->drawScene(depthShader, true);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::debugDepthMapToScreen() {
    renderToScreenSettings();
    depthDebugShader.use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D,depthMapFBO->getDepthTextureId());
    simpleModel.renderQuad();
}

ShadowMethod Renderer::getShadowMethod() const {
    return shadowMethod;
}

void Renderer::setIBLIndex(int index) {
    iblIndex = index;
}

int Renderer::getIBLIndex() const {
    return iblIndex;
}

void Renderer::saveColorBufferToFile() const {
    TextureTools::writeColorBufferToFile(screenShotColorFBO);
}

