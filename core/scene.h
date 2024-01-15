//
// Created by shiyin on 2023/12/8.
//
#pragma once

#ifndef SIMPLERENDERER_SCENE_H
#define SIMPLERENDERER_SCENE_H

#include "model.h"
#include "pointLight.h"
#include "camera.h"
#include "textureTools.h"
#include "memory"
#include "bvh.h"

class Scene{
public:

    Scene();
    ~Scene();

    void addModel(Model* model);
    void updateSceneBox();
    void drawScene(const Shader& shader, bool isDrawDepthMap);
    void addHdrPath(const std::string &path);
    bool hasHdr()const;
    std::string getHdrPath(int pathIndex = 0) const;
    void buildBvh();
    std::vector<BoundingBox3> getSceneBVHBoundingBox();
    std::shared_ptr<BVH> getBvh()const;
    Model* intersect(const Ray& ray);

    /**
     * load models and set modelMatrix of each
     */
    void initScene();

public:
    std::vector<Model*> models;
    BoundingBox3 sceneBox;
    std::vector<std::string> hdrPaths;
    std::shared_ptr<BVH> bvh;
};

#endif //SIMPLERENDERER_SCENE_H
