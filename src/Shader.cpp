#include "Shader.h"
#include <fstream>
#include <sstream>
#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>
Shader::Shader(const fs::path& vertex_path, const fs::path& fragment_path)
{
    // Create an input file stream.
    std::ifstream vertex_stream(vertex_path);
    std::stringstream vss;
    vss << vertex_stream.rdbuf();


    std::ifstream fragment_stream(fragment_path);
    std::stringstream fss;
    fss << fragment_stream.rdbuf();
    

    shader_id = CreateShader(vss.str(), fss.str());
}

Shader::~Shader()
{
    glDeleteProgram(shader_id);
}

unsigned int Shader::CompileShader(unsigned int type, const std::string& source)
{
    unsigned int id = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE)
    {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char* message = (char*)_malloca(length * sizeof(char));
        glGetShaderInfoLog(id, length, &length, message);
        spdlog::error("Failed to compile ", (type == GL_VERTEX_SHADER ? "vertex" : "fragment"), " shader");
        spdlog::error(message);
        glDeleteShader(id);
        return 0;
    }

    return id;
}

unsigned int Shader::CreateShader(const std::string& vertex_shader, const std::string& fragment_shader)
{
    // Create the shader program.
    unsigned int program = glCreateProgram();
    // Compile the individual shaders.
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertex_shader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragment_shader);

    // Attach shaders to the program and link them.
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glValidateProgram(program);

    // Delete the individual shaders as they are now linked into the program.
    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

void Shader::Bind() const
{
    // Make this shader program the active one.
    glUseProgram(shader_id);
}

void Shader::Unbind() const
{
    // Unbind the current shader program.
   glUseProgram(0);
}

int Shader::GetUniformLocation(const std::string& name)
{
    // Check if the uniform location is already in the cache.
    if (m_UniformLocationCache.find(name) != m_UniformLocationCache.end())
        return m_UniformLocationCache[name];

    // If not in the cache, retrieve it and store it.
    int location = glGetUniformLocation(shader_id, name.c_str());
    if (location == -1) {
        spdlog::error("Warning: uniform {} doesn't exist!", name);
    }

    m_UniformLocationCache[name] = location;
    return location;
}

void Shader::SetUniform1i(const std::string& name, int value)
{
    (glUniform1i(GetUniformLocation(name), value));
}

void Shader::SetUniform1f(const std::string& name, float value)
{
    glUniform1f(GetUniformLocation(name), value);
}
void Shader::SetUniform3f(const std::string& name, float v0, float v1, float v2)
{
    glUniform3f(GetUniformLocation(name), v0, v1, v2);
}

void Shader::SetUniform3f(const std::string& name, glm::vec3 xyz)
{
    glUniform3f(GetUniformLocation(name), xyz.x, xyz.y, xyz.z);
}

void Shader::SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3)
{
    glUniform4f(GetUniformLocation(name), v0, v1, v2, v3);
}

void Shader::SetUniformMat4f(const std::string& name, const glm::mat4& matrix)
{
    glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, &matrix[0][0]);
}
