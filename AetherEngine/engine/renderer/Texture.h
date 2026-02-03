#pragma once

#include <string>
#include <filesystem>
#include <glad/glad.h>

namespace aether {

    // Defines the layout of the texture data
    enum class ImageFormat
    {
        None = 0,
        R8,
        RGB8,
        RGBA8,
        RGBA32F
    };

    // Configuration for creating a texture.
    // This will eventually be serialized into the .aeth file.
    struct TextureSpecification
    {
        uint32_t Width = 1;
        uint32_t Height = 1;
        ImageFormat Format = ImageFormat::RGBA8;
        bool GenerateMips = true;

        // Filtering (GL_LINEAR or GL_NEAREST)
        int MinFilter = GL_LINEAR;
        int MagFilter = GL_NEAREST;

        // Wrapping (GL_REPEAT or GL_CLAMP_TO_EDGE)
        int WrapS = GL_REPEAT;
        int WrapT = GL_REPEAT;
    };

    class Texture2D
    {
    public:
        // Create an empty texture with specific settings (e.g. for Framebuffers)
        Texture2D(const TextureSpecification& specification);

        // Create from file with specific settings
        Texture2D(const std::string& path, const TextureSpecification& specification = TextureSpecification());

        ~Texture2D();

        const TextureSpecification& GetSpecification() const { return m_Specification; }

        uint32_t GetWidth() const { return m_Width; }
        uint32_t GetHeight() const { return m_Height; }
        uint32_t GetRendererID() const { return m_RendererID; }

        const std::string& GetPath() const { return m_Path; }

        // Uploads raw data to the GPU
        void SetData(void* data, uint32_t size);

        // Binds the texture to a slot (0-31)
        void Bind(uint32_t slot = 0) const;

        bool operator==(const Texture2D& other) const { return m_RendererID == other.m_RendererID; }

    private:
        TextureSpecification m_Specification;
        std::string m_Path;
        uint32_t m_Width, m_Height;
        uint32_t m_RendererID;
        GLenum m_InternalFormat, m_DataFormat;
    };
}