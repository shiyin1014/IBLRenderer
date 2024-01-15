//
// Created by shiyin on 2023/12/8.
//
#pragma once

#ifndef SIMPLERENDERER_BVH_H
#define SIMPLERENDERER_BVH_H

#include "bbox.h"
#include "vector"
#include "model.h"
#include "intersection.h"
#include "algorithm"
#include "logger.h"

struct BVHNode{
    BoundingBox3 box;
    BVHNode *left;
    BVHNode *right;
    float area;
    Model* modelPtr;

    BVHNode(){
        box = BoundingBox3();
        left = nullptr;
        right = nullptr;
        modelPtr = nullptr;
        area = 0.0f;
    }
};

class BVH{
public:

    enum class SplitMethod{
        NAIVE,
        SAH
    };

    explicit BVH(std::vector<Model*> models, int maxPrimsInNode = 1, SplitMethod method = SplitMethod::NAIVE);
    ~BVH();
    BVHNode* build(std::vector<Model*> meshes);

    int getNodeNumber()const;
    BVHNode* getRootNode()const;

    Intersection intersect(const Ray& ray);
    Intersection getIntersection(BVHNode* node, const Ray& ray);

    void traverseBvhBox(BVHNode* node, std::vector<BoundingBox3>& boxes);

    std::vector<BoundingBox3> getBVHBoundingBox();


private:

    BVHNode* root;
    int nodeNumber;
    const int maxPrimsInNode;
    const SplitMethod splitMethod;

};


#endif //SIMPLERENDERER_BVH_H
