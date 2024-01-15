//
// Created by shiyin on 2023/12/8.
//

#include "../core/scene.h"

Scene::Scene() : sceneBox() {
    models.clear();
    hdrPaths.clear();
    initScene();
    buildBvh();
}

void Scene::addModel(Model *model) {
    model->updateBox();
    models.push_back(model);
    if (model->enableBox) sceneBox = Union(sceneBox,model->box);
}

void Scene::updateSceneBox() {
    sceneBox = BoundingBox3();
    for (auto & model : models) {
        if (model->visible && model->enableBox) sceneBox = Union(sceneBox,model->box);
    }
}

void Scene::initScene() {
    // load model
    std::string rootPath = "../resource/";
    auto *helmet = new Model(rootPath + "objects/helmet/helmet.obj");
    helmet->modelName = "helmet";
    // model matrix of the helmet
    glm::mat4 translation(glm::translate (glm::mat4(1.0f),glm::vec3(-3.5f,-0.25f,0.0f)));
    glm::mat4 rotate(glm::rotate(glm::mat4(1.0f),glm::radians(45.0f),glm::vec3(1,0,0)));
    glm::mat4 scale(glm::scale(glm::mat4(1.0f),glm::vec3(1.2f,1.2f,1.2f)));
    helmet->setScaleMatrix(scale);
    helmet->setTranslateMatrix(translation);
    helmet->setRotateMatrix(rotate);

    // note that our obj file does not have the -mtl parameter, so we need to load texture ourselves
    // textures of above obj
    std::vector<Texture> helmetTextures;
    helmetTextures.push_back(Texture{
            TextureTools::loadTexture2D(rootPath + "objects/helmet/helmet_basecolor.tga"),
            TextureType::TextureDiffuse, ""});
    helmetTextures.push_back(Texture{TextureTools::loadTexture2D(rootPath + "objects/helmet/helmet_normal.tga"),
                                     TextureType::TextureNormal, ""});
    helmetTextures.push_back(Texture{
            TextureTools::loadTexture2D(rootPath + "objects/helmet/helmet_metalness.tga"),
            TextureType::TextureMetalness, ""});
    helmetTextures.push_back(Texture{
            TextureTools::loadTexture2D(rootPath + "objects/helmet/helmet_roughness.tga"),
            TextureType::TextureRoughness, ""});
    helmetTextures.push_back(Texture{
            TextureTools::loadTexture2D(rootPath + "objects/helmet/helmet_occlusion.tga"),
            TextureType::TextureAmbientOcclusion, ""});
    helmetTextures.push_back(Texture{
            TextureTools::loadTexture2D(rootPath + "objects/helmet/helmet_emission.tga"),
            TextureType::TextureEmission, ""});
    helmet->meshes[0].textures = helmetTextures;

    // add model to scene
    addModel(helmet);


    // load pony car
    auto* car_body = new Model(rootPath + "objects/ponycar/body/body.obj");
    car_body->modelName = "car_body";
    glm::mat4 car_scale(glm::scale(glm::mat4 (1.0f),glm::vec3(0.005,0.005,0.005)));
    glm::mat4 car_trans(glm::translate(glm::mat4(1.0f),glm::vec3(3.5f,-1.4f,0.0f)));
    glm::mat4 car_rotation(glm::rotate(glm::mat4(1.0f),glm::radians(-90.0f),glm::vec3(1,0,0)));
    car_body->setScaleMatrix(car_scale);
    car_body->setRotateMatrix(car_rotation);
    car_body->setTranslateMatrix(car_trans);

    std::vector<Texture> bodyTextures;
    bodyTextures.push_back(Texture{TextureTools::loadTexture2D(rootPath + "objects/ponycar/body/body_basecolor.tga"),
                                   TextureType::TextureDiffuse, ""});
    bodyTextures.push_back(Texture{TextureTools::loadTexture2D(rootPath + "objects/ponycar/body/body_normal.tga"),
                                   TextureType::TextureNormal, ""});
    bodyTextures.push_back(Texture{
            TextureTools::loadTexture2D(rootPath + "objects/ponycar/body/body_metalness.tga"),
            TextureType::TextureMetalness, ""});
    bodyTextures.push_back(Texture{
            TextureTools::loadTexture2D(rootPath + "objects/ponycar/body/body_roughness.tga"),
            TextureType::TextureRoughness, ""});
    bodyTextures.push_back(Texture{
            TextureTools::loadTexture2D(rootPath + "objects/ponycar/body/body_occlusion.jpg"),
            TextureType::TextureAmbientOcclusion, ""});
    bodyTextures.push_back(Texture{
            TextureTools::loadTexture2D(rootPath + "objects/ponycar/body/body_emission.tga"),
            TextureType::TextureEmission, ""});
    car_body->meshes[0].textures = bodyTextures;
    addModel(car_body);

    // load car interior
    auto* car_interior = new Model(rootPath + "objects/ponycar/interior/interior.obj");
    car_interior->modelName = "car_interior";
    car_interior->setScaleMatrix(car_scale);
    car_interior->setRotateMatrix(car_rotation);
    car_interior->setTranslateMatrix(car_trans);

    // textures
    std::vector<Texture> interiorTextures;
    interiorTextures.push_back(Texture{TextureTools::loadTexture2D(rootPath + "objects/ponycar/interior/interior_basecolor.tga"),
                                       TextureType::TextureDiffuse, ""});
    interiorTextures.push_back(Texture{TextureTools::loadTexture2D(rootPath + "objects/ponycar/interior/interior_normal.tga"),
                                       TextureType::TextureNormal, ""});
    interiorTextures.push_back(Texture{
            TextureTools::loadTexture2D(rootPath + "objects/ponycar/interior/interior_metalness.tga"),
            TextureType::TextureMetalness, ""});
    interiorTextures.push_back(Texture{
            TextureTools::loadTexture2D(rootPath + "objects/ponycar/interior/interior_roughness.tga"),
            TextureType::TextureRoughness, ""});
    interiorTextures.push_back(Texture{
            TextureTools::loadTexture2D(rootPath + "objects/ponycar/interior/interior_occlusion.jpg"),
            TextureType::TextureAmbientOcclusion, ""});
    interiorTextures.push_back(Texture{
            TextureTools::loadTexture2D(rootPath + "objects/ponycar/interior/interior_emission.png"),
            TextureType::TextureEmission, ""});
    car_interior->meshes[0].textures = interiorTextures;
    addModel(car_interior);


    // add a plane to scene
    auto* floor = new Model(rootPath + "objects/floor/floor.obj");
    glm::mat4 floorScale(glm::scale(glm::mat4(1.0f),glm::vec3(0.3f,0.3f,0.3f)));
    glm::mat4 floorTrans(glm::translate(glm::mat4(1.0f),glm::vec3(0.f,-1.5f,0.f)));
    floor->setTranslateMatrix(floorTrans);
    floor->setScaleMatrix(floorScale);
    floor->modelName = "floor";
    // load textures
    std::vector<Texture> floorTextures;
    floorTextures.push_back(Texture{TextureTools::loadTexture2D(rootPath + "objects/floor/floor_diffuse.jpg"),
                                    TextureType::TextureDiffuse, ""});
    floorTextures.push_back(Texture{TextureTools::loadTexture2D(rootPath + "objects/floor/floor_normal.jpg"),
                                    TextureType::TextureNormal, ""});
    floorTextures.push_back(Texture{
            TextureTools::loadTexture2D(rootPath + "objects/floor/floor_metalness.png"),
            TextureType::TextureMetalness, ""});
    floorTextures.push_back(Texture{
            TextureTools::loadTexture2D(rootPath + "objects/floor/floor_roughness.jpg"),
            TextureType::TextureRoughness, ""});
    floorTextures.push_back(Texture{
            TextureTools::loadTexture2D(rootPath + "objects/floor/floor_occlusion.jpg"),
            TextureType::TextureAmbientOcclusion, ""});
    floorTextures.push_back(Texture{
            TextureTools::loadTexture2D(rootPath + "objects/floor/floor_emission.png"),
            TextureType::TextureEmission, ""});
    floor->meshes[0].textures = floorTextures;
    addModel(floor);


    // add a sphere model to show HDR
    auto* sphereModel = new Model(rootPath + "objects/sphere/sphere.obj");
    sphereModel->modelName = "sphere";
    glm::mat4 sphereScale = glm::scale(glm::mat4 (1.0f),glm::vec3(1.5f,1.5f,1.5f));
//    glm::mat4 sphereTranslation = glm::translate(glm::mat4(1.0f),glm::vec3 (0.f,0.f,0.f));
    sphereModel->setScaleMatrix(sphereScale);
//    sphereModel->setTranslateMatrix(sphereTranslation);
    // sphere textures
    std::vector<Texture> sphereTextures;
    sphereTextures.push_back(Texture{TextureTools::loadTexture2D(rootPath + "objects/sphere/sphere_diffuse.png"),
                                     TextureType::TextureDiffuse, ""});
    sphereTextures.push_back(Texture{TextureTools::loadTexture2D(rootPath + "objects/sphere/sphere_normal.jpg"),
                                     TextureType::TextureNormal, ""});
    sphereTextures.push_back(Texture{
            TextureTools::loadTexture2D(rootPath + "objects/sphere/sphere_metalness.jpg"),
            TextureType::TextureMetalness, ""});
    sphereTextures.push_back(Texture{
            TextureTools::loadTexture2D(rootPath + "objects/sphere/sphere_roughness.png"),
            TextureType::TextureRoughness, ""});
    sphereTextures.push_back(Texture{
            TextureTools::loadTexture2D(rootPath + "objects/sphere/sphere_occlusion.jpg"),
            TextureType::TextureAmbientOcclusion, ""});
    sphereTextures.push_back(Texture{
            TextureTools::loadTexture2D(rootPath + "objects/sphere/sphere_emission.png"),
            TextureType::TextureEmission, ""});
    sphereModel->meshes[0].textures = sphereTextures;
    addModel(sphereModel);


    // HDR
    addHdrPath("../resource/hdr/outdoor.hdr");
    addHdrPath("../resource/hdr/colosseum.hdr");
    addHdrPath("../resource/hdr/mall.hdr");
    addHdrPath("../resource/hdr/thatch.hdr");

}

