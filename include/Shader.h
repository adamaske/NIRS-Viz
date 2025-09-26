#pragma once

#include <string>
#include <unordered_map>
#include <glm/glm.hpp>
#include <filesystem>

namespace fs = std::filesystem;
// This class provides a convenient wrapper for an OpenGL shader program.
// It handles file loading, compilation, linking, and uniform management.
class Shader
{
private:
    unsigned int shader_id;
    // Cache for uniform locations to avoid repeated calls to glGetUniformLocation
    std::unordered_map<std::string, int> m_UniformLocationCache;

public:
    // Constructor: Creates a shader program from a single file containing both
    // vertex and fragment shader source code, separated by preprocessor directives.
    Shader(const fs::path& vertex_path, const fs::path& fragment_path);
    // Destructor: Deletes the shader program from GPU memory.
    ~Shader();

    // Binds the shader program, making it the active one for rendering.
    void Bind() const;
    // Unbinds the currently active shader program.
    void Unbind() const;

    // --- Set Uniforms ---

    // Sets a uniform integer variable.
    void SetUniform1i(const std::string& name, int value);
    // Sets a uniform float variable.
    void SetUniform1f(const std::string& name, float value);
    void SetUniform3f(const std::string& name, float v0, float v1, float v2);
    void SetUniform3f(const std::string& name, glm::vec3 xyz);
    // Sets a uniform vec4 variable.
    void SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3);
    // Sets a uniform mat4 variable.
    void SetUniformMat4f(const std::string& name, const glm::mat4& matrix);

private:
    // Retrieves the location of a uniform variable by name.
    // It uses a cache to optimize repeated lookups.
    int GetUniformLocation(const std::string& name);
    // Parses the shader file, compiles the shaders, and links them into a program.
    unsigned int CreateShader(const std::string& vertex_shader, const std::string& fragment_shader);
    // Compiles a single shader (vertex or fragment) and checks for errors.
    unsigned int CompileShader(unsigned int type, const std::string& source);
};
