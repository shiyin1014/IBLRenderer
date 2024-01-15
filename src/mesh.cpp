//
// Created by shiyin on 2023/12/8.
//

#include "../core/mesh.h"

Mesh::Mesh(const std::vector<Vertex> &vertices, const std::vector<unsigned int> &indices,
           const std::vector<Texture> &textures, const BoundingBox3& boundingBox3,
           const std::vector<Triangle>& triangleFaces) :
           vertices(vertices), indices(indices), textures(textures), box(boundingBox3), triangles(triangleFaces){
    setupMesh();
    for (auto & triangle : triangles) {
        totalArea += triangle.area;
    }
}

void Mesh::draw(const Shader &shader) {
    unsigned int diffuseNumber = 1;
    unsigned int normalNumber = 1;
    unsigned int metalnessNumber = 1;
    unsigned int roughnessNumber = 1;
    unsigned int ambientOcclusionNumber = 1;
    unsigned int emission = 1;
    shader.use();
    for (int i = 0; i < textures.size(); ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        std::string number;
        TextureType name = textures[i].type;
        switch (name) {
            case TextureType::None:
                break;
            case TextureType::TextureDiffuse:
                number = std::to_string(diffuseNumber++);
                shader.setInt("texture_diffuse_" + number,i);
                break;
            case TextureType::TextureNormal:
                number = std::to_string(normalNumber++);
                shader.setInt("texture_normal_" + number,i);
                break;
            case TextureType::TextureMetalness:
                number = std::to_string(metalnessNumber++);
                shader.setInt("texture_metalness_" + number,i);
                break;
            case TextureType::TextureRoughness:
                number = std::to_string(roughnessNumber++);
                shader.setInt("texture_roughness_" + number,i);
                break;
            case TextureType::TextureAmbientOcclusion:
                number = std::to_string(ambientOcclusionNumber++);
                shader.setInt("texture_AO_" + number,i);
                break;
            case TextureType::TextureEmission:
                number = std::to_string(emission++);
                shader.setInt("texture_emission_" + number,i);
                break;
        }
        glBindTexture(GL_TEXTURE_2D,textures[i].id);
    }

    // draw mesh
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, static_cast<int>(indices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // always good practice to set everything to defaults once configured.
    glActiveTexture(GL_TEXTURE0);
}

void Mesh::setupMesh() {
    glGenVertexArrays(1,&VAO);
    glGenBuffers(1,&VBO);
    glGenBuffers(1,&EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER,VBO);
    glBufferData(GL_ARRAY_BUFFER,vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)offsetof(Vertex, texCoords));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)offsetof(Vertex,tangent));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(4,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)offsetof(Vertex,biTangent));
    glEnableVertexAttribArray(4);

    glBindVertexArray(0);//unbind
}

Intersection Mesh::intersect(const Ray &ray) {
    Intersection intersection;
    for (auto & triangle : triangles) {
        Intersection temp = triangle.intersect(ray);
        if (temp.happened && temp.distance < intersection.distance) intersection = temp;
    }
    return intersection;
}
