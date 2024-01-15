//
// Created by shiyin on 2023/12/8.
//

#include "../core/model.h"

Model::Model(const std::string &path): modelMatrix(glm::mat4(1.0f)), scale(glm::mat4(1.0f)), visible(true), showBoundingBox(false),
                                       rotate(glm::mat4 (1.0f)), translate(glm::mat4 (1.0f)) , highlightEnable(false),
                                       enableBox(true), hasShadow(true) {
    this->vertexNumber = 0;
    this->faceNumber = 0;

    Logger::Log<Model>(__FUNCTION__ ,"---------------------LOADING MODEL---------------------");
    Logger::Log<Model>(__FUNCTION__ ,"Loading model : " + path);

    loadModel(path);

    // calculate boundingBox of the model
    for (auto & mesh : meshes) {
        box = Union(box,mesh.box);
    }

    Logger::Log<Model>(__FUNCTION__ ,"Load finished.");
    Logger::Log<Model>(__FUNCTION__ ,"\tvertex_number = " + std::to_string(vertexNumber));
    Logger::Log<Model>(__FUNCTION__ ,"\tface_number = " + std::to_string(faceNumber));
    Logger::Log<Model>(__FUNCTION__ ,"\tmesh_number = " + std::to_string(meshes.size()));
    Logger::Log<Model>(__FUNCTION__ ,"---------------------LOADING MODEL---------------------");
}

void Model::draw(const Shader& shader) {
    if (visible){
        shader.use();
        shader.setBool("hasShadow",hasShadow);
        for (auto & mesh : meshes) {
            mesh.draw(shader);
        }
    }
}

void Model::drawBox(SimpleModel &simpleModel) const {
    if (showBoundingBox && enableBox){
        simpleModel.renderBbox(box);
    }
}

void Model::loadModel(const std::string& path) {
    Assimp::Importer import;
    const aiScene* scene = import.ReadFile(path,
                                           aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        Logger::Log<Model>(__FUNCTION__ ,"ASSIMP::" + std::string(import.GetErrorString()),Logger::LogType::ERROR);
        return;
    }

    directory = path.substr(0,path.find_last_of('/'));

    processNode(scene->mRootNode,scene);
}

void Model::processNode(aiNode *node, const aiScene *scene) {
    for (int i = 0; i < node->mNumMeshes; ++i) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh,scene));
    }
    for (int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i],scene);
    }
}

Mesh Model::processMesh(aiMesh *mesh, const aiScene *scene) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;
    std::vector<Triangle> triangles;
    BoundingBox3 boundingBox3;

    this->vertexNumber += mesh->mNumVertices;
    this->faceNumber += mesh->mNumFaces;

    // deal with vertices
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex{};

        //position
        glm::vec3 vector;
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.position = vector;

        // bbox
        boundingBox3 = Union(boundingBox3,vector);

        //normal
        vector.x = mesh->mNormals[i].x;
        vector.y = mesh->mNormals[i].y;
        vector.z = mesh->mNormals[i].z;
        vertex.normal = vector;

        //tangent
        vector.x = mesh->mTangents[i].x;
        vector.y = mesh->mTangents[i].y;
        vector.z = mesh->mTangents[i].z;
        vertex.tangent = vector;

        // uv coordinates
        if (mesh->mTextureCoords[0]) {
            glm::vec2 uv;
            uv.x = mesh->mTextureCoords[0][i].x;
            uv.y = mesh->mTextureCoords[0][i].y;
            vertex.texCoords = uv;
        }
        vertices.push_back(vertex);
    }

    // indices and faceTriangle
    for (int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];

        // indices
        for (int j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        }

        // triangles
        // position
        glm::vec3 v0(mesh->mVertices[face.mIndices[0]].x,mesh->mVertices[face.mIndices[0]].y,mesh->mVertices[face.mIndices[0]].z);
        glm::vec3 v1(mesh->mVertices[face.mIndices[1]].x,mesh->mVertices[face.mIndices[1]].y,mesh->mVertices[face.mIndices[1]].z);
        glm::vec3 v2(mesh->mVertices[face.mIndices[2]].x,mesh->mVertices[face.mIndices[2]].y,mesh->mVertices[face.mIndices[2]].z);
        // area
        glm::vec3 edge1 = v1 - v0;
        glm::vec3 edge2 = v2 - v0;
        float area = glm::length(glm::cross(edge1,edge2) * 0.5f);

        Triangle triangle(v0,v1,v2,area);
        triangles.push_back(triangle);
    }

    // textures
    if (mesh->mMaterialIndex >= 0) {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        // albedo
        std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, TextureType::TextureDiffuse);
        textures.insert(textures.end(),diffuseMaps.begin(),diffuseMaps.end());
        // normal
        std::vector<Texture> normalMaps = loadMaterialTextures(material,aiTextureType_NORMALS, TextureType::TextureNormal);
        textures.insert(textures.end(),normalMaps.begin(),normalMaps.end());
        // metalness
        std::vector<Texture> metalnessMaps = loadMaterialTextures(material, aiTextureType_METALNESS, TextureType::TextureMetalness);
        textures.insert(textures.end(), metalnessMaps.begin(), metalnessMaps.end());
        // roughness
        std::vector<Texture> roughnessMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE_ROUGHNESS, TextureType::TextureRoughness);
        textures.insert(textures.end(), roughnessMaps.begin(), roughnessMaps.end());
        // ao
        std::vector<Texture> ambientOcclusionMaps = loadMaterialTextures(material, aiTextureType_AMBIENT_OCCLUSION, TextureType::TextureAmbientOcclusion);
        textures.insert(textures.end(), ambientOcclusionMaps.begin(), ambientOcclusionMaps.end());
        // emission
        std::vector<Texture> emissionMaps = loadMaterialTextures(material,aiTextureType_EMISSION_COLOR, TextureType::TextureEmission);
        textures.insert(textures.end(), emissionMaps.begin(), emissionMaps.end());
    }

    return {vertices,indices,textures, boundingBox3, triangles};
}

