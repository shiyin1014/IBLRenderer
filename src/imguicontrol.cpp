//
// Created by shiyin on 2023/12/13.
//


#include "../core/imguicontrol.h"

ImGuiControl::ImGuiControl(std::shared_ptr<UserControl> sharedPtrUserControl,
                           std::shared_ptr<Renderer> sharedPtrRenderer)
    : userControl(std::move(sharedPtrUserControl)),
      renderer(std::move(sharedPtrRenderer)){
}

void ImGuiControl::initImGui(GLFWwindow *window) {
    // setup dear imGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    // load font
    io.Fonts->AddFontDefault(); // 将默认字体先加进去，否则新字体会覆盖旧字体
    ImFont* customFont = io.Fonts->AddFontFromFileTTF("../resource/ttf/Karla-Regular.ttf",18.0f);
    if (!customFont){
        Logger::Log<ImGuiControl>(__FUNCTION__ ,"load font failed.");
    }
}

void ImGuiControl::imGuiFunction(GLFWwindow *window) {
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    const ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse){
        userControl->mouseLeftButtonDown = false;
        userControl->mouseMiddleButtonDown = false;
        userControl->mouseScroll = false;
        userControl->mouseCastRayLeftButton = false;
    }else{
        userControl->mouseScroll = true;
    }
    imGuiPanel(window);
}

