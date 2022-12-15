//
// Created by Xiaoxia Zhang on 12/13/22.
//

#ifndef MESH_PREVIEW_SHADER_H
#define MESH_PREVIEW_SHADER_H
#include <string>
#include "glm/glm.hpp"

class Shader
{
public:
    // the program ID
    unsigned int ID;

    // constructor reads and builds the shader
    Shader(const char* vertexPath, const char* fragmentPath);
    // use/activate the shader
    void Use();
    // utility uniform functions
    void SetBool(const std::string &name, bool value) const;
    void SetInt(const std::string &name, int value) const;
    void SetFloat(const std::string &name, float value) const;
    void SetUniformFloat3(const std::string &name, const glm::vec3& value) const;
    void SetUniformMatrix4f(const std::string &name, const glm::mat4& matrix) const;
};

#endif //MESH_PREVIEW_SHADER_H
