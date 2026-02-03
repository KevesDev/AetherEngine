#pragma once
#include <string>
#include <filesystem>

namespace aether {

    class Texture2D
    {
    public:
        Texture2D(uint32_t width, uint32_t height);
        Texture2D(const std::string& path);
        ~Texture2D();

        uint32_t GetWidth() const { return m_Width; }
        uint32_t GetHeight() const { return m_Height; }
        uint32_t GetRendererID() const { return m_RendererID; }

        void SetData(void* data, uint32_t size);
        void Bind(uint32_t slot = 0) const;

        bool operator==(const Texture2D& other) const { return m_RendererID == other.m_RendererID; }

    private:
        std::string m_Path;
        uint32_t m_Width, m_Height;
        uint32_t m_RendererID;
        uint32_t m_InternalFormat, m_DataFormat;
    };
}