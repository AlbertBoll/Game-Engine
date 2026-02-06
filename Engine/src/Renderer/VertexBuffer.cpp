#include"Renderer/VertexBuffer.h"
#include <glad/gl.h>


VertexBuffer::VertexBuffer(std::size_t sizeBytes, BufferUsage usage, const void* initialData)
    : m_SizeBytes(sizeBytes), m_Usage(usage)
{
    glGenBuffers(1, &m_RendererID);
    glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)m_SizeBytes, initialData, ToGLUsage(m_Usage));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}


VertexBuffer::~VertexBuffer()
{
    if (m_RendererID != 0)
        glDeleteBuffers(1, &m_RendererID);
}

VertexBuffer::VertexBuffer(VertexBuffer&& other) noexcept
{
    *this = std::move(other);
}

VertexBuffer& VertexBuffer::operator=(VertexBuffer&& other) noexcept
{
    if (this != &other)
    {
        if (m_RendererID)
            glDeleteBuffers(1, &m_RendererID);

        m_RendererID = other.m_RendererID;
        m_SizeBytes = other.m_SizeBytes;
        m_Usage = other.m_Usage;
        m_Layout = std::move(other.m_Layout);

        other.m_RendererID = 0;
        other.m_SizeBytes = 0;
        other.m_Usage = BufferUsage::StaticDraw;
    }
    return *this;
}

void VertexBuffer::Bind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
}

void VertexBuffer::Unbind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

bool VertexBuffer::Resize(std::size_t newSizeBytes)
{
    if (!m_RendererID) return false;
    m_SizeBytes = newSizeBytes;

    glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)m_SizeBytes, nullptr, ToGLUsage(m_Usage));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    return true;
}

bool VertexBuffer::SetData(const void* data, std::size_t sizeBytes, BufferUpdateMode mode)
{
    if (!m_RendererID || !data || sizeBytes == 0) return false;

    glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);

    if (sizeBytes != m_SizeBytes)
    {
        // size changes, reallocate buffer 
        m_SizeBytes = sizeBytes;
        glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)m_SizeBytes, data, ToGLUsage(m_Usage));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        return true;
    }

    if (mode == BufferUpdateMode::OrphanThenSubData)
    {
        // orphan
        glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)m_SizeBytes, nullptr, ToGLUsage(m_Usage));
    }

    // sub data
    glBufferSubData(GL_ARRAY_BUFFER, 0, (GLsizeiptr)sizeBytes, data);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    return true;
}   

void VertexBuffer::SetLayout(const BufferLayout& layout)
{
    m_Layout = layout;
}

const BufferLayout& VertexBuffer::GetLayout() const
{
    return m_Layout;
}

uint32_t VertexBuffer::GetRendererID() const
{
    return m_RendererID;
}


bool VertexBuffer::UpdateSubData(std::size_t offsetBytes, const void* data, std::size_t sizeBytes)
{
    if (!m_RendererID || !data || sizeBytes == 0) return false;
    if (offsetBytes + sizeBytes > m_SizeBytes) return false;

    glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
    glBufferSubData(GL_ARRAY_BUFFER, (GLintptr)offsetBytes, (GLsizeiptr)sizeBytes, data);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    return true;
}


// IndexBuffer::IndexBuffer(const uint32_t* indices, uint32_t count)
//     : m_Count(count)
// {
//     glGenBuffers(1, &m_RendererID);
//     glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
//     glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint32_t), indices, GL_STATIC_DRAW);
// }
