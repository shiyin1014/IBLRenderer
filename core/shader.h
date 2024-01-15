//
// Created by shiyin on 2023/12/8.
//

#pragma once

#ifndef SIMPLERENDERER_SHADER_H
#define SIMPLERENDERER_SHADER_H

#include "logger.h"
#include "glad/glad.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <cstring>
#include <iostream>
#include <sstream>
#include <fstream>

class Shader {
public:
    //shader_id
    unsigned int ID;

    Shader() = default;

    Shader(const char* vertexShader, const char* fragmentShader, const char* geometryPath = nullptr);

    void use() const;

    //uniform tools
    void setBool(const std::string &name, bool value) const;
    void setInt(const std::string &name, int value) const;
    void setFloat(const std::string &name, float value) const;
    void setMat3(const std::string& name, const glm::mat3 & m3) const;
    void setMat4(const std::string &name, const glm::mat4& m4) const;
    void setVec3(const std::string& name, float x, float y, float z) const;
    void setVec3(const std::string& name, const glm::vec3& v3) const;
    void setVec4(const std::string& name, const glm::vec4& v4) const;
    void setVec4(const std::string& name, float x, float y, float z, float w) const ;
    void setVec2(const std::string& name, const glm::vec2& v2) const;
    void setVec2(const std::string& name, float x, float y) const;

private:

    static void checkCompileErrors(const GLuint& shaderId, const std::string& type, const std::string& path);

};

#endif //SIMPLERENDERER_SHADER_H
