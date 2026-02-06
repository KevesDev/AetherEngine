#include "Texture.h"
#include "../core/Log.h"
#include <glad/glad.h>
#include <stdexcept>

#define STB_IMAGE_IMPLEMENTATION
#include "../vendor/stb_image.h"

namespace aether {

    static GLenum ImageFormatToGLDataFormat(ImageFormat format)
    {
        switch (format)
        {
        case ImageFormat::RGB8:  return GL_RGB;
        case ImageFormat::RGBA8: return GL_RGBA;
        }
        return 0;
    }

    static GLenum ImageFormatToGLInternalFormat(ImageFormat format)
    {
        switch (format)
        {
        case ImageFormat::RGB8:  return GL_RGB8;
        case ImageFormat::RGBA8: return GL_RGBA8;
        }
        return 0;
    }

    Texture2D::Texture2D(const TextureSpecification& specification)
        : m_Specification(specification), m_Width(m_Specification.Width), m_Height(m_Specification.Height)
    {
        m_InternalFormat = ImageFormatToGLInternalFormat(m_Specification.Format);
        m_DataFormat = ImageFormatToGLDataFormat(m_Specification.Format);

        glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
        glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height);

        glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, m_Specification.MinFilter);
        glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, m_Specification.MagFilter);

        glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, m_Specification.WrapS);
        glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, m_Specification.WrapT);
    }

    Texture2D::Texture2D(const std::string& path, const TextureSpecification& specification)
        : m_Path(path), m_Specification(specification)
    {
        int width, height, channels;
        stbi_set_flip_vertically_on_load(1);
        stbi_uc* data = stbi_load(path.c_str(), &width, &height, &channels, 0);

        if (data)
        {
            m_Width = width;
            m_Height = height;

            m_Specification.Width = width;
            m_Specification.Height = height;

            GLenum internalFormat = 0, dataFormat = 0;
            if (channels == 4)
            {
                internalFormat = GL_RGBA8;
                dataFormat = GL_RGBA;
            }
            else if (channels == 3)
            {
                internalFormat = GL_RGB8;
                dataFormat = GL_RGB;
            }

            m_InternalFormat = internalFormat;
            m_DataFormat = dataFormat;

            if (internalFormat == 0)
            {
                AETHER_CORE_ERROR("Texture2D: Unsupported format (channels: {0}) in '{1}'", channels, path);
                stbi_image_free(data);
                m_RendererID = 0;
                throw std::runtime_error("Unsupported texture format");
            }

            glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
            glTextureStorage2D(m_RendererID, 1, internalFormat, m_Width, m_Height);

            glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, m_Specification.MinFilter);
            glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, m_Specification.MagFilter);

            glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, m_Specification.WrapS);
            glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, m_Specification.WrapT);

            glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, dataFormat, GL_UNSIGNED_BYTE, data);

            stbi_image_free(data);
        }
        else
        {
            // Ensure ID is 0 so Destructor doesn't delete garbage
            m_RendererID = 0;
            AETHER_CORE_ERROR("Texture2D: Failed to load image from '{0}'\nReason: {1}", path, stbi_failure_reason());
            throw std::runtime_error("Texture load failure");
        }
    }

    Texture2D::~Texture2D()
    {
        if (m_RendererID != 0)
            glDeleteTextures(1, &m_RendererID);
    }

    void Texture2D::SetData(void* data, uint32_t size)
    {
        if (m_RendererID == 0) return;

        uint32_t bpp = m_DataFormat == GL_RGBA ? 4 : 3;
        // Check size if strictly needed, otherwise trust caller for performance
        glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data);
    }

    void Texture2D::Bind(uint32_t slot) const
    {
        if (m_RendererID != 0)
            glBindTextureUnit(slot, m_RendererID);
    }
}