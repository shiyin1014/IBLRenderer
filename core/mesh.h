//
// Created by shiyin on 2023/12/8.
//
#pragma once

#ifndef SIMPLERENDERER_MESH_H
#define SIMPLERENDERER_MESH_H

#include <string>
#include <vector>
#include "glm/glm.hpp"
#include "shader.h"
#include "bbox.h"
#include "triangle.h"

struct Vertex{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
    glm::vec3 tangent;
    glm::vec3 biTangent;
};

enum class TextureType : int {
    None = -1,
    TextureDiffuse = 1,
    TextureNormal = 2,
    TextureMetalness = 3,
    TextureRoughness = 4,
    TextureAmbientOcclusion = 5,
    TextureEmission = 6
};

struct Texture {
    unsigned int id;
    TextureType type;
    std::string path;
};

class Mesh{
public:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;
    std::vector<Triangle> triangles;
    BoundingBox3 box;
    float totalArea{};

    unsigned int VAO{};

    Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices,
         const std::vector<Texture>& textures, const BoundingBox3& boundingBox3,
         const std::vector<Triangle>& triangleFaces);

    void draw(const Shader& shader);

    Intersection intersect(const Ray& ray);

private:

    unsigned int VBO{}, EBO{};
    void setupMesh();

};

#endif //SIMPLERENDERER_MESH_H