void Scene::drawScene(const Shader& shader, bool isDrawDepthMap) {
    for (auto & model : models) {
        if (model->highlightEnable && !isDrawDepthMap){
            glEnable(GL_STENCIL_TEST);
            glStencilOp(GL_KEEP,GL_KEEP,GL_REPLACE);
            glStencilFunc(GL_ALWAYS,1,0xFF);
        }
        model->setModelMatrix(shader);
        model->draw(shader);
        if (model->highlightEnable && !isDrawDepthMap){
            // 这个绘制完成之后关闭模板测试的写操作，防止其他物体也修改了模板缓冲
            glStencilMask(0x00);
        }
    }
}

void Scene::addHdrPath(const std::string &path) {
    hdrPaths.push_back(path);
}

bool Scene::hasHdr() const {
    return !hdrPaths.empty();
}

std::string Scene::getHdrPath(int pathIndex) const {
    return hdrPaths[pathIndex];
}

Scene::~Scene() {
    if (!models.empty()){
        for (auto & model : models) {
            delete model;
            model = nullptr;
        }
    }
}

void Scene::buildBvh() {
    Logger::Log<Scene>(__FUNCTION__ ,"---------------------BVH GENERATION--------------------");
    Logger::Log<Scene>(__FUNCTION__ ,"Generating BVH...");
    // the floor and the wall are not include
    std::vector<Model*> vectorModel;
    for (auto & model : models) {
        if (model->enableBox) vectorModel.push_back(model);
    }
    this->bvh = std::make_shared<BVH>(vectorModel);
}

std::vector<BoundingBox3> Scene::getSceneBVHBoundingBox() {
    return bvh->getBVHBoundingBox();
}

std::shared_ptr<BVH> Scene::getBvh() const {
    return bvh;
}

Model *Scene::intersect(const Ray& ray) {
    Intersection intersection = bvh->intersect(ray);
    if (intersection.happened) return intersection.model;
    return nullptr;
}