void ImGuiControl::shutDown() {
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void ImGuiControl::imGuiPanel(GLFWwindow *window) {
    // 获取窗口大小
    static int displayW, displayH;
    glfwGetFramebufferSize(window,&displayW,&displayH);
    // control part
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::Begin("Control Panel",nullptr);

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                ImGui::GetIO().Framerate);

    // button to save image to file
    static bool clickButton = false;
    if (ImGui::Button("ScreenShot")) {
        clickButton = true;
        renderer->saveColorBufferToFile();
    }

    // box
    static bool drawBox = false;

    // Camera
    if (ImGui::CollapsingHeader("Camera")){
        ImFont* customFont = ImGui::GetIO().Fonts->Fonts[1];
        float customFontSize = 18.0f;
        ImGui::PushFont(customFont);
        ImGui::SetWindowFontScale(customFontSize / ImGui::GetFontSize());
        ImGui::TextColored(ImVec4(1,1,0,1),"Camera");
        ImGui::PopFont();
        ImGui::SetWindowFontScale(1.0f);

        glm::vec3 pos = renderer->getCamera()->getEye();
        ImGui::InputFloat3("Position", glm::value_ptr(pos));
        float cameraRadius = renderer->getCamera()->getRadius();
        float cameraAzimuthAngle = renderer->getCamera()->getAzimuthAngle();
        float cameraPolarAngle = renderer->getCamera()->getPolarAngle();
        ImGui::SliderFloat("Radius",&cameraRadius,renderer->getCamera()->getMinRadius(),renderer->getCamera()->getMaxRadius());
        ImGui::SliderAngle("Azimuth",&cameraAzimuthAngle,renderer->getCamera()->getMinAzimuthAngle(),renderer->getCamera()->getMaxAzimuthAngle());
        ImGui::SliderAngle("Polar",&cameraPolarAngle,renderer->getCamera()->getMinPolarAngle(),renderer->getCamera()->getMaxPolarAngle());
        if (cameraRadius != renderer->getCamera()->getRadius()){
            renderer->getCamera()->setRadius(cameraRadius);
        }
        if (cameraAzimuthAngle != renderer->getCamera()->getAzimuthAngle()){
            renderer->getCamera()->setAzimuth(cameraAzimuthAngle);
        }
        if (cameraPolarAngle != renderer->getCamera()->getPolarAngle()){
            renderer->getCamera()->setPolar(cameraPolarAngle);
        }

        // Reset Camera
        if (ImGui::Button("Reset Camera")){
            Logger::Log<ImGuiControl>(__FUNCTION__ ,"Setting : Reset Camera ");
            renderer->getCamera()->restoreDefaultSetting();
        }

        ImGui::Separator();
    }

    // HDR and Gamma
    if (ImGui::CollapsingHeader("HDR and Gamma")){
        setNewFont("HDR and Gamma");

        bool hdr = renderer->getEnableHdr();
        bool gamma = renderer->getEnableGamma();
        ImGui::Checkbox("Enable HDR", &hdr);
        ImGui::SameLine();
        ImGui::Checkbox("Enable Gamma", &gamma);
        if (hdr!= renderer->getEnableHdr()){
            Logger::Log<ImGuiControl>(__FUNCTION__ ,"Setting : EnableHDR -> " + std::string(hdr ? "true" : "false"));
            renderer->setHdr();
        }
        if (gamma!= renderer->getEnableGamma()){
            Logger::Log<ImGuiControl>(__FUNCTION__ , "Setting : EnableGamma -> " + std::string(gamma ? "true" : "false"));
            renderer->setGamma();
        }

        ImGui::Separator();
    }

    // shadow
    if(ImGui::CollapsingHeader("Shadow")){
        if (!renderer->pointLights.empty()){
            setNewFont("Shadow");
            static int method = static_cast<int>(renderer->getShadowMethod());
            const char* options[] = { "NONE", "ShadowMapping", "PCF","PCSS" };
            for (int i = 0; i < 4; ++i) {
                if (ImGui::RadioButton(options[i], &method,i)){
                    if (method != static_cast<int>(renderer->getShadowMethod())){
                        renderer->setShadowMethod(static_cast<ShadowMethod>(method));
                    }
                }
            }
        }
    }

    // Point Lighting
    if (ImGui::CollapsingHeader("Point Lighting")){
        if (!renderer->pointLights.empty()){
            setNewFont("Point Lighting");
            std::vector<std::shared_ptr<PointLight>> lights = renderer->getPointLights();
            for (int i = 0; i < lights.size(); ++i) {
                // position
                glm::vec3 lightPosition = lights[i]->getLightPosition();
                std::string lightName = "lightPos" + std::to_string(i+1);
                float minPosition = -30.0f;
                float maxPosition = 30.0f;
                ImGui::DragFloat3(lightName.c_str(),glm::value_ptr(lightPosition),0.1f,minPosition,maxPosition);
                if (lightPosition!=lights[i]->getLightPosition()){
                    lights[i]->setLightPosition(lightPosition);
                }
                // color
                glm::vec3 lightColor = lights[i]->getLightColor();
                std::string colorName = "lightColor" + std::to_string(i+1);
                static float color[3] = {lightColor.r / 255.0f, lightColor.g / 255.0f, lightColor.b / 255.0f};
                ImGui::ColorEdit3(colorName.c_str(),color,0);
                lightColor = glm::vec3 (color[0],color[1],color[2]) * 255.0f;
                if (lightColor != lights[i]->getLightColor()){
                    Logger::Log<ImGuiControl>(__FUNCTION__ ," Change lights[" + std::to_string(i)+"].Color = " +
                            "(" + std::to_string(lightColor.x) +
                            ", " + std::to_string(lightColor.y) +
                            ", " + std::to_string(lightColor.z) + ")");
                    lights[i]->setLightColor(lightColor);
                }
            }
            ImGui::Separator();
        }
    }

    // Anti-Aliasing
    if (ImGui::CollapsingHeader("Anti-Aliasing")){
        // MSAA
        setNewFont("Anti-Aliasing");
        bool enableMsaa = renderer->getEnableMsaa();
        ImGui::Checkbox("MSAA",&enableMsaa);
        if (enableMsaa != renderer->getEnableMsaa()){
            Logger::Log<ImGuiControl>(__FUNCTION__ ,
                                      "Setting : EnableMSAA -> " + std::string (enableMsaa ? "true" : "false"));
            renderer->setMsaa();
        }

        ImGui::Separator();
    }

    // IBL
    if (ImGui::CollapsingHeader("IBL")){
        setNewFont("IBL");

        // choose IBL index
        static int iblIndex = renderer->getIBLIndex();
        const char* iblOptions[] = { "outdoor", "colosseum", "mall","thatch" };
        for (int i = 0; i < 4; ++i) {
            if (ImGui::RadioButton(iblOptions[i], &iblIndex,i)){
                if (iblIndex != renderer->getIBLIndex()) renderer->setIBLIndex(iblIndex);
            }
        }

        // show IBL image
        float imageSize = 160.0f;
        static int faceIndexOfEnvCubeFace = 0;
        static int faceIndexOfIrradianceCubeFace = 0;
        static int faceIndexOfPrefilterCubeMapLevel = 0;
        static int faceIndexOfPrefilterCubeMapFace = 0;

        static int showPrefilterLevel = renderer->prefilterCubeMapLevel;
        static int selectedOption = 0;
        const char* options[] = { "showEnvCube", "showIrradiance", "showPrefilter","showBrdfLut" };

        // diffuse
        // load envCubeMapFaces
        std::vector<unsigned int> textureOfEnvCubeFaces = renderer->getTextures2DOfEnvCubeMap();
        ImGui::Image((void*)(intptr_t)textureOfEnvCubeFaces[faceIndexOfEnvCubeFace], ImVec2(imageSize, imageSize));
        // load irradianceMapFaces
        ImGui::SameLine();
        std::vector<unsigned int> textureOfIrradianceCubeMapFaces = renderer->getTextures2DOfIrradianceCubeMap();
        ImGui::Image((void*)(intptr_t)textureOfIrradianceCubeMapFaces[faceIndexOfIrradianceCubeFace], ImVec2(imageSize, imageSize));

        // diffuse
        // choose index of each cubeMap
        ImGui::SetNextItemWidth(100);
        const char* options1[] = { "POSITIVE_X", "NEGATIVE_X", "POSITIVE_Y","NEGATIVE_Y","POSITIVE_Z","NEGATIVE_Z" };
        ImGui::Combo("HDRCubeMap", &faceIndexOfEnvCubeFace, options1, IM_ARRAYSIZE(options1));
        ImGui::SetNextItemWidth(100);
        const char* options2[] = { "POSITIVE_X", "NEGATIVE_X", "POSITIVE_Y","NEGATIVE_Y","POSITIVE_Z","NEGATIVE_Z" };
        ImGui::SameLine();
        ImGui::Combo("Irradiance", &faceIndexOfIrradianceCubeFace, options2, IM_ARRAYSIZE(options2));

        // show RadioButton
        ImGui::RadioButton(options[0], &selectedOption,0);
        if (selectedOption == 0 && !renderer->drawEnvCubeMap){
            renderer->drawEnvCubeMap = true;
            renderer->drawIrradianceCubeMap = false;
            renderer->drawPrefilterCubeMap = false;
            renderer->drawBrdfLutTexture2D = false;
            Logger::Log<ImGuiControl>(__FUNCTION__ , "Select showEnvCube button, drawEnvCubeMap = true.");
        }
        ImGui::SameLine();
        ImGui::Dummy(ImVec2(100,0));
        ImGui::SameLine();
        ImGui::RadioButton(options[1], &selectedOption,1);
        if (selectedOption == 1 && !renderer->drawIrradianceCubeMap){
            renderer->drawIrradianceCubeMap = true;
            renderer->drawEnvCubeMap = false;
            renderer->drawPrefilterCubeMap = false;
            renderer->drawBrdfLutTexture2D = false;
            drawBox = false;
            Logger::Log<ImGuiControl>(__FUNCTION__ , "Select showIrradiance button, drawIrradianceCubeMap = true.");
        }

        // specular
        // load prefilteredMapFaces
        std::vector<std::vector<unsigned int>> textureOfPrefilteredCubeMapFaces = renderer->getTextures2DOfPrefilteredCubeMap();
        ImGui::Image((void*)(intptr_t)textureOfPrefilteredCubeMapFaces
        [faceIndexOfPrefilterCubeMapLevel][faceIndexOfPrefilterCubeMapFace], ImVec2(imageSize, imageSize));
        if (faceIndexOfPrefilterCubeMapLevel != renderer->prefilterCubeMapLevel){
            renderer->prefilterCubeMapLevel = faceIndexOfPrefilterCubeMapLevel;
        }
        // brdf
        ImGui::SameLine();
        unsigned int iblBrdfLutTexture2D = renderer->getIblBrdfLutTexture2D();
        ImGui::Image((void*)(intptr_t)iblBrdfLutTexture2D, ImVec2(imageSize, imageSize),
                     ImVec2(0,1),ImVec2(1,0));

        // specular
        ImGui::SetNextItemWidth(100);
        const char* options3[] = { "0", "0.25", "0.5","0.75","1.0" };
        ImGui::Combo("roughness", &faceIndexOfPrefilterCubeMapLevel, options3, IM_ARRAYSIZE(options3));
        ImGui::SameLine();
        ImGui::Dummy(ImVec2(70,0));
        ImGui::SameLine();
        ImGui::Text("BRDFLut");
        ImGui::SetNextItemWidth(100);
        const char* options4[] = { "POSITIVE_X", "NEGATIVE_X", "POSITIVE_Y","NEGATIVE_Y","POSITIVE_Z","NEGATIVE_Z" };
        ImGui::Combo("faceIndex", &faceIndexOfPrefilterCubeMapFace, options4, IM_ARRAYSIZE(options4));

        // show RadioButton
        ImGui::RadioButton(options[2], &selectedOption,2);
        if (selectedOption == 2 && !renderer->drawPrefilterCubeMap){
            renderer->drawPrefilterCubeMap = true;
            renderer->drawEnvCubeMap = false;
            renderer->drawIrradianceCubeMap = false;
            renderer->drawBrdfLutTexture2D = false;
            drawBox = false;
            Logger::Log<ImGuiControl>(__FUNCTION__ , "Select showPrefilter button, drawPrefilterCubeMap = true.");
        }
        ImGui::SameLine();
        ImGui::Dummy(ImVec2(90,0));
        ImGui::SameLine();
        ImGui::RadioButton(options[3], &selectedOption,3);
        if (selectedOption == 3 && !renderer->drawBrdfLutTexture2D){
            renderer->drawBrdfLutTexture2D = true;
            renderer->drawPrefilterCubeMap = false;
            renderer->drawEnvCubeMap = false;
            renderer->drawIrradianceCubeMap = false;
            drawBox = false;
            Logger::Log<ImGuiControl>(__FUNCTION__ , "Select showBrdfLut button, drawBrdfLutTexture2D = true.");
        }

        ImGui::Separator();
    }

    // realtime-FBO
    if (ImGui::CollapsingHeader("FrameBuffer") && renderer->drawEnvCubeMap){
        if (!renderer->firstFrame){
            setNewFont("Scene FBO");

            // colorBuffer
            unsigned int colorFrame = renderer->lastFrameBuffer->getColorTextureId();
            ImGui::Image(reinterpret_cast<ImTextureID>(colorFrame),ImVec2(160,90),
                         ImVec2(0,1),ImVec2(1,0));
            ImGui::SameLine();

            // depthBuffer
            unsigned int depthFrame = renderer->lastFrameBuffer->getColorTextureId(1);
            ImGui::Image(reinterpret_cast<ImTextureID>(depthFrame), ImVec2(160,90),
                         ImVec2(0,1),ImVec2(1,0));

            // text description
            ImGui::Text("     ColorTexture");
            ImGui::SameLine();
            ImGui::Dummy(ImVec2(90,0));
            ImGui::SameLine();
            ImGui::Text("DepthBuffer");

            // outlineTexture
            unsigned int outlineFrame = renderer->lastFrameBuffer->getColorTextureId(2);
            ImGui::Image(reinterpret_cast<ImTextureID>(outlineFrame), ImVec2(160,90),
                         ImVec2(0,1),ImVec2(1,0));

            // text description
            ImGui::Text("   OutLineTexture");
        }
    }

    // boundingBox
    if (ImGui::CollapsingHeader("Hierarchy")){
        setNewFont("Hierarchy");

        // draw Scene Box
        ImGui::Checkbox("Draw BVH Box Of Scene",&drawBox);
        if (drawBox != renderer->drawSceneBox){
            renderer->drawSceneBox = drawBox;
            // set showBoundingBox of each model true when drawSceneBox is true, draw each box and draw scene box
            if (!renderer->drawSceneBox){
                for (int i = 0; i < renderer->getScene()->models.size(); ++i) {
                    renderer->getScene()->models[i]->showBoundingBox = false;
                }
            }
        }

        // line color
        glm::vec3 lineColor = renderer->boxLineColor;
        static float color[3] = {lineColor.r, lineColor.g, lineColor.b};
        ImGui::ColorEdit3("Box Line Color",color,0);
        glm::vec3 newLineColor(color[0],color[1],color[2]);
        if (newLineColor != lineColor){
            Logger::Log<ImGuiControl>(__FUNCTION__ ," Change lineColor = (" + std::to_string(newLineColor.x) +
                                                    ", " + std::to_string(newLineColor.y) +
                                                    ", " + std::to_string(newLineColor.z) + ")");
            renderer->boxLineColor = glm::vec3(color[0],color[1],color[2]);
        }


        // traverse root of the bvh and show information of each leaf node
        BVHNode* root = renderer->getScene()->getBvh()->getRootNode();
        displayBvhTree(root);

        ImGui::Separator();
    }

    // CastRay
    if (ImGui::CollapsingHeader("CastRay")){
        Model* hitModel = nullptr;
        static bool selectModel = renderer->enableSelect;
        ImGui::Checkbox("Enable CastRay",&selectModel);
        if (selectModel != renderer->enableSelect){
            Logger::Log<ImGuiControl>(__FUNCTION__ ,"Setting : Enable CastRay -> " + std::to_string(selectModel));
            renderer->enableSelect = selectModel;
        }
        if (selectModel && userControl->mouseCastRayLeftButton){
            double windowX, windowY;
            float windowZ;
            glfwGetCursorPos(window,&windowX,&windowY);     // 获取屏幕坐标
            glBindFramebuffer(GL_READ_FRAMEBUFFER, renderer->blitSceneFBO->getDepthTextureId());
            glReadPixels((int )windowX,displayH - (int)windowY,1,1,GL_DEPTH_COMPONENT,GL_FLOAT,&windowZ);
            MouseCastRay cast(renderer->getCamera(),0,0,displayW,displayH);
            glm::vec3 endPoint = cast.getWorldPosFromScreenCoords(glm::vec3(windowX,windowY,windowZ));
            if (endPoint != glm::vec3()){
                renderer->addCastedRay(endPoint);
                Ray castRay = cast.getRay(endPoint);
                if (castRay.origin != glm::vec3() && castRay.direction != glm::vec3()){
                    hitModel = renderer->selectModel(castRay);
                    renderer->hitModel = hitModel;
                    for (auto &model : renderer->getScene()->models) model->highlightEnable = (model == hitModel);
                }
            }else{
                renderer->hitModel = nullptr;
                for (auto &model : renderer->getScene()->models) model->highlightEnable = false;
            }
            userControl->mouseCastRayLeftButton = false;
        }
        if (!selectModel){
            if (renderer->hitModel) renderer->hitModel = nullptr;
            for (auto &model : renderer->getScene()->models) model->highlightEnable = false;
        }

        // show information of the selected model
        if(selectModel && renderer->hitModel){
            setNewFont("The information of the selected model.");
            ImGui::Text("Number of Vertex = %d", renderer->hitModel->vertexNumber);
            ImGui::Text("Number of Face = %d", renderer->hitModel->faceNumber);

            // visible
            static bool visible = renderer->hitModel->visible;
            ImGui::Checkbox("Visible",&visible);
            if (visible != renderer->hitModel->visible)renderer->hitModel->visible = visible;

            ImGui::SameLine();
            // show bbox
            static bool showBox = renderer->hitModel->showBoundingBox;
            ImGui::Checkbox("BoundingBox",&showBox);
            if (showBox != renderer->hitModel->showBoundingBox){
                renderer->hitModel->showBoundingBox = showBox;
            }
            ImGui::Separator();
        }

        // show rays
        static bool showCastRay = renderer->showCastedRays;
        ImGui::Checkbox("Show CastRay",&showCastRay);
        if (showCastRay != renderer->showCastedRays)renderer->showCastedRays = showCastRay;
        if (showCastRay){
            // pop rays
            auto numberRays = renderer->simpleModel.linesVector.size();
            if (numberRays > 0){

                // color of the ray
                glm::vec3 rayColor = renderer->castedRayColor;
                ImGui::ColorEdit3("rayColor",glm::value_ptr(rayColor));
                if (rayColor != renderer->castedRayColor){
                    Logger::Log<ImGuiControl>(__FUNCTION__ ," Change rayColor = (" + std::to_string(rayColor.x) +
                                                            ", " + std::to_string(rayColor.y) +
                                                            ", " + std::to_string(rayColor.z) + ")");
                    renderer->castedRayColor = rayColor;
                }

                // remove the castedRay
                if (ImGui::Button("Remove Last Ray")){
                    Logger::Log<ImGuiControl>(__FUNCTION__ ,"Remove Last Ray, remaining number = " + std::to_string(numberRays));
                    renderer->simpleModel.linesVector.pop_back();
                    numberRays--;
                }
                ImGui::SameLine();
                if (ImGui::Button("Clear")){
                    Logger::Log<ImGuiControl>(__FUNCTION__ ,"Clear All Rays.");
                    renderer->simpleModel.linesVector.clear();
                    numberRays = 0;
                }
                ImGui::Text("Total rays = %d",numberRays);
            }
        }
        ImGui::Separator();
    }

    // post-processing
    if (ImGui::CollapsingHeader("PostProcess")){
        static int processIndex = static_cast<int>(renderer->postProcess);
        const char* options[] = { "None", "Gaussian_Blur", "Sharpen","PCSS" };
        // None
        if (ImGui::RadioButton(options[0], &processIndex,0)){
            if (processIndex != static_cast<int>(renderer->postProcess)){
                renderer->postProcess = static_cast<POST_PROCESS>(processIndex);
            }
        }

        // Gaussian_blur
        ImGui::RadioButton(options[1], &processIndex,1);
        if (processIndex == 1){
            if ( processIndex != static_cast<int>(renderer->postProcess)){
                renderer->postProcess = static_cast<POST_PROCESS>(processIndex);
            }
            // mSize and mSigma
            static int kernelSize = renderer->gaussianParameter.mSize;
            static float sigma = renderer->gaussianParameter.mSigma;
            ImGui::DragInt("kernelSize",&kernelSize,2,1,21);
            if (kernelSize != renderer->gaussianParameter.mSize){
                renderer->gaussianParameter.mSize = kernelSize;
            }
            ImGui::DragFloat("sigma",&sigma,1,0.1f,10.0f);
            if (sigma != renderer->gaussianParameter.mSigma){
                renderer->gaussianParameter.mSigma = sigma;
            }
        }

        // sharpen
        ImGui::RadioButton(options[2], &processIndex,2);
        if(processIndex == 2){
            if ( processIndex != static_cast<int>(renderer->postProcess)){
                renderer->postProcess = static_cast<POST_PROCESS>(processIndex);
            }

            static int matrix1[3][6] = {
                    {0,-1,0,-1,-1,-1},
                    {-1,5,-1,-1,9,-1},
                    {0,-1,0,-1,-1,-1}
            };
            for (int row =0; row < 3; row++)
            {
                for (int col = 0; col < 6; col++)
                {
                    if ((row == 0 || row == 1 || row == 2) && col == 0){
                        ImGui::Dummy(ImVec2(10,0));
                        ImGui::SameLine();
                    }
                    setNewFont(std::to_string(matrix1[row][col]));
                    ImGui::SameLine();
                    if (col == 2 ){
                        ImGui::Dummy(ImVec2(80,0));
                        ImGui::SameLine();
                    }
                }
                ImGui::NewLine();
            }

            const char* kernelOptions[2] = {"kernel_1","kernel_2"};
            static int sharpenKernel = renderer->sharpenKernelParameter;
            ImGui::RadioButton(kernelOptions[0], &sharpenKernel,5);
            if(sharpenKernel == 5){
                if (renderer->sharpenKernelParameter != sharpenKernel){
                    renderer->sharpenKernelParameter = sharpenKernel;
                }
            }
            ImGui::SameLine();
            ImGui::Dummy(ImVec2(50,0));
            ImGui::SameLine();
            ImGui::RadioButton(kernelOptions[1], &sharpenKernel,9);
            if(sharpenKernel == 9){
                if (renderer->sharpenKernelParameter != sharpenKernel){
                    renderer->sharpenKernelParameter = sharpenKernel;
                }
            }
        }

        ImGui::Separator();
    }


    ImGui::End();
    ImGui::Render();
}