// checks all material textures of a given type and loads the textures if they're not loaded yet.
std::vector<Texture> Model::loadMaterialTextures(aiMaterial *mat, aiTextureType type, const TextureType& typeName) {
    std::vector<Texture> textures;
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
        aiString str;
        mat->GetTexture(type,i,&str);
        //check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
        bool skip = false;
        for (auto & j : textures_loaded) {
            if (std::strcmp(j.path.data(), str.C_Str()) == 0) {
                textures.push_back(j);
                skip = true;
                break;
            }
        }
        if (!skip) {
            // if texture hasn't been loaded already, load it
            Texture texture;
            texture.id = textureFromFile(str.C_Str(), directory);
            texture.type = typeName;
            texture.path = str.C_Str();
            textures.push_back(texture);
            textures_loaded.push_back(texture);
        }
    }
    return textures;
}

unsigned int Model::textureFromFile(const char *path, const std::string &directory) {
    std::string filename = std::string(path);
    filename = directory + '/' + filename;

    unsigned int textureId;
    glGenTextures(1, &textureId);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data){
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureId);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    else{
        Logger::Log<Model>(__FUNCTION__ ,"Texture failed to load at path: " + std::string(path), Logger::LogType::ERROR);
    }
    stbi_image_free(data);

    return textureId;
}

void Model::setScaleMatrix(const glm::mat4& matrix) {
    this->scale = matrix;
}

void Model::setRotateMatrix(const glm::mat4& matrix) {
    this->rotate = matrix;
}

void Model::setTranslateMatrix(const glm::mat4& matrix) {
    this->translate = matrix;
}

glm::mat4 Model::getModelMatrix() {
    modelMatrix = translate * rotate * scale;
    return modelMatrix;
}

void Model::setModelMatrix(const Shader &shader) {
    shader.use();
    shader.setMat4("model",getModelMatrix());
}

Model::~Model() {
    for (int i = 0; i < meshes.size(); ++i) {
        std::vector<Texture> vectorTextures = meshes[i].textures;
        for (int j = 0; j < vectorTextures.size(); ++j) {
            glDeleteTextures(1,&vectorTextures[i].id);
        }
    }
}

Intersection Model::intersect(const Ray& ray) {
    Intersection intersection;
    for (auto & mesh : meshes) {
        Intersection temp = mesh.intersect(ray);
        if (temp.happened && temp.distance < intersection.distance) intersection = temp;
    }
    if (intersection.happened) intersection.model = this;
    return intersection;
}

void Model::updateBox() {
    box = BoundingBox3();
    // update model's box
    glm::mat4 model = getModelMatrix();
    // update triangle's vertex coordinate for intersection test
    for (auto & triangle : meshes[0].triangles) {
        triangle.v0 = glm::vec3 (model * glm::vec4(triangle.v0,1.0f));
        triangle.v1 = glm::vec3 (model * glm::vec4(triangle.v1,1.0f));
        triangle.v2 = glm::vec3 (model * glm::vec4(triangle.v2,1.0f));
        box = Union(Union(Union(box,triangle.v0),triangle.v1),triangle.v2);
    }
    // also update mesh's box
    assert(meshes.size() == 1);
    meshes[0].box = box;
//    for (auto & mesh : meshes) {
//        mesh.box = box;
//    }
}

void Model::restoreBoundingBox(const glm::mat4 &invMatrix) {
    box.pMin = glm::vec3 (invMatrix * glm::vec4(box.pMin, 1.0f));
    box.pMax = glm::vec3 (invMatrix * glm::vec4(box.pMax, 1.0f));
    // also update mesh's box for intersection test
    assert(meshes.size() == 1);
    meshes[0].box = box;
    // update triangle's vertex coordinate for intersection test
    for (auto & triangle : meshes[0].triangles) {
        triangle.v0 = glm::vec3 (invMatrix * glm::vec4(triangle.v0, 1.0f));
        triangle.v1 = glm::vec3 (invMatrix * glm::vec4(triangle.v1, 1.0f));
        triangle.v2 = glm::vec3 (invMatrix * glm::vec4(triangle.v2, 1.0f));
    }
}

const glm::mat4 &Model::getScale() const {
    return scale;
}

const glm::mat4 &Model::getRotate() const {
    return rotate;
}

const glm::mat4 &Model::getTranslate() const {
    return translate;
}



