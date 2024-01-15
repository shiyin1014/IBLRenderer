//
// Created by shiyin on 2023/12/8.
//
#pragma once

#ifndef SIMPLERENDERER_MODEL_H
#define SIMPLERENDERER_MODEL_H

#include "stb_image.h"
#include "vector"
#include "mesh.h"
#include "logger.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "simplemodel.h"

class Model{
public:

    // model data
    std::vector<Texture> textures_loaded;
    std::vector<Mesh> meshes;   // in our model, there is only one Mesh in one's model.
    std::string directory;
    BoundingBox3 box;
    std::string modelName;
    bool visible;
    bool highlightEnable;
    bool showBoundingBox;
    bool hasShadow;
    bool enableBox;

    unsigned int vertexNumber; // record the total vertex number
    unsigned int faceNumber; // record the total face number

    explicit Model(const std::string& path);

    ~Model();

    void draw(const Shader& shader);
    void drawBox(SimpleModel &simpleModel) const;

    void updateBox();
    void setScaleMatrix(const glm::mat4& matrix);
    void setRotateMatrix(const glm::mat4& matrix);
    void setTranslateMatrix(const glm::mat4& matrix);
    void setModelMatrix(const Shader &shader);
    glm::mat4 getModelMatrix();

    const glm::mat4 &getScale() const;

    const glm::mat4 &getRotate() const;

    const glm::mat4 &getTranslate() const;

    Intersection intersect(const Ray& ray);
    void restoreBoundingBox(const glm::mat4& invMatrix);

private:
    // matrix
    glm::mat4 modelMatrix;
    glm::mat4 scale;
    glm::mat4 rotate;
    glm::mat4 translate;

    void loadModel(const std::string& path);
    void processNode(aiNode *node, const aiScene *scene);
    Mesh processMesh(aiMesh* mesh, const aiScene* scene);
    std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, const TextureType& typeName);
    static unsigned int textureFromFile(const char* path, const std::string& directory);
};

#endif //SIMPLERENDERER_MODEL_H
