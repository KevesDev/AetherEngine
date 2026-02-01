#pragma once
#pragma once

#include <string>
#include <glad/glad.h>

namespace aether {

    class Shader
    {
    public:
        // Constructor: Loads files via VFS, compiles, and links.
        Shader(const std::string& vertexPath, const std::string& fragmentPath);
        ~Shader();

        void Bind() const;
        void Unbind() const;

        // Uniforms: Ways to send data (Matrices, Floats) to the GPU
        // We use void* for matrices to avoid forcing a math library dependency just yet.
        void SetMat4(const std::string& name, const void* value);
        void SetFloat4(const std::string& name, float v0, float v1, float v2, float v3);
        void SetInt(const std::string& name, int value);
        void SetIntArray(const std::string& name, int* values, uint32_t count);

    private:
        // Helper to check for syntax errors in shaders
        void CheckCompileErrors(GLuint shader, std::string type);

    private:
        GLuint m_RendererID; // The GPU handle for this shader program
    };

}