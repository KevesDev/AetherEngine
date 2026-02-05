#include "Shader.h"
#include "../core/Log.h"
#include "../core/VFS.h"
#include <sstream>
#include <vector>

namespace aether {

    Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath)
    {
        // 1. Read source via VFS
        std::string source = VFS::ReadText(vertexPath);
        AETHER_ASSERT(!source.empty(), "Shader source is empty or file not found: {0}", vertexPath);

        // 2. Parse the single file into multiple shader sources
        // This is a robust line-by-line state machine
        std::stringstream ss(source);
        std::string line;
        std::stringstream shaderSources[2]; // 0: Vertex, 1: Fragment
        int mode = -1; // -1: None, 0: Vertex, 1: Fragment

        while (std::getline(ss, line))
        {
            if (line.find("#type") != std::string::npos)
            {
                if (line.find("vertex") != std::string::npos)
                    mode = 0;
                else if (line.find("fragment") != std::string::npos)
                    mode = 1;

                AETHER_ASSERT(mode != -1, "Invalid shader type specified in: {0}", vertexPath);
            }
            else if (mode != -1)
            {
                shaderSources[mode] << line << "\n";
            }
        }

        std::string vSource = shaderSources[0].str();
        std::string fSource = shaderSources[1].str();

        AETHER_ASSERT(!vSource.empty(), "Vertex shader source is missing in {0}", vertexPath);
        AETHER_ASSERT(!fSource.empty(), "Fragment shader source is missing in {0}", vertexPath);

        const char* vShaderCode = vSource.c_str();
        const char* fShaderCode = fSource.c_str();

        // 3. Compile Shaders
        uint32_t vertex, fragment;

        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vShaderCode, NULL);
        glCompileShader(vertex);
        CheckCompileErrors(vertex, "VERTEX");

        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fShaderCode, NULL);
        glCompileShader(fragment);
        CheckCompileErrors(fragment, "FRAGMENT");

        // 4. Link Program
        m_RendererID = glCreateProgram();
        glAttachShader(m_RendererID, vertex);
        glAttachShader(m_RendererID, fragment);
        glLinkProgram(m_RendererID);
        CheckCompileErrors(m_RendererID, "PROGRAM");

        // Production Guard: If linking failed, we must know immediately
        AETHER_ASSERT(m_RendererID != 0, "Shader program linking failed for {0}", vertexPath);

        glDeleteShader(vertex);
        glDeleteShader(fragment);

        AETHER_CORE_INFO("Shader Compiled Successfully: {0}", vertexPath);
    }

    Shader::~Shader()
    {
        glDeleteProgram(m_RendererID);
        // m_UniformLocationCache automatically cleared (std::unordered_map destructor)
    }

    void Shader::Bind() const
    {
        glUseProgram(m_RendererID);
    }

    void Shader::Unbind() const
    {
        glUseProgram(0);
    }

    void Shader::CheckCompileErrors(uint32_t shader, std::string type)
    {
        int success;
        char infoLog[1024];
        if (type != "PROGRAM")
        {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                AETHER_CORE_ERROR("SHADER_COMPILATION_ERROR ({0}):\n{1}", type, infoLog);
                AETHER_DEBUGBREAK(); // Force break on shader syntax errors
            }
        }
        else
        {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success)
            {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                AETHER_CORE_ERROR("PROGRAM_LINKING_ERROR:\n{0}", infoLog);
                AETHER_DEBUGBREAK();
            }
        }
    }

    // -------------------------------------------------------------------------
    // Cached Uniform Location Retrieval
    // -------------------------------------------------------------------------
    int Shader::GetUniformLocation(const std::string& name)
    {
        // Check cache first (fast path)
        if (m_UniformLocationCache.find(name) != m_UniformLocationCache.end())
            return m_UniformLocationCache[name];

        // Query GPU (slow path - only happens once per uniform)
        int location = glGetUniformLocation(m_RendererID, name.c_str());

        // Cache the result (even if -1, to avoid repeated failed queries)
        m_UniformLocationCache[name] = location;

        // Log warning for missing uniforms (helps catch typos in shader code)
        if (location == -1)
            AETHER_CORE_WARN("Shader: Uniform '{0}' not found or optimized out by compiler", name);

        return location;
    }

    // -------------------------------------------------------------------------
    // Uniform Upload Methods (Now Using Cache)
    // -------------------------------------------------------------------------

    void Shader::SetMat4(const std::string& name, const void* value)
    {
        int location = GetUniformLocation(name);
        if (location != -1)
            glUniformMatrix4fv(location, 1, GL_FALSE, (const float*)value);
    }

    void Shader::SetFloat4(const std::string& name, float v0, float v1, float v2, float v3)
    {
        int location = GetUniformLocation(name);
        if (location != -1)
            glUniform4f(location, v0, v1, v2, v3);
    }

    void Shader::SetInt(const std::string& name, int value)
    {
        int location = GetUniformLocation(name);
        if (location != -1)
            glUniform1i(location, value);
    }

    void Shader::SetIntArray(const std::string& name, int* values, uint32_t count)
    {
        int location = GetUniformLocation(name);
        if (location != -1)
            glUniform1iv(location, count, values);
    }
}