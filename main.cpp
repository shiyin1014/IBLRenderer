#include "core/glfwwrapper.h"

int main() {

    const int screenWidth = 1600;
    const int screenHeight = 900;

    // create window
    GlfwWrapper wrapper(screenWidth,screenHeight);

    // create a camera
    std::shared_ptr<Camera> camera = std::make_shared<Camera>(glm::vec3(0.f, 0.f, 0.f),
                                                              glm::vec3(0.f, 1.f, 0.f),
                                                              10.f, 0.f,
                                                              glm::radians(0.f),
                                                              screenWidth,screenHeight);
    // create renderer
    std::shared_ptr<Renderer> renderer = std::make_shared<Renderer>(camera);
    // create scene
    std::shared_ptr<Scene> scene = std::make_shared<Scene>();
    // set scene
    renderer->setScene(scene);
    // light
    std::shared_ptr<PointLight> pointLight = std::make_shared<PointLight>(glm::vec3(10.0f, 10.0f, 10.0f));
    renderer->addPointLight(pointLight);

    // run
    wrapper.setRenderer(renderer);
    wrapper.run();


    return 0;
}

