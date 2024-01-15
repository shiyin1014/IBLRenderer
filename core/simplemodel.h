//
// Created by shiyin on 2023/12/10.
//

#pragma once

#ifndef SIMPLERENDERER_SIMPLEMODEL_H
#define SIMPLERENDERER_SIMPLEMODEL_H

#include "glad/glad.h"
#include "glm/glm.hpp"
#include "bbox.h"
#include <vector>
#include <valarray>

class SimpleModel {

public:

    SimpleModel();

    void renderCube();
    void renderQuad();
    void renderSphere();
    void writeSphereObjToFile(const std::string & path);
    void renderBbox(const BoundingBox3& boundingBox3);
    void initializeBbox();
    void renderRayLines();
    void addRayVertices(const glm::vec3& startPoint, const glm::vec3& endPoint);

    ~SimpleModel();

private:
    unsigned int cubeVAO;
    unsigned int cubeVBO;

    unsigned int quadVAO;
    unsigned int quadVBO;

    unsigned int sphereVAO;
    unsigned int sphere_index_count;

    BoundingBox3 preBox;
    unsigned int bBoxVAO;
    unsigned int bBoxVBO;

public:
    std::vector<unsigned int> linesVector;

};

#endif //SIMPLERENDERER_SIMPLEMODEL_H
