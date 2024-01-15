//
// Created by shiyin on 2023/12/8.
//

#include "../core/shader.h"

std::string readDataFromFile(const char* path){
    std::string code;
    std::ifstream shaderFile;
    shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try{
        // open file
        shaderFile.open(path);
        std::stringstream shaderStream;
        shaderStream << shaderFile.rdbuf();
        // close file
        shaderFile.close();
        // translate stream data to string
        code = shaderStream.str();
    }catch(std::ifstream::failure e){
        Logger::Log<Shader>(__FUNCTION__ ,"ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ : " + std::string (path));
    }
    return code;
}

Shader::Shader(const char* vertexShaderPath, const char* fragmentShaderPath, const char* geometryPath){

    std::string vertexShaderString = readDataFromFile(vertexShaderPath);
    std::string fragmentShaderString = readDataFromFile(fragmentShaderPath);
    const char *vShaderCode = vertexShaderString.c_str();
    const char *fShaderCode = fragmentShaderString.c_str();

    std::string geometryShaderString;
    const char *geometryCode = nullptr;
    if (geometryPath){
        geometryShaderString = readDataFromFile(geometryPath);
        geometryCode = geometryShaderString.c_str();
    }

    //create shader
    unsigned int vertexShader;
    unsigned int fragmentShader;

    //vertex shader
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader,1,&vShaderCode, nullptr);
    glCompileShader(vertexShader);
    checkCompileErrors(vertexShader, "vertexShader",vertexShaderPath);

    //fragment shader
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader,1,&fShaderCode,nullptr);
    glCompileShader(fragmentShader);
    checkCompileErrors(fragmentShader,"fragmentShader",fragmentShaderPath);

    //geometry shader
    unsigned int geometryShader;
    if (geometryPath != nullptr) {
        const char* gShaderCode = geometryCode;
        geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(geometryShader, 1, &gShaderCode, nullptr);
        glCompileShader(geometryShader);
        checkCompileErrors(geometryShader, "geometryShader",geometryPath);
    }

    //shader program
    ID = glCreateProgram();
    glAttachShader(ID,vertexShader);
    glAttachShader(ID,fragmentShader);
    if (geometryPath != nullptr) {
        glAttachShader(ID, geometryShader);
    }
    glLinkProgram(ID);
    checkCompileErrors(ID,"PROGRAM","");

    //delete shaders after linking
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    if (geometryPath != nullptr) {
        glDeleteShader(geometryShader);
    }
}

void Shader::use() const
{
    glUseProgram(ID);
}

void Shader::setBool(const std::string& name, bool value) const
{
    glUniform1i(glGetUniformLocation(ID,name.c_str()),(int)value);
}

void Shader::setInt(const std::string& name, int value) const
{
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setFloat(const std::string& name, float value) const
{
    glUniform1f(glGetUniformLocation(ID,name.c_str()),value);
}

void Shader::setMat4(const std::string& name, const glm::mat4& m4) const
{
    glUniformMatrix4fv(glGetUniformLocation(ID,name.c_str()),1,GL_FALSE,&m4[0][0]);
}

void Shader::setVec3(const std::string& name, float x, float y, float z) const
{
    glUniform3f(glGetUniformLocation(ID,name.c_str()), x, y, z);
}

void Shader::setVec3(const std::string& name, const glm::vec3& v3) const
{
    glUniform3fv(glGetUniformLocation(ID,name.c_str()),1,&v3[0]);
}

void Shader::setVec4(const std::string& name, const glm::vec4& v4) const
{
    glUniform4fv(glGetUniformLocation(ID,name.c_str()),1,&v4[0]);
}

void Shader::setVec4(const std::string& name, float x, float y, float z, float w) const
{
    glUniform4f(glGetUniformLocation(ID,name.c_str()),x,y,z,w);
}


void Shader::setVec2(const std::string& name, const glm::vec2& v2) const
{
    glUniform2fv(glGetUniformLocation(ID,name.c_str()),1,&v2[0]);
}

void Shader::setVec2(const std::string& name, float x, float y) const
{
    glUniform2f(glGetUniformLocation(ID,name.c_str()),x,y);
}

void Shader::setMat3(const std::string &name, const glm::mat3 &m3) const {
    glUniformMatrix3fv(glGetUniformLocation(ID,name.c_str()),1,GL_FALSE,&m3[0][0]);
}

void Shader::checkCompileErrors(const GLuint& shaderId, const std::string& type, const std::string& path)
{
    GLint success;
    GLchar infoLog[1024];
    if (type == "PROGRAM") {
        glGetProgramiv(shaderId, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shaderId, 512, nullptr, infoLog);
            Logger::Log<Shader>(__FUNCTION__ ,
                                "ERROR::PROGRAM_LINKING_ERROR of path: " + path + "\n" + infoLog + "\n -- --------------------------------------------------- -- ");
        }
    }
    else {
        glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shaderId, 512, nullptr, infoLog);
            Logger::Log<Shader>(__FUNCTION__ ,
                                "ERROR::SHADER_COMPILATION_ERROR of PATH: " + path + "\n" + infoLog + "\n -- --------------------------------------------------- -- ");
        }
    }
}
