#include "Shader.h"
#include "../core/Log.h"
#include "../core/VFS.h" // Loading shaders via Virtual File System

#include <vector>

namespace aether {

    Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath)
    {
        // 1. Retrieve the source code from VFS
        std::string vertexCode = VFS::ReadText(vertexPath);
        std::string fragmentCode = VFS::ReadText(fragmentPath);

        // Sanity Check: Did the VFS fail?
        if (vertexCode.empty()) {
            AETHER_CORE_ERROR("Shader file empty or not found: {0}", vertexPath);
            return;
        }
        if (fragmentCode.empty()) {
            AETHER_CORE_ERROR("Shader file empty or not found: {0}", fragmentPath);
            return;
        }

        const char* vShaderCode = vertexCode.c_str();
        const char* fShaderCode = fragmentCode.c_str();

        // 2. Compile shaders
        unsigned int vertex, fragment;

        // Vertex Shader
        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vShaderCode, NULL);
        glCompileShader(vertex);
        CheckCompileErrors(vertex, "VERTEX");

        // Fragment Shader
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fShaderCode, NULL);
        glCompileShader(fragment);
        CheckCompileErrors(fragment, "FRAGMENT");

        // 3. Shader Program (Linking them together)
        m_RendererID = glCreateProgram();
        glAttachShader(m_RendererID, vertex);
        glAttachShader(m_RendererID, fragment);
        glLinkProgram(m_RendererID);
        CheckCompileErrors(m_RendererID, "PROGRAM");

        // 4. Cleanup: The individual shaders are linked now, we can delete the intermediates.
        glDeleteShader(vertex);
        glDeleteShader(fragment);

        AETHER_CORE_INFO("Shader Compiled Successfully: {0} / {1}", vertexPath, fragmentPath);
    }

    Shader::~Shader()
    {
        glDeleteProgram(m_RendererID);
    }

    void Shader::Bind() const
    {
        glUseProgram(m_RendererID);
    }

    void Shader::Unbind() const
    {
        glUseProgram(0);
    }

    void Shader::CheckCompileErrors(GLuint shader, std::string type)
    {
        GLint success;
        GLchar infoLog[1024];
        if (type != "PROGRAM")
        {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                AETHER_CORE_ERROR("SHADER_COMPILATION_ERROR of type: {0}\n{1}", type, infoLog);
            }
        }
        else
        {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success)
            {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                AETHER_CORE_ERROR("PROGRAM_LINKING_ERROR of type: {0}\n{1}", type, infoLog);
            }
        }
    }

    // --- Uniform Setters ---

    void Shader::SetMat4(const std::string& name, const void* value)
    {
        GLint location = glGetUniformLocation(m_RendererID, name.c_str());
        // GL_FALSE = Do not transpose the matrix
        glUniformMatrix4fv(location, 1, GL_FALSE, (const float*)value);
    }

    void Shader::SetFloat4(const std::string& name, float v0, float v1, float v2, float v3)
    {
        GLint location = glGetUniformLocation(m_RendererID, name.c_str());
        glUniform4f(location, v0, v1, v2, v3);
    }

    void Shader::SetInt(const std::string& name, int value)
    {
        GLint location = glGetUniformLocation(m_RendererID, name.c_str());
        glUniform1i(location, value);
    }

    void Shader::SetIntArray(const std::string& name, int* values, uint32_t count)
    {
        GLint location = glGetUniformLocation(m_RendererID, name.c_str());
        glUniform1iv(location, count, values);
    }
}