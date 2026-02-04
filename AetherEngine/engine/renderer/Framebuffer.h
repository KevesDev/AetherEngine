#pragma once
#include <cstdint>
#include <memory>
#include <vector>

namespace aether {

    /**
     * Framebuffer Texture Format
     * Defines the pixel layout for each attachment.
     */
    enum class FramebufferTextureFormat
    {
        None = 0,

        // Color Attachments
        RGBA8,
        RED_INTEGER,

        // Depth/Stencil
        Depth = 10,
        DEPTH24STENCIL8
    };

    struct FramebufferTextureSpecification
    {
        FramebufferTextureSpecification() = default;
        FramebufferTextureSpecification(FramebufferTextureFormat format)
            : TextureFormat(format) {
        }

        FramebufferTextureFormat TextureFormat = FramebufferTextureFormat::None;
    };

    struct FramebufferAttachmentSpecification
    {
        FramebufferAttachmentSpecification() = default;
        FramebufferAttachmentSpecification(std::initializer_list<FramebufferTextureSpecification> attachments)
            : Attachments(attachments) {
        }

        std::vector<FramebufferTextureSpecification> Attachments;
    };

    struct FramebufferSpecification
    {
        uint32_t Width = 0;
        uint32_t Height = 0;
        FramebufferAttachmentSpecification Attachments;
        uint32_t Samples = 1;
        bool SwapChainTarget = false;
    };

    /**
     * Framebuffer: Offscreen render target abstraction.
     * Supports Multiple Render Targets (MRT) for advanced rendering techniques.
     */
    class Framebuffer
    {
    public:
        virtual ~Framebuffer() = default;

        virtual void Bind() = 0;
        virtual void Unbind() = 0;

        virtual void Resize(uint32_t width, uint32_t height) = 0;

        /**
         * ReadPixel: Reads a single pixel from a specific attachment.
         * Used for mouse picking (Entity ID stored in RED_INTEGER attachment).
         *
         * @param attachmentIndex: Which attachment to read from (0 = color, 1 = entity ID)
         * @param x, y: Pixel coordinates
         * @return Pixel value (int for RED_INTEGER, color for RGBA8)
         */
        virtual int ReadPixel(uint32_t attachmentIndex, int x, int y) = 0;

        /**
         * ClearAttachment: Clears a specific attachment to a value.
         * Used to reset the entity ID buffer to -1 each frame.
         *
         * @param attachmentIndex: Which attachment to clear
         * @param value: Clear value
         */
        virtual void ClearAttachment(uint32_t attachmentIndex, int value) = 0;

        virtual uint32_t GetColorAttachmentRendererID(uint32_t index = 0) const = 0;
        virtual const FramebufferSpecification& GetSpecification() const = 0;

        static std::shared_ptr<Framebuffer> Create(const FramebufferSpecification& spec);
    };
}