void ImGuiControl::setNewFont(const std::string &str) {
    ImFont* customFont = ImGui::GetIO().Fonts->Fonts[1];
    static float customFontSize = 18.0f;
    ImGui::PushFont(customFont);
    ImGui::SetWindowFontScale(customFontSize / ImGui::GetFontSize());
    ImGui::TextColored(ImVec4(1,1,0,1),str.c_str());
    ImGui::PopFont();
    ImGui::SetWindowFontScale(1.0f);
}

void ImGuiControl::displayBvhTree(const BVHNode *node) {
    if (!node)return;
    bool nodeOpen = ImGui::TreeNode(node,"Node: %s",node->modelPtr ? node->modelPtr->modelName.c_str() : "non-leaf-node");
    if (nodeOpen){
        if (node->left == nullptr && node->right == nullptr){   // left-node
            // node information
            setNewFont(node->modelPtr->modelName);
            ImGui::Text("Number of Vertex : %d", node->modelPtr->vertexNumber);
            ImGui::Text("Number of Face : %d", node->modelPtr->faceNumber);

            // visible
            bool visible = node->modelPtr->visible;
            ImGui::Checkbox("Visible",&visible);
            if (visible != node->modelPtr->visible) node->modelPtr->visible = visible;

            // show bbox
            bool showBox = node->modelPtr->showBoundingBox;
            ImGui::Checkbox("Draw BoundingBox",&showBox);
            if (showBox != node->modelPtr->showBoundingBox) node->modelPtr->showBoundingBox = showBox;
            if (!visible) node->modelPtr->showBoundingBox = false;

            // show shadow
            bool showShadow = node->modelPtr->hasShadow;
            ImGui::Checkbox("Has Shadow", &showShadow);
            if (showShadow != node->modelPtr->hasShadow) node->modelPtr->hasShadow = showShadow;
            if (!visible) node->modelPtr->hasShadow = false;

        }else{      // non leaf-node
            // traverse left and right node
            if (node->left) displayBvhTree(node->left);
            if (node->right) displayBvhTree(node->right);
        }
        ImGui::TreePop();
    }
}
