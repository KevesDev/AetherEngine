#include "Buffer.h"
#include <glad/glad.h>

namespace aether {

    // --- VertexBuffer ---

    VertexBuffer::VertexBuffer(uint32_t size)
    {
        // Create a dynamic buffer (for batch rendering later)
        glCreateBuffers(1, &m_RendererID);
        glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
        glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
    }

    VertexBuffer::VertexBuffer(float* vertices, uint32_t size)
    {
        // Create a static buffer (for simple static geometry)
        glCreateBuffers(1, &m_RendererID);
        glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
        glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
    }

    VertexBuffer::~VertexBuffer()
    {
        glDeleteBuffers(1, &m_RendererID);
    }

    void VertexBuffer::Bind() const
    {
        glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
    }

    void VertexBuffer::Unbind() const
    {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void VertexBuffer::SetData(const void* data, uint32_t size)
    {
        glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
        glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
    }

    // --- IndexBuffer ---

    IndexBuffer::IndexBuffer(uint32_t* indices, uint32_t count)
        : m_Count(count)
    {
        glCreateBuffers(1, &m_RendererID);

        // GL_ELEMENT_ARRAY_BUFFER is special; it's only valid if a VAO is bound (usually).
        // But glCreateBuffers + glNamedBufferData (OpenGL 4.5+) handles state better.
        // For compatibility with 4.1/4.2, we use standard binding.
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint32_t), indices, GL_STATIC_DRAW);
    }

    IndexBuffer::~IndexBuffer()
    {
        glDeleteBuffers(1, &m_RendererID);
    }

    void IndexBuffer::Bind() const
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
    }

    void IndexBuffer::Unbind() const
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

}